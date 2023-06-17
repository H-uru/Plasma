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

#include "plRegistryKeyList.h"
#include "plRegistryHelpers.h"

#include <algorithm>
#include <string_theory/string>

#include "HeadSpin.h"
#include "hsStream.h"

#include "pnKeyedObject/plKeyImp.h"

#include "plStatusLog/plStatusLog.h"


plRegistryKeyList::~plRegistryKeyList()
{
    for (auto& key : fKeys) {
        if (key && !key->ObjectIsLoaded()) {
            delete key;
            key = nullptr;
        }
    }
}

plKeyImp* plRegistryKeyList::FindKey(const ST::string& keyName) const
{
    auto it = std::find_if(fKeys.begin(), fKeys.end(),
        [&] (plKeyImp* key) { return key && key->GetName().compare_i(keyName) == 0; }
    );
    if (it != fKeys.end())
        return *it;
    else
        return nullptr;
}

plKeyImp* plRegistryKeyList::FindKey(const plUoid& uoid) const
{
    uint32_t objectID = uoid.GetObjectID();

    // Key is dynamic or doesn't know its index.  Do a find by name.
    if (objectID == 0)
        return FindKey(uoid.GetObjectName());

    // Direct lookup
    if (objectID <= fKeys.size())
    {
        plKeyImp* keyImp = fKeys[objectID - 1];
#ifndef PLASMA_EXTERNAL_RELEASE
        if (!keyImp)
            plStatusLog::AddLineSF("resources.log", "FindKey: NULL KeyImp for Uoid {}", uoid);
        if (keyImp && keyImp->GetName().compare_i(uoid.GetObjectName()) != 0) {
            plStatusLog::AddLineSF(
                "resources.log",
                "FindKey: Uoid objID mismatch\n\tRequested: {}\n\tGot: {}",
                uoid,
                keyImp->GetUoid()
            );
        }
#endif
        return keyImp;
    }

    // If we got here it probably means we just deleted all our keys of the matching type
    // because no one was using them. No worries. The resManager will catch this and 
    // reload our keys, then try again.

    return nullptr;
}

bool plRegistryKeyList::IterateKeys(plRegistryKeyIterator* iterator)
{
    ILock();

    for (auto it = fKeys.begin(); it != fKeys.end(); ++it)
    {
        plKeyImp* keyImp = *it;
        if (keyImp)
        {
            if (!iterator->EatKey(plKey::Make(keyImp)))
            {
                IUnlock();
                return false;
            }
        }
    }

    IUnlock();
    return true;
}

void plRegistryKeyList::AddKey(plKeyImp* key, LoadStatus& loadStatusChange)
{
    loadStatusChange = kNoChange;

    hsAssert(fLocked == 0, "Don't currently support adding keys while locked");
    if (fLocked == 0 && key)
    {
        hsAssert(std::find(fKeys.begin(), fKeys.end(), key) == fKeys.end(), "Key already added");

        // first key to be added?
        if (fKeys.empty())
            loadStatusChange = kTypeLoaded;

        // Objects that already have an object ID will be respected.
        // Totally new keys will not have one, but keys from other sources (patches) will.
        if (key->GetUoid().GetObjectID() == 0)
        {
            fKeys.push_back(key);
            key->SetObjectID(fKeys.size());
        }
        else
        {
            uint32_t id = key->GetUoid().GetObjectID();
            if (fKeys.size() < id)
                fKeys.resize(id);
            fKeys[id - 1] = key;
        }
        ++fReffedKeys;
    }
}

bool plRegistryKeyList::SetKeyUnused(plKeyImp* key, LoadStatus& loadStatusChange)
{
    loadStatusChange = kNoChange;

    // Clones never officially get added to the key list (they're maintained by
    // the original key), so just ignore them
    if (key->GetUoid().IsClone())
    {
        delete key;
        return true;
    }

    uint32_t id = key->GetUoid().GetObjectID();
    plKeyImp* foundKey = nullptr;

    // Fixed Keys use ID == 0
    if (id == 0)
        hsAssert(key->GetUoid().GetLocation() == plLocation::kGlobalFixedLoc, "key id == 0 but not fixed?");

    // Recall that vectors are index zero but normal object IDs are index one...
    else if (id <= fKeys.size()) {
        plKeyImp* tempKey = fKeys[id-1];
        if (tempKey && tempKey->GetUoid().GetObjectID() == id)
            foundKey = tempKey;
    }

    // Last chance: do a slow name search for that key.
    if (!foundKey)
        foundKey = FindKey(key->GetUoid().GetObjectName());

    // Got that key, decrement the key counter
    if (foundKey) {
        --fReffedKeys;
        if (fReffedKeys == 0)
            loadStatusChange = kTypeUnloaded;
    }
    return foundKey != nullptr;
}

void plRegistryKeyList::Read(hsStream* s)
{
    uint32_t keyListLen = s->ReadLE32();
    if (!fKeys.empty())
    {
        s->Skip(keyListLen);
        return;
    }

    // deprecated flags. used to indicate alphabetically sorted keys for some "optimization"
    // that really appeared to do nothing. no loss.
    (void)s->ReadByte();

    uint32_t numKeys = s->ReadLE32();
    fKeys.reserve((numKeys * 3) / 2);

    for (uint32_t i = 0; i < numKeys; ++i)
    {
        plKeyImp* newKey = new plKeyImp;
        newKey->Read(s);

        uint32_t id = newKey->GetUoid().GetObjectID();
        if (fKeys.size() < id)
            fKeys.resize(id);
        fKeys[id - 1] = newKey;
    }
    fKeys.shrink_to_fit();
}

void plRegistryKeyList::Write(hsStream* s)
{
    // Save space for the length of our data
    uint32_t beginPos = s->GetPosition();
    s->WriteLE32(0);
    s->WriteByte(uint8_t(0)); // Deprecated flags

    // We only write out keys with data. Fill this value in later...
    uint32_t countPos = s->GetPosition();
    s->WriteLE32(0);

    // Write out all our keys with data
    uint32_t keyCount = 0;
    for (auto it = fKeys.begin(); it != fKeys.end(); ++it)
    {
        plKeyImp* key = *it;
        if (key && key->ObjectIsLoaded())
        {
            ++keyCount;
            key->Write(s);
        }
    }

    // Rewind and write out data size and key count
    uint32_t endPos = s->GetPosition();
    s->SetPosition(beginPos);
    uint32_t objSize = endPos - beginPos - sizeof(uint32_t);
    s->WriteLE32(objSize);
    s->SetPosition(countPos);
    s->WriteLE32(keyCount);
    s->SetPosition(endPos);
}
