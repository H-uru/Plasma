/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#include "plUnifiedTime/plUnifiedTime.h"

TEST(plUnifiedTime, SetGMTime)
{
    // test with two dates, one at the beginning and one in the middle of the year, so that one of them falls into local DST (if present)
    plUnifiedTime t1;
    t1.SetGMTime(1984, 1, 24, 18, 27, 43, 123456);
    EXPECT_EQ(plUnifiedTime::kGmt, t1.GetMode());
    EXPECT_EQ(1984, t1.GetYear());
    EXPECT_EQ(1, t1.GetMonth());
    EXPECT_EQ(24, t1.GetDay());
    EXPECT_EQ(18, t1.GetHour());
    EXPECT_EQ(27, t1.GetMinute());
    EXPECT_EQ(43, t1.GetSecond());
    EXPECT_EQ(123456, t1.GetMicros());

    plUnifiedTime t2;
    t2.SetGMTime(2003, 7, 10, 5, 46, 14, 654321);
    EXPECT_EQ(plUnifiedTime::kGmt, t2.GetMode());
    EXPECT_EQ(2003, t2.GetYear());
    EXPECT_EQ(7, t2.GetMonth());
    EXPECT_EQ(10, t2.GetDay());
    EXPECT_EQ(5, t2.GetHour());
    EXPECT_EQ(46, t2.GetMinute());
    EXPECT_EQ(14, t2.GetSecond());
    EXPECT_EQ(654321, t2.GetMicros());
}
