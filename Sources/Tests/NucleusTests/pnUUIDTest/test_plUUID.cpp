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
#include "pnUUID/pnUUID.h"

const char* TEST_UUID = "307c1e1c-c0a2-456b-91f0-fd6faef4920a";

TEST(plUUID, NullUUID)
{
    plUUID u = kNilUuid;
    EXPECT_TRUE(u.IsNull());
    EXPECT_FALSE(u.IsSet());

    plUUID u2;
    EXPECT_TRUE(u2.IsNull());
    EXPECT_FALSE(u2.IsSet());
    EXPECT_TRUE(u2.IsEqualTo(&u));
}

TEST(plUUID, GeneratedUUID)
{
    plUUID u = plUUID::Generate();
    EXPECT_FALSE(u.IsNull());
    EXPECT_TRUE(u.IsSet());
    EXPECT_FALSE(u.IsEqualTo(&kNilUuid));

    plUUID u2 = plUUID::Generate();
    EXPECT_FALSE(u.IsEqualTo(&u2)); // Should be unique
}

TEST(plUUID, Clear)
{
    plUUID u = plUUID::Generate();
    EXPECT_FALSE(u.IsEqualTo(&kNilUuid));

    u.Clear();
    EXPECT_TRUE(u.IsEqualTo(&kNilUuid));
}

TEST(plUUID, CopyFrom)
{
    plUUID u1 = plUUID::Generate();
    plUUID u2 = plUUID::Generate();
    plUUID u3(u2);

    EXPECT_FALSE(u1.IsEqualTo(&u3));
    EXPECT_TRUE(u2.IsEqualTo(&u3));

    u1.CopyFrom(u2);
    EXPECT_TRUE(u1.IsEqualTo(&u3));
}

TEST(plUUID, FromString)
{
    plUUID u1(TEST_UUID);
    plUUID u2;

    EXPECT_FALSE(u1.IsEqualTo(&u2));

    u2.FromString(TEST_UUID);

    EXPECT_TRUE(u1.IsEqualTo(&u2));

    EXPECT_STREQ(TEST_UUID, u1.AsString().c_str());
}

TEST(plUUID, endianness)
{
    plUUID u(TEST_UUID);

    // First 32 bits in little endian
    EXPECT_EQ(0x1c, u.fData[0]);
    EXPECT_EQ(0x1e, u.fData[1]);
    EXPECT_EQ(0x7c, u.fData[2]);
    EXPECT_EQ(0x30, u.fData[3]);

    // Next 16 bits in little endian
    EXPECT_EQ(0xa2, u.fData[4]);
    EXPECT_EQ(0xc0, u.fData[5]);

    // Next 16 bits in little endian
    EXPECT_EQ(0x6b, u.fData[6]);
    EXPECT_EQ(0x45, u.fData[7]);

    // Rest in big endian
    EXPECT_EQ(0x91, u.fData[8]);
    EXPECT_EQ(0xf0, u.fData[9]);

    EXPECT_EQ(0xfd, u.fData[10]);
    EXPECT_EQ(0x6f, u.fData[11]);
    EXPECT_EQ(0xae, u.fData[12]);
    EXPECT_EQ(0xf4, u.fData[13]);
    EXPECT_EQ(0x92, u.fData[14]);
    EXPECT_EQ(0x0a, u.fData[15]);
}
