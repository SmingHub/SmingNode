/*
 * Slave.h
 *
 *  Created on: 31 џэт. 2016 у.
 *      Author: Anakod
 */

#ifndef INCLUDE_SLAVE_H_
#define INCLUDE_SLAVE_H_

#include "BaseObject.h"

class Slave : public BaseObject
{
public:
	void write(bool state) { debugf("%s -> %d", getName().c_str(), state); digitalWrite(pin, state); }
	bool read() { return digitalRead(pin); }
	bool outputState() { return GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1<<pin); }

	virtual void load(JsonObject& data)
	{
		pin = data["pins"].as<int>();
		pinMode(pin, OUTPUT);
		debugf("%s on %d", getName().c_str(), pin);
	}
	virtual void load(JsonVariant& data)
	{
		if (data.is<JsonObject&>())
			load((JsonObject&)data);
		else
		{
			pin = data.as<int>();
			pinMode(pin, OUTPUT);
			debugf("%s on %d", getName().c_str(), pin);
		}
	}

private:
	int pin = 0;
};



#endif /* INCLUDE_SLAVE_H_ */
