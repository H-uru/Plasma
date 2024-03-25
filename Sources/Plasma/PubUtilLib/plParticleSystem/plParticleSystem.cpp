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


#include "plParticleSystem.h"

#include "plParticle.h"
#include "plParticleEffect.h"
#include "plParticleEmitter.h"
#include "plParticleGenerator.h"
#include "plParticleSDLMod.h"

#include "plgDispatch.h"
#include "plPipeline.h"
#include "plProfile.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "plTweak.h"

#include "pnMessage/plRefMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plDrawable/plParticleFiller.h"
#include "plSurface/hsGMaterial.h"
#include "plInterp/plController.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plParticleUpdateMsg.h"
#include "plMessage/plRenderMsg.h"

plProfile_CreateCounter("Num Particles", "Particles", NumParticles);

const float plParticleSystem::GRAVITY_ACCEL_FEET_PER_SEC2 = 32.0f;

plParticleSystem::~plParticleSystem()
{
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
    {
        delete fEmitters[i];
    }
    delete [] fEmitters;

    delete fAmbientCtl;
    delete fDiffuseCtl;
    delete fOpacityCtl;
    delete fWidthCtl;
    delete fHeightCtl;
}

void plParticleSystem::Init(uint32_t xTiles, uint32_t yTiles, uint32_t maxTotalParticles, uint32_t maxEmitters, 
                            plController *ambientCtl,   plController *diffuseCtl, plController *opacityCtl, 
                            plController *widthCtl, plController *heightCtl)
{
    fTarget = nullptr;
    fLastTime = 0;
    fCurrTime = 0;
    SetGravity(1.0f);
    fDrag = 0;
    fWindMult = 1.f;

    fNumValidEmitters = 0;
    fNextEmitterToGo = 0;
    fMiscFlags = 0;

    fForces.clear();
    fEffects.clear();
    fConstraints.clear();

    fXTiles = xTiles;
    fYTiles = yTiles;

    fMaxTotalParticles = fMaxTotalParticlesLeft = maxTotalParticles;
    fMaxEmitters = maxEmitters;
    fEmitters = new plParticleEmitter *[fMaxEmitters];
    int i;
    for (i = 0; i < maxEmitters; i++)
        fEmitters[i] = nullptr;

    fAmbientCtl = ambientCtl;
    fDiffuseCtl = diffuseCtl;
    fOpacityCtl = opacityCtl;
    fWidthCtl = widthCtl;
    fHeightCtl = heightCtl;
}

void plParticleSystem::IAddEffect(plParticleEffect *effect, uint32_t type)
{
    switch(type)
    {
    case kEffectForce:
        fForces.emplace_back(effect);
        break;
    case kEffectMisc:
    default:
        fEffects.emplace_back(effect);
        break;
    case kEffectConstraint:
        fConstraints.emplace_back(effect);
        break;
    }
}

plParticleEmitter* plParticleSystem::GetAvailEmitter()
{
    if( !fNumValidEmitters ) // got to start with at least one.
        return nullptr;

    float minTTL = 1.e33f;
    int iMinTTL = -1;
    int i;
    for( i = 0; i < fNumValidEmitters; i++ )
    {
        if( fEmitters[i]->GetTimeToLive() < minTTL )
        {
            minTTL = fEmitters[i]->GetTimeToLive();
            iMinTTL = i;
        }
    }
    if( minTTL > 0 )
    {
        if( fNumValidEmitters < fMaxEmitters )
        {
            minTTL = 0;
            iMinTTL = fNumValidEmitters++;
            fEmitters[iMinTTL] = new plParticleEmitter();
            fEmitters[iMinTTL]->Clone(fEmitters[0], iMinTTL);

            hsAssert(fMaxTotalParticlesLeft >= fEmitters[iMinTTL]->fMaxParticles,
                     "Should have planned better");
            fMaxTotalParticlesLeft -= fEmitters[iMinTTL]->fMaxParticles;

            // Don't really use this. fEmitters[i]->GetSpanIndex() always == i.
            fNextEmitterToGo = (fNextEmitterToGo + 1) % fMaxEmitters; 
        }
    }
    return fEmitters[iMinTTL];
}

uint32_t plParticleSystem::AddEmitter(uint32_t maxParticles, plParticleGenerator *gen, uint32_t emitterFlags)
{
    if (fMaxEmitters == 0) // silly rabbit, Trix are for kids!
        return 0;
    uint32_t currEmitter;
    if (fNumValidEmitters == fMaxEmitters) // No more free spots, snag the next in line.
    {
        int i;
        for (i = 0; i < fMaxEmitters; i++)
        {
            if (fEmitters[i]->GetSpanIndex() == fNextEmitterToGo)
                break;
        }
        currEmitter = i;

        fMaxTotalParticlesLeft += fEmitters[currEmitter]->fMaxParticles;
        hsAssert(fMaxTotalParticlesLeft <= fMaxTotalParticles, "Particle system somehow has more particles than it started with.");
        delete fEmitters[currEmitter];
    }
    else
    {
        currEmitter = fNumValidEmitters;
        fNumValidEmitters++;
    }

    if (maxParticles > fMaxTotalParticlesLeft)
        maxParticles = fMaxTotalParticlesLeft;

    fEmitters[currEmitter] = new plParticleEmitter();
    fEmitters[currEmitter]->Init(this, maxParticles, fNextEmitterToGo, emitterFlags, gen);

    fMaxTotalParticlesLeft -= fEmitters[currEmitter]->fMaxParticles;
    fNextEmitterToGo = (fNextEmitterToGo + 1) % fMaxEmitters;

    return maxParticles;
}

void plParticleSystem::AddParticle(hsPoint3 &pos, hsVector3 &velocity, uint32_t tileIndex, 
                                   float hSize, float vSize, float scale, float invMass, float life,
                                   hsPoint3 &orientation, uint32_t miscFlags, float radsPerSec)
{
    hsAssert(fNumValidEmitters > 0, "Trying to explicitly add particles to a system with no valid emitters.");
    if (fNumValidEmitters == 0)
        return;

    fEmitters[0]->AddParticle(pos, velocity, tileIndex, hSize, vSize, scale, invMass, life, orientation, miscFlags, radsPerSec);
}

void plParticleSystem::GenerateParticles(uint32_t num, float dt /* = 0.f */)
{
    if (num <= 0)
        return;

    if (fNumValidEmitters > 0 && fEmitters[0]->fGenerator)
        fEmitters[0]->fGenerator->AddAutoParticles(fEmitters[0], dt, num);

    GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);   
}

void plParticleSystem::WipeExistingParticles()
{
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
        fEmitters[i]->WipeExistingParticles();
}

void plParticleSystem::KillParticles(float num, float timeToDie, uint8_t flags)
{
    if (fEmitters[0])
        fEmitters[0]->KillParticles(num, timeToDie, flags);

    GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);   
}

void plParticleSystem::TranslateAllParticles(hsPoint3 &amount)
{
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
        fEmitters[i]->TranslateAllParticles(amount);
}

void plParticleSystem::DisableGenerators()
{
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
        fEmitters[i]->UpdateGenerator(plParticleUpdateMsg::kParamEnabled, 0.f);
}

uint16_t plParticleSystem::StealParticlesFrom(plParticleSystem *victim, uint16_t num)
{
    if (fNumValidEmitters <= 0)
        return 0; // you just lose

    if (victim)
    {
        uint16_t numStolen = fEmitters[0]->StealParticlesFrom(victim->fNumValidEmitters > 0 ? victim->fEmitters[0] : nullptr, num);
        GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);   
        victim->GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);   
        return numStolen;
    }
    
    return 0;
}

plParticleGenerator *plParticleSystem::GetExportedGenerator() const 
{ 
    return (fNumValidEmitters > 0 ? fEmitters[0]->fGenerator : nullptr);
}

plParticleEffect *plParticleSystem::GetEffect(uint16_t type) const
{
    for (plParticleEffect* forceEffect : fForces)
        if (forceEffect->ClassIndex() == type)
            return forceEffect;

    for (plParticleEffect* effect : fEffects)
        if (effect->ClassIndex() == type)
            return effect;

    for (plParticleEffect* constraint : fConstraints)
        if (constraint->ClassIndex() == type)
            return constraint;

    return nullptr;
}

uint32_t plParticleSystem::GetNumValidParticles(bool immortalOnly /* = false */) const
{
    uint32_t count = 0;
    int i, j;
    for (i = 0; i < fNumValidEmitters; i++)
    {
        if (immortalOnly)
        {
            for (j = 0; j < fEmitters[i]->fNumValidParticles; j++)
            {
                if (fEmitters[i]->fParticleExts[j].fMiscFlags & plParticleExt::kImmortal)
                    count++;
            }
        }
        else
            count += fEmitters[i]->fNumValidParticles;
    }
    return count;
}


const hsMatrix44 &plParticleSystem::GetLocalToWorld() const
{ 
    return fTarget->GetCoordinateInterface()->GetLocalToWorld(); 
}

bool plParticleSystem::IEval(double secs, float del, uint32_t dirty)
{
    return false;
}



bool plParticleSystem::IShouldUpdate(plPipeline* pipe) const
{

    if (fMiscFlags & kParticleSystemAlwaysUpdate)
        return true;

    if (IGetTargetDrawInterface(0) && IGetTargetDrawInterface(0)->GetProperty(plDrawInterface::kDisable))
        return false;

    // First, what are the cumulative bounds for this system.
    hsBounds3Ext wBnd;
    wBnd.MakeEmpty();
    int i;
    for( i = 0; i < fNumValidEmitters; i++ )
    {
        if( fEmitters[i]->GetBoundingBox().GetType() == kBoundsNormal )
            wBnd.Union(&fEmitters[i]->GetBoundingBox());
    }

    // Always update if we are currently empty
    if( wBnd.GetType() == kBoundsEmpty )
    {
        return true;
    }

    // Now, are we visible?
    bool isVisible = pipe->TestVisibleWorld(wBnd);

    float delta = fLastTime > 0 ? float(fCurrTime - fLastTime) : hsTimer::GetDelSysSeconds();
    if( isVisible )
    {
        // If we know how fast the fastest particle is moving, then we can
        // decide if the system is too far away to need to update every single frame.
        // In fact, based on the speed of the fastest particle, we can determine an
        // exact update rate for a given error (say 3 pixels?).
        // But we don't currently know the speed of the fastest particle, so
        // until we do, I'm commenting this out. Most of the speed gain from
        // culling particle updates was from not updating systems that aren't visible
        // anyway.
#if 0 // ALWAYS_IF_VISIBLE
        // We're in view, but how close are we to the camera? Look at closest point.
        hsPoint2 depth;
        wBnd.TestPlane(pipe->GetViewDirWorld(), depth);
        float eyeDist = pipe->GetViewDirWorld().InnerProduct(pipe->GetViewPositionWorld());
        float dist = depth.fX - eyeDist;

        static float kUpdateCutoffDist = 100.f;
        if( dist > kUpdateCutoffDist )
        {
            static float kDistantUpdateSecs = 0.1f;
            return delta >= kDistantUpdateSecs;
        }
#endif // ALWAYS_IF_VISIBLE

        return true;
    }
    

    static float kOffscreenUpdateSecs = 1.f;
    return delta >= kOffscreenUpdateSecs;
}

plDrawInterface* plParticleSystem::ICheckDrawInterface()
{
    plDrawInterface* di = IGetTargetDrawInterface(0);
    if( !di )
        return nullptr;

    if( di->GetDrawableMeshIndex(0) == uint32_t(-1) )
    {
        di->SetUpForParticleSystem( fMaxEmitters + 1, fMaxTotalParticles, fTexture, fPermaLights );
        hsAssert(di->GetDrawableMeshIndex( 0 ) != (uint32_t)-1, "SetUpForParticleSystem should never fail"); // still invalid, didn't fix it.
    }

    return di;
}

void plParticleSystem::IHandleRenderMsg(plPipeline* pipe)
{   
    fCurrTime = hsTimer::GetSysSeconds(); 
    float delta = float(fCurrTime - fLastTime);
    if (delta == 0)
        return;
    plConst(float) kMaxDelta(0.3f);
    if( delta > kMaxDelta )
        delta = kMaxDelta;

    plDrawInterface* di = ICheckDrawInterface();
    if( !di )
        return;

    bool disabled = di->GetProperty(plDrawInterface::kDisable);
    if (!IShouldUpdate(pipe))
    {
        if (disabled)
            di->ResetParticleSystem(); // Need to call this, otherwise particles get drawn, even though the DI is disabled.
                                       // (Yes, it's lame.)

        // Otherwise, we leave the DI alone, and the particles draw in the same place they were last frame.
        return;
    }

    fContext.fPipeline = pipe;
    fContext.fSystem = this;
    fContext.fSecs = fCurrTime;
    fContext.fDelSecs = delta;

    di->ResetParticleSystem();

    if (fPreSim > 0)
        IPreSim();
    
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
    {
        fEmitters[i]->IUpdate(delta);
        plProfile_IncCount(NumParticles, fEmitters[i]->fNumValidParticles);
        if (!disabled)
        {
            if( fEmitters[ i ]->GetParticleCount() > 0 )
                di->AssignEmitterToParticleSystem( fEmitters[ i ] ); // Go make those polys!
        }
    }
    
//  plParticleFiller::FillParticlePolys(pipe, di);

    fLastTime = fCurrTime;
}


#include "plProfile.h"
plProfile_CreateTimer("ParticleSys", "RenderSetup", ParticleSys);


bool plParticleSystem::MsgReceive(plMessage* msg)
{
    plGenRefMsg* refMsg;
    plParticleUpdateMsg *partMsg;
    plParticleKillMsg *killMsg;
    plSceneObject *scene;
    plParticleEffect *effect;
    hsGMaterial *mat;
    plRenderMsg *rend;
    plAgeLoadedMsg* ageLoaded;

    if ((rend = plRenderMsg::ConvertNoRef(msg)))
    {
        plProfile_BeginLap(ParticleSys, this->GetKey()->GetUoid().GetObjectName().c_str());
        IHandleRenderMsg(rend->Pipeline());
        plProfile_EndLap(ParticleSys, this->GetKey()->GetUoid().GetObjectName().c_str());
        return true;
    }
    else if ((refMsg = plGenRefMsg::ConvertNoRef(msg)))
    {
        if ((scene = plSceneObject::ConvertNoRef(refMsg->GetRef())))
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
                AddTarget(scene);
            else
                RemoveTarget(scene);
            return true;
        }
        if ((mat = hsGMaterial::ConvertNoRef(refMsg->GetRef())))
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
                fTexture = mat;
            else
                fTexture = nullptr;
            return true;
        }
        if ((effect = plParticleEffect::ConvertNoRef(refMsg->GetRef())))
        {
            if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
                IAddEffect(effect, refMsg->fType);
            //else
            //  IRemoveEffect(effect, refMsg->fType);
            return true;
        }
    }
    else if ((partMsg = plParticleUpdateMsg::ConvertNoRef(msg)))
    {
        UpdateGenerator(partMsg->GetParamID(), partMsg->GetParamValue());
        return true;
    }
    else if ((killMsg = plParticleKillMsg::ConvertNoRef(msg)))
    {
        KillParticles(killMsg->fNumToKill, killMsg->fTimeLeft, killMsg->fFlags);
        return true;
    }
    else if( (ageLoaded = plAgeLoadedMsg::ConvertNoRef(msg)) && ageLoaded->fLoaded )
    {
        ICheckDrawInterface();
        return true;
    }
    return plModifier::MsgReceive(msg);
}

void plParticleSystem::UpdateGenerator(uint32_t paramID, float value)
{
    int i;
    for (i = 0; i < fNumValidEmitters; i++)
        fEmitters[i]->UpdateGenerator(paramID, value);
}

void plParticleSystem::AddTarget(plSceneObject *so)
{
    if (fTarget != nullptr)
        RemoveTarget(fTarget);

    fTarget = so;
    plgDispatch::Dispatch()->RegisterForExactType(plTimeMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

    delete fParticleSDLMod;
    fParticleSDLMod = new plParticleSDLMod;
    fParticleSDLMod->SetAttachedToAvatar(fAttachedToAvatar);
    so->AddModifier(fParticleSDLMod);
}

void plParticleSystem::RemoveTarget(plSceneObject *so)
{
    if (so == fTarget && so != nullptr)
    {
        if (fParticleSDLMod)
        {
            so->RemoveModifier(fParticleSDLMod);
            delete fParticleSDLMod;
            fParticleSDLMod = nullptr;
        }
        fTarget = nullptr;
    }
}

// This can be done much faster, but it's only done on load, and very very clean as is. Saving optimization for
// when we observe that it's too slow.
void plParticleSystem::IPreSim()
{
    const double PRESIM_UPDATE_TICK = 0.1;

    int i;
    for (i = 0; i < fNumValidEmitters; i++)
    {
        double secs = fPreSim;
        while (secs > 0)
        {
            fEmitters[i]->IUpdateParticles((float)PRESIM_UPDATE_TICK);
            secs -= PRESIM_UPDATE_TICK;
        }
    }

    fPreSim = 0;
}

void plParticleSystem::IReadEffectsArray(std::vector<plParticleEffect *> &effects, uint32_t type, hsStream *s, hsResMgr *mgr)
{
    plGenRefMsg *msg;
    effects.clear();
    uint32_t count = s->ReadLE32();
    effects.reserve(count);
    for (uint32_t i = 0; i < count; i++)
    {
        msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, (int8_t)type);
        mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
    }
}

void plParticleSystem::Read(hsStream *s, hsResMgr *mgr)
{
    plModifier::Read(s, mgr);

    plGenRefMsg* msg;
    msg = new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0); // Material
    mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);

    fAmbientCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));    
    fDiffuseCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fOpacityCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fWidthCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
    fHeightCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));

    uint32_t xTiles = s->ReadLE32();
    uint32_t yTiles = s->ReadLE32();
    uint32_t maxTotal = s->ReadLE32();
    uint32_t maxEmitters = s->ReadLE32();
    Init(xTiles, yTiles, maxTotal, maxEmitters, fAmbientCtl, fDiffuseCtl, fOpacityCtl, fWidthCtl, fHeightCtl);

    fPreSim = s->ReadLEFloat();
    fAccel.Read(s);
    fDrag = s->ReadLEFloat();
    fWindMult = s->ReadLEFloat();

    fNumValidEmitters = s->ReadLE32();
    for (uint32_t i = 0; i < fNumValidEmitters; i++)
    {
        fEmitters[i] = plParticleEmitter::ConvertNoRef(mgr->ReadCreatable(s));
        fEmitters[i]->ISetSystem(this);
    }

    IReadEffectsArray(fForces, kEffectForce, s, mgr);
    IReadEffectsArray(fEffects, kEffectMisc, s, mgr);
    IReadEffectsArray(fConstraints, kEffectConstraint, s, mgr);

    uint32_t count = s->ReadLE32();
    fPermaLights.resize(count);
    for (uint32_t i = 0; i < count; i++)
        fPermaLights[i] = mgr->ReadKey(s);
}

void plParticleSystem::Write(hsStream *s, hsResMgr *mgr)
{
    plModifier::Write(s, mgr);

    int i;

    mgr->WriteKey(s, fTexture);

    // Arguments to the Init() function
    mgr->WriteCreatable(s, fAmbientCtl);
    mgr->WriteCreatable(s, fDiffuseCtl);
    mgr->WriteCreatable(s, fOpacityCtl);
    mgr->WriteCreatable(s, fWidthCtl);
    mgr->WriteCreatable(s, fHeightCtl);
    
    s->WriteLE32(fXTiles);
    s->WriteLE32(fYTiles);
    s->WriteLE32(fMaxTotalParticles);
    s->WriteLE32(fMaxEmitters);

    s->WriteLEFloat(fPreSim);
    fAccel.Write(s);
    s->WriteLEFloat(fDrag);
    s->WriteLEFloat(fWindMult);

    s->WriteLE32(fNumValidEmitters);
    for (i = 0; i < fNumValidEmitters; i++)
    {
        mgr->WriteCreatable(s, fEmitters[i]);
    }

    s->WriteLE32((uint32_t)fForces.size());
    for (plParticleEffect* forceEffect : fForces)
        mgr->WriteKey(s, forceEffect);

    s->WriteLE32((uint32_t)fEffects.size());
    for (plParticleEffect* effect : fEffects)
        mgr->WriteKey(s, effect);

    s->WriteLE32((uint32_t)fConstraints.size());
    for (plParticleEffect* constraint : fConstraints)
        mgr->WriteKey(s, constraint);

    s->WriteLE32((uint32_t)fPermaLights.size());
    for (const plKey& key : fPermaLights)
        mgr->WriteKey(s, key);
}

void plParticleSystem::SetAttachedToAvatar(bool attached)
{
    fAttachedToAvatar = attached;
    if (fParticleSDLMod)
        fParticleSDLMod->SetAttachedToAvatar(attached);
}

void plParticleSystem::AddLight(plKey liKey)
{
    fPermaLights.emplace_back(std::move(liKey));
}
