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
/** \file plAvBrainDrive.h
	*/
#ifndef AVBRAINDRIVE_INC
#define AVBRAINDRIVE_INC

// base class
#include "plAvBrain.h"

/** A simple brain used to steer the player around without regard
	for physics. Used in production to quickly fly through a scene
	without having to actually solve puzzles, jump, etc.
	The 'Drive Brain' uses the same input keys as the avatar, with
	a few secret additions for convenience. At the time of this
	writing, you invoke the drive brain by pressing shift-P, and
	then use the forward and back arrows to move and th e left and 
	right arrows to rotate. The 'u' and 'j' keys will move your avatar
	vertically.
	Gravity and collision are completely suspended for avatars in
	drive mode, but they *can* still trigger region detectors if
	you need to fire off triggers, puzzles, or whatnot.
	Note that the drive brain inherits from the user brain, which
	parses control input for us.
*/
class plAvBrainDrive : public plArmatureBrain
{
public:
	plAvBrainDrive();
	/** Canonical constructer. Use this one.
		\param maxVelocity The highest speed this avatar can fly at.
		\param turnRate The speed at which we will turn, in radians per second.
		*/
	plAvBrainDrive(hsScalar maxVelocity, hsScalar turnRate);


	// BRAIN PROTOCOL
	/** Suspend physics and get in line to receive keyboard control messages. */
	virtual void Activate(plArmatureModBase *avMod);

	/** Restore physical reality and stop handling input messages */
	virtual void Deactivate();

	/** Look at the key states and figure out if and how we should move */
	virtual hsBool Apply(double timeNow, hsScalar elapsed);		// main control loop. called by avatar eval()

	// the user brain base handles most of the details of control messages,
	// so this function just looks for the special command which gets us out
	// of drive mode. 
	virtual hsBool MsgReceive(plMessage* pMsg);	// handle control input from the user

	CLASSNAME_REGISTER( plAvBrainDrive );
	GETINTERFACE_ANY( plAvBrainDrive, plArmatureBrain );

protected:
	void IEnablePhysics(bool enable, plKey avKey);
	
	hsScalar	fMaxVelocity;
	hsScalar	fTurnRate;
};

#endif // AVBRAINDRIVE_INC
