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
/** \file plArmatureMod.h
    A modifier which manages multi-channel animation and has a physical body.
    Designed for avatars; also good for vehicles, creatures, etc.
*/

// PLARMATUREMOD
//
// An armature is an object with both a physical presence (physics behavior) and articulated animated parts.
// (The parts are not themselves physical)
// An avatar is a type of armature, as is a critter, and anything else that moves around.
//
// This modifier combines multi-channel animation with blending (inherited from plAGMasterMod)
// with convenience functions for moving a physical body around and some specialized animation support

#ifndef plArmatureMod_inc
#define plArmatureMod_inc

#include "plAnimation/plAGMasterMod.h"

// other local
#include "plAvDefs.h"

#include "pnSceneObject/plSimulationInterface.h"

#include "hsMatrix44.h"
#include "plNetCommon/plNetCommon.h"

#include <cfloat>

/////////////////////////////////////////////////////////////////////////////////////////
//
// FORWARDS
//
/////////////////////////////////////////////////////////////////////////////////////////

class hsQuat;
class plMatrixChannel;
class plAGModifier;
class plAGMasterMod;
class plAGChannel;
class plClothingOutfit;
class plClothingSDLModifier;
class plAvatarSDLModifier;
class plAvatarPhysicalSDLModifier;
class plMatrixDelayedCorrectionApplicator;
class plMatrixDifferenceApp;
class plDebugText;
class plArmatureEffectsMgr;
class plAvBoneMap;      // below
class plDrawable;
class plControlEventMsg;
class plAvatarInputStateMsg;
class plLayerLinkAnimation;
class plPipeline;
class plArmatureBrain;
class plArmatureUpdateMsg;
class plPhysicalControllerCore;

typedef std::vector<plKey> plKeyVector;
typedef std::vector<plArmatureBrain*> plBrainStack;

class plArmatureModBase : public plAGMasterMod
{
public:
    plArmatureModBase();
    virtual ~plArmatureModBase();

    CLASSNAME_REGISTER( plArmatureModBase );
    GETINTERFACE_ANY( plArmatureModBase, plAGMasterMod );   
    
    virtual bool    MsgReceive(plMessage* msg);
    virtual void    AddTarget(plSceneObject* so);   
    virtual void    RemoveTarget(plSceneObject* so);
    virtual bool    IEval(double secs, float del, uint32_t dirty);
    virtual void    Read(hsStream *stream, hsResMgr *mgr);
    virtual void    Write(hsStream *stream, hsResMgr *mgr);
    
    plMatrixDifferenceApp *GetRootAnimator() { return fRootAnimator; }
    plPhysicalControllerCore* GetController() const { return fController; }
    plKey GetWorldKey() const;
    virtual bool ValidatePhysics();
    virtual bool ValidateMesh();
    virtual void PushBrain(plArmatureBrain *brain);
    virtual void PopBrain();
    plArmatureBrain *GetCurrentBrain() const;
    plDrawable *FindDrawable() const;
    virtual void LeaveAge();
    virtual bool IsFinal();

    // LOD stuff
    void    AdjustLOD();                // see if we need to switch to a different resolution
    bool    SetLOD(int newLOD);     // switch to a different resolution
    void    RefreshTree();              // Resend an LOD update to all our nodes (for when geometry changes)
    int     AppendMeshKey(plKey meshKey);
    int     AppendBoneVec(plKeyVector *boneVec);
    uint8_t   GetNumLOD() const;
    
    // A collection of reasons (flags) that things might be disabled. When all flags are gone
    // The object is re-enabled.
    enum
    {
        kDisableReasonUnknown   = 0x0001,
        kDisableReasonRelRegion = 0x0002,
        kDisableReasonLinking   = 0x0004,
        kDisableReasonCCR       = 0x0008,
        kDisableReasonVehicle   = 0x0010,
        kDisableReasonGenericBrain  = 0x0020,
        kDisableReasonKinematic = 0x0040
    };  
    void EnablePhysics(bool status, uint16_t reason = kDisableReasonUnknown);
    void EnablePhysicsKinematic(bool status);
    void EnableDrawing(bool status, uint16_t reason = kDisableReasonUnknown);   
    bool IsPhysicsEnabled() { return fDisabledPhysics == 0; }
    bool IsDrawEnabled() { return fDisabledDraw == 0; }


    static void AddressMessageToDescendants(const plCoordinateInterface * CI, plMessage *msg);
    static void EnableDrawingTree(const plSceneObject *object, bool status);  

    static int fMinLOD;                     // throttle for lowest-indexed LOD
    static double fLODDistance;             // Distance for first LOD switch 2nd is 2x this distance (for now)
    
protected:
    virtual void IFinalize();
    virtual void ICustomizeApplicator();
    void IEnableBones(int lod, bool enable);
        
    // Some of these flags are only needed by derived classes, but I just want
    // the one waitFlags variable.
    enum
    {
        kNeedMesh               = 0x01,
        kNeedPhysics            = 0x02,
        kNeedAudio              = 0x04,
        kNeedCamera             = 0x08,
        kNeedSpawn              = 0x10,
        kNeedApplicator         = 0x20,
        kNeedBrainActivation    = 0x40,
    };
    uint16_t fWaitFlags;  
    
    int fCurLOD;
    plPhysicalControllerCore* fController;
    plKeyVector fMeshKeys;
    plBrainStack fBrains;   
    plMatrixDifferenceApp *fRootAnimator;
    std::vector<plKeyVector*> fUnusedBones;
    uint16_t fDisabledPhysics;
    uint16_t fDisabledDraw;
};

class plArmatureMod : public plArmatureModBase
{
    friend class plHBehavior;
    friend class plAvatarSDLModifier;
    friend class plAvatarPhysicalSDLModifier;
    friend class plClothingSDLModifier;
    friend class plAvOneShotLinkTask;
    
public:
    plArmatureMod();
    virtual ~plArmatureMod();
    
    CLASSNAME_REGISTER( plArmatureMod );
    GETINTERFACE_ANY( plArmatureMod, plArmatureModBase );
    
    virtual bool    MsgReceive(plMessage* msg);
    virtual void    AddTarget(plSceneObject* so);
    virtual void    RemoveTarget(plSceneObject* so);
    virtual bool    IEval(double secs, float del, uint32_t dirty);
    virtual void    Read(hsStream *stream, hsResMgr *mgr);
    virtual void    Write(hsStream *stream, hsResMgr *mgr);

    virtual bool ValidatePhysics();
    virtual bool ValidateMesh();

    // Get or set the position of the avatar in simulation space.  Set any
    // arguments you don't care about to nil.
    void SetPositionAndRotationSim(const hsPoint3* position, const hsQuat* rotation);
    void GetPositionAndRotationSim(hsPoint3* position, hsQuat* rotation);

    bool IsLocalAvatar();
    bool IsLocalAI();
    virtual const plSceneObject *FindBone(const plString & name) const;
    virtual const plSceneObject *FindBone(uint32_t id) const; // use an id from an appropriate taxonomy, such as plAvBrainHuman::BoneID
    virtual void AddBoneMapping(uint32_t id, const plSceneObject *bone);
    plAGModifier *GetRootAGMod();
    plAGAnim *FindCustomAnim(const plString& baseName) const;

    virtual void Spawn(double timeNow);
    virtual void SpawnAt(int which, double timeNow);
    virtual void EnterAge(bool reSpawn);
    virtual void LeaveAge();
    virtual void PanicLink(bool playLinkOutAnim = true);
    virtual void PersonalLink();

    virtual bool ToggleDontPanicLinkFlag() { fDontPanicLink = fDontPanicLink ? false : true; return fDontPanicLink; }

    int GetBrainCount();
    plArmatureBrain *GetNextBrain(plArmatureBrain *brain);
    plArmatureBrain *GetBrain(int index) { if(index <= fBrains.size()) return fBrains.at(index); else return nil; }
    plArmatureBrain *FindBrainByClass(uint32_t classID) const;

    void TurnToPoint(hsPoint3 &point);
    void SuspendInput();
    void ResumeInput();
    
    uint8_t       IsInputSuspended() { return fSuspendInputCount; }
    void        IProcessQueuedInput();
    void        PreserveInputState();
    void        RestoreInputState();
    bool        GetInputFlag(int f) const;
    void        SetInputFlag(int which, bool status);
    void        ClearInputFlags(bool saveAlwaysRun, bool clearBackup);
    bool        HasMovementFlag() const; // Is any *movement* input flag on?
    float    GetTurnStrength() const;
    float    GetKeyTurnStrength() const;
    float    GetAnalogTurnStrength() const;
    void        SetReverseFBOnIdle(bool val);
    bool        IsFBReversed();

    bool ForwardKeyDown() const;
    bool BackwardKeyDown() const;
    bool StrafeLeftKeyDown() const;
    bool StrafeRightKeyDown() const;
    bool StrafeKeyDown() const;
    bool FastKeyDown() const;
    bool TurnLeftKeyDown() const;
    bool TurnRightKeyDown() const;
    bool JumpKeyDown() const;
    bool ExitModeKeyDown() const;
    void SetForwardKeyDown();
    void SetBackwardKeyDown();
    void SetStrafeLeftKeyDown(bool on = true);
    void SetStrafeRightKeyDown(bool on = true);
    void SetFastKeyDown();
    void SetTurnLeftKeyDown(bool status = true);
    void SetTurnRightKeyDown(bool status = true);
    void SetJumpKeyDown();
    void DebugDumpMoveKeys(int &x, int &y, int lineHeight, plDebugText &debugTxt);
    plString GetMoveKeyString() const;

    void SynchIfLocal(double timeNow, int force); // Just physical state
    void SynchInputState(uint32_t rcvID = kInvalidPlayerID);  
    bool DirtySynchState(const plString& SDLStateName, uint32_t synchFlags );
    bool DirtyPhysicalSynchState(uint32_t synchFlags);
    plClothingOutfit *GetClothingOutfit() const { return fClothingOutfit; }
    plClothingSDLModifier *GetClothingSDLMod() const { return fClothingSDLMod; }
    const plSceneObject *GetClothingSO(uint8_t lod) const;
    plArmatureEffectsMgr *GetArmatureEffects() const { return fEffects; }

    enum
    {
        kWalk,
        kRun,
        kTurn,
        kImpact,
        kSwim,
    };

    plString GetAnimRootName(const plString &name);
    int8_t AnimNameToIndex(const plString &name);
    void SetBodyType(int type) { fBodyType = type; }
    int  GetBodyType(int type) { return fBodyType; }
    int  GetCurrentGenericType();
    bool FindMatchingGenericBrain(const char *names[], int count);
    plString MakeAnimationName(const plString& baseName) const;
    plString GetRootName();
    void SetRootName(const plString &name);
    
    int  RefreshDebugDisplay();
    void DumpToDebugDisplay(int &x, int &y, int lineHeight, plDebugText &debugTxt);
    void SetDebugState(bool state) { fDebugOn = (state != 0); }
    bool GetDebugState() { return fDebugOn; }

    virtual void    RefreshTree() {}
    bool    IsInStealthMode() const;
    int     GetStealthLevel() const { return fStealthLevel; }

    bool    IsOpaque();
    bool    IsLinkedIn();
    bool    IsMidLink();
    bool    ConsumeJump(); // returns true if the jump keypress was available to consume

    void SendBehaviorNotify(uint32_t type, bool start = true) { IFireBehaviorNotify(type,start); }
    // Discovered a bug which makes these values horribly out of scale. So we do the rescale
    // in the Get/Set functions for backwards compatability.
    static void     SetMouseTurnSensitivity(float val) { fMouseTurnSensitivity = val / 150.f; }
    static float GetMouseTurnSensitivity() { return fMouseTurnSensitivity * 150.f; } 
    
    static void SetSpawnPointOverride(const plString &overrideObjName);
    static void WindowActivate(bool active);
    void SetFollowerParticleSystemSO(plSceneObject *follower);
    plSceneObject *GetFollowerParticleSystemSO();
    void RegisterForBehaviorNotify(plKey key);
    void UnRegisterForBehaviorNotify(plKey key);
    const hsBitVector& GetRelRegionCareAbout() const    { return fRegionsICareAbout; }
    const hsBitVector& GetRelRegionImIn() const         { return fRegionsImIn; }

    bool    IsKILowestLevel();
    int     GetKILevel();
    void    SetLinkInAnim(const plString &animName);
    plKey   GetLinkInAnimKey() const;

    enum
    {
        kSwapTargetShadow,
        kMaxSwapType
    };
    enum
    {
        kBoneBaseMale = 0,
        kBoneBaseFemale,
        kBoneBaseCritter, // AI controlled avatar
        kBoneBaseActor, // human controlled, non human avatar
        kMaxBoneBase
    };  
    enum
    {
        kAvatarLOSGround,
        kAvatarLOSSwimSurface,
    };
    plMatrixDelayedCorrectionApplicator *fBoneRootAnimator;

    static const float kAvatarInputSynchThreshold;
    static bool fClickToTurn;
    static const char *BoneStrings[];
    
    void SetPhysicalDims(float height, float width) { fPhysHeight = height; fPhysWidth = width; }

    void SetBodyAgeName(const plString& ageName) { fBodyAgeName = ageName; }
    void SetBodyFootstepSoundPage(const plString& pageName) { fBodyFootstepSoundPage = pageName; }
    void SetAnimationPrefix(const plString& prefix) { fAnimationPrefix = prefix; }

    plString GetUserStr() const { return fUserStr; }

protected:
    void IInitDefaults();
    virtual void IFinalize();   
    virtual void ICustomizeApplicator();
    virtual void ISetupMarkerCallbacks(plATCAnim *anim, plAnimTimeConvert *atc);
    
    void    NetworkSynch(double timeNow, int force = 0);
    bool    IHandleControlMsg(plControlEventMsg* pMsg);
    void    IFireBehaviorNotify(uint32_t type, bool behaviorStart = true);
    void    IHandleInputStateMsg(plAvatarInputStateMsg *msg);
    void    ILinkToPersonalAge();
    int     IFindSpawnOverride(void);
    void    ISetTransparentDrawOrder(bool val);
    plLayerLinkAnimation *IFindLayerLinkAnim();
    
    plString            fRootName;          // the name of the player root (from the max file)
    hsBitVector         fMoveFlags;         // which keys/buttons are currently pressed
    hsBitVector         fMoveFlagsBackup;   // a copy of fMoveFlags
    typedef std::vector<plControlEventMsg*> CtrlMessageVec;
    CtrlMessageVec      fQueuedCtrlMessages;    // input messages we haven't processed
    float               fMouseFrameTurnStrength; // Sum turnage from mouse delta messages since last eval.
    plKey               fFootSoundSOKey;    // The Scene Object we attach to targets for footstep sounds
    plKey               fLinkSoundSOKey;    // Same thing for linking... wwwwawAWAWAwawa...
    plKey               fLinkInAnimKey;     // Set when we link out, this is the anim to play (backwards) when we link in.
    static float        fMouseTurnSensitivity;
    plArmatureUpdateMsg *fUpdateMsg;    
    
    // Trying to be a good lad here and align all our bools and bytes...
    bool fIsLinkedIn;       // We have finished playing the LinkEffects and are properly in the age
    bool fMidLink;          // We're in between a LeaveAge and an EnterAge
    bool fAlreadyPanicLinking;  // Cleared when you enter an age. Prevents spamming the server with panic link requests.
    bool fUnconsumedJump;   // We've pressed the jump key, but haven't jumped yet
    bool fReverseFBOnIdle;  // see set/getters for comments
    bool fPendingSynch;
    bool fDebugOn;
    bool fOpaque;
    uint8_t fSuspendInputCount;   
    uint8_t fStealthMode;
    int fStealthLevel;  // you are invisible to other players/CCRs of lower stealthLevel
    
    plAGModifier * fRootAGMod;
    plAvBoneMap * fBoneMap;                 // uses id codes to look up bones. set up by the brain as needed.
    double fLastSynch;
    int fBodyType;
    plClothingOutfit *fClothingOutfit;
    plClothingSDLModifier *fClothingSDLMod;
    plAvatarSDLModifier *fAvatarSDLMod;
    plAvatarPhysicalSDLModifier *fAvatarPhysicalSDLMod;
    hsTArray<const plSceneObject*> fClothToSOMap;
    plArmatureEffectsMgr *fEffects;
    plSceneObject *fFollowerParticleSystemSO;
    static plString fSpawnPointOverride;

    // These vectors are used with relevance regions for culling out other objects
    hsBitVector fRegionsImIn;
    hsBitVector fRegionsICareAbout;
    hsBitVector fOldRegionsImIn;
    hsBitVector fOldRegionsICareAbout;
    
    hsTArray<plKey> fNotifyKeys;

    // Extra info for creating our special physical at runtime
    float fPhysHeight;
    float fPhysWidth;

    bool fDontPanicLink;

    // strings for animations, age names, footstep sounds, etc
    plString fBodyAgeName;
    plString fBodyFootstepSoundPage;
    plString fAnimationPrefix;

    // user-defined string assigned to this avatar
    plString fUserStr;
};

// PLARMATURELOD
// This class has been phased into plArmatureModBase. It's left behind
// for backwards compatability. 
class plArmatureLODMod : public plArmatureMod
{
public:
    // tors
    plArmatureLODMod();
    plArmatureLODMod(const plString & root_name);
    virtual ~plArmatureLODMod();

    CLASSNAME_REGISTER( plArmatureLODMod );
    GETINTERFACE_ANY( plArmatureLODMod, plArmatureMod );

    virtual void    Read(hsStream *stream, hsResMgr *mgr);
    virtual void    Write(hsStream *stream, hsResMgr *mgr);
};

class plAvBoneMap
{
public:
    plAvBoneMap();
    virtual ~plAvBoneMap();

    const plSceneObject * FindBone(uint32_t boneID);          // you probably want to use plAvBrainHuman::BoneID;
    void AddBoneMapping(uint32_t boneID, const plSceneObject *SO);

protected:
    class BoneMapImp;       // forward declaration to keep the header clean: see .cpp for implementation
    BoneMapImp *fImp;       // the thing that actually holds our map
};

#define TWO_PI (M_PI * 2)

#endif //plArmatureMod_inc
