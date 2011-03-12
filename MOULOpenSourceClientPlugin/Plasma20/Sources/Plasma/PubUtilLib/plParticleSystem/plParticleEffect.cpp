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
#include "hsGeometry3.h"
#include "plParticle.h"
#include "plParticleEffect.h"
#include "plEffectTargetInfo.h"
#include "plConvexVolume.h"
#include "plBoundInterface.h"
#include "hsResMgr.h"
#include "plPipeline.h"
#include "hsFastMath.h"
#include "../plMath/plRandom.h"
#include "plParticleSystem.h"
#include "../plMessage/plParticleUpdateMsg.h"

///////////////////////////////////////////////////////////////////////////////////////////
plParticleCollisionEffect::plParticleCollisionEffect()
{
	fBounds = nil;
	fSceneObj = nil;
}

plParticleCollisionEffect::~plParticleCollisionEffect()
{
}

void plParticleCollisionEffect::PrepareEffect(const plEffectTargetInfo &target)
{
	if (fBounds == nil)
	{
		plBoundInterface *bi = plBoundInterface::ConvertNoRef(fSceneObj->GetGenericInterface(plBoundInterface::Index()));
		if (bi == nil)
			return;
		fBounds = bi->GetVolume();
	}
}

hsBool plParticleCollisionEffect::MsgReceive(plMessage* msg)
{
	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	plSceneObject *so;
	if (refMsg)
	{
		if (so = plSceneObject::ConvertNoRef(refMsg->GetRef()))
		{
			if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
				fSceneObj = so;
			else
				fSceneObj = nil;
			return true;
		}
	}
	return hsKeyedObject::MsgReceive(msg);
}

void plParticleCollisionEffect::Read(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Read(s, mgr);

	plGenRefMsg* msg;
	msg = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0); // SceneObject
	mgr->ReadKeyNotifyMe(s, msg, plRefFlags::kActiveRef);
	fBounds = nil;
}

void plParticleCollisionEffect::Write(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Write(s, mgr);

	mgr->WriteKey(s, fSceneObj);
}

///////////////////////////////////////////////////////////////////////////////////////////
// Some permutations on the CollisionEffect follow
///////////////////////////////////////////////////////////////////////////////////////////

plParticleCollisionEffectBeat::plParticleCollisionEffectBeat()
{
}

hsBool plParticleCollisionEffectBeat::ApplyEffect(const plEffectTargetInfo &target, Int32 i)
{
	hsAssert(i >= 0, "Use of default argument doesn't make sense for plParticleCollisionEffect");

	if( !fBounds )
		return false;

	hsPoint3 *currPos = (hsPoint3 *)(target.fPos + i * target.fPosStride);
	fBounds->ResolvePoint(*currPos);	

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////

plParticleCollisionEffectDie::plParticleCollisionEffectDie()
{
}

hsBool plParticleCollisionEffectDie::ApplyEffect(const plEffectTargetInfo &target, Int32 i)
{
	hsAssert(i >= 0, "Use of default argument doesn't make sense for plParticleCollisionEffect");

	if( !fBounds )
		return false;

	hsPoint3 *currPos = (hsPoint3 *)(target.fPos + i * target.fPosStride);
	return fBounds->IsInside(*currPos);	
}

///////////////////////////////////////////////////////////////////////////////////////////

plParticleCollisionEffectBounce::plParticleCollisionEffectBounce()
:	fBounce(1.f),
	fFriction(0.f)
{
}

hsBool plParticleCollisionEffectBounce::ApplyEffect(const plEffectTargetInfo &target, Int32 i)
{
	hsAssert(i >= 0, "Use of default argument doesn't make sense for plParticleCollisionEffect");

	if( !fBounds )
		return false;

	hsPoint3* currPos = (hsPoint3 *)(target.fPos + i * target.fPosStride);
	hsVector3* currVel = (hsVector3*)(target.fVelocity + i * target.fVelocityStride);
	fBounds->BouncePoint(*currPos, *currVel, fBounce, fFriction);	

	return false;
}

void plParticleCollisionEffectBounce::Read(hsStream *s, hsResMgr *mgr)
{
	plParticleCollisionEffect::Read(s, mgr);

	fBounce = s->ReadSwapScalar();
	fFriction = s->ReadSwapScalar();
}

void plParticleCollisionEffectBounce::Write(hsStream *s, hsResMgr *mgr)
{
	plParticleCollisionEffect::Write(s, mgr);

	s->WriteSwapScalar(fBounce);
	s->WriteSwapScalar(fFriction);
}



///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

plParticleFadeVolumeEffect::plParticleFadeVolumeEffect() : fLength(100.0f), fIgnoreZ(true)
{
}

plParticleFadeVolumeEffect::~plParticleFadeVolumeEffect()
{
}

//
// It's not really clear looking at the math here what's actually going on,
// but once you visualize it, it's pretty easy to follow. So the camera position,
// view direction, and length of the fade volume define a sphere, where the camera
// position is a point on the sphere, the view direction points from that surface
// point toward the center, and the length is the sphere's radius. Since the view
// direction points straight through the sphere, that sphere is the sweet zone for
// putting our particles to pile them up in front of the camera. But we'd like to
// do this independently for each axis (for efficiency, rather than true 3D calculations),
// so we put an axis aligned box around the sphere, and that's the volume we restrict
// our particles to.
// Now we could fade all around the box, but that's not really what we want, because
// that means fading particles that are behind us. And in the case where we're looking
// along an axis, the camera is pushed up against a face of the box (where the axis
// aligned box is tangent to the inscribed sphere), so we'd actually be fading 
// particles just in front of the camera. Because of this non-symmetry, we're going to
// define the Max in a given dimension as the world space value for that dimension 
// FARTHEST from the camera (NOT largest in value). So if the camera is looking
// in a negative direction in one dimension, the Max will be less than the Min for
// that dimension.
// So we set up our Max's and Min's for each dimension in PrepareEffect(), and then
// at runtime we calculate the parameter value of the particle ranging from 0 where
// particleLoc == Min to 1 where particleLoc == Max. If the parameter is outside
// [0..1], then we can move it into the box using the fractional part of the parameter.
// Finally, if the (possibly relocated) parameter value says the particle is approaching
// the Max value, we can calclulate its faded opacity from the parameter.
//

// Need to experiment to minimize this fade distance. The greater
// the fade distance, the more faded out (wasted) particles we're drawing.
// The shorter the distance, the more noticable the fade out.
// Note the wierdness between the fractions, because kFadeFrac is fraction
// of fLength, but kFadeParm and kInvFadeFrac are fraction of 2.f*fLength. Sorry.
const hsScalar kFadeFrac = 0.5f;
const hsScalar kFadeParm = 1.f - kFadeFrac * 0.5f;
const hsScalar kInvFadeFrac = 1.f / (kFadeFrac * 0.5f);

void plParticleFadeVolumeEffect::PrepareEffect(const plEffectTargetInfo &target)
{
	hsPoint3 viewLoc = target.fContext.fPipeline->GetViewPositionWorld();
	hsVector3 viewDir = target.fContext.fPipeline->GetViewDirWorld();

	// Nuking out the setting of viewDir.fZ to 0 when we aren't centering
	// about Z (fIgnoreZ == true), because we still want to center our
	// volume about the camera (rather than push the camera to the edge of
	// the cylinder) in that case, so we don't get artifacts when we look
	// straight up or down. mf

	hsPoint3 signs(viewDir.fX >= 0 ? 1.f : -1.f, viewDir.fY >= 0 ? 1.f : -1.f, viewDir.fZ >= 0 ? 1.f : -1.f);

	fMax.fX = viewLoc.fX + (viewDir.fX + signs.fX) * fLength;
	fMin.fX = fMax.fX - 2.f * signs.fX * fLength;

	fMax.fY = viewLoc.fY + (viewDir.fY + signs.fY) * fLength;
	fMin.fY = fMax.fY - 2.f * signs.fY * fLength;

	fMax.fZ = viewLoc.fZ + (viewDir.fZ + signs.fZ) * fLength;
	fMin.fZ = fMax.fZ - 2.f * signs.fZ * fLength;

	fNorm.fX = 1.f / (fMax.fX - fMin.fX);
	fNorm.fY = 1.f / (fMax.fY - fMin.fY);
	fNorm.fZ = 1.f / (fMax.fZ - fMin.fZ);
}

hsBool plParticleFadeVolumeEffect::ApplyEffect(const plEffectTargetInfo& target, Int32 i)
{
	hsPoint3 *currPos = (hsPoint3 *)(target.fPos + i * target.fPosStride);

	hsScalar parm;

	hsScalar fade = 1.f;

	parm = (currPos->fX - fMin.fX) * fNorm.fX;
	if( parm < 0 )
	{
		parm -= int(parm);
		currPos->fX = fMax.fX + parm * (fMax.fX - fMin.fX);
		parm += 1.f;
	}
	else if( parm > 1.f )
	{
		parm -= int(parm);
		currPos->fX = fMin.fX + parm * (fMax.fX - fMin.fX);
	}
	if( parm > kFadeParm )
	{
		parm = 1.f - parm;
		parm *= kInvFadeFrac;
		if( parm < fade )
			fade = parm;
	}

	parm = (currPos->fY - fMin.fY) * fNorm.fY;
	if( parm < 0 )
	{
		parm -= int(parm);
		currPos->fY = fMax.fY + parm * (fMax.fY - fMin.fY);
		parm += 1.f;
	}
	else if( parm > 1.f )
	{
		parm -= int(parm);
		currPos->fY = fMin.fY + parm * (fMax.fY - fMin.fY);
	}
	if( parm > kFadeParm )
	{
		parm = 1.f - parm;
		parm *= kInvFadeFrac;
		if( parm < fade )
			fade = parm;
	}

	if( !fIgnoreZ )
	{
		parm = (currPos->fZ - fMin.fZ) * fNorm.fZ;
		if( parm < 0 )
		{
			parm -= int(parm);
			currPos->fZ = fMax.fZ + parm * (fMax.fZ - fMin.fZ);
			parm += 1.f;
		}
		else if( parm > 1.f )
		{
			parm -= int(parm);
			currPos->fZ = fMin.fZ + parm * (fMax.fZ - fMin.fZ);
		}
		if( parm > kFadeParm )
		{
			parm = 1.f - parm;
			parm *= kInvFadeFrac;
			if( parm < fade )
				fade = parm;
		}
	}

	if( fade < 1.f )
	{
		UInt32 *color = (UInt32 *)(target.fColor + i * target.fColorStride);
		UInt32 alpha = (UInt32)((*color >> 24) * fade);
		*color = (*color & 0x00ffffff) | (alpha << 24);
	}

	return false;
}

void plParticleFadeVolumeEffect::Read(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Read(s, mgr);

	fLength = s->ReadSwapScalar();
	fIgnoreZ = s->ReadBool();
}

void plParticleFadeVolumeEffect::Write(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Write(s, mgr);

	s->WriteSwapScalar(fLength);
	s->WriteBool(fIgnoreZ);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Particle wind - Base class first
plParticleWindEffect::plParticleWindEffect()
:	fWindVec(0,0,0),
	fDir(1.f,0,0),
	fSwirl(0.1f),
	fConstancy(0),
	fHorizontal(0),
	fLastDirSecs(-1.f),
	fRefDir(0.f,0.f,0.f),
	fRandDir(1.f,0.f,0.f)
{
}

plParticleWindEffect::~plParticleWindEffect()
{
}

void plParticleWindEffect::Read(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Read(s, mgr);

	fStrength = s->ReadSwapScalar();
	fConstancy = s->ReadSwapScalar();
	fSwirl = s->ReadSwapScalar();
	fHorizontal = s->ReadBool();
	fRefDir.Read(s);
	fDir.Read(s);
	fRandDir = fDir;
}

void plParticleWindEffect::Write(hsStream *s, hsResMgr *mgr)
{
	hsKeyedObject::Write(s, mgr);

	s->WriteSwapScalar(fStrength);
	s->WriteSwapScalar(fConstancy);
	s->WriteSwapScalar(fSwirl);
	s->WriteBool(fHorizontal);
	fRefDir.Write(s);
	fDir.Write(s);
}

void plParticleWindEffect::SetRefDirection(const hsVector3& v)
{
	fRefDir = v;
	hsScalar lenSq = fRefDir.MagnitudeSquared();
	if( lenSq > 1.e-1f )
	{
		fDir = fRefDir * hsFastMath::InvSqrtAppr(lenSq);
		fRandDir = fDir;
	}
}

void plParticleWindEffect::PrepareEffect(const plEffectTargetInfo& target)
{
	if( fLastDirSecs != target.fContext.fSecs )
	{
		static plRandom random;
		fRandDir.fX += random.RandMinusOneToOne() * fSwirl;
		fRandDir.fY += random.RandMinusOneToOne() * fSwirl;
		if( !GetHorizontal() )
			fRandDir.fZ += random.RandMinusOneToOne() * fSwirl;
		hsFastMath::NormalizeAppr(fRandDir);
		fDir = fRandDir + fRefDir;
		hsFastMath::NormalizeAppr(fDir);

		fWindVec = fDir * (fStrength * target.fContext.fSystem->GetWindMult() * target.fContext.fDelSecs);

		fLastDirSecs = target.fContext.fSecs;
	}
}

////////////////////////////////////////////////////////////////////////
// Localized wind (how much wind you're getting depends on where you are
plParticleLocalWind::plParticleLocalWind()
:	fScale(0, 0, 0),
	fSpeed(0),
	fPhase(0,0,0),
	fInvScale(0,0,0),
	fLastPhaseSecs(-1.f)
{
}

plParticleLocalWind::~plParticleLocalWind()
{
}

void plParticleLocalWind::Read(hsStream *s, hsResMgr *mgr)
{
	plParticleWindEffect::Read(s, mgr);

	fScale.Read(s);
	fSpeed = s->ReadSwapScalar();
}

void plParticleLocalWind::Write(hsStream *s, hsResMgr *mgr)
{
	plParticleWindEffect::Write(s, mgr);

	fScale.Write(s);
	s->WriteSwapScalar(fSpeed);
}

void plParticleLocalWind::PrepareEffect(const plEffectTargetInfo& target)
{
	if( fLastPhaseSecs != target.fContext.fSecs )
	{
		plParticleWindEffect::PrepareEffect(target);

		fPhase += fDir * fSpeed * target.fContext.fDelSecs;

		fInvScale.fX = fScale.fX > 0 ? 1.f / fScale.fX : 0;
		fInvScale.fY = fScale.fY > 0 ? 1.f / fScale.fY : 0;
		fInvScale.fZ = fScale.fZ > 0 ? 1.f / fScale.fZ : 0;

		fLastPhaseSecs = target.fContext.fSecs;
	}
}


hsBool plParticleLocalWind::ApplyEffect(const plEffectTargetInfo& target, Int32 i)
{
	const hsPoint3& pos = *(hsPoint3 *)(target.fPos + i * target.fPosStride);
	hsVector3& vel = *(hsVector3*)(target.fVelocity + i * target.fVelocityStride);

	const hsScalar kMinToBother = 0;

	float strength = 1.f / ( (1.f + fConstancy) * (1.f + fConstancy) );
	float s, c, t;
	t = (pos[0] - fPhase[0]) * fInvScale[0];
	hsFastMath::SinCosAppr(t, s, c);
	c += fConstancy;
	if( c <= kMinToBother )
		return false;
	strength *= c;

	t = (pos[1] - fPhase[1]) * fInvScale[1];
	hsFastMath::SinCosAppr(t, s, c);
	c += fConstancy;
	if( c <= kMinToBother )
		return false;
	strength *= c;

#if 0 // if you turn this back on, strength needs to drop by another factor of (1.f + fConstancy)
	t = (pos[2] - fPhase[2]) * fInvScale[2];
	hsFastMath::SinCosAppr(t, s, c);
	c += fConstancy;
	if( c <= kMinToBother )
		return false;
	strength *= c;
#endif

	const hsScalar& invMass = *(hsScalar*)(target.fInvMass + i * target.fInvMassStride);
	strength *= invMass;

	vel += fWindVec * strength;

	return false;
}

////////////////////////////////////////////////////////////////////////
// Uniform wind - wind changes over time, but not space
plParticleUniformWind::plParticleUniformWind()
:	fFreqMin(0.1f),
	fFreqMax(0.2f),
	fFreqCurr(0.1f),
	fFreqRate(0.05f),

	fCurrPhase(0),
	fLastFreqSecs(-1.f),
	fCurrentStrength(0)
{
}

plParticleUniformWind::~plParticleUniformWind()
{
}

void plParticleUniformWind::Read(hsStream *s, hsResMgr *mgr)
{
	plParticleWindEffect::Read(s, mgr);

	fFreqMin = s->ReadSwapScalar();
	fFreqMax = s->ReadSwapScalar();
	fFreqRate = s->ReadSwapScalar();

#if 0
	fFreqMin = 1.f / 6.f;
	fFreqMax = 1.f / 1.f;
	fConstancy = -0.5f;
	fSwirl = 0.05f;
#endif

	fFreqCurr = fFreqMin;
}

void plParticleUniformWind::Write(hsStream *s, hsResMgr *mgr)
{
	plParticleWindEffect::Write(s, mgr);

	s->WriteSwapScalar(fFreqMin);
	s->WriteSwapScalar(fFreqMax);
	s->WriteSwapScalar(fFreqRate);
}

void plParticleUniformWind::SetFrequencyRange(hsScalar minSecsPerCycle, hsScalar maxSecsPerCycle)
{
	const hsScalar kMinSecsPerCycle = 1.f;
	if( minSecsPerCycle < kMinSecsPerCycle )
		minSecsPerCycle = kMinSecsPerCycle;
	if( minSecsPerCycle < kMinSecsPerCycle )
		minSecsPerCycle = kMinSecsPerCycle;

	if( minSecsPerCycle < maxSecsPerCycle )
	{
		fFreqMin = 1.f / maxSecsPerCycle;
		fFreqMax = 1.f / minSecsPerCycle;
	}
	else
	{
		fFreqMin = 1.f / minSecsPerCycle;
		fFreqMax = 1.f / maxSecsPerCycle;
	}
}

void plParticleUniformWind::SetFrequencyRate(hsScalar secsPerCycle)
{
	const hsScalar kMinSecsPerCycle = 1.f;
	if( secsPerCycle < kMinSecsPerCycle )
		secsPerCycle = kMinSecsPerCycle;
	fFreqRate = 1.f / secsPerCycle;
}


void plParticleUniformWind::PrepareEffect(const plEffectTargetInfo& target)
{
	plParticleWindEffect::PrepareEffect(target);

	if( fLastFreqSecs != target.fContext.fSecs )
	{
		static plRandom random;

		const double kTwoPi = hsScalarPI * 2.0;
		double t0 = fFreqCurr * fLastFreqSecs + fCurrPhase;
		hsScalar t1 = (hsScalar)fmod(t0, kTwoPi);
		fCurrPhase -= t0 - t1;

		fFreqCurr += fFreqRate * target.fContext.fDelSecs * random.RandZeroToOne();

		if( fFreqCurr > fFreqMax )
		{
			fFreqCurr = fFreqMax;
			fFreqRate = -fFreqRate;
		}
		else if( fFreqCurr < fFreqMin )
		{
			fFreqCurr = fFreqMin;
			fFreqRate = -fFreqRate;
		}
		
		hsScalar phaseDel = (hsScalar)(t1 - (fFreqCurr * fLastFreqSecs + fCurrPhase));
		fCurrPhase += phaseDel;
		
		hsScalar t = hsScalar(fFreqCurr * target.fContext.fSecs + fCurrPhase);
		hsScalar s;
		hsFastMath::SinCosAppr(t, s, fCurrentStrength);
		fCurrentStrength += fConstancy;
		fCurrentStrength /= (1.f + fConstancy);
		
		if( fCurrentStrength < 0 )
			fCurrentStrength = 0;
		
		fLastFreqSecs = target.fContext.fSecs;
	}
}


hsBool plParticleUniformWind::ApplyEffect(const plEffectTargetInfo& target, Int32 i)
{
	hsVector3& vel = *(hsVector3*)(target.fVelocity + i * target.fVelocityStride);
	
	const hsScalar& invMass = *(hsScalar*)(target.fInvMass + i * target.fInvMassStride);
	
	vel += fWindVec * (invMass * fCurrentStrength);
	
	return false;
}

////////////////////////////////////////////////////////////////////////
// Simplified flocking.

plParticleFlockEffect::plParticleFlockEffect() :
	fInfAvgRadSq(1),
	fInfRepRadSq(1),
	fAvgVelStr(1),
	fRepDirStr(1),
	fGoalOrbitStr(1),
	fGoalChaseStr(1),
	fGoalDistSq(1),
	fFullChaseDistSq(1),
	fMaxOrbitSpeed(1),
	fMaxChaseSpeed(1),
	fMaxParticles(0),
	fDistSq(nil),
	fInfluences(nil)
{
	fTargetOffset.Set(0.f, 0.f, 0.f);
	fDissenterTarget.Set(0.f, 0.f, 0.f);
}

plParticleFlockEffect::~plParticleFlockEffect()
{
	SetMaxParticles(0);
}

void plParticleFlockEffect::IUpdateDistances(const plEffectTargetInfo& target)
{
	int i, j;
	int numParticles = hsMinimum(fMaxParticles, target.fNumValidParticles);

	for (i = 0; i < numParticles; i++)
	{
		for (j = i + 1; j < numParticles; j++)
		{
			hsVector3 diff((hsPoint3*)(target.fPos + i * target.fPosStride), (hsPoint3*)(target.fPos + j * target.fPosStride));
			fDistSq[i * fMaxParticles + j] = fDistSq[j * fMaxParticles + i] = diff.MagnitudeSquared();
		}
	}
}

void plParticleFlockEffect::IUpdateInfluences(const plEffectTargetInfo &target)
{
	int i, j;
	int numParticles = hsMinimum(fMaxParticles, target.fNumValidParticles);
	
	for (i = 0; i < numParticles; i++)
	{
		int numAvg = 0;
		int numRep = 0;
		fInfluences[i].fAvgVel.Set(0.f, 0.f, 0.f);
		fInfluences[i].fRepDir.Set(0.f, 0.f, 0.f);

		for (j = 0; j < numParticles; j++)
		{
			if (i == j)
				continue;

			const int distIdx = i * fMaxParticles + j;
			if (fDistSq[distIdx] > fInfAvgRadSq)
			{
				numAvg++;
				fInfluences[i].fAvgVel += *(hsVector3*)(target.fVelocity + j * target.fVelocityStride);
			}

			if (fDistSq[distIdx] > fInfRepRadSq)
			{
				numRep++;
				hsVector3 repDir((hsPoint3*)(target.fPos + i * target.fPosStride), (hsPoint3*)(target.fPos + j * target.fPosStride));
				repDir.Normalize();
				fInfluences[i].fRepDir += repDir;
			}

		}

		if (numAvg > 0)
			fInfluences[i].fAvgVel /= (hsScalar)numAvg;
		if (numRep > 0)
			fInfluences[i].fRepDir /= (hsScalar)numRep;
	}
}

void plParticleFlockEffect::PrepareEffect(const plEffectTargetInfo& target)
{
	IUpdateDistances(target);
	IUpdateInfluences(target);
}

// Some of this is the same for every particle and should be cached in PrepareEffect().
// Holding off on that until I like the behavior.
hsBool plParticleFlockEffect::ApplyEffect(const plEffectTargetInfo& target, Int32 i)
{
	if (i >= fMaxParticles)
		return false; // Don't have the memory to deal with you. Good luck kid...

	const hsPoint3 &pos = *(hsPoint3*)(target.fPos + i * target.fPosStride);
	hsVector3 &vel = *(hsVector3*)(target.fVelocity + i * target.fVelocityStride);
	
	hsScalar curSpeed = vel.Magnitude();
	hsPoint3 goal;
	if (*(UInt32*)(target.fMiscFlags + i * target.fMiscFlagsStride) & plParticleExt::kImmortal)
		goal = target.fContext.fSystem->GetTarget(0)->GetLocalToWorld().GetTranslate() + fTargetOffset;
	else
		goal = fDissenterTarget;
	
	hsVector3 goalDir;
	goalDir.Set(&(goal - pos));
	hsScalar distSq = goalDir.MagnitudeSquared();
	
	goalDir.Normalize();
	
	hsScalar goalStr;
	hsScalar maxSpeed;
	hsScalar maxSpeedSq;
	if (distSq <= fGoalDistSq)
	{		
		goalStr = fGoalOrbitStr;
		if (i & 0x1)
			goalDir.Set(goalDir.fY, -goalDir.fX, goalDir.fZ);
		else
			goalDir.Set(-goalDir.fY, goalDir.fX, goalDir.fZ);

		maxSpeed = fMaxOrbitSpeed;
	}
	else if (distSq >= fFullChaseDistSq)
	{
		goalStr = fGoalChaseStr;
		maxSpeed = fMaxChaseSpeed;
	}
	else
	{
		hsScalar pct = (distSq - fGoalDistSq) / (fFullChaseDistSq - fGoalDistSq);
		goalStr = fGoalOrbitStr + (fGoalChaseStr - fGoalOrbitStr) * pct;
		maxSpeed = fMaxOrbitSpeed + (fMaxChaseSpeed - fMaxOrbitSpeed) * pct;
	}
	maxSpeedSq = maxSpeed * maxSpeed;
	
	vel += (fInfluences[i].fAvgVel - vel) * (fAvgVelStr * target.fContext.fDelSecs);
	vel += goalDir * (curSpeed * goalStr * target.fContext.fDelSecs);
	vel += fInfluences[i].fRepDir * (curSpeed * fRepDirStr * target.fContext.fDelSecs);

	if (vel.MagnitudeSquared() > maxSpeedSq)
	{
		vel.Normalize();
		vel *= maxSpeed;
	}

	return false;
}

void plParticleFlockEffect::SetMaxParticles(const UInt16 num)
{
	delete [] fDistSq;
	delete [] fInfluences;
	fMaxParticles = num;

	if (num > 0)
	{
		fDistSq = TRACKED_NEW hsScalar[num * num];
		fInfluences = TRACKED_NEW plParticleInfluenceInfo[num];
	}
}

void plParticleFlockEffect::Read(hsStream *s, hsResMgr *mgr)
{
	plParticleEffect::Read(s, mgr);

	fTargetOffset.Read(s);
	fDissenterTarget.Read(s);
	fInfAvgRadSq = s->ReadSwapScalar();
	fInfRepRadSq = s->ReadSwapScalar();
	fGoalDistSq = s->ReadSwapScalar();
	fFullChaseDistSq = s->ReadSwapScalar();
	fAvgVelStr = s->ReadSwapScalar();
	fRepDirStr = s->ReadSwapScalar();
	fGoalOrbitStr = s->ReadSwapScalar();
	fGoalChaseStr = s->ReadSwapScalar();
	SetMaxOrbitSpeed(s->ReadSwapScalar());
	SetMaxChaseSpeed(s->ReadSwapScalar());
	SetMaxParticles((UInt16)s->ReadSwapScalar());
}

void plParticleFlockEffect::Write(hsStream *s, hsResMgr *mgr)
{
	plParticleEffect::Write(s, mgr);

	fTargetOffset.Write(s);
	fDissenterTarget.Write(s);
	s->WriteSwapScalar(fInfAvgRadSq);
	s->WriteSwapScalar(fInfRepRadSq);
	s->WriteSwapScalar(fGoalDistSq);
	s->WriteSwapScalar(fFullChaseDistSq);
	s->WriteSwapScalar(fAvgVelStr);
	s->WriteSwapScalar(fRepDirStr);
	s->WriteSwapScalar(fGoalOrbitStr);
	s->WriteSwapScalar(fGoalChaseStr);
	s->WriteSwapScalar(fMaxOrbitSpeed);
	s->WriteSwapScalar(fMaxChaseSpeed);
	s->WriteSwapScalar(fMaxParticles);
}

hsBool plParticleFlockEffect::MsgReceive(plMessage *msg)
{
	plParticleFlockMsg *flockMsg = plParticleFlockMsg::ConvertNoRef(msg);
	if (flockMsg)
	{
		switch (flockMsg->fCmd)
		{
		case plParticleFlockMsg::kFlockCmdSetDissentPoint:
			fDissenterTarget.Set(flockMsg->fX, flockMsg->fY, flockMsg->fZ);
			break;
		case plParticleFlockMsg::kFlockCmdSetOffset:
			fTargetOffset.Set(flockMsg->fX, flockMsg->fY, flockMsg->fZ);
			break;
		default:
			break;
		}
		return true;
	}

	return plParticleEffect::MsgReceive(msg);
}

///////////////////////////////////////////////////////////////////////////////////////

plParticleFollowSystemEffect::plParticleFollowSystemEffect() : fEvalThisFrame(true)
{
	fOldW2L = hsMatrix44::IdentityMatrix();
}

void plParticleFollowSystemEffect::PrepareEffect(const plEffectTargetInfo& target)
{
	fEvalThisFrame = (fOldW2L != target.fContext.fSystem->GetTarget(0)->GetWorldToLocal());
}

hsBool plParticleFollowSystemEffect::ApplyEffect(const plEffectTargetInfo& target, Int32 i)
{
	if (fEvalThisFrame)
	{
		if (i < target.fFirstNewParticle && !fOldW2L.IsIdentity())
		{
			hsPoint3 &pos = *(hsPoint3*)(target.fPos + i * target.fPosStride);
			pos = target.fContext.fSystem->GetTarget(0)->GetLocalToWorld() * fOldW2L * pos;
		}
	}
	return true;
}

void plParticleFollowSystemEffect::EndEffect(const plEffectTargetInfo& target)
{
	if (fEvalThisFrame)
		fOldW2L = target.fContext.fSystem->GetTarget(0)->GetWorldToLocal();
}



