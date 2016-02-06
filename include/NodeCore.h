/*
 * AppSettings.h
 *
 *  Created on: 13 мая 2015 г.
 *      Author: Anakod
 */

#include <SmingCore/SmingCore.h>


#ifndef INCLUDE_CFGREADER_H_
#define INCLUDE_CFGREADER_H_

class Trigger;
class Sensor;
class TaskTimer;
class Variable;
class Slave;
class ScriptCore;
class MqttClient;

#define APP_SETTINGS_FILE ".settings.conf" // leading point for security reasons :)

class NodeCore
{
public:
	String name;
	String ssid;
	String password;
	String mqttPrefix;

	NodeCore();
	bool load();

	void save();

	virtual Sensor* createSensor(String type, JsonObject& data);

	void executeJS(String cmd);
	double evaluateJS(const String& cmd);
	void updateVariable(Variable* var, double value);

	bool exist() { return fileExist(APP_SETTINGS_FILE); }

	void startNetwork();
	void collectSensors(String& result);

protected:
	void readConfigTree(JsonObject& root);
	void onMqttMessageReceived(String topic, String message);

protected:
	Vector<Sensor*> sensors;
	Vector<Trigger*> triggers;
	Vector<TaskTimer*> timers;
	HashMap<String, Slave*> pins;

private:
	ScriptCore *js;
	MqttClient *mqtt = nullptr;
};

static NodeCore Core;

#endif /* INCLUDE_APPSETTINGS_H_ */
