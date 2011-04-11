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
#ifndef PL_PARTICLE_COMPONENTS_H
#define PL_PARTICLE_COMPONENTS_H

class plParticleMtl;
class plParticleSystem;
class plParticleEffect;
class plBoundInterface;
class plAGAnim;
class plLightGrpComponent;


//////////////////////////////////////////////////////////////////////////////////////////
//
// stuff for plParticleComponent

class ParticleStats
{	

public:
	hsScalar	fConeAngle;
	hsScalar	fVelocityMin;
	hsScalar	fVelocityMax;
	hsScalar	fLifeMin;
	hsScalar	fLifeMax;
	hsScalar	fPPS;
	hsScalar	fScaleMin;
	hsScalar	fScaleMax;
	hsScalar	fGravity;
	hsScalar	fDrag;
	hsScalar	fWindMult;
	hsScalar	fMassRange;
	hsScalar	fRotRange;
	hsScalar	fPreSim;
	hsScalar	fHSize;
	hsScalar	fVSize;
	UInt32		fGenType;
	UInt32		fXTiles;
	UInt32		fYTiles;
	UInt32		fNormal;
	UInt32		fOrientation;
	hsBool		fImmortal;
	
	ParticleStats() : fConeAngle(0.5), fVelocityMin(30.0), fVelocityMax(50.0), fLifeMin(5.0), fLifeMax(10.0), fPPS(20.0),
					  fScaleMin(80), fScaleMax(120), fGravity(100),	fDrag(0), fWindMult(1.f), fMassRange(0), fRotRange(0), 
					  fPreSim(0), fGenType(0), 
					  fXTiles(1), fYTiles(1), fHSize(1), fVSize(1), fImmortal(false) {}
};	

class ParticleCompDlgProc;
extern ParticleCompDlgProc gParticleCompDlgProc;

class plParticleCoreComponent : public plComponent
{
protected:
public:
	enum // generation types
	{
		kGenPoint,
		kGenMesh,
		kGenOnePerVertex,
		kGenNumOptions
	};
	static const char *GenStrings[];

	enum
	{
		kGenType,
		kConeAngle,
		kVelocityMin,
		kVelocityMax,
		kLifeMin,
		kLifeMax,
		kImmortal,
		kPPS,
		kScaleMin,
		kScaleMax,
		kGravity,
		kDrag,
		kPreSim,
		kWindMult,
		kMassRange,
		kRotRange,
		kFollowSystem
	};

	virtual void DeleteThis() { delete this; }
	static hsBool IsParticleSystemComponent(plComponentBase *comp);
	static hsBool NodeHasSystem(plMaxNode *pNode);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool AddToAnim(plAGAnim *anim, plMaxNode *node);
	virtual hsBool GetParamVals(plMaxNode *pNode) = 0;
	void SetParticleStats(plParticleMtl *mtl);

	virtual void CollectNonDrawables(INodeTab& nonDrawables) { AddTargetsToList(nonDrawables); }

	void IHandleLights(plLightGrpComponent* liGrp, plParticleSystem* sys);

	virtual void SetEmitterReserve(int numEmitters) = 0;
	virtual int GetEmitterReserve() const = 0;

	ParticleStats fUserInput;
};

class plParticleComponent : public plParticleCoreComponent
{
protected:
	int fEmitterReserve;

public:
	// Umm, don't touch this
	static bool fAllowUnhide;

	plParticleComponent();
	hsBool GetParamVals(plMaxNode *pNode);

	virtual bool AllowUnhide() { return fAllowUnhide; }

	virtual void SetEmitterReserve(int numEmitters) { fEmitterReserve = numEmitters; }
	virtual int GetEmitterReserve() const { return fEmitterReserve; }
};

#define PARTICLE_SYSTEM_COMPONENT_CLASS_ID Class_ID(0x684c5d88, 0x536b1a29)

//////////////////////////////////////////////////////////////////////////////////////////
//
// Particle Effects Base class

class plParticleEffectComponent : public plComponent
{
protected:
	plParticleEffect*		fEffect;
public:
	plParticleEffectComponent() : fEffect(nil) {}
	virtual void DeleteThis() { delete this; }

	virtual hsBool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg);
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual hsBool Convert(plMaxNode *node, plErrorMsg *pErrMsg) { return TRUE; }; 
	
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) = 0;
	static hsBool IsParticleEffectComponent(plComponentBase *comp);
};

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleFadeComponent

class plParticleFadeComponent : public plParticleEffectComponent
{
public:
	enum
	{
		kFadeDistance,
		kFadeZ
	};

	plParticleFadeComponent();
	virtual void DeleteThis() { delete this; }
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node);
};

#define PARTICLE_FADE_COMPONENT_CLASS_ID Class_ID(0x17496d81, 0x3bb14bc4)

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleVolumeComponent

class plParticleVolumeComponent : public plParticleEffectComponent
{
public:
	enum
	{
		kSourceNode,
		kOnImpact,
		kBounceAmt,
		kFrictionAmt
	};
	// Things to do on impact
	enum
	{
		kImpDefault,
		kImpDie,
		kImpBounce
	};


	plParticleVolumeComponent();
	virtual void DeleteThis() { delete this; }
	virtual hsBool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg); 
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node);
	void BuildVolume(plMaxNode *node);

	virtual void CollectNonDrawables(INodeTab& nonDrawables);

protected:
	plBoundInterface *fBound;
};

#define PARTICLE_VOLUME_COMPONENT_CLASS_ID Class_ID(0x46087f90, 0x77625ce1)


//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleWindComponent

class plParticleWindComponent : public plParticleEffectComponent
{
public:
	enum
	{
		kScaleX,
		kScaleY,
		kScaleZ,
		kSpeed,
		kStrength,
		kConstancy,
		kSwirl,
		kHorizontal,
		kLocalize,
		kClampAngle,
		kRefObject
	};

	plParticleWindComponent();
	virtual void DeleteThis() { delete this; }
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node);
};

const Class_ID PARTICLE_WIND_COMPONENT_CLASS_ID(0x728c40b2, 0x499068b3);

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleUniWindComponent

class plParticleUniWindComponent : public plParticleEffectComponent
{
public:
	enum
	{
		kStrength,
		kConstancy,
		kSwirl,
		kHorizontal,
		kMinSecs,
		kMaxSecs,
		kRate,
		kClampAngle,
		kRefObject
	};

	plParticleUniWindComponent();
	virtual void DeleteThis() { delete this; }
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node);
};

const Class_ID PARTICLE_UNIWIND_COMPONENT_CLASS_ID(0x2e1d4e50, 0x2f925aac);

//////////////////////////////////////////////////////////////////////////////////////////
//
// plParticleFlockComponent

class plParticleFlockComponent : public plParticleEffectComponent
{
public:
	enum
	{
		kOffsetX,
		kOffsetY,
		kOffsetZ,
		kInfAvgDist,
		kInfRepDist,
		kGoalDist,
		kInfAvgStr,
		kInfRepStr,
		kGoalOrbitStr,
		kMaxChaseSpeed,
		kGoalChaseStr,
		kMaxOrbitSpeed,
		kFullChaseDist,
	};

	plParticleFlockComponent();
	virtual void DeleteThis() { delete this; }
	virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node);
};

const Class_ID PARTICLE_FLOCK_COMPONENT_CLASS_ID(0x3f522d7f, 0x409b66cc);

#endif