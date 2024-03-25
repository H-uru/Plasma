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
/** \file plAGAnimInstance.h
    \brief The animation class for the AniGraph animation system

    \ingroup Avatar
    \ingroup AniGraph
*/

#ifndef PLAGANIMINSTANCE_INC
#define PLAGANIMINSTANCE_INC

#include "HeadSpin.h"
#include <map>

// local
#include "plScalarChannel.h"

// declarations
class plAGChannel;
class plAGAnim;
class plAGMasterMod;
class plAGChannelApplicator;
class plAnimCmdMsg;
class plAnimTimeConvert;
class plATCAnim;
class plOneShotCallbacks;

/////////////////
// PLAGANIMINSTANCE
/////////////////
/** \class plAGAnimInstance
    Whenever we attach an animation to a scene object hierarchy, we
    create an activation record -- a plAGAnimInstance -- that remembers
    all the ephemeral state associated with animation 
    Since animations have many channels and may involve blend operations,
    one of the primary responsibilities of this class is to keep track of
    all the animation node graphs that were created by the invocation of
    this animation.
    */
class plAGAnimInstance {
public:
    /** Used for the fade commands to select what to fade. */
    enum
    {
        kFadeBlend,     /// Fade the blend strength
        kFadeAmp,       /// Fade the amplitude
    } FadeType;

    /** Default constructor. */
    plAGAnimInstance();

    /** Construct from an animation and a master modifier.
        This attaches the animation channels to the channels of
        the master modifier and creates all the bookkeeping structures
        necessary to undo it later. */
    plAGAnimInstance(plAGAnim * anim, plAGMasterMod * master, float blend, uint16_t blendPriority, bool cache, bool useAmplitude);

    /** Destructor. Removes the animation from the scene objects it's attached to. */
    virtual ~plAGAnimInstance();

    /** Returns the animation that this instance mediates. */
    const plAGAnim * GetAnimation() { return fAnimation; };

    /** Returns the timeconvert object that controls the progress of time
        in this animation. */
    plAnimTimeConvert *GetTimeConvert() { return fTimeConvert; }

    /** Set the speed of the animation. This is expressed as a fraction of
        the speed with which the animation was defined. */
    void SetSpeed(float speed);

    // \{
    /**
        The current blend factor of the animation. This indicates the
        priority of this animation as opposed to other animations which
        were attached before it. Conceptually it may help to think of this
        as a layer in an stack of animations, where the blend value is the
        'opacity' of this animation relative to the ones below it.
        1.0 represents full strength.
        You may use values higher than 1.0, but this has not
        yet been seen to have any practical utility whatsoever. Note that
        even if an animation has a blend strength of 1.0, it may have another
        animation on top/downstream from it that is masking it completely. */
    float SetBlend(float blend);
    float GetBlend();
    // \}

    /** Set the strength of the animation with respect to its 0th frame.
        This can be used to dampen the motion of the animation.
        Animations must be designed to use this: frame 0 of the animation
        must be a reasonable "default pose" as it will be blended with the
        current frame of the animation to produce the result. */
    float SetAmplitude(float amp);
    /** Get the current strength of the animation. */
    float GetAmplitude();

    /** Make this animation loop (or not.) Note that the instance can loop
        or not without regard to whether the plAGAnim it is based on loops. */
    void SetLoop(bool status);

    /** Interpret and respond to an animation command message. /sa plAnimCmdMsg */
    bool HandleCmd(plAnimCmdMsg *msg);

    /** Start playback of the animation. You may optionally provide the a world
        time, which is needed for synchronizing the animation's timeline
        with the global timeline. If timeNow is -1 (the default,) the system
        time will be polled */
    void Start(double worldTimeNow = -1);
    
    /** Stop playback of the animation. */
    void Stop();

    /** Move the playback head of the animation to a specific time.
        Note that this time is in animation local time, not global time.
        The "jump" parameter specifies whether or not to fire callbacks
        that occur between the current time and the target time. */
    void SetCurrentTime(float newLocalTime, bool jump = false);

    /** Move the playback head by the specified relative amount within 
        the animation. This may cause looping. If the beginning or end
        of the animation is reached an looping is not on, the movement
        will pin.
        \param jump if true, don't look for callbacks between old time and TRACKED_NEW */
    void SeekRelative(float delta, bool jump);
    
    /** Gradually fade the blend strength or amplitude of the animation.
        \param goal is the desired blend strength
        \param rate is in blend units per second
        \type is either kFadeBlend or kFadeAmp */
    void Fade(float goal, float rate, uint8_t type = kFadeBlend);

    /** Fade the animation and detach it after the fade is complete.
        Extremely useful for situations where the controlling logic
        is terminating immediately but you want the animation to fade
        out gradually.
        \deprecated 
    */
    void FadeAndDetach(float goal, float rate);

    /** Has the animation terminated of natural causes?
        Primarily used to see if an animation has played all the 
        way to the end, but will also return true if the animation
        was stopped with a stop command */
    bool IsFinished();
    
    /** Is the animation playback head positioned at the end. */
    bool IsAtEnd();

    /** Get the name of the underlying animation. */
    ST::string GetName();

    /** Remove all channels from the master mode and remove us from
        our master modifier.
        Destructs the object! */
    void Detach();

    /** Remove all the instance's channels from the modifiers they're attached to.
        Typically called by the master mod prior to destructing the instance. */
    void DetachChannels();

    /** Prune any unused branches out of the animation graph; add any 
        newly active branches back in. */
    void Optimize();

    /** Convert the given world time to local animation time.
        May include the effects of looping or wraparound.
        If the local time passes the end of the animation, the returned
        time will be pinned appropriately. */
    double WorldToAnimTime(double foo);

    /** Attach a sequence of callback messages to the animation instance.
        Messages are each associated with a specific (local) time
        in the animation and will be sent when playback passes that time. */
    void AttachCallbacks(plOneShotCallbacks *callbacks);
    
    void ProcessFade(float elapsed);             // process any outstanding fades    
    void SearchForGlobals(); // Util function to setup SDL channels
protected:
    /** Set up bookkeeping for a fade. */
    void ISetupFade(float goal, float rate, bool detach, uint8_t type);

    void IRegisterDetach(const ST::string &channelName, plAGChannel *channel);

    void IInitAnimTimeConvert(plAnimTimeConvert* atc, plATCAnim* anim, plAGMasterMod* master);

    const plAGAnim * fAnimation;
    plAGMasterMod * fMaster;

    std::map<ST::string, plAGChannelApplicator *, ST::less_i> fChannels;

    typedef std::multimap<ST::string, plAGChannel *> plDetachMap;
    plDetachMap fManualDetachChannels;

    std::vector<plAGChannel*> fCleanupChannels;
    std::vector<plScalarSDLChannel*> fSDLChannels;

    plScalarConstant fBlend;        // blend factor vs. previous animations
    plScalarConstant fAmplitude;    // for animation scaling

    // Each activation gets its own timeline.
    plAnimTimeConvert       *fTimeConvert;

    bool                fFadeBlend;         /// we are fading the blend
    float            fFadeBlendGoal;     /// what blend level we're trying to reach
    float            fFadeBlendRate;     /// how fast are we fading in blend units per second (1 blend unit = full)
    bool                fFadeDetach;        /// detach after fade is finished? (only used for blend fades)
    
    bool                fFadeAmp;           /// we are fading the amplitude
    float            fFadeAmpGoal;       /// amplitude we're trying to reach 
    float            fFadeAmpRate;       /// how faster we're fading in blend units per second

    float ICalcFade(bool &fade, float curVal, float goal, float rate, float elapsed);

};

//#ifdef HS_DEBUGGING
//#define TRACK_AG_ALLOCS       // for now, automatically track AG allocations in debug
//#endif
#ifdef TRACK_AG_ALLOCS

extern ST::string gGlobalAnimName;
extern ST::string gGlobalChannelName;

void RegisterAGAlloc(plAGChannel *object, const char *chanName, const char *animName, uint16_t classIndex);
void UnRegisterAGAlloc(plAGChannel *object);
void DumpAGAllocs();

#endif // TRACK_AG_ALLOCS

#endif // PLAGANIMINSTANCE_INC
