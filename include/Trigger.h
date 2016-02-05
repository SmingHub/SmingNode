/*
 * Trigger.h
 *
 *  Created on: 24 џэт. 2016 у.
 *      Author: Anakod
 */

#ifndef INCLUDE_TRIGGER_H_
#define INCLUDE_TRIGGER_H_

#include "BaseObject.h"

class Trigger : public WorkingObject
{
public:
	Trigger()
	{
		setRepeatingInterval(5*1000);
	}
	virtual bool execute() = 0;
};

class ExpressionTrigger : public Trigger
{
public:
	virtual void load(JsonObject& data)
	{
		action = data["action"].asString();

		if (data.containsKey("interval"))
			setRepeatingInterval(data["interval"].as<int>());

		debugf("expr. action => %s", action.c_str());
	}
	virtual void load(JsonVariant& data)
	{
		if (data.is<JsonObject&>())
			load((JsonObject&)data);
		else
			action = data.asString();
	}

	virtual bool execute()
	{
		if (!isEnabled()) return false;
		getParent()->executeJS(action);
	}
private:
	String action;
};


#endif /* INCLUDE_TRIGGER_H_ */
