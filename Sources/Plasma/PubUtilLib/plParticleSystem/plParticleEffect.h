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
#ifndef plParticleEffect_inc
#define plParticleEffect_inc

#include "../pnKeyedObject/hsKeyedObject.h"
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
	//		ParticleContext).
	//  ApplyEffect is called some once for each particle (maybe zero times). 
	//		It can return true to kill a particle. 
	//		Target and Context passed in with Prepare will be
	//		guaranteed to remain valid until,
	//  EndEffect marks no more particles will be processed with the above
	//		context (invalidating anything cached).
	// Defaults for Prepare and End are no-ops.
	virtual void PrepareEffect(const plEffectTargetInfo& target) {}
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i) = 0;
	virtual void EndEffect(const plEffectTargetInfo& target) {}
};

class plParticleCollisionEffect : public plParticleEffect
{
public:
	plParticleCollisionEffect();
	~plParticleCollisionEffect();

	CLASSNAME_REGISTER( plParticleCollisionEffect );
	GETINTERFACE_ANY( plParticleCollisionEffect, plParticleEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);
	virtual hsBool MsgReceive(plMessage *msg);

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

	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);
};

// This particle blocker just kills any particles that hit it.
class plParticleCollisionEffectDie : public plParticleCollisionEffect
{
public:
	plParticleCollisionEffectDie();

	CLASSNAME_REGISTER( plParticleCollisionEffectDie );
	GETINTERFACE_ANY( plParticleCollisionEffectDie, plParticleCollisionEffect );

	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);
};

class plParticleCollisionEffectBounce : public plParticleCollisionEffect
{
protected:
	hsScalar	fBounce;
	hsScalar	fFriction;
public:
	plParticleCollisionEffectBounce();

	CLASSNAME_REGISTER( plParticleCollisionEffectBounce );
	GETINTERFACE_ANY( plParticleCollisionEffectBounce, plParticleCollisionEffect );

	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

	void SetBounce(hsScalar b) { fBounce = b; }
	hsScalar GetBounce() const { return fBounce; }

	void SetFriction(hsScalar f) { fFriction = f; }
	hsScalar GetFriction() const { return fFriction; }
};

class plParticleFadeVolumeEffect : public plParticleEffect
{
protected:
	// Some cached properties. These will be the same for all
	// particles between matching PrepareEffect and EndEffect calls.
	hsPoint3	fMax;
	hsPoint3	fMin;
	hsPoint3	fNorm;

public:
	plParticleFadeVolumeEffect();
	~plParticleFadeVolumeEffect();

	CLASSNAME_REGISTER( plParticleFadeVolumeEffect );
	GETINTERFACE_ANY( plParticleFadeVolumeEffect, plParticleEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);
	//virtual hsBool MsgReceive(plMessage *msg);

	hsScalar fLength;
	hsBool fIgnoreZ;
};

class plParticleWindEffect : public plParticleEffect
{
protected:
	// The properties that define the wind. These might/should be animatable.
	hsScalar	fStrength;
	hsScalar	fConstancy;
	hsScalar	fSwirl;
	hsBool		fHorizontal;
	hsVector3	fRefDir;

	// Some cached properties. These will be the same for all
	// particles between matching PrepareEffect and EndEffect calls.
	// These define the current state of the wind.
	hsVector3	fWindVec;
	hsVector3	fRandDir;
	hsVector3	fDir;
	double		fLastDirSecs;
public:

	plParticleWindEffect();
	~plParticleWindEffect();

	CLASSNAME_REGISTER( plParticleWindEffect );
	GETINTERFACE_ANY( plParticleWindEffect, plParticleEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i) = 0;

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

	void				SetStrength(hsScalar v) { fStrength = v; }
	hsScalar			GetStrength() const { return fStrength; }

	void				SetConstancy(hsScalar c) { fConstancy = c; }
	hsScalar			GetConstancy() const { return fConstancy; }

	void				SetSwirl(hsScalar s) { fSwirl = s; }
	hsScalar			GetSwirl() const { return fSwirl; }

	void				SetHorizontal(hsBool on) { fHorizontal = on; }
	hsBool				GetHorizontal() const { return fHorizontal; }

	void				SetRefDirection(const hsVector3& v);
	const hsVector3&	GetRefDirection() const { return fRefDir; }
};

class plParticleLocalWind : public plParticleWindEffect
{
protected:
	hsVector3	fScale;
	hsScalar	fSpeed;

	hsVector3	fPhase;
	hsVector3	fInvScale;
	double		fLastPhaseSecs;
	
public:
	plParticleLocalWind();
	~plParticleLocalWind();

	CLASSNAME_REGISTER( plParticleLocalWind );
	GETINTERFACE_ANY( plParticleLocalWind, plParticleWindEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);

	void				SetScale(const hsVector3& v) { fScale = v; }
	const hsVector3&	GetScale() const { return fScale; }

	void				SetSpeed(hsScalar v) { fSpeed = v; }
	hsScalar			GetSpeed() const { return fSpeed; }

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

};

class plParticleUniformWind : public plParticleWindEffect
{
protected:

	hsScalar	fFreqMin;
	hsScalar	fFreqMax;
	hsScalar	fFreqCurr;
	hsScalar	fFreqRate;
	double		fCurrPhase;

	double		fLastFreqSecs;

	hsScalar	fCurrentStrength;
public:
	plParticleUniformWind();
	~plParticleUniformWind();

	CLASSNAME_REGISTER( plParticleUniformWind );
	GETINTERFACE_ANY( plParticleUniformWind, plParticleWindEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);

	void		SetFrequencyRange(hsScalar minSecsPerCycle, hsScalar maxSecsPerCycle);
	void		SetFrequencyRate(hsScalar secsPerCycle);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);

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
	hsPoint3 fTargetOffset;		// Worldspace offset from our target to get the true goal
	hsPoint3 fDissenterTarget;	// Where to particles go when they get scared and leave us?
	hsScalar fInfAvgRadSq;		// Square of the radius of influence for average velocity matching.
	hsScalar fInfRepRadSq;		// Same, for repelling from neighbors.
	hsScalar fAvgVelStr;		// How strongly are we influenced by average dir?
	hsScalar fRepDirStr;		// Same for repelling
	hsScalar fGoalOrbitStr;		// Same for the goal (when we're within the desired distance)
	hsScalar fGoalChaseStr;		// Same for the goal (when we're too far away, and chasing it)
	hsScalar fGoalDistSq;
	hsScalar fFullChaseDistSq;
	hsScalar fMaxOrbitSpeed;
	hsScalar fMaxChaseSpeed;

	UInt16 fMaxParticles;
	hsScalar *fDistSq;			// Table of distances from particle to particle
	plParticleInfluenceInfo *fInfluences; 

	void IUpdateDistances(const plEffectTargetInfo &target);
	void IUpdateInfluences(const plEffectTargetInfo &target);

public:
	plParticleFlockEffect();
	~plParticleFlockEffect();

	CLASSNAME_REGISTER( plParticleFlockEffect );
	GETINTERFACE_ANY( plParticleFlockEffect, plParticleEffect );

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);	

	void SetTargetOffset(const hsPoint3 &offset) { fTargetOffset = offset; }
	void SetDissenterTarget(const hsPoint3 &target) { fDissenterTarget = target; }
	void SetInfluenceAvgRadius(hsScalar val) { fInfAvgRadSq = val * val; }
	void SetInfluenceRepelRadius(hsScalar val) { fInfRepRadSq = val * val; }
	void SetGoalRadius(hsScalar val) { fGoalDistSq = val * val; }
	void SetFullChaseRadius(hsScalar val) { fFullChaseDistSq = val * val; }
	void SetConformStr(hsScalar val) { fAvgVelStr = val; }
	void SetRepelStr(hsScalar val) { fRepDirStr = val; }
	void SetGoalOrbitStr(hsScalar val) { fGoalOrbitStr = val; }
	void SetGoalChaseStr(hsScalar val) { fGoalChaseStr = val; }
	void SetMaxOrbitSpeed(hsScalar val) { fMaxOrbitSpeed = val; }
	void SetMaxChaseSpeed(hsScalar val) { fMaxChaseSpeed = val; }
	void SetMaxParticles(UInt16 num);

	virtual void Read(hsStream *s, hsResMgr *mgr);
	virtual void Write(hsStream *s, hsResMgr *mgr);
	virtual hsBool MsgReceive(plMessage *msg);	
};	

class plParticleFollowSystemEffect : public plParticleEffect
{
public:
	CLASSNAME_REGISTER( plParticleFollowSystemEffect );
	GETINTERFACE_ANY( plParticleFollowSystemEffect, plParticleEffect );
	
	plParticleFollowSystemEffect();

	virtual void PrepareEffect(const plEffectTargetInfo& target);
	virtual hsBool ApplyEffect(const plEffectTargetInfo& target, Int32 i);
	virtual void EndEffect(const plEffectTargetInfo& target);
	
protected:
	hsMatrix44 fOldW2L;
	hsBool fEvalThisFrame;
};

#endif
