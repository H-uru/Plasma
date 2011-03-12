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
#ifndef plRegistryNode_h_inc
#define plRegistryNode_h_inc

#include "hsTypes.h"
#include "hsStream.h"
#include "plPageInfo.h"

#include <map>

class plRegistryKeyList;
class hsStream;
class plKeyImp;
class plRegistryKeyIterator;

enum PageCond
{
	kPageOk,
	kPageOutOfDate,
	kPageTooNew,
	kPageCorrupt,
};

//
// Represents one entire (age,page) location and contains all keys in that
// location. Note: just because the node exists does not mean that the keys are loaded.
//
class plRegistryPageNode 
{
protected:
	friend class plKeyFinder;

	// Map from class type to a list of keys of that type
	typedef std::map<UInt16, plRegistryKeyList*> KeyMap;
	KeyMap fKeyLists;
	int fDynLoadedTypes;	// The number of key types that have dynamic keys loaded
	int fStaticLoadedTypes;	// The number of key types that have all their keys loaded

	PageCond	fValid;			// Condition of the page
	char*		fPath;			// Path to the page file
	plPageInfo	fPageInfo;		// Info about this page

	hsBufferedStream fStream;	// Stream for reading/writing our page
	UInt8 fOpenRequests;		// How many handles there are to fStream (or
								// zero if it's closed)
	hsBool fIsNewPage;			// True if this page is new (not read off disk)

	plRegistryPageNode() {}

	inline plRegistryKeyList* IGetKeyList(UInt16 classType) const;
	PageCond IVerify();

public:
	// For reading a page off disk
	plRegistryPageNode(const char* path);

	// For creating a new page.
	plRegistryPageNode(const plLocation& location, const char* age, const char* page, const char* dataPath);
	~plRegistryPageNode();

	hsBool IsValid() const { return fValid == kPageOk; }
	PageCond GetPageCondition() { return fValid; }

	// True if we have any static or dynamic keys loaded
	hsBool IsLoaded() const		{ return fDynLoadedTypes > 0 || fStaticLoadedTypes > 0; }
	// True if all of our static keys are loaded
	hsBool IsFullyLoaded() const	{ return (fStaticLoadedTypes == fKeyLists.size() && !fKeyLists.empty()) || fIsNewPage; }

	// Export time only.  If we want to reuse a page, load the keys we want then
	// call SetNewPage, so it will be considered a new page from now on.  That
	// way we won't try to load it's keys again.
	hsBool IsNewPage() const	{ return fIsNewPage; }
	void SetNewPage()		{ fIsNewPage = true; }

	const plPageInfo& GetPageInfo() const { return fPageInfo; }

	void LoadKeys();	// Loads the keys off disk
	void UnloadKeys();	// Frees all our keys

	// Find a key by type and name
	plKeyImp* FindKey(UInt16 classType, const char* name) const;
	// Find a key by direct uoid lookup (or fallback to name lookup if that doesn't work)
	plKeyImp* FindKey(const plUoid& uoid) const;
	
	void AddKey(plKeyImp* key);

	// Sets a key as used or unused, ie there aren't any refs to it anymore.
	// When all the static keys are unused we can free the memory associated with
	// them.  When a dynamic key is unused we just delete it right away.
	void SetKeyUsed(plKeyImp* key);
	hsBool SetKeyUnused(plKeyImp* key);

	hsBool IterateKeys(plRegistryKeyIterator* iterator) const;
	hsBool IterateKeys(plRegistryKeyIterator* iterator, UInt16 classToRestrictTo) const;

	// Call this to get a read stream for the page.  If a valid pointer is
	// returned, make sure to call CloseStream when you're done using it.
	hsStream*	OpenStream();
	void		CloseStream();

	// Takes care of everything involved in writing this page to disk
	void Write();
	void DeleteSource();

	const char* GetPagePath() const { return fPath; }
};

#endif // plRegistryNode_h_inc
