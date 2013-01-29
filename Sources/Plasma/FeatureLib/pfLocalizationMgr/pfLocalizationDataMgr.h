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
//////////////////////////////////////////////////////////////////////
//
// pfLocalizationDataMgr - singleton class for managing the
//                         localization XML data tree
//
//////////////////////////////////////////////////////////////////////

#ifndef _pfLocalizationDataMgr_h
#define _pfLocalizationDataMgr_h

#include "HeadSpin.h"
#include <map>

#include "pfLocalizedString.h"
#include "plFileSystem.h"

class plStatusLog;

// Helper classes/structs that are only used in this main class
class LocalizationDatabase;
struct LocElementInfo;
struct LocSetInfo;
struct LocAgeInfo;

class pfLocalizationDataMgr
{
private:
    static pfLocalizationDataMgr*   fInstance;
    static plStatusLog*             fLog;

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
        typedef std::map<plString, std::map<plString, std::map<plString, mapT> > > ThreePartMap;
        ThreePartMap fData;

        void ISplitString(plString key, plString &age, plString &set, plString &name);
    public:
        // We will just have very basic functionality
        bool exists(const plString & key); // returns true if the key exists
        bool setExists(const plString & key); // returns true if the age.set exists (ignores name if passed in)
        void erase(const plString & key); // erases the key from the map

        mapT &operator[](const plString &key); // returns the item referenced by the key (and creates if necessary)

        std::vector<plString> getAgeList(); // returns a list of all ages in this map
        std::vector<plString> getSetList(const plString & age); // returns a list of all sets in the specified age
        std::vector<plString> getNameList(const plString & age, const plString & set);
    };

    LocalizationDatabase *fDatabase;

    typedef std::map<plString, pfLocalizedString> localizedElement;

    // Contains all localized strings, the key is the Age.Set.Name specified by XML, in localizedElement, the key is the language string
    pf3PartMap<localizedElement> fLocalizedElements;

    plFileName fDataPath;

    localizedElement ICreateLocalizedElement(); // ease of use function that creates a basic localized element object

    plString IGetCurrentLanguageName(); // get the name of the current language
    std::vector<plString> IGetAllLanguageNames();

    void IConvertElement(LocElementInfo *elementInfo, const plString & curPath);
    void IConvertSet(LocSetInfo *setInfo, const plString & curPath);
    void IConvertAge(LocAgeInfo *ageInfo, const plString & curPath);

    void IWriteText(const plFileName & filename, const plString & ageName, const plString & languageName); // Write localization text to the specified file

    pfLocalizationDataMgr(const plFileName & path);
public:
    virtual ~pfLocalizationDataMgr();

    static void Initialize(const plFileName & path);
    static void Shutdown();
    static pfLocalizationDataMgr &Instance(void) {return *fInstance;}
    static bool InstanceValid(void) {return fInstance != nil;}
    static plStatusLog* GetLog() { return fLog; }

    void SetupData();

    pfLocalizedString GetElement(const plString & name);
    pfLocalizedString GetSpecificElement(const plString & name, const plString & languageName);

    std::vector<plString> GetAgeList()
    {
        return fLocalizedElements.getAgeList();
    }
    std::vector<plString> GetSetList(const plString & ageName)
    {
        return fLocalizedElements.getSetList(ageName);
    }
    std::vector<plString> GetElementList(const plString & ageName, const plString & setName)
    {
        return fLocalizedElements.getNameList(ageName, setName);
    }
    std::vector<plString> GetLanguages(const plString & ageName, const plString & setName, const plString & elementName);    

    plString GetElementXMLData(const plString & name, const plString & languageName);
    plString GetElementPlainTextData(const plString & name, const plString & languageName);

    // These convert the XML data to the actual subtitle and return true if successful (editor only)
    bool SetElementXMLData(const plString & name, const plString & languageName, const plString & xmlData);
    bool SetElementPlainTextData(const plString & name, const plString & languageName, const plString & plainText);

    // Addition and deletion functions, return true if successful (editor only)
    bool AddLocalization(const plString & name, const plString & newLanguage);
    bool AddElement(const plString & name);
    bool DeleteLocalization(const plString & name, const plString & languageName);
    bool DeleteElement(const plString & name);

    // Writes the current database to the disk (editor only). It will create all the files and put them into path
    void WriteDatabaseToDisk(const plFileName & path);

    void OutputTreeToLog(); // prints the localization tree to the log file
};

#endif
