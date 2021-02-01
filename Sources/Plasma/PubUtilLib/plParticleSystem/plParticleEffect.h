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
#ifndef plParticleEffect_inc
#define plParticleEffect_inc

#include "pnKeyedObject/hsKeyedObject.h"
#include "hsMatrix44.h"

class plEffectTargetInfo;
class plConvexVolume;
class hsResMgr;
class plSceneObject;

class plParticleEffect : public hsKeyedObject
{
public:
    CLASSNAME_REGISTER( plParticleEffect );
    GETINTERFACE_ANY( plParticleEffect, hsKeyedObject );

    // Order is:
    //  PrepareEffect is called with a given target (including valid
    //      ParticleContext).
    //  ApplyEffect is called some once for each particle (maybe zero times). 
    //      It can return true to kill a particle. 
    //      Target and Context passed in with Prepare will be
    //      guaranteed to remain valid until,
    //  EndEffect marks no more particles will be processed with the above
    //      context (invalidating anything cached).
    // Defaults for Prepare and End are no-ops.
    virtual void PrepareEffect(const plEffectTargetInfo& target) {}
    virtual bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) = 0;
    virtual void EndEffect(const plEffectTargetInfo& target) {}
};

class plParticleCollisionEffect : public plParticleEffect
{
public:
    plParticleCollisionEffect();
    ~plParticleCollisionEffect();

    CLASSNAME_REGISTER( plParticleCollisionEffect );
    GETINTERFACE_ANY( plParticleCollisionEffect, plParticleEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;
    bool MsgReceive(plMessage *msg) override;

protected:
    plSceneObject *fSceneObj;
    plConvexVolume *fBounds;
};

// Default particle blocker. Doesn't affect particle's velocity,
// so it'll keep "beat"ing on the deflector until the velocity
// dotted with the deflector normal slides it off an edge.
class plParticleCollisionEffectBeat : public plParticleCollisionEffect
{
public:
    plParticleCollisionEffectBeat();

    CLASSNAME_REGISTER( plParticleCollisionEffectBeat );
    GETINTERFACE_ANY( plParticleCollisionEffectBeat, plParticleCollisionEffect );

    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;
};

// This particle blocker just kills any particles that hit it.
class plParticleCollisionEffectDie : public plParticleCollisionEffect
{
public:
    plParticleCollisionEffectDie();

    CLASSNAME_REGISTER( plParticleCollisionEffectDie );
    GETINTERFACE_ANY( plParticleCollisionEffectDie, plParticleCollisionEffect );

    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;
};

class plParticleCollisionEffectBounce : public plParticleCollisionEffect
{
protected:
    float    fBounce;
    float    fFriction;
public:
    plParticleCollisionEffectBounce();

    CLASSNAME_REGISTER( plParticleCollisionEffectBounce );
    GETINTERFACE_ANY( plParticleCollisionEffectBounce, plParticleCollisionEffect );

    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

    void SetBounce(float b) { fBounce = b; }
    float GetBounce() const { return fBounce; }

    void SetFriction(float f) { fFriction = f; }
    float GetFriction() const { return fFriction; }
};

class plParticleFadeVolumeEffect : public plParticleEffect
{
protected:
    // Some cached properties. These will be the same for all
    // particles between matching PrepareEffect and EndEffect calls.
    hsPoint3    fMax;
    hsPoint3    fMin;
    hsPoint3    fNorm;

public:
    plParticleFadeVolumeEffect();
    ~plParticleFadeVolumeEffect();

    CLASSNAME_REGISTER( plParticleFadeVolumeEffect );
    GETINTERFACE_ANY( plParticleFadeVolumeEffect, plParticleEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;
    //bool MsgReceive(plMessage *msg) override;

    float fLength;
    bool fIgnoreZ;
};

class plParticleWindEffect : public plParticleEffect
{
protected:
    // The properties that define the wind. These might/should be animatable.
    float    fStrength;
    float    fConstancy;
    float    fSwirl;
    bool        fHorizontal;
    hsVector3   fRefDir;

    // Some cached properties. These will be the same for all
    // particles between matching PrepareEffect and EndEffect calls.
    // These define the current state of the wind.
    hsVector3   fWindVec;
    hsVector3   fRandDir;
    hsVector3   fDir;
    double      fLastDirSecs;
public:

    plParticleWindEffect()
        : fWindVec(), fDir(1.f, 0.f, 0.f), fSwirl(0.1f), fConstancy(),
          fHorizontal(), fLastDirSecs(-1.f), fRefDir(),
          fRandDir(1.f, 0.f, 0.f), fStrength()
    { }

    CLASSNAME_REGISTER( plParticleWindEffect );
    GETINTERFACE_ANY( plParticleWindEffect, plParticleEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override = 0;

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

    void                SetStrength(float v) { fStrength = v; }
    float            GetStrength() const { return fStrength; }

    void                SetConstancy(float c) { fConstancy = c; }
    float            GetConstancy() const { return fConstancy; }

    void                SetSwirl(float s) { fSwirl = s; }
    float            GetSwirl() const { return fSwirl; }

    void                SetHorizontal(bool on) { fHorizontal = on; }
    bool                GetHorizontal() const { return fHorizontal; }

    void                SetRefDirection(const hsVector3& v);
    const hsVector3&    GetRefDirection() const { return fRefDir; }
};

class plParticleLocalWind : public plParticleWindEffect
{
protected:
    hsVector3   fScale;
    float       fSpeed;

    hsVector3   fPhase;
    hsVector3   fInvScale;
    double      fLastPhaseSecs;
    
public:
    plParticleLocalWind()
        : fScale(),
          fSpeed(),
          fPhase(),
          fInvScale(),
          fLastPhaseSecs(-1.f)
    { }

    CLASSNAME_REGISTER( plParticleLocalWind );
    GETINTERFACE_ANY( plParticleLocalWind, plParticleWindEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;

    void                SetScale(const hsVector3& v) { fScale = v; }
    const hsVector3&    GetScale() const { return fScale; }

    void                SetSpeed(float v) { fSpeed = v; }
    float            GetSpeed() const { return fSpeed; }

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

};

class plParticleUniformWind : public plParticleWindEffect
{
protected:

    float    fFreqMin;
    float    fFreqMax;
    float    fFreqCurr;
    float    fFreqRate;
    double      fCurrPhase;

    double      fLastFreqSecs;

    float    fCurrentStrength;
public:
    plParticleUniformWind();
    ~plParticleUniformWind();

    CLASSNAME_REGISTER( plParticleUniformWind );
    GETINTERFACE_ANY( plParticleUniformWind, plParticleWindEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;

    void        SetFrequencyRange(float minSecsPerCycle, float maxSecsPerCycle);
    void        SetFrequencyRate(float secsPerCycle);

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;

};

class plParticleInfluenceInfo
{
public:
    hsVector3 fAvgVel;
    hsVector3 fRepDir;
};

class plParticleFlockEffect : public plParticleEffect
{
//protected:
protected:
    hsPoint3 fTargetOffset;     // Worldspace offset from our target to get the true goal
    hsPoint3 fDissenterTarget;  // Where to particles go when they get scared and leave us?
    float fInfAvgRadSq;      // Square of the radius of influence for average velocity matching.
    float fInfRepRadSq;      // Same, for repelling from neighbors.
    float fAvgVelStr;        // How strongly are we influenced by average dir?
    float fRepDirStr;        // Same for repelling
    float fGoalOrbitStr;     // Same for the goal (when we're within the desired distance)
    float fGoalChaseStr;     // Same for the goal (when we're too far away, and chasing it)
    float fGoalDistSq;
    float fFullChaseDistSq;
    float fMaxOrbitSpeed;
    float fMaxChaseSpeed;

    uint16_t fMaxParticles;
    float *fDistSq;          // Table of distances from particle to particle
    plParticleInfluenceInfo *fInfluences; 

    void IUpdateDistances(const plEffectTargetInfo &target);
    void IUpdateInfluences(const plEffectTargetInfo &target);

public:
    plParticleFlockEffect()
        : fInfAvgRadSq(1.f),
          fInfRepRadSq(1.f),
          fAvgVelStr(1.f),
          fRepDirStr(1.f),
          fGoalOrbitStr(1.f),
          fGoalChaseStr(1.f),
          fGoalDistSq(1.f),
          fFullChaseDistSq(1.f),
          fMaxOrbitSpeed(1.f),
          fMaxChaseSpeed(1.f),
          fMaxParticles(),
          fDistSq(),
          fInfluences()
    { }

    ~plParticleFlockEffect();

    CLASSNAME_REGISTER( plParticleFlockEffect );
    GETINTERFACE_ANY( plParticleFlockEffect, plParticleEffect );

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;

    void SetTargetOffset(const hsPoint3 &offset) { fTargetOffset = offset; }
    void SetDissenterTarget(const hsPoint3 &target) { fDissenterTarget = target; }
    void SetInfluenceAvgRadius(float val) { fInfAvgRadSq = val * val; }
    void SetInfluenceRepelRadius(float val) { fInfRepRadSq = val * val; }
    void SetGoalRadius(float val) { fGoalDistSq = val * val; }
    void SetFullChaseRadius(float val) { fFullChaseDistSq = val * val; }
    void SetConformStr(float val) { fAvgVelStr = val; }
    void SetRepelStr(float val) { fRepDirStr = val; }
    void SetGoalOrbitStr(float val) { fGoalOrbitStr = val; }
    void SetGoalChaseStr(float val) { fGoalChaseStr = val; }
    void SetMaxOrbitSpeed(float val) { fMaxOrbitSpeed = val; }
    void SetMaxChaseSpeed(float val) { fMaxChaseSpeed = val; }
    void SetMaxParticles(uint16_t num);

    void Read(hsStream *s, hsResMgr *mgr) override;
    void Write(hsStream *s, hsResMgr *mgr) override;
    bool MsgReceive(plMessage *msg) override;
};  

class plParticleFollowSystemEffect : public plParticleEffect
{
public:
    CLASSNAME_REGISTER( plParticleFollowSystemEffect );
    GETINTERFACE_ANY( plParticleFollowSystemEffect, plParticleEffect );
    
    plParticleFollowSystemEffect();

    void PrepareEffect(const plEffectTargetInfo& target) override;
    bool ApplyEffect(const plEffectTargetInfo& target, int32_t i) override;
    void EndEffect(const plEffectTargetInfo& target) override;
    
protected:
    hsMatrix44 fOldW2L;
    bool fEvalThisFrame;
};

#endif
