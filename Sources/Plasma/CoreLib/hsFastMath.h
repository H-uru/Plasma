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

#ifndef hsFastMath_inc
#define hsFastMath_inc

#include "hsPoint2.h"
#include "hsGeometry3.h"

class hsFastMath {
protected:
    static const hsPoint2* fCosSinTable;

public:
    static float IATan2OverTwoPi(float y, float x);

    static inline float InvSqrtAppr(float x);
    static inline float InvSqrt(float x);
    static inline hsVector3& Normalize(hsVector3& v) { return (v *= InvSqrt(v.MagnitudeSquared())); }
    static inline hsVector3& NormalizeAppr(hsVector3& v) { return (v *= InvSqrtAppr(v.MagnitudeSquared())); }

    static inline void SinCosAppr(float rads, float& sinRads, float& cosRads);
    static inline void SinCosInRangeAppr(float rads, float& sinRads, float& cosRads);

    static inline void SinCos(float rads, float& sinRads, float& cosRads);
    static inline void SinCosInRange(float ang, float& sinRads, float& cosRads);

    static inline float Sin(float rads);
    static inline float Cos(float rads);
    static inline float SinInRange(float rads);
    static inline float CosInRange(float rads);
};


// One over Square Root - from Graphics Gems
// Interesting combo's are
// NUM_ITER     LOOKUP_BITS         err frac    us per call
// 0            8                   5e-3        0.045
// 1            8                   3e-5        0.082
// 0            6                   1e-2        0.045
// 1            6                   1e-4        0.082
// 2            6                   1e-7        0.11
// 1            4                   2e-3        0.082
// 2            4                   5e-6        0.11
// 2            3                   8e-5        0.11
// Tested on 5000 random numbers from [1.e-6..1.e3] over several runs
// These are tight loops, though, so they don't weigh in a bigger
// table trashing the cache.
#define NUM_ITER        0
#define LOOKUP_BITS     8
#define EXP_POS         23
#define EXP_BIAS        127

#define LOOKUP_POS      (EXP_POS - LOOKUP_BITS)
#define SEED_POS        (EXP_POS - 8)
#define TABLE_SIZE      (2 << LOOKUP_BITS)
#define LOOKUP_MASK     (TABLE_SIZE - 1)
#define GET_EXP(a)      (((a) >> EXP_POS) & 0xff)
#define SET_EXP(a)      ((a) << EXP_POS)
#define GET_EMANT(a)    (((a) >> LOOKUP_POS) & LOOKUP_MASK)

#define SET_MANTSEED(a) (((uint32_t) (a)) << SEED_POS)

inline float hsFastMath::InvSqrtAppr(float x)
{
    uint32_t a = *(int32_t*)&x;
#if NUM_ITER > 0
    float arg = x;
#endif
    union {
        int32_t i;
        float   f;
    } seed;
    float r;

    extern unsigned char statSeedTable[];

    seed.i = SET_EXP(((3*EXP_BIAS - 1) - GET_EXP(a)) >> 1) | SET_MANTSEED(statSeedTable[GET_EMANT(a)]);

    r = seed.f;

#if NUM_ITER > 0
    r = (3.0f - r * r * arg) * r * 0.5f;

#if NUM_ITER > 1
    r = (3.0f - r * r * arg) * r * 0.5f;
#endif
#endif

    return r;
}

inline float hsFastMath::InvSqrt(float x)
{
    uint32_t a = *(int32_t*)&x;
    float arg = x;
    union {
        int32_t i;
        float   f;
    } seed;
    float r;

    extern unsigned char statSeedTable[];

    seed.i = SET_EXP(((3*EXP_BIAS - 1) - GET_EXP(a)) >> 1) | SET_MANTSEED(statSeedTable[GET_EMANT(a)]);

    r = seed.f;

    r = (3.0f - r * r * arg) * r * 0.5f;

    r = (3.0f - r * r * arg) * r * 0.5f;

    return r;
}


inline void hsFastMath::SinCosAppr(float rads, float& sinRads, float& cosRads)
{
    rads = fmodf(rads, hsConstants::two_pi<float>);
    if( rads < 0 )
        rads += hsConstants::two_pi<float>;
    SinCosInRangeAppr(rads, sinRads, cosRads);
}

inline void hsFastMath::SinCosInRangeAppr(float rads, float& sinRads, float& cosRads)
{
    constexpr int kNumSinCosEntries = 8;
    constexpr float kNumEntriesOverTwoPI = kNumSinCosEntries * 0.5f / hsConstants::pi<float>;
    float t = rads * kNumEntriesOverTwoPI;
    int iLo = (int)t;
    t -= iLo;

    const hsPoint2* p = &fCosSinTable[iLo + 1];
    cosRads = p->fX;
    sinRads = p->fY;
    p--;
    cosRads -= p->fX;
    sinRads -= p->fY;
    cosRads *= t;
    sinRads *= t;
    cosRads += p->fX;
    sinRads += p->fY;

}

inline float hsFastMath::Sin(float rads)
{
    rads = fmodf(rads, hsConstants::two_pi<float>);
    if( rads < 0 )
        rads += hsConstants::two_pi<float>;

    return SinInRange(rads);
}

inline float hsFastMath::Cos(float rads)
{
    rads = fmodf(rads, hsConstants::two_pi<float>);
    if( rads < 0 )
        rads += hsConstants::two_pi<float>;

    return CosInRange(rads);
}

inline float hsFastMath::SinInRange(float ang)
{
    float sgn = 1.f;

    if (ang >= (0.75f * hsConstants::two_pi<float>))
        ang -= hsConstants::two_pi<float>;
    else if (ang >= (0.25f * hsConstants::two_pi<float>))
    {
        ang -= hsConstants::pi<float>;
        sgn = -1.0f;
    }
    
    return (ang - (ang*ang*ang) * (1.0f/6.0f) + (ang*ang*ang*ang*ang) / 120.0f) * sgn;
}

inline float hsFastMath::CosInRange(float ang)
{
    float sgn = 1.f;
    
    if (ang >= (0.75f * hsConstants::two_pi<float>))
        ang -= hsConstants::two_pi<float>;
    else if (ang >= (0.25f * hsConstants::two_pi<float>))
    {
        ang -= hsConstants::pi<float>;
        sgn = -1.0f;
    }
    
    return (1.0f - (ang*ang / 2.0f) + (ang*ang*ang*ang) / 24.0f) *sgn;
}

inline void hsFastMath::SinCos(float rads, float& sinRads, float& cosRads)
{
    rads = fmodf(rads, hsConstants::two_pi<float>);
    if( rads < 0 )
        rads += hsConstants::two_pi<float>;
    SinCosInRange(rads, sinRads, cosRads);
}

inline void hsFastMath::SinCosInRange(float ang, float& sinRads, float& cosRads)
{
    float sgn = 1.f;
    
    if (ang >= (0.75f * hsConstants::two_pi<float>))
        ang -= hsConstants::two_pi<float>;
    else if (ang >= (0.25f * hsConstants::two_pi<float>))
    {
        ang -= hsConstants::pi<float>;
        sgn = -1.0f;
    }
    
    sinRads = (ang - (ang*ang*ang) * (1.0f/6.0f) + (ang*ang*ang*ang*ang) / 120.0f) * sgn;
    cosRads = (1.0f - (ang*ang / 2.0f) + (ang*ang*ang*ang) / 24.0f) *sgn;
}
//
// Here's an interesting one from GDalgorithms, which doesn't need a LUT
// Not sure how the accuracy compares, but it's probably fine for this purpose.
#if 0 // For future reference
/*
From: "Jason Dorie" <jason.dorie@blackboxgames.com>
To: "GDAlgorithms" <gdalgorithms-list@lists.sourceforge.net>
Date: Wed, 14 Mar 2001 11:43:48 -0800
Subject: [Algorithms] Fast simultaneous Sin() and Cos()
Reply-To: gdalgorithms-list@lists.sourceforge.net


  I know someone (Jason Zisk?) was looking for fast rotation matrix
generation code.  I realize that a Sin/Cos lookup table is the way to go for
absolute speed, but if storage is a concern and the accuracy isn't, this
code is about 5x faster than using the built-in sin and cos instructions,
and accurate to about 4 decimal places.

  If you really want speed, and don't care about accuracy, drop the 2nd
polynomial from each term.  It's less accurate and faster still.  It could
probably be made even faster by replacing the if/else with branchless code,
but I haven't bothered to figure out how yet.


  My angles are 0-65535 so that they can be masked into range easily, stored
as shorts, and converted to normalized floats where necessary using SIMD
instructions.
*/

void FastSinCos(long Angle, float *pSin, float *pCos)
{
float ang, sgn;

        ang = (Angle & 65535) * ((1.0f/65536.0f) * TwoPI);

        sgn = 1.0f;
        if(ang >= (0.75f * TwoPI))
                ang -= TwoPI;
        else if(ang >= (0.25f * TwoPI))
        {
                ang -= hsConstants::pi<float>;
                sgn = -1.0f;
        }

        *pSin = (ang - (ang*ang*ang) * (1.0f/6.0f) + (ang*ang*ang*ang*ang) / 120.0f) * sgn;
        *pCos = (1.0f - (ang*ang / 2.0f) + (ang*ang*ang*ang) / 24.0f) *sgn;
}
#endif // For future reference
#endif // hsFastMath_inc
