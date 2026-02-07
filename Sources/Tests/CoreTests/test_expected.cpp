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

#include "hsExpected.h"

#include <gtest/gtest.h>
#include <string_theory/string>

static hsExpected<int, ST::string> ITestTrivialExNontrivialUnex(bool pass)
{
    if (pass)
        return 0;
    return hsUnexpected(ST_LITERAL("meow"));
}

static hsExpected<ST::string, int> ITestNontrivialExTrivialUnex(bool pass)
{
    if (pass)
        return ST_LITERAL("meow");
    return hsUnexpected(-1);
}

static hsExpected<int, int> ITestTrivial(bool pass)
{
    if (pass)
        return 0;
    return hsUnexpected(-1);
}

static hsExpected<ST::string, ST::string> ITestNontrivial(bool pass)
{
    if (pass)
        return ST_LITERAL("pass");
    return hsUnexpected<ST::string>("fail");
}

static hsExpected<std::unique_ptr<int>, int> ITestMoveOnlyExTrivialUnex(bool pass)
{
    if (pass)
        return std::make_unique<int>(0);
    return hsUnexpected(-1);
}

static hsExpected<int, std::unique_ptr<int>> ITestTrivialExMoveOnlyUnex(bool pass)
{
    if (pass)
        return 0;
    return hsUnexpected(std::make_unique<int>(-1));
}

TEST(expected, fail)
{
    EXPECT_FALSE(ITestTrivialExNontrivialUnex(false));
    EXPECT_STREQ(ITestNontrivial(false).Error().c_str(), "fail");
    EXPECT_THROW((void)ITestTrivial(false).Value(), hsBadExpectedAccess<int>);
    EXPECT_EQ(ITestTrivial(false).Error(), -1);

    // Accessing move-only contents in a mutable hsExpected:
    EXPECT_EQ(ITestMoveOnlyExTrivialUnex(false).Error(), -1);
    EXPECT_THROW((void)ITestMoveOnlyExTrivialUnex(false).Value(), hsBadExpectedAccess<int>);
    EXPECT_EQ(*ITestTrivialExMoveOnlyUnex(false).Error(), -1);
    EXPECT_THROW((void)ITestTrivialExMoveOnlyUnex(false).Value(), hsBadExpectedAccess<std::unique_ptr<int>>);

    // Accessing move-only contents in a const hsExpected:
    const auto constRes1 = ITestMoveOnlyExTrivialUnex(false);
    EXPECT_EQ(constRes1.Error(), -1);
    EXPECT_THROW((void)constRes1.Value(), hsBadExpectedAccess<hsMonostate>);
    const auto constRes2 = ITestTrivialExMoveOnlyUnex(false);
    EXPECT_EQ(*constRes2.Error(), -1);
    EXPECT_THROW((void)constRes2.Value(), hsBadExpectedAccess<hsMonostate>);
}

TEST(expected, success)
{
    EXPECT_TRUE(ITestTrivial(true));
    EXPECT_EQ(*ITestTrivial(true), 0);
    EXPECT_STREQ(ITestNontrivialExTrivialUnex(true)->c_str(), "meow");
    EXPECT_EQ(ITestTrivialExNontrivialUnex(true).Value(), 0);

    // Accessing move-only contents in a mutable hsExpected:
    EXPECT_EQ(**ITestMoveOnlyExTrivialUnex(true), 0);
    EXPECT_THROW((void)ITestMoveOnlyExTrivialUnex(true).Error(), hsBadExpectedAccess<std::unique_ptr<int>>);
    EXPECT_EQ(*ITestTrivialExMoveOnlyUnex(true), 0);
    EXPECT_THROW((void)ITestTrivialExMoveOnlyUnex(true).Error(), hsBadExpectedAccess<int>);

    // Accessing move-only contents in a const hsExpected:
    const auto constRes1 = ITestMoveOnlyExTrivialUnex(true);
    EXPECT_EQ(**constRes1, 0);
    EXPECT_THROW((void)constRes1.Error(), hsBadExpectedAccess<hsMonostate>);
    const auto constRes2 = ITestTrivialExMoveOnlyUnex(true);
    EXPECT_EQ(*constRes2, 0);
    EXPECT_THROW((void)constRes2.Error(), hsBadExpectedAccess<hsMonostate>);
}
