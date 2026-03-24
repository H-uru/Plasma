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
#include <string_theory/string>

#include "HeadSpin.h"

#include "hsEndian.h"
#include "hsMath.h"

TEST(hsEndian, detection_accuracy)
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

TEST(hsEndian, toLE16)
{
    uint16_t s = hsToLE16(0x0102);
    uint8_t c[sizeof(s)] {};
    memcpy(c, &s, sizeof(s));

    EXPECT_EQ(c[0], 0x02);
}

TEST(hsEndian, toBE16)
{
    uint16_t s = hsToBE16(0x0102);
    uint8_t c[sizeof(s)] {};
    memcpy(c, &s, sizeof(s));

    EXPECT_EQ(c[0], 0x01);
}

TEST(hsEndian, toLE32)
{
    uint32_t i = hsToLE32(0x01020304);
    uint8_t c[sizeof(i)] {};
    memcpy(c, &i, sizeof(i));

    EXPECT_EQ(c[0], 0x04);
}

TEST(hsEndian, toBE32)
{
    uint32_t i = hsToBE32(0x01020304);
    uint8_t c[sizeof(i)] {};
    memcpy(c, &i, sizeof(i));

    EXPECT_EQ(c[0], 0x01);
}

TEST(hsEndian, toLE64)
{
    uint64_t l = hsToLE64(0x0102030405060708);
    uint8_t c[sizeof(l)] {};
    memcpy(c, &l, sizeof(l));

    EXPECT_EQ(c[0], 0x08);
}

TEST(hsEndian, toBE64)
{
    uint64_t l = hsToBE64(0x0102030405060708);
    uint8_t c[sizeof(l)] {};
    memcpy(c, &l, sizeof(l));

    EXPECT_EQ(c[0], 0x01);
}

TEST(hsEndian, toLEFloat)
{
    // Float value of PI is 0x40490fdb
    float f = hsToLEFloat(hsConstants::pi<float>);
    uint8_t c[sizeof(f)] {};
    memcpy(c, &f, sizeof(f));

    EXPECT_EQ(c[0], 0xdb);
}

TEST(hsEndian, toBEFloat)
{
    // Float value of PI is 0x40490fdb
    float f = hsToBEFloat(hsConstants::pi<float>);
    uint8_t c[sizeof(f)] {};
    memcpy(c, &f, sizeof(f));

    EXPECT_EQ(c[0], 0x40);
}

TEST(hsEndian, toLEDouble)
{
    // Double value of PI is 0x400921fb54442d18
    double d = hsToLEDouble(hsConstants::pi<double>);
    uint8_t c[sizeof(d)] {};
    memcpy(c, &d, sizeof(d));

    EXPECT_EQ(c[0], 0x18);
}

TEST(hsEndian, toBEDouble)
{
    // Double value of PI is 0x400921fb54442d18
    double d = hsToBEDouble(hsConstants::pi<double>);
    uint8_t c[sizeof(d)] {};
    memcpy(c, &d, sizeof(d));

    EXPECT_EQ(c[0], 0x40);
}

const ST::string kTestString = ST_LITERAL("hḗllo");
constexpr char kTestStringUtf16[] = {
    'h', 0x00,
    0x17, 0x1e, // U+1E17 LATIN SMALL LETTER E WITH MACRON AND ACUTE
    'l', 0x00,
    'l', 0x00,
    'o', 0x00,
};

TEST(hsEndian, hsSTStringFromUTF16LE)
{
    constexpr size_t bufferSize = sizeof(kTestStringUtf16);
    // Force non-even alignment
    alignas(char16_t) char allocation[bufferSize + 1];
    char* buffer = allocation + 1;

    memcpy(buffer, kTestStringUtf16, bufferSize);

    ST::string string = hsSTStringFromUTF16LE(buffer, bufferSize / sizeof(char16_t));

    EXPECT_EQ(string, kTestString);
}

TEST(hsEndian, hsSTStringFromTerminatedUTF16LE)
{
    constexpr size_t bufferSize = sizeof(kTestStringUtf16) + 5;
    // Force non-even alignment
    alignas(char16_t) char allocation[bufferSize + 1];
    char* buffer = allocation + 1;

    memcpy(buffer, kTestStringUtf16, sizeof(kTestStringUtf16));
    // Add string terminator and some junk data that should be ignored
    buffer[sizeof(kTestStringUtf16)] = 0;
    buffer[sizeof(kTestStringUtf16) + 1] = 0;
    buffer[sizeof(kTestStringUtf16) + 2] = '\234';
    buffer[sizeof(kTestStringUtf16) + 3] = '\235';
    buffer[sizeof(kTestStringUtf16) + 4] = '\236';

    size_t consumedSize;

    ST::string string = hsSTStringFromTerminatedUTF16LE(buffer, bufferSize, consumedSize);
    // consumedSize includes the terminator.
    EXPECT_EQ(consumedSize, sizeof(kTestStringUtf16) + 2);
    EXPECT_EQ(string, kTestString);

    ST::string string2 = hsSTStringFromTerminatedUTF16LE(buffer, sizeof(kTestStringUtf16) + 1, consumedSize);
    // consumedSize does *not* count a terminator if there was none in the source buffer.
    EXPECT_EQ(consumedSize, sizeof(kTestStringUtf16));
    EXPECT_EQ(string2, kTestString);

    ST::string string3 = hsSTStringFromTerminatedUTF16LE(buffer, sizeof(kTestStringUtf16), consumedSize);
    EXPECT_EQ(consumedSize, sizeof(kTestStringUtf16));
    EXPECT_EQ(string3, kTestString);

    // Test that buffers shorter than one char16_t are handled correctly.

    ST::string string4 = hsSTStringFromTerminatedUTF16LE(buffer, 1, consumedSize);
    EXPECT_EQ(consumedSize, 0);
    EXPECT_TRUE(string4.empty());

    ST::string string5 = hsSTStringFromTerminatedUTF16LE(buffer, 0, consumedSize);
    EXPECT_EQ(consumedSize, 0);
    EXPECT_TRUE(string5.empty());
}

TEST(hsEndian, hsSTStringToUTF16LE)
{
    std::vector<uint8_t> buffer = hsSTStringToUTF16LE(kTestString);
    EXPECT_EQ(buffer.size(), sizeof(kTestStringUtf16));
    EXPECT_EQ(memcmp(buffer.data(), kTestStringUtf16, sizeof(kTestStringUtf16)), 0);
}
