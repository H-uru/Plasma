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

#include "plFile/plEncryptedStream.h"
#include "plStatusLog/plStatusLog.h"

#include "pfLocalizedString.h"
#include "pfLocalizationMgr.h"
#include "pfLocalizationDataMgr.h"

#include <expat.h>
#include <stack>


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

    // first string is language, second is data
    typedef std::map<plString, plString> element;

    // the string is the element name
    typedef std::map<plString, element> set;

    // the string is the set name
    typedef std::map<plString, set> age;

    // the string is the age name
    typedef std::map<plString, age> ageMap;

protected:
    bool fWeExploded; // alternative to massive error stack
    plFileName fFilename;
    XML_Parser fParser;

    struct tagInfo
    {
        plString fTag;
        std::map<plString, plString> fAttributes;
    };
    std::stack<tagInfo> fTagStack;

    int fSkipDepth; // if we need to skip a block, this is the depth we need to skip to

    bool fIgnoreContents; // are we ignoring the contents between tags?
    plString fCurrentAge, fCurrentSet, fCurrentElement, fCurrentTranslation;

    ageMap fData;

    void IHandleLocalizationsTag(const tagInfo & parentTag, const tagInfo & thisTag);

    void IHandleAgeTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleSetTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleElementTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleTranslationTag(const tagInfo & parentTag, const tagInfo & thisTag);

public:
    LocalizationXMLFile() : fWeExploded(false) { }

    bool Parse(const plFileName & fileName); // returns false on failure
    void AddError(const plString & errorText);
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

void XMLCALL LocalizationXMLFile::StartTag(void *userData, const XML_Char *element, const XML_Char **attributes)
{
    plString wElement = plString::FromWchar(element);
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;
    std::map<plString, plString> wAttributes;

    for (int i = 0; attributes[i]; i += 2)
        wAttributes[plString::FromWchar(attributes[i])] = plString::FromWchar(attributes[i+1]);

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
    if (wElement == "localizations")
        file->IHandleLocalizationsTag(parentTag, newTag);
    else if (wElement == "age")
        file->IHandleAgeTag(parentTag, newTag);
    else if (wElement == "set")
        file->IHandleSetTag(parentTag, newTag);
    else if (wElement == "element")
        file->IHandleElementTag(parentTag, newTag);
    else if (wElement == "translation")
        file->IHandleTranslationTag(parentTag, newTag);
    else
        file->AddError(plString::Format("Unknown tag %s found", wElement.c_str()));
}

void XMLCALL LocalizationXMLFile::EndTag(void *userData, const XML_Char *element)
{
    plString wElement = plString::FromWchar(element);
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;

    if (file->fSkipDepth != -1) // we're currently skipping
    {
        // check to see if we are done with the block we wanted skipped
        if (file->fTagStack.size() == file->fSkipDepth)
            file->fSkipDepth = -1; // we're done skipping
    }

    if (wElement == "age") // we left the age block
        file->fCurrentAge = "";
    else if (wElement == "set") // we left the set block
        file->fCurrentSet = "";
    else if (wElement == "element") // we left the element block
        file->fCurrentElement = "";
    else if (wElement == "translation") // we left the translation block
    {
        file->fIgnoreContents = true;
        file->fCurrentTranslation = "";
    }

    file->fTagStack.pop();
}

void XMLCALL LocalizationXMLFile::HandleData(void *userData, const XML_Char *data, int stringLength)
{
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;
    if (file->fIgnoreContents)
        return; // we're ignoring data, so just return
    if (file->fSkipDepth != -1) // we're currently skipping
        return;

    // This gets all data between tags, including indentation and newlines
    // so we'll have to ignore data when we aren't expecting it (not in a translation tag)
    plString contents = plString::FromWchar(data, stringLength);

    // we must be in a translation tag since that's the only tag that doesn't ignore the contents
    file->fData[file->fCurrentAge][file->fCurrentSet][file->fCurrentElement][file->fCurrentTranslation] += contents;
}

//////////////////////////////////////////////////////////////////////
//// LocalizationXMLFile Functions ///////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define FILEBUFFERSIZE 8192

//// IHandleSubtitlesTag() ///////////////////////////////////////////

void LocalizationXMLFile::IHandleLocalizationsTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    if (!parentTag.fTag.IsEmpty()) // we only allow <localizations> tags at root level
    {
        AddError("localizations tag only allowed at root level");
        return;
    }
}

//// IHandleAgeTag() /////////////////////////////////////////////////

void LocalizationXMLFile::IHandleAgeTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the subtitles tag
    if (parentTag.fTag != "localizations")
    {
        AddError("age tag can only be directly inside a localizations tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find("name") == thisTag.fAttributes.end())
    {
        AddError("age tag is missing the name attribute");
        return;
    }

    fCurrentAge = thisTag.fAttributes.find("name")->second;
}

//// IHandleSetTag() /////////////////////////////////////////////////

void LocalizationXMLFile::IHandleSetTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the age tag
    if (parentTag.fTag != "age")
    {
        AddError("set tag can only be directly inside a age tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find("name") == thisTag.fAttributes.end())
    {
        AddError("set tag is missing the name attribute");
        return;
    }

    fCurrentSet = thisTag.fAttributes.find("name")->second;
}

//// IHandleElementTag() /////////////////////////////////////////////

void LocalizationXMLFile::IHandleElementTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the element tag
    if (parentTag.fTag != "set")
    {
        AddError("element tag can only be directly inside a set tag");
        return;
    }

    // we have to have a name attribute
    if (thisTag.fAttributes.find("name") == thisTag.fAttributes.end())
    {
        AddError("element tag is missing the name attribute");
        return;
    }

    fCurrentElement = thisTag.fAttributes.find("name")->second;
}

//// IHandleTranslationTag() /////////////////////////////////////////

void LocalizationXMLFile::IHandleTranslationTag(const LocalizationXMLFile::tagInfo & parentTag, const LocalizationXMLFile::tagInfo & thisTag)
{
    // it has to be inside the element tag
    if (parentTag.fTag != "element")
    {
        AddError("translation tag can only be directly inside a element tag");
        return;
    }

    // we have to have a language attribute
    if (thisTag.fAttributes.find("language") == thisTag.fAttributes.end())
    {
        AddError("translation tag is missing the language attribute");
        return;
    }

    fIgnoreContents = false; // we now want contents between tags
    fCurrentTranslation = thisTag.fAttributes.find("language")->second;
}

//// Parse() /////////////////////////////////////////////////////////

bool LocalizationXMLFile::Parse(const plFileName& fileName)
{
    fFilename = fileName;

    while (!fTagStack.empty())
        fTagStack.pop();

    fCurrentAge = "";
    fCurrentSet = "";
    fCurrentElement = "";
    fCurrentTranslation = "";

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

    hsStream *xmlStream = plEncryptedStream::OpenEncryptedFile(fileName);
    if (!xmlStream)
    {
        pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Can't open file stream for %s", fileName.AsString().c_str());
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
    plFileName fDirectory; // the directory we're supposed to parse

    std::vector<LocalizationXMLFile> fFiles; // the various XML files in that directory

    LocalizationXMLFile::ageMap fData;

    LocalizationXMLFile::element IMergeElementData(LocalizationXMLFile::element firstElement, LocalizationXMLFile::element secondElement, const plFileName & fileName, const plString & path);
    LocalizationXMLFile::set IMergeSetData(LocalizationXMLFile::set firstSet, LocalizationXMLFile::set secondSet, const plFileName & fileName, const plString & path);
    LocalizationXMLFile::age IMergeAgeData(LocalizationXMLFile::age firstAge, LocalizationXMLFile::age secondAge, const plFileName & fileName, const plString & path);
    void IMergeData(); // merge all localization data in the files

    void IVerifyElement(const plString &ageName, const plString &setName, LocalizationXMLFile::set::iterator& curElement);
    void IVerifySet(const plString &ageName, const plString &setName);
    void IVerifyAge(const plString &ageName);
    void IVerifyData(); // verify the localization data once it has been merged in

public:
    LocalizationDatabase() {}

    void Parse(const plFileName & directory);
    LocalizationXMLFile::ageMap GetData() {return fData;}
};

//////////////////////////////////////////////////////////////////////
//// LocalizationDatabase Functions //////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// IMergeElementData ///////////////////////////////////////////////

LocalizationXMLFile::element LocalizationDatabase::IMergeElementData(LocalizationXMLFile::element firstElement, LocalizationXMLFile::element secondElement, const plFileName & fileName, const plString & path)
{
    // copy the data over, alerting the user to any duplicate translations
    LocalizationXMLFile::element::iterator curTranslation;
    for (curTranslation = secondElement.begin(); curTranslation != secondElement.end(); curTranslation++)
    {
        if (firstElement.find(curTranslation->first) != firstElement.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("Duplicate %s translation for %s found in file %s. Ignoring second translation.",
                curTranslation->first.c_str(), path.c_str(), fileName.AsString().c_str());
        }
        else
            firstElement[curTranslation->first] = curTranslation->second;
    }

    return firstElement;
}

//// IMergeSetData ///////////////////////////////////////////////////

LocalizationXMLFile::set LocalizationDatabase::IMergeSetData(LocalizationXMLFile::set firstSet, LocalizationXMLFile::set secondSet, const plFileName & fileName, const plString & path)
{
    // Merge all the elements
    LocalizationXMLFile::set::iterator curElement;
    for (curElement = secondSet.begin(); curElement != secondSet.end(); curElement++)
    {
        // if the element doesn't exist in the current set, add it
        if (firstSet.find(curElement->first) == firstSet.end())
            firstSet[curElement->first] = curElement->second;
        else // merge the element in
            firstSet[curElement->first] = IMergeElementData(firstSet[curElement->first], curElement->second, fileName, 
                plString::Format("%s.%s", path.c_str(), curElement->first.c_str()));
    }

    return firstSet;
}

//// IMergeAgeData ///////////////////////////////////////////////////

LocalizationXMLFile::age LocalizationDatabase::IMergeAgeData(LocalizationXMLFile::age firstAge, LocalizationXMLFile::age secondAge, const plFileName & fileName, const plString & path)
{
    // Merge all the sets
    LocalizationXMLFile::age::iterator curSet;
    for (curSet = secondAge.begin(); curSet != secondAge.end(); curSet++)
    {
        // if the set doesn't exist in the current age, just add it
        if (firstAge.find(curSet->first) == firstAge.end())
            firstAge[curSet->first] = curSet->second;
        else // merge the data in
            firstAge[curSet->first] = IMergeSetData(firstAge[curSet->first], curSet->second, fileName, 
                plString::Format("%s.%s", path.c_str(), curSet->first.c_str()));
    }

    return firstAge;
}

//// IMergeData() ////////////////////////////////////////////////////

void LocalizationDatabase::IMergeData()
{
    for (int i = 0; i < fFiles.size(); i++)
    {
        LocalizationXMLFile::ageMap fileData = fFiles[i].fData;
        LocalizationXMLFile::ageMap::iterator curAge;
        for (curAge = fileData.begin(); curAge != fileData.end(); curAge++)
        {
            // if the age doesn't exist in the current merged database, just add it with no more checking
            if (fData.find(curAge->first) == fData.end())
                fData[curAge->first] = curAge->second;
            else // otherwise, merge the data in
                fData[curAge->first] = IMergeAgeData(fData[curAge->first], curAge->second, fFiles[i].fFilename, curAge->first);
        }
    }
}

//// IVerifyElement() ////////////////////////////////////////////////

void LocalizationDatabase::IVerifyElement(const plString &ageName, const plString &setName, LocalizationXMLFile::set::iterator& curElement)
{
    std::vector<plString> languageNames;
    plString defaultLanguage;

    int numLocales = plLocalization::GetNumLocales();
    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        plString name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        languageNames.push_back(name);
    }
    defaultLanguage = languageNames[0];

    plString elementName = curElement->first;
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
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: The language %s used by %s.%s.%s is not supported. Discarding translation.",
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
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Language %s is missing from the translations in element %s.%s.%s. You'll want to get translations for that!",
                languageNames[i].c_str(), ageName.c_str(), setName.c_str(), elementName.c_str());
        }
    }
}

//// IVerifySet() ////////////////////////////////////////////////////

void LocalizationDatabase::IVerifySet(const plString &ageName, const plString &setName)
{
    LocalizationXMLFile::set& theSet = fData[ageName][setName];
    LocalizationXMLFile::set::iterator curElement = theSet.begin();

    plString defaultLanguage = plLocalization::GetLanguageName((plLocalization::Language)0);

    while (curElement != theSet.end())
    {
        // Check that we at least have a default language translation for fallback
        if (curElement->second.find(defaultLanguage) == curElement->second.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Default language %s is missing from the translations in element %s.%s.%s. Deleting element.",
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

void LocalizationDatabase::IVerifyAge(const plString &ageName)
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

void LocalizationDatabase::Parse(const plFileName & directory)
{
    fDirectory = directory;
    fFiles.clear();

    std::vector<plFileName> locFiles = plFileSystem::ListDir(directory, "*.loc");
    for (auto iter = locFiles.begin(); iter != locFiles.end(); ++iter)
    {
        LocalizationXMLFile newFile;
        bool retVal = newFile.Parse(*iter);
        if (!retVal)
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Errors in file %s", iter->GetFileName().c_str());

        fFiles.push_back(newFile);
        pfLocalizationDataMgr::GetLog()->AddLineF("File %s parsed and added to database", iter->GetFileName().c_str());
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
void pfLocalizationDataMgr::pf3PartMap<mapT>::ISplitString(plString key, plString &age, plString &set, plString &name)
{
    std::vector<plString> tokens = key.Tokenize(".");
    if (tokens.size() >= 1)
        age = tokens[0];
    if (tokens.size() >= 2)
        set = tokens[1];
    if (tokens.size() >= 3)
        name = tokens[2];
}

//// exists() ////////////////////////////////////////////////////////

template<class mapT>
bool pfLocalizationDataMgr::pf3PartMap<mapT>::exists(const plString & key)
{
    plString age, set, name;
    ISplitString(key, age, set, name);
    if (age.IsEmpty() || set.IsEmpty() || name.IsEmpty()) // if any are missing, it's invalid, so we don't have it
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
bool pfLocalizationDataMgr::pf3PartMap<mapT>::setExists(const plString & key)
{
    plString age, set, name;
    ISplitString(key, age, set, name);
    if (age.IsEmpty() || set.IsEmpty()) // if any are missing, it's invalid, so we don't have it (ignoring name)
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
void pfLocalizationDataMgr::pf3PartMap<mapT>::erase(const plString & key)
{
    plString age, set, name;
    ISplitString(key, age, set, name);
    if (age.IsEmpty() || set.IsEmpty() || name.IsEmpty()) // if any are missing, it's invalid, so we don't delete it
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
mapT &pfLocalizationDataMgr::pf3PartMap<mapT>::operator[](const plString &key)
{
    plString age, set, name;
    ISplitString(key, age, set, name);
    return fData[age][set][name];
}

//// getAgeList() ////////////////////////////////////////////////////

template<class mapT>
std::vector<plString> pfLocalizationDataMgr::pf3PartMap<mapT>::getAgeList()
{
    std::vector<plString> retVal;
    typename ThreePartMap::iterator curAge;

    for (curAge = fData.begin(); curAge != fData.end(); curAge++)
        retVal.push_back(curAge->first);

    return retVal;
}

//// getSetList() ////////////////////////////////////////////////////

template<class mapT>
std::vector<plString> pfLocalizationDataMgr::pf3PartMap<mapT>::getSetList(const plString & age)
{
    std::vector<plString> retVal;
    typename std::map<plString, std::map<plString, mapT> >::iterator curSet;

    if (fData.find(age) == fData.end())
        return retVal; // return an empty list, the age doesn't exist

    for (curSet = fData[age].begin(); curSet != fData[age].end(); curSet++)
        retVal.push_back(curSet->first);

    return retVal;
}

//// getNameList() ///////////////////////////////////////////////////

template<class mapT>
std::vector<plString> pfLocalizationDataMgr::pf3PartMap<mapT>::getNameList(const plString & age, const plString & set)
{
    std::vector<plString> retVal;
    typename std::map<plString, mapT>::iterator curName;

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

pfLocalizationDataMgr::pfLocalizationDataMgr(const plFileName & path)
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
        retVal[plLocalization::GetLanguageName((plLocalization::Language)curLocale)] = "";
    }

    return retVal;
}

//// IGetCurrentLanguageName /////////////////////////////////////////

plString pfLocalizationDataMgr::IGetCurrentLanguageName()
{
    return plLocalization::GetLanguageName(plLocalization::GetLanguage());
}

//// IGetAllLanguageNames ////////////////////////////////////////////

std::vector<plString> pfLocalizationDataMgr::IGetAllLanguageNames()
{
    int numLocales = plLocalization::GetNumLocales();
    std::vector<plString> retVal;

    for (int curLocale = 0; curLocale <= numLocales; curLocale++)
    {
        plString name = plLocalization::GetLanguageName((plLocalization::Language)curLocale);
        retVal.push_back(name);
    }

    return retVal;
}

//// IConvertSubtitle ////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertElement(LocElementInfo *elementInfo, const plString & curPath)
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
            fLog->AddLineF("WARNING: Argument number mismatch in element %s for %s", curPath.c_str(), curTranslation->first.c_str());
    }

    fLocalizedElements[curPath] = newElement;
}

//// IConvertSet /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertSet(LocSetInfo *setInfo, const plString & curPath)
{
    LocalizationXMLFile::set::iterator curElement;
    for (curElement = setInfo->fSet.begin(); curElement != setInfo->fSet.end(); curElement++)
    {
        LocElementInfo elementInfo;
        elementInfo.fElement = curElement->second;

        IConvertElement(&elementInfo, plString::Format("%s.%s", curPath.c_str(), curElement->first.c_str()));
    }
}

//// IConvertAge /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertAge(LocAgeInfo *ageInfo, const plString & curPath)
{
    LocalizationXMLFile::age::iterator curSet;
    for (curSet = ageInfo->fAge.begin(); curSet != ageInfo->fAge.end(); curSet++)
    {
        LocSetInfo setInfo;
        setInfo.fSet = curSet->second;

        IConvertSet(&setInfo, plString::Format("%s.%s", curPath.c_str(), curSet->first.c_str()));
    }
}

//// IWriteText //////////////////////////////////////////////////////

void pfLocalizationDataMgr::IWriteText(const plFileName & filename, const plString & ageName, const plString & languageName)
{
    bool weWroteData = false; // did we actually write any data of consequence?
    bool setEmpty = true;

    // we will try to pretty print it all so it's easy to read for the devs
    plStringStream fileData;
    fileData << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    fileData << "<localizations>\n";
    fileData << plString::Format("\t<age name=\"%s\">\n", ageName.c_str());

    std::vector<plString> setNames = GetSetList(ageName);
    for (int curSet = 0; curSet < setNames.size(); curSet++)
    {
        setEmpty = true; // so far, this set is empty
        plStringStream setCode;
        setCode << plString::Format("\t\t<set name=\"%s\">\n", setNames[curSet].c_str());

        std::vector<plString> elementNames = GetElementList(ageName, setNames[curSet]);
        for (int curElement = 0; curElement < elementNames.size(); curElement++)
        {
            setCode << plString::Format("\t\t\t<element name=\"%s\">\n", elementNames[curElement].c_str());
            plString key = plString::Format("%s.%s.%s", ageName.c_str(), setNames[curSet].c_str(), elementNames[curElement].c_str());

            if (fLocalizedElements[key].find(languageName) != fLocalizedElements[key].end())
            {
                weWroteData = true;
                setEmpty = false;
                setCode << plString::Format("\t\t\t\t<translation language=\"%s\">", languageName.c_str());
                setCode << fLocalizedElements[key][languageName].ToXML();
                setCode << "</translation>\n";
            }

            setCode << "\t\t\t</element>\n";
        }

        setCode << "\t\t</set>\n";

        if (!setEmpty)
            fileData << setCode.GetString();
    }

    fileData << "\t</age>\n";
    fileData << "</localizations>\n";

    if (weWroteData)
    {
        // now spit the results out to the file
        hsStream *xmlStream = plEncryptedStream::OpenEncryptedFileWrite(filename);
        xmlStream->Write(fileData.GetLength(), fileData.GetRawBuffer());
        xmlStream->Close();
        delete xmlStream;
    }
}

//// Initialize //////////////////////////////////////////////////////

void pfLocalizationDataMgr::Initialize(const plFileName & path)
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

    // transfer localization data
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

pfLocalizedString pfLocalizationDataMgr::GetElement(const plString & name)
{
    pfLocalizedString retVal; // if this returns before we initialize it, it will be empty, indicating failure

    if (!fLocalizedElements.exists(name)) // does the requested element exist?
        return retVal; // nope, so return failure

    plString languageName = IGetCurrentLanguageName();
    if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end()) // current language isn't specified
    {
        languageName = "English"; // force to english
        if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end()) // make sure english exists
            return retVal; // language doesn't exist
    }
    retVal = fLocalizedElements[name][languageName];
    return retVal;
}

//// GetSpecificElement //////////////////////////////////////////////

pfLocalizedString pfLocalizationDataMgr::GetSpecificElement(const plString & name, const plString & language)
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

std::vector<plString> pfLocalizationDataMgr::GetLanguages(const plString & ageName, const plString & setName, const plString & elementName)
{
    std::vector<plString> retVal;
    plString key = plString::Format("%s.%s.%s", ageName.c_str(), setName.c_str(), elementName.c_str());
    if (fLocalizedElements.exists(key))
    {
        // age, set, and element exists
        localizedElement elem = fLocalizedElements[key];
        localizedElement::iterator curLanguage;
        for (curLanguage = elem.begin(); curLanguage != elem.end(); curLanguage++)
        {
            plString language = curLanguage->first;
            if (!language.IsEmpty()) // somehow blank language names sneak in... so don't return them
                retVal.push_back(curLanguage->first);
        }
    }
    return retVal;
}

//// GetElementXMLData ///////////////////////////////////////////////

plString pfLocalizationDataMgr::GetElementXMLData(const plString & name, const plString & languageName)
{
    if (fLocalizedElements.exists(name) && (fLocalizedElements[name].find(languageName) != fLocalizedElements[name].end()))
        return fLocalizedElements[name][languageName].ToXML();
    return "";
}

//// GetElementPlainTextData /////////////////////////////////////////

plString pfLocalizationDataMgr::GetElementPlainTextData(const plString & name, const plString & languageName)
{
    if (fLocalizedElements.exists(name) && (fLocalizedElements[name].find(languageName) != fLocalizedElements[name].end()))
        return fLocalizedElements[name][languageName];
    return "";
}

//// SetElementXMLData ///////////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementXMLData(const plString & name, const plString & languageName, const plString & xmlData)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName].FromXML(xmlData);
    return true;
}

//// SetElementPlainTextData /////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementPlainTextData(const plString & name, const plString & languageName, const plString & plainText)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName] = plainText;
    return true;
}

//// AddLocalization /////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddLocalization(const plString & name, const plString & newLanguage)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // copy the english over so it can be localized
    fLocalizedElements[name][newLanguage] = fLocalizedElements[name]["English"];
    return true;
}

//// AddElement //////////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddElement(const plString & name)
{
    if (fLocalizedElements.exists(name))
        return false; // already exists

    pfLocalizedString newElement;
    fLocalizedElements[name]["English"] = newElement;
    return true;
}

//// DeleteLocalization //////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteLocalization(const plString & name, const plString & languageName)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end())
        return false; // doesn't exist

    fLocalizedElements[name].erase(languageName);
    return true;
}

//// DeleteElement ///////////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteElement(const plString & name)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // delete it!
    fLocalizedElements.erase(name);
    return true;
}

//// WriteDatabaseToDisk /////////////////////////////////////////////

void pfLocalizationDataMgr::WriteDatabaseToDisk(const plFileName & path)
{
    std::vector<plString> ageNames = GetAgeList();
    std::vector<plString> languageNames = IGetAllLanguageNames();
    for (int curAge = 0; curAge < ageNames.size(); curAge++)
    {
        for (int curLanguage = 0; curLanguage < languageNames.size(); curLanguage++)
        {
            plFileName locPath = plFileName::Join(path, plString::Format("%s%s.loc",
                                    ageNames[curAge].c_str(), languageNames[curLanguage].c_str()));
            IWriteText(locPath, ageNames[curAge], languageNames[curLanguage]);
        }
    }
}

//// OutputTreeToLog /////////////////////////////////////////////////

void pfLocalizationDataMgr::OutputTreeToLog()
{
    std::vector<plString> ages = GetAgeList();

    fLog->AddLine("\n");
    fLog->AddLine("Localization tree:\n");

    for (std::vector<plString>::iterator i = ages.begin(); i != ages.end(); ++i)
    {
        plString age = *i;
        fLog->AddLineF("\t%s", age.c_str());

        std::vector<plString> sets = GetSetList(age);
        for (std::vector<plString>::iterator j = sets.begin(); j != sets.end(); ++j)
        {
            plString set = (*j);
            fLog->AddLineF("\t\t%s", set.c_str());

            std::vector<plString> names = GetElementList(age, set);
            for (std::vector<plString>::iterator k = names.begin(); k != names.end(); ++k)
            {
                plString name = (*k);
                fLog->AddLineF("\t\t\t%s", name.c_str());
            }
        }
    }
}
