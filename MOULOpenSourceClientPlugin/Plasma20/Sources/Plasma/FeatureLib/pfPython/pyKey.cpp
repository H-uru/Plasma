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
//////////////////////////////////////////////////////////////////////
//
// pyKey   - the wrapper class around a plKey so that Python can handle it
//
//////////////////////////////////////////////////////////////////////

#include "pyKey.h"
#include "../pnKeyedObject/plKey.h"
#include "plgDispatch.h"
#include "plPythonFileMod.h"
#include "../pnMessage/plEnableMsg.h"
#include "hsResMgr.h"
#include "pySceneObject.h"
#include "../pnSceneObject/plSceneObject.h"

pyKey::pyKey()
{
	fKey=nil;
#ifndef BUILDING_PYPLASMA
	fPyFileMod=nil;
	fNetForce=false;
#endif
}

pyKey::pyKey(plKey key)
{
	fKey = key;
#ifndef BUILDING_PYPLASMA
	fPyFileMod=nil;
	fNetForce=false;
#endif
}

#ifndef BUILDING_PYPLASMA
pyKey::pyKey(plKey key, plPythonFileMod* pymod)
{
	fKey = key;
	fPyFileMod=pymod;
	fNetForce=false;
}

void pyKey::SetNetForce(hsBool state)
{
	// set our flag
	fNetForce = state;
}

// send enable message to the plKey
void pyKey::Enable()
{
	IEnable(true);
}

	
// send disable message to the plKey
void pyKey::Disable()
{
	IEnable(false);
}
#endif

hsBool pyKey::operator==(const pyKey &key) const
{
	plKey ours = ((pyKey*)this)->getKey();
	plKey theirs = ((pyKey&)key).getKey();
	if ( ours == nil && theirs == nil )
		return true;
	else if ( ours != nil && theirs != nil )
		return (ours->GetUoid()==theirs->GetUoid());
	else
		return false;
}

#ifndef BUILDING_PYPLASMA
PyObject* pyKey::GetPySceneObject()
{
	plKey theKey = getKey();
	//if a modifier return the scene objects key
	plModifier* mod = plModifier::ConvertNoRef(theKey->ObjectIsLoaded());
	if (mod)
	{
		if(mod->GetNumTargets()>0)
		{
			return pySceneObject::New(mod->GetTarget(0)->GetKey());
		}
		else return nil;
	}
	// create pySceneObject that will be managed by Python
	return pySceneObject::New(getKey());
}

// send disable message to the plKey
void pyKey::IEnable(hsBool state)
{
	// create message
	plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
	if (fNetForce )
	{
		// set the network propagate flag to make sure it gets to the other clients
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		pMsg->SetBCastFlag(plMessage::kNetForce);
	}
	pMsg->AddReceiver(fKey);
	// which way are we doin' it?
	if ( state )
		pMsg->SetCmd(plEnableMsg::kEnable);
	else
		pMsg->SetCmd(plEnableMsg::kDisable);
	plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
}


// if this is a modifier then get the (first) object its attached to
PyObject* pyKey::GetParentObject()
{
	if (fKey)
	{
		// see if this a modifier that is loaded
		plModifier* mod = plModifier::ConvertNoRef(fKey->ObjectIsLoaded());
		if (mod)
		{
			if ( mod->GetNumTargets() > 0 )
				return pyKey::New(mod->GetTarget(0)->GetKey());
		}
	}
	return nil;
}


// special functions when the plKey is a pointer to the PythonModifier
//
// Was the last plNotifyMsg a locally sent?
hsBool pyKey::WasLocalNotify()
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		return fPyFileMod->WasLocalNotify();
	// otherwise... just say it is local
	return true;
}

// Is python file mod attached to clone
hsBool pyKey::IsAttachedToClone()
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		return fPyFileMod->AmIAttachedToClone();
	// otherwise return nope
	return false;
}

plPipeline* pyKey::GetPipeline()
{
	if ( fPyFileMod )
		return fPyFileMod->GetPipeline();
	return nil;
}

// get the notify list count
Int32 pyKey::NotifyListCount()
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		return fPyFileMod->NotifyListCount();
	// otherwise... just say notify list receivers
	return 0;
}

// get a notify list item
plKey pyKey::GetNotifyListItem(Int32 i)
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		return fPyFileMod->GetNotifyListItem(i);
	// otherwise... just say it is local
	return nil;
}


// Set the dirty state on the PythonModifier
void pyKey::DirtySynchState(const char* SDLStateName, UInt32 sendFlags)
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		fPyFileMod->DirtySynchState(SDLStateName, sendFlags);
}


// register for control key events with the pythonfile modifier
void pyKey::EnableControlKeyEvents()
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		// if so, then pass on the command request
		fPyFileMod->EnableControlKeyEvents();
}

// unregister for control key events with the pythonfile modifier
void pyKey::DisableControlKeyEvents()
{
	// see if we have a PythonFileModifier pointer
	if ( fPyFileMod )
		// if so, then pass on the command request
		fPyFileMod->DisableControlKeyEvents();
}
#endif // BUILDING_PYPLASMA