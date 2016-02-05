#ifndef INCLUDE_BASEOBJECT_H_
#define INCLUDE_BASEOBJECT_H_

#include <SmingCore/SmingCore.h>

class NodeCore;

class BaseObject
{
public:
	virtual void load(JsonObject& data) = 0;

	virtual void load(JsonVariant& data)
	{
		if (data.is<JsonObject&>())
			load((JsonObject&)data);
		else
			debugf("NOT SUPPORTED");
	}

	NodeCore* getParent() const
	{
		return parent;
	}
	void setParent(NodeCore* parent)
	{
		this->parent = parent;
	}

	const String& getName() const
	{
		return name;
	}
	void setName(const String& name)
	{
		this->name = name;
	}
	bool isEnabled()
	{
		return enabled;
	}
	void setEnabled(bool enabled)
	{
		this->enabled = enabled;
	}

private:
	NodeCore* parent;
	String name;
	bool enabled = true;
};

class WorkingObject : public BaseObject
{
public:
	WorkingObject()
	{
		repeater.setCallback(TimerDelegate(&WorkingObject::onRrepeat, this));
	}

	virtual void executeRepeating()
	{
		if (!isEnabled()) return;
		execute();
		//if (isRepeating())
			repeater.start();
	}

	virtual bool execute() = 0;

	bool getRepeatingInterval() { return repeater.getIntervalMs(); }
	void setRepeatingInterval(int interval)
	{
		repeater.setIntervalMs(interval);
	}
//	bool isRepeating() const
//	{
//		return repeating;
//	}
//	void setRepeating(bool repeatingInBackground)
//	{
//		this->repeating = repeatingInBackground;
//		if (!repeatingInBackground)
//			repeater.stop();
//	}
	void executeOnce()
	{
		BaseObject::setEnabled(true);
		repeater.startOnce();
	}
	void setEnabled(bool enabled)
	{
		BaseObject::setEnabled(enabled);
		if (enabled)
			repeater.start();
		else
			repeater.stop();
	}

	void onRrepeat() { execute(); }

private:
	Timer repeater;
	//bool repeating = true;
};

#endif /* INCLUDE_BASEOBJECT_H_ */
