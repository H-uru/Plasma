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
#ifndef plAvBrainClimb_Include
#define plAvBrainClimb_Include
#pragma once

/////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////

#include "plAvBrain.h"
#include "../plMessage/plClimbMsg.h"

/////////////////////////////////////////////////////////////////
//
// PROTOTYPES
//
/////////////////////////////////////////////////////////////////

class plAnimStage;
class plLOSHitMsg;
class plStateDataRecord;

/////////////////////////////////////////////////////////////////
//
// DECLARATION
//
/////////////////////////////////////////////////////////////////
class plAvBrainClimb : public plArmatureBrain
{
public:
	enum Mode {
		kInactive,
		kUnknown,
		kFinishing,
		kDone,
		kClimbingUp,
		kClimbingDown,
		kClimbingLeft,
		kClimbingRight,
		kMountingUp,
		kMountingDown,
		kMountingLeft,
		kMountingRight,
		kDismountingUp,
		kDismountingDown,
		kDismountingLeft,
		kDismountingRight,
		kIdle,
		kReleasing,
		kFallingOff
	};
	
	plAvBrainClimb();
	plAvBrainClimb(Mode initialMode);
	virtual ~plAvBrainClimb();

	virtual void Activate(plArmatureModBase *avMod);
	virtual void Deactivate();
	virtual hsBool Apply(double timeNow, hsScalar elapsed);

	virtual void SaveToSDL(plStateDataRecord *sdl);
	virtual void LoadFromSDL(const plStateDataRecord *sdl);

	void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);
	const char * plAvBrainClimb::WorldDirStr(plClimbMsg::Direction dir);
	const char *plAvBrainClimb::ModeStr(Mode mode);

	// plasma protocol
	virtual hsBool MsgReceive(plMessage *msg);

	CLASSNAME_REGISTER( plAvBrainClimb );
	GETINTERFACE_ANY( plAvBrainClimb, plArmatureBrain);

private:
	bool IAdvanceCurrentStage(double time, float elapsed, float &overage);
	bool ITryStageTransition(double time, float overage);
	bool IChooseNextMode();

	/** Handle a climb message. Note that the "start climbing" climb message is handled
		by the human brain, since there's no climb brain there to hear it, since you
		(by definition) haven't started climbing yet... */
	inline hsBool IHandleClimbMsg(plClimbMsg *msg);
	inline hsBool IHandleLOSMsg(plLOSHitMsg *msg);

	/** Allow or block dismounting in the specified direction. */
	void IEnableDismount(plClimbMsg::Direction dir, bool status);

	/** Allow or block climbing in the specified direction. */
	void IEnableClimb(plClimbMsg::Direction dir, bool status);

	/** Figure out which directions we can go from our current position */
	void ICheckAllowedDirections();

	/** Look left, right, up, and down to see which directions are clear
		for our movement. We could do this by positioning our actual collision
		body and testing for hits, but it gives a lot more false positives *and*
		we won't get the normals of intersection, so it will be more complex
		to figure out which directions are actually blocked.
		The approach here is to do a raycast in the aforementioned directions
		and fail that direction if the raycast hits anything. */
	void IProbeEnvironment();

	/** When we probe to see if we can climb in some direction, we need to know
	    how much room we need in each direction. Right now this is hardcoded,
		but we may switch to using the actual animation climb distances. */
	void ICalcProbeLengths();

	/** Which direction are we trying to go? (up, down, left, or right)
		Just looks at keyboard input. */
	void IGetDesiredDirection();

	/** Let go of the wall and fall. */
	void IRelease(bool intentional);
	
	/** Decide how far forward or backward to move in the animation */
	float IGetAnimDelta(double time, float elapsed);

	/** Create all our animation stage objects. Doesn't actually apply any of the animations
		to the avatar yet. */
	virtual hsBool IInitAnimations();

	/** Find the animation stage corresponding with a mode */
	plAnimStage * IGetStageFromMode(Mode mode);
	Mode IGetModeFromStage(plAnimStage *stage);

	/** The exit stage is a special second stage that runs concurrently with the others.
		It's currently used to blend the "falloff" and "release" animations seamlessly
		with the others. It has a few limitations:
			- it's always expected to play forwards
			- it doesn't pay attention to the keyboard
	*/
	bool IProcessExitStage(double time, float elapsed);

	void IDumpClimbDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);
	void IDumpDismountDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);
	void IDumpBlockedDirections(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	////////////////////////////
	//
	// MEMBERS
	//
	////////////////////////////

	Mode fCurMode;
	Mode fNextMode;
	plClimbMsg::Direction fDesiredDirection;		// up / down / left / right
	float fControlDir;								// 1.0 = move current stage forward -1.0 = move current stage back
	UInt32	fAllowedDirections;
	UInt32	fPhysicallyBlockedDirections;
	UInt32	fOldPhysicallyBlockedDirections;		// for debug display convenience
	UInt32  fAllowedDismounts;

	float fVerticalProbeLength;
	float fHorizontalProbeLength;

	// climbing stages
	plAnimStage *fUp;
	plAnimStage *fDown;
	plAnimStage *fLeft;
	plAnimStage *fRight;
	plAnimStage *fMountUp;
	plAnimStage *fMountDown;
	plAnimStage *fMountLeft;
	plAnimStage *fMountRight;
	plAnimStage *fDismountUp;
	plAnimStage *fDismountDown;
	plAnimStage *fDismountLeft;
	plAnimStage *fDismountRight;
	plAnimStage *fIdle;
	plAnimStage *fRelease;
	plAnimStage *fFallOff;

//	/** Current position on the climbing grid. */
//	int fX;
//	int fY;

	/** The stage that is currently executing. */
	plAnimStage *fCurStage;

	/** A second stage we use (simultaneously) when we need to blend (fall or release) animations. */
	plAnimStage *fExitStage;

};

#endif
