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
#ifndef PL_AV_BRAIN_CRITTER_H
#define PL_AV_BRAIN_CRITTER_H

#include "plAvBrain.h"
#include "hsTemplates.h"
#include "pnKeyedObject/plKey.h"

class plArmatureMod;
class plWalkingController;
class plAIMsg;

class plAvBrainCritter : public plArmatureBrain
{
public:
    enum CritterMode
    {
        kIdle = 0,
        kRun,
        kNumDefaultModes
        // anything >= kNumDefaultModes is user
    };

    enum Anims
    {
        kIdleAnim = 0,
        kRunAnim,
        kNumDefaultAnims
        // anything >= kNumDefaultAnims this is user
    };

    plAvBrainCritter();
    virtual ~plAvBrainCritter();

    CLASSNAME_REGISTER(plAvBrainCritter);
    GETINTERFACE_ANY(plAvBrainCritter, plArmatureBrain);

    hsBool Apply(double time, float elapsed);
    hsBool MsgReceive(plMessage* msg);

    virtual void Activate(plArmatureModBase* avMod);
    virtual void Deactivate();
    virtual void Suspend();
    virtual void Resume();

    void AddBehavior(const std::string& animationName, const std::string& behaviorName, bool loop = true, bool randomStartPos = true,
        float fadeInLen = 2.f, float fadeOutLen = 2.f);
    void StartBehavior(const std::string& behaviorName, bool fade = true);
    bool RunningBehavior(const std::string& behaviorName) const;
    
    void LocallyControlled(bool local) {fLocallyControlled = local;}
    bool LocallyControlled() const {return fLocallyControlled;}

    std::string BehaviorName(int behavior) const;
    plString AnimationName(int behavior) const;
    int CurBehavior() const {return fCurMode;}
    int NextBehavior() const {return fNextMode;}

    std::string IdleBehaviorName() const;
    std::string RunBehaviorName() const;

    void GoToGoal(hsPoint3 newGoal, bool avoidingAvatars = false);
    hsPoint3 CurrentGoal() const {return fFinalGoalPos;}
    bool AvoidingAvatars() const {return fAvoidingAvatars;}
    bool AtGoal() const;

    void StopDistance(float stopDistance) {fStopDistance = stopDistance; fStopDistanceSquared = fStopDistance * fStopDistance;}
    float StopDistance() const {return fStopDistance;}

    void SightCone(float coneRad);
    float SightCone() const {return fSightConeAngle;}
    void SightDistance(float sightDis) {fSightDistance = sightDis; fSightDistanceSquared = fSightDistance * fSightDistance;}
    float SightDistance() const {return fSightDistance;}
    void HearingDistance(float hearDis);
    float HearingDistance() const {return fHearingDistance;}

    bool CanSeeAvatar(unsigned long id) const;
    bool CanHearAvatar(unsigned long id) const;

    std::vector<unsigned long> PlayersICanSee() const;
    std::vector<unsigned long> PlayersICanHear() const;

    hsVector3 VectorToPlayer(unsigned long id) const;

    void AddReceiver(const plKey key);
    void RemoveReceiver(const plKey key);

    virtual void DumpToDebugDisplay(int& x, int& y, int lineHeight, char* strBuf, plDebugText& debugTxt);

    plWalkingController* GetCallbackAction() {return fCallbackAction;}

    // For the console
    static bool fDrawDebug;

protected:
    virtual hsBool IInitBaseAnimations();

    int IPickBehavior(int behavior) const;
    int IPickBehavior(const std::string& behavior) const;

    void IFadeOutBehavior(); // fades out fCurMode
    void IStartBehavior(); // fades in and initializes fNextMode, then sets fCurMode
    void IProcessBehavior(double time, float elapsed); // processes fCurMode
    void IEvalGoal();
    float IGetTurnStrength(double time) const;

    std::vector<unsigned long> IGetAgePlayerIDList() const;

    bool ICanSeeAvatar(plArmatureMod* avatar) const;
    bool ICanHearAvatar(plArmatureMod* avatar) const;

    std::vector<plArmatureMod*> IAvatarsICanSee() const;
    std::vector<plArmatureMod*> IAvatarsICanHear() const;

    plWalkingController* fCallbackAction;
    int fCurMode; // current behavior we are running
    int fNextMode; // the next behavior to run (-1 if we aren't switching on next eval)
    bool fFadingNextBehavior; // is the next behavior supposed to blend?

    bool fLocallyControlled; // is our local AI script the one making all the choices?

    bool fAvoidingAvatars; // are we avoiding avatars to the best of our ability when pathfinding?
    hsPoint3 fFinalGoalPos; // the location we are pathfinding to
    hsPoint3 fImmediateGoalPos; // the location of the point we are immediately going towards (not necessarily our final goal)
    float fDotGoal; // dot product to our goal
    float fAngRight; // dot product of our local right-hand vector to our goal

    float fStopDistance; // how close we need to get to our goal before stopping
    float fStopDistanceSquared; // the above, squared, for faster calculation

    float fSightConeAngle; // in radians, the width of the cone we can see (/2 on each side of where we face, so 90deg cone is 45deg on each side)
    float fSightConeDotMin; // the minimum dot-product of the cone we can see (1 - straight ahead only, 0 - 90deg either side, -1 - 180 behind, or full 360)
    float fSightDistance; // how far away we can see (cone in front of us)
    float fSightDistanceSquared; // the above, squared, for faster calculation
    float fHearingDistance; // how far away we can hear (360 degrees)
    float fHearingDistanceSquared; // the above, squared, for faster calculation
    float fLoudHearingDistanceSquared; // how far away we can hear loud noises, squared, for faster calculation

    std::map<std::string, std::vector<int> > fUserBehaviors; // string is behavior name, internal vector is the list of behaviors that are randomly picked from

    std::vector<plKey> fReceivers; // list of people that want messages from us
};

#endif // PL_AV_BRAIN_CRITTER_H