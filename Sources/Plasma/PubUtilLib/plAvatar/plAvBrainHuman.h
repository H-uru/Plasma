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
#ifndef PLAVBRAINHUMAN_INC
#define PLAVBRAINHUMAN_INC

#define HI_RES_MOVEMENT

#include "plAvBrainGeneric.h"
#include "plAvBehaviors.h"
#include "plAGAnim.h"

#pragma warning(disable: 4284)
#include <deque>
#include "hsTemplates.h"

class plMatrixChannel;
class plMatrixMultiplyApplicator;
class plAGAnimInstance;
class plAvTask;
class plAvTaskMsg;
class plAvBrainHuman;
class plWalkingController;
class plArmatureUpdateMsg;
class plClimbMsg;
class plControlEventMsg;

class plAvBrainHuman : public plArmatureBrain
{
public:
	plAvBrainHuman(bool isActor = false);
	virtual ~plAvBrainHuman();

	CLASSNAME_REGISTER( plAvBrainHuman );
	GETINTERFACE_ANY( plAvBrainHuman, plArmatureBrain );

	virtual hsBool Apply(double timeNow, hsScalar elapsed);
	virtual void Activate(plArmatureModBase *avMod);
	virtual void Deactivate();
	virtual void Suspend();
	virtual void Resume();
	virtual void Spawn(double timeNow);
	virtual hsBool LeaveAge();

	bool IsActor() const {return fIsActor;}
	void IsActor(bool isActor) {fIsActor = isActor;}

	bool IsMovementZeroBlend();
	void TurnToPoint(hsPoint3 point);	
	hsBool RunStandardBehaviors(double timeNow, float elapsed);
	void SetStartedTurning(double when);

	virtual bool IsMovingForward();		// we're either walking or running. doesn't account for sliding.
	void ResetIdle();
	void IdleOnly(bool instantOff = false);

	bool IsBehaviorPlaying(int behavior); // returns whether the specified behavior is playing at all (see list of behaviors below)

	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual hsBool MsgReceive(plMessage *msg);
	virtual void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	// Hardwired Identifiers for all the canonical bones.
	enum HumanBoneID {
		Pelvis,
		// left leg
		LThigh,	LCalf, LFoot, LFootPrint, LToe0,

		// right leg
		RThigh,	RCalf, RFoot, RFootPrint, RToe0,

		// spine and head, starting at base of spine
		Spine, TrunkPrint, Spine1, Spine2, Neck, Head, Jaw, LMouthLower, RMouthLower, 
		LBrowInner,	LBrowOuter,	LCheek,	LEye, LEyeLid01, LEyeLid02, LMouthCorner, LMouthUpper, 
		RBrowInner,	RBrowOuter,	RCheek,	REye, REyeLid01, REyeLid02,	RMouthCorner, RMouthUpper,
		
		// Left Arm
		LClavicle, LUpperArm, LForearm, LHand, LHandPrint, LMiddleFinger1, LMiddleFinger2, LMiddleFinger3,
		LPinkyFinger1, LPinkyFinger2, LPinkyFinger3, LPointerFinger1, LPointerFinger2, LPointerFinger3,
		LRingFinger1, LRingFinger2, LRingFinger3, LThumb1, LThumb2, LThumb3,
		
		// Right Arm
		RClavicle, RUpperArm, RForearm, RHand, RHandPrint, RMiddleFinger1, RMiddleFinger2, RMiddleFinger3,
		RPinkyFinger1, RPinkyFinger2, RPinkyFinger3, RPointerFinger1, RPointerFinger2, RPointerFinger3,
		RRingFinger1, RRingFinger2, RRingFinger3, RThumb1, RThumb2, RThumb3
	};

	enum TurnCurve
	{
		kTurnLinear,
		kTurnExponential,
		kTurnLogarithmic
	};

	enum // Indicies for the brains array of behaviors
	{
		kIdle,
		kWalk,
		kRun,
		kWalkBack,
		kStandingTurnLeft,
		kStandingTurnRight,
		kStepLeft,
		kStepRight,
		kFall,
		kStandingJump,
		kWalkingJump,
		kRunningJump,
		kGroundImpact,
		kRunningImpact,
		kMovingTurnLeft,
		kMovingTurnRight,
		kPushWalk,
		//kPushIdle,
		kHuBehaviorMax,
	};
	
	static void SetTimeToMaxTurn(float time, hsBool walk);
	static float GetTimeToMaxTurn(hsBool walk);
	static void SetMaxTurnSpeed(float radsPerSec, hsBool walk);
	static float GetMaxTurnSpeed(hsBool walk);
	static void SetTurnCurve(TurnCurve curve, hsBool walk);
	static TurnCurve GetTurnCurve(hsBool walk);
	
	static const hsScalar kControlledFlightThreshold;
	static const hsScalar kAirTimeThreshold;
	static const hsScalar kAirTimePanicThreshold;
	plWalkingController* fCallbackAction;
	
protected:
	plAGAnim *FindCustomAnim(const char *baseName);
	virtual hsBool IHandleControlMsg(plControlEventMsg *ctrlMsg);
	virtual hsBool IHandleClimbMsg(plClimbMsg *msg);
	virtual hsBool IHandleTaskMsg(plAvTaskMsg *msg);
	virtual hsBool IInitAnimations();
	virtual void IInitBoneMap();
	hsScalar IGetTurnStrength(double timeNow);
	void IChatOn();
	void IChatOff();
	hsBool IValidateAnimations();
	
	plAGModifier	*fHandleAGMod;		// the ag modifier that's attached to our top object
	double fStartedTurning;			// when did we start turning?	
	UInt32 fPreconditions;
	bool fIsActor; // are we an actor with special privileges?
	static float fWalkTimeToMaxTurn;
	static float fRunTimeToMaxTurn;
	static float fWalkMaxTurnSpeed;
	static float fRunMaxTurnSpeed;
	static TurnCurve fWalkTurnCurve;
	static TurnCurve fRunTurnCurve;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

class plHBehavior : public plArmatureBehavior
{
	friend class plAvBrainHuman;
	friend class plAvBrainCritter;

public:
	// ID flags for human behaviors. These are used by plBehaviorNotifyMsg
	// to identify what type of behavior just triggered.
	// This is an old way of doing things. I'd like to remove it, but a lot of
	// python scripts use it as is.
	enum BehaviorType
	{
		kBehaviorTypeStandingJump		= 0x00000001,
		kBehaviorTypeWalkingJump		= 0x00000002,
		kBehaviorTypeRunningJump		= 0x00000004,
		kBehaviorTypeAnyJump			= kBehaviorTypeStandingJump | kBehaviorTypeWalkingJump | kBehaviorTypeRunningJump,
		kBehaviorTypeRunningImpact		= 0x00000008,
		kBehaviorTypeGroundImpact		= 0x00000010,
		kBehaviorTypeAnyImpact			= kBehaviorTypeRunningImpact | kBehaviorTypeGroundImpact,
		kBehaviorTypeIdle				= 0x00000020,
		kBehaviorTypeWalk				= 0x00000040,
		kBehaviorTypeRun				= 0x00000080,
		kBehaviorTypeWalkBack			= 0x00000100,
		kBehaviorTypeTurnLeft			= 0x00000200,
		kBehaviorTypeTurnRight			= 0x00000400,
		kBehaviorTypeSidestepLeft		= 0x00000800,
		kBehaviorTypeSidestepRight		= 0x00001000,
		kBehaviorTypeFall				= 0x00002000,
		kBehaviorTypeMovingTurnLeft		= 0x00004000,
		kBehaviorTypeMovingTurnRight	= 0x00008000,
		kBehaviorTypeLinkIn				= 0x00010000,
		kBehaviorTypeLinkOut			= 0x00020000,
		kBehaviorTypeEmote				= 0x00040000,
		kBehaviorTypePushWalk			= 0x00080000,
		//kBehaviorTypePushIdle			= 0x00100000,
		kBehaviorTypeNeedsRecalcMask	= kBehaviorTypeAnyJump | kBehaviorTypeRunningImpact | kBehaviorTypeWalk |
										  kBehaviorTypeRun | kBehaviorTypeWalkBack | kBehaviorTypeTurnLeft |
										  kBehaviorTypeTurnRight | kBehaviorTypeSidestepLeft |
										  kBehaviorTypeSidestepRight | kBehaviorTypePushWalk,
	};
	
	plHBehavior();
	~plHBehavior();

	void Init(plAGAnim *anim, hsBool loop, plAvBrainHuman *brain, plArmatureMod *body,
			  float fadeIn, float fadeOut, UInt8 index, UInt32 type = 0);
	virtual hsBool PreCondition(double time, float elapsed) { return true; }
	UInt32 GetType() const { return fType; }

protected:
	virtual void IStart();
	virtual void IStop();

	plArmatureMod *fAvMod;
	plAvBrainHuman *fHuBrain;
	float fFadeIn;						// speed at which the animation fades in, in blend units per second
	float fFadeOut;						// speed at which the animation fades out, in blend units per second
	float fMaxBlend;					// the maximum blend the animation should reach
	UInt32 fType;						// (see the behaviorType enum)

	bool fStartMsgSent;					// flags to prevent multiple start and stop messages from being sent
	bool fStopMsgSent;
};

class Idle : public plHBehavior
{
	virtual void IStart();
};

class Run : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class Walk : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class WalkBack : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class StepLeft : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class StepRight : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class StandingTurnLeft : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class StandingTurnRight : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class MovingTurn : public plHBehavior
{
public:

protected:
	void IStart();
};

class MovingTurnLeft : public MovingTurn
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class MovingTurnRight : public MovingTurn
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class Jump : public plHBehavior
{
protected:
	hsBool fReleased;
	virtual void IStart();
	virtual void IStop();
	
public:
	Jump() : plHBehavior(), fReleased(true) {}
};

class StandingJump : public Jump
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class WalkingJump : public Jump
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class RunningJump : public Jump
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

class GroundImpact : public plHBehavior
{
public:
	GroundImpact();
	virtual hsBool PreCondition(double time, float elapsed);

private:
	virtual void IStop();
	float fDuration;
};

class RunningImpact : public plHBehavior
{
public:
	RunningImpact();
	virtual hsBool PreCondition(double time, float elapsed);

private:
	virtual void IStop();
	float fDuration;
};

class Fall : public plHBehavior
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
	virtual void Process(double time, float elapsed);
};

class Push : public plHBehavior
{
public:
	virtual void Process(double time, float elapsed);
};

//class PushIdle : public Push
//{
//public:
//	virtual hsBool PreCondition(double time, float elapsed);
//};

class PushWalk : public Push
{
public:
	virtual hsBool PreCondition(double time, float elapsed);
};

bool PushSimpleMultiStage(plArmatureMod *avatar, const char *enterAnim, const char *idleAnim,
						  const char *exitAnim, bool netPropagate, bool autoExit, plAGAnim::BodyUsage bodyUsage,
						  plAvBrainGeneric::BrainType type = plAvBrainGeneric::kGeneric);

bool AvatarEmote(plArmatureMod *avatar, const char *emoteName);


#endif // PLAVBRAINHUMAN_INC
