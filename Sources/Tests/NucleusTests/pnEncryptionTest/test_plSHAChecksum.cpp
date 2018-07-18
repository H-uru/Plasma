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

TEST(plSHAChecksum, ctor_with_buffer)
{
    const char buffer[] = "Hello World";
    const char hexStr[] = "45d579c3582a30e6ec0cc15e7ebd586838b0f7fb";
    const ShaDigest value = {0x45, 0xd5, 0x79, 0xc3,
                             0x58, 0x2a, 0x30, 0xe6,
                             0xec, 0x0c, 0xc1, 0x5e,
                             0x7e, 0xbd, 0x58, 0x68,
                             0x38, 0xb0, 0xf7, 0xfb};

    plSHAChecksum sum(strlen(buffer), (const uint8_t*)buffer);

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 20));
    EXPECT_STREQ(hexStr, sum.GetAsHexString());
}

TEST(plSHAChecksum, update)
{
    const char* buffer[] = {"Hello ", "World"};
    const char hexStr[] = "45d579c3582a30e6ec0cc15e7ebd586838b0f7fb";
    const ShaDigest value = {0x45, 0xd5, 0x79, 0xc3,
                             0x58, 0x2a, 0x30, 0xe6,
                             0xec, 0x0c, 0xc1, 0x5e,
                             0x7e, 0xbd, 0x58, 0x68,
                             0x38, 0xb0, 0xf7, 0xfb};

    plSHAChecksum sum;
    sum.Start();
    sum.AddTo(strlen(buffer[0]), (const uint8_t*)buffer[0]);
    sum.AddTo(strlen(buffer[1]), (const uint8_t*)buffer[1]);
    sum.Finish();

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 20));
    EXPECT_STREQ(hexStr, sum.GetAsHexString());
}

TEST(plSHAChecksum, well_known_hashes)
{
    // From NIST FIPS-180
    const char case0_text[] = "";
    const char case0_digest[] = "f96cea198ad1dd5617ac084a3d92c6107708c0ef";
    plSHAChecksum case0(strlen(case0_text), (const uint8_t*)case0_text);
    EXPECT_STREQ(case0_digest, case0.GetAsHexString());

    const char case1_text[] = "abc";
    const char case1_digest[] = "0164b8a914cd2a5e74c4f7ff082c4d97f1edf880";
    plSHAChecksum case1(strlen(case1_text), (const uint8_t*)case1_text);
    EXPECT_STREQ(case1_digest, case1.GetAsHexString());

    const char case2_text[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const char case2_digest[] = "d2516ee1acfa5baf33dfc1c471e438449ef134c8";
    plSHAChecksum case2(strlen(case2_text), (const uint8_t*)case2_text);
    EXPECT_STREQ(case2_digest, case2.GetAsHexString());

    const char case3_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                              "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    const char case3_digest[] = "459f83b95db2dc87bb0f5b513a28f900ede83237";
    plSHAChecksum case3(strlen(case3_text), (const uint8_t*)case3_text);
    EXPECT_STREQ(case3_digest, case3.GetAsHexString());

    // 1,000,000 copies of 'a'
    uint8_t onek_a[1000];
    memset(onek_a, 'a', sizeof(onek_a));
    const char case4_digest[] = "3232affa48628a26653b5aaa44541fd90d690603";
    plSHAChecksum case4;
    case4.Start();
    for (size_t i = 0; i < 1000; ++i)
        case4.AddTo(sizeof(onek_a), onek_a);
    case4.Finish();
    EXPECT_STREQ(case4_digest, case4.GetAsHexString());

#if 0
    // case5_text repeated 16,777,216 times
    // This test is too slow (~12 sec) with the built-in SHA0, but it can be
    // enabled if more conformance testing is desired.
    const char case5_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno";
    const size_t case5_text_len = strlen(case5_text);
    const char case5_digest[] = "bd18f2e7736c8e6de8b5abdfdeab948f5171210c";
    plSHAChecksum case5;
    case5.Start();
    for (size_t i = 0; i < 16777216; ++i)
        case5.AddTo(case5_text_len, (const uint8_t*)case5_text);
    case5.Finish();
    EXPECT_STREQ(case5_digest, case5.GetAsHexString());
#endif
}
