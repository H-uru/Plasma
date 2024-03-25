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
#ifndef plResManager_h_inc
#define plResManager_h_inc

#include "hsResMgr.h"
#include <set>
#include <map>
#include <vector>
#include "plFileSystem.h"

class plRegistryPageNode;
class plRegistryKeyIterator;
class plRegistryPageIterator;
class plRegistryDataStream;
class plResAgeHolder;
class plResManagerHelper;
class plDispatch;

// plProgressProc is a proc called every time an object loads, to keep a progress bar for
// loading ages up-to-date.
typedef void(*plProgressProc)(const plKey& key);

class plResManager : public hsResMgr
{
public:
    plResManager();
    virtual ~plResManager();

    // If the ResManager has already been initialized, you should call Reset after setting this
    void SetDataPath(const plFileName& path) { fDataPath = path; }

    // Mainly for external tools.
    void                AddSinglePage(const plFileName& path);
    plRegistryPageNode* FindSinglePage(const plFileName& path) const;
    void                RemoveSinglePage(const plFileName& path);

    //---------------------------
    //  Load and Unload
    //---------------------------
    void        Load  (const plKey& objKey) override;        // places on list to be loaded
    bool        Unload(const plKey& objKey) override;        // Unregisters (deletes) an object, Return true if successful
    plKey       CloneKey(const plKey& objKey) override;

    //---------------------------
    //  Finding Functions
    //---------------------------
    plKey               FindOriginalKey(const plUoid&);
    plKey               FindKey(const plUoid&) override; // Same as above, but will check the uoid for clones
    const plLocation&   FindLocation(const ST::string& age, const ST::string& page) const;
    // Use nullptr for any strings you don't need
    void                GetLocationStrings(const plLocation& loc, ST::string* ageBuffer, ST::string* pageBuffer) const;

    //---------------------------
    //  Establish reference linkage 
    //---------------------------
    bool   AddViaNotify(const plKey& key, plRefMsg* msg, plRefFlags::Type flags) override;
    bool   AddViaNotify(plRefMsg* msg, plRefFlags::Type flags) override; // msg->fRef->GetKey() == sentKey

    bool   SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags) override;
    bool   SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags) override;

    //---------------------------
    //  Reding and Writing keys
    //---------------------------
    // Read a Key in, and Notify me when the Object is loaded
    plKey ReadKeyNotifyMe(hsStream* stream, plRefMsg* retMsg, plRefFlags::Type flags) override;
    // Just read the Key data in and find a match in the registry and return it.
    plKey ReadKey(hsStream* stream) override;

    // For convenience you can write a key using the KeyedObject or the Key...same result
    void WriteKey(hsStream* s, hsKeyedObject* obj) override;
    void WriteKey(hsStream* s, const plKey& key) override;

    //---------------------------
    //  Reding and Writing Objects directly
    //---------------------------
    plCreatable*    ReadCreatable(hsStream* s) override;
    void            WriteCreatable(hsStream* s, plCreatable* cre) override;

    plCreatable*    ReadCreatableVersion(hsStream* s) override;
    void            WriteCreatableVersion(hsStream* s, plCreatable* cre) override;

    //---------------------------
    // Registry Modification Functions
    //---------------------------
    plKey NewKey(const ST::string& name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m = plLoadMask::kAlways) override;
    plKey NewKey(plUoid& newUoid, hsKeyedObject* object) override;

    plDispatchBase* Dispatch() override;

    virtual void SetProgressBarProc(plProgressProc proc);

    //---------------------------
    //  Load optimizations
    //---------------------------
    void LoadAgeKeys(const ST::string& age);
    void DropAgeKeys(const ST::string& age);
    void PageInRoom(const plLocation& page, uint16_t objClassToRef, plRefMsg* refMsg);
    void PageInAge(const ST::string& age);

    // Usually, a page file is kept open during load because the first keyed object
    // read causes all the other objects to be read before it returns.  In some
    // cases though (mostly just the texture file), this doesn't work.  In that
    // case, we just want to force it to stay open until we're done reading the age.
    void KeepPageOpen(const plLocation& page, bool keepOpen);

    // We're on the way down, act accordingly.
    void BeginShutdown() override;

    // Determines whether the time to read each object is dumped to a log
    void LogReadTimes(bool logReadTimes);

    // All keys version
    bool IterateKeys(plRegistryKeyIterator* iterator);
    // Single page version
    bool IterateKeys(plRegistryKeyIterator* iterator, const plLocation& pageToRestrictTo);
    // Iterate through loaded pages
    bool IteratePages(plRegistryPageIterator* iterator, const ST::string& ageToRestrictTo = {});
    // Iterate through ALL pages, loaded or not
    bool IterateAllPages(plRegistryPageIterator* iterator);

    // Helpers for key iterators
    void LoadPageKeys(plRegistryPageNode* pageNode);
    void UnloadPageObjects(plRegistryPageNode* pageNode, uint16_t classIndexHint);
    void DumpUnusedKeys(plRegistryPageNode* page) const;
    plRegistryPageNode* FindPage(const plLocation& location) const;
    plRegistryPageNode* FindPage(const ST::string& age, const ST::string& page) const;

    // Runs through all the pages and verifies that the data versions are good
    bool VerifyPages();

protected:
    friend class hsKeyedObject;
    friend class plKeyImp;
    friend class plResManagerHelper;

    plKey   ReRegister(const ST::string& nm, const plUoid& uoid) override;
    bool    ReadObject(plKeyImp* key) override; // plKeys call this when needed
    virtual bool    IReadObject(plKeyImp* pKey, hsStream *stream);  

    plCreatable*    IReadCreatable(hsStream* s) const;
    plKey           ICloneKey(const plUoid& objUoid, uint32_t playerID, uint32_t cloneID);

    void    IKeyReffed(plKeyImp* key) override;
    void    IKeyUnreffed(plKeyImp* key) override;

    bool    IReset() override;
    bool    IInit() override;
    void    IShutdown() override;

    void    IPageOutSceneNodes(bool forceAll);
    void    IDropAllAgeKeys();

    hsKeyedObject* IGetSharedObject(plKeyImp* pKey);

    void IUnloadPageKeys(plRegistryPageNode* pageNode, bool dontClear = false);

    bool IDeleteBadPages(std::vector<plRegistryPageNode*>& invalidPages, bool conflictingSeqNums);
    bool IWarnNewerPages(std::vector<plRegistryPageNode*>& newerPages);

    void ILockPages();
    void IUnlockPages();

    void AddPage(plRegistryPageNode* page);

    // Adds a key to the registry. Assumes uoid already set
    void AddKey(plKeyImp* key);

    plRegistryPageNode* CreatePage(const plLocation& location, const ST::string& age, const ST::string& page);

    bool          fInited;

    // True if we're reading in an object. We only read one object at a time
    bool               fReadingObject;
    std::vector<plKey> fQueuedReads;

    plFileName      fDataPath;

    plDispatch*     fDispatch;

    uint32_t fCurCloneID;     // Current clone ID.  If it isn't zero, we're cloning
    uint32_t fCurClonePlayerID;
    uint32_t fCloningCounter; // Next clone ID to use.

    typedef std::map<ST::string, plResAgeHolder*>   HeldAgeKeyMap;
    HeldAgeKeyMap   fHeldAgeKeys;
    plProgressProc  fProgressProc;

    plResManagerHelper  *fMyHelper;

    bool    fLogReadTimes;

    uint8_t fPageListLock;     // Number of locks on the page lists.  If it's greater than zero, they can't be modified
    bool    fPagesNeedCleanup; // True if something modified the page lists while they were locked.

    typedef std::set<plRegistryPageNode*> PageSet;
    typedef std::map<plLocation, plRegistryPageNode*> PageMap;
    PageMap fAllPages;         // All the pages, loaded or not
    PageSet fLoadedPages;      // Just the loaded pages
    PageSet fConflictingPages; // Pages whose sequence numbers conflict

    mutable plRegistryPageNode* fLastFoundPage;
};

#endif // plResManager_h_inc
