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
#ifndef PLAVTASKSEEK_INC
#define PLAVTASKSEEK_INC
#pragma once

#include "plAvatarTasks.h"
#include "hsQuat.h"
#include "..\CoreLib\hsGeometry3.h"

class plArmatureMod;
class plArmatureBrain;
class plAvBrainHuman;
class plSceneObject;

// PLAVTASKSEEK
class plAvTaskSeek : public plAvTask
{
public:
	static hsBool fLogProcess;
		
	enum State {
		kSeekRunNormal,
		kSeekAbort,
	};
	UInt8 fState;

	enum 
	{
		kSeekFlagUnForce3rdPersonOnFinish	= 0x1,
		kSeekFlagForce3rdPersonOnStart		= 0x2,
		kSeekFlagNoWarpOnTimeout			= 0x4,
		kSeekFlagRotationOnly				= 0x8,
	};

	plAvTaskSeek();
	plAvTaskSeek(plKey target);
	plAvTaskSeek(plAvSeekMsg *msg);
	plAvTaskSeek(plKey target, plAvAlignment align, const char *animName, bool moving);

	void SetTarget(plKey target);
	void SetTarget(hsPoint3 &pos, hsPoint3 &lookAt);

	/** Initiate the task; make sure we're running on the right type of brain, save off
		user input state, and turn off any other running behaviors.*/
	virtual hsBool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);

	/** Progress towards the goal using a combination of walking and cheating-via-sliding.
		Returns true if we're still working on it; false if we're done. */
	
	virtual hsBool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);

	/** Restore user input state, etc. */
	virtual void Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);
	
	/** clear our target, and when we try to eval, we'll just finish */
	virtual void LeaveAge(plArmatureMod *avatar);

	/** Spew "useful" information to the game screen. Used when Avatar.Debug is active. */
	virtual void DumpDebug(const char *name, int &x, int&y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	void DumpToAvatarLog(plArmatureMod *avatar);
		
	CLASSNAME_REGISTER( plAvTaskSeek );
	GETINTERFACE_ANY( plAvTaskSeek, plAvTask );

protected:

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// METHODS
	//
	/////////////////////////////////////////////////////////////////////////////////////
	
	/** Called by constructors */
	void IInitDefaults();

	/** Make some observations about our current relation to our target.
		Done every frame. */
	hsBool IAnalyze(plArmatureMod *avatar);

	/** Progress towards the goal. We get as close as we can by just pushing the same
		buttons as the user (forward, turn, etc.) when we're really close we slide
		around a bit so we can wind up on the *exact* initial orientation. */
	hsBool IMoveTowardsGoal(plArmatureMod *avatar, plAvBrainHuman *brain, double time, hsScalar elapsed);

	/** Okay, we're in the pure cheating mode now. Try to wrap it up;
		returns true when it's finally there. */
	bool ITryFinish(plArmatureMod *avatar, plAvBrainHuman *brain, double time, hsScalar elapsed);

	/** Final cheating for position */
	hsBool IFinishPosition(hsPoint3 &newPosition, plArmatureMod *avatar, plAvBrainHuman *brain,
						   double time, hsScalar elapsed);

	/** Final cheating for rotation */
	hsBool IFinishRotation(hsQuat &newRotation, plArmatureMod *avatar, plAvBrainHuman *brain,
						   double time, hsScalar elapsed);

	/** If our target's moving, cache its new position and orientation for later math */
	hsBool IUpdateObjective(plArmatureMod *avatar);

	/////////////////////////////////////////////////////////////////////////////////////
	//
	// VARS
	//
	/////////////////////////////////////////////////////////////////////////////////////

	plSceneObject * fSeekObject;			// the seek target....	
	hsQuat fSeekRot;						// The (current) orientation of our objective
	hsPoint3 fSeekPos;						// The (current) position of our objective
	bool fMovingTarget;						// do we check our target's position each frame?
	// animation alignment:
	// sometimes your seek position depends on a particular animation
	// for example, you can say "find a good start point so that you can play this animation
	// and have your handle wind up here" i.e: aligntype = "kAlignHandleAnimEnd"
	plAvAlignment	fAlign;		// how to line up with the seek point
	const char * fAnimName;					// an (optional) anim to use to line up our target
											// so you can say "seek to a place where your hand
											// will be here after you play animation foo"

	hsPoint3 fPosition;						// our current position
	hsQuat fRotation;						// our current rotation
	
	// These are set to true once we EVER get close enough to the goal, so that if we fall out
	// of range from the anim blend out, we don't later try and correct again, and get in a fun
	// little back-and-forth loop.
	hsBool fPosGoalHit;
	hsBool fRotGoalHit;

	hsBool fStillPositioning;				// haven't yet reached the final position
	hsBool fStillRotating;					// haven't yet reached the final orientation
	
	hsVector3 fGoalVec;						// vec from us to the goal
	hsScalar fDistance;						// how far to the goal?
	hsScalar fAngForward;					// 1.0 = goal is forward; -1.0 = goal is backward
	hsScalar fAngRight;						// 1.0 = goal is directly right; -1.0 = goal is directly left
	hsScalar fDistForward;					// distance straight forward to goal plane
	hsScalar fDistRight;					// distance straight right to goal plane

	hsScalar fShuffleRange;
	hsScalar fMaxSidleAngle;		// in right . goal
	hsScalar fMaxSidleRange;		// in feet
	hsScalar fMinFwdAngle;			// in fwd . goal
	hsScalar fMaxBackAngle;			// in fwd . goal
	
	double	 fStartTime;
	UInt8	 fFlags;
	plKey	 fNotifyFinishedKey;	// Send a message to this key when we're done.
};

#endif