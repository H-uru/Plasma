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

#include "plRegistryNode.h"
#include <string_theory/format>

#include "hsStream.h"
#include "plRegistryHelpers.h"
#include "plRegistryKeyList.h"
#include "plVersion.h"

#include "pnKeyedObject/plKeyImp.h"

plRegistryPageNode::plRegistryPageNode()
{}

plRegistryPageNode::plRegistryPageNode(const plFileName& path)
    : fValid(kPageCorrupt)
    , fPath(path)
    , fLoadedTypes(0)
    , fStream(nullptr)
    , fOpenRequests(0)
    , fIsNewPage(false)
{
    hsStream* stream = OpenStream();
    if (stream)
    {
        fPageInfo.Read(stream);
        fValid = IVerify();
        CloseStream();
    }
}

plRegistryPageNode::plRegistryPageNode(const plLocation& location, const ST::string& age,
                                       const ST::string& page, const plFileName& dataPath)
    : fValid(kPageOk)
    , fPageInfo(location)
    , fLoadedTypes(0)
    , fStream(nullptr)
    , fOpenRequests(0)
    , fIsNewPage(true)
{
    fPageInfo.SetStrings(age, page);

    // Time to construct our actual file name. For now, we'll use the same old format
    // of age_page.extension
    fPath = plFileName::Join(dataPath, ST::format("{}_District_{}.prp",
                fPageInfo.GetAge(), fPageInfo.GetPage()));
}

plRegistryPageNode::~plRegistryPageNode()
{
    UnloadKeys();
}

PageCond plRegistryPageNode::IVerify()
{
    // Check the checksum values first, to make sure the files aren't corrupt
    uint32_t ourChecksum = 0;
    hsStream* stream = OpenStream();
    if (stream)
    {
        ourChecksum = stream->GetEOF() - fPageInfo.GetDataStart();
        CloseStream();
    }
    if (ourChecksum != fPageInfo.GetChecksum())
        return kPageCorrupt;

    // If major version out-of-date, entire location is screwed
    if (fPageInfo.GetMajorVersion() > plVersion::GetMajorVersion())
        return kPageTooNew;
    else if (fPageInfo.GetMajorVersion() < plVersion::GetMajorVersion())
        return kPageOutOfDate;

    // Check the minor versions
    const plPageInfo::ClassVerVec& classVersions = fPageInfo.GetClassVersions();
    for (size_t i = 0; i < classVersions.size(); i++)
    {
        const plPageInfo::ClassVersion& cv = classVersions[i];
        uint16_t curVersion = plVersion::GetCreatableVersion(cv.Class);

        if (curVersion > cv.Version)
            return kPageOutOfDate;
        else if (curVersion < cv.Version)
            return kPageTooNew;
    }

    return kPageOk;
}

hsStream* plRegistryPageNode::OpenStream()
{
    if (fOpenRequests == 0)
    {
        hsAssert(fStream == nullptr, "plRegistryPageNode::fStream should be nullptr when not open!");
        auto stream = std::make_unique<hsBufferedStream>();
        if (!stream->Open(fPath, "rb")) {
            return nullptr;
        }
        fStream = std::move(stream);
    }
    fOpenRequests++;
    return fStream.get();
}

void plRegistryPageNode::CloseStream()
{
    if (fOpenRequests > 0)
        fOpenRequests--;

    if (fOpenRequests == 0) {
        fStream.reset();
    }
}

void plRegistryPageNode::LoadKeys()
{
    hsAssert(IsValid(), "Trying to load keys for invalid page");
    hsAssert(!fIsNewPage, "Trying to read a new page");
    if (IsFullyLoaded())
        return;

    hsStream* stream = OpenStream();
    if (!stream)
    {
        hsAssert(0, ST::format("plRegistryPageNode::LoadKeysFromSource - bad stream {},{}",
                               GetPageInfo().GetAge(), GetPageInfo().GetPage()).c_str());
        return;
    }

    // If we're loading keys in the middle of a read because FindKey() failed, we'd better
    // make note of our stream position and restore it when we're done.
    uint32_t oldPos = stream->GetPosition();
    stream->SetPosition(GetPageInfo().GetIndexStart());

    // Read in the number of key types
    uint32_t numTypes = stream->ReadLE32();
    for (uint32_t i = 0; i < numTypes; i++)
    {
        uint16_t classType = stream->ReadLE16();
        plRegistryKeyList* keyList = IGetKeyList(classType);
        if (!keyList)
        {
            keyList = new plRegistryKeyList(classType);
            fKeyLists[classType] = keyList;
        }
        keyList->Read(stream);
    }

    stream->SetPosition(oldPos);
    CloseStream();
    fLoadedTypes = fKeyLists.size();
}

void plRegistryPageNode::UnloadKeys()
{
    KeyMap::iterator it = fKeyLists.begin();
    for (; it != fKeyLists.end(); it++)
    {
        plRegistryKeyList* keyList = it->second;
        it->second = nullptr;
        delete keyList;
    }
    fKeyLists.clear();

    fLoadedTypes = 0;
}

void plRegistryPageNode::PrepForWrite()
{
    if (!fIsNewPage)
        return;

    for (auto [idx, keyList] : fKeyLists)
        keyList->PrepForWrite();
}

//// plWriteIterator /////////////////////////////////////////////////////////
//  Key iterator for writing objects
class plWriteIterator : public plRegistryKeyIterator
{
protected:
    hsStream* fStream;

public:
    plWriteIterator(hsStream* s) : fStream(s) {}

    bool EatKey(const plKey& key) override
    {
        plKeyImp::GetFromKey(key)->WriteObject(fStream);
        return true;
    }
};

void plRegistryPageNode::Write()
{
    hsAssert(fStream == nullptr, "Trying to write while the page is open for reading");
    hsAssert(fOpenRequests == 0, "Trying to write while the page is open for reading");

    hsBufferedStream stream;
    if (!stream.Open(fPath, "wb"))
    {
        hsAssert(0, "Couldn't open file for writing");
        return;
    }

    // Some prep stuff.  Assign object IDs for every key in this page, and put the
    // versions of all our creatable types in the pageinfo.
    fPageInfo.ClearClassVersions();

    KeyMap::const_iterator it;
    for (it = fKeyLists.begin(); it != fKeyLists.end(); it++)
    {
        plRegistryKeyList* keyList = it->second;
        int ver = plVersion::GetCreatableVersion(keyList->GetClassType());
        fPageInfo.AddClassVersion(keyList->GetClassType(), ver);
    }

    // First thing we write is the pageinfo.  Later we'll rewind and overwrite this with the final values
    fPageInfo.Write(&stream);

    fPageInfo.SetDataStart(stream.GetPosition());

    // Write all our objects
    plWriteIterator writer(&stream);
    IterateKeys(&writer);

    fPageInfo.SetIndexStart(stream.GetPosition());

    // Write our keys
    stream.WriteLE32((uint32_t)fKeyLists.size());
    for (it = fKeyLists.begin(); it != fKeyLists.end(); it++)
    {
        plRegistryKeyList* keyList = it->second;
        stream.WriteLE16(keyList->GetClassType());
        keyList->Write(&stream);
    }

    // Rewind and write the pageinfo with the correct data and index offsets
    stream.Rewind();
    fPageInfo.SetChecksum(stream.GetEOF() - fPageInfo.GetDataStart());
    fPageInfo.Write(&stream);
}

//// IterateKeys /////////////////////////////////////////////////////////////

bool plRegistryPageNode::IterateKeys(plRegistryKeyIterator* iterator) const
{
    KeyMap::const_iterator it = fKeyLists.begin();
    for (; it != fKeyLists.end(); it++)
    {
        plRegistryKeyList* keyList = it->second;
        if (!keyList->IterateKeys(iterator))
            return false;
    }

    return true;
}

//// IterateKeys /////////////////////////////////////////////////////////////
//  Restricted version that only iterates through the keys of a given class
//  type.

bool plRegistryPageNode::IterateKeys(plRegistryKeyIterator* iterator, uint16_t classToRestrictTo) const
{
    plRegistryKeyList* keyList = IGetKeyList(classToRestrictTo);
    if (keyList != nullptr)
        return keyList->IterateKeys(iterator);

    return true;
}

plKeyImp* plRegistryPageNode::FindKey(uint16_t classType, const ST::string& name) const
{
    plRegistryKeyList* keys = IGetKeyList(classType);
    if (keys == nullptr)
        return nullptr;

    return keys->FindKey(name);
}

plKeyImp* plRegistryPageNode::FindKey(const plUoid& uoid) const
{
    plRegistryKeyList* keys = IGetKeyList(uoid.GetClassType());
    if (keys == nullptr)
        return nullptr;

    return keys->FindKey(uoid);
}

void plRegistryPageNode::AddKey(plKeyImp* key)
{
    uint16_t classType = key->GetUoid().GetClassType();
    plRegistryKeyList* keys = fKeyLists[classType];
    if (keys == nullptr)
    {
        keys = new plRegistryKeyList(classType);
        fKeyLists[classType] = keys;
    }

    // Error check
    if (keys->FindKey(key->GetUoid().GetObjectName()) != nullptr)
    {
        //hsStatusMessageF("Attempting to add a key with a duplicate name. Not allowed."
        //          "\n\n(Key name: {}, Class: {}, Loc: {})", key->GetUoid().GetObjectName(), 
        //          plFactory::GetNameOfClass(classType), key->GetUoid().GetLocation());
        bool recovered = false;

        // Attempt recovery
        for (int i = 0; i < 500; i++)
        {
            ST::string tempName = ST::format("{}{}", key->GetUoid().GetObjectName(), i);
            if (keys->FindKey(tempName) == nullptr)
            {
                plUoid uoid(key->GetUoid().GetLocation(), key->GetUoid().GetClassType(), tempName, key->GetUoid().GetLoadMask());
                key->SetUoid(uoid);
                recovered = true;
                break;
            }
        }

        if (!recovered)
        {
            hsAssert(0, "Couldn't allocate a unique key");
            return;
        }
    }

    plRegistryKeyList::LoadStatus loadStatusChange;
    keys->AddKey(key, loadStatusChange);

    if (loadStatusChange == plRegistryKeyList::kTypeLoaded)
        ++fLoadedTypes;
}

void plRegistryPageNode::SetKeyUsed(plKeyImp* key)
{
    plRegistryKeyList* keys = IGetKeyList(key->GetUoid().GetClassType());
    if (keys == nullptr)
        return;

    keys->SetKeyUsed(key);
}

bool plRegistryPageNode::SetKeyUnused(plKeyImp* key)
{
    plRegistryKeyList* keys = IGetKeyList(key->GetUoid().GetClassType());
    if (keys == nullptr)
        return false;

    plRegistryKeyList::LoadStatus loadStatusChange;
    bool removed = keys->SetKeyUnused(key, loadStatusChange);

    // If the key type just changed load status, update our load counts
    if (loadStatusChange == plRegistryKeyList::kTypeUnloaded)
        --fLoadedTypes;

    return removed;
}

plRegistryKeyList* plRegistryPageNode::IGetKeyList(uint16_t classType) const
{
    KeyMap::const_iterator it = fKeyLists.find(classType);
    if (it != fKeyLists.end())
        return it->second;

    return nullptr;
}

void plRegistryPageNode::DeleteSource()
{
    hsAssert(fOpenRequests == 0, "Deleting a stream that's open for reading");
    plFileSystem::Unlink(fPath);
}
