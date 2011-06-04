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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#ifndef plLocalization_h_inc
#define plLocalization_h_inc

#include "hsStlUtils.h"

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
	
	typedef enum encodingTypes
	{
		Enc_Unencoded,	// This can also mean that python did the decoding for us and we don't need to tweak it on our end
		Enc_Split_String,
		Enc_Hybrid_Split_String,
		Enc_UTF8,
		Enc_UTF16,
		Enc_Unicode_Escape,
		Enc_Raw_Unicode_Escape,
		Enc_Latin_1,
		Enc_ASCII,
		Enc_MBCS
	};

protected:
	static Language fLanguage;
	static char* fLangTags[kNumLanguages];
	static char* fLangNames[kNumLanguages];
	static bool fUsesUnicode[kNumLanguages];
	static encodingTypes fUnicodeEncoding[kNumLanguages];

	static hsBool IGetLocalized(const char* name, Language lang, char* localizedName);

public:
	// Sets the default language, as determined by the installer
	static void SetDefaultLanguage();
	
	static void SetLanguage(Language lang) { fLanguage = lang; }
	static Language GetLanguage() { return fLanguage; }

	static char* GetLanguageName(Language lang) { return fLangNames[lang]; }

	static hsBool UsingUnicode() { return fUsesUnicode[fLanguage]; }
	static encodingTypes UnicodeEncoding() { return fUnicodeEncoding[fLanguage]; }

	// Returns true if we're using localized assets.  If it returns false, you
	// don't need to bother calling GetLocalized
	static hsBool IsLocalized() { return fLanguage != kEnglish; }

	// Pass in a key name and this will give you the localized name
	// Returns false if the original keyname is not for a localized asset
	static hsBool GetLocalized(const char* name, char* localizedName) { return IGetLocalized(name, fLanguage, localizedName); }

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
	static hsBool ExportGetLocalized(const char* name, int lang, char* localizedName);
	// Just tells us if this is localized, doesn't actually convert it for us
	static hsBool IsLocalizedName(const char* name) { return IGetLocalized(name, kEnglish, nil); }

	// Converts a vector of translated strings to a encoded string that can be decoded by StringToLocal()
	// The index in the vector of a string is it's language
	static std::string LocalToString(const std::vector<std::string> & localizedText);
	// Converts a string encoded by LocalToString to a vector of translated strings
	static std::vector<std::string> StringToLocal(const std::string & localizedText);
	static std::vector<std::wstring> StringToLocal(const std::wstring & localizedText);
};

#endif // plLocalization_h_inc