/*
 * TaskTimer.h
 *
 *  Created on: 27 џэт. 2016 у.
 *      Author: Anakod
 */

#ifndef INCLUDE_TASKTIMER_H_
#define INCLUDE_TASKTIMER_H_

class TaskTimer : public WorkingObject
{
public:
	TaskTimer()
	{
		setRepeatingInterval(60*1000);
	}
	virtual void load(JsonObject& data)
	{
		if (data.containsKey("enabled"))
			setEnabled(data["enabled"].as<bool>());
		action = data["action"].asString();

		if (data.containsKey("interval"))
			setRepeatingInterval(data["interval"].as<int>());

		//setRepeating(isEnabled());

		debugf("tmr: %s", action.c_str());
	}
	virtual bool execute()
	{
		if (!isEnabled()) return false;
		debugf("tmr run: %s", action.c_str());
		getParent()->executeJS(action);
		return true;
	}

	void start()
	{
		debugf("%s.start", getName().c_str());
		setEnabled(true);
	}
	void startOnce()
	{
		debugf("%s.startOnce", getName().c_str());
		executeOnce();
	}

private:
	String action;
};




#endif /* INCLUDE_TASKTIMER_H_ */
