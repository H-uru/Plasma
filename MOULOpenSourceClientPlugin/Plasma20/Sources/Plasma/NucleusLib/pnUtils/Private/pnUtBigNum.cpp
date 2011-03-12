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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnUtils/Private/pnUtBigNum.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/****************************************************************************
*
*   Constants and macros
*
***/

const unsigned      VAL_BITS = 8 * sizeof(BigNum::Val);
const BigNum::DVal VAL_RANGE = ((BigNum::DVal)1) << VAL_BITS;

#define  LOW(dval)        ((Val)(dval))
#define  HIGH(dval)       ((Val)((dval) / VAL_RANGE))
#define  PACK(low, high)  ((DVal)((high) * VAL_RANGE + (low)))

#define  ALLOC_TEMP(struct, count)  \
    (struct).UseTempAlloc((Val *)_alloca((count) * sizeof(Val)), count)


/****************************************************************************
*
*   BigNum private methods
*
***/

//===========================================================================
void BigNum::SetVal (unsigned index, Val value) {
    ARRAY(Val)::operator[](index) = value;
}

//===========================================================================
void BigNum::SetVal (unsigned index, DVal value, Val * carry) {
    ARRAY(Val)::operator[](index) = LOW(value);
    *carry = HIGH(value);
}

//===========================================================================
void BigNum::Trim (unsigned count) {
    ASSERT(count <= Count());
    while (count && !ARRAY(Val)::operator[](count - 1))
        --count;
    SetCountFewer(count);
}

//===========================================================================
BigNum * BigNum::UseTempAlloc (Val * ptr, unsigned count) {
    m_isTemp = true;
    AttachTemp(ptr, count);
    return this;
}


/****************************************************************************
*
*   BigNum public methods
*
***/

//===========================================================================
BigNum::BigNum () :
    m_isTemp(false)
{
}

//===========================================================================
BigNum::BigNum (const BigNum & a) :
    m_isTemp(false)
{
    Set(a);
}

//===========================================================================
BigNum::BigNum (unsigned a) :
    m_isTemp(false)
{
    Set(a);
}

//===========================================================================
BigNum::BigNum (unsigned bytes, const void * data) :
    m_isTemp(false)
{
    FromData(bytes, data);
}

//===========================================================================
BigNum::BigNum (const wchar str[], Val radix) :
    m_isTemp(false)
{
    FromStr(str, radix);
}

//===========================================================================
BigNum::~BigNum () {
    if (m_isTemp)
        Detach();
}

//===========================================================================
void BigNum::Add (const BigNum & a, Val b) {
    // this = a + b

    const unsigned count = a.Count();
    GrowToCount(count + 1, true);
    unsigned index = 0;
    Val      carry = b;
    for (; index < count; ++index)
        SetVal(index, (DVal)((DVal)a[index] + (DVal)carry), &carry);
    if (carry)
        SetVal(index++, carry);
    Trim(index);

}

//===========================================================================
void BigNum::Add (const BigNum & a, const BigNum & b) {
    // this = a + b

    const unsigned aCount = a.Count();
    const unsigned bCount = b.Count();
    const unsigned count  = aCount + bCount;
    GrowToCount(count + 1, true);
    unsigned index = 0;
    Val      carry = 0;
    for (; index < count; ++index) {
        Val aVal = (index < aCount) ? a[index] : (Val)0;
        Val bVal = (index < bCount) ? b[index] : (Val)0;
        SetVal(index, (DVal)((DVal)aVal + (DVal)bVal + (DVal)carry), &carry);
    }
    if (carry)
        SetVal(index++, carry);
    Trim(index);

}

//===========================================================================
int BigNum::Compare (Val a) const {
    // -1 if (this <  a)
    //  0 if (this == a)
    //  1 if (this >  a)

    // Handle the case where this number has more digits than the comparand
    const unsigned count = Count();
    ASSERT(!count || (*this)[count - 1]);
    if (count > 1)
        return 1;

    // Handle the case where this number has fewer digits than the comparand
    if (!count)
        return a ? -1 : 0;

    // Handle the case where both numbers share the same number of digits
    Val thisVal = (*this)[0];
    return (thisVal > a) ? 1 : (thisVal < a) ? -1 : 0;

}

//===========================================================================
int BigNum::Compare (const BigNum & a) const {
    // -1 if (this <  a)
    //  0 if (this == a)
    //  1 if (this >  a)

    // Handle the case where this number has more digits than the comparand
    const unsigned thisCount = Count();
    const unsigned compCount = a.Count();
    ASSERT(!thisCount || (*this)[thisCount - 1]);
    ASSERT(!compCount || a[compCount - 1]);
    if (thisCount > compCount)
        return 1;

    // Handle the case where this number has fewer digits than the comparand
    if (thisCount < compCount)
        return -1;

    // Handle the case where both numbers share the same number of digits
    for (unsigned index = thisCount; index--; ) {
        Val thisVal = (*this)[index];
        Val compVal = a[index];
        if (thisVal == compVal)
            continue;
        return (thisVal > compVal) ? 1 : -1;
    }
    return 0;

}

//===========================================================================
void BigNum::Div (const BigNum & a, Val b, Val * remainder) {
    // this = a / b, remainder = a % b

    const unsigned count = a.Count();
    SetCount(count);
    *remainder = 0;
    for (unsigned index = count; index--; ) {
        DVal value = PACK(a[index], *remainder);
        SetVal(index, (Val)(value / b));
        *remainder = (Val)(value % b);
    }
    Trim(count);

}

//===========================================================================
void BigNum::Div (const BigNum & a, const BigNum & b, BigNum * remainder) {
    // this = a / b, remainder = a % b
    // either this or remainder may be nil

    ASSERT(this != remainder);

    // Check for division by zero
    ASSERT(b.Count() && b[b.Count() - 1]);

    // Normalize the operands so that the highest bit is set in the most
    // significant word of the denominator
    const unsigned shift = 8 * sizeof(Val) - MathHighBitPos(b[b.Count() - 1]) - 1;
    BigNum aaBuffer;
    BigNum bbBuffer;
    BigNum * aa = shift ? ALLOC_TEMP(aaBuffer, a.Count() + 1) : (BigNum *)&a;
    BigNum * bb = shift ? ALLOC_TEMP(bbBuffer, b.Count() + 1) : (BigNum *)&b;
    if (shift) {
        aa->Shl(a, shift);
        bb->Shl(b, shift);
    }

    // Perform the division
    DivNormalized(*aa, *bb, remainder);

    // Denormalize the remainder
    if (remainder)
        remainder->Shr(*remainder, shift);

}

//===========================================================================
void BigNum::DivNormalized (const BigNum & a, const BigNum & b, BigNum * remainder) {
    // this = a / b, remainder = a % b
    // either this or remainder may be nil
    // high bit of b must be set

    ASSERT(this != remainder);

    // Check for division by zero
    ASSERT(b.Count() && b[b.Count() - 1]);

    // Verify that the operands are normalized
    ASSERT(MathHighBitPos(b[b.Count() - 1]) == 8 * sizeof(Val) - 1);

    // Handle the case where the denominator is greater than the numerator
    if ((b.Count() > a.Count()) || (b.Compare(a) > 0)) {
        if (remainder && (remainder != &a))
            remainder->Set(a);
        if (this)
            ZeroCount();
        return;
    }

    // Create a distinct buffer for the denominator if necessary
    BigNum   denomTemp;
    BigNum * denom = ((&b != this) && (&b != remainder)) ? (BigNum *)&b : ALLOC_TEMP(denomTemp, b.Count());
    denom->Set(b);

    // Store the numerator into the remainder buffer
    BigNum   numerTemp;
    BigNum * numer = remainder ? remainder : ALLOC_TEMP(numerTemp, a.Count());
    numer->Set(a);

    // Prepare the destination buffer
    const unsigned numerCount = numer->Count();
    const unsigned denomCount = denom->Count();
    if (this)
        this->SetCount(numerCount + 1 - denomCount);

    // Calculate the quotient one word at a time
    DVal t = (DVal)((DVal)(*denom)[denomCount - 1] + (DVal)1);
    for (unsigned quotientIndex = numerCount + 1 - denomCount; quotientIndex--; ) {
        
        // Calculate the approximate value of the next quotient word, 
        // erring on the side of underestimation
        Val low  = (*numer)[quotientIndex + denomCount - 1];
        Val high = (quotientIndex + denomCount < numerCount) ? (*numer)[quotientIndex + denomCount] : (Val)0;
        ASSERT(high < t);
        Val quotient = (Val)(PACK(low, high) / t);

        // Calculate the product of the denominator and this quotient word
        // (using zero for all lower quotient words) and subtract the product
        // from the current numerator
        if (quotient) {
            Val borrow = 0;
            Val carry  = 0;
            for (unsigned denomIndex = 0; denomIndex != denomCount; ++denomIndex) {
                DVal product = (DVal)(Mul((*denom)[denomIndex], quotient) + carry);
                carry = HIGH(product);
                numer->SetVal(quotientIndex + denomIndex, (DVal)((DVal)(*numer)[quotientIndex + denomIndex] - (DVal)LOW(product) - (DVal)borrow), &borrow);
                borrow = (Val)((Val)0 - (Val)borrow);
            }
            if (quotientIndex + denomCount != numerCount) {
                numer->SetVal(quotientIndex + denomCount, (DVal)((DVal)(*numer)[quotientIndex + denomIndex] - (DVal)carry - (DVal)borrow), &borrow);
                carry = 0;
            }
            ASSERT(!carry);
            ASSERT(!borrow);
        }

        // Check whether we underestimated the quotient word, and adjust
        // it if necessary
        for (;;) {

            // Test whether the current numerator is still greater than or
            // equal to the denominator
            if ((quotientIndex + denomCount == numerCount) || !(*numer)[quotientIndex + denomCount]) {
                bool numerLess = false;
                for (unsigned denomIndex = denomCount; !numerLess && denomIndex--; ) {
                    Val numerVal = (*numer)[quotientIndex + denomIndex];
                    Val denomVal = (*denom)[denomIndex];
                    numerLess = (numerVal < denomVal);
                    if (numerVal != denomVal)
                        break;
                }
                if (numerLess)
                    break;
            }

            // Increment the quotient by one, and correct the current 
            // numerator for this adjustment by subtracting the denominator
            ++quotient;
            Val borrow = 0;
            for (unsigned denomIndex = 0; denomIndex != denomCount; ++denomIndex) {
                numer->SetVal(quotientIndex + denomIndex, (DVal)((DVal)(*numer)[quotientIndex + denomIndex] - (DVal)(*denom)[denomIndex] - (DVal)borrow), &borrow);
                borrow = (Val)((Val)0 - (Val)borrow);
            }
            if (borrow)
                numer->SetVal(quotientIndex + denomCount, (DVal)((DVal)(*numer)[quotientIndex + denomCount] - (DVal)borrow), &borrow);
            ASSERT(!borrow);

        }
        ASSERT((quotientIndex + denomCount == numerCount) || !(*numer)[quotientIndex + denomCount]);

        // Store the final quotient word
        if (this)
            this->SetVal(quotientIndex, quotient);

    }

    // The final remainder is the remaining portion of the numerator
    if (remainder) {
        ASSERT(remainder == numer);
        remainder->Trim(denomCount);
    }

    // Trim the result
    if (this)
        this->Trim(numerCount + 1 - denomCount);

}

//===========================================================================
void BigNum::FromData (unsigned bytes, const void * data) {
    ASSERT(data || !bytes);

    // Calculate the number of words required to hold the data
    unsigned count = (bytes + sizeof(Val) - 1) / sizeof(Val);
    SetCount(count);

    // Fill in whole words
    unsigned index  = 0;
    unsigned offset = 0;
    for (; offset + sizeof(Val) <= bytes; ++index, offset += sizeof(Val))
        SetVal(index, *(const Val *)((const byte *)data + offset));

    // Fill in the final partial word
    if (offset < bytes) {
        Val value = 0;
        MemCopy(&value, (const byte *)data + offset, bytes - offset);
        SetVal(index, value);
    }

}

//===========================================================================
void BigNum::FromStr (const wchar str[], Val radix) {
    ASSERT(str);    

    // Decode the prefix
    if (str[0] == L'0') {
        if ((str[1] == L'x') || (str[1] == L'X')) {
            str += 2;
            if (!radix)
                radix = 16;
        }
        else if ((str[1] >= L'0') && (str[1] <= L'9')) {
            str += 1;
            if (!radix)
                radix = 8;
        }
        else if (!radix) {
            ZeroCount();
            return;
        }
    }
    else if (!radix)
        radix = 10;

    // Decode the number
    ZeroCount();
    for (; *str; ++str) {

        // Decode the next character
        Val value;
        if ((*str >= L'0') && (*str <= '9'))
            value = (Val)(*str - L'0');
        else if ((*str >= L'a') && (*str <= L'z'))
            value = (Val)(*str + 10 - L'a');
        else if ((*str >= L'A') && (*str <= L'Z'))
            value = (Val)(*str + 10 - L'A');
        else
            break;
        if (value >= radix)
            break;

        // Apply it to the result
        Mul(*this, radix);
        Add(*this, value);

    }

}

//===========================================================================
void BigNum::Gcd (const BigNum & a, const BigNum & b) {

    // Allocate working copies of a and b
    BigNum aa;
    BigNum bb;
    unsigned maxCount = max(a.Count(), b.Count());
    ALLOC_TEMP(aa, maxCount + 1);
    ALLOC_TEMP(bb, maxCount + 1);
    aa.Set(a);
    bb.Set(b);

    // Find the greatest common denominator using Euclid's algorithm
    Set(bb);
    while (aa.Count()) {
        Set(aa);
        aa.Mod(bb, aa);
        bb.Set(*this);
    }

}

//===========================================================================
const void * BigNum::GetData (unsigned * bytes) const {
    if (bytes)
        *bytes = Bytes();
    return Ptr();
}

//===========================================================================
unsigned BigNum::HighBitPos () const {
    // returns the position of the highest set bit, or -1 if no bits are set

    for (unsigned index = Count(); index--; ) {
        Val val = (*this)[index];
        if (!val)
            continue;
        return index * 8 * sizeof(Val) + MathHighBitPos(val);
    }

    return (unsigned)-1;
}

//===========================================================================
bool BigNum::InverseMod (const BigNum & a, const BigNum & b) {
    // finds value for this such that (a ^ -1) == (this mod b)
    // returns false if a has no inverse modulo b

    // Verify that a and b are nonzero
    ASSERT(a.Count());
    ASSERT(b.Count());

    // Verify that a is less than b
    ASSERT(a.Compare(b) < 0);

    // Verify that either a or b is odd. If both are even then they cannot
    // possibly be relatively prime, so there cannot be a solution.
    if (!(a.IsOdd() || b.IsOdd()))
        return false;

    // Allocate buffers for intermediate values
    BigNum uArray[3];
    BigNum tArray[3];
    BigNum * u = uArray;
    BigNum * t = tArray;

    // Find the inverse using the extended Euclidean algorithm
    u[0].SetOne();
    u[1].SetZero();
    u[2].Set(b);
    t[0].Set(a);
    t[1].Sub(b, 1);
    t[2].Set(a);
    do {
        do {
            if (!u[2].IsOdd()) {
                if (u[0].IsOdd() || u[1].IsOdd()) {
                    u[0].Add(u[0], a);
                    u[1].Add(u[1], b);
                }
                u[0].Shr(u[0], 1);
                u[1].Shr(u[1], 1);
                u[2].Shr(u[2], 1);
            }
            if (!t[2].IsOdd() || (u[2].Compare(t[2]) < 0))
                SWAP(u, t);
        } while (!u[2].IsOdd());

        while ((u[0].Compare(t[0]) < 0) || (u[1].Compare(t[1]) < 0)) {
            u[0].Add(u[0], a);
            u[1].Add(u[1], b);
        }

        u[0].Sub(u[0], t[0]);
        u[1].Sub(u[1], t[1]);
        u[2].Sub(u[2], t[2]);
    } while (t[2].Count());

    while ((u[0].Compare(a) >= 0) && (u[1].Compare(b) >= 0)) {
        u[0].Sub(u[0], a);
        u[1].Sub(u[1], b);
    }

    // If the greatest common denominator is not one then there is no
    // solution
    if (u[2].Compare(1))
        return false;

    // Return the solution
    Sub(b, u[1]);
    return true;

}

//===========================================================================
bool BigNum::IsMultiple (Val a) const {
    // returns true if (this % a) == 0

    DVal remainder = 0;
    for (unsigned index = Count(); index--; )
        remainder = (DVal)(PACK((*this)[index], remainder) % a);

    return !remainder;
}

//===========================================================================
bool BigNum::IsOdd () const {
    // returns true if this is an odd number

    return Count() ? (*this)[0] & 1 : false;
}

//===========================================================================
bool BigNum::IsPrime () const {
    // returns true if there is a strong likelihood that this is prime

    // Verify that the number is odd, or is exactly equal to two
    if (!Count() || (!((*this)[0] & 1) && ((Count() > 1) || ((*this)[0] > 2))))
        return false;

    // Verify that the number is not evenly divisible by a small prime
    static const Val smallPrimes[] = {3, 5, 7, 11};
    unsigned loop;
    for (loop = 0; loop != arrsize(smallPrimes); ++loop)
        if (IsMultiple(smallPrimes[loop]))
            return false;
    if (Compare(smallPrimes[arrsize(smallPrimes)-1]) <= 0)
        return true;

    // Rabin-Miller Test

    // Calculate b, where b is the number of times 2 divides (this - 1)
    BigNum this_1;
    ALLOC_TEMP(this_1, Count());
    this_1.Sub(*this, 1);
    const unsigned b = this_1.LowBitPos();

    // Calculate m, such that this == 1 + 2 ^ b * m
    BigNum m;
    ALLOC_TEMP(m, Count());
    m.Shr(this_1, b);

    // For a number of witnesses, test whether the witness demonstrates this
    // number to be composite via Fermat's Little Theorem, or has a 
    // nontrivial square root mod n
    static const Val witnesses[] = {3, 5, 7};
    BigNum z;
    ALLOC_TEMP(z, 2 * (Count() + 1));
    for (loop = 0; loop != arrsize(witnesses); ++loop) {

        // Initialize z to (witness ^ m % this)
        z.PowMod(witnesses[loop], m, *this);

        // This passes the test if (z == 1)
        if (!z.Compare(1))
            continue;

        for (unsigned j = 0; z.Compare(this_1); ) {

            // This fails the test if we reach b iterations.
            ++j;
            if (j == b)
                return false;

            // Square z. This fails the test if z mod this equals 1.
            z.MulMod(z, z, *this);
            if (!z.Compare(1))
                return false;

        }      

    }

    return true;
}

//===========================================================================
unsigned BigNum::LowBitPos () const {
    // returns the position of the lowest set bit, or -1 if no bits are set

    for (unsigned index = 0, count = Count(); index < count; ++index) {
        Val val = (*this)[index];
        if (!val)
            continue;
        for (unsigned bit = 0; ; ++bit)
            if (val & (1 << bit))
                return index * 8 * sizeof(Val) + bit;
    }

    return (unsigned)-1;
}

//===========================================================================
void BigNum::Mod (const BigNum & a, const BigNum & b) {
    // this = a % b

    ((BigNum *)nil)->Div(a, b, this);
}

//===========================================================================
void BigNum::ModNormalized (const BigNum & a, const BigNum & b) {
    // this = a % b
    // high bit of b must be set

    ((BigNum *)nil)->DivNormalized(a, b, this);
}

//===========================================================================
BigNum::DVal BigNum::Mul (BigNum::Val a, BigNum::Val b) {
    // returns a * b

    return (DVal)a * (DVal)b;
}

//===========================================================================
void BigNum::Mul (const BigNum & a, Val b) {
    // this = a * b

    const unsigned count = a.Count();
    GrowToCount(count + 1, true);
    unsigned index = 0;
    Val      carry = 0;
    for (; index < count; ++index)
        SetVal(index, (DVal)(Mul(a[index], b) + carry), &carry);
    if (carry)
        SetVal(index++, carry);
    Trim(index);

}

//===========================================================================
void BigNum::Mul (const BigNum & a, const BigNum & b) {
    // this = a * b

    const unsigned aCount = a.Count();
    const unsigned bCount = b.Count();
    const unsigned count  = aCount + bCount;
    SetCount(count);
    if (!count)
        return;

    // We perform the multiplication from left to right, so that we don't
    // overwrite any operand words before they're used in the case that
    // the destination is not distinct from either of the operands
    SetVal(count - 1, 0);
    for (unsigned index = count - 1; index--; ) {
        
        // Iterate every combination of aIndex + bIndex that adds up to 
        // index, and sum the products of those words
        DVal value = 0;
        const unsigned aStart = (index < bCount) ? 0 : (index + 1 - bCount);
        const unsigned aTerm  = min(index + 1, aCount);
        for (unsigned aIndex = aStart; aIndex != aTerm; ++aIndex) {

            // Accumulate the product of this pair of words
            value = (DVal)(Mul(a[aIndex], b[index - aIndex]) + value);

            // If the product exceeds the word size, apply carry
            Val carry = HIGH(value);
            for (unsigned carryIndex = index + 1; carry; ++carryIndex)
                SetVal(carryIndex, (DVal)((DVal)(*this)[carryIndex] + (DVal)carry), &carry);
            value = LOW(value);

        }

        // Store the sum of products as the final value for index
        SetVal(index, LOW(value));

    }

    Trim(count);

}

//===========================================================================
void BigNum::MulMod (const BigNum & a, const BigNum & b, const BigNum & c) {
    // this = a * b % c

    if (this != &c) {
        Mul(a, b);
        Mod(*this, c);
    }
    else {
        BigNum temp;
        ALLOC_TEMP(temp, a.Count() + b.Count());
        temp.Mul(a, b);
        Mod(temp, c);
    }

}

//===========================================================================
void BigNum::PowMod (Val a, const BigNum & b, const BigNum & c) {
    // this = a ^ b % c

    // Verify that b is distinct from this
    BigNum bbBuffer;
    const BigNum & bb = (&b != this) ? b : bbBuffer;
    if (&bb != &b) {
        ALLOC_TEMP(bbBuffer, b.Count());
        bbBuffer.Set(b);
    }

    // Generate a table which may allow us to process two bits at once
    Val aMult[4] = {
        1,
        a,
        (Val)(a * a),
        (Val)(a * a * a)
    };
    bool overflow = (aMult[2] < a) || (aMult[3] < a) || (c.Compare(aMult[3]) <= 0);

    // Normalize the denominator so that the high bit is set. The result
    // will be built shifted an equivalent amount.
    const unsigned shift = 8 * sizeof(Val) - MathHighBitPos(c[c.Count() - 1]) - 1;
    BigNum cc;
    ALLOC_TEMP(cc, c.Count() + 1);
    cc.Shl(c, shift);

    // Perform the exponentiation from left to right two bits at a time
    if (!overflow) {
        SetBits(shift, 1);
        bool anySet = false;
        for (unsigned index = bb.Count(); index--; )
            for (unsigned bit = 8 * sizeof(Val); bit; ) {
                bit -= 2;

                if (anySet) {
                    Square(*this);
                    Shr(*this, shift);
                    ModNormalized(*this, cc);
                    Square(*this);
                    Shr(*this, shift);
                    ModNormalized(*this, cc);
                }

                unsigned entry = (bb[index] >> bit) & 3;
                if (entry) {
                    Mul(*this, aMult[entry]);
                    ModNormalized(*this, cc);
                    anySet = true;
                }

            }
    }

    // Perform the exponentiation from left to right a single bit at a time
    else {
        SetBits(shift, 1);
        bool anySet = false;
        for (unsigned index = bb.Count(); index--; )
            for (unsigned bit = 8 * sizeof(Val); bit--; ) {

                if (anySet) {
                    Square(*this);
                    ModNormalized(*this, cc);
                }

                if (bb[index] & (1 << bit)) {
                    Mul(*this, a);
                    ModNormalized(*this, cc);
                    anySet = true;
                }

            }
    }

    // Denormalize the result
    Shr(*this, shift);

}

//===========================================================================
void BigNum::PowMod (const BigNum & a, const BigNum & b, const BigNum & c) {
    // this = a ^ b % c

    // Verify that a and b are distinct from this
    BigNum distinctTemp;
    const BigNum & aa = (&a != this) ? a : distinctTemp;
    const BigNum & bb = (&b != this) ? b : distinctTemp;
    if ((&aa != &a) || (&bb != &b)) {
        ALLOC_TEMP(distinctTemp, Count());
        distinctTemp.Set(*this);
    }

    // Generate a table which will allow us to process two bits at once
    BigNum a2;
    BigNum a3;
    ALLOC_TEMP(a2, 2 * aa.Count() + 1);
    ALLOC_TEMP(a3, 3 * aa.Count() + 1);
    a2.Mul(aa, aa);
    a2.Mod(a2, c);
    a3.Mul(aa, a2);
    a3.Mod(a3, c);
    const BigNum * aMult[] = {
        nil,
        &aa,
        &a2,
        &a3
    };

    // Normalize the denominator so that the high bit is set. The result
    // will be built shifted an equivalent amount.
    const unsigned shift = 8 * sizeof(Val) - MathHighBitPos(c[c.Count() - 1]) - 1;
    BigNum cc;
    ALLOC_TEMP(cc, c.Count() + 1);
    cc.Shl(c, shift);

    // Perform the exponentiation from left to right two bits at a time
    SetBits(shift, 1);
    bool anySet = false;
    for (unsigned index = bb.Count(); index--; )
        for (unsigned bit = 8 * sizeof(Val); bit; ) {
            bit -= 2;

            if (anySet) {
                Square(*this);
                Shr(*this, shift);
                ModNormalized(*this, cc);
                Square(*this);
                Shr(*this, shift);
                ModNormalized(*this, cc);
            }

            unsigned entry = (bb[index] >> bit) & 3;
            if (entry) {
                Mul(*this, *aMult[entry]);
                ModNormalized(*this, cc);
                anySet = true;
            }

        }

    // Denormalize the result
    Shr(*this, shift);

}

//===========================================================================
void BigNum::Rand (const BigNum & a, BigNum * seed) {
    // this = random number less than a

    ASSERT(seed != &a);
    ASSERT(seed != this);

    // Verify that a is distinct from this
    BigNum distinctTemp;
    const BigNum & aa = (&a != this) ? a : distinctTemp;
    if (&aa != &a) {
        ALLOC_TEMP(distinctTemp, a.Count());
        distinctTemp.Set(a);
    }

    // Count the number of bits in a
    unsigned bits = aa.HighBitPos() + 1;

    for (;;) {

        // Generate a random number with the same number of bits as a
        Rand(bits, seed);

        // Check whether the number is less than a
        if (Compare(aa) < 0)
            break;

    }        

}

//===========================================================================
void BigNum::Rand (unsigned bits, BigNum * seed) {
    // this = random number with bits or fewer bits

    ASSERT(seed != this);

    // Prepare the output buffer
    const unsigned count = (bits + 8 * sizeof(Val) - 1) / (8 * sizeof(Val));
    SetCount(count);
    if (!count)
        return;

    // Prepare the seed
    unsigned seedCount = seed->Count();
    if (!seedCount)
        seed->SetCount(++seedCount);
    unsigned seedIndex = 0;

    // Produce a random number with the correct number of words
    for (unsigned index = 0; index < count; ++index) {

        // Read the next word of the seed
        dword randValue = (*seed)[seedIndex] ^ ((index == seedIndex) ? 0x075bd924 : 0);

        // Produce one word of randomness, 16 bits at a time
        Val value = 0;
        for (unsigned bit = 0; bit < 8 * sizeof(Val); bit += 16) {
            const dword A = 0xbc8f;
            const dword Q = 0xadc8;
            const dword R = 0x0d47;

            dword div  = randValue / Q;
            randValue  = A * (randValue - Q * div) - R * div;
            randValue &= 0x7fffffff;

            value |= (randValue & 0xffff) << bit;
        }

        // Store the random word
        SetVal(index, value);

        // Update the seed and move to the seed next word
        seed->SetVal(seedIndex, (Val)randValue);
        if (++seedIndex >= seedCount)
            seedIndex = 0;

    }

    // Mask the final word to contain the correct number of bits
    Val mask = (Val)((Val)-1 >> (count * 8 * sizeof(Val) - bits));
    SetVal(count - 1, (Val)((*this)[count - 1] & mask));

    // Trim the result
    Trim(count);

    // Rotate the seed so the next unused seed word will be the first seed
    // word used in the next random operation
    if (seedIndex) {
        BigNum saved;
        ALLOC_TEMP(saved, seedCount);
        saved.Set(*seed);

        for (unsigned index = 0; index < seedCount; ++index)
            (*seed)[index] = saved[(index + seedIndex) % seedCount];
    }

}

//===========================================================================
void BigNum::RandPrime (unsigned bits, BigNum * seed) {

    // Calculate the required number of words to hold the generated number
    unsigned count = (bits + 8 * sizeof(Val) - 1) / (8 * sizeof(Val));

    // For large bit counts, calculate the prime number as 2 * q * n + 1,
    // where q is a random prime with fewer bits, and n is a random number
    // chosen as follows:
    // n >= (2 ^ (bits - 1) - 1) / (2 * q)
    // n <  (2 ^ bits - 1) / (2 * q)
    if (bits > 128) {

        // Choose a prime number q, and multiply it by 2
        BigNum q_2;
        ALLOC_TEMP(q_2, count / 2 + 2);
        q_2.RandPrime(bits / 2, seed);
        q_2.Mul(q_2, 2);

        // Calculate the lower bound
        BigNum lowerBound;
        ALLOC_TEMP(lowerBound, count + 1);
        lowerBound.SetBits(0, bits - 1);
        lowerBound.Div(lowerBound, q_2, nil);

        // Calculate the upper bound
        BigNum upperBound;
        ALLOC_TEMP(upperBound, count + 1);
        upperBound.SetBits(0, bits);
        upperBound.Div(upperBound, q_2, nil);

        // Calculate the number of bits in the upper bound
        unsigned upperBoundBits = upperBound.HighBitPos() + 1;

        for (;;) {

            // Choose a random number between the lower and upper bounds
            Rand(upperBoundBits, seed);
            if (Compare(upperBound) >= 0)
                continue;
            if (Compare(lowerBound) < 0)
                continue;

            // Calculate 2 * q * n + 1
            Mul(*this, q_2);
            Add(*this, 1);

            // Test whether the result is prime
            if (IsPrime())
                break;

        }

    }

    // For small bit counts, choose a random number with the requested
    // number of bits, then keep incrementing it until we find a prime
    else {

        // Define the upper bound for a number with the requested number 
        // of bits
        BigNum bound;
        ALLOC_TEMP(bound, count + 1);
        bound.SetBits(bits, 1);

        for (;;) {

            // Choose a random number with (bits - 1) bits
            Rand(bits - 1, seed);

            // Subtract it from the upper bound to generate a number with 
            // the high bit set
            Sub(bound, *this);

            // Keep incrementing the number until we find a prime
            if (!IsOdd())
                Add(*this, 1);
            while (!IsPrime())
                Add(*this, 2);

            // If the number reached or exceeded the upper bound, try again
            if (Compare(bound) < 0)
                break;

        }

    }

}

//===========================================================================
void BigNum::Set (const BigNum & a) {
    // this = a

    if (&a == this)
        return;

    const unsigned count = a.Count();
    SetCount(count);
    for (unsigned index = count; index--; )
        SetVal(index, a[index]);

}

//===========================================================================
void BigNum::Set (unsigned a) {
    // this = a
        
    ZeroCount();
    if (a)
        for (unsigned index = 0; ; ++index) {
            SetCount(index + 1);
            SetVal(index, LOW(a));
            if (a < VAL_RANGE)
                break;
            a = (unsigned)(a / VAL_RANGE);
        }

}

//===========================================================================
void BigNum::SetBits (unsigned setBitsOffset, unsigned setBitsCount) {
    // this = binary [1...][0...], where 'setBitsOffset' is the number of
    // zero bits and 'setBitsCount' is the number of one bits

    if (!setBitsCount) {
        ZeroCount();
        return;
    }

    const unsigned setBitsTerm  = setBitsOffset + setBitsCount - 1;
    const unsigned bitsPerWord  = 8 * sizeof(Val);
    const unsigned firstSetWord = setBitsOffset / bitsPerWord;
    const unsigned lastSetWord  = (setBitsOffset + setBitsCount - 1) / bitsPerWord;
    Val firstSetMask = (Val)((Val)-1 << (setBitsOffset % bitsPerWord));
    Val lastSetMask  = (Val)((Val)-1 >> (bitsPerWord - setBitsTerm % bitsPerWord - 1));
    if (firstSetWord == lastSetWord)
        firstSetMask = lastSetMask = (Val)(firstSetMask & lastSetMask);

    SetCount(lastSetWord + 1);
    unsigned index = 0;
    for (; index < firstSetWord; ++index)
        SetVal(index, 0);
    SetVal(index++, firstSetMask);
    if (firstSetWord == lastSetWord)
        return;
    for (; index < lastSetWord; ++index)
        SetVal(index, (Val)-1);
    SetVal(index, lastSetMask);

}

//===========================================================================
void BigNum::SetOne () {
    // this = 1

    SetCount(1);
    SetVal(0, 1);
}

//===========================================================================
void BigNum::SetZero () {
    // this = 0

    ZeroCount();
}

//===========================================================================
void BigNum::Shl (const BigNum & a, unsigned b) {
    // this = a << b

    ASSERT(b < 8 * sizeof(Val));
    if (!b) {
        Set(a);
        return;
    }
    const unsigned bInv = 8 * sizeof(Val) - b;

    const unsigned count = a.Count();
    SetCount(count + 1);
    Val curr = 0;
    for (unsigned index = count; index >= 1; --index) {
        Val next = a[index - 1];
        SetVal(index, (Val)((next >> bInv) | (curr << b)));
        curr = next;
    }
    SetVal(0, (Val)(curr << b));
    Trim(count + 1);

}

//===========================================================================
void BigNum::Shr (const BigNum & a, unsigned b) {
    // this = a >> b

    ASSERT(b < 8 * sizeof(Val));
    if (!b) {
        Set(a);
        return;
    }
    const unsigned bInv = 8 * sizeof(Val) - b;

    const unsigned count = a.Count();
    SetCount(count);
    if (!count)
        return;

    Val curr = a[0];
    for (unsigned index = 0; index + 1 < count; ++index) {
        Val next = a[index + 1];
        SetVal(index, (Val)((next << bInv) | (curr >> b)));
        curr = next;
    }
    SetVal(count - 1, (Val)(curr >> b));
    Trim(count);

}

//===========================================================================
void BigNum::Square (const BigNum & a) {
    // this = a * a

    const unsigned aCount = a.Count();
    const unsigned count  = 2 * aCount;
    SetCount(count);
    if (!count)
        return;

    // We perform the multiplication from left to right, so that we don't
    // overwrite any operand words before they're used in the case that
    // the destination is not distinct from the operand
    SetVal(count - 1, 0);
    for (unsigned index = count - 1; index--; ) {
        
        // Iterate every combination of source indices that adds up to 
        // index, and sum the products of those words
        DVal     value  = 0;
        unsigned aIndex = (index < aCount) ? 0 : (index + 1 - aCount);
        unsigned bIndex;
        for (; (int)((bIndex = index - aIndex) - aIndex) >= 0; ++aIndex) {

            // Calculate the product of this pair of words
            DVal product = Mul(a[aIndex], a[bIndex]);

            // Add the product to the sum. If it exceeds the word size,
            // apply carry.
            value = (DVal)(value + product);
            Val carry = HIGH(value);
            unsigned carryIndex;
            for (carryIndex = index + 1; carry; ++carryIndex)
                SetVal(carryIndex, (DVal)((DVal)(*this)[carryIndex] + (DVal)carry), &carry);
            value = LOW(value);

            // If this pair of words should be multiplied twice, add the
            // product again.
            if (aIndex == bIndex)
                continue;
            value = (DVal)(value + product);
            carry = HIGH(value);
            for (carryIndex = index + 1; carry; ++carryIndex)
                SetVal(carryIndex, (DVal)((DVal)(*this)[carryIndex] + (DVal)carry), &carry);
            value = LOW(value);

        }

        // Store the sum of products as the final value for index
        SetVal(index, LOW(value));

    }

    Trim(count);

}

//===========================================================================
void BigNum::Sub (const BigNum & a, Val b) {
    // this = a - b

    const unsigned count = a.Count();
    SetCount(count);
    Val borrow = b;
    for (unsigned index = 0; index < count; ++index) {
        SetVal(index, (DVal)((DVal)a[index] - (DVal)borrow), &borrow);
        borrow = (Val)((Val)0 - (Val)borrow);
    }
    ASSERT(!borrow);
    Trim(index);

}

//===========================================================================
void BigNum::Sub (const BigNum & a, const BigNum & b) {
    // this = a - b

    const unsigned count  = a.Count();
    const unsigned bCount = b.Count();
    GrowToCount(count, true);
    Val borrow = 0;
    for (unsigned index = 0; index < count; ++index) {
        Val bVal = (index < bCount) ? b[index] : (Val)0;
        SetVal(index, (DVal)((DVal)a[index] - (DVal)bVal - (DVal)borrow), &borrow);
        borrow = (Val)((Val)0 - (Val)borrow);
    }
    ASSERT(!borrow);
    Trim(index);

}

//===========================================================================
void BigNum::ToStr (BigNum * buffer, Val radix) const {
    ASSERT(this != buffer);

    // Calculate the number of characters in the prefix
    unsigned prefixChars;
    if (radix == 16)
        prefixChars = 2;
    else if (radix == 8)
        prefixChars = 1;
    else
        prefixChars = 0;

    // Preallocate space for the output string
    unsigned charsPerVal = 0;
    for (Val testVal = (Val)-1; testVal; testVal = (Val)(testVal / radix))
        ++charsPerVal;
    const unsigned charsTotal = max(1, Count()) * charsPerVal + prefixChars + 1;
    buffer->SetCount((charsTotal * sizeof(wchar) + sizeof(Val) - 1) / sizeof(Val));

    // Build the prefix
    wchar * prefix = (wchar *)buffer->Ptr();
    if (prefixChars) {
        prefix[0] = L'0';
        if (radix == 16)
            prefix[1] = L'x';
        else
            ASSERT(prefixChars == 1);
    }

    // Build the number starting with the least significant digit
    wchar * start = prefix + prefixChars;
    wchar * curr  = start;
    BigNum work;
    ALLOC_TEMP(work, Count());
    work.Set(*this);
    do {

        // Extract the next value
        Val remainder;
        work.Div(work, radix, &remainder);

        // Encode it as a character in the output string
        if (remainder >= 10)
            *curr++ = (wchar)(L'a' + (unsigned)remainder - 10);
        else
            *curr++ = (wchar)(L'0' + (unsigned)remainder);

    } while (work.Count());
    *curr = 0;

    // Reverse the order of the output string
    for (wchar * left = start, * right = curr - 1; left < right; ++left, --right)
        SWAP(*left, *right);

}
