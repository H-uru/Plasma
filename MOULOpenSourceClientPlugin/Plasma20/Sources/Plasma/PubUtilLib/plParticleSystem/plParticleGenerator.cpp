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
#include "hsStream.h"
#include "hsFastMath.h"
#include "hsUtils.h"
#include "plParticle.h"
#include "plParticleSystem.h"
#include "plParticleEmitter.h"
#include "plParticleGenerator.h"
#include "../CoreLib/hsColorRGBA.h"
#include "../plMessage/plParticleUpdateMsg.h"
#include "../plInterp/plController.h"
#include "hsResMgr.h"
#include "../plMath/plRandom.h"

static const hsScalar DEFAULT_INVERSE_MASS = 1.f;

static plRandom sRandom;

const void plParticleGenerator::ComputeDirection(float pitch, float yaw, hsVector3 &direction)
{
	hsScalar cosPitch, sinPitch;
	hsScalar cosYaw, sinYaw;
	hsFastMath::SinCos(pitch, sinPitch, cosPitch);
	hsFastMath::SinCos(yaw, sinYaw, cosYaw);		

	direction.Set(-sinYaw * cosPitch, sinPitch, cosPitch * cosYaw);
}

// Inverse function of ComputeDirection. Give it a normalized vector, and it will tell you a
// pitch and yaw (angles for the unit Z vector) to get there.
const void plParticleGenerator::ComputePitchYaw(float &pitch, float &yaw, const hsVector3 &dir)
{
	const float PI = 3.14159f;
	pitch = asin(dir.fY);
	float cos_pitch = cos(pitch);
	if (cos_pitch == 0)
	{
		yaw = 0;
		return;
	}
	float inv = -dir.fX / cos_pitch;
	if (inv > 1.0f)
		inv = 1.0f;
	if (inv < -1.0f)
		inv = -1.0f;
	yaw = asin(inv);
	if (dir.fZ < 0)
		yaw = PI - yaw;
}

plSimpleParticleGenerator::plSimpleParticleGenerator()
{
}

plSimpleParticleGenerator::~plSimpleParticleGenerator()
{
	delete [] fInitPos;
	delete [] fInitPitch;
	delete [] fInitYaw;
}

void plSimpleParticleGenerator::Init(hsScalar genLife, hsScalar partLifeMin, hsScalar partLifeMax,
									 hsScalar particlesPerSecond, UInt32 numSources, hsPoint3 *initPos,
									 hsScalar *initPitch, hsScalar *initYaw, hsScalar angleRange, 
									 hsScalar initVelMin, hsScalar initVelMax,
									 hsScalar xSize, hsScalar ySize, 
									 hsScalar scaleMin, hsScalar scaleMax,
									 hsScalar massRange, hsScalar radsPerSecRange)
{
	fGenLife = genLife;
	fPartLifeMin = partLifeMin;
	fPartLifeMax = partLifeMax;
	fParticlesPerSecond = particlesPerSecond;
	fNumSources = numSources;
	fInitPos = initPos;
	fInitPitch = initPitch;
	fInitYaw = initYaw;
	fAngleRange = angleRange;
	fVelMin = initVelMin;
	fVelMax = initVelMax;
	fXSize = xSize;
	fYSize = ySize;
	fScaleMin = scaleMin;
	fScaleMax = scaleMax;

	fPartInvMassMin = 1.f / (DEFAULT_INVERSE_MASS + massRange);
	fPartInvMassRange = 1.f / DEFAULT_INVERSE_MASS - fPartInvMassMin;

	fPartRadsPerSecRange = radsPerSecRange;

	fParticleSum = 0;
	fMiscFlags = 0;
	if (fGenLife < 0) fMiscFlags |= kImmortal;
}

hsBool plSimpleParticleGenerator::AddAutoParticles(plParticleEmitter *emitter, float dt, UInt32 numForced /* = 0 */)
{
	Int32 numNewParticles;

	if (numForced == 0)
	{
		fGenLife -= dt;
		if ((fGenLife < 0 && !(fMiscFlags & kImmortal)) || (fMiscFlags & kDisabled))
			return true; // Leave it around so that a message can bring it back to life.

		fParticleSum += fParticlesPerSecond * dt;
		numNewParticles = (Int32)fParticleSum;
	
		if (numNewParticles <= 0 || fParticlesPerSecond == 0)
			return true;
	}
	else
	{
		numNewParticles = numForced;
	}

	UInt32 miscFlags = 0; 
	hsPoint3 currStart;
	fParticleSum -= numNewParticles;

	hsPoint3 orientation;
	hsVector3 initDirection;
	hsScalar vel = (fVelMax + fVelMin) * 0.5f;
	hsScalar velRange = vel - fVelMin;
	hsScalar initVelocity;
	hsScalar initLife;
	hsScalar life = (fPartLifeMax + fPartLifeMin) * 0.5f;
	hsScalar lifeRange = life - fPartLifeMin;
	hsScalar currSizeVar;
	hsScalar scale = (fScaleMax + fScaleMin) * 0.5f;
	hsScalar scaleRange = scale - fScaleMin;
	hsScalar radsPerSec = 0;
	UInt32 tile;
	UInt32 sourceIndex;

	const hsScalar lifeDiff = dt / numNewParticles;
	hsScalar lifeSoFar;
	int i;	
	for (i = 0, lifeSoFar = 0; i < numNewParticles; i++, lifeSoFar += lifeDiff)
	{
		initLife = life + lifeRange * sRandom.RandMinusOneToOne() - lifeSoFar;

		// Careful here... if we're supposed to generate immortal particles, we do so
		// by giving them a negative life. This is different that generating one with
		// a positive lifetime that is now negative because of "lifeSoFar". The if is
		// saying "if it's dead, but it was alive before we took away lifeSoFar, ignore it"
		if (initLife <= 0 && initLife + lifeSoFar >= 0)
			continue;

		sourceIndex = (UInt32)(sRandom.RandZeroToOne() * fNumSources);

		ComputeDirection(fInitPitch[sourceIndex] + fAngleRange * sRandom.RandMinusOneToOne(), 
						 fInitYaw[sourceIndex] + fAngleRange * sRandom.RandMinusOneToOne(), initDirection);
		initDirection = emitter->GetLocalToWorld() * initDirection;
		initVelocity = (vel + velRange * sRandom.RandMinusOneToOne());
		
		currStart = (emitter->GetLocalToWorld() * fInitPos[sourceIndex])
					+ (initDirection * initVelocity * lifeSoFar) // Vo * t
					+ (emitter->fSystem->fAccel * lifeSoFar * lifeSoFar); // at^2
		
		if (emitter->fMiscFlags & emitter->kOrientationUp)
			orientation.Set(0.0f, -1.0f, 0.0f);
		else
			orientation.Set(&initDirection);

		tile = (UInt32)(sRandom.RandZeroToOne() * emitter->GetNumTiles());
		currSizeVar = scale + scaleRange * sRandom.RandMinusOneToOne();

		hsScalar invMass = fPartInvMassMin;
		// Might be faster to just do the math instead of checking for zero...
		if( fPartInvMassRange > 0 )
			invMass += fPartInvMassRange * sRandom.RandZeroToOne();

		if( fPartRadsPerSecRange > 0 )
			radsPerSec = fPartRadsPerSecRange * sRandom.RandMinusOneToOne();

		emitter->AddParticle(currStart, initDirection * initVelocity, tile, fXSize, fYSize, currSizeVar, 
						 invMass, initLife, orientation, miscFlags, radsPerSec);
	}

	return true;
}

void plSimpleParticleGenerator::UpdateParam(UInt32 paramID, hsScalar paramValue)
{
	switch (paramID)
	{
	case plParticleUpdateMsg::kParamParticlesPerSecond:
		fParticlesPerSecond = paramValue;
		break;
	case plParticleUpdateMsg::kParamInitPitchRange:
	case plParticleUpdateMsg::kParamInitYawRange:
		fAngleRange = paramValue;
		break;
//	case plParticleUpdateMsg::kParamInitVel:
//		fInitVel = paramValue;
//		break;
//	case plParticleUpdateMsg::kParamInitVelRange:
//		fInitVelRange = paramValue;
//		break;
	case plParticleUpdateMsg::kParamVelMin:
		fVelMin = paramValue;
		break;
	case plParticleUpdateMsg::kParamVelMax:
		fVelMax = paramValue;
		break;
	case plParticleUpdateMsg::kParamXSize:
		fXSize = paramValue;
		break;
	case plParticleUpdateMsg::kParamYSize:
		fYSize = paramValue;
		break;
//	case plParticleUpdateMsg::kParamSizeRange:
//		fSizeRange = paramValue;
//		break;
	case plParticleUpdateMsg::kParamScaleMin:
		fScaleMin = paramValue;
		break;
	case plParticleUpdateMsg::kParamScaleMax:
		fScaleMax = paramValue;
		break;
	case plParticleUpdateMsg::kParamGenLife:
		fGenLife = paramValue;
		if (fGenLife < 0)
			fMiscFlags |= kImmortal;
		else
			fMiscFlags &= ~kImmortal;
		break;
//	case plParticleUpdateMsg::kParamPartLife:
//		fPartLife = paramValue;
//		if (fPartLife < 0)
//			fPartLifeRange = 0;
//		break;
//	case plParticleUpdateMsg::kParamPartLifeRange:
//		fPartLifeRange = paramValue;
//		break;
	case plParticleUpdateMsg::kParamPartLifeMin:
		fPartLifeMin = paramValue;
		break;
	case plParticleUpdateMsg::kParamPartLifeMax:
		fPartLifeMax = paramValue;
		break;
	case plParticleUpdateMsg::kParamEnabled:
		if (paramValue == 0.f)
			fMiscFlags |= kDisabled;
		else
			fMiscFlags &= ~kDisabled;
		break;
	default:
		break;
	}
}

void plSimpleParticleGenerator::Read(hsStream* s, hsResMgr *mgr)
{
	hsScalar genLife = s->ReadSwapScalar();
	hsScalar partLifeMin = s->ReadSwapScalar();
	hsScalar partLifeMax = s->ReadSwapScalar();
	hsScalar pps = s->ReadSwapScalar();
	UInt32 numSources = s->ReadSwap32();
	hsPoint3 *pos = TRACKED_NEW hsPoint3[numSources];
	hsScalar *pitch = TRACKED_NEW hsScalar[numSources];
	hsScalar *yaw = TRACKED_NEW hsScalar[numSources];
	int i;
	for (i = 0; i < numSources; i++)
	{
		pos[i].Read(s);
		pitch[i] = s->ReadSwapScalar();
		yaw[i] = s->ReadSwapScalar();
	}
	hsScalar angleRange = s->ReadSwapScalar();
	hsScalar velMin = s->ReadSwapScalar();
	hsScalar velMax = s->ReadSwapScalar();
	hsScalar xSize = s->ReadSwapScalar();
	hsScalar ySize = s->ReadSwapScalar();
	hsScalar scaleMin = s->ReadSwapScalar();
	hsScalar scaleMax = s->ReadSwapScalar();
	hsScalar massRange = s->ReadSwapScalar();
	hsScalar radsPerSec = s->ReadSwapScalar();

	Init(genLife, partLifeMin, partLifeMax, pps, numSources, pos, pitch, yaw, angleRange, velMin, velMax,
		 xSize, ySize, scaleMin, scaleMax, massRange, radsPerSec);
}

void plSimpleParticleGenerator::Write(hsStream* s, hsResMgr *mgr)
{
	s->WriteSwapScalar(fGenLife);
	s->WriteSwapScalar(fPartLifeMin);
	s->WriteSwapScalar(fPartLifeMax);
	s->WriteSwapScalar(fParticlesPerSecond);
	s->WriteSwap32(fNumSources);
	int i;
	for (i = 0; i < fNumSources; i++)
	{
		fInitPos[i].Write(s);
		s->WriteSwapScalar(fInitPitch[i]);
		s->WriteSwapScalar(fInitYaw[i]);
	}
	s->WriteSwapScalar(fAngleRange);
	s->WriteSwapScalar(fVelMin);
	s->WriteSwapScalar(fVelMax);
	s->WriteSwapScalar(fXSize);
	s->WriteSwapScalar(fYSize);
	s->WriteSwapScalar(fScaleMin);
	s->WriteSwapScalar(fScaleMax);

	hsScalar massRange = 1.f / fPartInvMassMin - DEFAULT_INVERSE_MASS;
	s->WriteSwapScalar(massRange);
	s->WriteSwapScalar(fPartRadsPerSecRange);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

plOneTimeParticleGenerator::plOneTimeParticleGenerator()
{
}

plOneTimeParticleGenerator::~plOneTimeParticleGenerator()
{
	delete [] fPosition;
	delete [] fDirection;
}

void plOneTimeParticleGenerator::Init(hsScalar count, hsPoint3 *pointArray, hsVector3 *dirArray, 
									  hsScalar xSize, hsScalar ySize, hsScalar scaleMin, hsScalar scaleMax, hsScalar radsPerSecRange)
{
	fCount = count;
	fPosition = pointArray;
	fDirection = dirArray;
	fXSize = xSize;
	fYSize = ySize;
	fScaleMin = scaleMin;
	fScaleMax = scaleMax;
	fPartRadsPerSecRange = radsPerSecRange;
}

// The numForced param is required by the parent class, but ignored by this particular generator
hsBool plOneTimeParticleGenerator::AddAutoParticles(plParticleEmitter *emitter, float dt, UInt32 numForced /* = 0 */)
{
	hsScalar currSizeVar;
	hsScalar scale = (fScaleMax + fScaleMin) / 2;
	hsScalar scaleRange = scale - fScaleMin;

	hsScalar tile;
	hsPoint3 currStart;
	hsPoint3 orientation;
	hsVector3 initDirection;
	hsVector3 zeroVel(0.f, 0.f, 0.f);
	hsScalar radsPerSec = 0;

	int i;
	for (i = 0; i < fCount; i++)
	{
		currStart = emitter->GetLocalToWorld() * fPosition[i];
		initDirection = emitter->GetLocalToWorld() * fDirection[i];

		if (emitter->fMiscFlags & emitter->kOrientationUp)
			orientation.Set(0.0f, -1.0f, 0.0f);
		else
			orientation.Set(&initDirection);

		tile = (hsScalar)(sRandom.Rand() % emitter->GetNumTiles());
		currSizeVar = scale + scaleRange * sRandom.RandMinusOneToOne();

		if( fPartRadsPerSecRange > 0 )
			radsPerSec = fPartRadsPerSecRange * sRandom.RandMinusOneToOne();

		emitter->AddParticle(currStart, zeroVel, (UInt32)tile, fXSize, fYSize, currSizeVar, 
							 DEFAULT_INVERSE_MASS, -1, orientation, 0, radsPerSec);
	}
	emitter->fMiscFlags &= ~plParticleEmitter::kNeedsUpdate;
	return false; // We've done our one-time job. Let the emitter know to delete us.
}

void plOneTimeParticleGenerator::Read(hsStream* s, hsResMgr *mgr)
{
	UInt32 count = s->ReadSwap32();
	hsScalar xSize = s->ReadSwapScalar();
	hsScalar ySize = s->ReadSwapScalar();
	hsScalar scaleMin = s->ReadSwapScalar();
	hsScalar scaleMax = s->ReadSwapScalar();
	hsScalar radsPerSecRange = s->ReadSwapScalar();

	hsPoint3 *pos = TRACKED_NEW hsPoint3[count];
	hsVector3 *dir = TRACKED_NEW hsVector3[count];

	int i;
	for (i = 0; i < count; i++)
	{
		pos[i].Read(s);
		dir[i].Read(s);
	}

	Init((hsScalar)count, pos, dir, xSize, ySize, scaleMin, scaleMax, radsPerSecRange);
}

void plOneTimeParticleGenerator::Write(hsStream* s, hsResMgr *mgr)
{
	s->WriteSwap32((UInt32)fCount);
	s->WriteSwapScalar(fXSize);
	s->WriteSwapScalar(fYSize);
	s->WriteSwapScalar(fScaleMin);
	s->WriteSwapScalar(fScaleMax);
	s->WriteSwapScalar(fPartRadsPerSecRange);

	int i;
	for (i = 0; i < fCount; i++)
	{
		fPosition[i].Write(s);
		fDirection[i].Write(s);
	}
}