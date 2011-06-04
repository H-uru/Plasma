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

#ifndef plDynaDecal_inc
#define plDynaDecal_inc

#include "hsGeometry3.h"

class plAuxSpan;

class plDynaDecalBin;

class plDecalVtxFormat
{
public:
	hsPoint3	fPos;
	hsVector3	fNorm;
	UInt32		fDiffuse;
	UInt32		fSpecular; // Not used anywhere, carried around everywhere.
	hsPoint3	fUVW[2];
};

// UVW[0] is the currently used UVW.
// UVW[1] is for the alpha hack (when necessary).
const UInt8 kDecalVtxFormat = 0x2; // Two UVW's, otherwise vanilla.


class plDynaDecal
{
public:
	enum
	{
		kFresh			= 0x1,
		kAttenColor		= 0x2,
		kVertexShader	= 0x4
	};
protected:

	// StartVtx and StartIdx are relative to the start of the data
	// owned by this decal's span, not relative to the start of the
	// underlying buffers.
	UInt16		fStartVtx;
	UInt16		fNumVerts;

	UInt16		fStartIdx;
	UInt16		fNumIdx;

	double		fBirth;
	hsScalar	fInitAtten;
	hsBool		fFlags;

	plDecalVtxFormat*	fVtxBase; // Safe pointer, the buffer data will outlive this decal
	
	plAuxSpan*			fAuxSpan;

	friend class plDynaDecalMgr;
public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life) = 0;
};

// No expansion
class plDynaSplot : public plDynaDecal
{
protected:

public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life);
};

// Expands radially from center
class plDynaRipple : public plDynaDecal
{
public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life);

	hsScalar fC1U;
	hsScalar fC2U;

	hsScalar fC1V;
	hsScalar fC2V;

};

// Expands in V from top (V=0), expands in U from center (U=0.5)
class plDynaWake : public plDynaDecal
{
public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life);

	hsScalar fC1U;
	hsScalar fC2U;

	hsScalar fC1V;
	hsScalar fC2V;

};

// Scrolls in V, no change in U
class plDynaWave : public plDynaDecal
{
public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life);

	hsScalar fScrollRate;
};

// About the same as a DynaRipple, but implemented with vertex/pixel shaders.
// Only useful with plWaveSet.
class plDynaRippleVS : public plDynaRipple
{
public:

	virtual hsBool		Age(double t, hsScalar ramp, hsScalar decay, hsScalar life);
};

#endif // plDynaDecal_inc

