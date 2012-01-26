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

#ifndef plRandom_inc
#define plRandom_inc

// FAST_Q is basically lifted from Numerical Recipes.
// The RandZeroToOne and RandMinusOneToOne were "enhanced"
// a bit, but tested out nicely to generate sequences with
// appropriate maxs, mins, and means.
// FAST_Q depends on IEEE floating point format. Undefine
// it for any platform which doesn't satisfy this, or
// come up with the appropriate constants for that platform.
#define FAST_Q


class plRandom
{
protected:
    mutable uint32_t      fSeed;
public:
    inline float        RandNorm() const;
    inline int          Rand() const;
    inline int          RandRangeI(int lo, int hi) const;
    inline float        RandRangeF(float lo, float hi) const;
    inline float        RandMinusOneToOne() const;
    inline float        RandZeroToOne() const;

    uint32_t              GetSeed() const { return fSeed; }
    void                SetSeed(int seed) { fSeed = seed; }

    plRandom(int seed = 1) : fSeed(seed) {}
};

inline float plRandom::RandNorm() const
{ 
#ifndef FAST_Q
    return 1.f / 32767.f; 
#else // FAST_Q
    return (1.f / float(~0UL));
#endif // FAST_Q
} 

inline int plRandom::Rand() const
{
#ifndef FAST_Q
    register int temp;
    fSeed = fSeed * 1103515245 + 12345;
    temp = (int)((fSeed/65536)&32767);
    return (temp);
#else // FAST_Q
    return fSeed = 1664525L * fSeed + 1013904223L;
#endif // FAST_Q
}

// RandZeroToOne - take our usual random uint32_t.
// We're going to mask in an exponent to make it
// a float in range [1..2). Then subtract 1.f to
// make it [0..1). We shift our random uint32_t down
// by 9 because the upper bits are the most random.
inline float plRandom::RandZeroToOne() const
{
#ifndef FAST_Q
    return Rand() * RandNorm();
#else // FAST_Q
    const uint32_t kOneExp = 0x3f800000;
    register uint32_t temp = kOneExp | (uint32_t(Rand()) >> 9);
    return (*(float*)&temp) - 1.f;
#endif // FAST_Q
}

// RandMinusOneToOne - same as RandZeroToOne, but we
// mask in an exponent putting it in range [2..4), 
// then subtract 3 to give [-1..1).
inline float plRandom::RandMinusOneToOne() const
{
#ifndef FAST_Q
    return RandZeroToOne() * 2.f - 1.f;
#else // FAST_Q
    const uint32_t kTwoExp = 0x40000000;
    register uint32_t temp = kTwoExp | (uint32_t(Rand()) >> 9);
    return (*(float*)&temp) - 3.f;
#endif // FAST_Q
}

inline float plRandom::RandRangeF(float lo, float hi) const
{
    return lo + RandZeroToOne() * (hi - lo);
}

inline int plRandom::RandRangeI(int lo, int hi) const
{
    return lo + int(RandZeroToOne() * float(hi + 1 - lo));
}


#endif // plRandom_inc

