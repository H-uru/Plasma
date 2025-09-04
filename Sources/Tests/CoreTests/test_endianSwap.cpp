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

#include <gtest/gtest.h>

#include "HeadSpin.h"

#include "hsMath.h"

TEST(endianSwap, detection_accuracy)
{
    uint32_t i = 0x01020304;
    uint8_t c[sizeof(i)] {};
    memcpy(c, &i, sizeof(i));

#ifdef HS_BIG_ENDIAN
    EXPECT_EQ(c[0], 0x01);
#else
    EXPECT_EQ(c[0], 0x04);
#endif
}

TEST(endianSwap, toLE16)
{
    uint16_t s = hsToLE16(0x0102);
    uint8_t c[sizeof(s)] {};
    memcpy(c, &s, sizeof(s));

    EXPECT_EQ(c[0], 0x02);
}

TEST(endianSwap, toBE16)
{
    uint16_t s = hsToBE16(0x0102);
    uint8_t c[sizeof(s)] {};
    memcpy(c, &s, sizeof(s));

    EXPECT_EQ(c[0], 0x01);
}

TEST(endianSwap, toLE32)
{
    uint32_t i = hsToLE32(0x01020304);
    uint8_t c[sizeof(i)] {};
    memcpy(c, &i, sizeof(i));

    EXPECT_EQ(c[0], 0x04);
}

TEST(endianSwap, toBE32)
{
    uint32_t i = hsToBE32(0x01020304);
    uint8_t c[sizeof(i)] {};
    memcpy(c, &i, sizeof(i));

    EXPECT_EQ(c[0], 0x01);
}

TEST(endianSwap, toLE64)
{
    uint64_t l = hsToLE64(0x0102030405060708);
    uint8_t c[sizeof(l)] {};
    memcpy(c, &l, sizeof(l));

    EXPECT_EQ(c[0], 0x08);
}

TEST(endianSwap, toBE64)
{
    uint64_t l = hsToBE64(0x0102030405060708);
    uint8_t c[sizeof(l)] {};
    memcpy(c, &l, sizeof(l));

    EXPECT_EQ(c[0], 0x01);
}

TEST(endianSwap, toLEFloat)
{
    // Float value of PI is 0x40490fdb
    float f = hsToLEFloat(hsConstants::pi<float>);
    uint8_t c[sizeof(f)] {};
    memcpy(c, &f, sizeof(f));

    EXPECT_EQ(c[0], 0xdb);
}

TEST(endianSwap, toBEFloat)
{
    // Float value of PI is 0x40490fdb
    float f = hsToBEFloat(hsConstants::pi<float>);
    uint8_t c[sizeof(f)] {};
    memcpy(c, &f, sizeof(f));

    EXPECT_EQ(c[0], 0x40);
}

TEST(endianSwap, toLEDouble)
{
    // Double value of PI is 0x400921fb54442d18
    double d = hsToLEDouble(hsConstants::pi<double>);
    uint8_t c[sizeof(d)] {};
    memcpy(c, &d, sizeof(d));

    EXPECT_EQ(c[0], 0x18);
}

TEST(endianSwap, toBEDouble)
{
    // Double value of PI is 0x400921fb54442d18
    double d = hsToBEDouble(hsConstants::pi<double>);
    uint8_t c[sizeof(d)] {};
    memcpy(c, &d, sizeof(d));

    EXPECT_EQ(c[0], 0x40);
}
