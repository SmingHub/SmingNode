/*
 * SpecificSensors.h
 *
 *  Created on: 28 џэт. 2016 у.
 *      Author: Anakod
 */

#ifndef SPECIFICSENSORS_H_
#define SPECIFICSENSORS_H_

#include "Sensor.h"

class DHT;
class DS18S20;

class SensorDHT : public Sensor
{
public:
	SensorDHT(String type);
	~SensorDHT();

	virtual void onLoad(JsonObject& data);
	virtual void onBegin();

private:
	DHT* dht = nullptr;
	int type;
	bool initialized = false;
};

class SensorAnalogReader : public Sensor
{
public:
	SensorAnalogReader() : Sensor(0) {}
	virtual void onBegin();
};

class SensorButton : public Sensor
{
public:
	SensorButton() : Sensor(20) { setRepeatingInterval(60); }
	virtual void onLoad(JsonObject& data);
	virtual void onBegin()
	{
//		int pin = pins[DFNAME];
//		state = digitalRead(pin);
	}
	virtual void onFinish();
};

class SensorDS1820 : public Sensor
{
public:
	SensorDS1820() : Sensor(700) {}
	~SensorDS1820();

	virtual void onLoad(JsonObject& data);
	virtual void onBegin();
	virtual void onFinish();

private:
	DS18S20 *ds;
};

#endif /* SPECIFICSENSORS_H_ */
