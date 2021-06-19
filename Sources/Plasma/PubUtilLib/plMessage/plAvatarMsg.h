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

#ifndef plAvatarMsg_inc
#define plAvatarMsg_inc

#include "hsBitVector.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "plAvatar/plAvDefs.h"

class plSceneObject;
class hsStream;
class hsResMgr;
class plAvTask;
class plKey;
class plArmatureMod;
class plArmatureModBase;
class plArmatureBrain;

/** \Class plAvatarMsg
    Abstract base class for messages to and from the avatar.
    */
class plAvatarMsg : public plMessage
{
public:
    // tors
    plAvatarMsg() : plMessage() { }
    plAvatarMsg(const plKey &sender, const plKey &receiver)
        : plMessage(sender, receiver, nullptr) { }


    // plasma protocol
    CLASSNAME_REGISTER( plAvatarMsg );
    GETINTERFACE_ANY( plAvatarMsg, plMessage );

    void Read(hsStream *stream, hsResMgr *mgr) override { plMessage::IMsgRead(stream, mgr); }
    void Write(hsStream *stream, hsResMgr *mgr) override { plMessage::IMsgWrite(stream, mgr); }
};


/** \Class plArmatureUpdateMsg
    Sent every frame by all armatures. Currently only sent by humans,
    mainly because currently that's all we have. Non-humans may send
    less often.
    */
class plArmatureUpdateMsg : public plAvatarMsg
{
public:
    plArmatureUpdateMsg();
    plArmatureUpdateMsg(const plKey &sender,
                        bool isLocal, bool isPlayerControlled,
                        plArmatureMod *armature);

    /** The avatar that sent this message is the local avatar for this client. */
    bool IsLocal() const { return fIsLocal; }
    void SetIsLocal(bool on) { fIsLocal = on; }
    /** The avatar that sent this message is controlled by a human being -- although
        not necessarily a local human being. */
    bool IsPlayerControlled() const { return fIsPlayerControlled; }
    void SetIsPlayerControlled(bool on) { fIsPlayerControlled = on; }
    bool IsInvis() const { return fIsInvis; }
    void SetInvis(bool val) { fIsInvis = val; }

    // plasma protocol
    CLASSNAME_REGISTER( plArmatureUpdateMsg );
    GETINTERFACE_ANY( plArmatureUpdateMsg, plAvatarMsg );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;

    plArmatureMod * fArmature;  // the armature that sent this message
                                // valid during the message's lifetime

protected:
    // these will probably change to enums + bitmasks .. don't count on the representation
    bool fIsLocal;
    bool fIsPlayerControlled;
    bool fIsInvis; // Avatar is invis. Don't update visable effects.
};

// use this to turn an npc into a player and vice-versa
class plAvatarSetTypeMsg : public plAvatarMsg
{
public:
    plAvatarSetTypeMsg();
    plAvatarSetTypeMsg(const plKey &sender, const plKey &receiver);

    // theoretically we will someday achieve a broader taxonomy
    void SetIsPlayer(bool is) { fIsPlayer = is; }
    bool IsPlayer() const { return fIsPlayer; }

    CLASSNAME_REGISTER(plAvatarSetTypeMsg);
    GETINTERFACE_ANY(plAvatarSetTypeMsg, plAvatarMsg);

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
    
private:
    bool fIsPlayer;
};

/** PLAVTASKMSG
    An abstract message class for Avatar Tasks -- things which may have multiple steps
    and which may be conducted over an extended period of time.
    There is currently no provision for tasks which may be executed in parallel.
    Activities which are parallelizable are handled by Behaviors
    */
class plAvTaskMsg : public plAvatarMsg
{
public:
    // tors
    plAvTaskMsg();
    plAvTaskMsg(const plKey &sender, const plKey &receiver);
    plAvTaskMsg(const plKey &sender, const plKey &receiver, plAvTask *task);

    plAvTask *GetTask() const { return fTask; }

    // plasma protocol
    CLASSNAME_REGISTER( plAvTaskMsg );
    GETINTERFACE_ANY( plAvTaskMsg, plAvatarMsg );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
private:
    plAvTask *fTask;
};

/** \Class plAvSeekMsg
    Tell the avatar to go to a specific location and assume
    a specific orientation.
    If smartSeek is true, the avatar will walk to the given spot.
    otherwise they will float magically through all intervening
    air and geometry.
    Duration is only meaningful for the latter case, when the avatar
    is floating - it determines the speed of float (i.e. reach the
    particular destination in this amount of time, regardless of distance.)
    Smartseek is definitely the preferred method; dumb seek is there
    for when the level geometry hasn't been quite worked out.
*/
class plAvSeekMsg : public plAvTaskMsg
{
public:
    enum 
    {
        kSeekFlagUnForce3rdPersonOnFinish   = 0x01,
        kSeekFlagForce3rdPersonOnStart      = 0x02,
        kSeekFlagNoWarpOnTimeout            = 0x04,
        kSeekFlagRotationOnly               = 0x08,
    };
    
    // tors
    plAvSeekMsg();
    plAvSeekMsg(const plKey& sender, const plKey& receiver, plKey seekKey, float duration, bool smartSeek,
                plAvAlignment align = kAlignHandle, const ST::string& animName = {}, bool noSeek = false,
                uint8_t flags = kSeekFlagForce3rdPersonOnStart, plKey finishKey = {});
    
    // plasma protocol
    CLASSNAME_REGISTER( plAvSeekMsg );
    GETINTERFACE_ANY( plAvSeekMsg, plAvTaskMsg );
    
    bool Force3rdPersonOnStart();
    bool UnForce3rdPersonOnFinish();
    bool NoWarpOnTimeout();
    bool RotationOnly();
    plKey GetFinishCallbackKey() { return fFinishKey; }
    
    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
    
    // public members
    plKey fSeekPoint;           // the key to the seekpoint we are going to find
    hsPoint3 fTargetPos;        // Or we specify the point/lookat explicitly
    hsPoint3 fTargetLookAt;
    float fDuration;            // take this much time to do the move (only if smartSeek is false)
    bool fSmartSeek;          // seek by walking rather than floating
    bool fNoSeek;
    ST::string fAnimName;
    plAvAlignment fAlignType;
    uint8_t fFlags;
    plKey fFinishKey;
    plMessage* fFinishMsg;
};

class plAvTaskSeekDoneMsg : public plAvatarMsg
{
public:
    bool fAborted;
    
    plAvTaskSeekDoneMsg() : plAvatarMsg(), fAborted(false) {}
    plAvTaskSeekDoneMsg(const plKey &sender, const plKey &receiver) : plAvatarMsg(sender, receiver), fAborted(false) {}

    // plasma protocol
    CLASSNAME_REGISTER( plAvTaskSeekDoneMsg );
    GETINTERFACE_ANY( plAvTaskSeekDoneMsg, plAvatarMsg );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

class plOneShotCallbacks;

/** \Class plAvOneShotMsg
    */
class plAvOneShotMsg : public plAvSeekMsg
{
public:

    // tors
    plAvOneShotMsg();
    virtual ~plAvOneShotMsg();
    plAvOneShotMsg(const plKey &sender, const plKey& receiver,
                   const plKey& seekKey, float duration, bool fSmartSeek,
                   const ST::string &animName, bool drivable, bool reversible);

    // plasma protocol
    CLASSNAME_REGISTER( plAvOneShotMsg );
    GETINTERFACE_ANY( plAvOneShotMsg, plAvSeekMsg );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;

    // public members
    bool fDrivable;               // are we animated by time or by mouse movement?
    bool fReversible;             // can we play backwards?
    plOneShotCallbacks *fCallbacks; // Callbacks given to us by a one-shot modifier
                                    // we share it, so release with UnRef
};

////////////////
//
// PLAVBRAINGENERICMSG
//
////////////////
class plAvBrainGenericMsg : public plAvatarMsg
{
public:

    // have to do members first for recursive definition
    // don't bother with encapsulation
    enum Type { kNextStage,
                kPrevStage,
                kGotoStage,
                kSetLoopCount
                }
        fType;
    int fWhichStage;                // used only by goto stage
    float fTransitionTime;       // for crossfade between stages
    bool fSetTime;
    float fNewTime;
    bool fSetDirection;
    bool fNewDirection;
    int fNewLoopCount;
    // tors
    plAvBrainGenericMsg();

    //! Older constructor version, allowing simple rewinding only
    plAvBrainGenericMsg(const plKey& sender, const plKey &receiver,
                        Type type, int stage, bool rewind, float transitionTime);

    /** Canonical constructor, allowing full control over time and direction of new stage.
        \param sender Message sender
        \param reciever Message receiver
        \param type The "verb" for the command - next stage, previous stage, goto stage
        \param stage The stage we're going to, if this is a goto command
        \param setTime Do we want to manually set the time on the target stage?
        \param newTime If setTime is true, this is the new (local) time used in the target stage
        \param setDirection Do we want to set the overall brain direction?
        \param isForward If setDirection is true, then true = forward, false = backward
        \param transitionTime Time in seconds to transition between stages.
    */  
    plAvBrainGenericMsg(const plKey& sender, const plKey &receiver,
                        Type type, int stage, bool setTime, float newTime,
                        bool setDirection, bool isForward, float transitiontime);
    
    /** Constructor for setting the loop count in a particular stage.
        \param sender The sender of this message.
        \param receiver. The (key to the) avatar brain this message is going to.
        \param type Must be kSetLoopCount
        \param stage Which stage are we setting the loop count for?
        \param newLoopCount The loop count we are setting on the stage
    */
    plAvBrainGenericMsg(const plKey& sender, const plKey& receiver,
                        Type type, int stage, int newLoopCount);
    // plasma protocol
    CLASSNAME_REGISTER( plAvBrainGenericMsg );
    GETINTERFACE_ANY( plAvBrainGenericMsg, plAvatarMsg );

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
    
    // WriteVersion writes the current version of this creatable and ReadVersion will read in
    // any previous version.
    void ReadVersion(hsStream* s, hsResMgr* mgr) override;
    void WriteVersion(hsStream* s, hsResMgr* mgr) override;
};

///////////////////
//
// PLAVPUSHBRAINMSG
//
///////////////////

class plAvPushBrainMsg : public plAvTaskMsg
{
public:
    // tors
    plAvPushBrainMsg();
    plAvPushBrainMsg(const plKey& sender, const plKey &receiver, plArmatureBrain *brain);

    CLASSNAME_REGISTER( plAvPushBrainMsg );
    GETINTERFACE_ANY( plAvPushBrainMsg, plAvTaskMsg);

    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;

    plArmatureBrain *fBrain;
};

//////////////////
//
// PLAVPOPBRAINMSG
//
//////////////////
class plAvPopBrainMsg : public plAvTaskMsg
{
public:
    // tors
    plAvPopBrainMsg() { }
    plAvPopBrainMsg(const plKey &sender, const plKey &receiver)
        : plAvTaskMsg(sender, receiver) { }

    CLASSNAME_REGISTER( plAvPopBrainMsg );
    GETINTERFACE_ANY( plAvPopBrainMsg, plAvTaskMsg);
};


// For entering/exiting "stealth mode"
class plAvatarStealthModeMsg : public plAvatarMsg
{
public:
    plAvatarStealthModeMsg();

    // modes
    enum
    {
        kStealthVisible,
        kStealthCloaked,
        kStealthCloakedButSeen,
    };
    uint8_t fMode;
    int fLevel;     // you are invisible to other players/CCRs of lower level
    
    CLASSNAME_REGISTER(plAvatarStealthModeMsg);
    GETINTERFACE_ANY(plAvatarStealthModeMsg, plAvatarMsg);
    
    void Read(hsStream *stream, hsResMgr *mgr) override;
    void Write(hsStream *stream, hsResMgr *mgr) override;
};

class plAvatarBehaviorNotifyMsg : public plMessage
{
public:
    uint32_t fType;
    bool state;
    
    plAvatarBehaviorNotifyMsg() : fType(0),state(false) {}
    
    CLASSNAME_REGISTER( plAvatarBehaviorNotifyMsg );
    GETINTERFACE_ANY( plAvatarBehaviorNotifyMsg, plMessage );
    
    // Local Only
    void Read(hsStream *stream, hsResMgr *mgr) override { }
    void Write(hsStream *stream, hsResMgr *mgr) override { }
};

class plAvatarOpacityCallbackMsg : public plEventCallbackMsg
{
public:
    plAvatarOpacityCallbackMsg() : plEventCallbackMsg() {}
    plAvatarOpacityCallbackMsg(const plKey& receiver, CallbackEvent e, int idx=0, float t=0, int16_t repeats=-1, uint16_t user=0) :
                               plEventCallbackMsg(receiver, e, idx, t, repeats, user) {}
    
    CLASSNAME_REGISTER( plAvatarOpacityCallbackMsg );
    GETINTERFACE_ANY( plAvatarOpacityCallbackMsg, plEventCallbackMsg );
    
    // These aren't meant to go across the net, so no IO necessary.
    void Read(hsStream* stream, hsResMgr* mgr) override { }
    void Write(hsStream* stream, hsResMgr* mgr) override { }
};

class plAvatarSpawnNotifyMsg : public plMessage
{
public:
    plArmatureMod *fAvMod;

    plAvatarSpawnNotifyMsg() : fAvMod() { }

    CLASSNAME_REGISTER( plAvatarSpawnNotifyMsg );
    GETINTERFACE_ANY( plAvatarSpawnNotifyMsg, plMessage );

    // Local Only
    void Read(hsStream *stream, hsResMgr *mgr) override { }
    void Write(hsStream *stream, hsResMgr *mgr) override { }
};

class plAvatarPhysicsEnableCallbackMsg : public plEventCallbackMsg
{
public:
    plAvatarPhysicsEnableCallbackMsg() : plEventCallbackMsg() {}
    plAvatarPhysicsEnableCallbackMsg(const plKey& receiver, CallbackEvent e, int idx=0, float t=0, int16_t repeats=-1, uint16_t user=0) :
                                     plEventCallbackMsg(receiver, e, idx, t, repeats, user) {}
    
    CLASSNAME_REGISTER( plAvatarPhysicsEnableCallbackMsg );
    GETINTERFACE_ANY( plAvatarPhysicsEnableCallbackMsg, plEventCallbackMsg );
    
    // These aren't meant to go across the net, so no IO necessary.
    void Read(hsStream* stream, hsResMgr* mgr) override { }
    void Write(hsStream* stream, hsResMgr* mgr) override { }
};

#endif // plAvatarMsg_inc
