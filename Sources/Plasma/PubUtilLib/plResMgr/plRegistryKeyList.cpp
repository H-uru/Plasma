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
#include "plRegistryKeyList.h"
#include "plRegistryHelpers.h"
#include "hsStream.h"
#include <algorithm>

plRegistryKeyList::plRegistryKeyList(UInt16 classType)
{
	fClassType = classType;
	fReffedStaticKeys = 0;
	fLocked = 0;
	fFlags = 0;
}

plRegistryKeyList::~plRegistryKeyList()
{
	hsAssert(fLocked == 0, "Key list still locked on delete");

	for (int i = 0; i < fStaticKeys.size(); i++)
	{
		plKeyImp* keyImp = fStaticKeys[i];
		if (!keyImp->ObjectIsLoaded())
			delete keyImp;
	}
}

// Special dummy key that lets us set the return value of the GetName call.
// Makes it easier to do STL searches.
class plSearchKeyImp : public plKeyImp
{
public:
	const char* fSearchKeyName;
	const char* GetName() const { return fSearchKeyName; }
};

plKeyImp* plRegistryKeyList::FindKey(const char* keyName)
{
	static plSearchKeyImp searchKey;
	searchKey.fSearchKeyName = keyName;

	// Search the static key list
	if (fFlags & kStaticUnsorted)
	{
		// We're unsorted, brute force it.  May do a separate search table in the
		// future if this is a bottlneck
		for (int i = 0; i < fStaticKeys.size(); i++)
		{
			plKeyImp* curKey = fStaticKeys[i];
			if (curKey && hsStrCaseEQ(keyName, curKey->GetName()))
				return curKey;
		}
	}
	else
	{
		// We're sorted, do a fast lookup
		StaticVec::const_iterator it = std::lower_bound(fStaticKeys.begin(), fStaticKeys.end(), &searchKey, KeySorter());
		if (it != fStaticKeys.end() && hsStrCaseEQ(keyName, (*it)->GetName()))
			return *it;
	}

	// Search the dynamic key list
	DynSet::const_iterator dynIt = fDynamicKeys.find(&searchKey);
	if (dynIt != fDynamicKeys.end())
		return *dynIt;

	return nil;
}

plKeyImp* plRegistryKeyList::FindKey(const plUoid& uoid)
{
	UInt32 objectID = uoid.GetObjectID();

	// Key is dynamic or doesn't know it's index.  Do a find by name.
	if (objectID == 0)
		return FindKey(uoid.GetObjectName());

	// Direct lookup
	if (objectID <= fStaticKeys.size())
	{
#ifdef PLASMA_EXTERNAL_RELEASE
		return fStaticKeys[objectID-1];
#else
		// If this is an internal release, our objectIDs might not match
		// because of local data. Verify that we have the right key by
		// name, and if it's wrong, do the slower find-by-name.
		plKeyImp *keyImp = fStaticKeys[objectID-1];
		if (!hsStrCaseEQ(keyImp->GetName(), uoid.GetObjectName()))
			return FindKey(uoid.GetObjectName());
		else
			return keyImp;
#endif // PLASMA_EXTERNAL_RELEASE
	}

	// If we got here it probably means we just deleted all our keys of the matching type
	// because no one was using them. No worries. The resManager will catch this and 
	// reload our keys, then try again.

	return nil;
}

void plRegistryKeyList::ILock()
{
	fLocked++;
}

void plRegistryKeyList::IUnlock()
{
	fLocked--;
	if (fLocked == 0)
		IRepack();
}

bool plRegistryKeyList::IterateKeys(plRegistryKeyIterator* iterator)
{
	ILock();

	for (int i = 0; i < fStaticKeys.size(); i++)
	{
		plKeyImp* keyImp = fStaticKeys[i];
		if (keyImp != nil)
		{
			if (!iterator->EatKey(plKey::Make(keyImp)))
			{
				IUnlock();
				return false;
			}
		}
	}

	DynSet::const_iterator it;
	for (it = fDynamicKeys.begin(); it != fDynamicKeys.end(); it++)
	{
		plKeyImp* keyImp = *it;
		hsAssert(keyImp, "Shouldn't ever have a nil dynamic key");
		if (!iterator->EatKey(plKey::Make(keyImp)))
		{
			IUnlock();
			return false;
		}
	}

	IUnlock();
	return true;
}

void plRegistryKeyList::AddKey(plKeyImp* key, LoadStatus& loadStatusChange)
{
	loadStatusChange = kNoChange;

	hsAssert(fLocked == 0, "Don't currently support adding keys while locked");
	if (fLocked == 0 && key != nil)
	{
		// If this is the first key added, we just became loaded
		if (fDynamicKeys.empty())
			loadStatusChange = kDynLoaded;

		hsAssert(fDynamicKeys.find(key) == fDynamicKeys.end(), "Key already added");
		fDynamicKeys.insert(key);
	}
}

void plRegistryKeyList::SetKeyUsed(plKeyImp* key)
{
	// If this is a static key, mark that we used it.  Otherwise, just ignore it.
	UInt32 id = key->GetUoid().GetObjectID();
	if (id > 0)
		fReffedStaticKeys++;
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

	// Check if it's a static key
	UInt32 id = key->GetUoid().GetObjectID();
	hsAssert(id <= fStaticKeys.size(), "Bad static key id");
	if (id != 0 && id <= fStaticKeys.size())
	{
		fReffedStaticKeys--;
		if (fLocked == 0)
			IRepack();

		// That was our last used static key, we're static unloaded
		if (fReffedStaticKeys == 0)
			loadStatusChange = kStaticUnloaded;

		return true;
	}

	// Try to find it in the dynamic key list
	DynSet::iterator dynIt = fDynamicKeys.find(key);
	if (dynIt != fDynamicKeys.end())
	{
		hsAssert(fLocked == 0, "Don't currently support removing dynamic keys while locked");
		if (fLocked == 0)
		{
			fDynamicKeys.erase(dynIt);
			delete key;

			// That was our last dynamic key, notify of dynamic unloaded
			if (fDynamicKeys.empty())
				loadStatusChange = kDynUnloaded;

			return true;
		}
		return false;
	}

	hsAssert(0, "Couldn't find this key, what is it?");
	return false;
}

//// IRepack /////////////////////////////////////////////////////////////////
//	Frees the memory for our static key array if none of them are loaded
void plRegistryKeyList::IRepack()
{
	if (fReffedStaticKeys == 0 && !fStaticKeys.empty())
	{

		for (int i = 0; i < fStaticKeys.size(); i++)
			delete fStaticKeys[i];

		fStaticKeys.clear();
	}
}

void plRegistryKeyList::PrepForWrite()
{
	// If we have any static keys already, we were read in.  To keep from
	// invalidating old key indexes any new keys have to go on the end, hence we're
	// unsorted now.
	if (!fStaticKeys.empty())
		fFlags |= kStaticUnsorted;

	// If a dynamic keys doesn't have an object assigned to it, we're not writing
	// it out.  Figure out how many valid keys we have.
	int numDynKeys = 0;
	DynSet::const_iterator cIt;
	for (cIt = fDynamicKeys.begin(); cIt != fDynamicKeys.end(); cIt++)
	{
		plKeyImp* key = *cIt;
		// We're only going to write out keys that have objects
		if (key->ObjectIsLoaded())
			numDynKeys++;
	}

	// Start our new object id's after any already created ones
	UInt32 objectID = fStaticKeys.size()+1;
	// Make room for our new keys
	fStaticKeys.resize(fStaticKeys.size()+numDynKeys);

	DynSet::iterator it = fDynamicKeys.begin();
	while (it != fDynamicKeys.end())
	{
		plKeyImp* key = *it;
		it++;

		// If we're gonna use this key, tag it with it's object id and move it to the static array.
		if (key->ObjectIsLoaded())
		{
			key->SetObjectID(objectID);
			fStaticKeys[objectID-1] = key;

			objectID++;
			fReffedStaticKeys++;
			fDynamicKeys.erase(key);
		}
	}
}

void plRegistryKeyList::Read(hsStream* s)
{
	UInt32 keyListLen = s->ReadSwap32();
	if (!fStaticKeys.empty())
	{
		s->Skip(keyListLen);
		return;
	}

	fFlags = s->ReadByte();

	UInt32 numKeys = s->ReadSwap32();
	fStaticKeys.resize(numKeys);

	for (int i = 0; i < numKeys; i++)
	{
		plKeyImp* newKey = TRACKED_NEW plKeyImp;
		newKey->Read(s);
		fStaticKeys[i] = newKey;
	}
}

void plRegistryKeyList::Write(hsStream* s)
{
	// Save space for the length of our data
	UInt32 beginPos = s->GetPosition();
	s->WriteSwap32(0);
	s->WriteByte(fFlags);

	int numKeys = fStaticKeys.size();
	s->WriteSwap32(numKeys);

	// Write out all our keys (anything in dynamic is unused, so just ignore those)
	for (int i = 0; i < numKeys; i++)
	{
		plKeyImp* key = fStaticKeys[i];
		key->Write(s);
	}

	// Go back to the start and write the length of our data
	UInt32 endPos = s->GetPosition();
	s->SetPosition(beginPos);
	s->WriteSwap32(endPos-beginPos-sizeof(UInt32));
	s->SetPosition(endPos);
}
