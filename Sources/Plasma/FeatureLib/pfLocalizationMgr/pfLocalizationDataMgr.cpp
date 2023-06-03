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

#include "plFile/plEncryptedStream.h"
#include "plResMgr/plLocalization.h"
#include "plStatusLog/plStatusLog.h"

#include "pfLocalizedString.h"
#include "pfLocalizationMgr.h"
#include "pfLocalizationDataMgr.h"

#include <expat.h>

#include <algorithm>
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
    typedef std::map<ST::string, ST::string> element;

    // the string is the element name
    typedef std::map<ST::string, element> set;

    // the string is the set name
    typedef std::map<ST::string, set> age;

    // the string is the age name
    typedef std::map<ST::string, age> ageMap;

protected:
    bool fWeExploded; // alternative to massive error stack
    plFileName fFilename;
    XML_Parser fParser;

    struct tagInfo
    {
        ST::string fTag;
        std::map<ST::string, ST::string> fAttributes;
    };
    std::stack<tagInfo> fTagStack;

    int fSkipDepth; // if we need to skip a block, this is the depth we need to skip to

    bool fIgnoreContents; // are we ignoring the contents between tags?
    ST::string fCurrentAge, fCurrentSet, fCurrentElement, fCurrentTranslation;

    ageMap fData;

    void IHandleLocalizationsTag(const tagInfo & parentTag, const tagInfo & thisTag);

    void IHandleAgeTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleSetTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleElementTag(const tagInfo & parentTag, const tagInfo & thisTag);
    void IHandleTranslationTag(const tagInfo & parentTag, const tagInfo & thisTag);

public:
    LocalizationXMLFile() : fWeExploded(), fParser(), fSkipDepth(), fIgnoreContents() { }
    LocalizationXMLFile(LocalizationXMLFile&& move)
        : fWeExploded(move.fWeExploded), fFilename(std::move(move.fFilename)),
          fParser(move.fParser), fTagStack(std::move(move.fTagStack)),
          fSkipDepth(move.fSkipDepth), fIgnoreContents(move.fIgnoreContents),
          fCurrentAge(std::move(move.fCurrentAge)), fCurrentSet(std::move(move.fCurrentSet)),
          fCurrentElement(std::move(move.fCurrentElement)),
          fCurrentTranslation(std::move(move.fCurrentTranslation)),
          fData(std::move(move.fData))
    {
        move.fParser = nullptr;
    }

    bool Parse(const plFileName & fileName); // returns false on failure
    void AddError(const ST::string & errorText);
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
    LocalizationXMLFile *file = (LocalizationXMLFile*)userData;

    const auto& parentTag = file->fTagStack.top();
    auto& newTag = file->fTagStack.emplace();
    newTag.fTag = element;
    for (size_t i = 0; attributes[i]; i += 2)
        newTag.fAttributes[attributes[i]] = attributes[i + 1];

    if (file->fSkipDepth != -1) // we're currently skipping
        return;

    // now we handle this tag
    if (newTag.fTag == "localizations")
        file->IHandleLocalizationsTag(parentTag, newTag);
    else if (newTag.fTag == "age")
        file->IHandleAgeTag(parentTag, newTag);
    else if (newTag.fTag == "set")
        file->IHandleSetTag(parentTag, newTag);
    else if (newTag.fTag == "element")
        file->IHandleElementTag(parentTag, newTag);
    else if (newTag.fTag == "translation")
        file->IHandleTranslationTag(parentTag, newTag);
    else
        file->AddError(ST::format("Unknown tag {} found", newTag.fTag));
}

void XMLCALL LocalizationXMLFile::EndTag(void *userData, const XML_Char *element)
{
    ST::string wElement = element;
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
    ST::string contents = ST::string::from_utf8(data, stringLength);

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
    if (!parentTag.fTag.empty()) // we only allow <localizations> tags at root level
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
    fTagStack.emplace();

    fCurrentAge = "";
    fCurrentSet = "";
    fCurrentElement = "";
    fCurrentTranslation = "";

    fIgnoreContents = true;
    fSkipDepth = -1;

    char Buff[FILEBUFFERSIZE];

    fParser = XML_ParserCreate_MM(nullptr, &gHeapAllocator, nullptr);
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
        pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Can't open file stream for {}", fileName);
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
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Parse error at line {}: {}",
                XML_GetCurrentLineNumber(fParser), XML_ErrorString(XML_GetErrorCode(fParser)));
            done = true;
        }

        if (fWeExploded) // some error occurred in the parser
            done = true;
    } while (!done);

    XML_ParserFree(fParser);
    fParser = nullptr;
    xmlStream->Close();
    delete xmlStream;
    return true;
}

//// AddError() //////////////////////////////////////////////////////

void LocalizationXMLFile::AddError(const ST::string& errorText)
{
    pfLocalizationDataMgr::GetLog()->AddLineF("ERROR (line {}): {}",
        XML_GetCurrentLineNumber(fParser), errorText);
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

    void IMergeElementData(LocalizationXMLFile::element& firstElement, const LocalizationXMLFile::element& secondElement, const plFileName & fileName, const ST::string & path);
    void IMergeSetData(LocalizationXMLFile::set& firstSet, const LocalizationXMLFile::set& secondSet, const plFileName & fileName, const ST::string & path);
    void IMergeAgeData(LocalizationXMLFile::age& firstAge, const LocalizationXMLFile::age& secondAge, const plFileName & fileName, const ST::string & path);
    void IMergeData(); // merge all localization data in the files

    void IVerifyElement(const ST::string &ageName, const ST::string &setName, LocalizationXMLFile::set::iterator& curElement);
    void IVerifySet(const ST::string &ageName, const ST::string &setName);
    void IVerifyAge(const ST::string &ageName);
    void IVerifyData(); // verify the localization data once it has been merged in

public:
    LocalizationDatabase() {}

    void Parse(const plFileName & directory);
    const LocalizationXMLFile::ageMap& GetData() const { return fData; }
};

//////////////////////////////////////////////////////////////////////
//// LocalizationDatabase Functions //////////////////////////////////
//////////////////////////////////////////////////////////////////////

//// IMergeElementData ///////////////////////////////////////////////

void LocalizationDatabase::IMergeElementData(LocalizationXMLFile::element& firstElement, const LocalizationXMLFile::element& secondElement, const plFileName& fileName, const ST::string& path)
{
    // copy the data over, alerting the user to any duplicate translations
    for (const auto& curTranslation : secondElement)
    {
        if (firstElement.find(curTranslation.first) != firstElement.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("Duplicate {} translation for {} found in file {}. Ignoring second translation.",
                curTranslation.first, path, fileName);
        }
        else
            firstElement[curTranslation.first] = curTranslation.second;
    }
}

//// IMergeSetData ///////////////////////////////////////////////////

void LocalizationDatabase::IMergeSetData(LocalizationXMLFile::set& firstSet, const LocalizationXMLFile::set& secondSet, const plFileName& fileName, const ST::string& path)
{
    // Merge all the elements
    for (const auto& curElement : secondSet)
    {
        // if the element doesn't exist in the current set, add it
        if (firstSet.find(curElement.first) == firstSet.end())
            firstSet[curElement.first] = curElement.second;
        else // merge the element in
            IMergeElementData(firstSet[curElement.first], curElement.second, fileName,
                ST::format("{}.{}", path, curElement.first));
    }
}

//// IMergeAgeData ///////////////////////////////////////////////////

void LocalizationDatabase::IMergeAgeData(LocalizationXMLFile::age& firstAge, const LocalizationXMLFile::age& secondAge, const plFileName& fileName, const ST::string& path)
{
    // Merge all the sets
    for (const auto& curSet : secondAge)
    {
        // if the set doesn't exist in the current age, just add it
        if (firstAge.find(curSet.first) == firstAge.end())
            firstAge[curSet.first] = curSet.second;
        else // merge the data in
            IMergeSetData(firstAge[curSet.first], curSet.second, fileName,
                ST::format("{}.{}", path, curSet.first));
    }
}

//// IMergeData() ////////////////////////////////////////////////////

void LocalizationDatabase::IMergeData()
{
    for (const auto& file : fFiles)
    {
        for (const auto& curAge : file.fData)
        {
            // if the age doesn't exist in the current merged database, just add it with no more checking
            if (fData.find(curAge.first) == fData.end())
                fData[curAge.first] = curAge.second;
            else // otherwise, merge the data in
                IMergeAgeData(fData[curAge.first], curAge.second, file.fFilename, curAge.first);
        }
    }
}

//// IVerifyElement() ////////////////////////////////////////////////

void LocalizationDatabase::IVerifyElement(const ST::string &ageName, const ST::string &setName, LocalizationXMLFile::set::iterator& curElement)
{
    auto languageNames = plLocalization::GetAllLanguageNames();

    ST::string elementName = curElement->first;
    LocalizationXMLFile::element& theElement = curElement->second;
    LocalizationXMLFile::element::iterator curTranslation = theElement.begin();

    while (curTranslation != theElement.end())
    {
        // Make sure this language exists!
        auto languageIt = std::find(languageNames.begin(), languageNames.end(), curTranslation->first);

        if (languageIt == languageNames.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: The language {} used by {}.{}.{} is not supported. Discarding translation.",
                curTranslation->first, ageName, setName, elementName);
            curTranslation = theElement.erase(curTranslation);
        }
        else
            curTranslation++;
    }

    for (auto lang : plLocalization::GetAllLanguages()) {
        if (
            plLocalization::IsLanguageUsable(lang)
            && theElement.find(plLocalization::GetLanguageName(lang)) == theElement.end()
        ) {
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Language {} is missing from the translations in element {}.{}.{}. You'll want to get translations for that!",
                plLocalization::GetLanguageName(lang), ageName, setName, elementName);
        }
    }
}

//// IVerifySet() ////////////////////////////////////////////////////

void LocalizationDatabase::IVerifySet(const ST::string &ageName, const ST::string &setName)
{
    LocalizationXMLFile::set& theSet = fData[ageName][setName];
    LocalizationXMLFile::set::iterator curElement = theSet.begin();

    ST::string defaultLanguage = plLocalization::GetLanguageName(plLocalization::kEnglish);

    while (curElement != theSet.end())
    {
        // Check that we at least have a default language translation for fallback
        if (curElement->second.find(defaultLanguage) == curElement->second.end())
        {
            pfLocalizationDataMgr::GetLog()->AddLineF("ERROR: Default language {} is missing from the translations in element {}.{}.{}. Deleting element.",
                defaultLanguage, ageName, setName, curElement->first);
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

void LocalizationDatabase::IVerifyAge(const ST::string &ageName)
{
    for (const auto& curSet : fData[ageName])
        IVerifySet(ageName, curSet.first);
}

//// IVerifyData() ///////////////////////////////////////////////////

void LocalizationDatabase::IVerifyData()
{
    for (const auto& curAge : fData)
        IVerifyAge(curAge.first);
}

//// Parse() /////////////////////////////////////////////////////////

void LocalizationDatabase::Parse(const plFileName & directory)
{
    fDirectory = directory;
    fFiles.clear();

    std::vector<plFileName> locFiles = plFileSystem::ListDir(directory, "*.loc");
    for (const auto& file : locFiles)
    {
        LocalizationXMLFile newFile;
        bool retVal = newFile.Parse(file);
        if (!retVal)
            pfLocalizationDataMgr::GetLog()->AddLineF("WARNING: Errors in file {}", file.GetFileName());

        fFiles.emplace_back(std::move(newFile));
        pfLocalizationDataMgr::GetLog()->AddLineF("File {} parsed and added to database", file.GetFileName());
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
void pfLocalizationDataMgr::pf3PartMap<mapT>::ISplitString(const ST::string& key, ST::string &age, ST::string &set, ST::string &name) const
{
    std::vector<ST::string> tokens = key.tokenize(".");
    if (tokens.size() >= 1)
        age = tokens[0];
    if (tokens.size() >= 2)
        set = tokens[1];
    if (tokens.size() >= 3)
        name = tokens[2];
}

//// exists() ////////////////////////////////////////////////////////

template<class mapT>
bool pfLocalizationDataMgr::pf3PartMap<mapT>::exists(const ST::string & key) const
{
    ST::string age, set, name;
    ISplitString(key, age, set, name);
    if (age.empty() || set.empty() || name.empty()) // if any are missing, it's invalid, so we don't have it
        return false;

    // now check individually
    auto curAge = fData.find(age);
    if (curAge == fData.end()) // age doesn't exist
        return false;
    auto curSet = curAge->second.find(set);
    if (curSet == curAge->second.end()) // set doesn't exist
        return false;
    auto curElement = curSet->second.find(name);
    if (curElement == curSet->second.end()) // name doesn't exist
        return false;

    // we passed all the tests, return true!
    return true;
}

//// setExists() /////////////////////////////////////////////////////

template<class mapT>
bool pfLocalizationDataMgr::pf3PartMap<mapT>::setExists(const ST::string & key) const
{
    ST::string age, set, name;
    ISplitString(key, age, set, name);
    if (age.empty() || set.empty()) // if any are missing, it's invalid, so we don't have it (ignoring name)
        return false;

    // now check individually
    auto curAge = fData.find(age);
    if (curAge == fData.end()) // age doesn't exist
        return false;
    auto curSet = curAge->second.find(set);
    if (curSet == curAge->second.end()) // set doesn't exist
        return false;

    // we passed all the tests, return true!
    return true;
}

//// erase() /////////////////////////////////////////////////////////

template<class mapT>
void pfLocalizationDataMgr::pf3PartMap<mapT>::erase(const ST::string & key)
{
    ST::string age, set, name;
    ISplitString(key, age, set, name);
    if (age.empty() || set.empty() || name.empty()) // if any are missing, it's invalid, so we don't delete it
        return;

    // now check individually
    auto curAge = fData.find(age);
    if (curAge == fData.end()) // age doesn't exist
        return;
    auto curSet = curAge->second.find(set);
    if (curSet == curAge->second.end()) // set doesn't exist
        return;
    auto curElement = curSet->second.find(name);
    if (curElement == curSet->second.end()) // name doesn't exist
        return;

    // ok, so now we want to nuke it!
    curSet->second.erase(name);
    if (curSet->second.size() == 0) // is the set now empty?
        curAge->second.erase(curSet); // nuke it!
    if (curAge->second.size() == 0) // is the age now empty?
        fData.erase(curAge); // nuke it!
}

//// operator[]() ////////////////////////////////////////////////////

template<class mapT>
const mapT& pfLocalizationDataMgr::pf3PartMap<mapT>::operator[](const ST::string& key) const
{
    ST::string age, set, name;
    ISplitString(key, age, set, name);
    return fData.at(age).at(set).at(name);
}

template<class mapT>
mapT &pfLocalizationDataMgr::pf3PartMap<mapT>::operator[](const ST::string &key)
{
    ST::string age, set, name;
    ISplitString(key, age, set, name);
    return fData[age][set][name];
}

//// getAgeList() ////////////////////////////////////////////////////

template<class mapT>
std::vector<ST::string> pfLocalizationDataMgr::pf3PartMap<mapT>::getAgeList() const
{
    std::vector<ST::string> retVal;

    for (const auto& curAge : fData)
        retVal.push_back(curAge.first);

    return retVal;
}

//// getSetList() ////////////////////////////////////////////////////

template<class mapT>
std::vector<ST::string> pfLocalizationDataMgr::pf3PartMap<mapT>::getSetList(const ST::string & age) const
{
    std::vector<ST::string> retVal;

    auto curAge = fData.find(age);
    if (curAge == fData.end())
        return retVal; // return an empty list, the age doesn't exist

    for (const auto& curSet : curAge->second)
        retVal.push_back(curSet.first);

    return retVal;
}

//// getNameList() ///////////////////////////////////////////////////

template<class mapT>
std::vector<ST::string> pfLocalizationDataMgr::pf3PartMap<mapT>::getNameList(const ST::string & age, const ST::string & set) const
{
    std::vector<ST::string> retVal;

    auto curAge = fData.find(age);
    if (curAge == fData.end())
        return retVal; // return an empty list, the age doesn't exist

    auto curSet = curAge->second.find(set);
    if (curSet == curAge->second.end())
        return retVal; // return an empty list, the set doesn't exist

    for (const auto& curName : curSet->second)
        retVal.push_back(curName.first);

    return retVal;
}

//////////////////////////////////////////////////////////////////////
//// pfLocalizationDataMgr Functions /////////////////////////////////
//////////////////////////////////////////////////////////////////////

pfLocalizationDataMgr   *pfLocalizationDataMgr::fInstance = nullptr;
plStatusLog             *pfLocalizationDataMgr::fLog = nullptr; // output logfile

//// Constructor/Destructor //////////////////////////////////////////

pfLocalizationDataMgr::pfLocalizationDataMgr(const plFileName & path)
{
    hsAssert(!fInstance, "Tried to create the localization data manager more than once!");
    fInstance = this;

    fDataPath = path;

    fDatabase = nullptr;
}

pfLocalizationDataMgr::~pfLocalizationDataMgr()
{
    fInstance = nullptr;

    if (fDatabase)
    {
        delete fDatabase;
        fDatabase = nullptr;
    }
}

//// IGetCurrentLanguageName /////////////////////////////////////////

ST::string pfLocalizationDataMgr::IGetCurrentLanguageName() const
{
    return plLocalization::GetLanguageName(plLocalization::GetLanguage());
}

//// IConvertSubtitle ////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertElement(const pfLocalizationDataMgr::element& elementInfo, const ST::string & curPath)
{
    pfLocalizationDataMgr::localizedElement newElement;
    int16_t numArgs = -1;

    for (const auto& curTranslation : elementInfo)
    {
        newElement[curTranslation.first].FromXML(curTranslation.second);
        uint16_t argCount = newElement[curTranslation.first].GetArgumentCount();
        if (numArgs == -1) // just started
            numArgs = argCount;
        else if (argCount != numArgs)
            fLog->AddLineF("WARNING: Argument number mismatch in element {} for {}", curPath, curTranslation.first);
    }

    fLocalizedElements[curPath] = newElement;
}

//// IConvertSet /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertSet(const pfLocalizationDataMgr::set& setInfo, const ST::string & curPath)
{
    for (const auto& curElement : setInfo)
        IConvertElement(curElement.second, ST::format("{}.{}", curPath, curElement.first));
}

//// IConvertAge /////////////////////////////////////////////////////

void pfLocalizationDataMgr::IConvertAge(const LocalizationXMLFile::age& ageInfo, const ST::string & curPath)
{
    for (const auto& curSet : ageInfo)
        IConvertSet(curSet.second, ST::format("{}.{}", curPath, curSet.first));
}

//// IWriteText //////////////////////////////////////////////////////

void pfLocalizationDataMgr::IWriteText(const plFileName & filename, const ST::string & ageName, const ST::string & languageName) const
{
    bool weWroteData = false; // did we actually write any data of consequence?
    bool setEmpty = true;

    // we will try to pretty print it all so it's easy to read for the devs
    ST::string_stream fileData;
    fileData << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
    fileData << "<localizations>\n";
    fileData << "\t<age name=\"" << ageName << "\">\n";

    std::vector<ST::string> setNames = GetSetList(ageName);
    for (const auto& setName : setNames)
    {
        setEmpty = true; // so far, this set is empty
        ST::string_stream setCode;
        setCode << "\t\t<set name=\"" << setName << "\">\n";

        std::vector<ST::string> elementNames = GetElementList(ageName, setName);
        for (const auto& elementName : elementNames)
        {
            setCode << "\t\t\t<element name=\"" << elementName << "\">\n";
            ST::string key = ST::format("{}.{}.{}", ageName, setName, elementName);

            auto findIt = fLocalizedElements[key].find(languageName);
            if (findIt != fLocalizedElements[key].end())
            {
                weWroteData = true;
                setEmpty = false;
                setCode << "\t\t\t\t<translation language=\"" << languageName << "\">";
                setCode << findIt->second.ToXML();
                setCode << "</translation>\n";
            }

            setCode << "\t\t\t</element>\n";
        }

        setCode << "\t\t</set>\n";

        if (!setEmpty)
            fileData << setCode.to_string();
    }

    fileData << "\t</age>\n";
    fileData << "</localizations>\n";

    if (weWroteData)
    {
        // now spit the results out to the file
        hsStream *xmlStream = plEncryptedStream::OpenEncryptedFileWrite(filename);
        xmlStream->Write(fileData.size(), fileData.raw_buffer());
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
    if (fLog != nullptr)
    {
        delete fLog;
        fLog = nullptr;
    }

    if (fInstance)
    {
        delete fInstance;
        fInstance = nullptr;
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
    for (const auto& curAge : fDatabase->GetData())
        IConvertAge(curAge.second, curAge.first);

    OutputTreeToLog();
}

//// GetElement //////////////////////////////////////////////////////

pfLocalizedString pfLocalizationDataMgr::GetElement(const ST::string & name) const
{
    if (!fLocalizedElements.exists(name)) // does the requested element exist?
        return {}; // nope, so return failure

    auto currLangIt = fLocalizedElements[name].find(IGetCurrentLanguageName());
    if (currLangIt != fLocalizedElements[name].cend())
        return currLangIt->second;

    // Force to English
    auto englishIt = fLocalizedElements[name].find("English");
    if (englishIt != fLocalizedElements[name].cend())
        return englishIt->second;

    return {};
}

//// GetSpecificElement //////////////////////////////////////////////

pfLocalizedString pfLocalizationDataMgr::GetSpecificElement(const ST::string & name, const ST::string & language) const
{
    if (!fLocalizedElements.exists(name)) // does the requested subtitle exist?
        return {}; // nope, so return failure

    auto findIt = fLocalizedElements[name].find(language);
    if (findIt == fLocalizedElements[name].cend())
        return {}; // language doesn't exist

    return findIt->second;
}

//// GetLanguages ////////////////////////////////////////////////////

std::vector<ST::string> pfLocalizationDataMgr::GetLanguages(const ST::string & ageName, const ST::string & setName, const ST::string & elementName) const
{
    std::vector<ST::string> retVal;
    ST::string key = ST::format("{}.{}.{}", ageName, setName, elementName);
    if (fLocalizedElements.exists(key))
    {
        // age, set, and element exists
        const auto& elem = fLocalizedElements[key];
        for (const auto& curLanguage : elem)
        {
            if (!curLanguage.first.empty()) // somehow blank language names sneak in... so don't return them
                retVal.push_back(curLanguage.first);
        }
    }
    return retVal;
}

//// GetElementXMLData ///////////////////////////////////////////////

ST::string pfLocalizationDataMgr::GetElementXMLData(const ST::string & name, const ST::string & languageName) const
{
    if (fLocalizedElements.exists(name)) {
        auto findIt = fLocalizedElements[name].find(languageName);
        if (findIt != fLocalizedElements[name].cend())
            return findIt->second.ToXML();
    }
    return "";
}

//// GetElementPlainTextData /////////////////////////////////////////

ST::string pfLocalizationDataMgr::GetElementPlainTextData(const ST::string & name, const ST::string & languageName) const
{
    if (fLocalizedElements.exists(name)) {
        auto findIt = fLocalizedElements[name].find(languageName);
        if (findIt != fLocalizedElements[name].cend())
            return findIt->second;
    }
    return "";
}

//// SetElementXMLData ///////////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementXMLData(const ST::string & name, const ST::string & languageName, const ST::string & xmlData)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName].FromXML(xmlData);
    return true;
}

//// SetElementPlainTextData /////////////////////////////////////////

bool pfLocalizationDataMgr::SetElementPlainTextData(const ST::string & name, const ST::string & languageName, const ST::string & plainText)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    fLocalizedElements[name][languageName] = plainText;
    return true;
}

//// AddLocalization /////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddLocalization(const ST::string & name, const ST::string & newLanguage)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // copy the english over so it can be localized
    fLocalizedElements[name][newLanguage] = fLocalizedElements[name]["English"];
    return true;
}

//// AddElement //////////////////////////////////////////////////////

bool pfLocalizationDataMgr::AddElement(const ST::string & name)
{
    if (fLocalizedElements.exists(name))
        return false; // already exists

    pfLocalizedString newElement;
    fLocalizedElements[name]["English"] = newElement;
    return true;
}

//// DeleteLocalization //////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteLocalization(const ST::string & name, const ST::string & languageName)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    if (fLocalizedElements[name].find(languageName) == fLocalizedElements[name].end())
        return false; // doesn't exist

    fLocalizedElements[name].erase(languageName);
    return true;
}

//// DeleteElement ///////////////////////////////////////////////////

bool pfLocalizationDataMgr::DeleteElement(const ST::string & name)
{
    if (!fLocalizedElements.exists(name))
        return false; // doesn't exist

    // delete it!
    fLocalizedElements.erase(name);
    return true;
}

//// WriteDatabaseToDisk /////////////////////////////////////////////

void pfLocalizationDataMgr::WriteDatabaseToDisk(const plFileName & path) const
{
    std::vector<ST::string> ageNames = GetAgeList();
    for (const auto& curAge : ageNames)
    {
        for (const auto& curLanguage : plLocalization::GetAllLanguageNames())
        {
            plFileName locPath = plFileName::Join(path, ST::format("{}{}.loc",
                                    curAge, curLanguage));
            IWriteText(locPath, curAge, curLanguage);
        }
    }
}

//// OutputTreeToLog /////////////////////////////////////////////////

void pfLocalizationDataMgr::OutputTreeToLog() const
{
    std::vector<ST::string> ages = GetAgeList();

    fLog->AddLine("\n");
    fLog->AddLine("Localization tree:\n");

    for (const auto& age : ages)
    {
        fLog->AddLineF("\t{}", age);

        std::vector<ST::string> sets = GetSetList(age);
        for (const auto& set : sets)
        {
            fLog->AddLineF("\t\t{}", set);

            std::vector<ST::string> names = GetElementList(age, set);
            for (const auto& name : names)
            {
                fLog->AddLineF("\t\t\t{}", name);
            }
        }
    }
}
