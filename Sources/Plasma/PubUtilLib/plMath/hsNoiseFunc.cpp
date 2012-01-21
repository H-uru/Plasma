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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "hsNoiseFunc.h"
#include "HeadSpin.h"

#include "hsGeometry3.h"

hsNoiseFunc::hsNoiseFunc() 
{
}

hsNoiseFunc::~hsNoiseFunc() 
{
}

void hsNoiseFunc::Seed(uint32_t s)
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

void hsTableNoise::SetTable(int len, float* arr)
{
    fTableLen = len;

    delete [] fTable;
    if( !len )
    {
        fTable = nil;
        return;
    }

    fTable = new float[len+2];

    int i;
    for( i = 0; i < len; i++ )
        fTable[i] = arr[i];
    fTable[i++] = fTable[i-1];
    fTable[i++] = fTable[i-1];

}

float hsTableNoise::Noise(float lo, float hi, float t)
{
    hsAssert(fTableLen, "Badly initialized table noise function");

    float r = float(rand()) / float(RAND_MAX);
    r = lo + (hi - lo) * r;

    if( t < 0 )
        t = 0;
    else if( t > 1.f )
        t = 1.f;

    float tIdx = t * fTableLen;
    uint32_t idx = uint32_t(tIdx);
    float frac = tIdx - float(idx);
    hsAssert((idx >= 0)&&(idx <= fTableLen), "Noise parm t out of range [0..1]");

    float scale = fTable[idx] + (fTable[idx+1] - fTable[idx]) * frac;
    
    r *= scale;

    return r;
}

float hsTableNoise::NoisePoint(const hsPoint3& p, float lo, float hi, float t)
{
    hsAssert(fTableLen, "Badly initialized table noise function");

    uint32_t sX = *((uint32_t*)&p.fX);
    uint32_t sY = *((uint32_t*)&p.fY);
    uint32_t sZ = *((uint32_t*)&p.fZ);

    uint32_t sAll = ((((sX & 0x07800000) >> 16) | ((sX & 0x007fffff) >> 17)) << 20)
                | ((((sY & 0x07800000) >> 16) | ((sY & 0x007fffff) >> 17)) << 10)
                | ((((sZ & 0x07800000) >> 16) | ((sZ & 0x007fffff) >> 17))      );

    const uint32_t kExp = 0x3f800000;
    const uint32_t kMsk = 0x007fffff;

    const uint32_t kA = 1665636L;
    const uint32_t kC = 1013904223L;

    uint32_t iR = kA * sAll + kC;
    iR &= kMsk;
    iR |= kExp;

    float r = (*(float*)&iR) - 1.f;

    r = lo + (hi - lo) * r;

    if( t < 0 )
        t = 0;
    else if( t > 1.f )
        t = 1.f;

    float tIdx = t * fTableLen;
    uint32_t idx = uint32_t(tIdx);
    float frac = tIdx - float(idx);
    hsAssert((idx >= 0)&&(idx <= fTableLen), "Noise parm t out of range [0..1]");

    float scale = fTable[idx] + (fTable[idx+1] - fTable[idx]) * frac;
    
    r *= scale;

    return r;
}
