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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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

#include <string_theory/string>

#include "plgDispatch.h"
#include "pyKey.h"
#include "hsResMgr.h"

#include "pnModifier/plModifier.h"

#include "plPythonFileMod.h"
#include "pnMessage/plEnableMsg.h"
#include "pySceneObject.h"
#include "pnSceneObject/plSceneObject.h"

pyKey::pyKey()
{
#ifndef BUILDING_PYPLASMA
    fPyFileMod = nullptr;
    fNetForce=false;
#endif
}

pyKey::pyKey(plKey key)
{
    fKey = std::move(key);
#ifndef BUILDING_PYPLASMA
    fPyFileMod = nullptr;
    fNetForce=false;
#endif
}

#ifndef BUILDING_PYPLASMA
pyKey::pyKey(plKey key, plPythonFileMod* pymod)
{
    fKey = std::move(key);
    fPyFileMod=pymod;
    fNetForce=false;
}
#endif

bool pyKey::operator==(const pyKey &key) const
{
    plKey ours = ((pyKey*)this)->getKey();
    plKey theirs = ((pyKey&)key).getKey();
    if (ours == nullptr && theirs == nullptr)
        return true;
    else if (ours != nullptr && theirs != nullptr)
        return (ours->GetUoid()==theirs->GetUoid());
    else
        return false;
}

ST::string pyKey::getName() const
{
    return fKey ? fKey->GetName() : "nil";
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
        else
        {
            return nullptr;
        }
    }
    // create pySceneObject that will be managed by Python
    return pySceneObject::New(getKey());
}

// send disable message to the plKey
void pyKey::IEnable(bool state)
{
    // create message
    plEnableMsg* pMsg = new plEnableMsg;
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
    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
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
    return nullptr;
}


// special functions when the plKey is a pointer to the PythonModifier
//
// Was the last plNotifyMsg a locally sent?
bool pyKey::WasLocalNotify()
{
    // see if we have a PythonFileModifier pointer
    if ( fPyFileMod )
        return fPyFileMod->WasLocalNotify();
    // otherwise... just say it is local
    return true;
}

// Is python file mod attached to clone
bool pyKey::IsAttachedToClone()
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
    return nullptr;
}

// get the notify list count
size_t pyKey::NotifyListCount() const
{
    // see if we have a PythonFileModifier pointer
    if ( fPyFileMod )
        return fPyFileMod->NotifyListCount();
    // otherwise... just say notify list receivers
    return 0;
}

// get a notify list item
plKey pyKey::GetNotifyListItem(size_t i) const
{
    // see if we have a PythonFileModifier pointer
    if ( fPyFileMod )
        return fPyFileMod->GetNotifyListItem(i);
    // otherwise... just say it is local
    return nullptr;
}


// Set the dirty state on the PythonModifier
void pyKey::DirtySynchState(const ST::string& SDLStateName, uint32_t sendFlags)
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
