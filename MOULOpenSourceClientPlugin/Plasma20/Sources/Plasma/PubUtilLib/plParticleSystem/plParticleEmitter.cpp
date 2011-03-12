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
#include "hsUtils.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"
#include "../plMessage/plParticleUpdateMsg.h"
#include "plParticleGenerator.h"
#include "plParticleEmitter.h"
#include "plParticleSystem.h"
#include "plParticle.h"
#include "plParticleEffect.h"
#include "../CoreLib/hsColorRGBA.h"
#include "../plInterp/plController.h"
#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"
#include "plProfile.h"
#include "hsFastMath.h"

plProfile_CreateTimer("Update", "Particles", ParticleUpdate);
plProfile_CreateTimer("Generate", "Particles", ParticleGenerate);

plParticleEmitter::plParticleEmitter()
{
	fParticleCores = nil;
	fParticleExts = nil;
	fGenerator = nil;
	fLocalToWorld.Reset();
	fTimeToLive = 0;
}

void plParticleEmitter::Init(plParticleSystem *system, UInt32 maxParticles, UInt32 spanIndex, UInt32 miscFlags,
							 plParticleGenerator *gen /* = nil */)
{
	IClear();
	fSystem = system;
	plLayerInterface *layer = system->fTexture->GetLayer(0)->BottomOfStack();
	fMiscFlags = miscFlags | kNeedsUpdate;
	if( fMiscFlags & kOnReserve )
		fTimeToLive = -1.f; // Wait for someone to give us a spurt of life.
	if( layer->GetShadeFlags() & hsGMatState::kShadeEmissive )
	{
		fMiscFlags |= kMatIsEmissive;
		fColor = layer->GetAmbientColor();
	}
	else
	{
		fColor = layer->GetRuntimeColor();
	}
	fColor.a = layer->GetOpacity();
	fGenerator = gen;
	fMaxParticles = maxParticles;
	fSpanIndex = spanIndex;
	ISetupParticleMem();
}

void plParticleEmitter::Clone(plParticleEmitter* src, UInt32 spanIndex)
{
	Init(src->fSystem,
		src->fMaxParticles,
		spanIndex,
		src->fMiscFlags,
		src->fGenerator);

	fMiscFlags |= kBorrowedGenerator;
	fTimeToLive = -1.f;
}

void plParticleEmitter::OverrideLocalToWorld(const hsMatrix44& l2w)
{
	fLocalToWorld = l2w;
	fMiscFlags |= kOverrideLocalToWorld;
}

plParticleEmitter::~plParticleEmitter()
{
	IClear();
}

void plParticleEmitter::IClear()
{
	delete [] fParticleCores;
	fParticleCores = nil;
	delete [] fParticleExts;
	fParticleExts = nil;
	if( !(fMiscFlags & kBorrowedGenerator) )
		delete fGenerator;
	fGenerator = nil;
}

void plParticleEmitter::ISetupParticleMem()
{
	fNumValidParticles = 0;

	fParticleCores = TRACKED_NEW plParticleCore[fMaxParticles];
	fParticleExts = TRACKED_NEW plParticleExt[fMaxParticles];

	fTargetInfo.fPos = (UInt8 *)fParticleCores;
	fTargetInfo.fColor = (UInt8 *)fParticleCores + sizeof(hsPoint3);
	fTargetInfo.fPosStride = fTargetInfo.fColorStride = sizeof(plParticleCore);

	fTargetInfo.fVelocity = (UInt8 *)fParticleExts;
	fTargetInfo.fInvMass = fTargetInfo.fVelocity + sizeof(hsVector3);
	fTargetInfo.fAcceleration = fTargetInfo.fInvMass + sizeof(hsScalar);
	fTargetInfo.fMiscFlags = (UInt8 *)&(fParticleExts[0].fMiscFlags);	
	fTargetInfo.fRadsPerSec = (UInt8 *)&(fParticleExts[0].fRadsPerSec);
	fTargetInfo.fVelocityStride 
		= fTargetInfo.fInvMassStride 
		= fTargetInfo.fAccelerationStride 
		= fTargetInfo.fRadsPerSecStride
		= fTargetInfo.fMiscFlagsStride 
		= sizeof(plParticleExt);
}

UInt32 plParticleEmitter::GetNumTiles() const
{ 
	return fSystem->GetNumTiles(); 
}

const hsMatrix44 &plParticleEmitter::GetLocalToWorld() const
{ 
	return fMiscFlags & kOverrideLocalToWorld ? fLocalToWorld : fSystem->GetLocalToWorld(); 
}

void plParticleEmitter::AddParticle(hsPoint3 &pos, hsVector3 &velocity, UInt32 tileIndex, 
								    hsScalar hSize, hsScalar vSize, hsScalar scale, hsScalar invMass, hsScalar life,
									hsPoint3 &orientation, UInt32 miscFlags, hsScalar radsPerSec)
{
	plParticleCore *core;
	plParticleExt *ext;
	UInt32 currParticle;

	if (fNumValidParticles == fMaxParticles)
		return; // No more room... you lose!
	else
		currParticle = fNumValidParticles++;

	core = &fParticleCores[currParticle];	
	core->fPos = pos;
	core->fOrientation = orientation;
	core->fColor = CreateHexColor(fColor);
	core->fHSize = hSize * scale;
	core->fVSize = vSize * scale;

	// even if the kNormalUp flag isn't there, we should initialize it to something sane.
	//if (core->fMiscFlags & kNormalUp != 0)
		core->fNormal.Set(0, 0, 1);

	hsScalar xOff = (tileIndex % fSystem->fXTiles) / (float)fSystem->fXTiles;
	hsScalar yOff = (tileIndex / fSystem->fXTiles) / (float)fSystem->fYTiles;

	core->fUVCoords[0].fX = xOff;
	core->fUVCoords[0].fY = yOff + 1.0f / fSystem->fYTiles;
	core->fUVCoords[0].fZ = 1.0f;
	core->fUVCoords[1].fX = xOff + 1.0f / fSystem->fXTiles;
	core->fUVCoords[1].fY = yOff + 1.0f / fSystem->fYTiles;
	core->fUVCoords[1].fZ = 1.0f;
	core->fUVCoords[2].fX = xOff + 1.0f / fSystem->fXTiles;
	core->fUVCoords[2].fY = yOff;
	core->fUVCoords[2].fZ = 1.0f;
	core->fUVCoords[3].fX = xOff;
	core->fUVCoords[3].fY = yOff;
	core->fUVCoords[3].fZ = 1.0f;

	ext = &fParticleExts[currParticle];
	ext->fVelocity = velocity;
	ext->fInvMass = invMass;
	ext->fLife = ext->fStartLife = life;
	ext->fMiscFlags = miscFlags; // Is this ever NOT zero?
	if (life <= 0) 
		ext->fMiscFlags |= plParticleExt::kImmortal;

	ext->fRadsPerSec = radsPerSec;
	ext->fAcceleration.Set(0, 0, 0);
	ext->fScale = scale;
}

void plParticleEmitter::WipeExistingParticles()
{ 
	fNumValidParticles = 0;  // That was easy.
}

// This method is called from a network received message. Don't trust the args without checking.
void plParticleEmitter::KillParticles(hsScalar num, hsScalar timeToDie, UInt8 flags)
{
	if (flags & plParticleKillMsg::kParticleKillPercentage)
	{
		if (num < 0)
			num = 0;
		if (num > 1)
			num = 1;

		num *= fNumValidParticles;
	}
	else
	{
		if (num < 0)
			num = 0;
	}

	if (timeToDie < 0)
		timeToDie = 0;

	int i;
	for (i = 0; i < fNumValidParticles && num > 0; i++)
	{
		if ((flags & plParticleKillMsg::kParticleKillImmortalOnly) && !(fParticleExts[i].fMiscFlags & plParticleExt::kImmortal))
			continue;

		fParticleExts[i].fLife = fParticleExts[i].fStartLife = timeToDie;
		fParticleExts[i].fMiscFlags &= ~plParticleExt::kImmortal;
		num--;
	}
}

void plParticleEmitter::UpdateGenerator(UInt32 paramID, hsScalar paramValue)
{
	if (fGenerator != nil)
		fGenerator->UpdateParam(paramID, paramValue);
}

UInt16 plParticleEmitter::StealParticlesFrom(plParticleEmitter *victim, UInt16 num)
{
	UInt16 spaceAvail = (UInt16)(fMaxParticles - fNumValidParticles);
	UInt16 numToCopy = (UInt16)(hsMinimum(num, (victim ? victim->fNumValidParticles : 0)));
	if (spaceAvail < numToCopy)
		numToCopy = spaceAvail;

	if (numToCopy > 0)
	{
		// copy them over
		memcpy(&(fParticleCores[fNumValidParticles]), &(victim->fParticleCores[victim->fNumValidParticles - numToCopy]), numToCopy * sizeof(plParticleCore));
		memcpy(&(fParticleExts[fNumValidParticles]), &(victim->fParticleExts[victim->fNumValidParticles- numToCopy]), numToCopy * sizeof(plParticleExt));

		fNumValidParticles += numToCopy;
		victim->fNumValidParticles -= numToCopy;
	}

	return numToCopy;
}

void plParticleEmitter::TranslateAllParticles(hsPoint3 &amount)
{
	int i;
	for (i = 0; i < fNumValidParticles; i++)
		fParticleCores[i].fPos += amount;
}

hsBool plParticleEmitter::IUpdate(hsScalar delta)
{
	if (fMiscFlags & kNeedsUpdate)
	{
		plProfile_BeginTiming(ParticleUpdate);

		IUpdateParticles(delta);
		IUpdateBoundsAndNormals(delta);

		plProfile_EndTiming(ParticleUpdate);
	}

	if (fGenerator == nil && fNumValidParticles <= 0)
		return false; // no generator, no particles, let the system decide if this emitter is done
	else
		return true;
}

void plParticleEmitter::IUpdateParticles(hsScalar delta)
{

	int i, j;

	// Have to remove particles before adding new ones, or we can run out of room.
	for (i = 0; i < fNumValidParticles; i++)
	{
		fParticleExts[i].fLife -= delta;
		if (fParticleExts[i].fLife <= 0 && !(fParticleExts[i].fMiscFlags & plParticleExt::kImmortal))
		{
			IRemoveParticle(i);
			i--; // so that we hit this index again on the next iteration
			continue;
		}
	}

	fTargetInfo.fFirstNewParticle = fNumValidParticles;
	
	if ((fGenerator != nil) && (fTimeToLive >= 0))
	{
		plProfile_BeginLap(ParticleGenerate, fSystem->GetKeyName());
		if (!fGenerator->AddAutoParticles(this, delta))
		{
			delete fGenerator;
			fGenerator = nil;
		}
		if( (fTimeToLive > 0) && ((fTimeToLive -= delta) <= 0) )
			fTimeToLive = -1.f;
		plProfile_EndLap(ParticleGenerate, fSystem->GetKeyName());
	}

	fTargetInfo.fContext = fSystem->fContext;
	fTargetInfo.fNumValidParticles = fNumValidParticles;
	const hsVector3 up(0, 0, 1.0f);
	hsVector3 currDirection;
	hsPoint3 *currPos;
	hsVector3 *currVelocity;
	hsVector3 *currAccel;
	hsPoint3 color(fColor.r, fColor.g, fColor.b);
	hsScalar alpha = fColor.a;
	plController *colorCtl;

	// Allow effects a chance to cache any upfront calculations
	// that will apply to all particles.
	for (j = 0; j < fSystem->fForces.GetCount(); j++)
	{
		fSystem->fForces[j]->PrepareEffect(fTargetInfo);
	}
	for (j = 0; j < fSystem->fEffects.GetCount(); j++)
	{
		fSystem->fEffects[j]->PrepareEffect(fTargetInfo);
	}
	for (j = 0; j < fSystem->fConstraints.GetCount(); j++) 
	{
		fSystem->fConstraints[j]->PrepareEffect(fTargetInfo);
	}

	for (i = 0; i < fNumValidParticles; i++)
	{
		if (!( fParticleExts[i].fMiscFlags & plParticleExt::kImmortal ))
		{			
			hsScalar percent = (1.0f - fParticleExts[i].fLife / fParticleExts[i].fStartLife);
			colorCtl = (fMiscFlags & kMatIsEmissive ? fSystem->fAmbientCtl : fSystem->fDiffuseCtl);
			if (colorCtl != nil)
				colorCtl->Interp(colorCtl->GetLength() * percent, &color);

			if (fSystem->fOpacityCtl != nil)
			{
				fSystem->fOpacityCtl->Interp(fSystem->fOpacityCtl->GetLength() * percent, &alpha);
				alpha /= 100.0f;
				if (alpha < 0)
					alpha = 0;
				else if (alpha > 1.f)
					alpha = 1.f;
			}

			if (fSystem->fWidthCtl != nil)
			{
				fSystem->fWidthCtl->Interp(fSystem->fWidthCtl->GetLength() * percent,
										   &fParticleCores[i].fHSize);
				fParticleCores[i].fHSize *= fParticleExts[i].fScale;
			}
			if (fSystem->fHeightCtl != nil)
			{
				fSystem->fHeightCtl->Interp(fSystem->fHeightCtl->GetLength() * percent,
										    &fParticleCores[i].fVSize);
				fParticleCores[i].fVSize *= fParticleExts[i].fScale;
			}

			fParticleCores[i].fColor = CreateHexColor(color.fX, color.fY, color.fZ, alpha);						
		}

		for (j = 0; j < fSystem->fForces.GetCount(); j++)
		{
			fSystem->fForces[j]->ApplyEffect(fTargetInfo, i);
		}
		
		currPos = (hsPoint3 *)(fTargetInfo.fPos + i * fTargetInfo.fPosStride);
		currVelocity = (hsVector3 *)(fTargetInfo.fVelocity + i * fTargetInfo.fVelocityStride);
		//currAccel = (hsVector3 *)(fTargetInfo.fAcceleration + i * fTargetInfo.fAccelerationStride);
		currAccel = &fSystem->fAccel; // Nothing accellerates on a per-particle basis (yet)

		*currPos += *currVelocity * delta;

		// This is the only orientation option (so far) that requires an update here
		if (fMiscFlags & (kOrientationVelocityBased | kOrientationVelocityStretch | kOrientationVelocityFlow))
			fParticleCores[i].fOrientation.Set(&(*currVelocity * delta)); // mf - want the orientation to be a delposition
		else if( fParticleExts[i].fRadsPerSec != 0 )
		{
			hsScalar sinX, cosX;
			hsFastMath::SinCos(fParticleExts[i].fLife * fParticleExts[i].fRadsPerSec * 2.f * hsScalarPI, sinX, cosX);
			fParticleCores[i].fOrientation.Set(sinX, -cosX, 0);
		}

		// Viscous force F(t) = -k V(t)
		// Integral S from t0 to t1 of F(t) is
		// = S(-kV(t))[t1..t0]
		// = -k(P(t1) - P(t0))
		// = -k*(currVelocity * delta)
		// or
		// V = V + -k*(V * delta)
		// V *= (1 + -k * delta)
		// Giving the change in velocity.
		hsScalar drag = 1.f + fSystem->fDrag * delta;
		// Clamp it at 0. Drag should never cause a reversal in velocity direction.
		if( drag < 0.f )
			drag = 0.f;
		*currVelocity *= drag;

		*currVelocity += *currAccel * delta;

		for (j = 0; j < fSystem->fEffects.GetCount(); j++)
		{
			fSystem->fEffects[j]->ApplyEffect(fTargetInfo, i);
		}

		// We may need to do more than one iteration through the constraints. It's a trade-off
		// between accurracy and speed (what's new?) but I'm going to go with just one
		// for now until we decide things don't "look right"
		for (j = 0; j < fSystem->fConstraints.GetCount(); j++) 
		{
			if( fSystem->fConstraints[j]->ApplyEffect(fTargetInfo, i) )
			{
				IRemoveParticle(i);
				i--; // so that we hit this index again on the next iteration
				break; 
				// break will break us out of loop over constraints,
				// and since we're last, we move onto next particle.
			}
		}
	}

	// Notify the effects that they are done for now.
	for (j = 0; j < fSystem->fForces.GetCount(); j++)
	{
		fSystem->fForces[j]->EndEffect(fTargetInfo);
	}
	for (j = 0; j < fSystem->fEffects.GetCount(); j++)
	{
		fSystem->fEffects[j]->EndEffect(fTargetInfo);
	}
	for (j = 0; j < fSystem->fConstraints.GetCount(); j++) 
	{
		fSystem->fConstraints[j]->EndEffect(fTargetInfo);
	}
}

plProfile_CreateTimer("Bound", "Particles", ParticleBound);
plProfile_CreateTimer("Normal", "Particles", ParticleNormal);

void plParticleEmitter::IUpdateBoundsAndNormals(hsScalar delta)
{
	plProfile_BeginTiming(ParticleBound);
	fBoundBox.MakeEmpty();
	int i;
	for (i = 0; i < fNumValidParticles; i++)
		fBoundBox.Union(&fParticleCores[i].fPos);
	
	hsPoint3 center;
	if (fNumValidParticles > 0)
		center = fBoundBox.GetCenter();
	plProfile_EndTiming(ParticleBound);

	plProfile_BeginTiming(ParticleNormal);
	hsVector3 normal;
	if (fMiscFlags & kNormalVelUpVel)
	{
		for (i = fNumValidParticles - 1; i >=0; i--) 
		{
			//currDirection.Set(&fParticleCores[i].fPos, &fParticleExts[i].fOldPos);
			//normal = (currDirection % up % currDirection);
			normal.Set(-fParticleExts[i].fVelocity.fX * fParticleExts[i].fVelocity.fZ,
					   -fParticleExts[i].fVelocity.fY * fParticleExts[i].fVelocity.fZ,
					   (fParticleExts[i].fVelocity.fX * fParticleExts[i].fVelocity.fX + 
					    fParticleExts[i].fVelocity.fY * fParticleExts[i].fVelocity.fY));
			if (!normal.IsEmpty()) // zero length check
			{
				normal.Normalize();
				fParticleCores[i].fNormal = normal;
			}
		}
	}
	else if (fMiscFlags & kNormalFromCenter)
	{
		for (i = fNumValidParticles - 1; i >=0; i--) 
		{
			normal.Set(&fParticleCores[i].fPos, &center);
			if (!normal.IsEmpty()) // zero length check
			{
				normal.Normalize();
				fParticleCores[i].fNormal = normal;
			}
		}
	}
	// otherwise we just keep the last normal.
	plProfile_EndTiming(ParticleNormal);
}

void plParticleEmitter::IRemoveParticle(UInt32 index)
{
	hsAssert(index < fNumValidParticles, "Trying to remove an invalid particle");

	fNumValidParticles--;
	if (fNumValidParticles <= 0)
	{
		fNumValidParticles = 0;
		return;
	}

	fParticleCores[index] = fParticleCores[fNumValidParticles];
	fParticleExts[index] = fParticleExts[fNumValidParticles];
}

// Reading and writing doesn't transfer individual particle info. We assume those are expendable.
// Only the configuration info necessary to began spawning particles again is saved.
// The particle system that owns this emitter is responsible for setting the pointer back to itself
void plParticleEmitter::Read(hsStream *s, hsResMgr *mgr)
{
	plCreatable::Read(s, mgr);

	fGenerator = plParticleGenerator::ConvertNoRef(mgr->ReadCreatable(s));
	fSpanIndex = s->ReadSwap32();
	fMaxParticles = s->ReadSwap32();
	fMiscFlags = s->ReadSwap32();
	fColor.Read(s);

	if( fMiscFlags & kOnReserve )
		fTimeToLive = -1.f; // Wait for someone to give us a spurt of life.

	ISetupParticleMem();
}

void plParticleEmitter::Write(hsStream *s, hsResMgr *mgr)
{
	plCreatable::Write(s, mgr);

	mgr->WriteCreatable(s, fGenerator);
	s->WriteSwap32(fSpanIndex);
	s->WriteSwap32(fMaxParticles);
	s->WriteSwap32(fMiscFlags);
	fColor.Write(s);
}

UInt32 plParticleEmitter::CreateHexColor(const hsColorRGBA &color)
{
	return CreateHexColor(color.r, color.g, color.b, color.a);
}

UInt32 plParticleEmitter::CreateHexColor(const hsScalar r, const hsScalar g, const hsScalar b, const hsScalar a)
{
	UInt32		ru, gu, bu, au;

	au = (UInt32)(a * 255.0f);
	ru = (UInt32)(r * 255.0f);
	gu = (UInt32)(g * 255.0f);
	bu = (UInt32)(b * 255.0f);
	return ( au << 24 ) | ( ru << 16 ) | ( gu << 8 ) | ( bu );
}

