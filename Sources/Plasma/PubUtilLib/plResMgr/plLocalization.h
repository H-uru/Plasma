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
#ifndef plLocalization_h_inc
#define plLocalization_h_inc

#include <set>
#include <vector>

class plFileName;
namespace ST { class string; }

class plLocalization
{
public:
    enum Language
    {
        kEnglish,
        kFrench,
        kGerman,
        kSpanish,
        kItalian,
        kJapanese,

        kNumLanguages,
    };

protected:
    static Language fLanguage;
    static const ST::string fLangTags[kNumLanguages];
    static std::set<ST::string> fLangCodes[kNumLanguages];
    static const ST::string fLangNames[kNumLanguages];

    static plFileName IGetLocalized(const plFileName& name, Language lang);

public:
    static void SetLanguage(Language lang) { fLanguage = lang; }
    static Language GetLanguage() { return fLanguage; }

    static ST::string GetLanguageName(Language lang);
    static std::set<ST::string> GetLanguageCodes(Language lang);

    // Returns true if we're using localized assets.  If it returns false, you
    // don't need to bother calling GetLocalized
    static bool IsLocalized() { return fLanguage != kEnglish; }

    // Pass in a key name and this will give you the localized name
    // Returns an invalid filename if the original keyname is not for a localized asset
    static plFileName GetLocalized(const plFileName& name);

    //
    // Export only
    //
    // When you're exporting an asset that could be localized, you'll want to do
    // a loop something like this to try and find any localized versions.
    //
    // for (int i = 0; i < plLocalization::GetNumLocales(); i++)
    // {
    //     char localName[MAX_PATH];
    //     if (plLocalization::ExportGetLocalized(fileName, i, localName))
    //     {
    //         ...
    //     }
    // }
    //
    static int GetNumLocales() { return kNumLanguages - 1; }
    static plFileName ExportGetLocalized(const plFileName& name, int lang);

    // Converts a vector of translated strings to a encoded string that can be decoded by StringToLocal()
    // The index in the vector of a string is it's language
    static ST::string LocalToString(const std::vector<ST::string>& localizedText);
    // Converts a string encoded by LocalToString to a vector of translated strings
    static std::vector<ST::string> StringToLocal(const ST::string& localizedText);
};

#endif // plLocalization_h_inc
