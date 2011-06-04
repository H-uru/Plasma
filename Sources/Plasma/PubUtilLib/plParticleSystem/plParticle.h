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
#ifndef plParticle_inc
#define plParticle_inc

#include "hsGeometry3.h"
#include "../CoreLib/hsColorRGBA.h"

// The meat of the particle. These classes, in combination with the plParticleEmitter that spawned it,
// should contain everything specific to a particle, necessary to build a renderable poly to represent a 
// particular particle. (The emitter is necessary for properties (like texture) that are common among all
// particles that originated from the same emitter.

// For any reference in this object to a particle's poly vertices, the structure is as follows:
/*

		|---| "HSize"

	V3-----V2  -
	|     / |  | "VSize"
	|    /  |  |
	|   P   |  -
	|  /    |
	| /     |  ("P" is the current position of the particle)
	V0-----V1

	So the vertices are arranged counter-clockwise, starting in the lower-left corner. Order all other attributes
	accordingly.

*/

// The class plParticleCore should ONLY contain data necessary for the Drawable to create renderable polys
// Everything else goes into plParticleExt.

// plParticleEmitter is depending on the order that member variables appear in these classes, so
// DON'T MODIFY THEM WITHOUT MAKING SURE THE CONSTRUCTOR TO plParticleEmitter PROPERLY COMPUTES
// BASE ADDRESSES AND STRIDES!

// No initialization on construct. In nearly all cases, a default value won't be appropriate
// so there's no sense doing extra memory writes

class plParticleCore
{
public:
	hsPoint3 fPos;
	UInt32 fColor; // Particle opacity goes into the color's alpha.
	hsPoint3 fOrientation; // fMiscFlags determines how this should be used.
	hsVector3 fNormal;
	hsScalar fHSize, fVSize; // distance from the heart of the particle to the borders of its poly.
	hsPoint3 fUVCoords[4];
};

class plParticleExt
{
public:
	//hsPoint3 fOldPos;
	hsVector3 fVelocity;
	hsScalar fInvMass; // The inverse (1 / mass) is what we actually need for calculations. Storing it this
					   // way allows us to make an object immovable with an inverse mass of 0 (and save a divide).
	hsVector3 fAcceleration; // Accumulated from multiple forces.
	hsScalar fLife; // how many seconds before we recycle this? (My particle has more of a life than I do...)
	hsScalar fStartLife;
	hsScalar fScale;
	hsScalar fRadsPerSec;
	//UInt32 fOrigColor;

	enum // Miscellaneous flags for particles
	{
		kImmortal					= 0x00000001,
	};
	UInt32 fMiscFlags;	// I know... 32 bits for a single flag...
						// Feel free to change this if you've got something to pack it against.
};

#endif
