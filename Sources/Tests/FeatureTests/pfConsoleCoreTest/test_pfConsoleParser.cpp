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

#include "pfConsoleCore/pfConsoleCmd.h"
#include "pfConsoleCore/pfConsoleParser.h"

using namespace ST::literals;

// Define a few test groups and commands so that we don't have to pull in
// all the real console commands just to test the parsing.

PF_CONSOLE_BASE_CMD(TestBaseCmd, "", "")
{}

PF_CONSOLE_GROUP(TestGroup)

PF_CONSOLE_CMD(TestGroup, SubCmd, "", "")
{}

PF_CONSOLE_SUBGROUP(TestGroup, SubGroup)

PF_CONSOLE_CMD(TestGroup_SubGroup, SubSubCmd, "", "")
{}

// Top-level command outside of any group

TEST(pfConsoleParser, ParseBaseCommand)
{
    ST::string string = "TestBaseCmd"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, pfConsoleCmdGroup::GetBaseGroup());
    EXPECT_EQ(token, "TestBaseCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestBaseCmd);
}

TEST(pfConsoleParser, ParseBaseCommandArgs)
{
    ST::string string = "TestBaseCmd arg1 arg2"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, pfConsoleCmdGroup::GetBaseGroup());
    EXPECT_EQ(token, "TestBaseCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.begin() + sizeof("TestBaseCmd ") - 1);

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestBaseCmd);
}

// Top-level group

TEST(pfConsoleParser, ParseBaseGroup)
{
    ST::string string = "TestGroup"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup);
    EXPECT_FALSE(token);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, nullptr);
}

// Command inside top-level group

TEST(pfConsoleParser, ParseSubCommand)
{
    ST::string string = "TestGroup.SubCmd"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup);
    EXPECT_EQ(token, "SubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubCmd);
}

TEST(pfConsoleParser, ParseSubCommandArgs)
{
    ST::string string = "TestGroup.SubCmd arg1 arg2"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup);
    EXPECT_EQ(token, "SubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.begin() + sizeof("TestGroup.SubCmd ") - 1);

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubCmd);
}

TEST(pfConsoleParser, ParseSubCommandSpace)
{
    ST::string string = "TestGroup SubCmd"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup);
    EXPECT_EQ(token, "SubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubCmd);
}

TEST(pfConsoleParser, ParseSubCommandSpaceArgs)
{
    ST::string string = "TestGroup SubCmd arg1 arg2"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup);
    EXPECT_EQ(token, "SubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.begin() + sizeof("TestGroup SubCmd ") - 1);

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubCmd);
}

// Subgroup inside other group

TEST(pfConsoleParser, ParseSubGroup)
{
    ST::string string = "TestGroup.SubGroup"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_FALSE(token);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, nullptr);
}

TEST(pfConsoleParser, ParseSubGroupSpace)
{
    ST::string string = "TestGroup SubGroup"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_FALSE(token);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, nullptr);
}

// Command inside subgroup

TEST(pfConsoleParser, ParseSubSubCommand)
{
    ST::string string = "TestGroup.SubGroup.SubSubCmd"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_EQ(token, "SubSubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubGroup_SubSubCmd);
}

TEST(pfConsoleParser, ParseSubSubCommandArgs)
{
    ST::string string = "TestGroup.SubGroup.SubSubCmd arg1 arg2"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_EQ(token, "SubSubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.begin() + sizeof("TestGroup.SubGroup.SubSubCmd ") - 1);

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubGroup_SubSubCmd);
}

TEST(pfConsoleParser, ParseSubSubCommandSpaces)
{
    ST::string string = "TestGroup SubGroup SubSubCmd"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_EQ(token, "SubSubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.end());

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubGroup_SubSubCmd);
}

TEST(pfConsoleParser, ParseSubSubCommandSpacesArgs)
{
    ST::string string = "TestGroup SubGroup SubSubCmd arg1 arg2"_st;
    pfConsoleParser parser(string);

    auto [group, token] = parser.ParseGroupAndName();
    EXPECT_EQ(group, &conGroup_TestGroup_SubGroup);
    EXPECT_EQ(token, "SubSubCmd"_st);
    EXPECT_EQ(parser.fTokenizer.fPos, string.begin() + sizeof("TestGroup SubGroup SubSubCmd ") - 1);

    pfConsoleParser parser2(string);
    auto cmd = parser2.ParseCommand();
    EXPECT_EQ(cmd, &conCmd_TestGroup_SubGroup_SubSubCmd);
}
