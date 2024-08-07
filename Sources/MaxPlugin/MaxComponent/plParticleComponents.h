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
    float    fConeAngle;
    float    fVelocityMin;
    float    fVelocityMax;
    float    fLifeMin;
    float    fLifeMax;
    float    fPPS;
    float    fScaleMin;
    float    fScaleMax;
    float    fGravity;
    float    fDrag;
    float    fWindMult;
    float    fMassRange;
    float    fRotRange;
    float    fPreSim;
    float    fHSize;
    float    fVSize;
    uint32_t      fGenType;
    uint32_t      fXTiles;
    uint32_t      fYTiles;
    uint32_t      fNormal;
    uint32_t      fOrientation;
    bool        fImmortal;
    
    ParticleStats() : fConeAngle(0.5), fVelocityMin(30.0), fVelocityMax(50.0), fLifeMin(5.0), fLifeMax(10.0), fPPS(20.0),
                      fScaleMin(80), fScaleMax(120), fGravity(100), fDrag(0), fWindMult(1.f), fMassRange(0), fRotRange(0), 
                      fPreSim(0), fGenType(0), 
                      fXTiles(1), fYTiles(1), fHSize(1), fVSize(1), fImmortal(false) {}
};  

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
    static const TCHAR* GenStrings[];

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

    void DeleteThis() override { delete this; }
    static bool IsParticleSystemComponent(plComponentBase *comp);
    static bool NodeHasSystem(plMaxNode *pNode);
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool AddToAnim(plAGAnim *anim, plMaxNode *node) override;
    virtual bool GetParamVals(plMaxNode *pNode) = 0;
    void SetParticleStats(plParticleMtl *mtl);

    void CollectNonDrawables(INodeTab& nonDrawables) override { AddTargetsToList(nonDrawables); }

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
    bool GetParamVals(plMaxNode *pNode) override;

    bool AllowUnhide() override { return fAllowUnhide; }

    void SetEmitterReserve(int numEmitters) override { fEmitterReserve = numEmitters; }
    int GetEmitterReserve() const override { return fEmitterReserve; }
};

#define PARTICLE_SYSTEM_COMPONENT_CLASS_ID Class_ID(0x684c5d88, 0x536b1a29)

//////////////////////////////////////////////////////////////////////////////////////////
//
// Particle Effects Base class

class plParticleEffectComponent : public plComponent
{
protected:
    plParticleEffect*       fEffect;
public:
    plParticleEffectComponent() : fEffect() { }
    void DeleteThis() override { delete this; }

    bool SetupProperties(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    bool Convert(plMaxNode *node, plErrorMsg *pErrMsg) override { return TRUE; }
    
    virtual void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) = 0;
    static bool IsParticleEffectComponent(plComponentBase *comp);
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
    void DeleteThis() override { delete this; }
    void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) override;
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
    void DeleteThis() override { delete this; }
    bool PreConvert(plMaxNode *node, plErrorMsg *pErrMsg) override;
    void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) override;
    void BuildVolume(plMaxNode *node);

    void CollectNonDrawables(INodeTab& nonDrawables) override;

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
    void DeleteThis() override { delete this; }
    void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) override;
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
    void DeleteThis() override { delete this; }
    void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) override;
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
    void DeleteThis() override { delete this; }
    void AddToParticleSystem(plParticleSystem *sys, plMaxNode *node) override;
};

const Class_ID PARTICLE_FLOCK_COMPONENT_CLASS_ID(0x3f522d7f, 0x409b66cc);

#endif
