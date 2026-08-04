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

TEST(plSHA512Checksum, lifecycle)
{
    plChecksum sum(plChecksum::Type::kSHA512);

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

TEST(plSHA512Checksum, ctor_with_buffer)
{
    const char buffer[] = "Hello World";
    const char hexStr[] = "2c74fd17edafd80e8447b0d46741ee243b7eb74dd2149a0ab1b9246fb30382f27e853d8585719e0e67cbda0daa8f51671064615d645ae27acb15bfb1447f459b";
    const uint8_t value[] = {
        0x2c, 0x74, 0xfd, 0x17,
        0xed, 0xaf, 0xd8, 0x0e,
        0x84, 0x47, 0xb0, 0xd4,
        0x67, 0x41, 0xee, 0x24,
        0x3b, 0x7e, 0xb7, 0x4d,
        0xd2, 0x14, 0x9a, 0x0a,
        0xb1, 0xb9, 0x24, 0x6f,
        0xb3, 0x03, 0x82, 0xf2,
        0x7e, 0x85, 0x3d, 0x85,
        0x85, 0x71, 0x9e, 0x0e,
        0x67, 0xcb, 0xda, 0x0d,
        0xaa, 0x8f, 0x51, 0x67,
        0x10, 0x64, 0x61, 0x5d,
        0x64, 0x5a, 0xe2, 0x7a,
        0xcb, 0x15, 0xbf, 0xb1,
        0x44, 0x7f, 0x45, 0x9b,
    };

    plChecksum sum(plChecksum::Type::kSHA512, strlen(buffer), (const uint8_t*)buffer);

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 64));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plSHA512Checksum, update)
{
    const char* buffer[] = {"Hello ", "World"};
    const char hexStr[] = "2c74fd17edafd80e8447b0d46741ee243b7eb74dd2149a0ab1b9246fb30382f27e853d8585719e0e67cbda0daa8f51671064615d645ae27acb15bfb1447f459b";
    const uint8_t value[] = {
        0x2c, 0x74, 0xfd, 0x17,
        0xed, 0xaf, 0xd8, 0x0e,
        0x84, 0x47, 0xb0, 0xd4,
        0x67, 0x41, 0xee, 0x24,
        0x3b, 0x7e, 0xb7, 0x4d,
        0xd2, 0x14, 0x9a, 0x0a,
        0xb1, 0xb9, 0x24, 0x6f,
        0xb3, 0x03, 0x82, 0xf2,
        0x7e, 0x85, 0x3d, 0x85,
        0x85, 0x71, 0x9e, 0x0e,
        0x67, 0xcb, 0xda, 0x0d,
        0xaa, 0x8f, 0x51, 0x67,
        0x10, 0x64, 0x61, 0x5d,
        0x64, 0x5a, 0xe2, 0x7a,
        0xcb, 0x15, 0xbf, 0xb1,
        0x44, 0x7f, 0x45, 0x9b,
    };

    plChecksum sum(plChecksum::Type::kSHA512);
    sum.Start();
    sum.AddTo(strlen(buffer[0]), (const uint8_t*)buffer[0]);
    sum.AddTo(strlen(buffer[1]), (const uint8_t*)buffer[1]);
    sum.Finish();

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 64));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plSHA512Checksum, well_known_hashes)
{
    const char case0_text[] = "";
    const char case0_digest[] = "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e";
    plChecksum case0(plChecksum::Type::kSHA512, strlen(case0_text), (const uint8_t*)case0_text);
    EXPECT_STREQ(case0_digest, case0.GetAsHexString().c_str());

    const char case1_text[] = "abc";
    const char case1_digest[] = "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f";
    plChecksum case1(plChecksum::Type::kSHA512, strlen(case1_text), (const uint8_t*)case1_text);
    EXPECT_STREQ(case1_digest, case1.GetAsHexString().c_str());

    const char case2_text[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const char case2_digest[] = "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445";
    plChecksum case2(plChecksum::Type::kSHA512, strlen(case2_text), (const uint8_t*)case2_text);
    EXPECT_STREQ(case2_digest, case2.GetAsHexString().c_str());

    const char case3_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                              "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    const char case3_digest[] = "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909";
    plChecksum case3(plChecksum::Type::kSHA512, strlen(case3_text), (const uint8_t*)case3_text);
    EXPECT_STREQ(case3_digest, case3.GetAsHexString().c_str());

    // 1,000,000 copies of 'a'
    uint8_t onek_a[1000];
    memset(onek_a, 'a', sizeof(onek_a));
    const char case4_digest[] = "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b";
    plChecksum case4(plChecksum::Type::kSHA512);
    case4.Start();
    for (size_t i = 0; i < 1000; ++i)
        case4.AddTo(sizeof(onek_a), onek_a);
    case4.Finish();
    EXPECT_STREQ(case4_digest, case4.GetAsHexString().c_str());

    // case5_text repeated 16,777,216 times
    const char case5_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno";
    const size_t case5_text_len = strlen(case5_text);
    const char case5_digest[] = "b47c933421ea2db149ad6e10fce6c7f93d0752380180ffd7f4629a712134831d77be6091b819ed352c2967a2e2d4fa5050723c9630691f1a05a7281dbe6c1086";
    plChecksum case5(plChecksum::Type::kSHA512);
    case5.Start();
    for (size_t i = 0; i < 16777216; ++i)
        case5.AddTo(case5_text_len, (const uint8_t*)case5_text);
    case5.Finish();
    EXPECT_STREQ(case5_digest, case5.GetAsHexString().c_str());
}
