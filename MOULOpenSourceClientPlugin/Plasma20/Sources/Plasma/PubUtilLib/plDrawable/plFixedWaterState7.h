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

#ifndef plFixedWaterState7_inc
#define plFixedWaterState7_inc

#include "hsGeometry3.h"
#include "hsColorRGBA.h"
#include "../pnTimer/plTimedValue.h"

class hsStream;

class plFixedWaterState7
{
public:
	class WaveState
	{
	public:
		plTimedSimple<hsScalar>		fMaxLength;
		plTimedSimple<hsScalar>		fMinLength;
		plTimedSimple<hsScalar>		fAmpOverLen;
		plTimedSimple<hsScalar>		fChop;
		plTimedSimple<hsScalar>		fAngleDev;

		void Set(const WaveState& w, hsScalar secs);

		void Read(hsStream* s);
		void Write(hsStream* s) const;
	};
	// Main body of water state

	// Geometric waves
	WaveState					fGeoState;

	// Texture waves
	WaveState					fTexState;
	plTimedSimple<hsScalar>		fRippleScale;

	// Geometric and Texture share wind direction
	plTimedCompound<hsVector3>		fWindDir;

	// Level of noise added during summation of texture waves
	enum {
		kNoise	= 0,
		kSpecStart = 1,
		kSpecEnd = 2
	};
	plTimedCompound<hsVector3>		fSpecVec; // (Noise, SpecStart, SpecEnd);

	// Depth parameters. Affect how the depth of
	// the water vertex is interpreted into water
	// surface properties.
	plTimedSimple<hsScalar>			fWaterHeight;
	plTimedCompound<hsVector3>		fWaterOffset;
	plTimedCompound<hsVector3>		fMaxAtten;
	plTimedCompound<hsVector3>		fMinAtten;
	plTimedCompound<hsVector3>		fDepthFalloff;

	// Shore parameters

	// Appearance
	plTimedSimple<hsScalar>		fWispiness;
	plTimedCompound<hsColorRGBA>		fShoreTint;
	// Next two only used in generation of bubble layer
	plTimedCompound<hsColorRGBA>		fMaxColor;
	plTimedCompound<hsColorRGBA>		fMinColor;
	plTimedSimple<hsScalar>		fEdgeOpac;
	plTimedSimple<hsScalar>		fEdgeRadius;

	// Simulation
	plTimedSimple<hsScalar>		fPeriod;
	plTimedSimple<hsScalar>		fFingerLength;

	// The rest aren't really part of the state, that is they are normally controlled
	// by something exterior to the state. They are included here to allow a convenient
	// override during development.

	// Water appearance.
	plTimedCompound<hsColorRGBA>		fWaterTint;
	plTimedCompound<hsColorRGBA>		fSpecularTint;

	plTimedCompound<hsPoint3>		fEnvCenter;
	plTimedSimple<hsScalar>		fEnvRefresh;
	plTimedSimple<hsScalar>		fEnvRadius;

	void Set(const plFixedWaterState7& s, hsScalar secs);

	void Read(hsStream* s);
	void Write(hsStream* s) const;
};


#endif // plFixedWaterState7_inc
