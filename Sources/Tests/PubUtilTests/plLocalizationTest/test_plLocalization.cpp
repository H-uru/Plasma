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

#include <gtest/gtest.h>
#include <string_theory/string>
#include <vector>

#include "plResMgr/plLocalization.h"

TEST(plLocalization, LocalToString)
{
    std::vector<ST::string> translations {ST_LITERAL("English")};
    EXPECT_EQ(ST_LITERAL("$En$English"), plLocalization::LocalToString(translations));

    translations.emplace_back(ST_LITERAL("French"));
    EXPECT_EQ(ST_LITERAL("$En$English$Fr$French"), plLocalization::LocalToString(translations));

    translations.emplace_back(ST_LITERAL("German"));
    translations.emplace_back(ST_LITERAL("Spanish"));
    translations.emplace_back(ST_LITERAL("Italian"));
    translations.emplace_back(ST_LITERAL("Japanese"));
    EXPECT_EQ(ST_LITERAL("$En$English$Fr$French$Ge$German$Sp$Spanish$It$Italian$Ja$Japanese"), plLocalization::LocalToString(translations));
}

TEST(plLocalization, StringToLocal)
{
    std::vector<ST::string> translations = plLocalization::StringToLocal(ST_LITERAL("$En$English"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kJapanese]);

    translations = plLocalization::StringToLocal(ST_LITERAL("$En$English$Fr$French"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST_LITERAL("French"), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kJapanese]);

    translations = plLocalization::StringToLocal(ST_LITERAL("$Fr$French$En$English"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST_LITERAL("French"), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kJapanese]);

    translations = plLocalization::StringToLocal(ST_LITERAL("$En$English$Fr$French$Sp$Spanish"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST_LITERAL("French"), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST_LITERAL("Spanish"), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kJapanese]);

    translations = plLocalization::StringToLocal(ST_LITERAL("$Sp$Spanish$Fr$French$En$English"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST_LITERAL("French"), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST_LITERAL("Spanish"), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST::string(), translations[plLocalization::kJapanese]);

    translations = plLocalization::StringToLocal(ST_LITERAL("$En$English$Fr$French$Ge$German$Sp$Spanish$It$Italian$Ja$Japanese"));
    EXPECT_EQ(plLocalization::kNumLanguages, translations.size());
    EXPECT_EQ(ST_LITERAL("English"), translations[plLocalization::kEnglish]);
    EXPECT_EQ(ST_LITERAL("French"), translations[plLocalization::kFrench]);
    EXPECT_EQ(ST_LITERAL("German"), translations[plLocalization::kGerman]);
    EXPECT_EQ(ST_LITERAL("Spanish"), translations[plLocalization::kSpanish]);
    EXPECT_EQ(ST_LITERAL("Italian"), translations[plLocalization::kItalian]);
    EXPECT_EQ(ST_LITERAL("Japanese"), translations[plLocalization::kJapanese]);
}
