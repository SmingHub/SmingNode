#ifndef INCLUDE_SENSOR_H_
#define INCLUDE_SENSOR_H_

#include "BaseObject.h"

#define DFNAME "val"
class NodeCore;
class Sensor;

class Variable
{
public:
	Variable() {};
	Variable(Sensor* parent, String var, String nam, bool pub = true)
		: sensor(parent), variable(var), named(nam), publish(pub) {}

	double getValue() const;
	void setValue(double value = 0);

	Sensor* getSensor() const
	{
		return sensor;
	}
	String getFullName(const unsigned char separator = '.');

	String variable;
	String named;
	bool publish;
	bool initialized = false;

private:
	double value = 0;
	Sensor* sensor;
};

class Sensor : public WorkingObject
{
public:
	virtual void load(JsonObject& data);

	virtual bool execute()
	{
		if (running || !isEnabled()) return false;
		running = true;
		onBegin();
		if (sleep.getIntervalMs() > 0) // async wait
			sleep.startOnce();
		else
			completeProcessing();
		return true;
	}

	bool isRunning() { return running; }
	bool isChangedNow() { bool res = changedFlag; changedFlag = false; return res; }

	int getVariablesCount() { return inversedVars.count(); }
	Variable* getVariable(int id) { return &inversedVars[inversedVars.keyAt(id)]; }

protected:
	Sensor(int sensorReadingDuration)
	{
		setReadingDuration(sensorReadingDuration);
		//setRepeating(callRepeat);
		setRepeatingInterval(10*1000);
	}

	void setReadingDuration(int readingDuration)
	{
		sleep.initializeMs(readingDuration, TimerDelegate(&Sensor::completeProcessing, this));
	}

	virtual void onLoad(JsonObject& data) {};
	virtual void onBegin() = 0;
	virtual void onFinish() {}

	void completeProcessing();

	void store(double value);
	void store(String outputName, double value);

protected:
	HashMap<String, Variable> inversedVars;
	HashMap<String, int> pins;

private:
	Timer sleep;
	bool running = false;
	String changedAction;
	bool changedFlag = false;
};

#endif
