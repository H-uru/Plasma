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
#include "hsTypes.h"
#include "plParticleSystem.h"
#include "plParticleEmitter.h"
#include "plParticleGenerator.h"
#include "plParticleEffect.h"
#include "plParticle.h"
#include "plParticleSDLMod.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../plMessage/plParticleUpdateMsg.h"
#include "../plInterp/plController.h"
#include "../plSurface/hsGMaterial.h"
#include "plPipeline.h"
#include "hsTimer.h"
#include "plProfile.h"
#include "plTweak.h"

#include "../plDrawable/plParticleFiller.h"

plProfile_CreateCounter("Num Particles", "Particles", NumParticles);

const hsScalar plParticleSystem::GRAVITY_ACCEL_FEET_PER_SEC2 = 32.0f;

plParticleSystem::plParticleSystem() : fParticleSDLMod(nil), fAttachedToAvatar(false)
{
}

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

void plParticleSystem::Init(UInt32 xTiles, UInt32 yTiles, UInt32 maxTotalParticles, UInt32 maxEmitters, 
							plController *ambientCtl,	plController *diffuseCtl, plController *opacityCtl,	
							plController *widthCtl, plController *heightCtl)
{
	fTarget = nil;
	fLastTime = 0;
	fCurrTime = 0;
	SetGravity(1.0f);
	fDrag = 0;
	fWindMult = 1.f;

	fNumValidEmitters = 0;
	fNextEmitterToGo = 0;
	fMiscFlags = 0;

	fForces.Reset();
	fEffects.Reset();
	fConstraints.Reset();

	fXTiles = xTiles;
	fYTiles = yTiles;

	fMaxTotalParticles = fMaxTotalParticlesLeft = maxTotalParticles;
	fMaxEmitters = maxEmitters;
	fEmitters = TRACKED_NEW plParticleEmitter *[fMaxEmitters];
	int i;
	for (i = 0; i < maxEmitters; i++)
		fEmitters[i] = nil;	

	fAmbientCtl = ambientCtl;
	fDiffuseCtl = diffuseCtl;
	fOpacityCtl = opacityCtl;
	fWidthCtl = widthCtl;
	fHeightCtl = heightCtl;
}

void plParticleSystem::IAddEffect(plParticleEffect *effect, UInt32 type)
{
	switch(type)
	{
	case kEffectForce:
		fForces.Append(effect);
		break;
	case kEffectMisc:
	default:
		fEffects.Append(effect);
		break;
	case kEffectConstraint:
		fConstraints.Append(effect);
		break;
	}
}

plParticleEmitter* plParticleSystem::GetAvailEmitter()
{
	if( !fNumValidEmitters ) // got to start with at least one.
		return nil;

	hsScalar minTTL = 1.e33;
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
			fEmitters[iMinTTL] = TRACKED_NEW plParticleEmitter();
			fEmitters[iMinTTL]->Clone(fEmitters[0], iMinTTL);

			fMaxTotalParticlesLeft -= fEmitters[iMinTTL]->fMaxParticles;
			hsAssert(fMaxTotalParticlesLeft >= 0, "Should have planned better");

			// Don't really use this. fEmitters[i]->GetSpanIndex() always == i.
			fNextEmitterToGo = (fNextEmitterToGo + 1) % fMaxEmitters; 
		}
	}
	return fEmitters[iMinTTL];
}

UInt32 plParticleSystem::AddEmitter(UInt32 maxParticles, plParticleGenerator *gen, UInt32 emitterFlags)
{
	if (fMaxEmitters == 0) // silly rabbit, Trix are for kids!
		return 0;
	UInt32 currEmitter;
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
	if (maxParticles < 0)
		maxParticles = 0;

	fEmitters[currEmitter] = TRACKED_NEW plParticleEmitter();
	fEmitters[currEmitter]->Init(this, maxParticles, fNextEmitterToGo, emitterFlags, gen);

	fMaxTotalParticlesLeft -= fEmitters[currEmitter]->fMaxParticles;
	fNextEmitterToGo = (fNextEmitterToGo + 1) % fMaxEmitters;

	return maxParticles;
}

void plParticleSystem::AddParticle(hsPoint3 &pos, hsVector3 &velocity, UInt32 tileIndex, 
								   hsScalar hSize, hsScalar vSize, hsScalar scale, hsScalar invMass, hsScalar life,
								   hsPoint3 &orientation, UInt32 miscFlags, hsScalar radsPerSec)
{
	hsAssert(fNumValidEmitters > 0, "Trying to explicitly add particles to a system with no valid emitters.");
	if (fNumValidEmitters == 0)
		return;

	fEmitters[0]->AddParticle(pos, velocity, tileIndex, hSize, vSize, scale, invMass, life, orientation, miscFlags, radsPerSec);
}

void plParticleSystem::GenerateParticles(UInt32 num, hsScalar dt /* = 0.f */)
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

void plParticleSystem::KillParticles(hsScalar num, hsScalar timeToDie, UInt8 flags)
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

UInt16 plParticleSystem::StealParticlesFrom(plParticleSystem *victim, UInt16 num)
{
	if (fNumValidEmitters <= 0)
		return 0; // you just lose

	if (victim)
	{
		UInt16 numStolen = fEmitters[0]->StealParticlesFrom(victim->fNumValidEmitters > 0 ? victim->fEmitters[0] : nil, num);
		GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);	
		victim->GetTarget(0)->DirtySynchState(kSDLParticleSystem, 0);	
		return numStolen;
	}
	
	return 0;
}

plParticleGenerator *plParticleSystem::GetExportedGenerator() const 
{ 
	return (fNumValidEmitters > 0 ? fEmitters[0]->fGenerator : nil);
}

plParticleEffect *plParticleSystem::GetEffect(UInt16 type) const
{
	int i;
	for (i = 0; i < fForces.GetCount(); i++)
		if (fForces[i]->ClassIndex() == type)
			return fForces[i];

	for (i = 0; i < fEffects.GetCount(); i++)
		if (fEffects[i]->ClassIndex() == type)
			return fEffects[i];

	for (i = 0; i < fConstraints.GetCount(); i++)
		if (fConstraints[i]->ClassIndex() == type)
			return fConstraints[i];

	return nil;
}

UInt32 plParticleSystem::GetNumValidParticles(hsBool immortalOnly /* = false */) const
{
	UInt32 count = 0;
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

hsBool plParticleSystem::IEval(double secs, hsScalar del, UInt32 dirty)
{
	return false;
}



hsBool plParticleSystem::IShouldUpdate(plPipeline* pipe) const
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
	hsBool isVisible = pipe->TestVisibleWorld(wBnd);

	hsScalar delta = fLastTime > 0 ? hsScalar(fCurrTime - fLastTime) : hsTimer::GetDelSysSeconds();
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
		hsScalar eyeDist = pipe->GetViewDirWorld().InnerProduct(pipe->GetViewPositionWorld());
		hsScalar dist = depth.fX - eyeDist;

		static hsScalar kUpdateCutoffDist = 100.f;
		if( dist > kUpdateCutoffDist )
		{
			static hsScalar kDistantUpdateSecs = 0.1f;
			return delta >= kDistantUpdateSecs;
		}
#endif // ALWAYS_IF_VISIBLE

		return true;
	}
	

	static hsScalar kOffscreenUpdateSecs = 1.f;
	return delta >= kOffscreenUpdateSecs;
}

plDrawInterface* plParticleSystem::ICheckDrawInterface()
{
	plDrawInterface* di = IGetTargetDrawInterface(0);
	if( !di )
		return nil;

	if( di->GetDrawableMeshIndex(0) == UInt32(-1) )
	{
		di->SetUpForParticleSystem( fMaxEmitters + 1, fMaxTotalParticles, fTexture, fPermaLights );
		hsAssert(di->GetDrawableMeshIndex( 0 ) != (UInt32)-1, "SetUpForParticleSystem should never fail"); // still invalid, didn't fix it.
	}

	return di;
}

void plParticleSystem::IHandleRenderMsg(plPipeline* pipe)
{	
	fCurrTime = hsTimer::GetSysSeconds(); 
	hsScalar delta = hsScalar(fCurrTime - fLastTime);
	if (delta == 0)
		return;
	plConst(hsScalar) kMaxDelta(0.3f);
	if( delta > kMaxDelta )
		delta = kMaxDelta;

	plDrawInterface* di = ICheckDrawInterface();
	if( !di )
		return;

	hsBool disabled = di->GetProperty(plDrawInterface::kDisable);
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
	
//	plParticleFiller::FillParticlePolys(pipe, di);

	fLastTime = fCurrTime;
}


#include "plProfile.h"
plProfile_CreateTimer("ParticleSys", "RenderSetup", ParticleSys);


hsBool plParticleSystem::MsgReceive(plMessage* msg)
{
	plGenRefMsg* refMsg;
	plParticleUpdateMsg *partMsg;
	plParticleKillMsg *killMsg;
	plSceneObject *scene;
	plParticleEffect *effect;
	hsGMaterial *mat;
	plRenderMsg *rend;
	plAgeLoadedMsg* ageLoaded;

	if (rend = plRenderMsg::ConvertNoRef(msg))
	{
		plProfile_BeginLap(ParticleSys, this->GetKey()->GetUoid().GetObjectName());
		IHandleRenderMsg(rend->Pipeline());
		plProfile_EndLap(ParticleSys, this->GetKey()->GetUoid().GetObjectName());
		return true;
	}
	else if (refMsg = plGenRefMsg::ConvertNoRef(msg))
	{
		if (scene = plSceneObject::ConvertNoRef(refMsg->GetRef()))
		{
			if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				AddTarget(scene);
			else
				RemoveTarget(scene);
			return true;
		}
		if (mat = hsGMaterial::ConvertNoRef(refMsg->GetRef()))
		{
			if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				fTexture = mat;
			else
				fTexture = nil;
			return true;
		}
		if (effect = plParticleEffect::ConvertNoRef(refMsg->GetRef()))
		{
			if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				IAddEffect(effect, refMsg->fType);
			//else
			//	IRemoveEffect(effect, refMsg->fType);
			return true;
		}
	}
	else if (partMsg = plParticleUpdateMsg::ConvertNoRef(msg))
	{
		UpdateGenerator(partMsg->GetParamID(), partMsg->GetParamValue());
		return true;
	}
	else if (killMsg = plParticleKillMsg::ConvertNoRef(msg)) 
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

void plParticleSystem::UpdateGenerator(UInt32 paramID, hsScalar value)
{
	int i;
	for (i = 0; i < fNumValidEmitters; i++)
		fEmitters[i]->UpdateGenerator(paramID, value);
}

void plParticleSystem::AddTarget(plSceneObject *so)
{
	if (fTarget != nil)
		RemoveTarget(fTarget);

	fTarget = so;
	plgDispatch::Dispatch()->RegisterForExactType(plTimeMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plRenderMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());

	delete fParticleSDLMod;
	fParticleSDLMod = TRACKED_NEW plParticleSDLMod;
	fParticleSDLMod->SetAttachedToAvatar(fAttachedToAvatar);
	so->AddModifier(fParticleSDLMod);
}

void plParticleSystem::RemoveTarget(plSceneObject *so)
{
	if (so == fTarget && so != nil)
	{
		if (fParticleSDLMod)
		{
			so->RemoveModifier(fParticleSDLMod);
			delete fParticleSDLMod;
			fParticleSDLMod = nil;
		}
		fTarget = nil;
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
			fEmitters[i]->IUpdateParticles((hsScalar)PRESIM_UPDATE_TICK);
			secs -= PRESIM_UPDATE_TICK;
		}
	}

	fPreSim = 0;
}

void plParticleSystem::IReadEffectsArray(hsTArray<plParticleEffect *> &effects, UInt32 type, hsStream *s, hsResMgr *mgr)
{
	plGenRefMsg *msg;
	effects.Reset();
	UInt32 count = s->ReadSwap32();
	int i;
	for (i = 0; i < count; i++)
	{
		msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, (Int8)type);
		mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
	}
}

void plParticleSystem::Read(hsStream *s, hsResMgr *mgr)
{
	plModifier::Read(s, mgr);

	plGenRefMsg* msg;
	msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0); // Material
	mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);

	fAmbientCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));	
	fDiffuseCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fOpacityCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fWidthCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));
	fHeightCtl = plController::ConvertNoRef(mgr->ReadCreatable(s));

	UInt32 xTiles = s->ReadSwap32();
	UInt32 yTiles = s->ReadSwap32();
	UInt32 maxTotal = s->ReadSwap32();
	UInt32 maxEmitters = s->ReadSwap32();
	Init(xTiles, yTiles, maxTotal, maxEmitters, fAmbientCtl, fDiffuseCtl, fOpacityCtl, fWidthCtl, fHeightCtl);

	fPreSim = s->ReadSwapScalar();
	fAccel.Read(s);
	fDrag = s->ReadSwapScalar();
	fWindMult = s->ReadSwapScalar();

	fNumValidEmitters = s->ReadSwap32();
	int i;
	for (i = 0; i < fNumValidEmitters; i++)
	{
		fEmitters[i] = plParticleEmitter::ConvertNoRef(mgr->ReadCreatable(s));
		fEmitters[i]->ISetSystem(this);
	}

	IReadEffectsArray(fForces, kEffectForce, s, mgr);
	IReadEffectsArray(fEffects, kEffectMisc, s, mgr);
	IReadEffectsArray(fConstraints, kEffectConstraint, s, mgr);

	int count = s->ReadSwap32();
	fPermaLights.SetCount(count);
	for( i = 0; i < count; i++ )
	{
		fPermaLights[i] = mgr->ReadKey(s);
	}
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
	
	s->WriteSwap32(fXTiles);
	s->WriteSwap32(fYTiles);
	s->WriteSwap32(fMaxTotalParticles);
	s->WriteSwap32(fMaxEmitters);

	s->WriteSwapScalar(fPreSim);
	fAccel.Write(s);
	s->WriteSwapScalar(fDrag);
	s->WriteSwapScalar(fWindMult);

	s->WriteSwap32(fNumValidEmitters);
	for (i = 0; i < fNumValidEmitters; i++)
	{
		mgr->WriteCreatable(s, fEmitters[i]);
	}

	int count;
	count = fForces.GetCount();
	s->WriteSwap32(count);
	for (i = 0; i < count; i++)
		mgr->WriteKey(s, fForces.Get(i));

	count = fEffects.GetCount();
	s->WriteSwap32(count);
	for (i = 0; i < count; i++)
		mgr->WriteKey(s, fEffects.Get(i));

	count = fConstraints.GetCount();
	s->WriteSwap32(count);
	for (i = 0; i < count; i++)
		mgr->WriteKey(s, fConstraints.Get(i));

	count = fPermaLights.GetCount();
	s->WriteSwap32(count);
	for( i = 0; i < count; i++ )
		mgr->WriteKey(s, fPermaLights[i]);
}

void plParticleSystem::SetAttachedToAvatar(bool attached)
{
	fAttachedToAvatar = attached;
	if (fParticleSDLMod)
		fParticleSDLMod->SetAttachedToAvatar(attached);
}

void plParticleSystem::AddLight(plKey liKey)
{
	fPermaLights.Append(liKey);
}