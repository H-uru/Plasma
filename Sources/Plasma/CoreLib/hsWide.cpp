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
#include "hsWide.h"

/////////////////////////////////////////////////////////////////////////

inline hsBool OverflowAdd(uint32_t* sum, uint32_t a, uint32_t b)
{
    *sum = a + b;

    return (a | b) > *sum;  // true if overflow
}

/*
    Return the overflow from adding the three longs into a signed-wide
    
    wide = (high << 32) + (middle << 16) + low
*/
inline hsBool SetWide3(hsWide* target, int32_t high, uint32_t middle, uint32_t low)
{
    hsAssert(high >= 0, "high is neg");

    target->fLo = low + (middle << 16);
    target->fHi = high + (middle >> 16) + (((low >> 16) + (uint16_t)middle) >> 16);

    return target->fHi < 0; // true if overflow
}

/////////////////////////////////////////////////////////////////////////

hsWide* hsWide::Mul(int32_t src1, int32_t src2)
{
    int neg = 0;
    
    if (src1 < 0)
    {   src1 = -src1;
        neg = ~0;
    }
    if (src2 < 0)
    {   src2 = -src2;
        neg = ~neg;
    }
    
    uint32_t  a = src1 >> 16;
    uint32_t  b = (uint16_t)src1;
    uint32_t  c = src2 >> 16;
    uint32_t  d = (uint16_t)src2;
    
    (void)SetWide3(this, a * c, a * d + c * b, b * d);

    if (neg)
        this->Negate();
    return this;
}

hsWide* hsWide::Mul(int32_t A)
{
    int     neg = 0;
    uint32_t  B = fLo;
    int32_t   C = fHi;
    int32_t   tmp;
    uint32_t  clo,blo,bhi,alo;

    if (A < 0)
    {   A = -A;
        neg = ~0;
    }
    if (WIDE_ISNEG(C, B))
    {   WIDE_NEGATE(C, B);
        neg = ~neg;
    }

    uint32_t  ahi = A >> 16;
    uint32_t  chi = C >> 16;
    if (ahi != 0 && chi != 0)
        goto OVER_FLOW;

    alo = (uint16_t)A;
    bhi = B >> 16;
    blo = (uint16_t)B;
    clo = (uint16_t)C;

    tmp = alo * clo;
    if (tmp < 0 || SetWide3(this, tmp, alo * bhi, alo * blo))
        goto OVER_FLOW;

    if (chi != 0)
    {   uint32_t  Vh = alo * chi;
        if (Vh >> 15)
            goto OVER_FLOW;
        if (((this->fHi >> 16) + (uint16_t)Vh) >> 15)
            goto OVER_FLOW;
        this->fHi += Vh << 16;
    }
    else                            // ahi != 0 && chi == 0
    {   hsWide  w;
        uint32_t  Vh = ahi * clo;
        if (Vh >> 16)
            goto OVER_FLOW;
        tmp = ahi * bhi;
        if (tmp < 0 || SetWide3(&w, tmp, ahi * blo, 0))
            goto OVER_FLOW;
        if (((w.fHi >> 16) + (uint16_t)Vh) >> 15)
            goto OVER_FLOW;
        w.fHi += Vh << 16;
        this->Add(&w);
    }
    
    if (neg)
        this->Negate();
    return this;

OVER_FLOW:
    *this = neg ? kNegInfinity64 : kPosInfinity64;
    return this;
}

hsWide* hsWide::Div(int32_t denom)
{
    if (denom == 0)
    {   if (this->IsNeg())
        {   hsSignalMathUnderflow();
            *this = kNegInfinity64;
        }
        else
        {   hsSignalMathOverflow();
            *this = kPosInfinity64;
        }
        return this;
    }

    int     neg = 0;
    int32_t   resultH = 0;
    uint32_t  resultL = 0;
    int32_t   numerH = this->fHi;
    uint32_t  numerL = this->fLo;

    if (denom < 0)
    {   denom = -denom;
        neg = ~0;
    }
    if (WIDE_ISNEG(numerH, numerL))
    {   WIDE_NEGATE(numerH, numerL);
        neg = ~neg;
    }
    
    WIDE_ADDPOS(numerH, numerL, denom >> 1);    // add denom/2 to get a round result

    uint32_t  curr = (uint32_t)numerH >> 31;

    for (int i = 0; i < 64; i++)
    {
        WIDE_SHIFTLEFT(resultH, resultL, resultH, resultL, 1);
        if (uint32_t(denom) <= curr)
        {
            resultL |= 1;
            curr -= denom;
        }
        WIDE_SHIFTLEFT(numerH, numerL, numerH, numerL, 1);
        curr = (curr << 1) | ((uint32_t)numerH >> 31);
    }

    if (neg)
        WIDE_NEGATE(resultH, resultL);
    return this->Set(resultH, resultL);
}

hsWide* hsWide::Div(const hsWide* denom)
{
    hsWide  d = *denom;
    int     shift = 0;
    
    while (d.IsWide())
    {   (void)d.ShiftRight(1);
        shift += 1;
    }
    if (shift)
    {   d = *denom;
        (void)this->RoundRight(shift);
        (void)d.RoundRight(shift);
    }
    return this->Div(d.AsLong());
}

inline int MaxLeftShift(const hsWide* w)
{
    int32_t hi = w->fHi;

    if (hi == 0)
        return 31;
    else
    {   int shift = -1;

        if (hi < 0) hi = -hi;

        do {
            hi <<= 1;
            shift += 1;
        } while (hi > 0);
        return shift;
    }
}

int32_t hsWide::FixDiv(const hsWide* denom) const
{
    hsWide  num = *this;
    hsWide  den = *denom;
    int     maxShift = MaxLeftShift(this);

    if (maxShift >= 16) // easy case
        (void)num.ShiftLeft(16);
    else
    {   (void)num.ShiftLeft(maxShift);
        (void)den.RoundRight(16 - maxShift);
    }
    
    return num.Div(&den)->AsLong();
}

int32_t hsWide::FracDiv(const hsWide* denom) const
{
    hsWide  num = *this;
    hsWide  den = *denom;
    int     maxShift = MaxLeftShift(this);

    if (maxShift >= 30) // easy case
        (void)num.ShiftLeft(30);
    else
    {   (void)num.ShiftLeft(maxShift);
        (void)den.RoundRight(30 - maxShift);
    }
    
    return num.Div(&den)->AsLong();
}

////////////////////////////////////////////////////////////////////////////////////

int32_t hsWide::Sqrt() const
{
    int     bits = 32;
    uint32_t  root = 0;
    uint32_t  valueH = (uint32_t)fHi;
    uint32_t  valueL = fLo;
    uint32_t  currH = 0;
    uint32_t  currL = 0;
    uint32_t  guessH, guessL;
    
    do {
        WIDE_SHIFTLEFT(currH, currL, currH, currL, 2);
        currL |= TOP2BITS(valueH);
        WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 2);      
        WIDE_SHIFTLEFT(guessH, guessL, 0, root, 2);
        root <<= 1;
        if (WIDE_LESSTHAN(guessH, guessL, currH, currL))
        {   WIDE_ADDPOS(guessH, guessL, 1);
            WIDE_SUBWIDE(currH, currL, guessH, guessL);
            root |= 1;
        }
    } while (--bits);

    return (int32_t)root;
}

int32_t hsWide::CubeRoot() const
{
    int     bits = 21;
    uint32_t  root = 0;
    uint32_t  valueH = (uint32_t)fHi;
    uint32_t  valueL = fLo;
    uint32_t  currH, currL;
    uint32_t  guessH, guessL;
    hsBool  neg = false;

    if (WIDE_ISNEG(valueH, valueL))
    {   neg = true;
        WIDE_NEGATE(valueH, valueL);
    }

    currH = currL = 0;
    WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 1);  
    do {
        WIDE_SHIFTLEFT(currH, currL, currH, currL, 3);
        currL |= TOP3BITS(valueH);
        WIDE_SHIFTLEFT(valueH, valueL, valueH, valueL, 3);      

        root <<= 1;

        hsWide w;
        w.Mul(root, root)->Add(root);
    #if 0
        w.Mul(3);
    #else
        hsWide w2 = w;
        w.ShiftLeft(1)->Add(&w2);
    #endif
        guessH = (uint32_t)w.fHi;
        guessL = w.fLo;

        if (WIDE_LESSTHAN(guessH, guessL, currH, currL))
        {   WIDE_ADDPOS(guessH, guessL, 1);
            WIDE_SUBWIDE(currH, currL, guessH, guessL);
            root |= 1;
        }
    } while (--bits);

    if (neg)
        root = -int32_t(root);
    return (int32_t)root;
}

