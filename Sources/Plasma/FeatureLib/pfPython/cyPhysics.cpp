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
#include "cyPhysics.h"

#include "plgDispatch.h"
#include "../pnMessage/plEnableMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../plMessage/plSimInfluenceMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plLinearVelocityMsg.h"
#include "../plMessage/plAngularVelocityMsg.h"

#include "pyGeometry3.h"
#include "pyMatrix44.h"
#include "pyKey.h"
#include "hsQuat.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"

cyPhysics::cyPhysics(plKey sender, plKey recvr)
{
	SetSender(sender);
	AddRecvr(recvr);
	fNetForce = false;
}

// setters
void cyPhysics::SetSender(plKey &sender)
{
	fSender = sender;
}

void cyPhysics::AddRecvr(plKey &recvr)
{
	if ( recvr != nil )
		fRecvr.Append(recvr);
}

void cyPhysics::SetNetForce(hsBool state)
{
	// set our flag
	fNetForce = state;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Enable
//  PARAMETERS : 
//
//  PURPOSE    : Enable physics (must already be there)
//
void cyPhysics::EnableT(hsBool state)
{
	// must have a receiver!
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		// jump back to frame 0
		pMsg->SetCmd(plEnableMsg::kPhysical);
		// which way are we doin' it?
		if ( state )
			pMsg->SetCmd(plEnableMsg::kEnable);
		else
			pMsg->SetCmd(plEnableMsg::kDisable);
		// make sure to propagate this to the modifiers to tell things like the clickables to disable.
		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}
void cyPhysics::Enable()
{
	EnableT(true);
}

void cyPhysics::Disable()
{
	EnableT(false);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Enable / Disable Collision
//
//  PURPOSE    : Enable / Disable collision for terrain and proxy terrain objects
//				 because using cyPhysics::Enable() does not work for these physical types
//
//

void  cyPhysics::EnableCollision()
{
	hsAssert(0, "Who uses this?");
	/*
	// must have a receiver!
	if ( fRecvr.Count() > 0 )
	{
		plEventGroupEnableMsg* pMsg = TRACKED_NEW plEventGroupEnableMsg;
		if (fNetForce )
		{
			// set the network propagate flag to make sure it gets to the other clients
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
			pMsg->SetBCastFlag(plMessage::kNetForce);
		}
		if ( fSender )
			pMsg->SetSender(fSender);

		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetFlags(plEventGroupEnableMsg::kCollideOn);	
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
	*/
}

void  cyPhysics::DisableCollision()
{
	hsAssert(0, "Who uses this?");
	/*
	// must have a receiver!
	if ( fRecvr.Count() > 0 )
	{
		plEventGroupEnableMsg* pMsg = TRACKED_NEW plEventGroupEnableMsg;
		if (fNetForce )
		{
			// set the network propagate flag to make sure it gets to the other clients
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
			pMsg->SetBCastFlag(plMessage::kNetForce);
		}
		if ( fSender )
			pMsg->SetSender(fSender);

		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetFlags(plEventGroupEnableMsg::kCollideOff);	
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
	mat->fMatrix.IdentityMatrix();
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
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plWarpMsg* pMsg = TRACKED_NEW plWarpMsg(mat.fMatrix);
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
void cyPhysics::Move(pyVector3& direction, hsScalar distance)
{
	//move each receiver (object) separately
	int i;
	for ( i=0; i<fRecvr.Count(); i++ )
	{
		// get the object pointer of just the first one in the list
		// (We really can't tell which one the user is thinking of if they are
		// referring to multiple objects, so the first one in the list will do.)
		plSceneObject* obj = plSceneObject::ConvertNoRef(fRecvr[i]->GetObjectPtr());
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
					plWarpMsg* pMsg = TRACKED_NEW plWarpMsg(target_matrix);
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
					pMsg->AddReceiver(fRecvr[i]);
					plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
				char errmsg[256];
				sprintf(errmsg,"Sceneobject %s does not have a coordinate interface.",obj->GetKeyName());
				PyErr_SetString(PyExc_RuntimeError, errmsg);
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
void cyPhysics::Rotate(hsScalar rad, pyVector3& axis)
{
	// rotate each receiver (object) separately
	int i;
	for ( i=0; i<fRecvr.Count(); i++ )
	{
		// get the object pointer of just the first one in the list
		// (We really can't tell which one the user is thinking of if they are
		// referring to multiple objects, so the first one in the list will do.)
		plSceneObject* obj = plSceneObject::ConvertNoRef(fRecvr[i]->GetObjectPtr());
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
					plWarpMsg* pMsg = TRACKED_NEW plWarpMsg(target_matrix);
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
					pMsg->AddReceiver(fRecvr[i]);
					plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
				char errmsg[256];
				sprintf(errmsg,"Sceneobject %s does not have a coordinate interface.",obj->GetKeyName());
				PyErr_SetString(PyExc_RuntimeError, errmsg);
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plForceMsg* pMsg = TRACKED_NEW plForceMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetForce(force.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plOffsetForceMsg* pMsg = TRACKED_NEW plOffsetForceMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetForce(force.fVector);
		pMsg->SetPoint(offset.fPoint);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plTorqueMsg* pMsg = TRACKED_NEW plTorqueMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetTorque(torque.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
	hsAssert(0, "Who uses this?");
	// must have a receiver!
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plImpulseMsg* pMsg = TRACKED_NEW plImpulseMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetImpulse(impulse.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
*/
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plOffsetImpulseMsg* pMsg = TRACKED_NEW plOffsetImpulseMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetImpulse(impulse.fVector);
		pMsg->SetPoint(offset.fPoint);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plAngularImpulseMsg* pMsg = TRACKED_NEW plAngularImpulseMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetImpulse(impulse.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
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
void cyPhysics::Damp(hsScalar damp)
{
	hsAssert(0, "Who uses this?");
	// must have a receiver!
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plDampMsg* pMsg = TRACKED_NEW plDampMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetDamp(damp);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
*/
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
/*	if ( fRecvr.Count() > 0 )
	{
		// create message
		plShiftMassMsg* pMsg = TRACKED_NEW plShiftMassMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->SetOffset(offset.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
*/
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Suppress
//  PARAMETERS : doSuppress:	if true, remove the physical (see below)
//								if false, add it back
//
//  PURPOSE    : Completely remove the physical, but keep it around so it
//				 can be added back later.
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
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plLinearVelocityMsg* pMsg = TRACKED_NEW plLinearVelocityMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		
		pMsg->Velocity(velocity.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}
void cyPhysics::SetAngularVelocity(pyVector3& angVel)
{
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plAngularVelocityMsg* pMsg = TRACKED_NEW plAngularVelocityMsg;
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
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		pMsg->AngularVelocity(angVel.fVector);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}

}