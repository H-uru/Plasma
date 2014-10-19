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

#include <iomanip>
#include <gtest/gtest.h>

#include "pnAllCreatables.h"
#include "plAllCreatables.h"
#include "pfAllCreatables.h"

#include "pfPython/cyMisc.h"

// from zoneinfo/America/Denver version 2011g
static const time_t dsttransitions[] = {
    0x45F3C510, 0x472D7C00,
    0x47D3A710, 0x490D5E00,
    0x49B38910, 0x4AED4000,
    0x4B9CA590, 0x4CD65C80,
    0x4D7C8790, 0x4EB63E80,
    0x4F5C6990, 0x50962080,
    0x513C4B90, 0x52760280,
    0x531C2D90, 0x5455E480,
    0x54FC0F90, 0x5635C680,
    0x56E52C10, 0x581EE300,
    0x58C50E10, 0x59FEC500,
    0x5AA4F010, 0x5BDEA700,
    0x5C84D210, 0x5DBE8900,
    0x5E64B410, 0x5F9E6B00,
    0x604DD090, 0x61878780,
    0x622DB290, 0x63676980,
    0x640D9490, 0x65474B80,
    0x65ED7690, 0x67272D80,
    0x67CD5890, 0x69070F80,
    0x69AD3A90, 0x6AE6F180,
    0x6B965710, 0x6CD00E00,
    0x6D763910, 0x6EAFF000,
    0x6F561B10, 0x708FD200,
    0x7135FD10, 0x726FB400,
    0x7315DF10, 0x744F9600,
    0x74FEFB90, 0x7638B280,
    0x76DEDD90, 0x78189480,
    0x78BEBF90, 0x79F87680,
    0x7A9EA190, 0x7BD85880,
    0x7C7E8390, 0x7DB83A80,
    0x7E5E6590, 0x7F981C80
};

static ::testing::AssertionResult TmHasHourMinSec(struct tm *tm, int hour, int min, int sec) {
    if (tm->tm_hour == hour && tm->tm_min == min && tm->tm_sec == sec) {
        return ::testing::AssertionSuccess();
    }
    else {
        char buf[128];
        strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S", tm);
        return ::testing::AssertionFailure() << "Actual: " << buf << ", Expected: " << hour << ":" << (::testing::Message() << std::setfill('0') << std::setw(2) << min << ":" << std::setw(2) << sec);
        // (intermediate Message required because AssertionResult::operator<< will create a new Message for every invocation, losing the manipulators)
    }
}

TEST(cyMisc, ConvertGMTtoDni)
{
    for (int i = 0; i < sizeof(dsttransitions)/sizeof(dsttransitions[0]); i += 2) {
        time_t dni1, dni2;
        struct tm *tm;
        dni1 = cyMisc::ConvertGMTtoDni(dsttransitions[i] - 1);
        dni2 = cyMisc::ConvertGMTtoDni(dsttransitions[i]);
        tm = gmtime(&dni1);
        EXPECT_TRUE(TmHasHourMinSec(tm, 1, 59, 59)) << "(start before)";
        tm = gmtime(&dni2);
        EXPECT_TRUE(TmHasHourMinSec(tm, 3, 0, 0)) << "(start after)";

        dni1 = cyMisc::ConvertGMTtoDni(dsttransitions[i+1] - 1);
        dni2 = cyMisc::ConvertGMTtoDni(dsttransitions[i+1]);
        tm = gmtime(&dni1);
        EXPECT_TRUE(TmHasHourMinSec(tm, 1, 59, 59)) << "(end before)";
        tm = gmtime(&dni2);
        EXPECT_TRUE(TmHasHourMinSec(tm, 1, 0, 0)) << "(end after)";
    }
}
