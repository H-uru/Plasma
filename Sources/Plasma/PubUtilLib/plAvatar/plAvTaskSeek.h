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
#ifndef PLAVTASKSEEK_INC
#define PLAVTASKSEEK_INC

#include "plAvatarTasks.h"
#include "hsQuat.h"
#include "hsGeometry3.h"

class plArmatureMod;
class plArmatureBrain;
class plAvBrainHuman;
class plSceneObject;

// PLAVTASKSEEK
class plAvTaskSeek : public plAvTask
{
public:
    static bool fLogProcess;
        
    enum State {
        kSeekRunNormal,
        kSeekAbort,
    };
    uint8_t fState;

    enum 
    {
        kSeekFlagUnForce3rdPersonOnFinish   = 0x1,
        kSeekFlagForce3rdPersonOnStart      = 0x2,
        kSeekFlagNoWarpOnTimeout            = 0x4,
        kSeekFlagRotationOnly               = 0x8,
    };

    plAvTaskSeek();
    plAvTaskSeek(const plKey& target);
    plAvTaskSeek(plAvSeekMsg *msg);
    plAvTaskSeek(const plKey& target, plAvAlignment align, ST::string animName, bool moving);

    void SetTarget(const plKey& target);
    void SetTarget(hsPoint3 &pos, hsPoint3 &lookAt);

    /** Initiate the task; make sure we're running on the right type of brain, save off
        user input state, and turn off any other running behaviors.*/
    bool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;

    /** Progress towards the goal using a combination of walking and cheating-via-sliding.
        Returns true if we're still working on it; false if we're done. */
    
    bool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;

    /** Restore user input state, etc. */
    void Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    
    /** clear our target, and when we try to eval, we'll just finish */
    void LeaveAge(plArmatureMod *avatar) override;

    /** Spew "useful" information to the game screen. Used when Avatar.Debug is active. */
    void DumpDebug(const char *name, int &x, int&y, int lineHeight, plDebugText &debugTxt) override;

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
    bool IAnalyze(plArmatureMod *avatar);

    /** Progress towards the goal. We get as close as we can by just pushing the same
        buttons as the user (forward, turn, etc.) when we're really close we slide
        around a bit so we can wind up on the *exact* initial orientation. */
    bool IMoveTowardsGoal(plArmatureMod *avatar, plAvBrainHuman *brain, double time, float elapsed);

    /** Okay, we're in the pure cheating mode now. Try to wrap it up;
        returns true when it's finally there. */
    bool ITryFinish(plArmatureMod *avatar, plAvBrainHuman *brain, double time, float elapsed);

    /** Final cheating for position */
    bool IFinishPosition(hsPoint3 &newPosition, plArmatureMod *avatar, plAvBrainHuman *brain,
                           double time, float elapsed);

    /** Final cheating for rotation */
    bool IFinishRotation(hsQuat &newRotation, plArmatureMod *avatar, plAvBrainHuman *brain,
                           double time, float elapsed);

    /** If our target's moving, cache its new position and orientation for later math */
    bool IUpdateObjective(plArmatureMod *avatar);

    /////////////////////////////////////////////////////////////////////////////////////
    //
    // VARS
    //
    /////////////////////////////////////////////////////////////////////////////////////

    plSceneObject * fSeekObject;            // the seek target....  
    hsQuat fSeekRot;                        // The (current) orientation of our objective
    hsPoint3 fSeekPos;                      // The (current) position of our objective
    bool fMovingTarget;                     // do we check our target's position each frame?
    // animation alignment:
    // sometimes your seek position depends on a particular animation
    // for example, you can say "find a good start point so that you can play this animation
    // and have your handle wind up here" i.e: aligntype = "kAlignHandleAnimEnd"
    plAvAlignment   fAlign;     // how to line up with the seek point
    ST::string fAnimName;                   // an (optional) anim to use to line up our target
                                            // so you can say "seek to a place where your hand
                                            // will be here after you play animation foo"
    plMessage* fFinishMsg;

    hsPoint3 fPosition;                     // our current position
    hsQuat fRotation;                       // our current rotation
    
    // These are set to true once we EVER get close enough to the goal, so that if we fall out
    // of range from the anim blend out, we don't later try and correct again, and get in a fun
    // little back-and-forth loop.
    bool fPosGoalHit;
    bool fRotGoalHit;

    bool fSweepTestHit;

    bool fStillPositioning;               // haven't yet reached the final position
    bool fStillRotating;                  // haven't yet reached the final orientation
    
    hsVector3 fGoalVec;                     // vec from us to the goal
    float fDistance;                     // how far to the goal?
    float fAngForward;                   // 1.0 = goal is forward; -1.0 = goal is backward
    float fAngRight;                     // 1.0 = goal is directly right; -1.0 = goal is directly left
    float fDistForward;                  // distance straight forward to goal plane
    float fDistRight;                    // distance straight right to goal plane

    float fShuffleRange;
    float fMaxSidleAngle;        // in right . goal
    float fMaxSidleRange;        // in feet
    float fMinFwdAngle;          // in fwd . goal
    float fMaxBackAngle;         // in fwd . goal
    
    double   fStartTime;
    uint8_t    fFlags;
    plKey    fNotifyFinishedKey;    // Send a message to this key when we're done.
};

#endif
