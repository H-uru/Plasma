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

#include <cstring>
#include <gtest/gtest.h>
#include "pnEncryption/plChecksum.h"
#include <string_theory/string>

TEST(plMD5Checksum, lifecycle)
{
    plChecksum sum(plChecksum::Type::kMD5);

    // Can't add to or finish until Start() is called.
    EXPECT_THROW(sum.AddTo(1, (const uint8_t*)"a"), plChecksumException);
    EXPECT_THROW(sum.Finish(), plChecksumException);

    // Calling Start() twice doesn't make sense.
    EXPECT_NO_THROW(sum.Start());
    EXPECT_THROW(sum.Start(), plChecksumException);

    // Can't get the value or size until Finish() is called.
    EXPECT_THROW(sum.GetValue(), plChecksumException);
    EXPECT_THROW(sum.GetSize(), plChecksumException);

    // Can't add after Finish() is called.
    EXPECT_NO_THROW(sum.AddTo(1, (const uint8_t*)"a"));
    EXPECT_NO_THROW(sum.Finish());
    EXPECT_THROW(sum.AddTo(1, (const uint8_t*)"a"), plChecksumException);

    // Value and size should work now.
    EXPECT_NO_THROW(sum.GetSize());
    EXPECT_NO_THROW(sum.GetValue());

    // Should be able to restart and reuse the checksum object.
    EXPECT_NO_THROW(sum.Start());
    EXPECT_NO_THROW(sum.AddTo(1, (const uint8_t*)"a"));
    EXPECT_NO_THROW(sum.Finish());

    // Moving invalidates the source.
    plChecksum sum2 = std::move(sum);
    EXPECT_THROW(sum.GetValue(), plChecksumException);
    EXPECT_THROW(sum.GetSize(), plChecksumException);
    EXPECT_THROW(sum.Start(), plChecksumException);
    EXPECT_THROW(sum.AddTo(1, (const uint8_t*)"a"), plChecksumException);
    EXPECT_THROW(sum.Finish(), plChecksumException);
}

TEST(plMD5Checksum, ctor_with_buffer)
{
    const char buffer[] = "Hello World";
    const char hexStr[] = "b10a8db164e0754105b7a99be72e3fe5";
    const uint8_t value[16] = {0xb1, 0x0a, 0x8d, 0xb1, 0x64, 0xe0, 0x75, 0x41,
                               0x05, 0xb7, 0xa9, 0x9b, 0xe7, 0x2e, 0x3f, 0xe5};

    plChecksum sum(plChecksum::Type::kMD5, strlen(buffer), (const uint8_t*)buffer);

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 16));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plMD5Checksum, update)
{
    const char* buffer[] = {"Hello ", "World"};
    const char hexStr[] = "b10a8db164e0754105b7a99be72e3fe5";
    const uint8_t value[16] = {0xb1, 0x0a, 0x8d, 0xb1, 0x64, 0xe0, 0x75, 0x41,
                               0x05, 0xb7, 0xa9, 0x9b, 0xe7, 0x2e, 0x3f, 0xe5};

    plChecksum sum(plChecksum::Type::kMD5);
    sum.Start();
    sum.AddTo(strlen(buffer[0]), (const uint8_t*)buffer[0]);
    sum.AddTo(strlen(buffer[1]), (const uint8_t*)buffer[1]);
    sum.Finish();

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 16));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plMD5Checksum, well_known_hashes)
{
    // From NIST FIPS-180
    const char case0_text[] = "";
    const char case0_digest[] = "d41d8cd98f00b204e9800998ecf8427e";
    plChecksum case0(
        plChecksum::Type::kMD5,
        strlen(case0_text),
        (const uint8_t*)case0_text
    );
    EXPECT_STREQ(case0_digest, case0.GetAsHexString().c_str());

    const char case1_text[] = "abc";
    const char case1_digest[] = "900150983cd24fb0d6963f7d28e17f72";
    plChecksum case1(
        plChecksum::Type::kMD5,
        strlen(case1_text),
        (const uint8_t*)case1_text);
    EXPECT_STREQ(case1_digest, case1.GetAsHexString().c_str());

    const char case2_text[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const char case2_digest[] = "8215ef0796a20bcaaae116d3876c664a";
    plChecksum case2(
        plChecksum::Type::kMD5,
        strlen(case2_text),
        (const uint8_t*)case2_text);
    EXPECT_STREQ(case2_digest, case2.GetAsHexString().c_str());

    const char case3_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                              "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    const char case3_digest[] = "03dd8807a93175fb062dfb55dc7d359c";
    plChecksum case3(
        plChecksum::Type::kMD5,
        strlen(case3_text),
        (const uint8_t*)case3_text
    );
    EXPECT_STREQ(case3_digest, case3.GetAsHexString().c_str());

    // 1,000,000 copies of 'a'
    uint8_t onek_a[1000];
    memset(onek_a, 'a', sizeof(onek_a));
    const char case4_digest[] = "7707d6ae4e027c70eea2a935c2296f21";
    plChecksum case4(plChecksum::Type::kMD5);
    case4.Start();
    for (size_t i = 0; i < 1000; ++i)
        case4.AddTo(sizeof(onek_a), onek_a);
    case4.Finish();
    EXPECT_STREQ(case4_digest, case4.GetAsHexString().c_str());

    // case5_text repeated 16,777,216 times
    const char case5_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno";
    const size_t case5_text_len = strlen(case5_text);
    const char case5_digest[] = "d338139169d50f55526194c790ec0448";
    plChecksum case5(plChecksum::Type::kMD5);
    case5.Start();
    for (size_t i = 0; i < 16777216; ++i)
        case5.AddTo(case5_text_len, (const uint8_t*)case5_text);
    case5.Finish();
    EXPECT_STREQ(case5_digest, case5.GetAsHexString().c_str());
}
