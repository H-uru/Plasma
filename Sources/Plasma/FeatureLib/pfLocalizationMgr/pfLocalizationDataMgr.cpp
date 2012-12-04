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

#include "HeadSpin.h"
#include "plString.h"

#include "plResMgr/plLocalization.h"

#include "hsFiles.h"
#include "plFile/plEncryptedStream.h"
#include "plStatusLog/plStatusLog.h"

#include "pfLocalizedString.h"
#include "pfLocalizationMgr.h"
#include "pfLocalizationDataMgr.h"

#include <expat.h>
#include <stack>

// MinGW sucks
#if defined(_WIN32) && !defined(_MSC_VER)
#   define swprintf _snwprintf
#endif

//////////////////////////////////////////////////////////////////////
//
// LocalizationXMLFile - a basic class for storing all the
//                       localization data grabbed from a single XML
//                       file
//
//////////////////////////////////////////////////////////////////////

class LocalizationXMLFile
{
public:
    //replace friend by static because of conflits with subtitleXMLFile
    static void XMLCALL StartTag(void *userData, const XML_Char *element, const XML_Char **attributes);
    static void XMLCALL EndTag(void *userData, const XML_Char *element);
    static void XMLCALL HandleData(void *userData, const XML_Char *data, int stringLength);
    friend class LocalizationDatabase;

    // first wstring is language, second is data
    typedef std::map<std::wstring, std::wstring> element;

    // the wstring is the element name
    typedef std::map<std::wstring, element> set;

    // the wstring is the set name
    typedef std::map<std::wstring, set> age;

    // the wstring is the age name
    typedef std::map<std::wstring, age> ageMap;

protected:
    bool fWeExploded; // alternative to massive error stack
    std::string fFilename;
    XML_Parser fParser;

    struct tagInfo
    {
        std::wstring fTag;
        std::map<std::wstring, std::wstring> fAttributes;
    };
    std::stack<tagInfo> fTagStack;

    int fSkipDepth; // if we need to skip a block, this is the depth we need to skip to

    bool fIgnoreContents; // are we ignoring the contents between tags?
    std::wstring fCurrentAge, fCurrentSet, fCurrentElement, fCurrentTranslation;

    ageMap fData;

    void IHandleLocalizationsTag(const tagInfo & parentTag, const tagInfo & thisTag);

    void IHandleAgeTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleSetTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleElementTag(const tagInfo & parentTag, const tagInfo & thisTag);

    void IHandleTranslationTag(const tagInfo & parentTag, const tagInfo & thisTag);

public:
    LocalizationXMLFile() : fWeExploded(false), fFilename("") { }

    bool Parse(const std::string & fileName); // returns false on failure
    void AddError(const plString& errorText);
};

// A few small helper structs
// I am setting these up so the header file can use this data without having to put
// the LocalizationXMLFile class into the header file
struct LocElementInfo
{
    LocalizationXMLFile::element fElement;
};

struct LocSetInfo
{
    LocalizationXMLFile::set fSet;
};

struct LocAgeInfo
{
    LocalizationXMLFile::age fAge;
};

//////////////////////////////////////////////////////////////////////
// Memory functions
//////////////////////////////////////////////////////////////////////

static void * XMLCALL XmlMalloc (size_t size) {
    return malloc(size);
}

static void * XMLCALL XmlRealloc (void * ptr, size_t size) {
    return realloc(ptr, size);
}

static void XMLCALL XmlFree (void * ptr) {
    free(ptr);
}

XML_Memory_Handling_Suite gHeapAllocator = {
    XmlMalloc,
    XmlRealloc,
    XmlFree
};


//////////////////////////////////////////////////////////////////////
//// XML Parsing functions ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//metmet remove static
void XMLCALL LocalizationXMLFile::StartTag(void *userData, const XML_Char *element, const XML_Char **attributes)
{
    std::wstring wElement = element;
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;
    std::map<std::wstring, std::wstring> wAttributes;

    for (int i = 0; attributes[i]; i += 2)
        wAttributes[attributes[i]] = attributes[i+1];

    LocalizationXMLFile::tagInfo parentTag;
    if (!file->fTagStack.empty())
        parentTag = file->fTagStack.top();

    LocalizationXMLFile::tagInfo newTag;
    newTag.fTag = wElement;
    newTag.fAttributes = wAttributes;

    file->fTagStack.push(newTag);

    if (file->fSkipDepth != -1) // we're currently skipping
        return;

    // now we handle this tag
    if (wElement == L"localizations")
        file->IHandleLocalizationsTag(parentTag, newTag);
    else if (wElement == L"age")
        file->IHandleAgeTag(parentTag, newTag);
    else if (wElement == L"set")
        file->IHandleSetTag(parentTag, newTag);
    else if (wElement == L"element")
        file->IHandleElementTag(parentTag, newTag);
    else if (wElement == L"translation")
        file->IHandleTranslationTag(parentTag, newTag);
    else
        file->AddError(plString::Format("Unknown tag %S found", wElement.c_str()));
}
//metmet remove static and include the function inside LocalizationXMLFile
void XMLCALL LocalizationXMLFile::EndTag(void *userData, const XML_Char *element)
{
    std::wstring wElement = element;
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;

    if (file->fSkipDepth != -1) // we're currently skipping
    {
        // check to see if we are done with the block we wanted skipped
        if (file->fTagStack.size() == file->fSkipDepth)
            file->fSkipDepth = -1; // we're done skipping
    }

    if (wElement == L"age") // we left the age block
        file->fCurrentAge = L"";
    else if (wElement == L"set") // we left the set block
        file->fCurrentSet = L"";
    else if (wElement == L"element") // we left the element block
        file->fCurrentElement = L"";
    else if (wElement == L"translation") // we left the translation block
    {
        file->fIgnoreContents = true;
        file->fCurrentTranslation = L"";
    }

    file->fTagStack.pop();
}
//metmet remove static and include the function inside LocalizationXMLFile
void XMLCALL LocalizationXMLFile::HandleData(void *userData, const XML_Char *data, int stringLength)
{
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;
    if (file->fIgnoreContents)
        return; // we're ignoring data, so just return
    if (file->fSkipDepth != -1) // we're currently skipping
        return;

    // This gets all data between tags, including indentation and newlines
    // so we'll have to ignore data when we aren't expecting it (not in a translation tag)
    std::wstring wData = std::wstring(data, stringLength);

    // we must be in a translation tag since that's the only tag that doesn't ignore the contents
    file->fData[file->fCurrentAge][file->fCurrentSet][file->fCurrentElement][file->fCurrentTranslation] += wData;
}

//////////////////////////////////////////////////////////////////////
//// LocalizationXMLFile Functions ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define FILEBUFFERSIZE 8192

//// IHandleSubtitlesTag() ///////////////////////////////////////////

void LocalizationXMLFile::IHandleLocalizationsTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    if (parentTag.fTag != L"") // we only allow <localizations> tags at root level
    {
        AddError("localizations tag only allowed at root level");
        return;
    }
}

//// IHandleAgeTag() /////////////////////////////////////////////////

void LocalizationXMLFile::IHandleAgeTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the subtitles tag
    if (parentTag.fTag != L"localizations")
    {
        AddError("age tag can only be directly inside a localizations tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find(L"name") == thisTag.fAttributes.end())
    {
        AddError("age tag is missing the name attribute");
        return;
    }

    fCurrentAge = thisTag.fAttributes.find(L"name")->second;
}

//// IHandleSetTag() /////////////////////////////////////////////////

void LocalizationXMLFile::IHandleSetTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the age tag
    if (parentTag.fTag != L"age")
    {
        AddError("set tag can only be directly inside a age tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find(L"name") == thisTag.fAttributes.end())
    {
        AddError("set tag is missing the name attribute");
        return;
    }

    fCurrentSet = thisTag.fAttributes.find(L"name")->second;
}

//// IHandleElementTag() /////////////////////////////////////////////

void LocalizationXMLFile::IHandleElementTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the element tag
    if (parentTag.fTag != L"set")
    {
        AddError("element tag can only be directly inside a set tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find(L"name") == thisTag.fAttributes.end())
    {
        AddError("element tag is missing the name attribute");
        return;
    }

    fCurrentElement = thisTag.fAttributes.find(L"name")->second;
}

//// IHandleTranslationTag() /////////////////////////////////////////

void LocalizationXMLFile::IHandleTranslationTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the element tag
    if (parentTag.fTag != L"element")
    {
        AddError("translation tag can only be directly inside a element tag");
        return;
    }

    // we have to have a language attribute
    if (thisTag.fAttributes.find(L"language") == thisTag.fAttributes.end())
    {
        AddError("translation tag is missing the language attribute");
        return;
    }

    fIgnoreContents = false; // we now want contents between tags
    fCurrentTranslation = thisTag.fAttributes.find(L"language")->second;
}

//// Parse() /////////////////////////////////////////////////////////

bool LocalizationXMLFile::Parse(const std::string & fileName)
{
    fFilename = fileName;

    while (!fTagStack.empty())
        fTagStack.pop();

    fCurrentAge = L"";
    fCurrentSet = L"";
    fCurrentElement = L"";
    fCurrentTranslation = L"";

    fIgnoreContents = true;
    fSkipDepth = -1;

    char Buff[FILEBUFFERSIZE];

    fParser = XML_ParserCreate_MM(NULL, &gHeapAllocator, NULL);
    if (!fParser)
    {
        AddError("ERROR: Couldn't allocate memory for parser");
        return false;
    }

    XML_SetElementHandler(fParser, StartTag, EndTag);
    XML_SetCharacterDataHandler(fParser, HandleData);
    XML_SetUserData(fParser, (void*)this);

    hsStream *xmlStream = plEncryptedStream::OpenEncryptedFile(fileName.c_str());
    if (!xmlStream)
    {
        pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Can't open file stream for %s", fileName.c_str());
        return false;
    }

    bool done = false;
    do
    {
        size_t len;

        len = xmlStream->Read(FILEBUFFERSIZE, Buff);
        done = xmlStream->AtEnd();

        if (XML_Parse(fParser, Buff, (int)len, done) == XML_STATUS_ERROR)
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Parse error at line %d: %S",
                XML_GetCurrentLineNumber(fParser), XML_ErrorString(XML_GetErrorCode(fParser)));
            done = true;
        }

        if (fWeExploded) // some error occurred in the parser
            done = true;
    } while (!done);

    XML_ParserFree(fParser);
    fParser = nil;
    xmlStream->Close();
    delete xmlStream;
    return true;
}

//// AddError() //////////////////////////////////////////////////////

void LocalizationXMLFile::AddError(const plString& errorText)
{
    pfLocalizationDataMgr::GetLog()->AddLineF("ERROR (line %d): %s",
        XML_GetCurrentLineNumber(fParser), errorText.c_str());
    fSkipDepth = fTagStack.size(); // skip this block
    fWeExploded = true;
    return;
}

//////////////////////////////////////////////////////////////////////
//
// LocalizationDatabase - a basic class for storing all the subtitle
//                        data grabbed from a XML directory (handles
//                        the merging and final validation of the data)
//
//////////////////////////////////////////////////////////////////////

class LocalizationDatabase
{
protected:
    std::string fDirectory; // the directory we're supposed to parse

    std::vector<LocalizationXMLFile> fFiles; // the various XML files in that directory

    LocalizationXMLFile::ageMap fData;

    LocalizationXMLFile::element IMergeElementData(LocalizationXMLFile::element firstElement, LocalizationXMLFile::element secondElement, const std::wstring & fileName, const std::wstring & path);
    LocalizationXMLFile::set IMergeSetData(LocalizationXMLFile::set firstSet, LocalizationXMLFile::set secondSet, const std::wstring & fileName, const std::wstring & path);
    LocalizationXMLFile::age IMergeAgeData(LocalizationXMLFile::age firstAge, LocalizationXMLFile::age secondAge, const std::wstring & fileName, const std::wstring & path);
    void IMergeData(); // merge all localization data in the files

    void IVerifyElement(const std::wstring &ageName, const std::wstring &setName, LocalizationXMLFile::set::iterator& curElement);
    void IVerifySet(const std::wstring &ageName, const std::wstring &setName);
    void IVerifyAge(const std::wstring &ageName);
    void IVerifyData(); // verify the localization data once it has been merged in

public:
    LocalizationDatabase() {}

    void Parse(const std::string & directory);

    LocalizationXMLFile::ageMap GetData() {return fData;}
};

//////////////////////////////////////////////////////////////////////
//// LocalizationDatabase Functions //////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// IMergeElementData ///////////////////////////////////////////////

LocalizationXMLFile::element LocalizationDatabase::IMergeElementData(LocalizationXMLFile::element firstElement, LocalizationXMLFile::element secondElement, const std::wstring & fileName, const std::wstring & path)
{
    // copy the data over, alerting the user to any duplicate translations
    LocalizationXMLFile::element::iterator curTranslation;
    for (curTranslation = secondElement.begin(); curTranslation != secondElement.end(); curTranslation++)
    {
        if (firstElement.find(curTranslation->first) != firstElement.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("Duplicate %S translation for %S found in file %S Ignoring second translation",
                curTranslation->first.c_str(), path.c_str(), fileName.c_str());
        }
        else
            firstElement[curTranslation->first] = curTranslation->second;
    }

    return firstElement;
}

//// IMergeSetData ///////////////////////////////////////////////////

LocalizationXMLFile::set LocalizationDatabase::IMergeSetData(LocalizationXMLFile::set firstSet, LocalizationXMLFile::set secondSet, const std::wstring & fileName, const std::wstring & path)
{
    // Merge all the elements
    LocalizationXMLFile::set::iterator curElement;
    for (curElement = secondSet.begin(); curElement != secondSet.end(); curElement++)
    {
        // if the element doesn't exist in the current set, add it
        if (firstSet.find(curElement->first) == firstSet.end())
            firstSet[curElement->first] = curElement->second;
        else // merge the element in
            firstSet[curElement->first] = IMergeElementData(firstSet[curElement->first], curElement->second, fileName, path + L"." + curElement->first);
    }

    return firstSet;
}

//// IMergeAgeData ///////////////////////////////////////////////////

LocalizationXMLFile::age LocalizationDatabase::IMergeAgeData(LocalizationXMLFile::age firstAge, LocalizationXMLFile::age secondAge, const std::wstring & fileName, const std::wstring & path)
{
    // Merge all the sets
    LocalizationXMLFile::age::iterator curSet;
    for (curSet = secondAge.begin(); curSet != secondAge.end(); curSet++)
    {
        // if the set doesn't exist in the current age, just add it
        if (firstAge.find(curSet->first) == firstAge.end())
            firstAge[curSet->first] = curSet->second;
        else // merge the data in
            firstAge[curSet->first] = IMergeSetData(firstAge[curSet->first], curSet->second, fileName, path + L"." + curSet->first);
    }

    return firstAge;
}

//// IMergeData() ////////////////////////////////////////////////////

void LocalizationDatabase::IMergeData()
{
    for (int i = 0; i < fFiles.size(); i++)
    {
        std::wstring wFilename;
        wchar_t *buff = hsStringToWString(fFiles[i].fFilename.c_str());
        wFilename = buff;
        delete [] buff;

        LocalizationXMLFile::ageMap fileData = fFiles[i].fData;
        LocalizationXMLFile::ageMap::iterator curAge;
        for (curAge = fileData.begin(); curAge != fileData.end(); curAge++)
        {
            // if the age doesn't exist in the current merged database, just add it with no more checking
            if (fData.find(curAge->first) == fData.end())
                fData[curAge->first] = curAge->second;
            else // otherwise, merge the data in
                fData[curAge->first] = IMergeAgeData(fData[curAge->first], curAge->second, wFilename, curAge->first);
        }
    }
}

//// IVerifyElement() ////////////////////////////////////////////////

void LocalizationDatabase::IVerifyElement(const std::wstring &ageName, const std::wstring &setName, LocalizationXMLFile::set::iterator& curElement)
{
    WStringVector languageNames;
    std::wstring defaultLanguage;

    int numLocales = plLocalization::GetNumLocales();
    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        const char *name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        wchar_t *wName = hsStringToWString(name);
        languageNames.push_back(wName);
        delete [] wName;
    }
    defaultLanguage = languageNames[0];

    std::wstring elementName = curElement->first;
    LocalizationXMLFile::element& theElement = curElement->second;
    LocalizationXMLFile::element::iterator curTranslation = theElement.begin();

    while (curTranslation != theElement.end())
    {
        // Make sure this language exists!
        bool languageExists = false;
        for (int i = 0; i < languageNames.size(); i++)
        {
            if (languageNames[i] == curTranslation->first)
            {
                languageExists = true;
                break;
            }
        }

        if (!languageExists)
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: The language %S used by %S.%S.%S is not supported, discarding translation",
                curTranslation->first.c_str(), ageName.c_str(), setName.c_str(), elementName.c_str());
            curTranslation = theElement.erase(curTranslation);
        }
        else
            curTranslation++;
    }

    for (int i = 1; i < languageNames.size(); i++)
    {
        if (theElement.find(languageNames[i]) == theElement.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Language %S is missing from the translations in element %S.%S.%S, you'll want to get translations for that!",
                languageNames[i].c_str(), ageName.c_str(), setName.c_str(), elementName.c_str());
        }
    }
}

//// IVerifySet() ////////////////////////////////////////////////////

void LocalizationDatabase::IVerifySet(const std::wstring &ageName, const std::wstring &setName)
{
    LocalizationXMLFile::set& theSet = fData[ageName][setName];
    LocalizationXMLFile::set::iterator curElement = theSet.begin();
    wchar_t *wDefLang = hsStringToWString(plLocalization::GetLanguageName((plLocalization::Language)0));
    std::wstring defaultLanguage = wDefLang;
    delete [] wDefLang;

    while (curElement != theSet.end())
    {
        // Check that we at least have a default language translation for fallback
        if (curElement->second.find(defaultLanguage) == curElement->second.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Default language %S is missing from the translations in element %S.%S.%S, deleting element",
                defaultLanguage.c_str(), ageName.c_str(), setName.c_str(), curElement->first.c_str());
            curElement = theSet.erase(curElement);
        }
        else
        {
            IVerifyElement(ageName, setName, curElement);
            curElement++;
        }
    }
}

//// IVerifyAge() ////////////////////////////////////////////////////

void LocalizationDatabase::IVerifyAge(const std::wstring &ageName)
{
    LocalizationXMLFile::age& theAge = fData[ageName];
    LocalizationXMLFile::age::iterator curSet;
    for (curSet = theAge.begin(); curSet != theAge.end(); curSet++)
        IVerifySet(ageName, curSet->first);
}

//// IVerifyData() ///////////////////////////////////////////////////

void LocalizationDatabase::IVerifyData()
{
    LocalizationXMLFile::ageMap::iterator curAge;
    for (curAge = fData.begin(); curAge != fData.end(); curAge++)
        IVerifyAge(curAge->first);
}

//// Parse() /////////////////////////////////////////////////////////

void LocalizationDatabase::Parse(const std::string & directory)
{
    fDirectory = directory;
    fFiles.clear();

    char filename[255];
    hsFolderIterator xmlFolder((directory+PATH_SEPARATOR_STR).c_str());
    while(xmlFolder.NextFileSuffix(".loc"))
    {
        xmlFolder.GetPathAndName(filename);

        LocalizationXMLFile newFile;
        bool retVal = newFile.Parse(filename);
        if (!retVal)
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Errors in file %s", filename);

        fFiles.push_back(newFile);
        pfLocalizationDataMgr::GetLog()->AddLineF("File %s parsed and added to database", filename);
    }

    IMergeData();
    IVerifyData();

    return;
}

//////////////////////////////////////////////////////////////////////
//// pf3PartMap Functions ////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// ISplitString() //////////////////////////////////////////////////

template<class mapT>
void pfLocalizationDataMgr::pf3PartMap<mapT>::ISplitString(std::wstring key, std::wstring &age, std::wstring &set, std::wstring &name)
{
    std::wstring::size_type periodLoc = key.find(L".");
    age = key.substr(0, periodLoc);
    if (periodLoc >= key.length())
        return; // don't get set or name if there isn't any period

    key = key.substr(periodLoc + 1, key.length());
    periodLoc = key.find(L".");
    set = key.substr(0, periodLoc);
    if (periodLoc >= key.length())
        return; // don't get name if there isn't another period

    name = key.substr(periodLoc + 1, key.length());
}

//// exists() ////////////////////////////////////////////////////////

template<class mapT>
bool pfLocalizationDataMgr::pf3PartMap<mapT>::exists(const std::wstring & key)
{
    std::wstring age, set, name;
    ISplitString(key, age, set, name);
    if (age == L"" || set == L"" || name == L"") // if any are missing, it's invalid, so we don't have it
        return false;

    // now check individually
    if (fData.find(age) == fData.end()) // age doesn't exist
        return false;
    if (fData[age].find(set) == fData[age].end()) // set doesn't exist
        return false;
    if (fData[age][set].find(name) == fData[age][set].end()) // name doesn't exist
        return false;

    // we passed all the tests, return true!
    return true;
}

//// setExists() /////////////////////////////////////////////////////

template<class mapT>
bool pfLocalizationDataMgr::pf3PartMap<mapT>::setExists(const std::wstring & key)
{
    std::wstring age, set, name;
    ISplitString(key, age, set, name);
    if (age == L"" || set == L"") // if any are missing, it's invalid, so we don't have it (ignoring name)
        return false;

    // now check individually
    if (fData.find(age) == fData.end()) // age doesn't exist
        return false;
    if (fData[age].find(set) == fData[age].end()) // set doesn't exist
        return false;

    // we passed all the tests, return true!
    return true;
}

//// erase() /////////////////////////////////////////////////////////

template<class mapT>
void pfLocalizationDataMgr::pf3PartMap<mapT>::erase(const std::wstring & key)
{
    std::wstring age, set, name;
    ISplitString(key, age, set, name);
    if (age == L"" || set == L"" || name == L"") // if any are missing, it's invalid, so we don't delete it
        return;

    // now check individually
    if (fData.find(age) == fData.end()) // age doesn't exist
        return;
    if (fData[age].find(set) == fData[age].end()) // set doesn't exist
        return;
    if (fData[age][set].find(name) == fData[age][set].end()) // name doesn't exist
        return;

    // ok, so now we want to nuke it!
    fData[age][set].erase(name);
    if (fData[age][set].size() == 0) // is the set now empty?
        fData[age].erase(set); // nuke it!
    if (fData[age].size() == 0) // is the age now empty?
        fData.erase(age); // nuke it!
}

//// operator[]() ////////////////////////////////////////////////////

template<class mapT>
mapT &pfLocalizationDataMgr::pf3PartMap<mapT>::operator[](const std::wstring &key)
{
    std::wstring age, set, name;
    ISplitString(key, age, set, name);
    return fData[age][set][name];
}

//// getAgeList() ////////////////////////////////////////////////////

template<class mapT>
WStringVector pfLocalizationDataMgr::pf3PartMap<mapT>::getAgeList()
{
    WStringVector retVal;
    typename ThreePartMap::iterator curAge;

    for (curAge = fData.begin(); curAge != fData.end(); curAge++)
        retVal.push_back(curAge->first);

    return retVal;
}

//// getSetList() ////////////////////////////////////////////////////

template<class mapT>
WStringVector pfLocalizationDataMgr::pf3PartMap<mapT>::getSetList(const std::wstring & age)
{
    WStringVector retVal;
    typename std::map<std::wstring, std::map<std::wstring, mapT> >::iterator curSet;

    if (fData.find(age) == fData.end())
        return retVal; // return an empty list, the age doesn't exist

    for (curSet = fData[age].begin(); curSet != fData[age].end(); curSet++)
        retVal.push_back(curSet->first);

    return retVal;
}

//// getNameList() ///////////////////////////////////////////////////

template<class mapT>
WStringVector pfLocalizationDataMgr::pf3PartMap<mapT>::getNameList(const std::wstring & age, const std::wstring & set)
{
    WStringVector retVal;
    typename std::map<std::wstring, mapT>::iterator curName;

    if (fData.find(age) == fData.end())
        return retVal; // return an empty list, the age doesn't exist

    if (fData[age].find(set) == fData[age].end())
        return retVal; // return an empty list, the set doesn't exist

    for (curName = fData[age][set].begin(); curName != fData[age][set].end(); curName++)
        retVal.push_back(curName->first);

    return retVal;
}

//////////////////////////////////////////////////////////////////////
//// pfLocalizationDataMgr Functions /////////////////////////////////
//////////////////////////////////////////////////////////////////////

pfLocalizationDataMgr   *pfLocalizationDataMgr::fInstance = nil;
plStatusLog             *pfLocalizationDataMgr::fLog = nil; // output logfile

//// Constructor/Destructor //////////////////////////////////////////

pfLocalizationDataMgr::pfLocalizationDataMgr(const std::string & path)
{
    hsAssert(!fInstance, "Tried to create the localization data manager more than once!");
    fInstance = this;

    fDataPath = path;

    fDatabase = nil;
}

pfLocalizationDataMgr::~pfLocalizationDataMgr()
{
    fInstance = nil;

    if (fDatabase)
    {
        delete fDatabase;
        fDatabase = nil;
    }
}

//// ICreateLocalizedElement /////////////////////////////////////////

pfLocalizationDataMgr::localizedElement pfLocalizationDataMgr::ICreateLocalizedElement()
{
    int numLocales = plLocalization::GetNumLocales();
    pfLocalizationDataMgr::localizedElement retVal;

    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        const char *name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        wchar_t *wName = hsStringToWString(name);
        retVal[wName] = L"";
        delete [] wName;
    }

    return retVal;
}

//// IGetCurrentLanguageName /////////////////////////////////////////

std::wstring pfLocalizationDataMgr::IGetCurrentLanguageName()
{
    std::wstring retVal;
    const char *name = plLocalization::GetLanguageName(plLocalization::GetLanguage());
    wchar_t *wName = hsStringToWString(name);
    retVal = wName;
    delete [] wName;
    return retVal;
}

//// IGetAllLanguageNames ////////////////////////////////////////////

WStringVector pfLocalizationDataMgr::IGetAllLanguageNames()
{
    int numLocales = plLocalization::GetNumLocales();
    WStringVector retVal;

    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        const char *name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        wchar_t *wName = hsStringToWString(name);
        retVal.push_back(wName);
        delete [] wName;
    }

    return retVal;
}

//// IConvertSubtitle ////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertElement(LocElementInfo *elementInfo, const std::wstring & curPath)
{
    pfLocalizationDataMgr::localizedElement newElement;
    int16_t numArgs = -1;

    LocalizationXMLFile::element::iterator curTranslation;
    for (curTranslation = elementInfo->fElement.begin(); curTranslation != elementInfo->fElement.end(); curTranslation++)
    {
        newElement[curTranslation->first].FromXML(curTranslation->second);
        uint16_t argCount = newElement[curTranslation->first].GetArgumentCount();
        if (numArgs == -1) // just started
            numArgs = argCount;
        else if (argCount != numArgs)
            fLog->AddLineF("WARNING: Argument number mismatch in element %S for %S", curPath.c_str(), curTranslation->first.c_str());
    }

    fLocalizedElements[curPath] = newElement;
}

//// IConvertSet /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertSet(LocSetInfo *setInfo, const std::wstring & curPath)
{
    LocalizationXMLFile::set::iterator curElement;
    for (curElement = setInfo->fSet.begin(); curElement != setInfo->fSet.end(); curElement++)
    {
        LocElementInfo elementInfo;
        elementInfo.fElement = curElement->second;

        IConvertElement(&elementInfo, curPath + L"." + curElement->first);
    }
}

//// IConvertAge /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertAge(LocAgeInfo *ageInfo, const std::wstring & curPath)
{
    LocalizationXMLFile::age::iterator curSet;
    for (curSet = ageInfo->fAge.begin(); curSet != ageInfo->fAge.end(); curSet++)
    {
        LocSetInfo setInfo;
        setInfo.fSet = curSet->second;

        IConvertSet(&setInfo, curPath + L"." + curSet->first);
    }
}

//// IConvertToByteStream ////////////////////////////////////////////

char *pfLocalizationDataMgr::IConvertToByteStream(const std::wstring & data, uint32_t &len)
{
    len = data.length() * 2 + 2; // each wchar_t is two chars and add two bytes for the header
    char *retVal = new char[len]; // we don't add an extra byte for the 0 because the parser doesn't need it
    char lowByte = 0, highByte = 0;
    retVal[0] = (char)0xFF; // insert FFFE for little-endian UTF-16 (big-endian would be FEFF)
    retVal[1] = (char)0xFE;
    int curByteStreamPos = 2;
    for (int curLoc = 0; curLoc < data.length(); curLoc++)
    {
        wchar_t curChar = data[curLoc];
        lowByte = (char)(curChar & 0x00FF);
        highByte = (char)((curChar & 0xFF00) >> 8);

        // since the data is AABBCCDD, we need to put in in our uint8_t stream as BBAADDCC
        // (so it kinda looks backward because we're storing this as little-endian)
        retVal[curByteStreamPos + 1] = highByte;
        retVal[curByteStreamPos] = lowByte;
        curByteStreamPos += 2;
    }
    return retVal;
}

//// IWriteText //////////////////////////////////////////////////////

void pfLocalizationDataMgr::IWriteText(const std::string & filename, const std::wstring & ageName, const std::wstring & languageName)
{
    bool weWroteData = false; // did we actually write any data of consequence?
    bool setEmpty = true;

    // we will try to pretty print it all so it's easy to read for the devs
    std::wstring fileData = L"<?xml version=\"1.0\" encoding=\"utf-16\"?>\n"; // stores the xml we are going to write to the file (UTF-16 format)
    fileData += L"<localizations>\n";
    fileData += L"\t<age name=\"" + ageName + L"\">\n";

    WStringVector setNames = GetSetList(ageName);
    for (int curSet = 0; curSet < setNames.size(); curSet++)
    {
        setEmpty = true; // so far, this set is empty
        std::wstring setCode = L"";
        setCode += L"\t\t<set name=\"" + setNames[curSet] + L"\">\n";

        WStringVector elementNames = GetElementList(ageName, setNames[curSet]);
        for (int curElement = 0; curElement < elementNames.size(); curElement++)
        {
            setCode += L"\t\t\t<element name=\"" + elementNames[curElement] + L"\">\n";
            std::wstring key = ageName + L"." + setNames[curSet] + L"." + elementNames[curElement];

            if (fLocalizedElements[key].find(languageName) != fLocalizedElements[key].end())
            {
                std::wstring key = ageName + L"." + setNames[curSet] + L"." + elementNames[curElement];
                weWroteData = true;
                setEmpty = false;
                setCode += L"\t\t\t\t<translation language=\"" + languageName + L"\">";
                setCode += fLocalizedElements[key][languageName].ToXML();
                setCode += L"</translation>\n";
            }

            setCode += L"\t\t\t</element>\n";
        }

        setCode += L"\t\t</set>\n";

        if (!setEmpty)
            fileData += setCode;
    }

    fileData += L"\t</age>\n";
    fileData += L"</localizations>\n";

    if (weWroteData)
    {
        // now spit the results out to the file
        uint32_t numBytes;
        char *byteStream = IConvertToByteStream(fileData, numBytes);
        hsStream *xmlStream = plEncryptedStream::OpenEncryptedFileWrite(filename.c_str());
        xmlStream->Write(numBytes, byteStream);
        xmlStream->Close();
        delete xmlStream;
        delete [] byteStream;
    }
}

//// Initialize //////////////////////////////////////////////////////

void pfLocalizationDataMgr::Initialize(const std::string & path)
{
    if (fInstance)
        return;

    fInstance = new pfLocalizationDataMgr(path);
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(30, "LocalizationDataMgr.log",
        plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kTimestamp);
    fInstance->SetupData();
}

//// Shutdown ////////////////////////////////////////////////////////

void pfLocalizationDataMgr::Shutdown()
{
    if ( fLog != nil )
    {
        delete fLog;
        fLog = nil;
    }

    if (fInstance)
    {
        delete fInstance;
        fInstance = nil;
    }
}

//// SetupData ///////////////////////////////////////////////////////

void pfLocalizationDataMgr::SetupData()
{
    if (fDatabase)
        delete fDatabase;

    fDatabase = new LocalizationDatabase();
    fDatabase->Parse(fDataPath);

    fLog->AddLine("File reading complete, converting to native data format");

    // and now we read all the data out of the database and convert it to our native formats

    // transfer subtitle data
    LocalizationXMLFile::ageMap data = fDatabase->GetData();
    LocalizationXMLFile::ageMap::iterator curAge;
    for (curAge = data.begin(); curAge != data.end(); curAge++)
    {
        LocAgeInfo ageInfo;
        ageInfo.fAge = curAge->second;

        IConvertAge(&ageInfo, curAge->first);
    }

    OutputTreeToLog();
}

//// GetElement //////////////////////////////////////////////////////

pfLocalizedString pfLocalizationDataMgr::GetElement(const std::wstring & name)
{
    pfLocalizedString retVal; // if this returns before we initialize it, it will be empty, indicating failure

    if (!fLocalizedElements.exists(name)) // does the requested element exist?
        return retVal; // nope, so return failure

    std::wstring languageName = IGetCurrentLanguageName();
    if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end()) // current language isn't specified
    {
        languageName = L"English"; // force to english
        if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end()) // make sure english exists
            return retVal; // language doesn't exist
    }
    retVal = fLocalizedElements[name][languageName];
    return retVal;
}

//// GetSpecificElement //////////////////////////////////////////////

pfLocalizedString pfLocalizationDataMgr::GetSpecificElement(const std::wstring & name, const std::wstring & language)
{
    pfLocalizedString retVal; // if this returns before we initialize it, it will have an ID of 0, indicating failure

    if (!fLocalizedElements.exists(name)) // does the requested subtitle exist?
        return retVal; // nope, so return failure

    if (fLocalizedElements[name].find(language) == fLocalizedElements[name].end())
        return retVal; // language doesn't exist

    retVal = fLocalizedElements[name][language];
    return retVal;
}

//// GetLanguages ////////////////////////////////////////////////////

WStringVector pfLocalizationDataMgr::GetLanguages(const std::wstring & ageName, const std::wstring & setName, const std::wstring & elementName)
{
    WStringVector retVal;
    std::wstring key = ageName + L"." + setName + L"." + elementName;
    if (fLocalizedElements.exists(key))
    {
        // age, set, and element exists
        localizedElement elem = fLocalizedElements[key];
        localizedElement::iterator curLanguage;
        for (curLanguage = elem.begin(); curLanguage != elem.end(); curLanguage++)
        {
            std::wstring language = curLanguage->first;
            if (!language.empty()) // somehow blank language names sneak in... so don't return them
                retVal.push_back(curLanguage->first);
        }
    }
    return retVal;
}

//// GetElementXMLData ///////////////////////////////////////////////

std::wstring pfLocalizationDataMgr::GetElementXMLData(const std::wstring & name, const std::wstring & languageName)
{
    std::wstring retVal = L"";
    if (fLocalizedElements.exists(name))
    {
        if (fLocalizedElements[name].find(languageName) != fLocalizedElements[name].end())
            retVal = fLocalizedElements[name][languageName].ToXML();
    }
    return retVal;
}

//// GetElementPlainTextData /////////////////////////////////////////

std::wstring pfLocalizationDataMgr::GetElementPlainTextData(const std::wstring & name, const std::wstring & languageName)
{
    std::wstring retVal = L"";
    if (fLocalizedElements.exists(name))
    {
        if (fLocalizedElements[name].find(languageName) != fLocalizedElements[name].end())
            retVal = fLocalizedElements[name][languageName];
    }
    return retVal;
}

//// SetElementXMLData ///////////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementXMLData(const std::wstring & name, const std::wstring & languageName, const std::wstring & xmlData)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName].FromXML(xmlData);
    return true;
}

//// SetElementPlainTextData /////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementPlainTextData(const std::wstring & name, const std::wstring & languageName, const std::wstring & plainText)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName] = plainText;
    return true;
}

//// AddLocalization /////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddLocalization(const std::wstring & name, const std::wstring & newLanguage)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // copy the english over so it can be localized
    fLocalizedElements[name][newLanguage] = fLocalizedElements[name][L"English"];
    return true;
}

//// AddElement //////////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddElement(const std::wstring & name)
{
    if (fLocalizedElements.exists(name))
        return false; // already exists

    pfLocalizedString newElement;
    fLocalizedElements[name][L"English"] = newElement;
    return true;
}

//// DeleteLocalization //////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteLocalization(const std::wstring & name, const std::wstring & languageName)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end())
        return false; // doesn't exist

    fLocalizedElements[name].erase(languageName);
    return true;
}

//// DeleteElement ///////////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteElement(const std::wstring & name)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // delete it!
    fLocalizedElements.erase(name);
    return true;
}

//// WriteDatabaseToDisk /////////////////////////////////////////////

void pfLocalizationDataMgr::WriteDatabaseToDisk(const std::string & path)
{
    // first, write the styles and panel settings to styles.sub
    WStringVector ageNames = GetAgeList();
    WStringVector languageNames = IGetAllLanguageNames();
    for (int curAge = 0; curAge < ageNames.size(); curAge++)
    {
        for (int curLanguage = 0; curLanguage < languageNames.size(); curLanguage++)
        {
            std::string cAgeName, cLanguageName;
            char *temp = hsWStringToString(ageNames[curAge].c_str());
            cAgeName = temp;
            delete [] temp;
            temp = hsWStringToString(languageNames[curLanguage].c_str());
            cLanguageName = temp;
            delete [] temp;

            IWriteText(path + "/" + cAgeName + cLanguageName + ".loc", ageNames[curAge], languageNames[curLanguage]);
        }
    }
}

//// OutputTreeToLog /////////////////////////////////////////////////

void pfLocalizationDataMgr::OutputTreeToLog()
{
    WStringVector ages = GetAgeList();

    fLog->AddLine("\n");
    fLog->AddLine("Localization tree:\n");

    for (WStringVector::iterator i = ages.begin(); i != ages.end(); ++i)
    {
        std::wstring age = *i;
        fLog->AddLineF("\t%S", age.c_str());

        WStringVector sets = GetSetList(age);
        for (WStringVector::iterator j = sets.begin(); j != sets.end(); ++j)
        {
            std::wstring set = (*j);
            fLog->AddLineF("\t\t%S", set.c_str());

            WStringVector names = GetElementList(age, set);
            for (WStringVector::iterator k = names.begin(); k != names.end(); ++k)
            {
                std::wstring name = (*k);
                fLog->AddLineF("\t\t\t%S", name.c_str());
            }
        }
    }
}
