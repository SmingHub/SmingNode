#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include "Sensor.h"
#include "NodeCore.h"
#include "SpecificSensors.h"

double Variable::getValue() const
{
	return value;
}
void Variable::setValue(double value)
{
	this->value = value;
	sensor->getParent()->updateVariable(this, value);
}

String Variable::getFullName(const unsigned char separator /*= '.'*/)
{
	char buf[64];
	sprintf(buf, "%s%c%s", this->getSensor()->getName().c_str(), separator, this->named.c_str());
	return String(buf);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

Sensor* Sensor::sensorFactory(String type)
{
	Sensor* result = nullptr;
	if (type == "analog")
		result = new SensorAnalogReader();
	else if (type == "button")
		result = new SensorButton();
	else if (type == "dht22" || type == "dht11" || type == "dht21")
		result = new SensorDHT(type);
	else if (type == "ds1820" ||  type == "ds18b20")
		result = new SensorDS1820();
	else
	{
		debugf("UNKN SENSOR: %s", type.c_str());
		return result;
	}

	return result;
}

void Sensor::load(JsonObject& data)
{
	if (data.containsKey("pins"))
	{
		if (data["pins"].is<JsonObject&>())
		{
			JsonObject& pin = data["pins"];
			for (JsonObject::iterator it = pin.begin(); it != pin.end(); ++it)
			{
				String var = (const char*)it->key;
				int gpio = it->value.as<int>();
				debugf("pin %s on gpio%d", var.c_str(), gpio);
				pins[var] = gpio;
				pinMode(gpio, INPUT_PULLUP);
			}
		}
		else
		{
			int gpio = data["pins"].as<int>();
			debugf("pin gpio%d", gpio);
			pins[DFNAME] = gpio;
			pinMode(gpio, INPUT_PULLUP);
		}
	}
	if (data.containsKey("output"))
	{
		if (data["output"].is<JsonObject&>())
		{
			JsonObject& output = data["output"];
			for (JsonObject::iterator it = output.begin(); it != output.end(); ++it)
			{
				String var = (const char*)it->key;
				String named = it->value.asString();
				//debugf("output %s as %s", var.c_str(), named.c_str());
				inversedVars[named] = Variable(this, var, named);
			}
		}
		else
		{
			String named = data["output"].asString();
			//debugf("output %s", named.c_str());
			inversedVars[named] = Variable(this, DFNAME, named);
		}
	}
	else
		inversedVars[DFNAME] = Variable(this, DFNAME, DFNAME, false);
	if (data.containsKey("interval"))
		setRepeatingInterval(data["interval"].as<int>());

	if (data.containsKey("changed"))
		changedAction = data["changed"].asString();

	onLoad(data);
}

void Sensor::store(double value)
{
	store(DFNAME, value);
}

void Sensor::store(String outputName, double value)
{
	for (int i = 0; i < inversedVars.count(); i++)
	{
		auto key = inversedVars.keyAt(i);
		auto& varb = inversedVars[key];
		if (varb.variable == outputName)
		{
			if (varb.getValue() != value)
			{
				varb.setValue(value);
				if (varb.initialized)
				{
					changedFlag = true;
					debugf("updated %s.%s (%s) = %f", getName().c_str(),
							key.c_str(), outputName.c_str(), value);
				}
			}
			varb.initialized = true;
			return;
		}
	}
	debugf("CAN'T FIND VAR: %s::%s", getName().c_str(), outputName.c_str());
}

void Sensor::completeProcessing()
{
	onFinish();
	if (isChangedNow() && changedAction.length() > 0)
	{
		getParent()->executeJS(changedAction);
	}
	running = false;
}
