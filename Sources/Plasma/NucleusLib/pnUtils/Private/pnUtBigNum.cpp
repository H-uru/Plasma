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

#include <openssl/rand.h>
#include <algorithm>

static inline void byteswap(size_t size, unsigned char * data)
{
    for (size_t i = 0; i < (size / 2); ++i)
        std::swap(data[i], data[size - i - 1]);
}

/****************************************************************************
*
*   BigNum public methods
*
***/

//===========================================================================
BigNum::BigNum () : m_context(nil)
{
    BN_init(&m_number);
}

//===========================================================================
BigNum::BigNum (const BigNum & a) : m_context(nil)
{
    BN_init(&m_number);
    BN_copy(&m_number, &a.m_number);
}

//===========================================================================
BigNum::BigNum (unsigned a) : m_context(nil)
{
    BN_init(&m_number);
    BN_set_word(&m_number, a);
}

//===========================================================================
BigNum::BigNum (unsigned bytes, const void * data, bool le) : m_context(nil)
{
    BN_init(&m_number);
    if (le)
        FromData_LE(bytes, data);
    else
        FromData_BE(bytes, data);
}

//===========================================================================
BigNum::~BigNum ()
{
    if (m_context)
        BN_CTX_free(m_context);
    BN_free(&m_number);
}

//===========================================================================
int BigNum::Compare (dword a) const {
    // -1 if (this <  a)
    //  0 if (this == a)
    //  1 if (this >  a)

    if (BN_is_word(&m_number, a))
        return 0;

    // This returns 0xFFFFFFFFL if the number is bigger than one word, so
    // it doesn't need any size check
    if (BN_get_word(&m_number) < a)
        return -1;

    // Not less or equal, must be greater
    return 1;
}

//===========================================================================
void BigNum::FromData_LE (unsigned bytes, const void * data)
{
    unsigned char * buffer = TRACKED_NEW unsigned char[bytes];
    memcpy(buffer, data, bytes);
    byteswap(bytes, buffer);
    BN_bin2bn(buffer, bytes, &m_number);
    delete [] buffer;
}

//===========================================================================
unsigned char * BigNum::GetData_BE (unsigned * bytes) const
{
    *bytes = BN_num_bytes(&m_number);
    unsigned char * data = TRACKED_NEW unsigned char[*bytes];
    BN_bn2bin(&m_number, data);
    return data;
}

//===========================================================================
unsigned char * BigNum::GetData_LE (unsigned * bytes) const
{
    *bytes = BN_num_bytes(&m_number);
    unsigned char * data = TRACKED_NEW unsigned char[*bytes];
    BN_bn2bin(&m_number, data);
    byteswap(*bytes, data);
    return data;
}

//===========================================================================
void BigNum::Rand (unsigned bits, BigNum * seed)
{
    // this = random number with bits or fewer bits

    unsigned seedBytes;
    unsigned char * seedData = seed->GetData_BE(&seedBytes);
    RAND_seed(seedData, seedBytes);
    BN_rand(&m_number, bits, 0, 0);
    delete [] seedData;
}
