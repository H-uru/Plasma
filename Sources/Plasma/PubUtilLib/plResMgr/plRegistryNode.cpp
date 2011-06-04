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
#include "plRegistryNode.h"
#include "plRegistryKeyList.h"
#include "plRegistryHelpers.h"

#include "../pnKeyedObject/plKeyImp.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnFactory/plFactory.h"
#include "../plFile/plFileUtils.h"
#include "hsStlUtils.h"

#include "plVersion.h"

plRegistryPageNode::plRegistryPageNode(const char* path)
	: fValid(kPageCorrupt)
	, fPath(nil)
	, fDynLoadedTypes(0)
	, fStaticLoadedTypes(0)
	, fOpenRequests(0)
	, fIsNewPage(false)
{
	fPath = hsStrcpy(path);

	hsStream* stream = OpenStream();
	if (stream)
	{
		fPageInfo.Read(&fStream);
		fValid = IVerify();
		CloseStream();
	}
}

plRegistryPageNode::plRegistryPageNode(const plLocation& location, const char* age, const char* page, const char* dataPath)
	: fValid(kPageOk)
	, fPath(nil)
	, fPageInfo(location)
	, fDynLoadedTypes(0)
	, fStaticLoadedTypes(0)
	, fOpenRequests(0)
	, fIsNewPage(true)
{
	fPageInfo.SetStrings(age, page);

	char filePath[512];

	// Copy the path over
	strncpy(filePath, dataPath, sizeof(filePath));
	plFileUtils::AddSlash(filePath);

	// Time to construct our actual file name. For now, we'll use the same old format
	// of age_page.extension
	strncat(filePath, fPageInfo.GetAge(), sizeof(filePath));
	strncat(filePath, "_District_", sizeof(filePath));
	strncat(filePath, fPageInfo.GetPage(), sizeof(filePath));
	strncat(filePath, ".prp", sizeof(filePath));

	fPath = hsStrcpy(filePath);
}

plRegistryPageNode::~plRegistryPageNode()
{
	delete [] fPath;
	UnloadKeys();
}

PageCond plRegistryPageNode::IVerify()
{
	// Check the checksum values first, to make sure the files aren't corrupt
	UInt32 ourChecksum = 0;
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
	for (int i = 0; i < classVersions.size(); i++)
	{
		const plPageInfo::ClassVersion& cv = classVersions[i];
		UInt16 curVersion = plVersion::GetCreatableVersion(cv.Class);

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
		if (!fStream.Open(fPath, "rb"))
			return nil;
	}
	fOpenRequests++;
	return &fStream;
}

void plRegistryPageNode::CloseStream()
{
	if (fOpenRequests > 0)
		fOpenRequests--;

	if (fOpenRequests == 0)
		fStream.Close();
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
		hsAssert(0, xtl::format("plRegistryPageNode::LoadKeysFromSource - bad stream %s,%s", 
			GetPageInfo().GetAge(), GetPageInfo().GetPage()).c_str());
		return;
	}

	// If we're loading keys in the middle of a read because FindKey() failed, we'd better
	// make note of our stream position and restore it when we're done.
	UInt32 oldPos = stream->GetPosition();
	stream->SetPosition(GetPageInfo().GetIndexStart());

	// Read in the number of key types
	UInt32 numTypes = stream->ReadSwap32();
	for (UInt32 i = 0; i < numTypes; i++)
	{
		UInt16 classType = stream->ReadSwap16();
		plRegistryKeyList* keyList = IGetKeyList(classType);
		if (!keyList)
		{
			keyList = TRACKED_NEW plRegistryKeyList(classType);
			fKeyLists[classType] = keyList;
		}
		keyList->Read(stream);
	}

	stream->SetPosition(oldPos);
	CloseStream();
	fStaticLoadedTypes = fKeyLists.size();
}

void plRegistryPageNode::UnloadKeys()
{
	KeyMap::iterator it = fKeyLists.begin();
	for (; it != fKeyLists.end(); it++)
	{
		plRegistryKeyList* keyList = it->second;
		delete keyList;
	}
	fKeyLists.clear();

	fDynLoadedTypes = 0;
	fStaticLoadedTypes = 0;
}

//// plWriteIterator /////////////////////////////////////////////////////////
//	Key iterator for writing objects
class plWriteIterator : public plRegistryKeyIterator
{
protected:
	hsStream* fStream;

public:
	plWriteIterator(hsStream* s) : fStream(s) {}

	virtual hsBool EatKey(const plKey& key)
	{
		plKeyImp* imp = (plKeyImp*)key;
		imp->WriteObject(fStream);
		return true;
	}
};

void plRegistryPageNode::Write()
{
	hsAssert(fOpenRequests == 0, "Trying to write while the page is open for reading");

	if (!fStream.Open(fPath, "wb"))
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
		keyList->PrepForWrite();

		int ver = plVersion::GetCreatableVersion(keyList->GetClassType());
		fPageInfo.AddClassVersion(keyList->GetClassType(), ver);
	}

	// First thing we write is the pageinfo.  Later we'll rewind and overwrite this with the final values
	fPageInfo.Write(&fStream);

	fPageInfo.SetDataStart(fStream.GetPosition());

	// Write all our objects
	plWriteIterator	writer(&fStream);
	IterateKeys(&writer);

	fPageInfo.SetIndexStart(fStream.GetPosition());

	// Write our keys
	fStream.WriteSwap32(fKeyLists.size());
	for (it = fKeyLists.begin(); it != fKeyLists.end(); it++)
	{
		plRegistryKeyList* keyList = it->second;
		fStream.WriteSwap16(keyList->GetClassType());
		keyList->Write(&fStream);
	}

	// Rewind and write the pageinfo with the correct data and index offsets
	fStream.Rewind();
	fPageInfo.SetChecksum(fStream.GetEOF() - fPageInfo.GetDataStart());
	fPageInfo.Write(&fStream);

	fStream.Close();
}

//// IterateKeys /////////////////////////////////////////////////////////////

hsBool plRegistryPageNode::IterateKeys(plRegistryKeyIterator* iterator) const
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
//	Restricted version that only iterates through the keys of a given class
//	type.

hsBool plRegistryPageNode::IterateKeys(plRegistryKeyIterator* iterator, UInt16 classToRestrictTo) const
{
	plRegistryKeyList* keyList = IGetKeyList(classToRestrictTo);
	if (keyList != nil)
		return keyList->IterateKeys(iterator);

	return true;
}

plKeyImp* plRegistryPageNode::FindKey(UInt16 classType, const char* name) const
{
	plRegistryKeyList* keys = IGetKeyList(classType);
	if (keys == nil)
		return nil;

	return keys->FindKey(name);
}

plKeyImp* plRegistryPageNode::FindKey(const plUoid& uoid) const
{
	plRegistryKeyList* keys = IGetKeyList(uoid.GetClassType());
	if (keys == nil)
		return nil;

	return keys->FindKey(uoid);
}

void plRegistryPageNode::AddKey(plKeyImp* key)
{
	UInt16 classType = key->GetUoid().GetClassType();
	plRegistryKeyList* keys = fKeyLists[classType];
	if (keys == nil)
	{
		keys = TRACKED_NEW plRegistryKeyList(classType);
		fKeyLists[classType] = keys;
	}

	// Error check
	if (keys->FindKey(key->GetUoid().GetObjectName()) != nil)
	{
		//char str[512], tempStr[128];
		//sprintf(str, "Attempting to add a key with a duplicate name. Not allowed."
		//			"\n\n(Key name: %s, Class: %s, Loc: %s)", key->GetUoid().GetObjectName(), 
		//			plFactory::GetNameOfClass(classType), key->GetUoid().GetLocation().StringIze(tempStr));
		//hsStatusMessage(str);
		hsBool recovered = false;

		// Attempt recovery
		for (int i = 0; i < 500; i++)
		{
			char tempName[512];
			sprintf(tempName, "%s%d", key->GetUoid().GetObjectName(), i);
			if (keys->FindKey(tempName) == nil)
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

	if (loadStatusChange == plRegistryKeyList::kDynLoaded)
		fDynLoadedTypes++;
}

void plRegistryPageNode::SetKeyUsed(plKeyImp* key)
{
	plRegistryKeyList* keys = IGetKeyList(key->GetUoid().GetClassType());
	if (keys == nil)
		return;

	keys->SetKeyUsed(key);
}

hsBool plRegistryPageNode::SetKeyUnused(plKeyImp* key)
{
	plRegistryKeyList* keys = IGetKeyList(key->GetUoid().GetClassType());
	if (keys == nil)
		return false;

	plRegistryKeyList::LoadStatus loadStatusChange;
	hsBool removed = keys->SetKeyUnused(key, loadStatusChange);

	// If the key type just changed load status, update our load counts
	if (loadStatusChange == plRegistryKeyList::kDynUnloaded)
		fDynLoadedTypes--;
	else if (loadStatusChange == plRegistryKeyList::kStaticUnloaded)
		fStaticLoadedTypes--;

	return removed;
}

plRegistryKeyList* plRegistryPageNode::IGetKeyList(UInt16 classType) const
{
	KeyMap::const_iterator it = fKeyLists.find(classType);
	if (it != fKeyLists.end())
		return it->second;

	return nil;
}

void plRegistryPageNode::DeleteSource()
{
	hsAssert(fOpenRequests == 0, "Deleting a stream that's open for reading");
	plFileUtils::RemoveFile(fPath);
}

