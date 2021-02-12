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
#ifndef hsWideDefined
#define hsWideDefined

#include "HeadSpin.h"

struct hsWide {
    int32_t   fHi;
    uint32_t  fLo;

    hsWide* Set(int32_t lo) { fLo = lo; if (lo < 0) fHi = -1L; else fHi = 0; return this; }
    hsWide* Set(int32_t hi, uint32_t lo) { fHi = hi; fLo = lo; return this; }

    inline bool   IsNeg() const { return fHi < 0; }
    inline bool   IsPos() const { return fHi > 0 || (fHi == 0 && fLo != 0); }
    inline bool   IsZero() const { return fHi == 0 && fLo == 0; }
    inline bool   IsWide() const;


    bool operator==(const hsWide& b) const { return fHi == b.fHi && fLo == b.fLo; }
    bool operator<(const hsWide& b) const { return fHi < b.fHi || (fHi == b.fHi && fLo < b.fLo); }
    bool operator>( const hsWide& b) const { return fHi > b.fHi || (fHi == b.fHi && fLo > b.fLo); }
    bool operator!=( const hsWide& b) const { return !( *this == b); }
    bool operator<=(const hsWide& b) const { return !(*this > b); }
    bool operator>=(const hsWide& b) const { return !(*this < b); }

    inline hsWide*  Negate();
    inline hsWide*  Add(int32_t scaler);
    inline hsWide*  Add(const hsWide* a);
    inline hsWide*  Sub(const hsWide* a);
    inline hsWide*  ShiftLeft(unsigned shift);
    inline hsWide*  ShiftRight(unsigned shift);
    inline hsWide*  RoundRight(unsigned shift);

    inline int32_t    AsLong() const;             // return bits 31-0, checking for over/under flow
    inline int32_t  AsFixed() const;            // return bits 47-16, checking for over/under flow
    inline int32_t  AsFract() const;            // return bits 61-30, checking for over/under flow

    hsWide* Mul(int32_t a);                   // this updates the wide
    hsWide* Mul(int32_t a, int32_t b);          // this sets the wide
    hsWide* Div(int32_t denom);               // this updates the wide
    hsWide* Div(const hsWide* denom);       // this updates the wide

    int32_t FixDiv(const hsWide* denom) const;
    int32_t FracDiv(const hsWide* denom) const;

    int32_t   Sqrt() const;
    int32_t   CubeRoot() const;

    double  AsDouble() const { return fHi * double(65536) * double(65536) + fLo; }
    hsWide* Set(double d) 
    { 
        int32_t hi = int32_t(d / double(65536) / double(65536));
        int32_t lo = int32_t(d - double(hi));
        return Set(hi, lo);
    }

};

const hsWide kPosInfinity64 = { kPosInfinity32, 0xffffffff };
const hsWide kNegInfinity64 = { static_cast<int32_t>(kNegInfinity32), 0 };

/////////////////////// Inline implementations ///////////////////////

#define TOP2BITS(n) (uint32_t(n) >> 30)
#define TOP3BITS(n) (uint32_t(n) >> 29)

    #define hsSignalMathOverflow()
    #define hsSignalMathUnderflow()

#define WIDE_ISNEG(hi, lo)                      (int32_t(hi) < 0)
#define WIDE_LESSTHAN(hi, lo, hi2, lo2)             ((hi) < (hi2) || ((hi) == (hi2) && (lo) < (lo2)))
#define WIDE_SHIFTLEFT(outH, outL, inH, inL, shift)     do { (outH) = ((inH) << (shift)) | ((inL) >> (32 - (shift))); (outL) = (inL) << (shift); } while (0)
#define WIDE_NEGATE(hi, lo)                     do { (hi) = ~(hi); if (((lo) = -int32_t(lo)) == 0) (hi) += 1; } while (0) 
#define WIDE_ADDPOS(hi, lo, scaler)             do { uint32_t tmp = (lo) + (scaler); if (tmp < (lo)) (hi) += 1; (lo) = tmp; } while (0)
#define WIDE_SUBWIDE(hi, lo, subhi, sublo)          do { (hi) -= (subhi); if ((lo) < (sublo)) (hi) -= 1; (lo) -= (sublo); } while (0) 

/////////////////////// Inline implementations ///////////////////////

inline hsWide* hsWide::Negate()
{
    WIDE_NEGATE(fHi, fLo);
    
    return this;
}

inline hsWide* hsWide::Add(int32_t scaler)
{
    if (scaler >= 0)
        WIDE_ADDPOS(fHi, fLo, scaler);
    else
    {   scaler = -scaler;
        if (fLo < uint32_t(scaler))
            fHi--;
        fLo -= scaler;
    }

    return this;
}

inline hsWide* hsWide::Add(const hsWide* a)
{
    uint32_t  newLo = fLo + a->fLo;

    fHi += a->fHi;
    if (newLo < (fLo | a->fLo))
        fHi++;
    fLo = newLo;

    return this;
}

inline hsWide* hsWide::Sub(const hsWide* a)
{
    WIDE_SUBWIDE(fHi, fLo, a->fHi, a->fLo);

    return this;
}

inline hsWide* hsWide::ShiftLeft(unsigned shift)
{
    WIDE_SHIFTLEFT(fHi, fLo, fHi, fLo, shift);

    return this;
}

inline hsWide* hsWide::ShiftRight(unsigned shift)
{
    fLo = (fLo >> shift) | (fHi << (32 - shift));
    fHi = fHi >> shift;     // fHi >>= shift;   Treated as logical shift on CW9-WIN32, which breaks for fHi < 0

    return this;
}

inline hsWide* hsWide::RoundRight(unsigned shift)
{
    return this->Add(1L << (shift - 1))->ShiftRight(shift);
}

inline int32_t hsWide::AsLong() const
{
    return (int32_t)fLo;
}

inline bool hsWide::IsWide() const
{
    return (fHi > 0 || (fHi == 0 && (int32_t)fLo < 0)) || (fHi < -1L || (fHi == -1L && (int32_t)fLo >= 0));
}

inline int32_t hsWide::AsFixed() const
{
    hsWide tmp = *this;

    return tmp.RoundRight(16)->AsLong();
}

inline int32_t hsWide::AsFract() const
{
    hsWide tmp = *this;

    return tmp.RoundRight(30)->AsLong();
}

#endif
