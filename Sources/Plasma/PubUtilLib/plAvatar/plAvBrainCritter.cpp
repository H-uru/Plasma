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

#include "plAvBrainCritter.h"

#include "plgDispatch.h"

#include <algorithm>
#include <string_theory/formatter>
#include <utility>

#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvBehaviors.h"
#include "plAvBrainHuman.h"
#include "plAvDefs.h"
#include "plPhysicalControllerCore.h"

#include "pnEncryption/plRandom.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAnimation/plAGAnim.h"
#include "plAnimation/plAGAnimInstance.h"
#include "plAnimation/plAGModifier.h"

#include "plMessage/plAIMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plPipeline/plDebugText.h"

///////////////////////////////////////////////////////////////////////////////

plRandom plAvBrainCritter::sRandom; // random number generator

const ST::string kDefaultIdleAnimName = ST_LITERAL("Idle");
const ST::string kDefaultIdleBehName = ST_LITERAL("Idle");
const ST::string kDefaultRunAnimName = ST_LITERAL("Run");
const ST::string kDefaultRunBehName = ST_LITERAL("Run");

const float kLoudSoundMultiplyer = 2.0f;

///////////////////////////////////////////////////////////////////////////////

class CritterBehavior : public plArmatureBehavior
{
    friend class plAvBrainCritter;

public: 
    CritterBehavior(ST::string name, bool randomStart = false, float fadeInLength = 2.f, float fadeOutLength = 2.f) : plArmatureBehavior(),
        fAvMod(), fCritterBrain(), fName(std::move(name)), fRandomStartPoint(randomStart), fFadeInLength(fadeInLength), fFadeOutLength(fadeOutLength) { }
    virtual ~CritterBehavior() {}

    void Init(plAGAnim* anim, bool loop, plAvBrainCritter* brain, plArmatureMod* body, uint8_t index)
    {
        plArmatureBehavior::Init(anim, loop, brain, body, index);
        fAvMod = body;
        fCritterBrain = brain;
        fAnimName = anim->GetName();
    }

    virtual bool PreCondition(double time, float elapsed) {return true;}

    float GetAnimLength() {return (fAnim->GetAnimation()->GetLength());}
    void SetAnimTime(float time) {fAnim->SetCurrentTime(time, true);}

    ST::string Name() const {return fName;}
    ST::string AnimName() const {return fAnimName;}
    bool RandomStartPoint() const {return fRandomStartPoint;}
    float FadeInLength() const {return fFadeInLength;}
    float FadeOutLength() const {return fFadeOutLength;}

protected:
    void IStart() override
    {
        plArmatureBehavior::IStart();
        fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
    }

    void IStop() override
    {
        plArmatureBehavior::IStop();
        fAvMod->SynchIfLocal(hsTimer::GetSysSeconds(), false);
    }       

    plArmatureMod *fAvMod;
    plAvBrainCritter *fCritterBrain;

    ST::string fName; // user-created name for this behavior, also used as the index into the brain's behavior map
    ST::string fAnimName; // physical animation's name, for reference
    bool fRandomStartPoint; // do we want this behavior to start at a random frame every time we start it?
    float fFadeInLength; // how long to fade in this behavior
    float fFadeOutLength; // how long to fade out this behavior
};

///////////////////////////////////////////////////////////////////////////////

plAvBrainCritter::plAvBrainCritter()
    : fWalkingStrategy(), fCurMode(kIdle), fNextMode(kIdle),
      fFadingNextBehavior(true), fAvoidingAvatars(), fDotGoal(),
      fAngRight()
{
    SightCone(hsConstants::half_pi<float>); // 90deg
    StopDistance(1.f);
    SightDistance(10.f);
    HearingDistance(10.f);
}

plAvBrainCritter::~plAvBrainCritter()
{
    for (size_t i = 0; i < fBehaviors.size(); ++i)
    {
        delete fBehaviors[i];
        fBehaviors[i] = nullptr;
    }

    delete fWalkingStrategy;
    fWalkingStrategy = nullptr;

    fUserBehaviors.clear();
    fReceivers.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool plAvBrainCritter::Apply(double time, float elapsed)
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
    fWalkingStrategy->SetTurnStrength(IGetTurnStrength(time));
    fWalkingStrategy->RecalcVelocity(time, elapsed);

    return plArmatureBrain::Apply(time, elapsed);
}

bool plAvBrainCritter::MsgReceive(plMessage* msg)
{
    if (auto* pGoToMsg = plAIGoToGoalMsg::ConvertNoRef(msg)) {
        GoToGoal(pGoToMsg->Goal(), pGoToMsg->AvoidingAvatars());
        return true;
    }

    return plArmatureBrain::MsgReceive(msg);
}

///////////////////////////////////////////////////////////////////////////////

void plAvBrainCritter::Activate(plArmatureModBase* avMod)
{
    plArmatureBrain::Activate(avMod);

    // initialize our base "Run" and "Idle" behaviors
    IInitBaseAnimations();

    // create the controller if we haven't done so already
    if (!fWalkingStrategy)
    {
        plSceneObject* avObj = fArmature->GetTarget(0);
        plAGModifier* agMod = const_cast<plAGModifier*>(plAGModifier::ConvertNoRef(FindModifierByClass(avObj, plAGModifier::Index())));
        plPhysicalControllerCore* controller = avMod->GetController();
        fWalkingStrategy = new plWalkingStrategy(agMod->GetApplicator(kAGPinTransform), controller);
        controller->SetMovementStrategy(fWalkingStrategy);
    }

    // tell people that care that we are good to go
    plAIBrainCreatedMsg* brainCreated = new plAIBrainCreatedMsg(fArmature->GetKey());
    plgDispatch::MsgSend(brainCreated);
}

void plAvBrainCritter::Deactivate()
{
    // Although "destroyed" is in the past tense, this is really a warning message.
    // The brain is about to go away, so save anything you need to and toss it!
    plAIBrainDestroyedMsg* brainDestroyed = new plAIBrainDestroyedMsg(fArmature->GetKey(), nullptr);
    brainDestroyed->AddReceivers(fReceivers);
    brainDestroyed->Send();

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

    fWalkingStrategy->Reset(false);

    plArmatureBrain::Resume();
}

plSceneObject* plAvBrainCritter::GetTarget() const
{
    if (fArmature)
        return fArmature->GetTarget(0);
    return nullptr;
}

void plAvBrainCritter::AddBehavior(const ST::string& animationName, const ST::string& behaviorName, bool loop /* = true */, bool randomStartPos /* = true */,
                 float fadeInLen /* = 2.f */, float fadeOutLen /* = 2.f */)
{
    // grab the animations
    plAGAnim* anim = fAvMod->FindCustomAnim(animationName);
    if (!anim)
        return; // can't find it, die

    // create the behavior and set it up
    CritterBehavior* behavior = new CritterBehavior(behaviorName, randomStartPos, fadeInLen, fadeOutLen);
    fBehaviors.emplace_back(behavior);
    behavior->Init(anim, loop, this, fAvMod, fBehaviors.size() - 1);
    fUserBehaviors[behaviorName].push_back(fBehaviors.size() - 1);
}

void plAvBrainCritter::StartBehavior(const ST::string& behaviorName, bool fade /* = true */)
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

bool plAvBrainCritter::RunningBehavior(const ST::string& behaviorName) const
{
    // make sure the behavior exists
    auto behaviorIterator = fUserBehaviors.find(behaviorName);
    if (behaviorIterator == fUserBehaviors.end())
        return false;
    else
    {
        if (behaviorIterator->second.size() == 0)
            return false;
    }

    // check all behaviors that use this tag and return true if we are running one of them
    for (size_t i = 0; i < behaviorIterator->second.size(); ++i)
    {
        if (fCurMode == behaviorIterator->second[i])
            return true;
    }
    return false;
}

ST::string plAvBrainCritter::BehaviorName(int behavior) const
{
    if ((behavior < 0) || ((size_t)behavior >= fBehaviors.size()))
        return {};
    return ((CritterBehavior*)fBehaviors[behavior])->Name();
}

ST::string plAvBrainCritter::AnimationName(int behavior) const
{
    if ((behavior < 0) || ((size_t)behavior >= fBehaviors.size()))
        return {};
    return ((CritterBehavior*)fBehaviors[behavior])->AnimName();
}

ST::string plAvBrainCritter::IdleBehaviorName() const
{
    return kDefaultIdleBehName;
}

ST::string plAvBrainCritter::RunBehaviorName() const
{
    return kDefaultRunBehName;
}

void plAvBrainCritter::GoToGoal(hsPoint3 newGoal, bool avoidingAvatars /* = false */)
{
    fFinalGoalPos = newGoal;
    fAvoidingAvatars = avoidingAvatars; // TODO: make this do something?

    // Only play the run behavior if it's not already activated
    // Why? This might just be an update to a preexisting goal.
    if(!RunningBehavior(RunBehaviorName()))
        fNextMode = IPickBehavior(kRun);
    // Missing TODO Turd: Pathfinding.

    // Let everyone who's listending know that we're going somewhere.
    plAIGoToGoalMsg* pMsg = new plAIGoToGoalMsg(fArmature->GetKey(), nullptr);
    pMsg->AddReceivers(fReceivers);
    pMsg->Goal(newGoal);
    pMsg->AvoidingAvatars(avoidingAvatars);
    pMsg->Send();
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

void plAvBrainCritter::SightCone(float coneRad)
{
    fSightConeAngle = coneRad;

    // calculate the minimum dot product for the cone of sight (angle/2 vector dotted with straight ahead)
    hsVector3 straightVector(1.f, 0.f, 0.f);
    hsVector3 viewVector(1.f, 0.f, 0.f);
    hsVector3 up(0.f, 1.f, 0.f);
    hsQuat rotation(fSightConeAngle/2, &up);
    viewVector = hsVector3(rotation.Rotate(&viewVector));
    viewVector.Normalize();
    fSightConeDotMin = straightVector * viewVector;
}

void plAvBrainCritter::HearingDistance(float hearDis)
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
        return {};

    hsPoint3 avPos;
    hsQuat avRot;
    avatar->GetPositionAndRotationSim(&avPos, &avRot);

    hsPoint3 creaturePos;
    hsQuat creatureRot;
    fAvMod->GetPositionAndRotationSim(&creaturePos, &creatureRot);

    return hsVector3(creaturePos - avPos);
}

void plAvBrainCritter::AddReceiver(plKey key)
{
    for (unsigned i = 0; i < fReceivers.size(); ++i)
    {
        if (fReceivers[i] == key)
            return; // already in our list
    }
    fReceivers.emplace_back(std::move(key));
}

void plAvBrainCritter::RemoveReceiver(const plKey& key)
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

void plAvBrainCritter::DumpToDebugDisplay(int& x, int& y, int lineHeight, plDebugText& debugTxt)
{
    debugTxt.DrawString(x, y, ST_LITERAL("Brain type: Critter"), 0, 255, 255);
    y += lineHeight;

    // extract the name from the behavior running
    ST::string mode = ST_LITERAL("Mode: Unknown");
    if (fBehaviors[fCurMode])
        mode = ST::format("Mode: {}", ((CritterBehavior*)(fBehaviors[fCurMode]))->Name());
    
    // draw it
    debugTxt.DrawString(x, y, mode);
    y += lineHeight;
    for (plArmatureBehavior* behavior : fBehaviors)
        behavior->DumpDebug(x, y, lineHeight, debugTxt);
}

///////////////////////////////////////////////////////////////////////////////

bool plAvBrainCritter::IInitBaseAnimations()
{
    // create the basic idle and run behaviors, and put them into our appropriate structures
    plAGAnim* idle = fAvMod->FindCustomAnim(kDefaultIdleAnimName);
    plAGAnim* run = fAvMod->FindCustomAnim(kDefaultRunAnimName);

    hsAssert(idle, "Creature is missing idle animation");
    hsAssert(run, "Creature is missing run animation");

    fBehaviors.assign(kNumDefaultModes, nullptr);

    CritterBehavior* behavior;
    if (idle)
    {
        fBehaviors[kIdle] = behavior = new CritterBehavior(kDefaultIdleBehName, true); // starts at a random start point each time
        behavior->Init(idle, true, this, fAvMod, kIdle);
        fUserBehaviors[kDefaultIdleBehName].push_back(kIdle);
    }

    if (run)
    {
        fBehaviors[kRun] = behavior = new CritterBehavior(kDefaultRunBehName);
        behavior->Init(run, true, this, fAvMod, kRun);
        fUserBehaviors[kDefaultRunBehName].push_back(kRun);
    }

    return true;
}

int plAvBrainCritter::IPickBehavior(int behavior) const
{
    if ((behavior < 0) || ((size_t)behavior >= fBehaviors.size()))
        return IPickBehavior(kDefaultIdleBehName); // do an idle if the behavior is invalid

    CritterBehavior* behaviorObj = (CritterBehavior*)(fBehaviors[behavior]);
    return IPickBehavior(behaviorObj->Name());
}

int plAvBrainCritter::IPickBehavior(const ST::string& behavior) const
{
    // make sure the behavior exists
    auto behaviorIterator = fUserBehaviors.find(behavior);
    if (behaviorIterator == fUserBehaviors.end())
    {
        if (behavior != kDefaultIdleBehName)
            return IPickBehavior(kDefaultIdleBehName); // do an idle if the behavior is invalid
        return -1; // can't recover from being unable to find an idle!
    }
    else
    {
        size_t numBehaviors = behaviorIterator->second.size();
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
    if ((fCurMode < 0) || ((size_t)fCurMode >= fBehaviors.size()))
        return; // invalid fCurMode

    // fade out currently playing behavior
    CritterBehavior* behavior = (CritterBehavior*)fBehaviors[fCurMode];
    behavior->SetStrength(0.f, fFadingNextBehavior ? behavior->FadeOutLength() : 0.f);
}

void plAvBrainCritter::IStartBehavior()
{
    if ((fNextMode < 0) || (fNextMode >= fBehaviors.size()))
        return; // invalid fNextMode

    // fade in our behavior
    CritterBehavior* behavior = (CritterBehavior*)fBehaviors[fNextMode];
    behavior->SetStrength(1.f, fFadingNextBehavior ? behavior->FadeInLength() : 0.f);

    // if we start at a random point, do so
    if (behavior->RandomStartPoint())
    {
        float newStart = sRandom.RandZeroToOne() * behavior->GetAnimLength();
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

            float dotAv = avVec * goalVec;
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
            plAIArrivedAtGoalMsg* msg = new plAIArrivedAtGoalMsg(fArmature->GetKey(), nullptr);
            msg->AddReceivers(fReceivers);
            msg->Goal(fFinalGoalPos);
            msg->Send();
        }
    }
}

float plAvBrainCritter::IGetTurnStrength(double time) const
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
    plNetClientMgr* nc = plNetClientMgr::GetInstance();
    for (size_t i = 0; i < nc->TransportMgr().GetNumMembers(); ++i)
    {
        plNetTransportMember* mbr = nc->TransportMgr().GetMember(i);
        unsigned long id = mbr->GetPlayerID();
        playerIDs.push_back(id);
    }
    // add the local player if he isn't already in the list
    unsigned long localID = nc->GetPlayerID();
    if (std::find(playerIDs.begin(), playerIDs.end(), localID) == playerIDs.end())
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
    float avDot = view * avVec;
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
    float distSq = avVec.MagnitudeSquared();
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
