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

#include "hsDarwin.h"
#import <Foundation/Foundation.h>

TEST(hsDarwin_Foundation, converts_to_ST_string)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSString* str = @"Test 123";
    ST::string st = STStringFromNSString(str);
    EXPECT_STREQ(st.c_str(), "Test 123");

    [pool drain];
}

TEST(hsDarwin_Foundation, converts_to_NSString)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    ST::string st = ST_LITERAL("Test 123");
    NSString* nstr = NSStringCreateWithSTString(st);
    EXPECT_EQ(YES, [nstr isEqualToString:@"Test 123"]);

    [pool drain];
}

TEST(hsDarwin_Foundation, converts_to_ST_string_UTF8)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    NSString* str = [NSString stringWithUTF8String:"Test ö"];
    ST::string st = STStringFromNSString(str);
    EXPECT_STREQ(st.c_str(), "Test \xc3\xb6");

    [pool drain];
}

TEST(hsDarwin_Foundation, converts_to_NSString_UTF8)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    ST::string st = ST_LITERAL("Test ö");
    NSString* nstr = NSStringCreateWithSTString(st);
    EXPECT_EQ(YES, [nstr isEqualToString:[NSString stringWithUTF8String:"Test ö"]]);

    [pool drain];
}

TEST(hsDarwin_Foundation, returns_retained_NSString)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

    ST::string st = ST::format("{} Test {}", 12345, "Hello");
    NSString* nstr = NSStringCreateWithSTString(st);
    EXPECT_EQ(1, [nstr retainCount]);

    [pool drain];
}
