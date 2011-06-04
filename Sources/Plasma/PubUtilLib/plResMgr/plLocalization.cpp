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
#include "hsTypes.h"
#include "plLocalization.h"
#include "../plFile/plFileUtils.h"
#include "hsUtils.h"

plLocalization::Language plLocalization::fLanguage = plLocalization::kEnglish;

char* plLocalization::fLangTags[] =
{
	"_eng", // kEnglish
	"_fre", // kFrench
	"_ger", // kGerman
	"_spa", // kSpanish
	"_ita", // kItalian
	"_jpn"  // kJapanese
};
const int kLangTagLen = 4;

char* plLocalization::fLangNames[] =
{
	"English", // kEnglish
	"French",  // kFrench
	"German",  // kGerman
	"Spanish", // kSpanish
	"Italian", // kItalian
	"Japanese" // kJapanese
};

bool plLocalization::fUsesUnicode[] =
{
	false,	// kEnglish
	false,	// kFrench
	false,	// kGerman
	false,	// kSpanish
	false,	// kItalian
	true	// kJapanese
};

plLocalization::encodingTypes plLocalization::fUnicodeEncoding[] =
{
	Enc_Unencoded,	// kEnglish
	Enc_Unencoded,	// kFrench
	Enc_Unencoded,	// kGerman
	Enc_Unencoded,	// kSpanish
	Enc_Unencoded,	// kItalian
	Enc_UTF8,		// kJapanese
};

hsBool plLocalization::IGetLocalized(const char* name, Language lang, char* localizedName)
{
	const char* underscore = strrchr(name, '_');
	
	if (underscore)
	{
		char langTag[kLangTagLen+1];
		strncpy(langTag,underscore,kLangTagLen);
		langTag[kLangTagLen] = '\0';
		
		if (strncmp(langTag, fLangTags[kEnglish], kLangTagLen) == 0)
		{
			if (localizedName)
			{
				strcpy(localizedName, name);
				int underscorePos = underscore - name;
				memcpy(localizedName + underscorePos, fLangTags[lang], kLangTagLen);
			}

			return true;
		}
	}

	return false;
}

hsBool plLocalization::ExportGetLocalized(const char* name, int lang, char* localizedName)
{
	return IGetLocalized(name, Language(lang+1), localizedName) &&
			plFileUtils::FileExists(localizedName);
}

std::string plLocalization::LocalToString(const std::vector<std::string> & localizedText)
{
	std::string retVal = "";
	for (int i=0; i<localizedText.size(); i++)
	{
		if (i > kNumLanguages-1)
			break;
		std::string langHeader = "$";
		std::string langName = GetLanguageName((Language)i);
		langHeader += langName.substr(0,2) + "$";
		retVal += langHeader + localizedText[i];
	}
	return retVal;
}

std::vector<std::string> plLocalization::StringToLocal(const std::string & localizedText)
{
	std::vector<std::string> retVal;
	wchar_t *temp = hsStringToWString(localizedText.c_str());
	std::wstring wLocalizedText = temp;
	delete [] temp;

	std::vector<std::wstring> wStringVector = StringToLocal(wLocalizedText);
	int i;
	for (i=0; i<wStringVector.size(); i++)
	{
		char *local = hsWStringToString(wStringVector[i].c_str());
		std::string val = local;
		delete [] local;
		retVal.push_back(val);
	}

	return retVal;
}

std::vector<std::wstring> plLocalization::StringToLocal(const std::wstring & localizedText)
{
	std::vector<std::wstring> tags;
	std::vector<int> tagLocs;
	std::vector<int> sortedTagLocs;
	std::vector<std::wstring> retVal;
	int i;
	for (i=0; i<kNumLanguages; i++)
	{
		std::wstring tag = L"$";
		std::string temp = GetLanguageName((Language)i);
		wchar_t *wTemp = hsStringToWString(temp.c_str());
		std::wstring langName = wTemp;
		delete [] wTemp;
		
		tag += langName.substr(0,2) + L"$";
		tags.push_back(tag);
		tagLocs.push_back(localizedText.find(tag));
		sortedTagLocs.push_back(i);
		retVal.push_back(L"");
	}
	for (i=0; i<kNumLanguages-1; i++)
	{
		for (int j=i; j<kNumLanguages; j++)
		{
			if (tagLocs[sortedTagLocs[i]] > tagLocs[sortedTagLocs[j]])
				sortedTagLocs[i]^=sortedTagLocs[j]^=sortedTagLocs[i]^=sortedTagLocs[j]; // swap the contents (yes, it works)
		}
	}
	// now sortedTagLocs has the indexes of tagLocs sorted from smallest loc to highest loc
	hsBool noTags = true;
	for (i=0; i<kNumLanguages; i++)
	{
		int lang = sortedTagLocs[i]; // the language we are extracting
		if (tagLocs[lang] != -1)
		{
			noTags = false; // at least one tag was found in the text
			int startLoc = tagLocs[lang] + tags[lang].length();
			int endLoc;
			if (i+1 == kNumLanguages)
				endLoc = localizedText.length();
			else
				endLoc = tagLocs[sortedTagLocs[i+1]];
			retVal[lang] = localizedText.substr(startLoc,endLoc-startLoc);
		}
	}
	if (noTags)
		retVal[0] = localizedText; // if no tags were in the text, we assume it to be English
	return retVal;
}

#include "hsWindows.h"

void plLocalization::SetDefaultLanguage()
{
	fLanguage = kEnglish;
#if 0 // disable all languages
#ifdef HS_BUILD_FOR_WIN32
	HKEY hLocalKey = NULL;
	HKEY hMSKey = NULL;
	HKEY hWindowsKey = NULL;
	HKEY hCurVerKey = NULL;
	HKEY hUninstKey = NULL;
	HKEY hUruKey = NULL;

	#define RegOpen(key, retKey, subKeyName) RegOpenKeyEx(key, subKeyName, 0, KEY_READ, &retKey) == ERROR_SUCCESS

	if (RegOpen(HKEY_LOCAL_MACHINE,	hLocalKey,		"software") &&
		RegOpen(hLocalKey,			hMSKey,			"microsoft") &&
		RegOpen(hMSKey,				hWindowsKey,	"windows") &&
		RegOpen(hWindowsKey,		hCurVerKey,		"currentversion") &&
		RegOpen(hCurVerKey,			hUninstKey,		"uninstall") &&
		RegOpen(hUninstKey,			hUruKey,		"Uru - Ages Beyond Myst"))
	{
		char value[20];
		DWORD bufsize = sizeof(value);
		if (RegQueryValueEx(hUruKey, "Lang", NULL, NULL, (LPBYTE)value, &bufsize) == ERROR_SUCCESS)
		{
			if (hsStrEQ(value, "en"))
				fLanguage = kEnglish;
			else if (hsStrEQ(value, "fr"))
				fLanguage = kFrench;
			else if (hsStrEQ(value, "de"))
				fLanguage = kGerman;
			else if (hsStrEQ(value, "es"))
				fLanguage = kSpanish;
			else if (hsStrEQ(value, "it"))
				fLanguage = kItalian;
			else if (hsStrEQ(value, "jp"))
				fLanguage = kJapanese;
			else
				fLanguage = kEnglish;
		}
	}

	RegCloseKey(hLocalKey);
	RegCloseKey(hMSKey);
	RegCloseKey(hWindowsKey);
	RegCloseKey(hCurVerKey);
	RegCloseKey(hUninstKey);
	RegCloseKey(hUruKey);
#endif // HS_BUILD_FOR_WIN32
#endif // 0
}
