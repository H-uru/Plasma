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
#ifndef plParticleGenerator_inc
#define plParticleGenerator_inc

#include "hsGeometry3.h"
#include "../pnNetCommon/plSynchedValue.h"
class plParticleEmitter;
class plScalarController;

// This class is responsible for all the details of automatically generating particles for a plParticleEmitter.
// It gets a change in time, and must do whatever necessary to generate the appropriate number of particles
// for that timespan

class plParticleGenerator : public plCreatable
{

public:
	// returns false if it's done generating particles and is safe to delete.
	virtual hsBool AddAutoParticles(plParticleEmitter *emitter, float dt, UInt32 numForced = 0) = 0;

	virtual void UpdateParam(UInt32 paramID, hsScalar paramValue) = 0;

	CLASSNAME_REGISTER( plParticleGenerator );
	GETINTERFACE_ANY( plParticleGenerator, plCreatable );

	static const void ComputeDirection(float pitch, float yaw, hsVector3 &direction);
	static const void ComputePitchYaw(float &pitch, float &yaw, const hsVector3 &dir);
	static inline float GetRandomVar() { return 2.0f * (float)hsRand() / RAND_MAX - 1; } // returns a num between +/- 1.0
};

class plSimpleParticleGenerator : public plParticleGenerator
{
public:
	plSimpleParticleGenerator();
	~plSimpleParticleGenerator();
	void Init(hsScalar genLife, hsScalar partLifeMin, hsScalar partLifeMax, hsScalar particlesPerSecond, 
			  UInt32 numSources, hsPoint3 *pos, hsScalar *initPitch, hsScalar *initYaw, hsScalar angleRange,
			  hsScalar initVelMin, hsScalar initVelMax, hsScalar xSize, hsScalar ySize, 
			  hsScalar scaleMin, hsScalar scaleMax,
			  hsScalar massRange, hsScalar radsPerSecRange);

	CLASSNAME_REGISTER( plSimpleParticleGenerator );
	GETINTERFACE_ANY( plSimpleParticleGenerator, plParticleGenerator);
	
	virtual hsBool AddAutoParticles(plParticleEmitter *emitter, float dt, UInt32 numForced);
	virtual void UpdateParam(UInt32 paramID, hsScalar paramValue);

	virtual void Read(hsStream* s, hsResMgr *mgr); 
	virtual void Write(hsStream* s, hsResMgr *mgr);

protected:
	hsScalar fParticlesPerSecond;
	UInt32 fNumSources;
	hsPoint3 *fInitPos;
	hsScalar *fInitPitch, *fInitYaw;
	hsScalar fAngleRange;
	hsScalar fVelMin, fVelMax;
	hsScalar fXSize, fYSize, fScaleMin, fScaleMax;
	hsScalar fGenLife;  // How long shall we spit out particles from this location? When this time runs out, we stop
						// spitting particles, but we don't actually die until all of our particles die naturally.
						// (Even the ones that we feel are suffering needlessly.)

	hsScalar fPartLifeMin; // lifespan for the particles we generate
	hsScalar fPartLifeMax;

	hsScalar fPartInvMassMin;	// Doing a uniform variant over the inverse mass range (instead of over the mass range
	hsScalar fPartInvMassRange;	// and then inverting) will favor the low end of the mass range, but then again,
								// it's just a freaking game. Note though that fPartInvMassMin == 1.f / massMAX.

	hsScalar fPartRadsPerSecRange; // Zero means no rot, otherwise uniform random between [-range..range]

	hsScalar fParticleSum;

	enum
	{
		kImmortal	= 0x1,
		kDisabled	= 0x2,
	};
	UInt32 fMiscFlags;
};

class plOneTimeParticleGenerator : public plParticleGenerator
{
public:

	plOneTimeParticleGenerator();
	~plOneTimeParticleGenerator();
	void Init(hsScalar count, hsPoint3 *pointArray, hsVector3 *dirArray, 
			  hsScalar xSize, hsScalar ySize, hsScalar scaleMin, hsScalar scaleMax, hsScalar radsPerSec);

	CLASSNAME_REGISTER( plOneTimeParticleGenerator );
	GETINTERFACE_ANY( plOneTimeParticleGenerator, plParticleGenerator);

	virtual hsBool AddAutoParticles(plParticleEmitter *emitter, float dt, UInt32 numForced = 0);
	virtual void UpdateParam(UInt32 paramID, hsScalar paramValue) {}

	virtual void Read(hsStream* s, hsResMgr *mgr); 
	virtual void Write(hsStream* s, hsResMgr *mgr);

protected:
	hsScalar fCount;
	hsPoint3 *fPosition;
	hsVector3 *fDirection;
	hsScalar fXSize, fYSize, fScaleMin, fScaleMax;
	hsScalar fPartRadsPerSecRange; // Zero means no rot, otherwise uniform random between [-range..range]
};

#endif
