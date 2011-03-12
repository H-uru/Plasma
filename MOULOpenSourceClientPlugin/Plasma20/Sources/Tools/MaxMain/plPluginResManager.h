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
#ifndef plPluginResManager_h_inc
#define plPluginResManager_h_inc

#include "../plResMgr/plResManager.h"
#include "../pnKeyedObject/plKey.h"
#include "hsTemplates.h"

class plPageInfo;
class plRegistryPageNode;
class plSceneNode;

class plPluginResManager : public plResManager
{
public:
	static plPluginResManager* ResMgr() { return (plPluginResManager*)hsgResMgr::ResMgr(); }

	//------------------------
	// Location management 
	//------------------------

	// Given a page string combo, returns the key of the sceneNode for that page. If the page does not exist, it creates one.
	plKey NameToLoc(const char* age, const char* page, Int32 sequenceNumber, hsBool itinerant = false);

	// Verifies that the given sequence number belongs to the given string combo and ONLY that combo. Returns a new, unique sequenceNumber if not
	Int32 VerifySeqNumber(Int32 sequenceNumber, const char* age, const char* page);

	enum VerifyErrors
	{
		kNoVerifyError,
		kErrRightPageWrongSeq,
		kErrSeqAlreadyTaken,
		kErrCantFindValid
	};
	VerifyErrors		GetLastVerifyError() const { return fLastVerifyError; }
	const plPageInfo*	GetLastVerifyPage() const { return fLastVerifyPage; }

	// Write all pages. Duh.
	void WriteAllPages();

	// Given a location, returns the plLocation corresponding to the common page from the same age
	const plLocation& GetCommonPage(const plLocation& sisterPage, int whichPage);

	// If we have the key of an object that needs to get written out, but we're too lazy to set up
	// a proper reference too (e.g. adding it to a scenenode), we can add it here. On add, it will be RefObject()'d,
	// and then after we've written out everything, it will be UnRefObject()'d (at the same time we UnRef all
	// our scenenodes in EndExport.
	void AddLooseEnd(plKey key);

	// Given a key, will ref and unref the associated object so it goes away, then nukes the key and sets it to nil.
	// The key's UOID should be safe to reuse at that point, unless someone else still has a ref, in which case
	// this function returns false (returns true if successful).
	bool NukeKeyAndObject(plKey& objectKey);

	// Flushes all the created scene nodes out
	void EndExport();

protected:
	plLocation ICreateLocation(const char* age, const char* page, hsBool itinerant);
	plLocation ICreateLocation(const char* age, const char* page, Int32 seqNum, hsBool itinerant);

	plRegistryPageNode* INameToPage(const char* age, const char* page, Int32 sequenceNumber, hsBool itinerant = false);

	void IPreLoadTextures(plRegistryPageNode* pageNode, Int32 origSeqNumber);

	void IShutdown();

	VerifyErrors			fLastVerifyError;
	const plPageInfo*		fLastVerifyPage;
	hsTArray<plSceneNode*>	fExportedNodes;
	hsTArray<plKey>			fLooseEnds;
};

#endif // plPluginResManager_h_inc
