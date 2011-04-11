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
#ifndef plEffectTargetInfo_inc
#define plEffectTargetInfo_inc

#include "hsTypes.h"

struct hsPoint3;
class plPipeline;
class plParticleSystem;

// This is the rendering context passed into an effect to let it cache up
// anything it needs to compute that will be the same for all particles.
// Not a lot of context to go on to begin with, but this will let that
// expand without any interface changes.
class plParticleContext
{
public:
	plPipeline*			fPipeline;
	plParticleSystem*	fSystem;
	double				fSecs;
	hsScalar			fDelSecs;
};


// This is just a collection of arrays and strides that a plParticleEffect object will reference and modify
// in the course of doing its job.

class plEffectTargetInfo
{
public:
	// Byte arrays. Declared as type UInt8 so that adding the stride to the pointer is guaranteed to advance
	// the exact number of bytes.
	UInt8 *fPos;
	UInt8 *fVelocity;
	UInt8 *fInvMass;
	UInt8 *fAcceleration;
	UInt8 *fColor;
	UInt8 *fRadsPerSec;
	UInt8 *fMiscFlags;
	
	UInt32 fPosStride;
	UInt32 fVelocityStride;
	UInt32 fInvMassStride;
	UInt32 fAccelerationStride;
	UInt32 fColorStride;
	UInt32 fRadsPerSecStride;
	UInt32 fMiscFlagsStride;

	plParticleContext	fContext;
	UInt32				fNumValidParticles;
	UInt32				fFirstNewParticle;
	
	// We're going to need some sort of connectivity data for constraint satisfaction, but at least we have
	// a system that allows that to be added in smoothly when it's needed, so for now, let's get the main
	// goop working.
};

#endif
