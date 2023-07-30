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

#include "pfConsoleCore/pfConsoleParser.h"

using namespace ST::literals;

// Tokenize command name, 1 token

TEST(pfConsoleTokenizer, TokenizeCommandNameSingle)
{
    const char buf[] = "SampleCmd1";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "SampleCmd1"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSingleWhitespace)
{
    const char buf[] = "  SampleCmd1   ";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "SampleCmd1"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize command name, 2 tokens

TEST(pfConsoleTokenizer, TokenizeCommandNameDot)
{
    const char buf[] = "App.Quit";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(line, buf + sizeof("App.") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeCommandNameUnderscore)
{
    const char buf[] = "App_Quit";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(line, buf + sizeof("App_") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSpace)
{
    const char buf[] = "App  Quit";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(line, buf + sizeof("App  ") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize command name, 3 tokens

TEST(pfConsoleTokenizer, TokenizeCommandNameDots)
{
    const char buf[] = "Graphics.Renderer.SetYon";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics.") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics.Renderer.") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeCommandNameUnderscores)
{
    const char buf[] = "Graphics_Renderer_SetYon";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics_") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics_Renderer_") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSpaces)
{
    const char buf[] = "Graphics Renderer   SetYon";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics ") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(line, buf + sizeof("Graphics Renderer   ") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeCommandName(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize arguments, 1 token

TEST(pfConsoleTokenizer, TokenizeArgumentsSingle)
{
    const char buf[] = "arg";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleWhitespace)
{
    const char buf[] = "  arg   ";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleUnderscore)
{
    const char buf[] = "arg_test";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg_test"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleDoubleQuote)
{
    const char buf[] = "\"(Default Device)\"";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "(Default Device)"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleSingleQuote)
{
    const char buf[] = "'(Default Device)'";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "(Default Device)"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token2);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleDoubleQuoteUnclosed)
{
    const char buf[] = "\"(Default Device)";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "\xff"_st);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleSingleQuoteUnclosed)
{
    const char buf[] = "'(Default Device)";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "\xff"_st);
}

// Tokenize arguments, 2 tokens

TEST(pfConsoleTokenizer, TokenizeArgumentsPair)
{
    const char buf[] = "arg1 arg2";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(line, buf + sizeof("arg1 ") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairWhitespace)
{
    const char buf[] = " arg1  arg2   ";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(line, buf + sizeof(" arg1  ") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairComma)
{
    const char buf[] = "arg1, arg2";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(line, buf + sizeof("arg1,") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairMixedQuotes)
{
    const char buf[] = "\"argument '1'\" 'argument \"2\"'";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "argument '1'"_st);
    EXPECT_EQ(line, buf + sizeof("\"argument '1'\"") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "argument \"2\""_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token3);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize arguments, 3 tokens

TEST(pfConsoleTokenizer, TokenizeArgumentsTriple)
{
    const char buf[] = "1.2 3.4 5.6";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "1.2"_st);
    EXPECT_EQ(line, buf + sizeof("1.2 ") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "3.4"_st);
    EXPECT_EQ(line, buf + sizeof("1.2 3.4 ") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token3, "5.6"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsTripleCommas)
{
    const char buf[] = "1.2, 3.4, 5.6";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, "1.2"_st);
    EXPECT_EQ(line, buf + sizeof("1.2,") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, "3.4"_st);
    EXPECT_EQ(line, buf + sizeof("1.2, 3.4,") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token3, "5.6"_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleTokenizer, TokenizeArgumentsTripleEmptyQuotes)
{
    const char buf[] = "'' \"\" ''";
    const char* line = buf;

    auto token1 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token1, ""_st);
    EXPECT_EQ(line, buf + sizeof("''") - 1);

    auto token2 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token2, ""_st);
    EXPECT_EQ(line, buf + sizeof("'' \"\"") - 1);

    auto token3 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_EQ(token3, ""_st);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    auto token4 = pfConsoleTokenizer::TokenizeArguments(line);
    EXPECT_FALSE(token4);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}
