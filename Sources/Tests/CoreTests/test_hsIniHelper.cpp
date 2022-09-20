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

#include "hsIniHelper.h"
#include <stdio.h>
#include <string_theory/string>


TEST(hsIniHelper, entry_comment)
{
    hsIniEntry entry("#This is a comment");
    EXPECT_EQ(entry.fType, hsIniEntry::Type::kComment);
    EXPECT_STREQ(entry.fComment.c_str(), "This is a comment");
    EXPECT_EQ(entry.fValues.size(), 0);
    EXPECT_STREQ(entry.fCommand.c_str(), "");
}

TEST(hsIniHelper, entry_blankLine)
{
    hsIniEntry entry("\n");
    EXPECT_EQ(entry.fType, hsIniEntry::Type::kBlankLine);
    EXPECT_STREQ(entry.fComment.c_str(), "");
    EXPECT_EQ(entry.fValues.size(), 0);
    EXPECT_STREQ(entry.fCommand.c_str(), "");
}

TEST(hsIniHelper, entry_command)
{
    hsIniEntry entry("Graphics.Height 1024");
    EXPECT_EQ(entry.fType, hsIniEntry::Type::kCommandValue);
    EXPECT_STREQ(entry.fComment.c_str(), "");
    EXPECT_EQ(entry.fValues.size(), 1);
    EXPECT_STREQ(entry.fValues[0].c_str(), "1024");
    EXPECT_STREQ(entry.fCommand.c_str(), "Graphics.Height");
}

TEST(hsIniHelper, entry_command_quoted)
{
    hsIniEntry entry("Graphics.Height \"1024 1024\"");
    EXPECT_EQ(entry.fType, hsIniEntry::Type::kCommandValue);
    EXPECT_STREQ(entry.fComment.c_str(), "");
    EXPECT_EQ(entry.fValues.size(), 1);
    EXPECT_STREQ(entry.fValues[0].c_str(), "1024 1024");
    EXPECT_STREQ(entry.fCommand.c_str(), "Graphics.Height");
}

TEST(hsIniHelper, entry_stream_parse)
{
    hsRAMStream s;
    std::string line = "Graphics.Height 1024\n";
    s.Write(line.length(), line.data());
    line = "Graphics.Width 768";
    s.Write(line.length(), line.data());
    s.Rewind();
    
    hsIniFile file(s);
    
    std::shared_ptr<hsIniEntry> heightEntry = file.findByCommand("Graphics.Height");
    EXPECT_NE(heightEntry, nullptr);
    EXPECT_EQ(heightEntry->fType, hsIniEntry::kCommandValue);
    EXPECT_EQ(heightEntry->fCommand, "Graphics.Height");
    EXPECT_EQ(heightEntry->fValues, std::vector<ST::string>({"1024"}));
    
    std::shared_ptr<hsIniEntry> widthEntry = file.findByCommand("Graphics.Width");
    EXPECT_NE(widthEntry, nullptr);
    EXPECT_EQ(widthEntry->fType, hsIniEntry::kCommandValue);
    EXPECT_EQ(widthEntry->fCommand, "Graphics.Width");
    EXPECT_EQ(widthEntry->fValues, std::vector<ST::string>({"768"}));
    
    std::shared_ptr<hsIniEntry> notAnEntry = file.findByCommand("NotACommand");
    EXPECT_EQ(notAnEntry, nullptr);
}

TEST(hsIniHelper, entry_stream_mutate)
{
    hsRAMStream s;
    std::string line = "Graphics.Height 1024\n";
    s.Write(line.length(), line.data());
    s.Rewind();
    
    hsIniFile file(s);
    
    std::shared_ptr<hsIniEntry> heightEntry = file.findByCommand("Graphics.Height");
    EXPECT_NE(heightEntry, nullptr);
    EXPECT_EQ(heightEntry->fType, hsIniEntry::kCommandValue);
    EXPECT_EQ(heightEntry->fCommand, "Graphics.Height");
    EXPECT_EQ(heightEntry->fValues, std::vector<ST::string>({"1024"}));
    
    heightEntry->setValue(0, "2048");
    
    heightEntry = file.findByCommand("Graphics.Height");
    EXPECT_EQ(heightEntry->fValues, std::vector<ST::string>({"2048"}));
}
