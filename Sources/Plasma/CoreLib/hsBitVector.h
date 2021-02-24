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

#ifndef hsBitVector_inc
#define hsBitVector_inc

#include "HeadSpin.h"

#include <vector>

class hsStream;

class hsBitVector {

protected:
    uint32_t*                 fBitVectors;
    uint32_t                  fNumBitVectors;

    void        IGrow(uint32_t newNumBitVectors);

    friend      class hsBitIterator;
public:
    hsBitVector(const hsBitVector& other);
    hsBitVector() : fBitVectors(), fNumBitVectors() { }
    virtual ~hsBitVector() { Reset(); }

    hsBitVector& Reset() { delete [] fBitVectors; fBitVectors = nullptr; fNumBitVectors = 0; return *this; }
    hsBitVector& Clear(); // everyone clear, but no dealloc
    hsBitVector& Set(int upToBit=-1); // WARNING - see comments at function

    bool operator==(const hsBitVector& other) const; // unset (ie uninitialized) bits are clear, 
    bool operator!=(const hsBitVector& other) const { return !(*this == other); }
    hsBitVector& operator=(const hsBitVector& other); // will wind up identical

    bool ClearBit(uint32_t which) { return SetBit(which, 0); } // returns previous state
    bool SetBit(uint32_t which, bool on = true); // returns previous state
    bool IsBitSet(uint32_t which) const; // returns current state
    bool ToggleBit(uint32_t which); // returns previous state
    hsBitVector& RemoveBit(uint32_t which); // removes bit, sliding higher bits down to fill the gap.

    friend inline int Overlap(const hsBitVector& lhs, const hsBitVector& rhs) { return lhs.Overlap(rhs); }
    bool Overlap(const hsBitVector& other) const;
    bool Empty() const;

    bool operator[](uint32_t which) const { return IsBitSet(which); }

    friend inline hsBitVector operator&(const hsBitVector& lhs, const hsBitVector& rhs); // See Overlap()
    friend inline hsBitVector operator|(const hsBitVector& lhs, const hsBitVector& rhs);
    friend inline hsBitVector operator^(const hsBitVector& lhs, const hsBitVector& rhs);
    friend inline hsBitVector operator-(const hsBitVector& lhs, const hsBitVector& rhs); // return lhs w/ rhs's bits turned off

    hsBitVector& operator&=(const hsBitVector& other); // See Overlap()
    hsBitVector& operator|=(const hsBitVector& other);
    hsBitVector& operator^=(const hsBitVector& other);
    hsBitVector& operator-=(const hsBitVector& other); // return me w/ other's bits turned off

    hsBitVector& Compact();
    hsBitVector& SetSize(uint32_t numBits) { ClearBit(numBits+1); return *this; }
    uint32_t GetSize() { return fNumBitVectors << 5; }

    // integer level access
    uint32_t GetNumBitVectors() const { return fNumBitVectors; }
    uint32_t GetBitVector(int i) const { return fBitVectors[i]; }
    void SetNumBitVectors(uint32_t n) { Reset(); fNumBitVectors=n; fBitVectors = new uint32_t[n]; }
    void SetBitVector(int i, uint32_t val) { fBitVectors[i]=val; }

    // Do dst.clear(), then add each set bit's index into dst, returning dst.
    std::vector<int16_t>& Enumerate(std::vector<int16_t>& dst) const;

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

inline hsBitVector::hsBitVector(const hsBitVector& other)
{
    if ((fNumBitVectors = other.fNumBitVectors) != 0) {
        fBitVectors = new uint32_t[fNumBitVectors];
        for (uint32_t i = 0; i < fNumBitVectors; i++)
            fBitVectors[i] = other.fBitVectors[i];
    } else {
        fBitVectors = nullptr;
    }
}

inline bool hsBitVector::Empty() const
{
    for (uint32_t i = 0; i < fNumBitVectors; i++ ) {
        if (fBitVectors[i])
            return false;
    }
    return true;
}

inline bool hsBitVector::Overlap(const hsBitVector& other) const
{
    if (fNumBitVectors > other.fNumBitVectors)
        return other.Overlap(*this);

    for (uint32_t i = 0; i < fNumBitVectors; i++ ){
        if (fBitVectors[i] & other.fBitVectors[i])
            return true;
    }
    return false;
}

inline hsBitVector& hsBitVector::operator=(const hsBitVector& other)
{
    if (this != &other) {
        if (fNumBitVectors < other.fNumBitVectors) {
            Reset();
            fNumBitVectors = other.fNumBitVectors;
            fBitVectors = new uint32_t[fNumBitVectors];
        } else {
            Clear();
        }

        for (uint32_t i = 0; i < other.fNumBitVectors; i++)
            fBitVectors[i] = other.fBitVectors[i];
    }
    return *this;
}

inline bool hsBitVector::operator==(const hsBitVector& other) const
{
    if (fNumBitVectors < other.fNumBitVectors)
        return other.operator==(*this);
    uint32_t i;
    for (i = 0; i < other.fNumBitVectors; i++)
        if (fBitVectors[i] != other.fBitVectors[i])
            return false;
    for (; i < fNumBitVectors; i++)
        if (fBitVectors[i])
            return false;
    return true;
}

inline hsBitVector& hsBitVector::operator&=(const hsBitVector& other)
{
    if (this == &other)
        return *this;

    if (fNumBitVectors > other.fNumBitVectors)
        fNumBitVectors = other.fNumBitVectors;
    for (uint32_t i = 0; i < fNumBitVectors; i++)
        fBitVectors[i] &= other.fBitVectors[i];
    return *this;
}

inline hsBitVector& hsBitVector::operator|=(const hsBitVector& other)
{
    if (this == &other)
        return *this;

    if (fNumBitVectors < other.fNumBitVectors)
        IGrow(other.fNumBitVectors);
    for (uint32_t i = 0; i < other.fNumBitVectors; i++)
        fBitVectors[i] |= other.fBitVectors[i];
    return *this;
}

inline hsBitVector& hsBitVector::operator^=(const hsBitVector& other)
{
    if (this == &other) {
        Clear();
        return *this;
    }

    if (fNumBitVectors < other.fNumBitVectors)
        IGrow(other.fNumBitVectors);
    for (uint32_t i = 0; i < other.fNumBitVectors; i++)
        fBitVectors[i] ^= other.fBitVectors[i];
    return *this;
}

inline hsBitVector& hsBitVector::operator-=(const hsBitVector& other)
{
    if (this == &other) {
        Clear();
        return *this;
    }

    uint32_t minNum = fNumBitVectors < other.fNumBitVectors ? fNumBitVectors : other.fNumBitVectors;
    for (uint32_t i = 0; i < minNum; i++)
        fBitVectors[i] &= ~other.fBitVectors[i];
    return *this;
}

inline hsBitVector operator&(const hsBitVector& rhs, const hsBitVector& lhs)
{
    hsBitVector ret(rhs);
    return ret &= lhs;
}

inline hsBitVector operator|(const hsBitVector& rhs, const hsBitVector& lhs)
{
    hsBitVector ret(rhs);
    return ret |= lhs;
}

inline hsBitVector operator^(const hsBitVector& rhs, const hsBitVector& lhs)
{
    hsBitVector ret(rhs);
    return ret ^= lhs;
}

inline hsBitVector operator-(const hsBitVector& rhs, const hsBitVector& lhs)
{
    hsBitVector ret(rhs);
    return ret -= lhs;
}

inline hsBitVector& hsBitVector::Clear()
{
    for (uint32_t i = 0; i < fNumBitVectors; i++)
        fBitVectors[i] = 0;
    return *this;
}

// WARNING - since the bitvector is conceptually infinitely long,
// we can't actually set all the bits. If you pass in a non-negative
// upToBit, this sets all bits up to and including that one, otherwise
// it just sets however many bits are currently allocated. You can
// assure this is as many as you want by first calling SetSize, but
// if there are more bits than the requested size, these will also
// get set. Calling Set with a non-negative upToBit will only set
// the bits from 0 to upToBit, but won't clear any higher bits.
inline hsBitVector& hsBitVector::Set(int upToBit)
{
    if (upToBit >= 0) {
        uint32_t major = upToBit >> 5;
        uint32_t minor = 1 << (upToBit & 0x1f);
        if (major >= fNumBitVectors)
            IGrow(major+1);

        uint32_t i;
        for (i = 0; i < major; i++)
            fBitVectors[i] = 0xffffffff;
        for (i = 1; i <= minor && i > 0; i <<= 1)
            fBitVectors[major] |= i;
    } else {
        for(uint32_t i = 0; i < fNumBitVectors; i++ )
            fBitVectors[i] = 0xffffffff;
    }
    return *this;
}

inline bool hsBitVector::IsBitSet(uint32_t which) const
{
    uint32_t major = which >> 5;
    return (major < fNumBitVectors) && (0 != (fBitVectors[major] & 1 << (which & 0x1f)));
}

inline bool hsBitVector::SetBit(uint32_t which, bool on)
{
    uint32_t major = which >> 5;
    uint32_t minor = 1 << (which & 0x1f);
    if (major >= fNumBitVectors)
        IGrow(major+1);
    bool ret = 0 != (fBitVectors[major] & minor);
    if (ret != on) {
        if (on)
            fBitVectors[major] |= minor;
        else
            fBitVectors[major] &= ~minor;
    }

    return ret;
}

inline bool hsBitVector::ToggleBit(uint32_t which)
{
    uint32_t major = which >> 5;
    uint32_t minor = 1 << (which & 0x1f);
    if (major >= fNumBitVectors)
        IGrow(major);
    bool ret = 0 != (fBitVectors[major] & minor);
    if (ret)
        fBitVectors[major] &= ~minor;
    else
        fBitVectors[major] |= minor;
    return ret;
}

inline hsBitVector& hsBitVector::RemoveBit(uint32_t which)
{
    uint32_t major = which >> 5;
    if (major >= fNumBitVectors)
        return *this;
    uint32_t minor = 1 << (which & 0x1f);
    uint32_t lowMask = minor-1;
    uint32_t hiMask = ~(lowMask);

    fBitVectors[major] = (fBitVectors[major] & lowMask) | ((fBitVectors[major] >> 1) & hiMask);

    while (major < fNumBitVectors-1) {
        if (fBitVectors[major+1] & 0x1)
            fBitVectors[major] |= 0x80000000;
        else
            fBitVectors[major] &= ~0x80000000;

        major++;

        fBitVectors[major] >>= 1;
    }
    fBitVectors[major] &= ~0x80000000;

    return *this;
}

class hsBitIterator
{
protected:
    const hsBitVector&  fBits;

    int                 fCurrent;

    int                 fCurrVec;
    int                 fCurrBit;

    int                 IAdvanceBit();
    int                 IAdvanceVec();

public:
    // Must call begin after instanciating.
    hsBitIterator(const hsBitVector& bits) : fBits(bits), fCurrent(), fCurrVec(), fCurrBit() { }

    int                 Begin();
    int                 Current() const { return fCurrent; }
    int                 Advance();
    int                 End() const { return fCurrVec < 0; }
};


#endif // hsBitVector_inc
