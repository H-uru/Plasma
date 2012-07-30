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

#include "HeadSpin.h"

#include "plDynaDecal.h"
#include "plAuxSpan.h"

bool plDynaSplot::Age(double t, float ramp, float decay, float life)
{
    float age = float(t - fBirth);
    if( age >= life )
        return true;

    int n = fNumVerts;
    if( !n )
        return true;

    float atten = fInitAtten;
    if( age < ramp )
    {
        atten *= age / ramp;
        fFlags |= kFresh;
    }
    else if( age > decay )
    {
        atten *= (life - age) / (life - decay);
        fFlags |= kFresh;
    }
    else if( fFlags & kFresh )
    {
        fFlags &= ~kFresh;
    }
    else 
    {
        return false;
    }

    hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

    if( fFlags & kAttenColor )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            uint32_t diff = uint32_t(origUVW->fZ * atten * 255.99f);
            vtx->fDiffuse = 0xff000000 
                | (diff << 16)
                | (diff << 8)
                | diff;

            vtx++;
            origUVW++;
        }
    }
    else
    if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
    {
        const int stride = sizeof(plDecalVtxFormat);

        float* sPtr = &origUVW->fZ;

        unsigned char* alpha = (unsigned char*)&fVtxBase->fDiffuse;
        alpha += 3;
        while( n-- )
        {
            float initOpac = *sPtr;
            *alpha = (unsigned char)(initOpac * atten * 255.99f);

            alpha += stride;
            sPtr += 3;
        }
    }
    else
    {
        float* sPtr = &origUVW->fZ;

        char* oPtr = (char*)&fVtxBase->fUVW[1].fX;

        const int stride = sizeof(plDecalVtxFormat);

        while( n-- )
        {
            (*(float*)oPtr) = *sPtr * atten;

            oPtr += stride;
            sPtr += 3;
        }
    }
    return false;
}

bool plDynaRipple::Age(double t, float ramp, float decay, float life)
{
    float age = float(t - fBirth);
    if( age >= life )
        return true;

    int n = fNumVerts;
    if( !n )
        return true;

    float atten = fInitAtten;
    if( age < ramp )
    {
        atten *= age / ramp;
    }
    else if( age > decay )
    {
        atten *= (life - age) / (life - decay);
    }

    float scaleU = fC1U / (age*fC2U + 1.f);
    float scaleV = fC1V / (age*fC2V + 1.f);

    hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

    if( fFlags & kAttenColor )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            uint32_t diff = uint32_t(origUVW->fZ * atten * 255.99f);
            vtx->fDiffuse = 0xff000000 
                | (diff << 16)
                | (diff << 8)
                | diff;

            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

            vtx++;
            origUVW++;
        }
    }
    else
    if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
            *alpha = (unsigned char)(origUVW->fZ * atten * 255.99f);

            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

            vtx++;
            origUVW++;
        }
    }
    else
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = (origUVW->fY - 0.5f) * scaleV + 0.5f;

            vtx->fUVW[1].fX = origUVW->fZ * atten;

            vtx++;
            origUVW++;
        }
    }
    fFlags &= ~kFresh;
    return false;
}

bool plDynaWake::Age(double t, float ramp, float decay, float life)
{
    float age = float(t - fBirth);
    if( age >= life )
        return true;

    int n = fNumVerts;
    if( !n )
        return true;

    float atten = fInitAtten;
    if( age < ramp )
    {
        atten *= age / ramp;
    }
    else if( age > decay )
    {
        atten *= (life - age) / (life - decay);
    }

    float scaleU = fC1U / (age*fC2U + 1.f);
    float scaleV = fC1V / (age*fC2V + 1.f);

    hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

    if( fFlags & kAttenColor )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            uint32_t diff = uint32_t(origUVW->fZ * atten * 255.99f);
            vtx->fDiffuse = 0xff000000 
                | (diff << 16)
                | (diff << 8)
                | diff;

            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = origUVW->fY * scaleV;

            vtx++;
            origUVW++;
        }
    }
    else
    if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
            *alpha = (unsigned char)(origUVW->fZ * atten * 255.99f);

            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = origUVW->fY * scaleV;

            vtx++;
            origUVW++;
        }
    }
    else
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            vtx->fUVW[0].fX = (origUVW->fX - 0.5f) * scaleU + 0.5f;
            vtx->fUVW[0].fY = origUVW->fY * scaleV;

            vtx->fUVW[1].fX = origUVW->fZ * atten;

            vtx++;
            origUVW++;
        }
    }
    fFlags &= ~kFresh;
    return false;
}

bool plDynaWave::Age(double t, float ramp, float decay, float life)
{
    float age = float(t - fBirth);
    if( age >= life )
        return true;

    int n = fNumVerts;
    if( !n )
        return true;

    float atten = fInitAtten;
    if( age < ramp )
    {
        atten *= age / ramp;
    }
    else if( age > decay )
    {
        atten *= (life - age) / (life - decay);
    }

    float scale = 1.f + life * fScrollRate;
    float scroll = -fScrollRate * age;

    hsPoint3* origUVW = &fAuxSpan->fOrigUVW[fStartVtx];

    if( fFlags & kAttenColor )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            uint32_t diff = uint32_t(origUVW->fZ * atten * 255.99f);
            vtx->fDiffuse = 0xff000000 
                | (diff << 16)
                | (diff << 8)
                | diff;

            vtx->fUVW[0].fX = origUVW->fX;
            vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

            vtx++;
            origUVW++;
        }
    }
    else
    if( fAuxSpan->fFlags & plAuxSpan::kRTLit )
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            unsigned char* alpha = ((unsigned char*)&vtx->fDiffuse) + 3;
            *alpha = (unsigned char)(origUVW->fZ * atten * 255.99f);

            vtx->fUVW[0].fX = origUVW->fX;
            vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

            vtx++;
            origUVW++;
        }
    }
    else
    {
        plDecalVtxFormat* vtx = fVtxBase;

        while( n-- )
        {
            vtx->fUVW[0].fX = origUVW->fX;
            vtx->fUVW[0].fY = origUVW->fY * scale + scroll;

            vtx->fUVW[1].fX = origUVW->fZ * atten;

            vtx++;
            origUVW++;
        }
    }
    fFlags &= ~kFresh;
    return false;
}

bool plDynaRippleVS::Age(double t, float ramp, float decay, float life)
{
    float age = float(t - fBirth);
    if( age >= life )
        return true;

    int n = fNumVerts;
    if( !n )
        return true;

    fFlags &= ~kFresh;
    return false;
}

