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

#include "plResManager.h"
#include "plLocalization.h"
#include "plRegistryNode.h"
#include "plResManagerHelper.h"
#include "plResMgrSettings.h"

#include "hsTimer.h"
#include "plTimerCallbackManager.h"

#include "pnDispatch/plDispatch.h"
#include "pnFactory/plCreator.h"
#include "pnFactory/plFactory.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKeyImp.h"
#include "pnMessage/plClientMsg.h"
#include "pnMessage/plRefMsg.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnNetCommon/plSynchedObject.h"
#include "pnNetCommon/plNetApp.h"

#include "plAgeDescription/plAgeDescription.h"
#include "plMessageBox/hsMessageBox.h"
#include "plScene/plSceneNode.h"
#include "plStatusLog/plStatusLog.h"

bool gDataServerLocal = false;

/// Logging #define for easier use
#define kResMgrLog(level, log) if (plResMgrSettings::Get().GetLoggingLevel() >= level) log

template<typename... _Args>
static void ILog(uint8_t level, const char* format, _Args&&... args)
{
    static plStatusLog* log = plStatusLogMgr::GetInstance().CreateStatusLog
        (
        plStatusLogMgr::kDefaultNumLines,
        "resources.log",
        plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe
        );

    uint32_t color = 0;
    switch (level)
    {
    case 1: color = 0xffffffff; break;
    case 2: color = 0xff8080ff; break;
    case 3: color = 0xffffff80; break;
    case 4: color = 0xff8080ff; break;
    }

    log->AddLineF(color, format, std::forward<_Args>(args)...);
}

plResManager::plResManager():
    fInited(),
    fDispatch(),
    fReadingObject(),
    fCurCloneID(),
    fCurClonePlayerID(),
    fCloningCounter(),
    fProgressProc(),
    fMyHelper(),
    fLogReadTimes(),
    fPageListLock(),
    fPagesNeedCleanup(),
    fLastFoundPage()
{
}

plResManager::~plResManager()
{
    // verify shutDown
    hsAssert(!fInited,"ResMgr not shutdown");
}

bool plResManager::IInit()
{
    if (fInited)
        return true;
    fInited = true;

    kResMgrLog(1, ILog(1, "Initializing resManager..."));

    if (plResMgrSettings::Get().GetLoadPagesOnInit()) {
        // We want to go through all the data files in our data path and add new
        // plRegistryPageNodes to the regTree for each
        std::vector<plFileName> prpFiles = plFileSystem::ListDir(fDataPath, "*.prp");
        for (auto iter = prpFiles.begin(); iter != prpFiles.end(); ++iter) {
            plRegistryPageNode* node = new plRegistryPageNode(*iter);
            const plPageInfo& pi = node->GetPageInfo();

            // If a page is already added with this location, add both the already known page
            // and the newly discovered page to a set of conflicts.
            auto pageResult = fAllPages.emplace(pi.GetLocation(), node);
            if (!pageResult.second) {
                fConflictingPages.insert(pageResult.first->second);
                fConflictingPages.insert(node);
            }
        }
    }

    // Special case: we always create pages for the predefined pages
    CreatePage(plLocation::kGlobalFixedLoc, "Global", "FixedKeys");

    hsAssert(!fDispatch, "Dispatch already set");
    fDispatch = new plDispatch;
    
    plgTimerCallbackMgr::Init(); 

    // Init our helper
    fMyHelper = new plResManagerHelper(this);
    fMyHelper->Init();
    hsAssert(fMyHelper->GetKey() != nullptr, "ResManager helper didn't init properly!");

    kResMgrLog(1, ILog(1, "   ...Init was successful!"));

    return true; 
}

bool plResManager::IReset()   // Used to Re-Export (number of times)
{
    BeginShutdown();
    IShutdown();
    return IInit();
}

void plResManager::BeginShutdown()
{
    if (fMyHelper)
        fMyHelper->SetInShutdown(true);
}

void plResManager::IShutdown()
{
    if (!fInited)
        return;

    kResMgrLog(1, ILog(1, "Shutting down resManager..."));

    // Make sure we're not holding on to any ages for load optimization
    IDropAllAgeKeys();

    // At this point, we may have an undelivered future time stamped message
    // in the Dispatch, which is reffing a bunch of keys we "temporarily" loaded.
    // The obvious problems with that solution to avoid loading and unloading
    // and reloading keys aside, those keys will continue to exist until the
    // dispatch is destroyed, so they will show up as key leaks in the report.
    // But they won't show up as memory leaks, because they'll be destroyed
    // when the Dispatch is destructed.
    // Update - now we call BeginShutdown, well, at the beginning of Shutdown.
    // We pass that on to the Helper, so it knows to immediately dump the keys
    // for any pages it loads, cuz there's no tomorrow.

    IPageOutSceneNodes(false);

    // Shut down our helper
    fMyHelper->Shutdown();  // This will call UnregisterAs(), which will delete itself
    fMyHelper = nullptr;

    // TimerCallbackMgr is a fixed-keyed object, so needs to shut down before the registry
    plgTimerCallbackMgr::Shutdown(); 

    // Formerly, we destroyed the Dispatcher here to clean up keys for leak reporting.
    // However, if there are *real* leaked keys, then they will want to unload after this step.
    // To unload, plKeyImp needs to dispatcher. SO, we're going to pitch everything currently in the
    // Dispatcher and kill it later
    fDispatch->BeginShutdown();

    // Just before we shut down the registry, page out any keys that still exist.
    // (They shouldn't... they're baaaaaad.)
    IPageOutSceneNodes(true);

    // Shut down the registry (finally!)
    ILockPages();

    // Unload all keys before actually deleting the pages.
    // When a key's refcount drops to zero, IKeyUnreffed looks up the key's page.
    // If the page is already deleted at that point, this causes a use after free and potential crash.
    for (const auto& pair : fAllPages) {
        pair.second->UnloadKeys();
    }
    fLoadedPages.clear();
    fLastFoundPage = nullptr;
    for (const auto& pair : fAllPages) {
        delete pair.second;
    }
    fAllPages.clear();

    IUnlockPages();

    // Now, kill off the Dispatcher
    hsRefCnt_SafeUnRef(fDispatch);
    fDispatch = nullptr;

    kResMgrLog(1, ILog(1, "   ...Shutdown successful!"));

    fInited = false;
}

void plResManager::AddSinglePage(const plFileName& pagePath)
{
    plRegistryPageNode* node = new plRegistryPageNode(pagePath);
    AddPage(node);
}

plRegistryPageNode* plResManager::FindSinglePage(const plFileName& path) const
{
    PageMap::const_iterator it;
    for (it = fAllPages.begin(); it != fAllPages.end(); it++)
    {
        if (it->second->GetPagePath().AsString().compare_i(path.AsString()) == 0)
            return it->second;
    }

    return nullptr;
}

void plResManager::RemoveSinglePage(const plFileName& path)
{
    plRegistryPageNode* node = FindSinglePage(path);
    if (node)
    {
        plLocation loc = node->GetPageInfo().GetLocation();
        fAllPages.erase(loc);
        delete node;
    }
}

plDispatchBase *plResManager::Dispatch()
{
    return fDispatch;
}


void plResManager::LogReadTimes(bool logReadTimes)
{
    fLogReadTimes = logReadTimes;
    if (fLogReadTimes)
    {
        plStatusLog::AddLineS("readtimings.log", plStatusLog::kWhite, "Created readtimings log");
    }
}

hsKeyedObject* plResManager::IGetSharedObject(plKeyImp* pKey)
{
    plKeyImp* origKey = plKeyImp::GetFromKey(pKey->GetCloneOwner());

    // Find the first non-nil key and ask it to clone itself
    size_t count = origKey->GetNumClones();
    for (size_t i = 0; i < count; i++)
    {
        plKey cloneKey = origKey->GetCloneByIdx(i);
        if (cloneKey)
        {
            hsKeyedObject* obj = cloneKey->ObjectIsLoaded();
            if (obj)
            {
                hsKeyedObject* sharedObj = obj->GetSharedObject();
                if (sharedObj)
                    return sharedObj;
            }
        }
    }

    return nullptr;
}

//// ReadObject /////////////////////////////////////////////////////////////
//  Given a key, goes off and reads in the actual object from its source
bool plResManager::ReadObject(plKeyImp* key)
{
    // Read in the object. If while we are doing this something else requests a
    // load (through AddViaNotify or ReadKeyNotifyMe) we consider it a child load
    // and put it in a queue. This keeps us from jumping over to another object's
    // data while we're still reading in its parent (which is bad because that
    // trashes our file buffering)
    //
    // Also, we find the pageNode and open its stream here. We close the
    // stream when child reads are done. If a child load is using the same stream,
    // it will just inc/dec the open/close count during its read, and not actually 
    // close the stream, so we don't lose our place, lose our file handle, and thrash.

    kResMgrLog(4, ILog(4, "   ...Opening page data stream for location {}...", key->GetUoid().GetLocation().StringIze()));
    plRegistryPageNode *pageNode = FindPage(key->GetUoid().GetLocation());
    if (!pageNode)
    {
        kResMgrLog(3, ILog(3, "   ...Data stream failed to open on read!"));
        return false;
    }
    fReadingObject = true;
    bool ret = IReadObject(key, pageNode->OpenStream());
    fReadingObject = false;

    if (!fQueuedReads.empty())
    {
        // Now that the parent object is completely read in, we can do the child
        // loads. We copy off all the children that were queued during our load so
        // that we won't get our children's child loads mixed in (a parent only loads
        // its immediate children)
        std::vector<plKey> children = fQueuedReads;
        fQueuedReads.clear();

        for (const auto& childKey : children)
            childKey->VerifyLoaded();
    }

    // we're done loading, and all our children are too, so send the notify
    key->NotifyCreated();

    pageNode->CloseStream();

    return ret;
}

bool plResManager::IReadObject(plKeyImp* pKey, hsStream *stream)
{
    static uint64_t totalTime = 0;

    uint64_t startTotalTime = totalTime;
    uint64_t startTime = 0;
    if (fLogReadTimes)
        startTime = hsTimer::GetTicks();

    hsKeyedObject* ko = nullptr;

    hsAssert(pKey, "Null Key");
    if (pKey->GetUoid().GetLoadMask().DontLoad())
        return false;

    hsAssert(pKey->GetStartPos() != uint32_t(-1), "Missing StartPos");
    hsAssert(pKey->GetDataLen() != uint32_t(-1), "Missing Data Length");

    if (pKey->GetStartPos() == uint32_t(-1) || pKey->GetDataLen() == uint32_t(-1))
        return false; // Try to recover from this by just not reading an object

    kResMgrLog(3, ILog(3, "   Reading object {}::{}", plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()), pKey->GetUoid().GetObjectName()));

    const plUoid& uoid = pKey->GetUoid();

    bool isClone = uoid.IsClone();

    kResMgrLog(4, ILog(4, "   ...is{} a clone", isClone ? "" : " not"));

    // If we're loading the root object of a clone (the object for the key that
    // was actually cloned), set up the global cloning flags so any child objects
    // read in will get them.  Also turn off synching until the object is fully
    // loaded, so we don't send out any partially loaded state.
    bool setClone = false;
    if (isClone && fCurCloneID != uoid.GetCloneID())
    {
        kResMgrLog(4, ILog(4, "   ...fCurCloneID = {}, uoid's cloneID = {}", fCurCloneID, uoid.GetCloneID()));

        if (fCurCloneID != 0)
        {
            hsAssert(false, "Recursive clone");
            kResMgrLog(3, ILog(3, "   ...RECURSIVE CLONE DETECTED. ABORTING READ..."));
            return false;
        }
        fCurClonePlayerID = uoid.GetClonePlayerID();
        fCurCloneID = uoid.GetCloneID();
        setClone = true;

        kResMgrLog(4, ILog(4, "   ...now fCurCloneID = {}, fCurClonePlayerID = {}", fCurCloneID, fCurClonePlayerID));
    }

    // If this is a clone key, try and get the original object to give us a clone
    if (isClone)
    {
        kResMgrLog(4, ILog(4, "   ...Trying to get shared object..."));
        ko = IGetSharedObject(pKey);
        kResMgrLog(4, ILog(4, "   ...IGetSharedObject() {}", (ko != nullptr) ? "succeeded" : "failed"));
    }

    // If we couldn't share the object, read in a fresh copy
    if (!ko)
    {
        stream->SetPosition(pKey->GetStartPos());
        kResMgrLog(4, ILog(4, "   ...Reading from position {} bytes...", pKey->GetStartPos()));

        plCreatable* cre = ReadCreatable(stream);
        hsAssert(cre, "Could not Create Object");
        if (cre)
        {   
            ko = hsKeyedObject::ConvertNoRef(cre);

            if (ko != nullptr)
            {
                kResMgrLog(4, ILog(4, "   ...Creatable read and valid"));
            }
            else
            {
                kResMgrLog(3, ILog(3, "   ...Creatable read from stream not keyed object!"));
            }

            if (fProgressProc != nullptr)
            {
                fProgressProc(plKey::Make(pKey));
            }
        }
        else
        {
            kResMgrLog(3, ILog(3, "   ...ERROR: Unable to read creatable from stream!"));
        }
    }

    if (isClone && setClone)
    {
        fCurClonePlayerID = 0;
        fCurCloneID = 0;
    }

    kResMgrLog(4, ILog(4, "   ...Read complete for object {}::{}", plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()), pKey->GetUoid().GetObjectName()));

    if (fLogReadTimes)
    {
        uint64_t ourTime = hsTimer::GetTicks() - startTime;
        uint64_t childTime = totalTime - startTotalTime;
        ourTime -= childTime;

        plStatusLog::AddLineSF("readtimings.log", plStatusLog::kWhite, "{}, {}, {}, {.1f}",
            pKey->GetUoid().GetObjectName(),
            plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()),
            pKey->GetDataLen(),
            hsTimer::GetMilliSeconds<float>(ourTime));

        totalTime += (hsTimer::GetTicks() - startTime) - childTime;
    }

    return (ko != nullptr);
}

//// plPageOutIterator ///////////////////////////////////////////////////////
//  See below function
class plPageOutIterator : public plRegistryPageIterator
{
protected:
    plResManager* fResMgr;
    uint16_t      fHint;

public:
    plPageOutIterator(plResManager* resMgr, uint16_t hint) : fResMgr(resMgr), fHint(hint)
    {
        fResMgr->IterateAllPages(this);
    }

    bool EatPage(plRegistryPageNode* page) override
    {
        fResMgr->UnloadPageObjects(page, fHint);
        return true;
    }
};

// Just the scene nodes (and objects referenced by the node... and so on)
void plResManager::IPageOutSceneNodes(bool forceAll)
{
    plSynchEnabler ps(false);   // disable dirty tracking while paging out

    if (forceAll)
    {
        hsStatusMessage( "--- plResManager Object Leak Report (BEGIN) ---" );
        plPageOutIterator iter(this, static_cast<uint16_t>(-1));
        hsStatusMessage( "--- plResManager Object Leak Report (END) ---" );
    }
    else
    {
        plPageOutIterator iter(this, 0);
    }
}

//// FindKey /////////////////////////////////////////////////////////////////

inline plKeyImp* IFindKeyLocalized(const plUoid& uoid, plRegistryPageNode* page)
{
    const ST::string& objectName = uoid.GetObjectName();

    // If we're running localized, try to find a localized version first
    if ((!objectName.empty()) && plLocalization::IsLocalized())
    {
        plFileName localName = plLocalization::GetLocalized(objectName);
        if (localName.IsValid())
        {
            plKeyImp* localKey = page->FindKey(uoid.GetClassType(), localName.AsString());
            if (localKey != nullptr)
                return localKey;
        }
    }

    // Try to find the non-localized version
    return page->FindKey(uoid);
}

plKey plResManager::FindOriginalKey(const plUoid& uoid)
{
    plKey key;
    plKeyImp* foundKey = nullptr;

    plRegistryPageNode* page = FindPage(uoid.GetLocation());
    if (page == nullptr)
        return key;

    // Try our find first, without loading
    foundKey = IFindKeyLocalized(uoid, page);
    if (foundKey != nullptr)
        key = plKey::Make(foundKey);

    if (!key && plResMgrSettings::Get().GetPassiveKeyRead())
    {
        // Passive key read mode is where we read keys in and attempt to match
        // them to keys in the registry, but we will NOT force a load on the page
        // to find the keys. If the key isn't already in the registry to match,
        // we create what we call a "passive key", i.e. it's a key with no real
        // info apart from the uoid. Used when you want to read in a object that 
        // contains keys but don't want to actually use those keys (only write 
        // them back out).

        // Note: startPos of -1 means we didn't read it from disk, but 0 length
        // is our special key that we're a passively created key
        foundKey = new plKeyImp(uoid, uint32_t(-1), uint32_t(0));
        key = plKey::Make(foundKey);
    }

    // OK, find didn't work. Can we load and try again?
    if (!key && !page->IsFullyLoaded())
    {
        // Tell the resManager's helper to load and hold our page keys temporarily
        plResManagerHelper::GetInstance()->LoadAndHoldPageKeys(page);

        // Try again
        foundKey = IFindKeyLocalized(uoid, page);
        if (foundKey != nullptr)
            key = plKey::Make(foundKey);
    }
    return key;
}

plKey plResManager::FindKey(const plUoid& uoid)
{
    plKey key = FindOriginalKey(uoid);

    // If we're looking for a clone, get the clone instead of the original
    if (key && uoid.IsClone())
        key = plKeyImp::GetFromKey(key)->GetClone(uoid.GetClonePlayerID(), uoid.GetCloneID());

    return key;
}

const plLocation& plResManager::FindLocation(const ST::string& age, const ST::string& page) const
{
    static plLocation invalidLoc;

    plRegistryPageNode* pageNode = FindPage(age, page);
    if (pageNode)
        return pageNode->GetPageInfo().GetLocation();

    return invalidLoc;
}

void plResManager::GetLocationStrings(const plLocation& loc, ST::string* ageBuffer, ST::string* pageBuffer) const
{
    plRegistryPageNode* page = FindPage(loc);
    const plPageInfo& info = page->GetPageInfo();

    if (ageBuffer)
        *ageBuffer = info.GetAge();
    if (pageBuffer)
        *pageBuffer = info.GetPage();
}

bool plResManager::AddViaNotify(plRefMsg* msg, plRefFlags::Type flags)
{
    hsAssert(msg && msg->GetRef() && msg->GetRef()->GetKey(), "Improperly filled out ref message");
    plKey key = msg->GetRef()->GetKey();        // for linux build
    return AddViaNotify(key, msg, flags);
}

bool plResManager::AddViaNotify(const plKey &key, plRefMsg* msg, plRefFlags::Type flags)
{
    hsAssert(key, "Can't add without a Key");
    if (!key)
    {
        hsRefCnt_SafeUnRef(msg);
        return false;
    }

    plKeyImp::GetFromKey(key)->SetupNotify(msg,flags);
    
    if (flags != plRefFlags::kPassiveRef)
    {
        hsKeyedObject* ko = key->ObjectIsLoaded();
        if (!ko)
            Load(key);
    }

    return true;
}

bool plResManager::SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags)
{
    if (!ko)
        return false;
    const plKey& key = ko->GetKey();
    return SendRef(key, refMsg, flags);
}

//////////////////////////
// This one does the dirty. Calls, the protected ISetupNotify on the key being reffed.
// That will setup the notifications on the key, but not send any. If the object is
// currently not loaded, we're done (and this behaves exactly like AddViaNotify().
// If it is in memory, and the one making the reference is in memory (it presumably
// is in the absence of strange doings), we bypass the Dispatch system and call
// the object making the reference's MsgReceive directly, so the object will have
// received the reference via its normal message processing without the message
// having to wait in the queue, and more importantly, before the SendRef call returns.
// This doesn't mean you are guaranteed to have your ref at the return of SendRef,
// because if it's not in memory, we don't wait around while we load it, we just
// return false.
bool plResManager::SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags)
{
    if (!key)
    {
        hsRefCnt_SafeUnRef(refMsg);
        return false;
    }

    plKeyImp* iKey = plKeyImp::GetFromKey(key);
    iKey->ISetupNotify(refMsg, flags);
    hsRefCnt_SafeUnRef(refMsg);

    if (flags != plRefFlags::kPassiveRef)
        iKey->VerifyLoaded();

    hsKeyedObject* ko = key->ObjectIsLoaded();
    if (!ko)
        return false;

    refMsg->SetRef(ko);
    refMsg->SetTimeStamp(hsTimer::GetSysSeconds());

    for (size_t i = 0; i < refMsg->GetNumReceivers(); i++)
    {
        hsKeyedObject* rcv = refMsg->GetReceiver(i)->ObjectIsLoaded();
        if (rcv)
            rcv->MsgReceive(refMsg);
    }
    return true;
}

void plResManager::Load(const plKey &key)       // places on list to be loaded
{
    if (fReadingObject)
        fQueuedReads.push_back(key);
    else
        key->VerifyLoaded();    // force Load
}

plKey plResManager::ReadKeyNotifyMe(hsStream* stream, plRefMsg* msg, plRefFlags::Type flags)
{
    plKey key = ReadKey(stream);

    if (!key)
    {
        hsRefCnt_SafeUnRef(msg);
        return nullptr;
    }
    if(key->GetUoid().GetLoadMask().DontLoad())
    {
        hsStatusMessageF("{} being skipped because of load mask", key->GetName());
        hsRefCnt_SafeUnRef(msg);
        return nullptr;
    }

    plKeyImp::GetFromKey(key)->SetupNotify(msg,flags);

    hsKeyedObject* ko = key->ObjectIsLoaded();

    if (!ko)
    {
        Load(key);
    }

    return key;
}

//// NewKey //////////////////////////////////////////////////////////////////
//  Creates a new key and assigns it to the given keyed object, also placing
//  it into the registry.

plKey plResManager::NewKey(const ST::string& name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m )
{
    hsAssert(!name.empty(), "No name for new key");
    plUoid newUoid(loc, object->ClassIndex(), name, m);
    return NewKey(newUoid, object);
}

plKey plResManager::NewKey(plUoid& newUoid, hsKeyedObject* object)
{
    hsAssert(fInited, "Attempting to create a new key before we're inited!");

    plKeyImp* newKey = new plKeyImp;
    newKey->SetUoid(newUoid);
    AddKey(newKey);

    plKey keyPtr = plKey::Make(newKey);
    object->SetKey(keyPtr);

    return keyPtr;
}

plKey plResManager::ReRegister(const ST::string& nm, const plUoid& oid)
{
    hsAssert(fInited, "Attempting to reregister a key before we're inited!");

    bool canClone = false;
    if (fCurCloneID != 0)
    {   
        // Not allowed to clone these things
        int oidType = oid.GetClassType();
        if (oidType  != CLASS_INDEX_SCOPED(plSceneNode) &&
            oidType  != CLASS_INDEX_SCOPED(plLOSDispatch) &&
            oidType  != CLASS_INDEX_SCOPED(plTimerCallbackManager) &&
            oidType  != CLASS_INDEX_SCOPED(pfConsole) &&
            oidType  != CLASS_INDEX_SCOPED(plAudioSystem) &&
            oidType  != CLASS_INDEX_SCOPED(plInputManager) &&
            oidType  != CLASS_INDEX_SCOPED(plClient) &&
            oidType  != CLASS_INDEX_SCOPED(plNetClientMgr) &&
            oidType  != CLASS_INDEX_SCOPED(plAvatarAnimMgr) &&
            oidType  != CLASS_INDEX_SCOPED(plSoundBuffer) &&
            oidType  != CLASS_INDEX_SCOPED(plResManagerHelper) &&
            oidType  != CLASS_INDEX_SCOPED(plSharedMesh))
            canClone = true;

        // Can't clone fixed keys
        if (oid.GetLocation() == plLocation::kGlobalFixedLoc)
            canClone = false;
    }
         
    plKey pOrigKey = FindOriginalKey(oid);
    if (!canClone)
    {   
        if (pOrigKey)
        {   
            return pOrigKey;
        }
        // the clone doesn't exist
        else if (oid.IsClone())
        {
            return nullptr;
        }
    }
    else    //we are cloning
    {
        if (pOrigKey)
        {
            plKey cloneKey = plKeyImp::GetFromKey(pOrigKey)->GetClone(fCurClonePlayerID, fCurCloneID);
            if (cloneKey)
                return cloneKey;
        }
    }

    plKeyImp* pKey = new plKeyImp;
    if (canClone && pOrigKey)
    {   
        pKey->CopyForClone(plKeyImp::GetFromKey(pOrigKey), fCurClonePlayerID, fCurCloneID);
        plKeyImp::GetFromKey(pOrigKey)->AddClone(pKey);
    }
    else
    {
        // Make sure key doesn't already exist
        if (pOrigKey)
        {
            hsAssert(false, "Attempting to add duplicate key");
            delete pKey;
            return nullptr;
        }

        pKey->SetUoid(oid);         // Tell the Key its ID
        AddKey(pKey);
    }

    hsAssert(pKey, "ReRegister: returning nil key?");
    return plKey::Make(pKey);
}

//// ReadKey /////////////////////////////////////////////////////////////////
//  Reads a "key" from the given stream. What we secretly do is read in the
//  plUoid for a key and look up to find the key. Nobody else will know :)

plKey plResManager::ReadKey(hsStream* s)
{
    bool nonNil = s->ReadBool();
    if (!nonNil)
        return nullptr;

    plUoid uoid;
    uoid.Read(s);

    plKey key;

    if (fCurCloneID != 0)
    {
        // We're reading child of a clone object, it needs to be cloned too
        key = ReRegister(uoid.GetObjectName(), uoid);
    }
    else if (uoid.GetCloneID() != 0)
    {
        // We're reading a clone key. first see if we already have that key around....
        key = FindKey(uoid);
        if (key == nullptr)
        {
            fCurClonePlayerID = uoid.GetClonePlayerID();
            fCurCloneID = uoid.GetCloneID();
            key = ReRegister(uoid.GetObjectName(), uoid);
            fCurClonePlayerID = 0;
            fCurCloneID = 0;
        }
    }
    else
    {
        // We're reading a regular, non-clone object
        key = FindKey(uoid);
    }

    return key;
}

//// WriteKey ////////////////////////////////////////////////////////////////

void plResManager::WriteKey(hsStream* s, hsKeyedObject* obj)
{
    if (obj)
        WriteKey(s, obj->GetKey());
    else
        WriteKey(s, plKey());
}

void    plResManager::WriteKey(hsStream *s, const plKey &key)
{
    s->WriteBool(key != nullptr);
    if (key)
        key->GetUoid().Write(s);
}

//
// Create cloned key but don't load yet
//
plKey plResManager::CloneKey(const plKey& objKey)
{
    if (!objKey)
    {
        hsStatusMessage("CloneKey: nil key, returning nil");
        return nullptr;
    }

    fCloningCounter++;
    return ICloneKey(objKey->GetUoid(), plNetClientApp::GetInstance()->GetPlayerID(), fCloningCounter);
}

plKey plResManager::ICloneKey(const plUoid& objUoid, uint32_t playerID, uint32_t cloneID)
{
    hsAssert(fCurCloneID == 0, "Recursive clone");
    fCurCloneID = cloneID;
    fCurClonePlayerID = playerID;

    plKey cloneKey = ReRegister("", objUoid);

    fCurClonePlayerID = 0;
    fCurCloneID = 0;

    // Then notify NetClientMgr when object loads
    plObjRefMsg* refMsg = new plObjRefMsg(plNetClientApp::GetInstance()->GetKey(), plRefMsg::kOnCreate, 0, 0);
    AddViaNotify(cloneKey, refMsg, plRefFlags::kPassiveRef);    

    return cloneKey;
}

//
// Unregisters (deletes) an object.
// Currently, this means the object is going away permanently.
// When support for paging is added, key->UnRegister() should not clear its notify lists.
// Return true if successful.
//
bool plResManager::Unload(const plKey& objKey)
{
    if (objKey)
    {
        plKeyImp::GetFromKey(objKey)->UnRegister();
        fDispatch->UnRegisterAll(objKey);
        return true;
    }
    return false;
}

plCreatable* plResManager::IReadCreatable(hsStream* s) const
{
    uint16_t hClass = s->ReadLE16();
    plCreatable* pCre = plFactory::Create(hClass);
    if (!pCre)
        hsAssert( hClass == 0x8000, "Invalid creatable index" );

    return pCre;
}

plCreatable* plResManager::ReadCreatable(hsStream* s)
{
    plCreatable *pCre = IReadCreatable(s);
    if (pCre)
        pCre->Read(s, this);
    return pCre;
}

plCreatable* plResManager::ReadCreatableVersion(hsStream* s)
{
    plCreatable *pCre = IReadCreatable(s);
    if (pCre)
        pCre->ReadVersion(s, this);
    return pCre;
}

inline void IWriteCreatable(hsStream* s, plCreatable* pCre)
{
    int16_t hClass = pCre ? pCre->ClassIndex() : 0x8000;
    hsAssert(pCre == nullptr || plFactory::IsValidClassIndex(hClass), "Invalid class index on write");
    s->WriteLE16(hClass);
}

void plResManager::WriteCreatable(hsStream* s, plCreatable* pCre)
{
    IWriteCreatable(s, pCre);
    if (pCre)
        pCre->Write(s, this);
}

void plResManager::WriteCreatableVersion(hsStream* s, plCreatable* pCre)
{
    IWriteCreatable(s, pCre);
    if (pCre)
        pCre->WriteVersion(s, this);
}

void plResManager::SetProgressBarProc(plProgressProc proc)
{
    fProgressProc = proc;
}

//////////////////////////////////////////////////////////////////////////////
//// Paging Functions ////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// plResAgeHolder //////////////////////////////////////////////////////////
//  Helper object that stores all the keys for an age, to optimize the load
//  process. 

class plResAgeHolder : public hsRefCnt
{
    public:
        std::set<plKey> fKeys;
        ST::string      fAge;

        plResAgeHolder() {}
        plResAgeHolder( const ST::string& age ) : fAge( age ) {}
};

//// plResHolderIterator /////////////////////////////////////////////////////

class plResHolderIterator : public plRegistryPageIterator
{
protected:
    std::set<plKey>& fKeys;
    ST::string fAgeName;
    plResManager* fResMgr;

public:
    plResHolderIterator(const ST::string& age, std::set<plKey>& keys, plResManager* resMgr)
            : fAgeName(age), fKeys(keys), fResMgr(resMgr) {}

    bool EatPage(plRegistryPageNode* page) override
    {
        if (page->GetPageInfo().GetAge().compare_i(fAgeName) == 0)
        {
            fResMgr->LoadPageKeys(page);
            plKeyCollector collector(fKeys);
            page->IterateKeys(&collector);
        }

        return true;
    }
};

//// LoadAndHoldAgeKeys //////////////////////////////////////////////////////

void plResManager::LoadAgeKeys(const ST::string& age)
{
    hsAssert(!age.empty(), "age is nil");
    HeldAgeKeyMap::const_iterator it = fHeldAgeKeys.find(age);
    if (it != fHeldAgeKeys.end())
    {
        kResMgrLog(1, ILog(1, "Reffing age keys for age {}", age));
        hsStatusMessageF("*** Reffing age keys for age {} ***", age);
        plResAgeHolder* holder = it->second;
        holder->Ref();
    }
    else
    {
        kResMgrLog(1, ILog(1, "Loading age keys for age {}", age));
        hsStatusMessageF("*** Loading age keys for age {} ***", age);

        plResAgeHolder* holder = new plResAgeHolder(age);
        fHeldAgeKeys[age] = holder;
        // Go find pages that match this age, load the keys, and ref them all
        plResHolderIterator iter(age, holder->fKeys, this);
        IterateAllPages(&iter);
    }
}

//// DropAgeKeys /////////////////////////////////////////////////////////////

void plResManager::DropAgeKeys(const ST::string& age)
{
    HeldAgeKeyMap::iterator it = fHeldAgeKeys.find(age);
    if (it != fHeldAgeKeys.end())
    {
        plResAgeHolder* holder = it->second;
        if (holder->RefCnt() == 1)
        {
            // Found it!
            kResMgrLog(1, ILog(1, "Dropping held age keys for age {}", age));
            fHeldAgeKeys.erase(it);
        }
        else
        {
            kResMgrLog(1, ILog(1, "Unreffing age keys for age {}", age));
        }

        holder->UnRef();
    }
}

//// IDropAllAgeKeys /////////////////////////////////////////////////////////

void plResManager::IDropAllAgeKeys()
{
    kResMgrLog(1, ILog(1, "Dropping any remaining age keys"));
    for (HeldAgeKeyMap::iterator it = fHeldAgeKeys.begin(); it != fHeldAgeKeys.end(); ++it)
    {
        plResAgeHolder* holder = it->second;
        kResMgrLog(1, ILog(1, "Dropping age keys for age {}", holder->fAge));
        while (holder->RefCnt() > 1)
            holder->UnRef();
        holder->UnRef();    // deletes holder
    }
}

//// PageInRoom //////////////////////////////////////////////////////////////
//  Normal finds will have to potentially reload all the keys for a page, but
//  paging in this way will avoid having to reload keys every single find
//  during a load--we load all the keys once, ref them so they're in, then
//  do the entire load and unref when we're done.
//
//  The objClassToRef parameter is a bit tricky. Basically, you assume that
//  there's one object in the page that you're loading based off of (say, a
//  sceneNode). That's the object that'll get reffed by the refMsg passed in.
//  The only reason we abstract it is so we can keep plResManager from being
//  dependent on any particular class type.
//
//  This function is not guaranteed to be synchronous, so you better wait for 
//  the refMsg to be sent before you assume it's done.

class plOurRefferAndFinder : public plRegistryKeyIterator
{
    std::vector<plKey> &fRefArray;
    uint16_t           fClassToFind;
    plKey              &fFoundKey;

    public:

        plOurRefferAndFinder(std::vector<plKey> &refArray, uint16_t classToFind, plKey &foundKey)
                : fRefArray( refArray ), fClassToFind( classToFind ), fFoundKey( foundKey ) { }

        bool EatKey(const plKey& key) override
        {
            // This is cute. Thanks to our new plKey smart pointers, all we have to
            // do is append the key to our ref array. This automatically guarantees us
            // an extra ref on the key, which is what we're trying to do. Go figure.
            fRefArray.emplace_back(key);

            // Also do our find
            if( key->GetUoid().GetClassType() == fClassToFind )
                fFoundKey = key;
            
            return true;
        }
};

void plResManager::PageInRoom(const plLocation& page, uint16_t objClassToRef, plRefMsg* refMsg)
{
    uint64_t readRoomTime = 0;
    if (fLogReadTimes)
        readRoomTime = hsTimer::GetTicks();

    plSynchEnabler ps(false);   // disable dirty tracking while paging in

    kResMgrLog(1, ILog(1, "Paging in room 0x{x}...", page.GetSequenceNumber()));

    // Step 0: Find the pageNode
    plRegistryPageNode* pageNode = FindPage(page);
    if (pageNode == nullptr)
    {
        kResMgrLog(1, ILog(1, "...Page not found!"));
        hsAssert(false, "Invalid location given to PageInRoom()");
        return;
    }

    kResMgrLog(2, ILog(2, "...Found, page is ID'd as {}>{}", pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage()));

    // Step 0.5: Verify the page, just to make sure we really should be loading it
    PageCond cond = pageNode->GetPageCondition();
    if (cond != kPageOk)
    {
        ST::string condStr = ST_LITERAL("Checksum invalid");
        if (cond == kPageTooNew) {
            condStr = ST_LITERAL("Page Version too new");
        } else if (cond == kPageOutOfDate) {
            condStr = ST_LITERAL("Page Version out of date");
        }

        kResMgrLog(1, ILog(1, "...IGNORING pageIn request; verification failed! ({})", condStr));

        ST::string msg = ST::format("Data Problem: Age:{}  Page:{}  Error:{}",
            pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage(), condStr);
        hsMessageBox(msg, ST_LITERAL("Error"), hsMessageBoxNormal, hsMessageBoxIconError);

        hsRefCnt_SafeUnRef(refMsg);
        return;
    }

    // Step 0.9: Open the stream on this page, so it remains open for the entire loading process
    pageNode->OpenStream();

    // Step 1: We force a load on all the keys in the given page
    kResMgrLog(2, ILog(2, "...Loading page keys..."));
    LoadPageKeys(pageNode);

    // Step 2: Now ref all the keys in that page, every single one. This lets us unref 
    // (and thus potentially delete) them later. Note that we also use this for our find.
    kResMgrLog(2, ILog(2, "...Reffing keys..."));
    plKey objKey;
    std::vector<plKey> keyRefList;
    plOurRefferAndFinder reffer(keyRefList, objClassToRef, objKey);
    pageNode->IterateKeys(&reffer);

    // Step 3: Do our load
    if (objKey == nullptr)
    {
        kResMgrLog(1, ILog(1, "...SceneNode not found to base page-in op on. Aborting..."));
        // This is coming up a lot lately; too intrusive to be an assert.
        // hsAssert( false, "No object found on which to base our PageInRoom()" );
        pageNode->CloseStream();
        return;
    }

    // Forces a load
    kResMgrLog(2, ILog(2, "...Forcing load via sceneNode..."));
    objKey->VerifyLoaded();
    
    // Step 4: Unref the keys. This'll make the unused ones go away again. And guess what,
    // since we just have an array of keys, all we have to do to do this is clear the array.
    // Note that since objKey is a plKey, our object that we loaded will have an extra ref...
    // Scary, huh?
    kResMgrLog(2, ILog(2, "...Dumping extra key refs..."));
    keyRefList.clear();

    // Step 5: Ref the object
    kResMgrLog(2, ILog(2, "...Dispatching refMessage..."));
    AddViaNotify(objKey, refMsg, plRefFlags::kActiveRef);

    // Step 5.9: Close the page stream
    pageNode->CloseStream();

    // All done!
    kResMgrLog(1, ILog(1, "...Page in complete!"));

    if (fLogReadTimes)
    {
        readRoomTime = hsTimer::GetTicks() - readRoomTime;

        plStatusLog::AddLineSF("readtimings.log", plStatusLog::kWhite, "----- Reading page {}>{} took {.1f} ms",
            pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage(),
            hsTimer::GetMilliSeconds<float>(readRoomTime));
    }
}

class plPageInAgeIter : public plRegistryPageIterator
{
private:
    plKey       fDestKey;
    ST::string  fAgeName;
    std::vector<plLocation> fLocations;
    plAgeDescription fAgeDesc;

public:
    plPageInAgeIter(plKey destKey, const plFileName& dataPath, const ST::string &ageName)
        : fDestKey(std::move(destKey)), fAgeName(ageName)
    {
        plFileName ageFile = plFileName::Join(dataPath, ST::format("{}.age", ageName));
        if (!fAgeDesc.ReadFromFile(ageFile))
            kResMgrLog(3, ILog(3, "PageInAge: Failed to load '{}'", ageFile.AsString()));
    }

    ~plPageInAgeIter()
    {
        plClientMsg* pMsg1 = new plClientMsg(plClientMsg::kLoadRoomHold);
        for (size_t i = 0; i < fLocations.size(); i++)
            pMsg1->AddRoomLoc(fLocations[i]);
        pMsg1->Send(fDestKey);
    }

    bool EatPage(plRegistryPageNode* page) override
    {
        if (page->GetPageInfo().GetAge().compare_i(fAgeName) == 0) {
            if (fAgeDesc.FindPage(page->GetPageInfo().GetPage()))
                fLocations.push_back(page->GetPageInfo().GetLocation());
        }
        return true;
    }
};

// PageInAge is intended for bulk global ages, like GlobalAnimations or GlobalClothing
// that store a lot of data we always want available. (Used to be known as PageInHold)
void plResManager::PageInAge(const ST::string &age)
{
    plSynchEnabler ps(false);   // disable dirty tracking while paging in
    plUoid lu(kClient_KEY);
    plKey clientKey = hsgResMgr::ResMgr()->FindKey(lu);

    // Tell the client to load all the keys for this age, to make the loading process work better
    plClientMsg *loadAgeKeysMsg = new plClientMsg(plClientMsg::kLoadAgeKeys);
    loadAgeKeysMsg->SetAgeName(age);
    loadAgeKeysMsg->Send(clientKey);

    // Then iterate through each room in the age. The iterator will send the load message
    // off on destruction.
    plPageInAgeIter iter(clientKey, fDataPath, age);
    IterateAllPages(&iter);
}

//// VerifyPages /////////////////////////////////////////////////////////////
//  Runs through all the pages and ensures they are all up-to-date in version
//  numbers and that no out-of-date objects exist in them

bool plResManager::VerifyPages()
{
    std::vector<plRegistryPageNode*> invalidPages, newerPages;
    PageMap::iterator it = fAllPages.begin();

    // Step 1: verify major/minor version changes
    if (plResMgrSettings::Get().GetFilterNewerPageVersions() ||
        plResMgrSettings::Get().GetFilterOlderPageVersions())
    {
        while (it != fAllPages.end())
        {
            plLocation loc = it->first;
            plRegistryPageNode* page = it->second;
            ++it;

            if (page->GetPageCondition() == kPageTooNew && plResMgrSettings::Get().GetFilterNewerPageVersions())
            {
                newerPages.emplace_back(page);
                fAllPages.erase(loc);
            }
            else if (
                (page->GetPageCondition() == kPageCorrupt ||
                page->GetPageCondition() == kPageOutOfDate)
                && plResMgrSettings::Get().GetFilterOlderPageVersions())
            {
                invalidPages.emplace_back(page);
                fAllPages.erase(loc);
            }
        }
    }

    // Handle all our invalid pages now
    if (!invalidPages.empty())
    {
        if (!IDeleteBadPages(invalidPages, false))
            return false;
    }

    // Warn about newer pages
    if (!newerPages.empty())
    {
        if (!IWarnNewerPages(newerPages))
            return false;
    }

    // Step 2 of verification: make sure no sequence numbers conflict
    for (auto badPage : fConflictingPages) {
        fAllPages.erase(badPage->GetPageInfo().GetLocation());
        invalidPages.emplace_back(badPage);
    }
    fConflictingPages.clear();

    // Redo our loaded pages list, since Verify() might force the page's keys to load or unload
    fLoadedPages.clear();
    it = fAllPages.begin();
    while (it != fAllPages.end())
    {
        plRegistryPageNode* page = it->second;
        ++it;

        if (page->IsLoaded())
            fLoadedPages.insert(page);
    }

    // Handle all our conflicting pages now
    if (!invalidPages.empty())
        return IDeleteBadPages(invalidPages, true);

    return true;
}

//// IDeleteBadPages /////////////////////////////////////////////////////////
//  Given an array of pages that are invalid (major version out-of-date or
//  whatnot), asks the user what we should do about them.

static ST::string ICatPageNames(std::vector<plRegistryPageNode*>& pages, const ST::string& msg)
{
    ST::string_stream ss;
    ss << msg;

    for (size_t i = 0; i < pages.size(); i++)
    {
        if (i >= 25)
        {
            ss << ST_LITERAL("...\n");
            break;
        }

        ST::string pageFile = pages[i]->GetPagePath().GetFileName();
        ss << pageFile << '\n';
    }

    return ss.to_string();
}

bool plResManager::IDeleteBadPages(std::vector<plRegistryPageNode*>& invalidPages, bool conflictingSeqNums)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (!hsMessageBox_SuppressPrompts)
    {
        ST::string msg;

        // Prompt what to do
        if (conflictingSeqNums)
            msg = ST_LITERAL("The following pages have conflicting sequence numbers. This usually happens when "
                             "you copy data files between machines that had random sequence numbers assigned at "
                             "export. To avoid crashing, these pages will be deleted:\n\n");
        else
            msg = ST_LITERAL("The following pages are out of date and will be deleted:\n\n");

        msg = ICatPageNames(invalidPages, msg);

        hsMessageBox(msg, ST_LITERAL("Warning"), hsMessageBoxNormal);
    }
#endif // PLASMA_EXTERNAL_RELEASE

    // Delete 'em
    for (plRegistryPageNode* page : invalidPages)
    {
        page->DeleteSource();
        delete page;
    }
    invalidPages.clear();

    fLastFoundPage = nullptr;

    return true;
}

//// IWarnNewerPages /////////////////////////////////////////////////////////
//  Given an array of pages that are newer (minor or major version are newer
//  than the "current" one), warns the user about them but does nothing to
//  them.

bool plResManager::IWarnNewerPages(std::vector<plRegistryPageNode*> &newerPages)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (!hsMessageBox_SuppressPrompts)
    {
        // Prompt what to do
        ST::string msg = ST_LITERAL("The following pages have newer version numbers than this client and cannot be \nloaded. "
                                    "They will be ignored but their files will NOT be deleted:\n\n");

        msg = ICatPageNames(newerPages, msg);

        hsMessageBox(msg, ST_LITERAL("Warning"), hsMessageBoxNormal);
    }
#endif // PLASMA_EXTERNAL_RELEASE


    // Not deleting the files, just delete them from memory
    for (plRegistryPageNode* page : newerPages)
        delete page;
    newerPages.clear();

    fLastFoundPage = nullptr;

    return true;
}

//// plOurReffer /////////////////////////////////////////////////////////////
//  Our little reffer key iterator

class plOurReffer : public plRegistryKeyIterator
{
protected:
    std::vector<plKey> fRefArray;

public:
    plOurReffer() {}
    virtual ~plOurReffer() { UnRef(); }

    void UnRef() { fRefArray.clear(); }

    bool EatKey(const plKey& key) override
    {
        // This is cute. Thanks to our new plKey smart pointers, all we have to
        // do is append the key to our ref array. This automatically guarantees us
        // an extra ref on the key, which is what we're trying to do. Go figure.
        fRefArray.emplace_back(key);

        return true;
    }
};

void plResManager::DumpUnusedKeys(plRegistryPageNode* page) const
{
    plOurReffer reffer;
    page->IterateKeys(&reffer);
}

plRegistryPageNode* plResManager::CreatePage(const plLocation& location, const ST::string& age, const ST::string& page)
{
    plRegistryPageNode* pageNode = new plRegistryPageNode(location, age, page, fDataPath);
    fAllPages[location] = pageNode;

    return pageNode;
}

//// AddPage /////////////////////////////////////////////////////////////////

void plResManager::AddPage(plRegistryPageNode* page)
{
    plLocation loc = page->GetPageInfo().GetLocation();

    fAllPages[loc] = page;
    if (page->IsLoaded())
        fLoadedPages.insert(page);
}

//// LoadPageKeys ///////////////////////////////////////////////////////////

void plResManager::LoadPageKeys(plRegistryPageNode* pageNode)
{
    if (pageNode->IsFullyLoaded())
        return;

    // Load it and add it to the loaded list
    pageNode->LoadKeys();

    if (fPageListLock == 0)
        fLoadedPages.insert(pageNode);
    else
        fPagesNeedCleanup = true;
}

//// sIReportLeak ////////////////////////////////////////////////////////////
//  Handy tiny function here

static void sIReportLeak(plKeyImp* key, plRegistryPageNode* page)
{
    class plKeyImpRef : public plKeyImp
    {
    public:
        uint16_t GetRefCnt() const { return fRefCount; }
    };

    static bool alreadyDone = false;
    static plRegistryPageNode* lastPage;

    if (page != nullptr)
        lastPage = page;

    if (key == nullptr)
    {
        alreadyDone = false;
        return;
    }

    if (!alreadyDone)
    {
        // Print out page header
        hsStatusMessageF("\tLeaks in page {}>{}[{08x}]:", lastPage->GetPageInfo().GetAge(), lastPage->GetPageInfo().GetPage(), lastPage->GetPageInfo().GetLocation().GetSequenceNumber());
        alreadyDone = true;
    }

    int refsLeft = ((plKeyImpRef*)key)->GetRefCnt() - 1;
    if (refsLeft == 0)
        return;

    ST::string_stream ss;
    ss << "\t\t" << plFactory::GetNameOfClass(key->GetUoid().GetClassType()) << ": ";
    ss << key->GetUoid().StringIze() << " ";
    if (key->ObjectIsLoaded())
        ss << "- " << key->GetDataLen() << " bytes - " << refsLeft << " refs left";
    else
        ss << "(key only, " << refsLeft << " refs left)";
    hsStatusMessage(ss.to_string());
}

//// UnloadPageObjects ///////////////////////////////////////////////////////
//  Unloads all the objects in a given page. Once this is complete, all 
//  object pointers for every key in the page *should* be nil. Note that we're
//  given a hint class index to start with (like plSceneNode) that should do
//  most of the work for us via unreffing.
//
//  Update 5.20: since there are so many problems with doing this, don't
//               delete the objects, just print out a memleak report. -mcn

void plResManager::UnloadPageObjects(plRegistryPageNode* pageNode, uint16_t classIndexHint)
{
    if (!pageNode->IsLoaded())
        return;

    class plUnloadObjectsIterator : public plRegistryKeyIterator
    {
    public:
        bool EatKey(const plKey& key) override
        {
            sIReportLeak(plKeyImp::GetFromKey(key), nullptr);
            return true;
        }
    };

    sIReportLeak(nullptr, pageNode);

    plUnloadObjectsIterator iterator;

    if (classIndexHint != static_cast<uint16_t>(-1))
        pageNode->IterateKeys(&iterator, classIndexHint);
    else
        pageNode->IterateKeys(&iterator);
}

//// FindPage ////////////////////////////////////////////////////////////////

plRegistryPageNode* plResManager::FindPage(const plLocation& location) const
{
    // Quick optimization
    if (fLastFoundPage != nullptr && fLastFoundPage->GetPageInfo().GetLocation() == location)
        return fLastFoundPage;

    PageMap::const_iterator it = fAllPages.find(location);
    if (it != fAllPages.end())
    {
        fLastFoundPage = it->second;
        return it->second;
    }

    return nullptr;
}

//// FindPage ////////////////////////////////////////////////////////////////

plRegistryPageNode* plResManager::FindPage(const ST::string& age, const ST::string& page) const
{
    PageMap::const_iterator it;
    for (it = fAllPages.begin(); it != fAllPages.end(); ++it)
    {
        const plPageInfo& info = (it->second)->GetPageInfo();
        if (info.GetAge().compare_i(age) == 0 && info.GetPage().compare_i(page) == 0)
            return it->second;
    }

    return nullptr;
}

//////////////////////////////////////////////////////////////////////////////
//// Key Operations //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// AddKey //////////////////////////////////////////////////////////////////
//  Adds a key to the registry. Assumes uoid already set.

void plResManager::AddKey(plKeyImp* key)
{
    plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
    if (page == nullptr)
        return;

    page->AddKey(key);
    fLoadedPages.insert(page);
}

void plResManager::IKeyReffed(plKeyImp* key)
{
    plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
    if (page == nullptr)
    {
        hsAssert(0, "Couldn't find page that key belongs to");
        return;
    }

    page->SetKeyUsed(key);
}

void plResManager::IKeyUnreffed(plKeyImp* key)
{
    plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
    if (!page)
    {
        hsAssert(0, "Couldn't find page that key belongs to");
        return;
    }

    if (page->SetKeyUnused(key))
    {
        if (!page->IsLoaded())
        {
            if (fPageListLock == 0)
                fLoadedPages.erase(page);
            else
                fPagesNeedCleanup = true;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
//// Iterator Functions //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// IterateKeys Helper Class ////////////////////////////////////////////////

class plKeyIterEater : public plRegistryPageIterator
{
protected:
    plRegistryKeyIterator* fIter;

public:
    plKeyIterEater(plRegistryKeyIterator* iter) : fIter(iter) {}
    bool EatPage(plRegistryPageNode* keyNode) override
    {
        return keyNode->IterateKeys(fIter);
    }
};

//// IterateKeys /////////////////////////////////////////////////////////////

bool plResManager::IterateKeys(plRegistryKeyIterator* iterator)
{
    plKeyIterEater myEater(iterator);
    return IteratePages(&myEater);
}

bool plResManager::IterateKeys(plRegistryKeyIterator* iterator, const plLocation& pageToRestrictTo)
{
    plRegistryPageNode* page = FindPage(pageToRestrictTo);
    if (page == nullptr)
    {
        hsAssert(false, "Page not found to iterate through");
        return false;
    }

    plKeyIterEater myEater(iterator);
    return myEater.EatPage(page);
}

//// IteratePages ////////////////////////////////////////////////////////////
//  Iterate through all LOADED pages

bool plResManager::IteratePages(plRegistryPageIterator* iterator, const ST::string& ageToRestrictTo)
{
    ILockPages();

    PageSet::const_iterator it;
    for (it = fLoadedPages.begin(); it != fLoadedPages.end(); it++)
    {
        plRegistryPageNode* page = *it;
        if (page->GetPageInfo().GetLocation() == plLocation::kGlobalFixedLoc)
            continue;

        if (ageToRestrictTo.empty() || page->GetPageInfo().GetAge().compare_i(ageToRestrictTo) == 0)
        {
            if (!iterator->EatPage(page))
            {
                IUnlockPages();
                return false;
            }
        }
    }

    IUnlockPages();

    return true;
}

//// IterateAllPages /////////////////////////////////////////////////////////
//  Iterate through ALL pages

bool plResManager::IterateAllPages(plRegistryPageIterator* iterator)
{
    ILockPages();

    PageMap::const_iterator it;
    for (it = fAllPages.begin(); it != fAllPages.end(); ++it)
    {
        if (it->first == plLocation::kGlobalFixedLoc)
            continue;

        plRegistryPageNode* page = it->second;

        if (!iterator->EatPage(page))
        {
            IUnlockPages();
            return false;
        }
    }

    IUnlockPages();

    return true;
}

//// ILockPages //////////////////////////////////////////////////////////////
//  See, when we iterate through pages, our iterate function might decide to
//  move pages, either explicitly through loads or implicitly through key
//  deletions. So, before we iterate, we lock 'em all so they won't move,
//  then unlock and move them to their proper places at the end.

void plResManager::ILockPages()
{
    if (fPageListLock == 0)
        fPagesNeedCleanup = false;

    fPageListLock++;
}

//// IUnlockPages ////////////////////////////////////////////////////////////

void plResManager::IUnlockPages()
{
    fPageListLock--;
    if (fPageListLock == 0 && fPagesNeedCleanup)
    {
        fPagesNeedCleanup = false;

        fLoadedPages.clear();

        PageMap::const_iterator it;
        for (it = fAllPages.begin(); it != fAllPages.end(); ++it)
        {
            plRegistryPageNode* page = it->second;
            if (page->IsLoaded())
                fLoadedPages.insert(page);
        }
    }
}

// Defined here 'cause release build hates it defined in settings.h for some reason
plResMgrSettings& plResMgrSettings::Get()
{
    static plResMgrSettings fSettings;
    return fSettings;
}
