/*
 * ScriptCore.h
 *
 *  Created on: 26 џэт. 2016 у.
 *      Author: Anakod
 */

#ifndef INCLUDE_SCRIPTCORE_H_
#define INCLUDE_SCRIPTCORE_H_

#include "TinyJS.h"
//#include "TinyJS_Functions.h"
#include "TinyJS_MathFunctions.h"

class ScriptCore : public CTinyJS
{
public:
	typedef Delegate<void()> DelegateV;
	typedef Delegate<void(bool)> DelegateSetB;
	typedef Delegate<bool()> DelegateGetB;

	ScriptCore()
	{
		//registerFunctions(this);
		registerMathFunctions(this);
	}

	// VOID //
public:
	void addHandler(String objName, String mtdName, DelegateV t)
	{
		vVoids.addElement(t);
		addNative("function "+objName+"."+mtdName+"()", &ScriptCore::voidHandler, (void*)&vVoids.lastElement());
	}

private:
	static void voidHandler(CScriptVar *v, void *userdata)
	{
		auto* t = (DelegateV*)userdata;
		(*t)();
	}

	// BOOL //
public:
	void addHandler(String objName, String mtdName, const DelegateGetB& t)
	{
		vGBools.addElement(t);
		addNative("function "+objName+"."+mtdName+"()", &ScriptCore::boolRetHandler, (void*)&vGBools.lastElement());
	}
	void addHandler(String objName, String mtdName, const DelegateSetB& t)
	{
		vSBools.addElement(t);
		addNative("function "+objName+"."+mtdName+"(arg1)", &ScriptCore::bool1Handler, (void*)&vSBools.lastElement());
	}
private:
	static void bool1Handler(CScriptVar *v, void *userdata)
	{
		bool a1 =  v->getParameter("arg1")->getBool();
		auto* t = (DelegateSetB*)userdata;
		(*t)(a1);
	}
	static void boolRetHandler(CScriptVar *v, void *userdata)
	{
		auto* t = (DelegateGetB*)userdata;
		auto result = (*t)();
		v->getReturnVar()->setInt((int)result);
	}

private:
	Vector<DelegateV> vVoids;
	Vector<DelegateGetB> vGBools;
	Vector<DelegateSetB> vSBools;
};


#endif /* INCLUDE_SCRIPTCORE_H_ */
