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
#ifndef plResManager_h_inc
#define plResManager_h_inc

#include "hsResMgr.h"
#include <set>
#include <map>
#include <vector>

class plRegistryPageNode;
class plRegistryKeyIterator;
class plRegistryPageIterator;
class plRegistryDataStream;
class plResAgeHolder;
class plResManagerHelper;
class plDispatch;

// plProgressProc is a proc called every time an object loads, to keep a progress bar for
// loading ages up-to-date.
typedef	void(*plProgressProc)(plKey key);

class plResManager : public hsResMgr
{
public:
	plResManager();
	virtual ~plResManager();

	// If the ResManager has already been initialized, you should call Reset after setting this
	void SetDataPath(const char* path) { fDataPath = path; }

	// Mainly for external tools.
	void				AddSinglePage(const char* path);
	plRegistryPageNode* FindSinglePage(const char* path) const;
	void				RemoveSinglePage(const char* path);

	//---------------------------
	//  Load and Unload
	//---------------------------
	virtual void		Load  (const plKey& objKey);		// places on list to be loaded
	virtual hsBool		Unload(const plKey& objKey);		// Unregisters (deletes) an object, Return true if successful
	virtual plKey		CloneKey(const plKey& objKey);

	//---------------------------
	//  Finding Functions
	//---------------------------
	plKey				FindOriginalKey(const plUoid&);
	virtual plKey		FindKey(const plUoid&); // Same as above, but will check the uoid for clones
	const plLocation&	FindLocation(const char* age, const char* page) const;
	// Use nil for any strings you don't need
	const void			GetLocationStrings(const plLocation& loc, char* ageBuffer, char* pageBuffer) const;

	//---------------------------
	//  Establish reference linkage	
	//---------------------------
	virtual hsBool AddViaNotify(const plKey& key, plRefMsg* msg, plRefFlags::Type flags);
	virtual hsBool AddViaNotify(plRefMsg* msg, plRefFlags::Type flags); // msg->fRef->GetKey() == sentKey

	virtual hsBool SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags);
	virtual hsBool SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags);

	//---------------------------
	//  Reding and Writing keys
	//---------------------------
	// Read a Key in, and Notify me when the Object is loaded
	virtual plKey ReadKeyNotifyMe(hsStream* stream, plRefMsg* retMsg, plRefFlags::Type flags); 
	// Just read the Key data in and find a match in the registry and return it.
	virtual plKey ReadKey(hsStream* stream); 

	// For convenience you can write a key using the KeyedObject or the Key...same result
	virtual void WriteKey(hsStream* s, hsKeyedObject* obj); 
	virtual void WriteKey(hsStream* s, const plKey& key); 

	//---------------------------
	//  Reding and Writing Objects directly
	//---------------------------
	virtual plCreatable*	ReadCreatable(hsStream* s);
	virtual void			WriteCreatable(hsStream* s, plCreatable* cre);

	virtual plCreatable*	ReadCreatableVersion(hsStream* s);
	virtual void			WriteCreatableVersion(hsStream* s, plCreatable* cre);

	//---------------------------
	// Registry Modification Functions
	//---------------------------
	virtual plKey NewKey(const char* name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m = plLoadMask::kAlways);
	virtual plKey NewKey(plUoid& newUoid, hsKeyedObject* object);

	virtual plDispatchBase* Dispatch();

	virtual void SetProgressBarProc(plProgressProc proc);

	//---------------------------
	//  Load optimizations
	//---------------------------
	void LoadAgeKeys(const char* age);
	void DropAgeKeys(const char* age);
	void PageInRoom(const plLocation& page, UInt16 objClassToRef, plRefMsg* refMsg);
	void PageInAge(const char* age);

	// Usually, a page file is kept open during load because the first keyed object
	// read causes all the other objects to be read before it returns.  In some
	// cases though (mostly just the texture file), this doesn't work.  In that
	// case, we just want to force it to stay open until we're done reading the age.
	void KeepPageOpen(const plLocation& page, hsBool keepOpen);

	// We're on the way down, act accordingly.
	virtual void BeginShutdown();

	// Determines whether the time to read each object is dumped to a log
	void LogReadTimes(hsBool logReadTimes);

	// All keys version
	hsBool IterateKeys(plRegistryKeyIterator* iterator);	
	// Single page version
	hsBool IterateKeys(plRegistryKeyIterator* iterator, const plLocation& pageToRestrictTo);
	// Iterate through loaded pages
	hsBool IteratePages(plRegistryPageIterator* iterator, const char* ageToRestrictTo = nil);
	// Iterate through ALL pages, loaded or not
	hsBool IterateAllPages(plRegistryPageIterator* iterator);

	// Helpers for key iterators
	void LoadPageKeys(plRegistryPageNode* pageNode);
	void UnloadPageObjects(plRegistryPageNode* pageNode, UInt16 classIndexHint);
	void DumpUnusedKeys(plRegistryPageNode* page) const;
	plRegistryPageNode* FindPage(const plLocation& location) const;
	plRegistryPageNode* FindPage(const char* age, const char* page) const;

	// Runs through all the pages and verifies that the data versions are good
	hsBool VerifyPages();

protected:
	friend class hsKeyedObject;
	friend class plKeyImp;
	friend class plResManagerHelper;

	virtual plKey	ReRegister(const char* nm, const plUoid& uoid);
	virtual hsBool	ReadObject(plKeyImp* key); // plKeys call this when needed
	virtual hsBool	IReadObject(plKeyImp* pKey, hsStream *stream);  

	plCreatable*	IReadCreatable(hsStream* s) const;
	plKey			ICloneKey(const plUoid& objUoid, UInt32 playerID, UInt32 cloneID);

	virtual void	IKeyReffed(plKeyImp* key);
	virtual void	IKeyUnreffed(plKeyImp* key);

	virtual hsBool	IReset();	
	virtual hsBool	IInit();
	virtual void	IShutdown();

	void	IPageOutSceneNodes(hsBool forceAll);
	void	IDropAllAgeKeys();

	hsKeyedObject* IGetSharedObject(plKeyImp* pKey);

	void IUnloadPageKeys(plRegistryPageNode* pageNode, hsBool dontClear = false);

	hsBool IDeleteBadPages(hsTArray<plRegistryPageNode*>& invalidPages, hsBool conflictingSeqNums);
	hsBool IWarnNewerPages(hsTArray<plRegistryPageNode*>& newerPages);

	void ILockPages();
	void IUnlockPages();

	void AddPage(plRegistryPageNode* page);

	// Adds a key to the registry. Assumes uoid already set
	void AddKey(plKeyImp* key);

	plRegistryPageNode* CreatePage(const plLocation& location, const char* age, const char* page);

	hsBool			fInited;
	UInt16			fPageOutHint;

	// True if we're reading in an object. We only read one object at a time
	hsBool			fReadingObject;
	std::vector<plKey> fQueuedReads;

	std::string fDataPath;

	plDispatch*		fDispatch;

	UInt32 fCurCloneID;		// Current clone ID.  If it isn't zero, we're cloning
	UInt32 fCurClonePlayerID;
	UInt32 fCloningCounter;	// Next clone ID to use.

	typedef std::map<std::string,plResAgeHolder*>	HeldAgeKeyMap;
	HeldAgeKeyMap	fHeldAgeKeys;
	plProgressProc	fProgressProc;

	plResManagerHelper	*fMyHelper;

	hsBool fLogReadTimes;

	UInt8 fPageListLock;	// Number of locks on the page lists.  If it's greater than zero, they can't be modified
	hsBool fPagesNeedCleanup;	// True if something modified the page lists while they were locked.

	typedef std::set<plRegistryPageNode*> PageSet;
	PageSet fAllPages;		// All the pages, loaded or not
	PageSet fLoadedPages;	// Just the loaded pages

	mutable plRegistryPageNode* fLastFoundPage;
};

#endif // plResManager_h_inc
