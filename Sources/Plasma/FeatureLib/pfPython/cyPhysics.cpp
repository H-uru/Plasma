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

#include "cyPhysics.h"

#include <Python.h>
#include <string_theory/format>

#include "plgDispatch.h"
#include "hsQuat.h"

#include "pnKeyedObject/plKey.h"
#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plAngularVelocityMsg.h"
#include "plMessage/plDampMsg.h"
#include "plMessage/plImpulseMsg.h"
#include "plMessage/plLinearVelocityMsg.h"

#include "pyGeometry3.h"
#include "pyKey.h"
#include "pyMatrix44.h"

cyPhysics::cyPhysics(plKey sender, plKey recvr)
{
    SetSender(std::move(sender));
    AddRecvr(std::move(recvr));
    fNetForce = false;
}

// setters
void cyPhysics::SetSender(plKey sender)
{
    fSender = std::move(sender);
}

void cyPhysics::AddRecvr(plKey recvr)
{
    if (recvr != nullptr)
        fRecvr.emplace_back(std::move(recvr));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Enable
//  PARAMETERS : 
//
//  PURPOSE    : Enable physics (must already be there)
//
void cyPhysics::EnableT(bool state)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plEnableMsg* pMsg = new plEnableMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // jump back to frame 0
        pMsg->SetCmd(plEnableMsg::kPhysical);
        // which way are we doin' it?
        if ( state )
            pMsg->SetCmd(plEnableMsg::kEnable);
        else
            pMsg->SetCmd(plEnableMsg::kDisable);
        // make sure to propagate this to the modifiers to tell things like the clickables to disable.
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Enable / Disable Collision
//
//  PURPOSE    : Enable / Disable collision for terrain and proxy terrain objects
//               because using cyPhysics::Enable() does not work for these physical types
//
//

void  cyPhysics::EnableCollision()
{
    hsAssert(0, "Who uses this?");
    /*
    // must have a receiver!
    if (!fRecvr.empty())
    {
        plEventGroupEnableMsg* pMsg = new plEventGroupEnableMsg;
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetFlags(plEventGroupEnableMsg::kCollideOn);  
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
    */
}

void  cyPhysics::DisableCollision()
{
    hsAssert(0, "Who uses this?");
    /*
    // must have a receiver!
    if (!fRecvr.empty())
    {
        plEventGroupEnableMsg* pMsg = new plEventGroupEnableMsg;
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetFlags(plEventGroupEnableMsg::kCollideOff); 
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
    */
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Warp
//  PARAMETERS : pos    - the point to translate to
//
//  PURPOSE    : Suggest to physics engine where you want to place something
//
//
void cyPhysics::Warp(pyPoint3& pos)
{
    // create message
    PyObject* matObj = pyMatrix44::New();
    pyMatrix44* mat = pyMatrix44::ConvertFrom(matObj);
    mat->fMatrix.SetTranslate(&pos.fPoint);
    WarpMat(*mat);
    Py_DECREF(matObj);
}

// warp v2 - for warping to the matching transform of an object (like a reference point)
void cyPhysics::WarpObj(pyKey& obj)
{
    plKey obKey = obj.getKey();
    plSceneObject* pObj = plSceneObject::ConvertNoRef(obKey->GetObjectPtr());
    if (pObj && pObj->GetCoordinateInterface())
    {
        // create message
        PyObject* matObj = pyMatrix44::New();
        pyMatrix44* mat = pyMatrix44::ConvertFrom(matObj);
        mat->fMatrix = pObj->GetCoordinateInterface()->GetLocalToWorld();
        WarpMat(*mat);
        Py_DECREF(matObj);
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : WarpMat
//  PARAMETERS : mat     - the matrix to translate to
//
//  PURPOSE    : Suggest to physics engine where you want to place something
//
//
void cyPhysics::WarpMat(pyMatrix44& mat)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plWarpMsg* pMsg = new plWarpMsg(mat.fMatrix);
        pMsg->SetWarpFlags(plWarpMsg::kFlushTransform);
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Move
//  PARAMETERS : direction  - vector of direction to move towards
//             : distance   - how far to move in that direction
//
//  PURPOSE    : Move the object in a direction and distance
//             : if the object is physical then warp it
//             : otherwise just use the coordinate interface and set the transform
//
void cyPhysics::Move(pyVector3& direction, float distance)
{
    //move each receiver (object) separately
    for (const plKey& rcKey : fRecvr)
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(rcKey->GetObjectPtr());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
            {
                hsVector3 offset = direction.fVector * distance;
                hsMatrix44 trans;
                trans.MakeTranslateMat(&offset);

                hsMatrix44 target_matrix = ci->GetWorldToLocal();
                target_matrix = target_matrix * trans;

                // see if this has a physical interface, if so, then its physical, therefore use warp
                const plSimulationInterface* si = obj->GetSimulationInterface();
                if ( si )
                {
                    // create message for each receiver
                    plWarpMsg* pMsg = new plWarpMsg(target_matrix);
                    // check if this needs to be network forced to all clients
                    if (fNetForce )
                    {
                        // set the network propagate flag to make sure it gets to the other clients
                        pMsg->SetBCastFlag(plMessage::kNetPropagate);
                        pMsg->SetBCastFlag(plMessage::kNetForce);
                    }
                    if ( fSender )
                        pMsg->SetSender(fSender);
                    // must have a receiver!
                    pMsg->AddReceiver(rcKey);
                    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                }
                else
                {
                    // else just use the coordinate interface
                    hsMatrix44 w2l;
                    target_matrix.GetInverse(&w2l);
                    obj->SetTransform(target_matrix,w2l);
                }
            }
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Rotate
//  PARAMETERS : rad  - radians to rotate
//             : axis - axis to rotate around
//
//  PURPOSE    : Rotate the object
//             : if the object is physical then warp it
//             : otherwise just use the coordinate interface and set the transform
//
void cyPhysics::Rotate(float rad, pyVector3& axis)
{
    // rotate each receiver (object) separately
    for (const plKey& rcKey : fRecvr)
    {
        // get the object pointer of just the first one in the list
        // (We really can't tell which one the user is thinking of if they are
        // referring to multiple objects, so the first one in the list will do.)
        plSceneObject* obj = plSceneObject::ConvertNoRef(rcKey->GetObjectPtr());
        if ( obj )
        {
            const plCoordinateInterface* ci = obj->GetCoordinateInterface();
            if ( ci )
            {
                hsQuat q(rad, &axis.fVector);
                q.Normalize();
                hsMatrix44 rot;
                q.MakeMatrix(&rot);

                hsMatrix44 target_matrix = ci->GetWorldToLocal();
                target_matrix = target_matrix * rot;

                // see if this has a physical interface, then its physical, therefore use warp
                const plSimulationInterface* si = obj->GetSimulationInterface();
                if ( si )
                {
                    // create message for each receiver
                    plWarpMsg* pMsg = new plWarpMsg(target_matrix);
                    // check if this needs to be network forced to all clients
                    if (fNetForce )
                    {
                        // set the network propagate flag to make sure it gets to the other clients
                        pMsg->SetBCastFlag(plMessage::kNetPropagate);
                        pMsg->SetBCastFlag(plMessage::kNetForce);
                    }
                    if ( fSender )
                        pMsg->SetSender(fSender);
                    // must have a receiver!
                    pMsg->AddReceiver(rcKey);
                    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                }
                else
                {
                    // else just use the coordinate interface
                    hsMatrix44 w2l;
                    target_matrix.GetInverse(&w2l);
                    obj->SetTransform(target_matrix,w2l);
                }
            }
            else
            {
                ST::string errmsg = ST::format("Sceneobject {} does not have a coordinate interface.",
                                               obj->GetKeyName());
                PyErr_SetString(PyExc_RuntimeError, errmsg.c_str());
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Force
//  PARAMETERS : 
//
//  PURPOSE    : apply a force to the center of mass of the receiver
//
//
void cyPhysics::Force(pyVector3& force)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plForceMsg* pMsg = new plForceMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetForce(force.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ForceWithOffset
//  PARAMETERS : 
//
//  PURPOSE    : apply a force to the receiver as though it were being impacted at the
//             : given point in global space
//
//
void cyPhysics::ForceWithOffset(pyVector3& force, pyPoint3& offset)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plOffsetForceMsg* pMsg = new plOffsetForceMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetForce(force.fVector);
        pMsg->SetPoint(offset.fPoint);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Torque
//  PARAMETERS : 
//
//  PURPOSE    : Apply the given torque force to the body
//             : The vector indicates the axes, and the magnitude indicates the strength
//
//
void cyPhysics::Torque(pyVector3& torque)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plTorqueMsg* pMsg = new plTorqueMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetTorque(torque.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Impulse
//  PARAMETERS : 
//
//  PURPOSE    : Add the given vector to the objects velocity
//
//
void cyPhysics::Impulse(pyVector3& impulse)
{
    if (!fRecvr.empty())
    {
        // create message
        plImpulseMsg* pMsg = new plImpulseMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce)
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if (fSender)
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetImpulse(impulse.fVector);
        plgDispatch::MsgSend(pMsg);   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ImpulseWithOffset
//  PARAMETERS : 
//
//  PURPOSE    : Apply the given impulse to the object at the given point in global space
//             : Will impart torque if not applied to center of mass
//
//
void cyPhysics::ImpulseWithOffset(pyVector3& impulse, pyPoint3& offset)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plOffsetImpulseMsg* pMsg = new plOffsetImpulseMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetImpulse(impulse.fVector);
        pMsg->SetPoint(offset.fPoint);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AngularImpulse
//  PARAMETERS : 
//
//  PURPOSE    : Add the given vector (representing a rotation axis and magnitude)
//
//
void cyPhysics::AngularImpulse(pyVector3& impulse)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plAngularImpulseMsg* pMsg = new plAngularImpulseMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetImpulse(impulse.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Damp
//  PARAMETERS : 
//
//  PURPOSE    : Decrease all velocities on the given object.
//             : A damp factor of 0 nulls them all entirely;
//             : A damp factor of 1 leaves them alone.
//
//
void cyPhysics::Damp(float damp)
{
    if (!fRecvr.empty())
    {
        // create message
        plDampMsg* pMsg = new plDampMsg();
        // check if this needs to be network forced to all clients
        if (fNetForce)
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if (fSender)
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetDamp(damp);
        plgDispatch::MsgSend(pMsg);   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ShiftMass
//  PARAMETERS : 
//
//  PURPOSE    : Shift the center of mass of the given object by the given
//             : amount in the given direction.
//
//
void cyPhysics::ShiftMass(pyVector3& offset)
{
    hsAssert(0, "Who uses this?");
    // must have a receiver!
/*  if (!fRecvr.empty())
    {
        // create message
        plShiftMassMsg* pMsg = new plShiftMassMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->SetOffset(offset.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Suppress
//  PARAMETERS : doSuppress:    if true, remove the physical (see below)
//                              if false, add it back
//
//  PURPOSE    : Completely remove the physical, but keep it around so it
//               can be added back later.
//
//
void cyPhysics::Suppress(bool doSuppress)
{
    EnableT(!doSuppress);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetLinearVelocity
//  PARAMETERS : velocity
//
//  PURPOSE    : Change the objects linear velocity to this 
//
//
void cyPhysics::SetLinearVelocity(pyVector3& velocity)
{
    if (!fRecvr.empty())
    {
        // create message
        plLinearVelocityMsg* pMsg = new plLinearVelocityMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);
        
        pMsg->Velocity(velocity.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}
void cyPhysics::SetAngularVelocity(pyVector3& angVel)
{
    if (!fRecvr.empty())
    {
        // create message
        plAngularVelocityMsg* pMsg = new plAngularVelocityMsg;
        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        if ( fSender )
            pMsg->SetSender(fSender);

        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        pMsg->AngularVelocity(angVel.fVector);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }

}
