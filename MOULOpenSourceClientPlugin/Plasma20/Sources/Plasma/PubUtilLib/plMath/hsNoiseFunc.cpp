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
#include "hsNoiseFunc.h"
#include "hsTypes.h"
#include "hsScalar.h"
#include "hsGeometry3.h"

hsNoiseFunc::hsNoiseFunc() 
{
}

hsNoiseFunc::~hsNoiseFunc() 
{
}

void hsNoiseFunc::Seed(UInt32 s)
{
	srand(s);
}

hsTableNoise::hsTableNoise()
: fTable(nil), fTableLen(0)
{
}

hsTableNoise::~hsTableNoise()
{
	delete [] fTable;
}

void hsTableNoise::SetTable(int len, hsScalar* arr)
{
	fTableLen = len;

	delete [] fTable;
	if( !len )
	{
		fTable = nil;
		return;
	}

	fTable = TRACKED_NEW hsScalar[len+2];

	int i;
	for( i = 0; i < len; i++ )
		fTable[i] = arr[i];
	fTable[i++] = fTable[i-1];
	fTable[i++] = fTable[i-1];

}

hsScalar hsTableNoise::Noise(hsScalar lo, hsScalar hi, hsScalar t)
{
	hsAssert(fTableLen, "Badly initialized table noise function");

	hsScalar r = hsScalar(rand()) / hsScalar(RAND_MAX);
	r = lo + (hi - lo) * r;

	if( t < 0 )
		t = 0;
	else if( t > hsScalar1 )
		t = hsScalar1;

	hsScalar tIdx = t * fTableLen;
	UInt32 idx = UInt32(tIdx);
	hsScalar frac = tIdx - hsScalar(idx);
	hsAssert((idx >= 0)&&(idx <= fTableLen), "Noise parm t out of range [0..1]");

	hsScalar scale = fTable[idx] + (fTable[idx+1] - fTable[idx]) * frac;
	
	r *= scale;

	return r;
}

hsScalar hsTableNoise::NoisePoint(const hsPoint3& p, hsScalar lo, hsScalar hi, hsScalar t)
{
	hsAssert(fTableLen, "Badly initialized table noise function");

	UInt32 sX = *((UInt32*)&p.fX);
	UInt32 sY = *((UInt32*)&p.fY);
	UInt32 sZ = *((UInt32*)&p.fZ);

	UInt32 sAll = ((((sX & 0x07800000) >> 16) | ((sX & 0x007fffff) >> 17)) << 20)
				| ((((sY & 0x07800000) >> 16) | ((sY & 0x007fffff) >> 17)) << 10)
				| ((((sZ & 0x07800000) >> 16) | ((sZ & 0x007fffff) >> 17))      );

	const UInt32 kExp = 0x3f800000;
	const UInt32 kMsk = 0x007fffff;

	const UInt32 kA = 1665636L;
	const UInt32 kC = 1013904223L;

	UInt32 iR = kA * sAll + kC;
	iR &= kMsk;
	iR |= kExp;

	hsScalar r = (*(float*)&iR) - 1.f;

	r = lo + (hi - lo) * r;

	if( t < 0 )
		t = 0;
	else if( t > hsScalar1 )
		t = hsScalar1;

	hsScalar tIdx = t * fTableLen;
	UInt32 idx = UInt32(tIdx);
	hsScalar frac = tIdx - hsScalar(idx);
	hsAssert((idx >= 0)&&(idx <= fTableLen), "Noise parm t out of range [0..1]");

	hsScalar scale = fTable[idx] + (fTable[idx+1] - fTable[idx]) * frac;
	
	r *= scale;

	return r;
}
