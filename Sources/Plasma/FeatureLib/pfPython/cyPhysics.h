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
#ifndef cyPhysics_h
#define cyPhysics_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyPhysics
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//

#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

class pyPoint3;
class pyVector3;
class pyMatrix44;
class pyKey;

class cyPhysics
{
protected:
	plKey			fSender;
	hsTArray<plKey>	fRecvr;
	hsBool			fNetForce;

	cyPhysics(plKey sender=nil,plKey recvr=nil);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptPhysics);
	static PyObject *New(PyObject *sender = nil, PyObject *recvr = nil);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyPhysics object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyPhysics); // converts a PyObject to a cyPhysics (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// setters
	void SetSender(plKey &sender);
	void AddRecvr(plKey &recvr);

	virtual void SetNetForce(hsBool state);

	// Enable physics (must already be there)
	virtual void EnableT(hsBool state);
	virtual void Enable();
	virtual void Disable();
	
	virtual void EnableCollision();
	virtual void DisableCollision();

	// Suggest to physics engine where you want to place something
	virtual void Warp(pyPoint3& pos);
	virtual void WarpObj(pyKey& obj);

	// Suggest to physics engine where you want to place something
	virtual void WarpMat(pyMatrix44& mat);

	// Move the object in a direction and distance
	// if the object is physical then warp it
	// otherwise just use the coordinate interface and set the transform
	virtual void Move(pyVector3& direction, hsScalar distance);

	//  Rotate the object
	//  if the object is physical then warp it
	//  otherwise just use the coordinate interface and set the transform
	virtual void Rotate(hsScalar rad, pyVector3& axis);

	// apply a force to the center of mass of the receiver
	virtual void Force(pyVector3& force);

	// apply a force to the receiver as though it were being impacted at the
	// given point in global space
	virtual void ForceWithOffset(pyVector3& force, pyPoint3& offset);

	// Apply the given torque force to the body
	// The vector indicates the axes, and the magnitude indicates the strength
	virtual void Torque(pyVector3& torque);

	// Add the given vector to the objects velocity
	virtual void Impulse(pyVector3& impulse);

	// Apply the given impulse to the object at the given point in global space
	// Will impart torque if not applied to center of mass
	virtual void ImpulseWithOffset(pyVector3& impulse, pyPoint3& offset);

	// Add the given vector (representing a rotation axis and magnitude)
	virtual void AngularImpulse(pyVector3& impulse);

	// Decrease all velocities on the given object.
	// A damp factor of 0 nulls them all entirely;
	// A damp factor of 1 leaves them alone.
	virtual void Damp(hsScalar damp);

	// Shift the center of mass of the given object by the given
	// amount in the given direction.
	virtual void ShiftMass(pyVector3& offset);
	
	virtual void Suppress(bool doSuppress);
	
	//Set the Linear Velocity of the Object
	virtual void SetLinearVelocity(pyVector3& velocity);
	virtual void SetAngularVelocity(pyVector3& angVel);
};

#endif  // cyPhysics_h
