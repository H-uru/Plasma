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

#include "HeadSpin.h"
#include "plCmdParser.h"

TEST(plCmdParser, basic_parsing)
{
    const plCmdArgDef cmds[] = {
        { kCmdArgRequired | kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    bool success = parser.Parse("plCmdParser ~/.plasma/config.dat");

    ST::string prog = parser.GetProgramName();
    ST::string path = parser.GetString(0);

    EXPECT_EQ(success, true);
    EXPECT_STREQ(prog.c_str(), "plCmdParser");
    EXPECT_STREQ(path.c_str(), "~/.plasma/config.dat");
    EXPECT_EQ(parser.GetError(), kCmdErrorSuccess);
}

TEST(plCmdParser, argv_parsing)
{
    const plCmdArgDef cmds[] = {
        { kCmdArgRequired | kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));

    const char* args[] = {"plCmdParser", "~/.plasma/config.dat"};
    int argc = 2;

    std::vector<ST::string> tokens(args, args+argc);

    bool success = parser.Parse(tokens);

    ST::string prog = parser.GetProgramName();
    ST::string path = parser.GetString(0);

    EXPECT_EQ(success, true);
    EXPECT_STREQ(prog.c_str(), "plCmdParser");
    EXPECT_STREQ(path.c_str(), "~/.plasma/config.dat");
    EXPECT_EQ(parser.GetError(), kCmdErrorSuccess);
}

TEST(plCmdParser, argv_preserving_spaces)
{
    const plCmdArgDef cmds[] = {
        { kCmdArgRequired | kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));

    const char* args[] = {"plCmdParser", "~/.plasma/Uru Live/config.dat"};
    int argc = 2;

    std::vector<ST::string> tokens(args, args+argc);

    bool success = parser.Parse(tokens);

    ST::string prog = parser.GetProgramName();
    ST::string path = parser.GetString(0);

    EXPECT_EQ(success, true);
    EXPECT_STREQ(prog.c_str(), "plCmdParser");
    EXPECT_STREQ(path.c_str(), "~/.plasma/Uru Live/config.dat");
    EXPECT_EQ(parser.GetError(), kCmdErrorSuccess);
}

TEST(plCmdParser, wchar_argv_parsing)
{
    const plCmdArgDef cmds[] = {
        { kCmdArgRequired | kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));

    const wchar_t* args[] = {L"plCmdParser", L"~/.plasma/config.dat"};
    int argc = 2;

    std::vector<ST::string> tokens(argc);
    for (int i = 0; i < argc; i++) {
        tokens.push_back(ST::string::from_wchar(args[i]));
    }

    bool success = parser.Parse(tokens);

    ST::string prog = parser.GetProgramName();
    ST::string path = parser.GetString(0);

    EXPECT_EQ(success, true);
    EXPECT_STREQ(prog.c_str(), "plCmdParser");
    EXPECT_STREQ(path.c_str(), "~/.plasma/config.dat");
    EXPECT_EQ(parser.GetError(), kCmdErrorSuccess);
}


TEST(plCmdParser, flagged_int)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size 5");

    int32_t size = parser.GetInt(0);

    EXPECT_EQ(size, 5);
}

TEST(plCmdParser, flagged_int_short)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser -s 5");

    int32_t size = parser.GetInt(0);

    EXPECT_EQ(size, 5);
}

TEST(plCmdParser, flagged_int_assign)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size=5");

    int32_t size = parser.GetInt(0);

    EXPECT_EQ(size, 5);
}

TEST(plCmdParser, flagged_int_slash)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser /size -5");

    int32_t size = parser.GetInt(0);

    EXPECT_EQ(size, -5);
}

TEST(plCmdParser, flagged_int_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size 5");

    int32_t size = parser.GetInt("size");

    EXPECT_EQ(size, 5);
}


TEST(plCmdParser, flagged_uint)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeUint, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size 5");

    uint32_t size = parser.GetUint(0);

    EXPECT_EQ(size, 5);
}

TEST(plCmdParser, flagged_uint_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeUint, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size 5");

    uint32_t size = parser.GetUint("size");

    EXPECT_EQ(size, 5);
}


TEST(plCmdParser, flagged_float)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeFloat, "volume", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --volume 0.5");

    float vol = parser.GetFloat(0);

    EXPECT_EQ(vol, 0.5);
}

TEST(plCmdParser, flagged_float_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeFloat, "volume", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --volume 0.5");

    float vol = parser.GetFloat("volume");

    EXPECT_EQ(vol, 0.5);
}


TEST(plCmdParser, flagged_string)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --path foo");

    ST::string path = parser.GetString(0);

    EXPECT_STREQ(path.c_str(), "foo");
}

TEST(plCmdParser, flagged_string_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --path foo");

    ST::string path = parser.GetString("path");

    EXPECT_STREQ(path.c_str(), "foo");
}


TEST(plCmdParser, flagged_bool_default)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeBool, "verbose", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --verbose");

    bool verbose = parser.GetBool(0);

    EXPECT_EQ(verbose, true);
}

TEST(plCmdParser, flagged_bool_true)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeBool, "verbose", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --verbose=TRUE");

    bool verbose = parser.GetBool(0);

    EXPECT_EQ(verbose, true);
}

TEST(plCmdParser, flagged_bool_false)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeBool, "verbose", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --verbose=FALSE");

    bool verbose = parser.GetBool(0);

    EXPECT_EQ(verbose, false);
}

TEST(plCmdParser, flagged_bool_invalid)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeBool, "verbose", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --verbose=foo");

    bool verbose = parser.GetBool(0);

    EXPECT_EQ(verbose, false);
    EXPECT_EQ(parser.GetError(), kCmdErrorInvalidValue);
}

TEST(plCmdParser, flagged_bool_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeBool, "verbose", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --verbose");

    bool verbose = parser.GetBool("verbose");

    EXPECT_EQ(verbose, true);
}


TEST(plCmdParser, optional_unspecified)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdArgOptional, "speed", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser");

    bool specified = parser.IsSpecified(0);
    int32_t speed = parser.GetInt(0);

    EXPECT_EQ(specified, false);
    EXPECT_EQ(speed, 0);
}

TEST(plCmdParser, optional_specified)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdArgOptional, "speed", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser 1");

    bool specified = parser.IsSpecified(0);
    int32_t speed = parser.GetInt(0);

    EXPECT_EQ(specified, true);
    EXPECT_EQ(speed, 1);
}

TEST(plCmdParser, optional_specified_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdArgOptional, "speed", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser 1");

    bool specified = parser.IsSpecified("speed");
    int32_t speed = parser.GetInt("speed");

    EXPECT_EQ(specified, true);
    EXPECT_EQ(speed, 1);
}


TEST(plCmdParser, specified_invalid)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdArgOptional, "speed", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser 1");

    bool specified = parser.IsSpecified(1);

    EXPECT_EQ(specified, false);
}

TEST(plCmdParser, specified_invalid_bystring)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdArgOptional, "speed", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser 1");

    bool specified = parser.IsSpecified("path");

    EXPECT_EQ(specified, false);
}


TEST(plCmdParser, flagged_weird_id)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 10}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size 5");

    int32_t size = parser.GetInt(10);

    EXPECT_EQ(size, 5);
}


TEST(plCmdParser, fake_flag)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    bool success = parser.Parse("plCmdParser --speed 5");

    EXPECT_EQ(success, false);
    EXPECT_EQ(parser.GetError(), kCmdErrorInvalidArg);
}


TEST(plCmdParser, too_many_args)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    bool success = parser.Parse("plCmdParser --size 10 foo");

    EXPECT_EQ(success, false);
    EXPECT_EQ(parser.GetError(), kCmdErrorTooManyArgs);
}


TEST(plCmdParser, missing_required)
{
    const plCmdArgDef cmds[] = {
        { kCmdArgRequired | kCmdTypeString, "path", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    bool success = parser.Parse("plCmdParser");

    EXPECT_EQ(success, false);
    EXPECT_EQ(parser.GetError(), kCmdErrorTooFewArgs);
}


TEST(plCmdParser, combined_assign)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt, "size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --size5");

    int32_t size = parser.GetInt(0);

    EXPECT_EQ(size, 5);
}


TEST(plCmdParser, case_sensitive_nomatch)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdCaseSensitive, "Size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser -s 5");

    bool specified = parser.IsSpecified(0);

    EXPECT_EQ(specified, false);
}

TEST(plCmdParser, case_sensitive_match)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeInt | kCmdCaseSensitive, "Size", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser -S 5");

    bool specified = parser.IsSpecified(0);

    EXPECT_EQ(specified, true);
}

TEST(plCmdParser, separator_in_value_nix)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeString, "color", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser --color=reddish-blue");

    ST::string value = parser.GetString("color");

    EXPECT_EQ(value, "reddish-blue");
}

TEST(plCmdParser, separator_in_value_windows)
{
    const plCmdArgDef cmds[] = {
        { kCmdTypeString, "color", 0}
    };

    plCmdParser parser(cmds, std::size(cmds));
    parser.Parse("plCmdParser /color=reddish-blue");

    ST::string value = parser.GetString("color");

    EXPECT_EQ(value, "reddish-blue");
}
