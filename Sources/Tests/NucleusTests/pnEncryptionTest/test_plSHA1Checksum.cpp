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

TEST(plSHA1Checksum, ctor_with_buffer)
{
    const char* buffer = "Hello World";
    const char* hexStr = "0a4d55a8d778e5022fab701977c5d840bbc486d0";
    const ShaDigest value = {0x0a, 0x4d, 0x55, 0xa8,
                             0xd7, 0x78, 0xe5, 0x02,
                             0x2f, 0xab, 0x70, 0x19,
                             0x77, 0xc5, 0xd8, 0x40,
                             0xbb, 0xc4, 0x86, 0xd0};

    plSHA1Checksum sum(strlen(buffer), (uint8_t*)buffer);

    EXPECT_EQ(sum.GetSize(), sizeof(value));
    EXPECT_EQ(memcmp(sum.GetValue(), value, 20), 0);
    EXPECT_STREQ(sum.GetAsHexString(), hexStr);
}
