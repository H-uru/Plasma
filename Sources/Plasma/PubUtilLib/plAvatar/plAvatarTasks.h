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
/** \file plAvatarTasks.h
*/
#ifndef PLAVATARTASKS_INC
#define PLAVATARTASKS_INC

// base class
#include "plAvTask.h"

#include "plAvDefs.h"

#include "hsQuat.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "pnKeyedObject/plKey.h"

class plAGAnim;
class plAGAnimInstance;
class plAvSeekMsg;
class plAvOneShotMsg;
class plAvEnterModeMsg;
class plOneShotCallbacks;
class plMessage;
class plAvPushBrainMsg;

/** \class plAvAnimTask
    Attach or detach an animation from the avatar.
    Optionally begin playing.
    It's perfectly reasonable to attach an animation with a blend of zero and start of false.
    This will have no effect visually until you start blending the animation in. */
class plAvAnimTask : public plAvTask
{
public:
    /** Default constructor for the class factory and subclases */
    plAvAnimTask();

    /** Canonical constructor form when attaching
        \param animName The name of the animation we'll be attaching or removing
        \param initialBlend Initial blend value; only relevant if we're attaching
        \param targetBlend The blend value we're shooting for
        \param fadeSpeed How fast (in fade units per second) to fade in
        \param setTime Local time for the animation to start
        \param start Start the animation, or just attach it?
        \param loop Make the animation loop?
        \param attach Are we attaching or detaching the animation?
    */
    plAvAnimTask(const ST::string &animName, float initialBlend, float targetBlend, float fadeSpeed,
                 float setTime, bool start, bool loop, bool attach);

    /** Canonical constructor form form for detaching
        \param animName The name of the animation we're detaching
        \param fadeSpeed How fast to fade it out. */
    plAvAnimTask(const ST::string &animName, float fadeSpeed, bool attach = false);

    // task protocol
    bool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    bool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    void LeaveAge(plArmatureMod *avatar) override;

    // plasma protocol
    CLASSNAME_REGISTER( plAvAnimTask );
    GETINTERFACE_ANY( plAvAnimTask, plAvTask );

    void Write(hsStream *stream, hsResMgr *mgr) override;
    void Read(hsStream *stream, hsResMgr *mgr) override;

protected:
    // public members
    ST::string fAnimName;               // the animation we're operating on
    float fInitialBlend;                // the blend to establish (attaching only)
    float fTargetBlend;                 // the blend to achieve eventually (attaching only)
    float fFadeSpeed;                   // how fast to achieve the blend
    float fSetTime;                     // set the animation to this time
    bool fStart;                      // start the animation playing? (attaching only)
    bool fLoop;                       // turn on looping? (attaching only)
    bool fAttach;                     // attach? (otherwise detach)
    plAGAnimInstance *fAnimInstance;    // the animation we're monitoring (detaching only)
};


/** \task plAvSeekTask
    Magically fly to a specific location and orientation. Not for use in final production scenes; mainly as a hack
    to fly through incomplete areas of a level.
    !!! Generally deprecated in favor of avBlendedSeekTask, a slightly less-gross-looking hack.
    \deprecated */
class plAvSeekTask : public plAvTask
{
public:
    /** Default constructor used by class factory and descendants. */
    plAvSeekTask();
    /** Common constructor. Seeks to the point at the normal speed. */
    plAvSeekTask(plKey target);
    /** Constructor form for calculated seek points: uses aligment type and (optionally) animation
        The animation is used if we're trying to align some future pose with a seek target. */
    plAvSeekTask(plKey target, plAvAlignment alignType, ST::string animName);

    // task protocol
    bool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    bool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    void LeaveAge(plArmatureMod *avatar) override;
    
    // plasma protocol
    CLASSNAME_REGISTER( plAvSeekTask );
    GETINTERFACE_ANY( plAvSeekTask, plAvTask );

    // *** implement reader and writer if needed for network propagation
protected:
    // -- specification members --
    ST::string                  fAnimName;          // the animation we're using, if any;
    plAvAlignment               fAlign;             // how to line up with the seek point
    double                      fDuration;          // the time we want the task to take
    plKey                       fTarget;            // the thing we're seeking towards

    // -- implementation members --
    plAGAnimInstance *          fAnimInstance;      // the animation we're using
    hsPoint3                    fTargetPosition;    // the position we're seeking
    hsQuat                      fTargetRotation;    // the orientation we're seeking
    double                      fTargetTime;        // the time we want to be done with the task (start + duration)
    bool                        fPhysicalAtStart;   // was the avatar physical before we started?
    bool                        fCleanup;           // last frame of processing
};

// ---- THE CUTOFF ----

/** \class plAvOneShotTask
    Makes the avatar play a single animation and then return to user control.
    Typically used in combination with a seek task to position the avatar
    at the proper start point first.
    Deprecated. Will be replaced by single-stage generic brains.
*/
class plAvOneShotTask : public plAvTask {
public:
    /** Put default values for all member variables. */
    void InitDefaults();
    
    /** Default constructor for the class factor and descendants. */
    plAvOneShotTask();

    /** Canonical constructor
        \param animName The name of the animation we're going to play
        \param drivable Unused. Allows the oneshot to be controlled by keyboard input
        \param reversable Unused. Allows the oneshot to be backed up by keyboard input
        \param callbacks A vector of callback messages to be sent at specific times during the animation
        */
    plAvOneShotTask(const ST::string &animName, bool drivable, bool reversible, plOneShotCallbacks *callbacks);
    /** Construct from a oneshot message.
        \param msg The message to copy our parameters from
        \param brain The brain to attach the task to.
        */
    plAvOneShotTask (plAvOneShotMsg *msg, plArmatureMod *avatar, plArmatureBrain *brain);
    virtual ~plAvOneShotTask();

    // task protocol
    bool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    bool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    void LeaveAge(plArmatureMod *avatar) override;
    
    void SetAnimName(const ST::string &name);
    
    static bool fForce3rdPerson;

    // plasma protocol
    CLASSNAME_REGISTER( plAvOneShotTask );
    GETINTERFACE_ANY( plAvOneShotTask, plAvTask );

    bool fBackwards;                  // play the anim backwards
    bool fDisableLooping;             // explicitly kill looping on this anim;    
    bool fDisablePhysics;             // disable physics when we play this oneshot
    
    // *** implement reader and writer if needed for network propagation
protected:
    ST::string fAnimName;             // the name of the one-shot animation we want to use
    bool fMoveHandle;                 // move the handle after the oneshot's done playing?
    plAGAnimInstance *fAnimInstance;    // the animation instance (available only after it starts playing)
    bool fDrivable;                   // the user can control the animation with the mouse
    bool fReversible;                 // the user can back up the animation with the mouse
    bool fEnablePhysicsAtEnd;         // was the avatar physical before we started (and did we disable physics?)
    bool fDetachAnimation;            // should we detach the animation when we're done?
    bool fIgnore;                     // if this gets set before we start, we just finish without doing anything.
    
    plOneShotCallbacks *fCallbacks;     // a set of callbacks to set up on our animation
    
    hsMatrix44 fPlayer2Anim;            // matrix from player root space to animation root space
    hsMatrix44 fAnim2Player;            // matrix from animation root space to player root space
    
    hsMatrix44 fWorld2Anim;
    hsMatrix44 fAnim2World;
    
    int fWaitFrames;                    // wait a couple frames before finalizing position to allow changes
    // from any animation callbacks to propagate. based on weird interaction
    // with Attach() command. sheeeit.
};

void GetPositionAndRotation(hsMatrix44 transform, hsScalarTriple *position, hsQuat *rotation);


// A quick task to play a oneshot, wait on a marker, and link to your personal age.
// Could be done with a responder, but this gives me a quick way to trigger it from code
// without the responder and callback messages flying around.
class plAvOneShotLinkTask : public plAvOneShotTask
{
public:
    plAvOneShotLinkTask();
    virtual ~plAvOneShotLinkTask();

    // task protocol
    bool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;
    bool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, float elapsed) override;

    CLASSNAME_REGISTER( plAvOneShotLinkTask );
    GETINTERFACE_ANY( plAvOneShotLinkTask, plAvOneShotTask );   

    // only read/writes enough to send an unstarted task across the net. Not intended for
    // use with a running task.
    void Write(hsStream *stream, hsResMgr *mgr) override;
    void Read(hsStream *stream, hsResMgr *mgr) override;

    void SetMarkerName(const ST::string &name);
        
protected:
    ST::string fMarkerName;
    double fStartTime;
    float fMarkerTime;
    bool fLinkFired;
};


#endif

