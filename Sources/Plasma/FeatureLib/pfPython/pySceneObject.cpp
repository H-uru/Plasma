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

#include "pySceneObject.h"

#include <string_theory/format>

#include "plAudible.h"
#include "plgDispatch.h"

#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnSceneObject/plAudioInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plModifier/plLogicModifier.h"
#include "plModifier/plResponderModifier.h"

#include "pfCamera/plCameraModifier.h"

#include "cyAvatar.h"
#include "cyDraw.h"
#include "cyParticleSys.h"
#include "cyPhysics.h"
#include "plPythonFileMod.h"
#include "pyGeometry3.h"
#include "pyGlueHelpers.h"
#include "pyMatrix44.h"
#include "plModifier/plResponderModifier.h"
#include "plModifier/plImageLibMod.h"

void pySceneObject::IAddObjKeyToAll(const plKey& key)
{
    // set the sender and the receiver to the same thing
    cyDraw::ConvertFrom(fDraw)->AddRecvr(key);
    cyPhysics::ConvertFrom(fPhysics)->AddRecvr(key);
    cyAvatar::ConvertFrom(fAvatar)->AddRecvr(key);
    cyParticleSys::ConvertFrom(fParticle)->AddRecvr(key);
}

void pySceneObject::ISetAllSenderKeys()
{
    // set the sender and the receiver to the same thing
    cyDraw::ConvertFrom(fDraw)->SetSender(fSenderKey);
    cyPhysics::ConvertFrom(fPhysics)->SetSender(fSenderKey);
    cyAvatar::ConvertFrom(fAvatar)->SetSender(fSenderKey);
    cyParticleSys::ConvertFrom(fParticle)->SetSender(fSenderKey);
}

pySceneObject::pySceneObject()
{
    // make sure these are created
    fDraw = cyDraw::New();
    fPhysics = cyPhysics::New();
    fAvatar = cyAvatar::New();
    fParticle = cyParticleSys::New();
    fNetForce = false;
}

pySceneObject::pySceneObject(const pyKey& objkey, const pyKey& selfkey)
{
    // make sure these are created
    fDraw = cyDraw::New();
    fPhysics = cyPhysics::New();
    fAvatar = cyAvatar::New();
    fParticle = cyParticleSys::New();

    addObjKey(objkey.getKey());
    setSenderKey(selfkey.getKey());
    setPyMod(selfkey);
    fNetForce = false;
}

pySceneObject::pySceneObject(const plKey& objkey, const pyKey& selfkey)
{
    // make sure these are created
    fDraw = cyDraw::New();
    fPhysics = cyPhysics::New();
    fAvatar = cyAvatar::New();
    fParticle = cyParticleSys::New();

    addObjKey(objkey);
    setSenderKey(selfkey.getKey());
    setPyMod(selfkey);
    fNetForce = false;
}


pySceneObject::pySceneObject(const plKey& objkey)
{
    // make sure these are created
    fDraw = cyDraw::New();
    fPhysics = cyPhysics::New();
    fAvatar = cyAvatar::New();
    fParticle = cyParticleSys::New();

    addObjKey(objkey);
    setSenderKey(objkey);
    fNetForce = false;
}

pySceneObject::~pySceneObject()
{
    Py_XDECREF(fDraw);
    Py_XDECREF(fPhysics);
    Py_XDECREF(fAvatar);
    Py_XDECREF(fParticle);
}


bool pySceneObject::operator==(const pySceneObject &sobj) const
{
    plKey ours = getObjKey();
    plKey theirs = sobj.getObjKey();
    if (ours == nullptr && theirs == nullptr)
        return true;
    else if (ours != nullptr && theirs != nullptr)
        return (ours->GetUoid()==theirs->GetUoid());
    else
        return false;
}

// getter and setters
void pySceneObject::addObjKey(const plKey& key)
{
    if (key)
    {
        fSceneObjects.emplace_back(key);
        IAddObjKeyToAll(key);
    }
}

void pySceneObject::addObjPyKey(const pyKey& objkey)
{
    if (objkey.getKey() != nullptr)
    {
        fSceneObjects.emplace_back(objkey.getKey());
        IAddObjKeyToAll(objkey.getKey());
    }
}

plKey pySceneObject::getObjKey() const
{
    if (!fSceneObjects.empty())
        return fSceneObjects[0];
    else
        return nullptr;
}

PyObject* pySceneObject::getObjPyKey() const
{
    PyObject* pyobj;    // Python will manage this... it only knows when everyone is done with it
    if (!fSceneObjects.empty())
        pyobj = pyKey::New(fSceneObjects[0]);
    else
    {
        Py_INCREF(Py_None);
        pyobj = Py_None;
    }
    return pyobj;
}

void pySceneObject::setSenderKey(plKey key)
{ 
    fSenderKey = std::move(key);
    ISetAllSenderKeys();
}

void pySceneObject::SetNetForce(bool state)
{
    // set our flag
    fNetForce = state;
    // then set the netForce flag in the subs
    cyDraw::ConvertFrom(fDraw)->SetNetForce(state);
    cyPhysics::ConvertFrom(fPhysics)->SetNetForce(state);
    cyAvatar::ConvertFrom(fAvatar)->SetNetForce(state);
    cyParticleSys::ConvertFrom(fParticle)->SetNetForce(state);
}


ST::string pySceneObject::GetName()
{
    if (!fSceneObjects.empty())
        return fSceneObjects[0]->GetName();
    return ST::string();
}

PyObject* pySceneObject::findObj(const ST::string& name)
{
    PyObject* pSobj = nullptr;
    // search through the plKeys that we have looking for this name
    for (const plKey& objKey : fSceneObjects)
    {
        if (name == objKey->GetName())
        {
            pSobj = pySceneObject::New(objKey, fPyMod);
            break;
        }
    }

    // did we find one? if not make an object with nil object
    if (pSobj == nullptr)
    {
        // throw a Python error, so the coder knows it didn't work
        PyErr_SetString(PyExc_KeyError, name.c_str());
    }

    return pSobj;
}


//
// deteremine if this object (or the first object in the list)
// ...is locally owned
bool pySceneObject::IsLocallyOwned()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSynchedObject* obj = plSynchedObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj && obj->IsLocallyOwned() == plSynchedObject::kYes )
            return true;
        else
            // both No and Maybe answers will be assumed to be "not locally ownded"
            return false;
    }
    else
        // if we couldn't find any sceneobject, then there is no way that it could be local... heh
        return false;
}


//
// get the local to world matrix
PyObject* pySceneObject::GetLocalToWorld()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyMatrix44::New((hsMatrix44)ci->GetLocalToWorld());
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyMatrix44::New();
}

//
// get the local to world matrix
PyObject* pySceneObject::GetWorldToLocal()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyMatrix44::New((hsMatrix44)ci->GetWorldToLocal());
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyMatrix44::New();
}

//
// get the local to world matrix
PyObject* pySceneObject::GetLocalToParent()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyMatrix44::New((hsMatrix44)ci->GetLocalToParent());
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyMatrix44::New();
}

//
// get the local to world matrix
PyObject* pySceneObject::GetParentToLocal()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyMatrix44::New((hsMatrix44)ci->GetParentToLocal());
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyMatrix44::New();
}

//
// get the local to world matrix
void pySceneObject::SetTransform(pyMatrix44& l2w, pyMatrix44& w2l)
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
            obj->SetTransform(l2w.fMatrix,w2l.fMatrix);
    }
}

//
// find the position of this object (if there are more than one, just the first one)
PyObject* pySceneObject::GetWorldPosition()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyPoint3::New(ci->GetWorldPos());
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyPoint3::New();
}

//
// find the view vector for this object (if there are more than one, just the first one)
PyObject* pySceneObject::GetViewVector()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyVector3::New(ci->GetLocalToWorld().GetAxis(hsMatrix44::kView));
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyVector3::New();
}

//
// find the up vector for this object (if there are more than one, just the first one)
PyObject* pySceneObject::GetUpVector()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyVector3::New(ci->GetLocalToWorld().GetAxis(hsMatrix44::kUp));
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyVector3::New();
}

//
// find the up vector for this object (if there are more than one, just the first one)
PyObject* pySceneObject::GetRightVector()
{
    // make sure that there are sceneobjects
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
                return pyVector3::New(ci->GetLocalToWorld().GetAxis(hsMatrix44::kRight));
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
                return nullptr; // return nullptr to tell python we errored
            }
        }
    }
    // if we couldn't find any sceneobject or a coordinate interface
    return pyVector3::New();
}

//
// deteremine if this object (or any of the object attached)
// ...is an avatar, of any type
bool pySceneObject::IsAvatar()
{
    // loop through all the sceneobject... looking for avatar modifiers
    for (const plKey& objKey : fSceneObjects)
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(objKey->ObjectIsLoaded());
        if ( obj )
        {
            // search through its modifiers to see if one of them is an avatar modifier
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plModifier* mod = obj->GetModifier(i);
                // see if it is an avatar mod base class
                const plArmatureMod* avatar = plArmatureMod::ConvertNoRef(mod);
                if ( avatar )
                    return true;
            }
        }
    }
    // if we couldn't find any sceneobject that had an avatar mod then this ain't an avatar
    return false;
}

#include "plAvatar/plPhysicalControllerCore.h"

PyObject* pySceneObject::GetAvatarVelocity()
{
    // loop through all the sceneobject... looking for avatar modifiers
    for (const plKey& objKey : fSceneObjects)
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(objKey->ObjectIsLoaded());
        if ( obj )
        {
            // search through its modifiers to see if one of them is an avatar modifier
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plModifier* mod = obj->GetModifier(i);
                // see if it is an avatar mod base class
                const plArmatureMod* avatar = plArmatureMod::ConvertNoRef(mod);
                if ( avatar && avatar->GetController() )
                {
                    hsVector3 vel = avatar->GetController()->GetLinearVelocity();
                    return pyVector3::New(vel);
                }
            }
        }
    }

    // if we couldn't find any sceneobject that had an avatar mod then this ain't an avatar
    return pyVector3::New();
}


//
// deteremine if this object (or the first object in the list)
// ...is a human avatar
bool pySceneObject::IsHumanAvatar()
{
    // loop through all the sceneobject... looking for avatar modifiers
    for (const plKey& objKey : fSceneObjects)
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(objKey->ObjectIsLoaded());
        if ( obj )
        {
            // search through its modifiers to see if one of them is an avatar modifier
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plModifier* mod = obj->GetModifier(i);
                // see if it is an avatar mod base class
                plArmatureMod* avatar = (plArmatureMod*)plArmatureMod::ConvertNoRef(mod);
                if ( avatar )
                {
                    for (size_t j = 0; j < avatar->GetBrainCount(); ++j)
                    {
                        if (plAvBrainHuman::ConvertNoRef(avatar->GetBrain(j)))
                            return true;
                    }
                }
            }
        }
    }
    // if we couldn't find any sceneobject that had an avatar mod then this ain't an avatar
    return false;
}

// switch to / from this object (assuming that it is actually a camera)
void pySceneObject::PushCutsceneCamera(bool cut, pyKey& avKey)
{
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {   
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plCameraModifier1* pCam = plCameraModifier1::ConvertNoRef(obj->GetModifier(i));
                if (pCam)
                {
                    plCameraMsg* pMsg = new plCameraMsg;
                    pMsg->SetSender(pCam->GetKey());
                    pMsg->SetBCastFlag(plMessage::kBCastByType);
                    // set command to do the transition
                    if (cut)
                        pMsg->SetCmd(plCameraMsg::kPythonOverridePushCut);
                    else
                        pMsg->SetCmd(plCameraMsg::kPythonOverridePush);
                    // set the new camera
                    pMsg->SetNewCam(pCam->GetKey());
                    pMsg->SetTriggerer(avKey.getKey());
                    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                    return;
                }
            }
        }
    }
}

void pySceneObject::PopCutsceneCamera(pyKey& avKey)
{
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {   
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plCameraModifier1* pCam = plCameraModifier1::ConvertNoRef(obj->GetModifier(i));
                if (pCam)
                {
                    plCameraMsg* pMsg = new plCameraMsg;
                    pMsg->SetSender(pCam->GetKey());
                    pMsg->SetBCastFlag(plMessage::kBCastByType);
                    // set command to do the transition
                    pMsg->SetCmd(plCameraMsg::kPythonOverridePop);
                    // set the new camera
                    pMsg->SetTriggerer(avKey.getKey());
                    pMsg->SetNewCam(pCam->GetKey());
                    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                    return;
                }
            }
        }
    }

}

void pySceneObject::PushCamera(pyKey& avKey)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {   
        for (size_t i = 0; i < obj->GetNumModifiers(); i++)
        {
            const plCameraModifier1* pCam = plCameraModifier1::ConvertNoRef(obj->GetModifier(i));
            if (pCam)
            {
                                    
                // create message
                plCameraMsg* pMsg = new plCameraMsg;
                pMsg->SetSender(pCam->GetKey());
                pMsg->SetBCastFlag(plMessage::kBCastByType);

                // set command to do the transition
                pMsg->SetCmd(plCameraMsg::kResponderTrigger);
                pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
                // set the new camera
                pMsg->SetTriggerer(avKey.getKey());
                pMsg->SetNewCam(pCam->GetKey());

                plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
            }
        }
    }
}

void pySceneObject::PushCameraCut(pyKey& avKey)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {   
        for (size_t i = 0; i < obj->GetNumModifiers(); i++)
        {
            const plCameraModifier1* pCam = plCameraModifier1::ConvertNoRef(obj->GetModifier(i));
            if (pCam)
            {
                                    
                // create message
                plCameraMsg* pMsg = new plCameraMsg;
                pMsg->SetSender(pCam->GetKey());
                pMsg->SetBCastFlag(plMessage::kBCastByType);

                // set command to do the transition
                pMsg->SetCmd(plCameraMsg::kResponderTrigger);
                pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
                pMsg->SetCmd(plCameraMsg::kCut);
                // set the new camera
                pMsg->SetTriggerer(avKey.getKey());
                pMsg->SetNewCam(pCam->GetKey());

                plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
            }
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Pop
//  PARAMETERS :
//
//  PURPOSE    : Restore the state of the virtual camera with a previously saved setting
//
void pySceneObject::PopCamera(pyKey& avKey)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {   
        for (size_t i = 0; i < obj->GetNumModifiers(); i++)
        {
            const plCameraModifier1* pCam = plCameraModifier1::ConvertNoRef(obj->GetModifier(i));
            if (pCam)
            {
                                    
                // create message
                plCameraMsg* pMsg = new plCameraMsg;
                pMsg->SetSender(pCam->GetKey());
                pMsg->SetBCastFlag(plMessage::kBCastByType);

                // set command to do the transition
                pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
                // set the new camera
                pMsg->SetTriggerer(avKey.getKey());
                pMsg->SetNewCam(pCam->GetKey());

                plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
            }
        }
    }
}

std::vector<PyObject*> pySceneObject::GetResponders()
{
    std::vector<PyObject*> pyPL;
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {   
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plResponderModifier* resp = plResponderModifier::ConvertNoRef(obj->GetModifier(i));
                if (resp)
                    pyPL.push_back(pyKey::New(resp->GetKey()));
            }
        }
    }
    return pyPL;
}

std::vector<PyObject*> pySceneObject::GetPythonMods()
{
    std::vector<PyObject*> pyPL;
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if ( obj )
        {   
            for (size_t i = 0; i < obj->GetNumModifiers(); i++)
            {
                const plPythonFileMod* pfm = plPythonFileMod::ConvertNoRef(obj->GetModifier(i));
                if (pfm)
                    pyPL.push_back(pyKey::New(pfm->GetKey()));
            }
        }
    }
    return pyPL;
}

std::vector<PyObject*> pySceneObject::GetImageLibMods()
{
    std::vector<PyObject*> pyPL;
    if (!fSceneObjects.empty())
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if (obj)
        {
            const plImageLibMod* ilm = plImageLibMod::ConvertNoRef(obj->GetModifierByType(plImageLibMod::Index()));
            if (ilm)
                pyPL.push_back(pyKey::New(ilm->GetKey()));
        }
    }
    return pyPL;
}

void pySceneObject::Animate()
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
        pMsg->AddReceiver(obj->GetKey());
        pMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
        pMsg->SetCmd(plAnimCmdMsg::kContinue);
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

int8_t pySceneObject::GetResponderState()
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        for (size_t i = 0; i < obj->GetNumModifiers(); i++)
        {
            const plResponderModifier* resp = plResponderModifier::ConvertNoRef(obj->GetModifier(i));
            if (resp)
                return resp->GetState();
        }
    }
    return -1;
}

void pySceneObject::RewindAnim(const ST::string& animName)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {   
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
        pMsg->AddReceiver(obj->GetKey());
        pMsg->SetAnimName(animName);
        pMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes    
    }
}

void pySceneObject::PlayAnim(const ST::string& animName)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
        pMsg->AddReceiver(obj->GetKey());
        pMsg->SetAnimName(animName);
        pMsg->SetCmd(plAnimCmdMsg::kContinue);
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes    
    }
}

void pySceneObject::StopAnim(const ST::string& animName)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
        pMsg->AddReceiver(obj->GetKey());
        pMsg->SetAnimName(animName);
        pMsg->SetCmd(plAnimCmdMsg::kStop);
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes    
    }
}

void pySceneObject::RunResponder(int state)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        const plModifier* pMod = obj->GetModifierByType(plResponderModifier::Index());
        if (pMod)
        {
            plNotifyMsg* pMsg = new plNotifyMsg;
            pMsg->fType = plNotifyMsg::kResponderChangeState;
            pMsg->AddResponderStateEvent(state);
            pMsg->AddReceiver(pMod->GetKey());
            pMsg->Send();
        
            plNotifyMsg* pMsg0 = new plNotifyMsg;
            pMsg0->fState = true;
            pMsg0->AddReceiver(pMod->GetKey());
            pMsg0->Send();
        }
    }
}


void pySceneObject::FFResponder(int state)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        const plModifier* pMod = obj->GetModifierByType(plResponderModifier::Index());
        if (pMod)
        {
            plNotifyMsg* pMsg = new plNotifyMsg;
            pMsg->fType = plNotifyMsg::kResponderFF;
            pMsg->AddResponderStateEvent(state);
            pMsg->AddReceiver(pMod->GetKey());
            pMsg->Send();
        }
    }
}

void pySceneObject::SetSoundFilename(int index, const ST::string& filename, bool isCompressed)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        const plAudioInterface* ai = obj->GetAudioInterface();
        if (ai)
        {
            plAudible* au = ai->GetAudible();
            if (au)
            {
                au->SetFilename(index, filename.c_str(), isCompressed);
            }
        }
    }
}

int pySceneObject::GetSoundObjectIndex(const ST::string& sndObj)
{
    plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
    if ( obj )
    {
        const plAudioInterface* ai = obj->GetAudioInterface();
        if (ai)
        {
            plAudible* au = ai->GetAudible();
            if (au)
            {
                return au->GetSoundIndex(sndObj.c_str());
            }
        }
    }

    return -1;
}

void pySceneObject::VolumeSensorIgnoreExtraEnters(bool ignore)
{
    if (!fSceneObjects.empty())
    {
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if (obj)
        {
            for (size_t i = 0; i < obj->GetNumModifiers(); ++i)
            {
                plLogicModifier* logic = const_cast<plLogicModifier*>(plLogicModifier::ConvertNoRef(obj->GetModifier(i)));
                if (logic)
                    logic->VolumeIgnoreExtraEnters(ignore);
            }
        }
    }
}

void pySceneObject::VolumeSensorNoArbitration(bool noArbitration)
{
    if (!fSceneObjects.empty()) {
        plSceneObject* obj = plSceneObject::ConvertNoRef(fSceneObjects[0]->ObjectIsLoaded());
        if (obj) {
            for (size_t i = 0; i < obj->GetNumModifiers(); ++i) {
                plLogicModifier* logic = const_cast<plLogicModifier*>(plLogicModifier::ConvertNoRef(obj->GetModifier(i)));
                if (logic)
                    logic->VolumeNoArbitration(noArbitration);
            }
        }
    }
}
