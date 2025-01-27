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

#ifndef plBigNum_inc
#define plBigNum_inc

/*****************************************************************************
*
*   plBigNum class
*
***/

#include "HeadSpin.h"
#include <openssl/bn.h>

class plBigNum
{
private:
    BIGNUM* m_number;
    mutable BN_CTX* m_context;

    BN_CTX* GetContext() const
    {
        if (!m_context)
            m_context = BN_CTX_new();
        return m_context;
    }

public:
    plBigNum();
    plBigNum(const plBigNum& a);
    plBigNum(uint32_t a);
    ~plBigNum();

    plBigNum& operator=(const plBigNum& a)
    {
        BN_copy(m_number, a.m_number);
        return *this;
    }

    // Constant parameters need not be distinct from the destination or from
    // each other

    void Add(const plBigNum& a, uint32_t b)
    {
        // this = a + b
        BN_copy(m_number, a.m_number);
        BN_add_word(m_number, b);
    }

    void Add(const plBigNum& a, const plBigNum& b)
    {
        // this = a + b
        BN_add(m_number, a.m_number, b.m_number);
    }

    int Compare(uint32_t a) const;

    int Compare(const plBigNum& a) const
    {
        return BN_cmp(m_number, a.m_number);
    }

    bool isZero() const
    {
        return BN_is_zero(m_number);
    }

    void Div(const plBigNum& a, uint32_t b, uint32_t* remainder)
    {
        // this = a / b, remainder = a % b
        BN_copy(m_number, a.m_number);
        *remainder = (uint32_t)BN_div_word(m_number, b);
    }

    void FromData_BE(uint32_t bytess, const void* data)
    {
        BN_bin2bn((const uint8_t*)data, bytess, m_number);
    }

    void FromData_LE(uint32_t bytess, const void* data);

    uint8_t* GetData_BE(uint32_t* bytess) const;
    uint8_t* GetData_LE(uint32_t* bytess) const;

    bool IsPrime() const
    {
#if OPENSSL_VERSION_NUMBER < 0x30000000L
        // Cyan's code uses 3 checks, so we'll follow suit.
        // This provides an accurate answer to p < 0.015625
        return BN_is_prime_fasttest_ex(m_number, 3, GetContext(), 1, nullptr) > 0;
#else
        return BN_check_prime(m_number, GetContext(), nullptr) > 0;
#endif
    }

    void Mod(const plBigNum& a, const plBigNum& b)
    {
        // this = a % b
        BN_div(nullptr, m_number, a.m_number, b.m_number, GetContext());
    }

    void Mul(const plBigNum& a, uint32_t b)
    {
        // this = a * b
        BN_copy(m_number, a.m_number);
        BN_mul_word(m_number, b);
    }

    void Mul(const plBigNum& a, const plBigNum& b)
    {
        // this = a * b
        BN_mul(m_number, a.m_number, b.m_number, GetContext());
    }

    void PowMod(uint32_t a, const plBigNum& b, const plBigNum& c)
    {
        // this = a ^ b % c
        PowMod(plBigNum(a), b, c);
    }

    void PowMod(const plBigNum& a, const plBigNum& b, const plBigNum& c)
    {
        // this = a ^ b % c
        BN_mod_exp(m_number, a.m_number, b.m_number, c.m_number, GetContext());
    }

    void Rand(const plBigNum& a, plBigNum* seed)
    {
        // this = random number less than a
        int bits = BN_num_bits(a.m_number);
        do
            Rand(bits, seed);
        while (Compare(a) >= 0);
    }

    void Rand(uint32_t bits, plBigNum* seed);

    void RandPrime(uint32_t bits, plBigNum* seed)
    {
        BN_generate_prime_ex(m_number, bits, 1, nullptr, nullptr, nullptr);
    }

    void Set(const plBigNum& a)
    {
        BN_copy(m_number, a.m_number);
    }

    void Set(uint32_t a)
    {
        BN_set_word(m_number, a);
    }

    void SetOne() { Set(1); }
    void SetZero() { Set(0); }

    void Shl(const plBigNum& a, uint32_t b)
    {
        // this = a << b
        BN_lshift(m_number, a.m_number, b);
    }

    void Shr(const plBigNum& a, uint32_t b)
    {
        // this = a >> b
        BN_rshift(m_number, a.m_number, b);
    }

    void Sub(const plBigNum& a, uint32_t b)
    {
        // this = a - b
        BN_copy(m_number, a.m_number);
        BN_sub_word(m_number, b);
    }

    void Sub(const plBigNum& a, const plBigNum& b)
    {
        // this = a - b
        BN_sub(m_number, a.m_number, b.m_number);
    }
};
#endif // plBigNum_inc
