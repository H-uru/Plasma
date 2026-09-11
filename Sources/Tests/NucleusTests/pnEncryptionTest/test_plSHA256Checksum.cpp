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

TEST(plSHA256Checksum, lifecycle)
{
    plChecksum sum(plChecksum::Type::kSHA256);

    // We can set the checksum value directly if no checksum is in progress.
    EXPECT_NO_THROW(sum.SetFromHexString("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
    EXPECT_THROW(sum.SetFromHexString("1"), plChecksumException);

    // Can't add to or finish until Start() is called.
    EXPECT_THROW(sum.AddTo(1, (const uint8_t*)"a"), plChecksumException);
    EXPECT_THROW(sum.Finish(), plChecksumException);

    // Calling Start() twice doesn't make sense.
    EXPECT_NO_THROW(sum.Start());
    EXPECT_THROW(sum.Start(), plChecksumException);

    // Now that we've started, we can't set the checksum value directly.
    EXPECT_THROW(sum.SetFromHexString("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"), plChecksumException);

    // Can't get the value until Finish() is called.
    EXPECT_THROW(sum.GetValue(), plChecksumException);

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

    // We can set the checksum value directly if no checksum is in progress.
    EXPECT_NO_THROW(sum.SetFromHexString("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));

    // Moving invalidates the source.
    plChecksum sum2 = std::move(sum);
    EXPECT_THROW(sum.GetValue(), plChecksumException);
    EXPECT_THROW(sum.GetSize(), plChecksumException);
    EXPECT_THROW(sum.Start(), plChecksumException);
    EXPECT_THROW(sum.AddTo(1, (const uint8_t*)"a"), plChecksumException);
    EXPECT_THROW(sum.Finish(), plChecksumException);
    EXPECT_THROW(sum.SetFromHexString("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"), plChecksumException);
}

TEST(plSHA256Checksum, ctor_with_buffer)
{
    const char buffer[] = "Hello World";
    const char hexStr[] = "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e";
    const uint8_t value[] = {
        0xa5, 0x91, 0xa6, 0xd4,
        0x0b, 0xf4, 0x20, 0x40,
        0x4a, 0x01, 0x17, 0x33,
        0xcf, 0xb7, 0xb1, 0x90,
        0xd6, 0x2c, 0x65, 0xbf,
        0x0b, 0xcd, 0xa3, 0x2b,
        0x57, 0xb2, 0x77, 0xd9,
        0xad, 0x9f, 0x14, 0x6e,
    };

    plChecksum sum(plChecksum::Type::kSHA256, strlen(buffer), (const uint8_t*)buffer);

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 32));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plSHA256Checksum, update)
{
    const char* buffer[] = {"Hello ", "World"};
    const char hexStr[] = "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e";
    const uint8_t value[] = {
        0xa5, 0x91, 0xa6, 0xd4,
        0x0b, 0xf4, 0x20, 0x40,
        0x4a, 0x01, 0x17, 0x33,
        0xcf, 0xb7, 0xb1, 0x90,
        0xd6, 0x2c, 0x65, 0xbf,
        0x0b, 0xcd, 0xa3, 0x2b,
        0x57, 0xb2, 0x77, 0xd9,
        0xad, 0x9f, 0x14, 0x6e,
    };

    plChecksum sum(plChecksum::Type::kSHA256);
    sum.Start();
    sum.AddTo(strlen(buffer[0]), (const uint8_t*)buffer[0]);
    sum.AddTo(strlen(buffer[1]), (const uint8_t*)buffer[1]);
    sum.Finish();

    EXPECT_EQ(sizeof(value), sum.GetSize());
    EXPECT_EQ(0, memcmp(sum.GetValue(), value, 32));
    EXPECT_STREQ(hexStr, sum.GetAsHexString().c_str());
}

TEST(plSHA256Checksum, well_known_hashes)
{
    const char case0_text[] = "";
    const char case0_digest[] = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    plChecksum case0(plChecksum::Type::kSHA256, strlen(case0_text), (const uint8_t*)case0_text);
    EXPECT_STREQ(case0_digest, case0.GetAsHexString().c_str());

    const char case1_text[] = "abc";
    const char case1_digest[] = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";
    plChecksum case1(plChecksum::Type::kSHA256, strlen(case1_text), (const uint8_t*)case1_text);
    EXPECT_STREQ(case1_digest, case1.GetAsHexString().c_str());

    const char case2_text[] = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    const char case2_digest[] = "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1";
    plChecksum case2(plChecksum::Type::kSHA256, strlen(case2_text), (const uint8_t*)case2_text);
    EXPECT_STREQ(case2_digest, case2.GetAsHexString().c_str());

    const char case3_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmn"
                              "hijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu";
    const char case3_digest[] = "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1";
    plChecksum case3(plChecksum::Type::kSHA256, strlen(case3_text), (const uint8_t*)case3_text);
    EXPECT_STREQ(case3_digest, case3.GetAsHexString().c_str());

    // 1,000,000 copies of 'a'
    uint8_t onek_a[1000];
    memset(onek_a, 'a', sizeof(onek_a));
    const char case4_digest[] = "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0";
    plChecksum case4(plChecksum::Type::kSHA256);
    case4.Start();
    for (size_t i = 0; i < 1000; ++i)
        case4.AddTo(sizeof(onek_a), onek_a);
    case4.Finish();
    EXPECT_STREQ(case4_digest, case4.GetAsHexString().c_str());

    // case5_text repeated 16,777,216 times
    const char case5_text[] = "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmno";
    const size_t case5_text_len = strlen(case5_text);
    const char case5_digest[] = "50e72a0e26442fe2552dc3938ac58658228c0cbfb1d2ca872ae435266fcd055e";
    plChecksum case5(plChecksum::Type::kSHA256);
    case5.Start();
    for (size_t i = 0; i < 16777216; ++i)
        case5.AddTo(case5_text_len, (const uint8_t*)case5_text);
    case5.Finish();
    EXPECT_STREQ(case5_digest, case5.GetAsHexString().c_str());
}
