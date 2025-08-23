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
#ifndef PLAVBRAINHUMAN_INC
#define PLAVBRAINHUMAN_INC

#include "plAvBrainGeneric.h"
#include "plAvBehaviors.h"
#include "plAnimation/plAGAnim.h"

#include <deque>

class plAGAnimInstance;
class plAGModifier;
class plArmatureUpdateMsg;
class plAvTask;
class plAvTaskMsg;
class plAvBrainHuman;
class plClimbMsg;
class plControlEventMsg;
class plMatrixChannel;
class plMatrixMultiplyApplicator;
class plWalkingStrategy;

class plAvBrainHuman : public plArmatureBrain
{
public:
    plAvBrainHuman(bool isActor = false);
    virtual ~plAvBrainHuman();

    CLASSNAME_REGISTER( plAvBrainHuman );
    GETINTERFACE_ANY( plAvBrainHuman, plArmatureBrain );

    bool Apply(double timeNow, float elapsed) override;
    void Activate(plArmatureModBase *avMod) override;
    void Deactivate() override;
    void Suspend() override;
    void Resume() override;
    void Spawn(double timeNow) override;
    bool LeaveAge() override;

    bool IsActor() const {return fIsActor;}
    void IsActor(bool isActor) {fIsActor = isActor;}

    bool IsMovementZeroBlend();
    void TurnToPoint(hsPoint3 point);   
    bool RunStandardBehaviors(double timeNow, float elapsed);
    void SetStartedTurning(double when);

    virtual bool IsMovingForward();     // we're either walking or running. doesn't account for sliding.
    void ResetIdle();
    void IdleOnly(bool instantOff = false);

    bool IsBehaviorPlaying(int behavior); // returns whether the specified behavior is playing at all (see list of behaviors below)

    void Write(hsStream *stream, hsResMgr *mgr) override;
    void Read(hsStream *stream, hsResMgr *mgr) override;
    bool MsgReceive(plMessage *msg) override;
    void DumpToDebugDisplay(int &x, int &y, int lineHeight, plDebugText &debugTxt) override;

    // Hardwired Identifiers for all the canonical bones.
    enum HumanBoneID {
        Pelvis,
        // left leg
        LThigh, LCalf, LFoot, LFootPrint, LToe0,

        // right leg
        RThigh, RCalf, RFoot, RFootPrint, RToe0,

        // spine and head, starting at base of spine
        Spine, TrunkPrint, Spine1, Spine2, Neck, Head, Jaw, LMouthLower, RMouthLower, 
        LBrowInner, LBrowOuter, LCheek, LEye, LEyeLid01, LEyeLid02, LMouthCorner, LMouthUpper, 
        RBrowInner, RBrowOuter, RCheek, REye, REyeLid01, REyeLid02, RMouthCorner, RMouthUpper,
        
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
    
    static void SetTimeToMaxTurn(float time, bool walk);
    static float GetTimeToMaxTurn(bool walk);
    static void SetMaxTurnSpeed(float radsPerSec, bool walk);
    static float GetMaxTurnSpeed(bool walk);
    static void SetTurnCurve(TurnCurve curve, bool walk);
    static TurnCurve GetTurnCurve(bool walk);
    
    static const float kControlledFlightThreshold;
    static const float kAirTimeThreshold;
    static const float kAirTimePanicThreshold;
    plWalkingStrategy* fWalkingStrategy;
    
protected:
    plAGAnim *FindCustomAnim(const char *baseName);
    virtual bool IHandleControlMsg(plControlEventMsg *ctrlMsg);
    virtual bool IHandleClimbMsg(plClimbMsg *msg);
    bool IHandleTaskMsg(plAvTaskMsg *msg) override;
    virtual bool IInitAnimations();
    virtual void IInitBoneMap();
    float IGetTurnStrength(double timeNow);
    void IChatOn();
    void IChatOff();
    bool IValidateAnimations();
    
    plAGModifier    *fHandleAGMod;      // the ag modifier that's attached to our top object
    double fStartedTurning;         // when did we start turning?   
    uint32_t fPreconditions;
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
        kBehaviorTypeStandingJump       = 0x00000001,
        kBehaviorTypeWalkingJump        = 0x00000002,
        kBehaviorTypeRunningJump        = 0x00000004,
        kBehaviorTypeAnyJump            = kBehaviorTypeStandingJump | kBehaviorTypeWalkingJump | kBehaviorTypeRunningJump,
        kBehaviorTypeRunningImpact      = 0x00000008,
        kBehaviorTypeGroundImpact       = 0x00000010,
        kBehaviorTypeAnyImpact          = kBehaviorTypeRunningImpact | kBehaviorTypeGroundImpact,
        kBehaviorTypeIdle               = 0x00000020,
        kBehaviorTypeWalk               = 0x00000040,
        kBehaviorTypeRun                = 0x00000080,
        kBehaviorTypeWalkBack           = 0x00000100,
        kBehaviorTypeTurnLeft           = 0x00000200,
        kBehaviorTypeTurnRight          = 0x00000400,
        kBehaviorTypeSidestepLeft       = 0x00000800,
        kBehaviorTypeSidestepRight      = 0x00001000,
        kBehaviorTypeFall               = 0x00002000,
        kBehaviorTypeMovingTurnLeft     = 0x00004000,
        kBehaviorTypeMovingTurnRight    = 0x00008000,
        kBehaviorTypeLinkIn             = 0x00010000,
        kBehaviorTypeLinkOut            = 0x00020000,
        kBehaviorTypeEmote              = 0x00040000,
        kBehaviorTypePushWalk           = 0x00080000,
        //kBehaviorTypePushIdle         = 0x00100000,
        kBehaviorTypeNeedsRecalcMask    = kBehaviorTypeAnyJump | kBehaviorTypeRunningImpact | kBehaviorTypeWalk |
                                          kBehaviorTypeRun | kBehaviorTypeWalkBack | kBehaviorTypeTurnLeft |
                                          kBehaviorTypeTurnRight | kBehaviorTypeSidestepLeft |
                                          kBehaviorTypeSidestepRight | kBehaviorTypePushWalk,
    };
    
    plHBehavior();
    ~plHBehavior();

    void Init(plAGAnim *anim, bool loop, plAvBrainHuman *brain, plArmatureMod *body,
              float fadeIn, float fadeOut, uint8_t index, uint32_t type = 0);
    virtual bool PreCondition(double time, float elapsed) { return true; }
    uint32_t GetType() const { return fType; }

protected:
    void IStart() override;
    void IStop() override;

    plArmatureMod *fAvMod;
    plAvBrainHuman *fHuBrain;
    float fFadeIn;                      // speed at which the animation fades in, in blend units per second
    float fFadeOut;                     // speed at which the animation fades out, in blend units per second
    float fMaxBlend;                    // the maximum blend the animation should reach
    uint32_t fType;                       // (see the behaviorType enum)

    bool fStartMsgSent;                 // flags to prevent multiple start and stop messages from being sent
    bool fStopMsgSent;
};

class Idle : public plHBehavior
{
    void IStart() override;
};

class Run : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class Walk : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class WalkBack : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class StepLeft : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class StepRight : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class StandingTurnLeft : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class StandingTurnRight : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class MovingTurn : public plHBehavior
{
public:

protected:
    void IStart() override;
};

class MovingTurnLeft : public MovingTurn
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class MovingTurnRight : public MovingTurn
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class Jump : public plHBehavior
{
protected:
    bool fReleased;
    void IStart() override;
    void IStop() override;
    
public:
    Jump() : plHBehavior(), fReleased(true) {}
};

class StandingJump : public Jump
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class WalkingJump : public Jump
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class RunningJump : public Jump
{
public:
    bool PreCondition(double time, float elapsed) override;
};

class GroundImpact : public plHBehavior
{
public:
    GroundImpact();
    bool PreCondition(double time, float elapsed) override;

private:
    void IStop() override;
    float fDuration;
};

class RunningImpact : public plHBehavior
{
public:
    RunningImpact();
    bool PreCondition(double time, float elapsed) override;

private:
    void IStop() override;
    float fDuration;
};

class Fall : public plHBehavior
{
public:
    bool PreCondition(double time, float elapsed) override;
    void Process(double time, float elapsed) override;
};

class Push : public plHBehavior
{
public:
    void Process(double time, float elapsed) override;
};

//class PushIdle : public Push
//{
//public:
//  bool PreCondition(double time, float elapsed) override;
//};

class PushWalk : public Push
{
public:
    bool PreCondition(double time, float elapsed) override;
};

bool PushSimpleMultiStage(plArmatureMod *avatar, const ST::string& enterAnim, const ST::string& idleAnim,
                          const ST::string& exitAnim, bool netPropagate, bool autoExit, plAGAnim::BodyUsage bodyUsage,
                          plAvBrainGeneric::BrainType type = plAvBrainGeneric::kGeneric);
bool PushRepeatEmote(plArmatureMod* avatar, const ST::string& anim);
bool AvatarEmote(plArmatureMod *avatar, const ST::string& emoteName);


#endif // PLAVBRAINHUMAN_INC
