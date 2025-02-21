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

#include "plBigNum.h"
#include <openssl/rand.h>
#include <algorithm>

static inline void byteswap(size_t size, uint8_t* data)
{
    for (size_t i = 0; i < (size / 2); ++i)
        std::swap(data[i], data[size - i - 1]);
}

/****************************************************************************
*
*   plBigNum public methods
*
***/

plBigNum::plBigNum () : m_context()
{
    m_number = BN_new();
}

plBigNum::plBigNum(const plBigNum& a) : m_context()
{
    m_number = BN_new();
    BN_copy(m_number, a.m_number);
}

plBigNum::plBigNum(unsigned a) : m_context()
{
    m_number = BN_new();
    BN_set_word(m_number, a);
}

plBigNum::~plBigNum ()
{
    if (m_context)
        BN_CTX_free(m_context);
    BN_free(m_number);
}

int plBigNum::Compare(uint32_t a) const
{
    // -1 if (this <  a)
    //  0 if (this == a)
    //  1 if (this >  a)

    if (BN_is_word(m_number, a))
        return 0;

    // This returns 0xFFFFFFFFL if the number is bigger than one uint16_t, so
    // it doesn't need any size check
    if (BN_get_word(m_number) < a)
        return -1;

    // Not less or equal, must be greater
    return 1;
}

void plBigNum::FromData_LE(uint32_t bytes, const void* data)
{
    uint8_t* buffer = new uint8_t[bytes];
    memcpy(buffer, data, bytes);
    byteswap(bytes, buffer);
    BN_bin2bn(buffer, bytes, m_number);
    delete[] buffer;
}

uint8_t* plBigNum::GetData_BE(uint32_t* bytes) const
{
    *bytes = BN_num_bytes(m_number);
    uint8_t* data = new uint8_t[*bytes];
    BN_bn2bin(m_number, data);
    return data;
}

uint8_t* plBigNum::GetData_LE(uint32_t* bytes) const
{
    *bytes = BN_num_bytes(m_number);
    uint8_t* data = new uint8_t[*bytes];
    BN_bn2bin(m_number, data);
    byteswap(*bytes, data);
    return data;
}

void plBigNum::Rand(uint32_t bits, plBigNum* seed)
{
    // this = random number with bits or fewer bits

    uint32_t seedbytes;
    uint8_t* seedData = seed->GetData_BE(&seedbytes);
    RAND_seed(seedData, seedbytes);
    BN_rand(m_number, bits, 0, 0);
    delete[] seedData;
}
