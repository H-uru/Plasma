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
    ST::string string = "SampleCmd1"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "SampleCmd1"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextNamePart();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSingleWhitespace)
{
    ST::string string = "  SampleCmd1   "_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "SampleCmd1"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextNamePart();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

// Tokenize command name, 2 tokens

TEST(pfConsoleTokenizer, TokenizeCommandNameDot)
{
    ST::string string = "App.Quit"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("App.") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextNamePart();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeCommandNameUnderscore)
{
    ST::string string = "App_Quit"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("App_") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextNamePart();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSpace)
{
    ST::string string = "App  Quit"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "App"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("App  ") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Quit"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextNamePart();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

// Tokenize command name, 3 tokens

TEST(pfConsoleTokenizer, TokenizeCommandNameDots)
{
    ST::string string = "Graphics.Renderer.SetYon"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics.") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics.Renderer.") - 1);

    auto token3 = tokenizer.NextNamePart();
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextNamePart();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeCommandNameUnderscores)
{
    ST::string string = "Graphics_Renderer_SetYon"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics_") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics_Renderer_") - 1);

    auto token3 = tokenizer.NextNamePart();
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextNamePart();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeCommandNameSpaces)
{
    ST::string string = "Graphics Renderer   SetYon"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextNamePart();
    EXPECT_EQ(token1, "Graphics"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics ") - 1);

    auto token2 = tokenizer.NextNamePart();
    EXPECT_EQ(token2, "Renderer"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("Graphics Renderer   ") - 1);

    auto token3 = tokenizer.NextNamePart();
    EXPECT_EQ(token3, "SetYon"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextNamePart();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

// Tokenize arguments, 1 token

TEST(pfConsoleTokenizer, TokenizeArgumentsSingle)
{
    ST::string string = "arg"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextArgument();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleWhitespace)
{
    ST::string string = "  arg   "_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextArgument();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleUnderscore)
{
    ST::string string = "arg_test"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg_test"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextArgument();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleDoubleQuote)
{
    ST::string string = "\"(Default Device)\""_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "(Default Device)"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextArgument();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleSingleQuote)
{
    ST::string string = "'(Default Device)'"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "(Default Device)"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token2 = tokenizer.NextArgument();
    EXPECT_FALSE(token2);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleDoubleQuoteUnclosed)
{
    ST::string string = "\"(Default Device)"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_FALSE(token1);
    EXPECT_FALSE(tokenizer.fErrorMsg.empty());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsSingleSingleQuoteUnclosed)
{
    ST::string string = "'(Default Device)"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_FALSE(token1);
    EXPECT_FALSE(tokenizer.fErrorMsg.empty());
}

// Tokenize arguments, 2 tokens

TEST(pfConsoleTokenizer, TokenizeArgumentsPair)
{
    ST::string string = "arg1 arg2"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("arg1 ") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextArgument();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairWhitespace)
{
    ST::string string = " arg1  arg2   "_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof(" arg1  ") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextArgument();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairComma)
{
    ST::string string = "arg1, arg2"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "arg1"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("arg1,") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "arg2"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextArgument();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsPairMixedQuotes)
{
    ST::string string = "\"argument '1'\" 'argument \"2\"'"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "argument '1'"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("\"argument '1'\"") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "argument \"2\""_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token3 = tokenizer.NextArgument();
    EXPECT_FALSE(token3);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

// Tokenize arguments, 3 tokens

TEST(pfConsoleTokenizer, TokenizeArgumentsTriple)
{
    ST::string string = "1.2 3.4 5.6"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "1.2"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("1.2 ") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "3.4"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("1.2 3.4 ") - 1);

    auto token3 = tokenizer.NextArgument();
    EXPECT_EQ(token3, "5.6"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextArgument();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsTripleCommas)
{
    ST::string string = "1.2, 3.4, 5.6"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, "1.2"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("1.2,") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, "3.4"_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("1.2, 3.4,") - 1);

    auto token3 = tokenizer.NextArgument();
    EXPECT_EQ(token3, "5.6"_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextArgument();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}

TEST(pfConsoleTokenizer, TokenizeArgumentsTripleEmptyQuotes)
{
    ST::string string = "'' \"\" ''"_st;
    pfConsoleTokenizer tokenizer(string);

    auto token1 = tokenizer.NextArgument();
    EXPECT_EQ(token1, ""_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("''") - 1);

    auto token2 = tokenizer.NextArgument();
    EXPECT_EQ(token2, ""_st);
    EXPECT_EQ(tokenizer.fPos, string.begin() + sizeof("'' \"\"") - 1);

    auto token3 = tokenizer.NextArgument();
    EXPECT_EQ(token3, ""_st);
    EXPECT_EQ(tokenizer.fPos, string.end());

    auto token4 = tokenizer.NextArgument();
    EXPECT_FALSE(token4);
    EXPECT_TRUE(tokenizer.fErrorMsg.empty());
    EXPECT_EQ(tokenizer.fPos, string.end());
}
