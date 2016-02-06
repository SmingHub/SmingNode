#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "NodeCore.h"
#include "Sensor.h"
#include "Trigger.h"
#include "TaskTimer.h"
#include "ScriptCore.h"
#include "Slave.h"

NodeCore::NodeCore()
{
	js = new ScriptCore();
}

bool NodeCore::load()
{
	if (!exist()) return false;

	DynamicJsonBuffer jsonBuffer;
	bool parsed = false;
	if (exist())
	{
		int size = fileGetSize(APP_SETTINGS_FILE);
		debugf("size: %d", size);
		char* jsonString = new char[size + 1];
		fileGetContent(APP_SETTINGS_FILE, jsonString, size + 1);
		JsonObject& root = jsonBuffer.parseObject(jsonString);
		parsed = root.success();
		if (parsed)
		{
			debugf("json parsed");
			readConfigTree(root);
		}
		else
			debugf("Can't load file");

		delete[] jsonString;
	}
	return parsed;
}

void NodeCore::startNetwork()
{
	mqtt->connect(name);
	mqtt->subscribe(mqttPrefix + "/commands/#");
}

void NodeCore::readConfigTree(JsonObject& root)
{
	name = root["name"].asString();
	debugf("%s starting", name.c_str());

	JsonObject& network = root["network"];
	ssid = network["ssid"].asString();
	password = network["password"].asString();

	if (root.containsKey("mqtt"))
	{
		JsonObject& treeMqtt = root["mqtt"];
		String host = treeMqtt["host"].asString();
		int port = treeMqtt["port"].as<int>();
		mqttPrefix = "SmingNode/" + name;
		debugf("MQTT %s: %s %d", mqttPrefix.c_str(), host.c_str(), port);
		mqtt = new MqttClient(host, port, MqttStringSubscriptionCallback(&NodeCore::onMqttMessageReceived, this));
		startNetwork();
	}

	if (root.containsKey("sensors"))
	{
		JsonObject& sensors = root["sensors"];
		for (JsonObject::iterator it = sensors.begin(); it != sensors.end(); ++it)
		{
			JsonObject& data = it->value;
			String name = (const char*)it->key;
			String type = name;
			if (data.containsKey("type"))
				type = data["type"].asString();
			debugf("sensor: %s of type %s", name.c_str(), type.c_str());
			auto sensor = createSensor(type, data);
			if (sensor == nullptr)
				continue;
			sensor->setName(name);
			sensor->executeRepeating();
			this->sensors.add(sensor);
		}
	}
	if (root.containsKey("slaves"))
	{
		debugf("slaves:");
		JsonObject& treePins = root["slaves"];
		for (JsonObject::iterator it = treePins.begin(); it != treePins.end(); ++it)
		{
			JsonVariant& data = it->value;
			String name = (const char*)it->key;

			int gpio = data.as<int>();
			Slave *p = new Slave();
			p->load(data);
			p->setParent(this);
			p->setName(name);
			pins[name] = p;

			js->addHandler(name, "write", ScriptCore::DelegateSetB(&Slave::write, p));
			js->addHandler(name, "read", ScriptCore::DelegateGetB(&Slave::read, p));
			//js->addHandler(name, "outputState", ScriptCore::DelegateGetB(&Pin::outputState, p));
			debugf("%s on %d", name.c_str(), gpio);
		}
	}
	if (root.containsKey("triggers"))
	{
		JsonObject& treeTrig = root["triggers"];
		for (JsonObject::iterator it = treeTrig.begin(); it != treeTrig.end(); ++it)
		{
			JsonVariant& data = it->value;
			String name = (const char*)it->key;
			String type = (const char*)it->key;
//			if (data.containsKey("type"))
//				type = data["type"];

			Trigger* trig = new ExpressionTrigger();
			trig->load(data);
			if (trig == nullptr)
				continue;
			trig->setParent(this);
			trig->setName(name);
			trig->executeRepeating();
			triggers.add(trig);

			js->addHandler(name, "isEnabled", ScriptCore::DelegateGetB(&BaseObject::isEnabled, (BaseObject*)trig));
			js->addHandler(name, "setEnabled", ScriptCore::DelegateSetB(&BaseObject::setEnabled, (BaseObject*)trig));
		}
	}
	if (root.containsKey("timers"))
	{
		JsonObject& treeTimers = root["timers"];
		for (JsonObject::iterator it = treeTimers.begin(); it != treeTimers.end(); ++it)
		{
			JsonObject& data = it->value;
			String name = (const char*)it->key;

			TaskTimer* tmr = new TaskTimer();
			tmr->load(data);
			if (tmr == nullptr)
				continue;
			tmr->setParent(this);
			tmr->setName(name);
			tmr->executeRepeating();
			timers.add(tmr);

//			js->addHandler(name, "isEnabled", ScriptCore::DelegateGetB(&BaseObject::isEnabled, (BaseObject*)tmr));
//			js->addHandler(name, "setEnabled", ScriptCore::DelegateSetB(&BaseObject::setEnabled, (BaseObject*)tmr));
			js->addHandler(name, "start", ScriptCore::DelegateV(&TaskTimer::start, tmr));
			js->addHandler(name, "startOnce", ScriptCore::DelegateV(&TaskTimer::startOnce, tmr));
		}
	}
}

void NodeCore::save()
{
	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	JsonObject& network = jsonBuffer.createObject();
	root["network"] = network;
	network["ssid"] = ssid.c_str();
	network["password"] = password.c_str();
	//TODO: add direct file stream writing
	String rootString;
	root.printTo(rootString);
	fileSetContent(APP_SETTINGS_FILE, rootString);
}

void NodeCore::collectSensors(String& result)
{
	for (int i = 0; i < sensors.count(); i++)
	{
		for (int k = 0; k < sensors[i]->getVariablesCount(); k++)
		{
			Variable* var = sensors[i]->getVariable(k);
			String sv = String(var->getValue());
			result += var->getFullName() + ": " + sv + "\r\n";
		}
	}
}

void NodeCore::updateVariable(Variable* var, double value)
{
	char buf[96];
	String sv = String(value);
	sprintf(buf, "var %s = %s", var->getFullName().c_str(), sv.c_str());
	executeJS(buf);

	if (mqtt->getConnectionState() == eTCS_Connected)
		mqtt->publish(mqttPrefix + "/sensors/" + var->getFullName('/'), sv);

	if (WifiStation.isConnected() && mqtt->getConnectionState() != eTCS_Connected && mqtt->getConnectionState() != eTCS_Connecting)
		startNetwork();
}

void NodeCore::executeJS(String cmd)
{
	if (!cmd.endsWith(";"))
		cmd += ";";
	//debugf("JS> %s", cmd.c_str());
	js->execute(cmd);
}

double NodeCore::evaluateJS(const String& cmd)
{
	String str = js->evaluate(cmd);
	float res = str.toFloat();
	debugf("JS res: %s", str.c_str());
	return res;
}

Sensor* NodeCore::createSensor(String type, JsonObject& data)
{
	Sensor* result = Sensor::sensorFactory(type);

	result->setParent(this);
	result->load(data);
	return result;
}

void NodeCore::onMqttMessageReceived(String topic, String message)
{
	debugf("MQTT: %s", message.c_str());
}
