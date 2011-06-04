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
//////////////////////////////////////////////////////////////////////
//
// pfLocalizationDataMgr - singleton class for managing the
//                         localization XML data tree
//
//////////////////////////////////////////////////////////////////////

#ifndef _pfLocalizationDataMgr_h
#define _pfLocalizationDataMgr_h

#include "hsTypes.h"
#include "hsStlUtils.h"

#include "pfLocalizedString.h"

class plStatusLog;

// Helper classes/structs that are only used in this main class
class LocalizationDatabase;
struct LocElementInfo;
struct LocSetInfo;
struct LocAgeInfo;

class pfLocalizationDataMgr
{
private:
	static pfLocalizationDataMgr*	fInstance;
	static plStatusLog*				fLog;

protected:
	// This is a special case map class that will deconstruct the "Age.Set.Name" key into component parts
	// and store them so that a list of each part given it's parent part is easy to grab. I.e. I can grab
	// a list of all age names, a list of all set names (given age name) and a set of all names (given
	// age and set names)
	template<class mapT>
	class pf3PartMap
	{
	protected:
		// Outer map is Age, then Set, finally Name
		typedef std::map<std::wstring, std::map<std::wstring, std::map<std::wstring, mapT> > > ThreePartMap;
		ThreePartMap fData;

		void ISplitString(std::wstring key, std::wstring &age, std::wstring &set, std::wstring &name);
	public:
		// We will just have very basic functionality
		bool exists(const std::wstring & key); // returns true if the key exists
		bool setExists(const std::wstring & key); // returns true if the age.set exists (ignores name if passed in)
		void erase(const std::wstring & key); // erases the key from the map

		mapT &operator[](const std::wstring &key); // returns the item referenced by the key (and creates if necessary)

		std::vector<std::wstring> getAgeList(); // returns a list of all ages in this map
		std::vector<std::wstring> getSetList(const std::wstring & age); // returns a list of all sets in the specified age
		std::vector<std::wstring> getNameList(const std::wstring & age, const std::wstring & set);
	};

	LocalizationDatabase *fDatabase;

	typedef std::map<std::wstring, pfLocalizedString> localizedElement;

	// Contains all localized strings, the key is the Age.Set.Name specified by XML, in localizedElement, the key is the language string
	pf3PartMap<localizedElement> fLocalizedElements;

	std::string fDataPath;

	localizedElement ICreateLocalizedElement(); // ease of use function that creates a basic localized element object

	std::wstring IGetCurrentLanguageName(); // get the name of the current language
	std::vector<std::wstring> IGetAllLanguageNames();

	void IConvertElement(LocElementInfo *elementInfo, const std::wstring & curPath);
	void IConvertSet(LocSetInfo *setInfo, const std::wstring & curPath);
	void IConvertAge(LocAgeInfo *ageInfo, const std::wstring & curPath);

	char *IConvertToByteStream(const std::wstring & data, UInt32 &len); // converts the wstring data to a string of bytes for file writing
	void IWriteText(const std::string & filename, const std::wstring & ageName, const std::wstring & languageName); // Write localization text to the specified file

	pfLocalizationDataMgr(const std::string & path);
public:
	virtual ~pfLocalizationDataMgr();

	static void Initialize(const std::string & path);
	static void Shutdown();
	static pfLocalizationDataMgr &Instance(void) {return *fInstance;}
	static bool InstanceValid(void) {return fInstance != nil;}

	void SetupData();

	pfLocalizedString GetElement(const std::wstring & name);
	pfLocalizedString GetSpecificElement(const std::wstring & name, const std::wstring & languageName);

	std::vector<std::wstring> GetAgeList();
	std::vector<std::wstring> GetSetList(const std::wstring & ageName);
	std::vector<std::wstring> GetElementList(const std::wstring & ageName, const std::wstring & setName);
	std::vector<std::wstring> GetLanguages(const std::wstring & ageName, const std::wstring & setName, const std::wstring & elementName);

	std::wstring GetElementXMLData(const std::wstring & name, const std::wstring & languageName);
	std::wstring GetElementPlainTextData(const std::wstring & name, const std::wstring & languageName);

	// These convert the XML data to the actual subtitle and return true if successful (editor only)
	bool SetElementXMLData(const std::wstring & name, const std::wstring & languageName, const std::wstring & xmlData);
	bool SetElementPlainTextData(const std::wstring & name, const std::wstring & languageName, const std::wstring & plainText);

	// Addition and deletion functions, return true if successful (editor only)
	bool AddLocalization(const std::wstring & name, const std::wstring & newLanguage);
	bool AddElement(const std::wstring & name);
	bool DeleteLocalization(const std::wstring & name, const std::wstring & languageName);
	bool DeleteElement(const std::wstring & name);

	// Writes the current database to the disk (editor only). It will create all the files and put them into path
	void WriteDatabaseToDisk(const std::string & path);

	void OutputTreeToLog(); // prints the localization tree to the log file
};

#endif
