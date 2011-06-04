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
#include "plPageOptimizer.h"

#include "../pnKeyedObject/plUoid.h"
#include "../plResMgr/plResManager.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plResMgr/plRegistryNode.h"

#include "../pnFactory/plFactory.h"
#include "../pnKeyedObject/plKeyImp.h"

#include "../plFile/plFileUtils.h"
#include "hsStream.h"

plPageOptimizer* plPageOptimizer::fInstance = nil;

plPageOptimizer::plPageOptimizer(const char* pagePath) :
	fOptimized(true),
	fPageNode(nil),
	fPagePath(pagePath)
{
	fInstance = this;

	strcpy(fTempPagePath, fPagePath);
	plFileUtils::StripExt(fTempPagePath);
	strcat(fTempPagePath, "_opt.prp");

	fResMgr = (plResManager*)hsgResMgr::ResMgr();
}

void plPageOptimizer::IFindLoc()
{
	class plPageIt : public plRegistryPageIterator 
	{
	public:
		plLocation fLoc;

		virtual hsBool EatPage(plRegistryPageNode* keyNode)
		{
			fLoc = keyNode->GetPageInfo().GetLocation();
			return true;
		}
	};

	plPageIt it;
	fResMgr->IterateAllPages(&it);
	fLoc = it.fLoc;
}

void plPageOptimizer::Optimize()
{
	fResMgr->AddSinglePage(fPagePath);

	// Get the location of the page we're optimizing
	IFindLoc();

	hsBool loaded = true;

	// Get the key for the scene node, we'll load it to force a load on all the objects
	plKey snKey = plKeyFinder::Instance().FindSceneNodeKey(fLoc);
	if (snKey)
	{
		// Load all the keys
		fPageNode = fResMgr->FindPage(fLoc);
		fResMgr->LoadPageKeys(fPageNode);

		// Put all the keys in a vector, so they won't get unreffed
		class plVecKeyCollector : public plRegistryKeyIterator
		{
		public:
			KeyVec& fKeys;
			plVecKeyCollector(KeyVec& keys) : fKeys(keys) {}
			virtual hsBool EatKey(const plKey& key) { fKeys.push_back(key); return true; }
		};
		plVecKeyCollector keyIt(fAllKeys);
		fResMgr->IterateKeys(&keyIt);

		// Set our load proc, which will track the order that objects are loaded
		fResMgr->SetProgressBarProc(KeyedObjectProc);

		// Load the page
		snKey->VerifyLoaded();

		// Unload everything
		snKey->RefObject();
		snKey->UnRefObject();
		snKey = nil;
	}
	else
	{
		loaded = false;
	}

	if (loaded)
		IRewritePage();

	UInt32 oldSize = plFileUtils::GetFileSize(fPagePath);
	UInt32 newSize = plFileUtils::GetFileSize(fTempPagePath);

	if (!loaded)
	{
		printf("no scene node.\n");
	}
	else if (fOptimized)
	{
		plFileUtils::RemoveFile(fTempPagePath);
		printf("already optimized.\n");
	}
	else if (oldSize == newSize)
	{
		plFileUtils::RemoveFile(fPagePath, true);
		plFileUtils::FileMove(fTempPagePath, fPagePath);

		printf("complete\n");
	}
	else
	{
		plFileUtils::RemoveFile(fTempPagePath);
		printf("failed.  File sizes different\n");
	}
}

void plPageOptimizer::KeyedObjectProc(plKey key)
{
	const char* keyName = key->GetName();
	const char* className = plFactory::GetNameOfClass(key->GetUoid().GetClassType());

	// For now, ignore any key that isn't in the location we're looking at.  That means stuff like textures.
	if (fInstance->fLoc != key->GetUoid().GetLocation())
		return;

	KeySet& loadedKeys = fInstance->fLoadedKeys;
	KeyVec& loadOrder = fInstance->fKeyLoadOrder;

	KeySet::iterator it = loadedKeys.lower_bound(key);
	if (it != loadedKeys.end() && *it == key)
	{
		printf("Keyed object %s(%s) loaded more than once\n", keyName, className);
	}
	else
	{
		loadedKeys.insert(it, key);
		loadOrder.push_back(key);
	}
}

void plPageOptimizer::IWriteKeyData(hsStream* oldPage, hsStream* newPage, plKey key)
{
	class plUpdateKeyImp : public plKeyImp
	{
	public:
		void SetStartPos(UInt32 startPos) { fStartPos = startPos; }
	};

	plUpdateKeyImp* keyImp = (plUpdateKeyImp*)(plKeyImp*)key;
	UInt32 startPos = keyImp->GetStartPos();
	UInt32 len = keyImp->GetDataLen();

	oldPage->SetPosition(startPos);
	if (len > fBuf.size())
		fBuf.resize(len);
	oldPage->Read(len, &fBuf[0]);

	UInt32 newStartPos = newPage->GetPosition();

	// If we move any buffers, this page wasn't optimized already
	if (newStartPos != startPos)
		fOptimized = false;

	keyImp->SetStartPos(newStartPos);
	newPage->Write(len, &fBuf[0]);
}

void plPageOptimizer::IRewritePage()
{
	hsUNIXStream newPage;

	if (newPage.Open(fTempPagePath, "wb"))
	{
		hsUNIXStream oldPage;
		oldPage.Open(fPagePath);

		const plPageInfo& pageInfo = fPageNode->GetPageInfo();

		UInt32 dataStart = pageInfo.GetDataStart();

		fBuf.resize(dataStart);

		oldPage.Read(dataStart, &fBuf[0]);
		newPage.Write(dataStart, &fBuf[0]);

		int size = (int)fKeyLoadOrder.size();
		for (int i = 0; i < size; i++)
			IWriteKeyData(&oldPage, &newPage, fKeyLoadOrder[i]);

		// If there are any objects that we didn't write (because they didn't load for
		// some reason), put them at the end
		for (int i = 0; i < fAllKeys.size(); i++)
		{
			hsBool found = (fLoadedKeys.find(fAllKeys[i]) != fLoadedKeys.end());
			if (!found)
				IWriteKeyData(&oldPage, &newPage, fAllKeys[i]);
		}

		UInt32 newKeyStart = newPage.GetPosition();
		UInt32 oldKeyStart = pageInfo.GetIndexStart();
		oldPage.SetPosition(oldKeyStart);

		UInt32 numTypes = oldPage.ReadSwap32();
		newPage.WriteSwap32(numTypes);

		for (UInt32 i = 0; i < numTypes; i++)
		{
			UInt16 classType = oldPage.ReadSwap16();
			UInt32 len = oldPage.ReadSwap32();
			UInt8 flags = oldPage.ReadByte();
			UInt32 numKeys = oldPage.ReadSwap32();

			newPage.WriteSwap16(classType);
			newPage.WriteSwap32(len);
			newPage.WriteByte(flags);
			newPage.WriteSwap32(numKeys);

			for (UInt32 j = 0; j < numKeys; j++)
			{
				plUoid uoid;
				uoid.Read(&oldPage);
				UInt32 startPos = oldPage.ReadSwap32();
				UInt32 dataLen = oldPage.ReadSwap32();

				// Get the new start pos
				plKeyImp* key = (plKeyImp*)fResMgr->FindKey(uoid);
				startPos = key->GetStartPos();

				uoid.Write(&newPage);
				newPage.WriteSwap32(startPos);
				newPage.WriteSwap32(dataLen);
			}
		}

		newPage.Close();
		oldPage.Close();
	}
}
