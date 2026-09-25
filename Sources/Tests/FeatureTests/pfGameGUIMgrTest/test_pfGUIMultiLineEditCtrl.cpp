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
#include <pfGameGUIMgr/pfGUIMultiLineEditCtrl.h>
#include <string_theory/string>

#include "pfAllCreatables.h"
#include "plAllCreatables.h"
#include "pnAllCreatables.h"

// Macro helpers that will compile down to constants when used with hardcoded strings.
#define STRLEN(x) std::char_traits<char>::length(x)
#define WSTRLEN(x) std::char_traits<wchar_t>::length(x)

TEST(pfGUIMultiLineEditCtrl, BasicWordNavigation)
{
    pfGUIMultiLineEditCtrl textBox;
    ST::string testStr = "Hi, this is a test. Woop!";
    textBox.InsertString(testStr);
    ASSERT_EQ(textBox.GetCursor(), testStr.size());
    // No change when moving right at end.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), testStr.size());
    // Expected left word movement in pure text.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is a test. "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is a "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), 0);
    // No weirdness when moving left at start.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    ASSERT_EQ(textBox.GetCursor(), 0);
    // Expected right movement in pure text.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is a "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), STRLEN("Hi, this is a test. "));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    ASSERT_EQ(textBox.GetCursor(), testStr.size());
}

TEST(pfGUIMultiLineEditCtrl, BasicWordBackwardDeletion)
{
    pfGUIMultiLineEditCtrl textBox;
    ST::string testStr = "Hi, this is a test!";
    textBox.InsertString(testStr);
    ASSERT_EQ(textBox.GetCursor(), testStr.size());
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("Hi, this is a "));
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("Hi, this is "));
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("Hi, this "));
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("Hi, "));
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL(""));
    // No breakage when deleting at start.
    textBox.DeleteWord(false);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL(""));
}

TEST(pfGUIMultiLineEditCtrl, BasicWordForwardDeletion)
{
    pfGUIMultiLineEditCtrl textBox;
    textBox.InsertString(ST_LITERAL("Hi, this is a test!"));
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kBufferStart);
    ASSERT_EQ(textBox.GetCursor(), 0);
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("this is a test!"));
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("is a test!"));
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("a test!"));
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("test!"));
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL(""));
    // No breakage when deleting at start.
    textBox.DeleteWord(true);
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL(""));
    ASSERT_EQ(textBox.GetCursor(), 0);
}

TEST(pfGUIMultiLineEditCtrl, WordBreakerBehavior) {
    pfGUIMultiLineEditCtrl textBox;
    // Test string that includes a word breaker block and an arbitrary code with a 0 param.
    textBox.InsertString(ST_LITERAL("random, ; ,.   \x02\x00\x02words "));

    // Moving a word back will move to the end of the word breaker block before the previous word.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    int32_t cursor = textBox.GetCursor();
    auto buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L'\x02');

    // Moving a word forward will move past an entire word breaker block after the next word.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kBufferStart);
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L'\x02');

    textBox.DeleteWord(false);
    textBox.DeleteChar();
    ASSERT_EQ(textBox.GetCodedBuffer(), ST_WCHAR_LITERAL("words "));
}

TEST(pfGUIMultiLineEditCtrl, AdvancedMovementTest)
{
    // Welcome to me literally just slapping other random use cases that come to mind together into one.
    pfGUIMultiLineEditCtrl textBox;
    textBox.InsertString(ST_LITERAL("Lorem ipsum! "));
    hsColorRGBA red;
    red.Set(1.f, 0.f, 0.f, 1.f);
    textBox.InsertColor(red);
    textBox.InsertString(ST_LITERAL("Words of wisdom, for ages untold, outside the bounds of reality and reason, in very long lines that are sure to wrap into more lines, have been spoken.\n"));
    // Insert a manual color code with space key code as color values.
    textBox.InsertString(ST_LITERAL("\x01   \x01Until now!\n"));
    textBox.InsertLink(0);
    textBox.InsertString(ST_LITERAL("https://www.mystonline.com"));
    textBox.InsertLink(-1);
    textBox.InsertString(ST_LITERAL("\nTHE wordwith"));
    textBox.InsertStyle(1);
    textBox.InsertString(ST_LITERAL("stylecode"));
    textBox.InsertStyle(0);
    textBox.InsertString(ST_LITERAL("inside END"));

    // Move two words back and delete the word with a style code inside.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    textBox.DeleteWord(true);
    ASSERT_STREQ(&textBox.GetCodedBuffer()[textBox.GetCursor()], L"END");

    // Moving another word back and deleting a char should move us right to the end of the link and in front of T.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordBack);
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneBack);
    textBox.DeleteChar();
    int32_t cursor = textBox.GetCursor();
    auto buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L'\x03');
    ASSERT_STREQ(&buffer[cursor], L"THE END");

    textBox.DeleteWord(false); // Delete link up to com since dots are word breakers.
    textBox.DeleteWord(false); // Delete link up to mystonline since dots are word breakers.
    textBox.DeleteWord(false); // Delete again which should leave us at right after the "Until now!" newline.
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L'\n');
    ASSERT_EQ(buffer[cursor], L'T');

    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kBufferStart);
    textBox.DeleteWord(true); // Delete Lorem
    textBox.DeleteWord(true); // Delete ipsum!
    // We should now be in front of the red color code.
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor], L'\x01');
    ASSERT_EQ(buffer[cursor + 1], L'\xFF');

    // Moving a word right will now move past the color code and the word.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L'o');
    ASSERT_EQ(buffer[cursor + 1], L'f');

    // Delete "for ages untold"
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    textBox.DeleteWord(true);
    textBox.DeleteWord(true);
    textBox.DeleteWord(true);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 3], L'm');
    ASSERT_EQ(buffer[cursor - 2], L',');
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L'o');
    ASSERT_EQ(buffer[cursor + 1], L'u');
    ASSERT_EQ(buffer[cursor + 2], L't');

    // Moving 21 words right now should put us in front of "spoken."
    for (size_t i = 0; i < 21; i++)
        textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L's');
    ASSERT_EQ(buffer[cursor + 1], L'p');
    ASSERT_EQ(buffer[cursor + 2], L'o');

    // Now, another move right will go in front of the manual color code with "space characters" inside it.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L'\n');
    ASSERT_EQ(buffer[cursor], L'\x01');
    
    // And the next will move past the entire code sequence and word.
    textBox.MoveCursor(pfGUIMultiLineEditCtrl::kOneWordForward);
    cursor = textBox.GetCursor();
    buffer = textBox.GetCodedBuffer();
    ASSERT_EQ(buffer[cursor - 1], L' ');
    ASSERT_EQ(buffer[cursor], L'n');
    ASSERT_EQ(buffer[cursor + 1], L'o');
    ASSERT_EQ(buffer[cursor + 2], L'w');
}
