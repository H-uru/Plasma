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
#ifndef cyPhysics_h
#define cyPhysics_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyPhysics
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//

#include <vector>

#include "pnKeyedObject/plKey.h"
#include "pyGlueDefinitions.h"

class pyPoint3;
class pyVector3;
class pyMatrix44;

class cyPhysics
{
protected:
    plKey           fSender;
    std::vector<plKey> fRecvr;
    bool            fNetForce;

    cyPhysics(plKey sender = {}, plKey recvr = {});

public:
    // required functions for PyObject interoperability
    PYTHON_CLASS_NEW_FRIEND(ptPhysics);
    static PyObject *New(PyObject *sender = nullptr, PyObject *recvr = nullptr);
    PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a cyPhysics object
    PYTHON_CLASS_CONVERT_FROM_DEFINITION(cyPhysics); // converts a PyObject to a cyPhysics (throws error if not correct type)

    static void AddPlasmaClasses(PyObject *m);

    // setters
    void SetSender(plKey sender);
    void AddRecvr(plKey recvr);

    void SetNetForce(bool state) { fNetForce = state; }

    // Enable physics (must already be there)
    void EnableT(bool state);
    void Enable() { EnableT(true); }
    void Disable() { EnableT(false); }
    
    void EnableCollision();
    void DisableCollision();

    // Suggest to physics engine where you want to place something
    void Warp(pyPoint3& pos);
    void WarpObj(pyKey& obj);

    // Suggest to physics engine where you want to place something
    void WarpMat(pyMatrix44& mat);

    // Move the object in a direction and distance
    // if the object is physical then warp it
    // otherwise just use the coordinate interface and set the transform
    void Move(pyVector3& direction, float distance);

    //  Rotate the object
    //  if the object is physical then warp it
    //  otherwise just use the coordinate interface and set the transform
    void Rotate(float rad, pyVector3& axis);

    // apply a force to the center of mass of the receiver
    void Force(pyVector3& force);

    // apply a force to the receiver as though it were being impacted at the
    // given point in global space
    void ForceWithOffset(pyVector3& force, pyPoint3& offset);

    // Apply the given torque force to the body
    // The vector indicates the axes, and the magnitude indicates the strength
    void Torque(pyVector3& torque);

    // Add the given vector to the objects velocity
    void Impulse(pyVector3& impulse);

    // Apply the given impulse to the object at the given point in global space
    // Will impart torque if not applied to center of mass
    void ImpulseWithOffset(pyVector3& impulse, pyPoint3& offset);

    // Add the given vector (representing a rotation axis and magnitude)
    void AngularImpulse(pyVector3& impulse);

    // Decrease all velocities on the given object.
    // A damp factor of 0 nulls them all entirely;
    // A damp factor of 1 leaves them alone.
    void Damp(float damp);

    // Shift the center of mass of the given object by the given
    // amount in the given direction.
    void ShiftMass(pyVector3& offset);
    
    void Suppress(bool doSuppress);
    
    //Set the Linear Velocity of the Object
    void SetLinearVelocity(pyVector3& velocity);
    void SetAngularVelocity(pyVector3& angVel);
};

#endif  // cyPhysics_h
