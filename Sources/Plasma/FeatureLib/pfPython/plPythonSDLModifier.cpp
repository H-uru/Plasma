/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011  Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsConfig.h"
#include "plPythonSDLModifier.h"
#include "cyPythonInterface.h"

#include "plPythonFileMod.h"
#include "pyKey.h"
#include "cyMisc.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plAgeDescription/plAgeDescription.h"

#include "../plSDL/plSDL.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"

plStateDataRecord * GetAgeSDL()
{
	const plPythonSDLModifier * mod = plPythonSDLModifier::FindAgeSDL();
	if ( mod )
		return mod->GetStateCache();
	return nil;
}


#define PyLoggedAssert(cond, text)					\
	if (!cond) PythonInterface::WriteToLog(text);	\
	hsAssert(cond, text);

plPythonSDLModifier::plPythonSDLModifier(plPythonFileMod* owner) : fOwner(owner)
{
	plStateDescriptor* desc = plSDLMgr::GetInstance()->FindDescriptor(fOwner->fPythonFile, plSDL::kLatestVersion);
	if (desc)
	{
		// Create a Python SDL record with no values set
		int numVars = desc->GetNumVars();
		for (int i = 0; i < numVars; i++)
		{
			plVarDescriptor* var = desc->GetVar(i);

			const char* name = var->GetName();
			int count = var->GetCount();

			fMap[name] = SDLObj(nil, count, false);
		}
	}
}

plPythonSDLModifier::~plPythonSDLModifier()
{
	SDLMap::iterator it;
	for (it = fMap.begin(); it != fMap.end(); it++)
	{
		PyObject* obj = it->second.obj;
		Py_XDECREF(obj);
	}
}

PyObject* plPythonSDLModifier::GetItem(const char* key)
{
	SDLMap::iterator it = fMap.find(key);

	if (it == fMap.end())
	{
		char errmsg[256];
		sprintf(errmsg,"SDL key %s not found",key);
		PyErr_SetString(PyExc_KeyError, errmsg);
		PYTHON_RETURN_ERROR;
	}

	PyObject* val = it->second.obj;
	if (!val)
		val = PyTuple_New(0);

	Py_INCREF(val);
	return val;
}

void plPythonSDLModifier::ISetItem(const char* key, PyObject* value)
{
	if (!value || !PyTuple_Check(value))
	{
		PyLoggedAssert(0, "[SDL] Trying to set a tuple value to something that isn't a tuple");
		return;
	}

	SDLMap::iterator it = fMap.find(key);

	if (it == fMap.end())
	{
		PyLoggedAssert(0, "[SDL] Tried to set a nonexistent SDL value");
		return;
	}

	SDLObj& oldObj = it->second;

	if (oldObj.size != 0 && PyTuple_Size(value) != oldObj.size)
	{
		PyLoggedAssert(0, "[SDL] Wrong size for a fixed size SDL value");
		return;
	}

	Py_XDECREF(oldObj.obj);

	Py_XINCREF(value);
	oldObj.obj = value;
}

void plPythonSDLModifier::SendToClients(const char* key)
{
	SDLMap::iterator it = fMap.find(key);
	if (it != fMap.end())
		it->second.sendToClients = true;
}

void plPythonSDLModifier::SetNotify(pyKey& selfkey, const char* key, float tolerance)
{
	AddNotifyForVar(selfkey.getKey(), key, tolerance);
}

void plPythonSDLModifier::SetItem(const char* key, PyObject* value)
{
	ISetItem(key, value);
	IDirtySynchState(key);
}

void plPythonSDLModifier::SetItemFromSDLVar(plSimpleStateVariable* var)
{
	const char* name = var->GetName();

	// Get the SDL value in Python format
	PyObject* pyVar = ISDLVarToPython(var);

	ISetItem(name, pyVar);
	Py_XDECREF(pyVar);
	// let the sender do the dirty sync state stuff
}


void plPythonSDLModifier::SetDefault(const char* key, PyObject* value)
{
	ISetItem(key, value);
}

void plPythonSDLModifier::SetItemIdx(const char* key, int idx, PyObject* value, hsBool sendImmediate)
{
	if (!value)
	{
		PyLoggedAssert(0, "[SDL] Trying to set a value to nil");
		return;
	}

	SDLMap::iterator it = fMap.find(key);

	if (it == fMap.end())
	{
		PyLoggedAssert(0, "[SDL] Tried to set a nonexistent SDL value");
		return;
	}

	PyObject* pyTuple = it->second.obj;
	int size = it->second.size;

	if (size != 0 && idx >= size)
	{
		PyLoggedAssert(0, "[SDL] Trying to resize a SDL value that can't be");
		return;
	}

	if (pyTuple)
	{
		if (PyTuple_Size(pyTuple) <= idx)
		{
			int oldsize = PyTuple_Size(pyTuple);
			_PyTuple_Resize(&pyTuple, idx+1);
			// initialize the tuple elements to None, because Python don't like NULLs
			int j;
			for ( j=oldsize; j<idx+1; j++ )
			{
				Py_INCREF(Py_None);
				PyTuple_SetItem(pyTuple, j, Py_None);
			}
		}
	}
	else
	{
		int newSize = (size == 0) ? idx+1 : size;
		pyTuple = PyTuple_New(newSize);
		// initialize the tuple elements to None, because Python don't like NULLs
		int j;
		for ( j=0; j<newSize; j++ )
		{
			Py_INCREF(Py_None);
			PyTuple_SetItem(pyTuple, j, Py_None);
		}
		it->second.obj = pyTuple;
	}

	Py_XINCREF(value);	// PyTuple_SetItem doesn't increment the ref count
	PyTuple_SetItem(pyTuple, idx, value);

	IDirtySynchState(key, sendImmediate);
}


const char* plPythonSDLModifier::GetSDLName() const
{
	return fOwner->fPythonFile;
}

void plPythonSDLModifier::SetFlags(const char* name, bool sendImmediate, bool skipOwnershipCheck)
{
	SDLMap::iterator it = fMap.find(name);
	if (it != fMap.end())
	{
		it->second.sendImmediate = sendImmediate;
		it->second.skipLocalCheck = skipOwnershipCheck;
	}
}

void plPythonSDLModifier::SetTagString(const char* name, const char* tag)
{
	SDLMap::iterator it = fMap.find(name);
	if (it != fMap.end())
	{
		it->second.hintString = tag;
	}
}

void plPythonSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
	plStateDataRecord::SimpleVarsList vars;
	int num = srcState->GetUsedVars(&vars);

	for (int i = 0; i < num; i++)
	{
		plSimpleStateVariable* var = vars[i];

		const char* name = var->GetName();

		// Get the SDL value in Python format
		PyObject* pyVar = ISDLVarToPython(var);

		SetItem(name, pyVar);
		Py_XDECREF(pyVar);
	}

	// Notify the Python code that we updated the SDL record
	if (fOwner->fPyFunctionInstances[plPythonFileMod::kfunc_Load] != nil)
	{
		PyObject* retVal = PyObject_CallMethod(fOwner->fPyFunctionInstances[plPythonFileMod::kfunc_Load],
					fOwner->fFunctionNames[plPythonFileMod::kfunc_Load], nil);
		if (retVal == nil)
		{
#ifndef PLASMA_EXTERNAL_RELEASE
			// for some reason this function didn't, remember that and not call it again
			fOwner->fPyFunctionInstances[plPythonFileMod::kfunc_Load] = nil;
#endif //PLASMA_EXTERNAL_RELEASE
			// if there was an error make sure that the stderr gets flushed so it can be seen
			fOwner->ReportError();
		}
		Py_XDECREF(retVal);
	}

	// display any output
	fOwner->DisplayPythonOutput();
}

void plPythonSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
	SDLMap::iterator it = fMap.begin();
	for (; it != fMap.end(); it++)
	{
		IPythonVarToSDL(dstState, it->first.c_str());
	}
}

void plPythonSDLModifier::IDirtySynchState(const char* name, hsBool sendImmediate)
{
	SDLMap::iterator it = fMap.find(name);
	if (it != fMap.end())
	{
		UInt32 flags = 0;
		if (it->second.sendToClients)
			flags |= kBCastToClients;
		if (it->second.sendImmediate)
			flags |= kSendImmediately;
		if (it->second.skipLocalCheck)
			flags |= kSkipLocalOwnershipCheck;
		if (sendImmediate)
			flags |= kSendImmediately;

		GetTarget()->DirtySynchState(fOwner->fPythonFile, flags);
	}
}

bool plPythonSDLModifier::IPythonVarIdxToSDL(plSimpleStateVariable* var, int varIdx, int type, PyObject* pyVar, 
											 const char* hintstring)
{
	switch (type)
	{
	case plVarDescriptor::kShort:
	case plVarDescriptor::kByte:
	case plVarDescriptor::kBool:
	case plVarDescriptor::kInt:
		if (PyInt_Check(pyVar))
		{
			int v = PyInt_AsLong(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		else if (PyLong_Check(pyVar))
		{
			int v = (int)PyLong_AsLong(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}

		else if (PyFloat_Check(pyVar))
		{
			int v = (int)PyFloat_AsDouble(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		break;

	case plVarDescriptor::kFloat:
		if (PyFloat_Check(pyVar))
		{
			float v = (float)PyFloat_AsDouble(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		// does python think that its an integer? too bad, we'll make it a float anyhow!
		else if (PyInt_Check(pyVar))
		{
			float v = (float)PyInt_AsLong(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		break;

	case plVarDescriptor::kString32:
		if (PyString_Check(pyVar))
		{
			char* v = PyString_AsString(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
		}
		break;

	case plVarDescriptor::kKey:
		{
			pyKey* key = PythonInterface::GetpyKeyFromPython(pyVar);
			if ( key )
				var->Set(key->getKey(),varIdx);
				if (hintstring)
					var->GetNotificationInfo().SetHintString(hintstring);
		}
		break;

	case plVarDescriptor::kDouble:
		if (PyFloat_Check(pyVar))
		{
			double v = PyFloat_AsDouble(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		else if (PyInt_Check(pyVar))
		{
			double v = (double)PyInt_AsLong(pyVar);
			var->Set(v, varIdx);
			if (hintstring)
				var->GetNotificationInfo().SetHintString(hintstring);
			return true;
		}
		break;

	case plVarDescriptor::kAgeTimeOfDay:
		break;

	default:
		hsAssert(0, "Not supported yet");
	}

	return false;
}

void plPythonSDLModifier::IPythonVarToSDL(plStateDataRecord* state, const char* name)
{
	plSimpleStateVariable* var = state->FindVar(name);
	PyObject* pyVar = nil;
	SDLMap::iterator it = fMap.find(name);
	if (it != fMap.end())
		pyVar = it->second.obj;

	if (!var || !pyVar)
		return;

	if (PyTuple_Check(pyVar))
	{
		int count = PyTuple_Size(pyVar);
		plSimpleVarDescriptor::Type type = var->GetSimpleVarDescriptor()->GetType();

		for (int i = 0; i < count; i++)
		{
			PyObject* pyVarItem = PyTuple_GetItem(pyVar, i);
			if (pyVarItem)
				IPythonVarIdxToSDL(var, i, type, pyVarItem,it->second.hintString.c_str());
		}
	}
}

PyObject* plPythonSDLModifier::ISDLVarIdxToPython(plSimpleStateVariable* var, int type, int idx)
{
	switch (type)
	{
	case plVarDescriptor::kShort:
	case plVarDescriptor::kByte:
	case plVarDescriptor::kInt:
		{
			int v;
			var->Get(&v, idx);
			return PyInt_FromLong(v);
		}

	case plVarDescriptor::kFloat:
	case plVarDescriptor::kAgeTimeOfDay:
		{
			float v;
			var->Get(&v, idx);
			return PyFloat_FromDouble(v);
		}

	case plVarDescriptor::kBool:
		{
			bool v;
			var->Get(&v, idx);
			return PyLong_FromLong(v);
		}

	case plVarDescriptor::kString32:
		{
			char v[256];
			var->Get(v, idx);
			return PyString_FromString(v);
		}

	case plVarDescriptor::kKey:
		{
			plKey v;
			var->Get(&v, idx);
			PyObject* keyObj = pyKey::New(v);
			return keyObj;
		}

	case plVarDescriptor::kDouble:
		{
			double v;
			var->Get(&v, idx);
			return PyFloat_FromDouble(v);
		}

//	case plVarDescriptor::kStateDescriptor:
//	case plVarDescriptor::kCreatable:
//	case plVarDescriptor::kVector3:
//	case plVarDescriptor::kPoint3:
//	case plVarDescriptor::kRGB:
//	case plVarDescriptor::kRGBA:
//	case plVarDescriptor::kQuaternion:
	default:
		hsAssert(0, "Not supported yet");
	}

	PYTHON_RETURN_NONE;
}

PyObject* plPythonSDLModifier::ISDLVarToPython(plSimpleStateVariable* var)
{
	plSimpleVarDescriptor::Type type = var->GetSimpleVarDescriptor()->GetType();
	int count = var->GetCount();

	PyObject* pyTuple = PyTuple_New(count);

	for (int i = 0; i < count; i++)
	{
		PyObject* varVal = ISDLVarIdxToPython(var, type, i);
		PyTuple_SetItem(pyTuple, i, varVal);
	}

	return pyTuple;
}

bool plPythonSDLModifier::HasSDL(const char* pythonFile)
{
	return (plSDLMgr::GetInstance()->FindDescriptor(pythonFile, plSDL::kLatestVersion) != nil);
}

const plSDLModifier *ExternFindAgeSDL()
{
	return plPythonSDLModifier::FindAgeSDL();
}

const plPythonSDLModifier *ExternFindAgePySDL()
{
	return plPythonSDLModifier::FindAgeSDL();
}

const plPythonSDLModifier* plPythonSDLModifier::FindAgeSDL()
{
	const char* ageName = cyMisc::GetAgeName();

	if (strcmp(ageName, "") == 0)
		return nil; // don't have an age, probably because we're running in max?

	// find the Age Global object
	plLocation loc = plKeyFinder::Instance().FindLocation(ageName,plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
	if ( loc.IsValid() )
	{
		plUoid oid(loc,plPythonFileMod::Index(), plPythonFileMod::kGlobalNameKonstant);
		if ( oid.IsValid() )
		{
			plKey key = hsgResMgr::ResMgr()->FindKey(oid);

			plPythonFileMod *pfmod = plPythonFileMod::ConvertNoRef(key ? key->ObjectIsLoaded() : nil);
			if ( pfmod )
			{
				plPythonSDLModifier * sdlMod = pfmod->GetSDLMod();
				if(sdlMod)
					// we found it!
					return sdlMod;
				
				plNetClientApp::StaticErrorMsg("pfmod %s has a nil python SDL modifier for age sdl %s", 
					pfmod->GetKeyName() ? pfmod->GetKeyName() : "?", ageName);				
			}
			else
			{
				char str[256];
				if (!key)
					plNetClientApp::StaticErrorMsg("nil key %s for age sdl %s", ageName, oid.StringIze(str));
				else
				if (!key->ObjectIsLoaded())
					plNetClientApp::StaticErrorMsg("key %s not loaded for age sdl %s", 
						key->GetName() ? key->GetName() : "?", ageName);
				else
				if (!plPythonFileMod::ConvertNoRef(key->ObjectIsLoaded()))
					plNetClientApp::StaticErrorMsg("key %s is not a python file mod for age sdl %s", 
						key->GetName() ? key->GetName() : "?", ageName);
			}
		}
		else
			plNetClientApp::StaticErrorMsg("Invalid plUoid for age sdl %s", ageName);
	}
	else
		plNetClientApp::StaticErrorMsg("Invalid plLocation for age sdl %s", ageName);

	// couldn't find one (maybe because we didn't look)
	return nil;
}

plKey ExternFindAgeSDLTarget()
{
	return plPythonSDLModifier::FindAgeSDLTarget();
}

plKey plPythonSDLModifier::FindAgeSDLTarget()
{

	// find the Age Global object
	plLocation loc = plKeyFinder::Instance().FindLocation(cyMisc::GetAgeName(),plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
	if ( loc.IsValid() )
	{
		plUoid oid(loc,plPythonFileMod::Index(), plPythonFileMod::kGlobalNameKonstant);
		if ( oid.IsValid() )
		{
			plKey key = hsgResMgr::ResMgr()->FindKey(oid);

			plPythonFileMod *pfmod = plPythonFileMod::ConvertNoRef(key ? key->GetObjectPtr() : nil);
			if ( pfmod )
			{
				if (pfmod->GetTarget(0))
					return pfmod->GetTarget(0)->GetKey();
			}
		}
	}

	// couldn't find one (maybe because we didn't look)
	return nil;
}


/////////////////////////////////////////////

pySDLModifier::pySDLModifier(plPythonSDLModifier* sdlMod)
{
	fRecord = sdlMod;
}


PyObject* pySDLModifier::GetAgeSDL()
{
	const char* ageName = cyMisc::GetAgeName();
	if (strcmp(ageName, "") == 0)
		PYTHON_RETURN_NONE; // just return none if the age is blank (running in max?)

	const plPythonSDLModifier* ageSDL = plPythonSDLModifier::FindAgeSDL();
	if ( ageSDL )
	{
		return pySDLModifier::New((plPythonSDLModifier*)ageSDL);
	}

	// didn't find one, throw an exception for the python programmer to chew on
	char errmsg[256];
	sprintf(errmsg,"Age Global SDL for %s does not exist!",ageName);
	plNetClientApp::StaticErrorMsg(errmsg);
	PyErr_SetString(PyExc_KeyError, errmsg);
	PYTHON_RETURN_ERROR;
}


void pySDLModifier::SetDefault(pySDLModifier& self, std::string key, PyObject* value)
{
	self.fRecord->SetDefault(key.c_str(), value);
}

void pySDLModifier::SendToClients(pySDLModifier& self, std::string key)
{
	self.fRecord->SendToClients(key.c_str());
}

void pySDLModifier::SetNotify(pySDLModifier& self, pyKey& selfkey, std::string key, float tolerance)
{
	self.fRecord->SetNotify(selfkey, key.c_str(), tolerance);
}

PyObject* pySDLModifier::GetItem(pySDLModifier& self, std::string key)
{
	return self.fRecord->GetItem(key.c_str());
}

void pySDLModifier::SetItem(pySDLModifier& self, std::string key, PyObject* value)
{
	self.fRecord->SetItem(key.c_str(), value);
}


void pySDLModifier::SetItemIdx(pySDLModifier& self, std::string key, int idx, PyObject* value)
{
	self.fRecord->SetItemIdx(key.c_str(), idx, value);
}

void pySDLModifier::SetItemIdxImmediate(pySDLModifier& self, std::string key, int idx, PyObject* value)
{
	self.fRecord->SetItemIdx(key.c_str(), idx, value, true);
}

void pySDLModifier::SetFlags(pySDLModifier& self, const char* name, bool sendImmediate, bool skipOwnershipCheck)
{
	self.fRecord->SetFlags(name,sendImmediate,skipOwnershipCheck);
}

void pySDLModifier::SetTagString(pySDLModifier& self, const char* name, const char* tag)
{
	self.fRecord->SetTagString(name,tag);
}
