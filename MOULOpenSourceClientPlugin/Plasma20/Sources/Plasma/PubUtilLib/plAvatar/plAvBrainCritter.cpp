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
#include "hsConfig.h"
#include "hsWindows.h"

#include "plAvCallbackAction.h"
#include "plAvBrainCritter.h"
#include "plAvBrainHuman.h"
#include "plArmatureMod.h"
#include "plAvBehaviors.h"
#include "plAGAnim.h"
#include "plAGAnimInstance.h"
#include "plAvatarMgr.h"

#include "plgDispatch.h"

#include "../plMessage/plAIMsg.h"

#include "../plPipeline/plDebugText.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plMath/plRandom.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetTransport/plNetTransportMember.h"

///////////////////////////////////////////////////////////////////////////////

static plRandom sRandom; // random number generator

const char kDefaultIdleAnimName[] = "Idle";
const char kDefaultIdleBehName[] = "Idle";
const char kDefaultRunAnimName[] = "Run";
const char kDefaultRunBehName[] = "Run";

const float kLoudSoundMultiplyer = 2.0f;

///////////////////////////////////////////////////////////////////////////////

class CritterBehavior : public plArmatureBehavior
{
	friend class plAvBrainCritter;

public:	
	CritterBehavior(const std::string& name, bool randomStart = false, float fadeInLength = 2.f, float fadeOutLength = 2.f) : plArmatureBehavior(),
		fAvMod(nil), fCritterBrain(nil), fName(name), fRandomStartPoint(randomStart), fFadeInLength(fadeInLength), fFadeOutLength(fadeOutLength) {}
	virtual ~CritterBehavior() {}

	void Init(plAGAnim* anim, hsBool loop, plAvBrainCritter* brain, plArmatureMod* body, UInt8 index)
	{
		plArmatureBehavior::Init(anim, loop, brain, body, index);
		fAvMod = body;
		fCritterBrain = brain;
		fAnimName = anim->GetName();
	}

	virtual hsBool PreCondition(double time, float elapsed) {return true;}

	hsScalar GetAnimLength() {return (fAnim->GetAnimation()->GetLength());}
	void SetAnimTime(hsScalar time) {fAnim->SetCurrentTime(time, true);}

	std::string Name() const {return fName;}
	std::string AnimName() const {return fAnimName;}
	bool RandomStartPoint() const {return fRandomStartPoint;}
	float FadeInLength() const {return fFadeInLength;}
	float FadeOutLength() const {return fFadeOutLength;}

protected:
	virtual void IStart()
	{
		plArmatureBehavior::IStart();
		fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
	}

	virtual void IStop()
	{
		plArmatureBehavior::IStop();
		fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
	}		

	plArmatureMod *fAvMod;
	plAvBrainCritter *fCritterBrain;

	std::string fName; // user-created name for this behavior, also used as the index into the brain's behavior map
	std::string fAnimName; // physical animation's name, for reference
	bool fRandomStartPoint; // do we want this behavior to start at a random frame every time we start it?
	float fFadeInLength; // how long to fade in this behavior
	float fFadeOutLength; // how long to fade out this behavior
};

///////////////////////////////////////////////////////////////////////////////

plAvBrainCritter::plAvBrainCritter(): fCallbackAction(nil), fCurMode(kIdle), fNextMode(kIdle), fFadingNextBehavior(true),
	fLocallyControlled(false), fAvoidingAvatars(false), fFinalGoalPos(0, 0, 0), fImmediateGoalPos(0, 0, 0), fDotGoal(0),
	fAngRight(0)
{
	SightCone(hsScalarPI/2); // 90deg
	StopDistance(1);
	SightDistance(10);
	HearingDistance(10);
}

plAvBrainCritter::~plAvBrainCritter()
{
	for (int i = 0; i < fBehaviors.GetCount(); ++i)
	{
		delete fBehaviors[i];
		fBehaviors[i] = nil;
	}

	delete fCallbackAction;
	fCallbackAction = nil;

	fUserBehaviors.clear();
	fReceivers.clear();
}

///////////////////////////////////////////////////////////////////////////////

hsBool plAvBrainCritter::Apply(double time, hsScalar elapsed)
{
	// update internal pathfinding variables
	IEvalGoal();

	if (fNextMode >= kIdle)
	{
		// next mode is set, fade out the previous mode and start up the new one
		IFadeOutBehavior();
		IStartBehavior();
	}
	else
		IProcessBehavior(time, elapsed); // just continue with the currently running one

	// update our controller to keep us turned and moving to where we want to go
	fCallbackAction->RecalcVelocity(time, time - elapsed);		
	fCallbackAction->SetTurnStrength(IGetTurnStrength(time));

	return plArmatureBrain::Apply(time, elapsed);
}

hsBool plAvBrainCritter::MsgReceive(plMessage* msg)
{
	return plArmatureBrain::MsgReceive(msg);
}

///////////////////////////////////////////////////////////////////////////////

void plAvBrainCritter::Activate(plArmatureModBase* avMod)
{
	plArmatureBrain::Activate(avMod);

	// initialize our base "Run" and "Idle" behaviors
	IInitBaseAnimations();

	// create the controller if we haven't done so already
	if (!fCallbackAction)
	{
		plSceneObject* avObj = fArmature->GetTarget(0);
		plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
		plPhysicalControllerCore* controller = avMod->GetController();
		fCallbackAction = TRACKED_NEW plWalkingController(avObj, agMod->GetApplicator(kAGPinTransform), controller);
		fCallbackAction->ActivateController();
	}

	// tell people that care that we are good to go
	plAIBrainCreatedMsg* brainCreated = TRACKED_NEW plAIBrainCreatedMsg(fArmature->GetKey());
	plgDispatch::MsgSend(brainCreated);
}

void plAvBrainCritter::Deactivate()
{
	plArmatureBrain::Deactivate();
}

void plAvBrainCritter::Suspend()
{
	// fade out the previous behavior
	CritterBehavior *behavior = (CritterBehavior*)fBehaviors[fCurMode];
	behavior->SetStrength(0.f, fFadingNextBehavior ? behavior->FadeOutLength() : 0.f);

	// fade in the idle
	fNextMode = kIdle;

	plArmatureBrain::Suspend();
}

void plAvBrainCritter::Resume()
{
	// fade in the idle
	fNextMode = kIdle;

	fCallbackAction->Reset(false);

	plArmatureBrain::Resume();
}

void plAvBrainCritter::AddBehavior(const std::string& animationName, const std::string& behaviorName, bool loop /* = true */, bool randomStartPos /* = true */,
				 float fadeInLen /* = 2.f */, float fadeOutLen /* = 2.f */)
{
	// grab the animations
	plAGAnim* anim = fAvMod->FindCustomAnim(animationName.c_str());
	if (!anim)
		return; // can't find it, die

	// create the behavior and set it up
	CritterBehavior* behavior = TRACKED_NEW CritterBehavior(behaviorName, randomStartPos, fadeInLen, fadeOutLen);
	fBehaviors.Push(behavior);
	behavior->Init(anim, loop, this, fAvMod, fBehaviors.Count() - 1);
	fUserBehaviors[behaviorName].push_back(fBehaviors.Count() - 1);
}

void plAvBrainCritter::StartBehavior(const std::string& behaviorName, bool fade /* = true */)
{
	// make sure the new behavior exists
	if (fUserBehaviors.find(behaviorName) == fUserBehaviors.end())
		return;
	else
	{
		if (fUserBehaviors[behaviorName].size() == 0)
			return;
	}

	// remember the fade request
	fFadingNextBehavior = fade;

	// pick our next behavior
	fNextMode = IPickBehavior(behaviorName);
}

bool plAvBrainCritter::RunningBehavior(const std::string& behaviorName) const
{
	// make sure the behavior exists
	std::map<std::string, std::vector<int> >::const_iterator behaviorIterator = fUserBehaviors.find(behaviorName);
	if (behaviorIterator == fUserBehaviors.end())
		return false;
	else
	{
		if (behaviorIterator->second.size() == 0)
			return false;
	}

	// check all behaviors that use this tag and return true if we are running one of them
	for (unsigned i = 0; i < behaviorIterator->second.size(); ++i)
	{
		if (fCurMode == behaviorIterator->second[i])
			return true;
	}
	return false;
}

std::string plAvBrainCritter::BehaviorName(int behavior) const
{
	if ((behavior >= fBehaviors.Count()) || (behavior < 0))
		return "";
	return ((CritterBehavior*)fBehaviors[behavior])->Name();
}

std::string plAvBrainCritter::AnimationName(int behavior) const
{
	if ((behavior >= fBehaviors.Count()) || (behavior < 0))
		return "";
	return ((CritterBehavior*)fBehaviors[behavior])->AnimName();
}

std::string plAvBrainCritter::IdleBehaviorName() const
{
	return kDefaultIdleBehName;
}

std::string plAvBrainCritter::RunBehaviorName() const
{
	return kDefaultRunBehName;
}

void plAvBrainCritter::GoToGoal(hsPoint3 newGoal, bool avoidingAvatars /* = false */)
{
	fFinalGoalPos = newGoal;
	fAvoidingAvatars = avoidingAvatars;
	fNextMode = IPickBehavior(kRun);
	// TODO: Pathfinding here!
}

bool plAvBrainCritter::AtGoal() const
{
	// we are at our goal if our distance from it is less then or equal to our stopping distance
	hsPoint3 creaturePos;
	hsQuat creatureRot;
	fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);
	hsVector3 finalGoalVec(creaturePos - fFinalGoalPos);
	return (finalGoalVec.MagnitudeSquared() <= fStopDistanceSquared);
}

void plAvBrainCritter::SightCone(hsScalar coneRad)
{
	fSightConeAngle = coneRad;

	// calculate the minimum dot product for the cone of sight (angle/2 vector dotted with straight ahead)
	hsVector3 straightVector(1, 0, 0), viewVector(1, 0, 0);
	hsQuat rotation(fSightConeAngle/2, &hsVector3(0, 1, 0));
	viewVector = hsVector3(rotation.Rotate(&viewVector));
	viewVector.Normalize();
	fSightConeDotMin = straightVector * viewVector;
}

void plAvBrainCritter::HearingDistance(hsScalar hearDis)
{
	fHearingDistance = hearDis;
	fHearingDistanceSquared = fHearingDistance * fHearingDistance;
	fLoudHearingDistanceSquared = (fHearingDistance * kLoudSoundMultiplyer) * (fHearingDistance * kLoudSoundMultiplyer);
}

bool plAvBrainCritter::CanSeeAvatar(unsigned long id) const
{
	plArmatureMod* avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID(id);
	if (avatar)
		return ICanSeeAvatar(avatar);
	return false;
}

bool plAvBrainCritter::CanHearAvatar(unsigned long id) const
{
	plArmatureMod* avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID(id);
	if (avatar)
		return ICanHearAvatar(avatar);
	return false;
}

std::vector<unsigned long> plAvBrainCritter::PlayersICanSee() const
{
	std::vector<unsigned long> allPlayers = IGetAgePlayerIDList();
	std::vector<unsigned long> onesICanSee;
	for (unsigned i = 0; i < allPlayers.size(); ++i)
	{
		if (CanSeeAvatar(allPlayers[i]))
			onesICanSee.push_back(allPlayers[i]);
	}
	return onesICanSee;
}

std::vector<unsigned long> plAvBrainCritter::PlayersICanHear() const
{
	std::vector<unsigned long> allPlayers = IGetAgePlayerIDList();
	std::vector<unsigned long> onesICanHear;
	for (unsigned i = 0; i < allPlayers.size(); ++i)
	{
		if (CanHearAvatar(allPlayers[i]))
			onesICanHear.push_back(allPlayers[i]);
	}
	return onesICanHear;
}

hsVector3 plAvBrainCritter::VectorToPlayer(unsigned long id) const
{
	plArmatureMod* avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID(id);
	if (!avatar)
		return hsVector3(0, 0, 0);

	hsPoint3 avPos;
	hsQuat avRot;
	avatar->GetPositionAndRotationSim(&avPos, &avRot);

	hsPoint3 creaturePos;
	hsQuat creatureRot;
	fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);

	return hsVector3(creaturePos - avPos);
}

void plAvBrainCritter::AddReceiver(const plKey key)
{
	for (unsigned i = 0; i < fReceivers.size(); ++i)
	{
		if (fReceivers[i] == key)
			return; // already in our list
	}
	fReceivers.push_back(key);
}

void plAvBrainCritter::RemoveReceiver(const plKey key)
{
	for (unsigned i = 0; i < fReceivers.size(); ++i)
	{
		if (fReceivers[i] == key)
		{
			fReceivers.erase(fReceivers.begin() + i);
			return;
		}
	}
	return; // not found, do nothing
}

void plAvBrainCritter::DumpToDebugDisplay(int& x, int& y, int lineHeight, char* strBuf, plDebugText& debugTxt)
{
	sprintf(strBuf, "Brain type: Critter");
	debugTxt.DrawString(x, y, strBuf, 0, 255, 255);
	y += lineHeight;

	// extract the name from the behavior running
	if (fBehaviors[fCurMode])
		sprintf(strBuf, "Mode: %s", ((CritterBehavior*)(fBehaviors[fCurMode]))->Name().c_str());
	else
		sprintf(strBuf, "Mode: Unknown");
	
	// draw it
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;
	for (int i = 0; i < fBehaviors.GetCount(); ++i)
		fBehaviors[i]->DumpDebug(x, y, lineHeight, strBuf, debugTxt);
}

///////////////////////////////////////////////////////////////////////////////

hsBool plAvBrainCritter::IInitBaseAnimations()
{
	// create the basic idle and run behaviors, and put them into our appropriate structures
	plAGAnim* idle = fAvMod->FindCustomAnim(kDefaultIdleAnimName);
	plAGAnim* run = fAvMod->FindCustomAnim(kDefaultRunAnimName);

	hsAssert(idle, "Creature is missing idle animation");
	hsAssert(run, "Creature is missing run animation");

	fBehaviors.SetCountAndZero(kNumDefaultModes);

	CritterBehavior* behavior;
	if (idle)
	{
		fBehaviors[kIdle] = behavior = TRACKED_NEW CritterBehavior(kDefaultIdleBehName, true); // starts at a random start point each time
		behavior->Init(idle, true, this, fAvMod, kIdle);
		fUserBehaviors[kDefaultIdleBehName].push_back(kIdle);
	}

	if (run)
	{
		fBehaviors[kRun] = behavior = TRACKED_NEW CritterBehavior(kDefaultRunBehName);
		behavior->Init(run, true, this, fAvMod, kRun);
		fUserBehaviors[kDefaultRunBehName].push_back(kRun);
	}

	return true;
}

int plAvBrainCritter::IPickBehavior(int behavior) const
{
	if ((behavior >= fBehaviors.Count()) || (behavior < 0))
		return IPickBehavior(kDefaultIdleBehName); // do an idle if the behavior is invalid

	CritterBehavior* behaviorObj = (CritterBehavior*)(fBehaviors[behavior]);
	return IPickBehavior(behaviorObj->Name());
}

int plAvBrainCritter::IPickBehavior(const std::string& behavior) const
{
	// make sure the behavior exists
	std::map<std::string, std::vector<int> >::const_iterator behaviorIterator = fUserBehaviors.find(behavior);
	if (behaviorIterator == fUserBehaviors.end())
	{
		if (behavior != kDefaultIdleBehName)
			return IPickBehavior(kDefaultIdleBehName); // do an idle if the behavior is invalid
		return -1; // can't recover from being unable to find an idle!
	}
	else
	{
		unsigned numBehaviors = behaviorIterator->second.size();
		if (numBehaviors == 0)
		{
			if (behavior != kDefaultIdleBehName)
				return IPickBehavior(kDefaultIdleBehName); // do an idle if the behavior is invalid
			return -1; // can't recover from being unable to find an idle!
		}

		// pick our behavior
		unsigned index = sRandom.RandRangeI(0, numBehaviors - 1);
		return behaviorIterator->second[index];
	}
}

void plAvBrainCritter::IFadeOutBehavior()
{
	if ((fCurMode >= fBehaviors.Count()) || (fCurMode < 0))
		return; // invalid fCurMode

	// fade out currently playing behavior
	CritterBehavior* behavior = (CritterBehavior*)fBehaviors[fCurMode];
	behavior->SetStrength(0.f, fFadingNextBehavior ? behavior->FadeOutLength() : 0.f);
}

void plAvBrainCritter::IStartBehavior()
{
	if ((fNextMode >= fBehaviors.Count()) || (fNextMode < 0))
		return; // invalid fNextMode

	// fade in our behavior
	CritterBehavior* behavior = (CritterBehavior*)fBehaviors[fNextMode];
	behavior->SetStrength(1.f, fFadingNextBehavior ? behavior->FadeInLength() : 0.f);

	// if we start at a random point, do so
	if (behavior->RandomStartPoint())
	{
		hsScalar newStart = sRandom.RandZeroToOne() * behavior->GetAnimLength();
		behavior->SetAnimTime(newStart);
	}

	// clean up the internal variables
	fCurMode = fNextMode;
	fNextMode = -1;
}

void plAvBrainCritter::IProcessBehavior(double time, float elapsed)
{
	// run the currently running behavior
	CritterBehavior* behavior = (CritterBehavior*)fBehaviors[fCurMode];
	behavior->SetStrength(1.f, fFadingNextBehavior ? behavior->FadeInLength() : 0.f);
	behavior->Process(time, elapsed);
}

void plAvBrainCritter::IEvalGoal()
{
	// TODO: Implement pathfinding logic here
	// (for now, this runs directly towards the goal)
	fImmediateGoalPos = fFinalGoalPos;

	// where am I relative to my goal?
	const plSceneObject* creatureObj = fArmature->GetTarget(0);
	hsVector3 view(creatureObj->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView));

	hsPoint3 creaturePos;
	hsQuat creatureRot;
	fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);
	hsVector3 goalVec(creaturePos - fImmediateGoalPos);
	goalVec.Normalize();
	fDotGoal = goalVec * view; // 1 = directly facing, 0 = 90 deg off, -1 = facing away

	// calculate a vector pointing to the creature's right
	hsQuat invRot = creatureRot.Conjugate();
	hsPoint3 globRight = invRot.Rotate(&kAvatarRight);
	fAngRight = globRight.InnerProduct(goalVec); // dot product, 1 = goal is 90 to the right, 0 = goal is in front or behind, -1 = goal is 90 to the left

	if (fAvoidingAvatars)
	{
		// check to see we can see anyone in our way (if we can't see them, we can't avoid them)
		std::vector<plArmatureMod*> playersICanSee = IAvatarsICanSee();
		for (unsigned i = 0; i < playersICanSee.size(); ++i)
		{
			hsPoint3 avPos;
			hsQuat avRot;
			playersICanSee[i]->GetPositionAndRotationSim(&avPos, &avRot);
			hsVector3 avVec(creaturePos - avPos);
			avVec.Normalize();

			hsScalar dotAv = avVec * goalVec;
			if (dotAv > 0.5f) // within a 45deg angle in front of us
			{
				// a player is in the way, so we will change our "goal" to a 90deg angle from the player
				// then we stop searching, since any other players in the way will just produce the same (or similar) result
				avVec.fZ = goalVec.fZ = 0.f;
				goalVec = goalVec % avVec;
				fAngRight = globRight.InnerProduct(goalVec);
				break;
			}
		}
	}

	// are we at our final goal?
	if (AtGoal())
	{
		if (RunningBehavior(kDefaultRunBehName)) // don't do anything if we're not running!
		{
			// we're close enough, stop running and pick an idle
			fNextMode = IPickBehavior(kIdle);

			// tell everyone who cares that we have arrived
			for (unsigned i = 0; i < fReceivers.size(); ++i)
			{
				plAIArrivedAtGoalMsg* msg = TRACKED_NEW plAIArrivedAtGoalMsg(fArmature->GetKey(), fReceivers[i]);
				msg->Goal(fFinalGoalPos);
				msg->Send();
			}
		}
	}
}

hsScalar plAvBrainCritter::IGetTurnStrength(double time) const
{
	if (!RunningBehavior(kDefaultRunBehName))
		return 0.0f;

	// am I directly facing my goal?
	if (fDotGoal < -0.98)
		return 0.f;

	if (fAngRight > 0.f)
		return 1.f;
	return -1.f;
}

std::vector<unsigned long> plAvBrainCritter::IGetAgePlayerIDList() const
{
	// make a list of non-local players
	std::vector<unsigned long> playerIDs;
	std::map<unsigned long, bool> tempMap; // slightly hacky way to remove dups
	plNetClientMgr* nc = plNetClientMgr::GetInstance();
	for (int i = 0; i < nc->TransportMgr().GetNumMembers(); ++i)
	{
		plNetTransportMember* mbr = nc->TransportMgr().GetMember(i);
		unsigned long id = mbr->GetPlayerID();
		if (tempMap.find(id) == tempMap.end())
		{
			playerIDs.push_back(id);
			tempMap[id] = true;
		}
	}
	// add the local player if he isn't already in the list
	unsigned long localID = nc->GetPlayerID();
	if (tempMap.find(localID) == tempMap.end())
		playerIDs.push_back(localID);

	// return result
	return playerIDs;
}

bool plAvBrainCritter::ICanSeeAvatar(plArmatureMod* avatar) const
{
	// sight is a x deg cone in front of the critter, cuts off at a certain distance
	hsPoint3 avPos;
	hsQuat avRot;
	avatar->GetPositionAndRotationSim(&avPos, &avRot);

	hsPoint3 creaturePos;
	hsQuat creatureRot;
	fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);

	hsVector3 avVec(creaturePos - avPos);
	if (avVec.MagnitudeSquared() > fSightDistanceSquared)
		return false; // too far away
	avVec.Normalize();
	
	const plSceneObject* creatureObj = fArmature->GetTarget(0);
	hsVector3 view(creatureObj->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView));
	hsScalar avDot = view * avVec;
	if (avDot < fSightConeDotMin)
		return false; // out of our cone of view
	return true;
}

bool plAvBrainCritter::ICanHearAvatar(plArmatureMod* avatar) const
{
	// check to see if the avatar is being loud (running or jumping)
	bool isLoud = false;
	plAvBrainHuman* humanBrain = plAvBrainHuman::ConvertNoRef(avatar->FindBrainByClass(plAvBrainHuman::Index()));
	if (humanBrain)
	{
		isLoud = humanBrain->IsBehaviorPlaying(plAvBrainHuman::kRun) || humanBrain->IsBehaviorPlaying(plAvBrainHuman::kStandingJump) ||
			humanBrain->IsBehaviorPlaying(plAvBrainHuman::kWalkingJump) || humanBrain->IsBehaviorPlaying(plAvBrainHuman::kRunningJump) ||
			humanBrain->IsBehaviorPlaying(plAvBrainHuman::kGroundImpact) || humanBrain->IsBehaviorPlaying(plAvBrainHuman::kRunningImpact);
	}

	// hearing is 360 degrees around the critter, cuts off at a certain distance
	hsPoint3 avPos;
	hsQuat avRot;
	avatar->GetPositionAndRotationSim(&avPos, &avRot);

	hsPoint3 creaturePos;
	hsQuat creatureRot;
	fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);

	hsVector3 avVec(creaturePos - avPos);
	hsScalar distSq = avVec.MagnitudeSquared();
	if (distSq <= fHearingDistanceSquared)
		return true; // within our normal hearing distance
	else if (isLoud && (distSq <= fLoudHearingDistanceSquared))
		return true; // they are being loud, and within our loud hearing distance
	return false;
}

std::vector<plArmatureMod*> plAvBrainCritter::IAvatarsICanSee() const
{
	std::vector<unsigned long> allPlayers = IGetAgePlayerIDList();
	std::vector<plArmatureMod*> onesICanSee;
	for (unsigned i = 0; i < allPlayers.size(); ++i)
	{
		plArmatureMod* avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID(allPlayers[i]);
		if (!avatar)
			continue;

		if (ICanSeeAvatar(avatar))
			onesICanSee.push_back(avatar);
	}
	return onesICanSee;
}

std::vector<plArmatureMod*> plAvBrainCritter::IAvatarsICanHear() const
{
	std::vector<unsigned long> allPlayers = IGetAgePlayerIDList();
	std::vector<plArmatureMod*> onesICanHear;
	for (unsigned i = 0; i < allPlayers.size(); ++i)
	{
		plArmatureMod* avatar = plAvatarMgr::GetInstance()->FindAvatarByPlayerID(allPlayers[i]);
		if (!avatar)
			continue;

		if (ICanHearAvatar(avatar))
			onesICanHear.push_back(avatar);
	}
	return onesICanHear;
}
