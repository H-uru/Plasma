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

#include "pfConsoleCore/pfConsoleEngine.h"

// Tokenize command name, 1 token

TEST(pfConsoleCore, TokenizeCommandNameSingle)
{
    char buf[] = "SampleCmd1";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "SampleCmd1");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeCommandNameSingleWhitespace)
{
    char buf[] = "  SampleCmd1   ";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "SampleCmd1");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize command name, 2 tokens

TEST(pfConsoleCore, TokenizeCommandNameDot)
{
    char buf[] = "App.Quit";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "App");
    EXPECT_EQ(line, buf + sizeof("App.") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Quit");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeCommandNameUnderscore)
{
    char buf[] = "App_Quit";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "App");
    EXPECT_EQ(line, buf + sizeof("App_") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Quit");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeCommandNameSpace)
{
    char buf[] = "App  Quit";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "App");
    EXPECT_EQ(line, buf + sizeof("App  ") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Quit");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize command name, 3 tokens

TEST(pfConsoleCore, TokenizeCommandNameDots)
{
    char buf[] = "Graphics.Renderer.SetYon";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "Graphics");
    EXPECT_EQ(line, buf + sizeof("Graphics.") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Renderer");
    EXPECT_EQ(line, buf + sizeof("Graphics.Renderer.") - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token3, "SetYon");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token4 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token4, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeCommandNameUnderscores)
{
    char buf[] = "Graphics_Renderer_SetYon";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "Graphics");
    EXPECT_EQ(line, buf + sizeof("Graphics_") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Renderer");
    EXPECT_EQ(line, buf + sizeof("Graphics_Renderer_") - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token3, "SetYon");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token4 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token4, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeCommandNameSpaces)
{
    char buf[] = "Graphics Renderer   SetYon";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token1, "Graphics");
    EXPECT_EQ(line, buf + sizeof("Graphics ") - 1);

    const char* token2 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token2, "Renderer");
    EXPECT_EQ(line, buf + sizeof("Graphics Renderer   ") - 1);

    const char* token3 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_STREQ(token3, "SetYon");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token4 = pfConsoleEngine::TokenizeCommandName(line);
    EXPECT_EQ(token4, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize arguments, 1 token

TEST(pfConsoleCore, TokenizeArgumentsSingle)
{
    char buf[] = "arg";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsSingleWhitespace)
{
    char buf[] = "  arg   ";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsSingleUnderscore)
{
    char buf[] = "arg_test";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg_test");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsSingleDoubleQuote)
{
    char buf[] = "\"(Default Device)\"";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "(Default Device)");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsSingleSingleQuote)
{
    char buf[] = "'(Default Device)'";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "(Default Device)");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token2, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsSingleDoubleQuoteUnclosed)
{
    char buf[] = "\"(Default Device)";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "\xff");
}

TEST(pfConsoleCore, TokenizeArgumentsSingleSingleQuoteUnclosed)
{
    char buf[] = "'(Default Device)";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "\xff");
}

// Tokenize arguments, 2 tokens

TEST(pfConsoleCore, TokenizeArgumentsPair)
{
    char buf[] = "arg1 arg2";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg1");
    EXPECT_EQ(line, buf + sizeof("arg1 ") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "arg2");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsPairWhitespace)
{
    char buf[] = " arg1  arg2   ";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg1");
    EXPECT_EQ(line, buf + sizeof(" arg1  ") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "arg2");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsPairComma)
{
    char buf[] = "arg1, arg2";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "arg1");
    EXPECT_EQ(line, buf + sizeof("arg1,") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "arg2");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsPairMixedQuotes)
{
    char buf[] = "\"argument '1'\" 'argument \"2\"'";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "argument '1'");
    EXPECT_EQ(line, buf + sizeof("\"argument '1'\"") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "argument \"2\"");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token3, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

// Tokenize arguments, 3 tokens

TEST(pfConsoleCore, TokenizeArgumentsTriple)
{
    char buf[] = "1.2 3.4 5.6";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "1.2");
    EXPECT_EQ(line, buf + sizeof("1.2 ") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "3.4");
    EXPECT_EQ(line, buf + sizeof("1.2 3.4 ") - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token3, "5.6");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token4 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token4, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}

TEST(pfConsoleCore, TokenizeArgumentsTripleCommas)
{
    char buf[] = "1.2, 3.4, 5.6";
    char* line = buf;

    const char* token1 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token1, "1.2");
    EXPECT_EQ(line, buf + sizeof("1.2,") - 1);

    const char* token2 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token2, "3.4");
    EXPECT_EQ(line, buf + sizeof("1.2, 3.4,") - 1);

    const char* token3 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_STREQ(token3, "5.6");
    EXPECT_EQ(line, buf + sizeof(buf) - 1);

    const char* token4 = pfConsoleEngine::TokenizeArguments(line);
    EXPECT_EQ(token4, nullptr);
    EXPECT_EQ(line, buf + sizeof(buf) - 1);
}
