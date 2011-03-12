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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtBigNum.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTBIGNUM_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtBigNum.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNUTILS_PRIVATE_PNUTBIGNUM_H


/*****************************************************************************
*
*   BigNum class
*
***/

class BigNum : private ARRAY(dword) {
public:
    typedef dword Val;   // must match base array
    typedef qword DVal;  // must be twice as large as Val

private:
    bool m_isTemp;

    void DivNormalized (const BigNum & a, const BigNum & b, BigNum * remainder);
    void ModNormalized (const BigNum & a, const BigNum & b);

    inline static DVal Mul (Val a, Val b);

    inline void SetVal (unsigned index, Val value);
    inline void SetVal (unsigned index, DVal value, Val * carry);
    inline void Trim (unsigned count);
    inline BigNum * UseTempAlloc (Val * ptr, unsigned count);

public:
    BigNum ();
    BigNum (const BigNum & a);
    BigNum (unsigned a);
    BigNum (unsigned bytes, const void * data);
    BigNum (const wchar str[], Val radix);
    ~BigNum ();

    // Constant parameters need not be distinct from the destination or from
    // each other

    void Add (const BigNum & a, Val b);
    void Add (const BigNum & a, const BigNum & b);
    int  Compare (Val a) const;
    int  Compare (const BigNum & a) const;
    void Div (const BigNum & a, Val b, Val * remainder);
    void Div (const BigNum & a, const BigNum & b, BigNum * remainder);
    void FromData (unsigned bytes, const void * data);
    void FromStr (const wchar str[], Val radix);
    void Gcd (const BigNum & a, const BigNum & b);
    const void * GetData (unsigned * bytes) const;
    unsigned HighBitPos () const;
    bool InverseMod (const BigNum & a, const BigNum & b);
    bool IsMultiple (Val a) const;
    bool IsOdd () const;
    bool IsPrime () const;
    unsigned LowBitPos () const;
    void Mod (const BigNum & a, const BigNum & b);
    void Mul (const BigNum & a, Val b);
    void Mul (const BigNum & a, const BigNum & b);
    void MulMod (const BigNum & a, const BigNum & b, const BigNum & c);
    void PowMod (Val a, const BigNum & b, const BigNum & c);
    void PowMod (const BigNum & a, const BigNum & b, const BigNum & c);
    void Rand (const BigNum & a, BigNum * seed);
    void Rand (unsigned bits, BigNum * seed);
    void RandPrime (unsigned bits, BigNum * seed);
    void Set (const BigNum & a);
    void Set (unsigned a);
    void SetBits (unsigned setBitsOffset, unsigned setBitsCount);
    void SetOne ();
    void SetZero ();
    void Shl (const BigNum & a, unsigned b);
    void Shr (const BigNum & a, unsigned b);
    void Square (const BigNum & a);
    void Sub (const BigNum & a, Val b);
    void Sub (const BigNum & a, const BigNum & b);
    void ToStr (BigNum * buffer, Val radix) const;

};
