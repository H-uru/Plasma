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
#include "plResManager.h"
#include "plRegistryNode.h"
#include "plResManagerHelper.h"
#include "plResMgrSettings.h"
#include "plLocalization.h"
#include "hsSTLStream.h"

#include "hsTimer.h"
#include "../pnTimer/plTimerCallbackManager.h"
#include "hsStlUtils.h"

#include "../plScene/plSceneNode.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../pnDispatch/plDispatch.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnMessage/plRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../plFile/hsFiles.h"
#include "../plFile/plFileUtils.h"
#include "../pnFactory/plCreator.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../pnNetCommon/plNetApp.h"

hsBool gDataServerLocal = false;

/// Logging #define for easier use
#define kResMgrLog(level, log) if (plResMgrSettings::Get().GetLoggingLevel() >= level) log

static void ILog(UInt8 level, const char* format, ...)
{
	static plStatusLog* log = plStatusLogMgr::GetInstance().CreateStatusLog
		(
		plStatusLogMgr::kDefaultNumLines,
		"resources.log",
		plStatusLog::kFilledBackground | plStatusLog::kDeleteForMe
		);

	va_list arguments;
	va_start(arguments, format);

	UInt32 color = 0;
	switch (level)
	{
	case 1:	color = 0xffffffff; break;
	case 2:	color = 0xff8080ff; break;
	case 3:	color = 0xffffff80; break;
	case 4:	color = 0xff8080ff; break;
	}

	log->AddLineV(color, format, arguments);
}

plResManager::plResManager():
	fInited(false),
	fPageOutHint(0),
	fDispatch(nil),
	fReadingObject( false ),
	fCurCloneID(0),
	fCurClonePlayerID(0),
	fCloningCounter(0),
	fProgressProc(nil),
	fMyHelper(nil),
	fLogReadTimes(false),
	fPageListLock(0),
	fPagesNeedCleanup(false),
	fLastFoundPage(nil)
{
#ifdef HS_DEBUGGING
	plFactory::Validate(hsKeyedObject::Index());
#endif
}

plResManager::~plResManager()
{
	// verify shutDown
	hsAssert(!fInited,"ResMgr not shutdown");
}

hsBool plResManager::IInit()
{
	if (fInited)
		return true;
	fInited = true;

	kResMgrLog(1, ILog(1, "Initializing resManager..."));

	if (plResMgrSettings::Get().GetLoadPagesOnInit())
	{
		// We want to go through all the data files in our data path and add new
		// plRegistryPageNodes to the regTree for each
		hsFolderIterator pathIterator(fDataPath.c_str());
		while (pathIterator.NextFileSuffix(".prp"))
		{
			char fileName[kFolderIterator_MaxPath];
			pathIterator.GetPathAndName(fileName);

			plRegistryPageNode* node = TRACKED_NEW plRegistryPageNode(fileName);
			fAllPages.insert(node);
		}
	}

	// Special case: we always create pages for the predefined pages
	CreatePage(plLocation::kGlobalFixedLoc, "Global", "FixedKeys");

	hsAssert(!fDispatch, "Dispatch already set");
	fDispatch = TRACKED_NEW plDispatch;
	
	plgTimerCallbackMgr::Init(); 

	// Init our helper
	fMyHelper = TRACKED_NEW plResManagerHelper(this);
	fMyHelper->Init();
	hsAssert(fMyHelper->GetKey() != nil, "ResManager helper didn't init properly!" );

	kResMgrLog(1, ILog(1, "   ...Init was successful!"));

	return true; 
}

hsBool plResManager::IReset()	// Used to Re-Export (number of times)
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
	fMyHelper->Shutdown();	// This will call UnregisterAs(), which will delete itself
	fMyHelper = nil;

	// TimerCallbackMgr is a fixed-keyed object, so needs to shut down before the registry
	plgTimerCallbackMgr::Shutdown(); 

	// Destroy the dispatch. Note that we do this before the registry so that any lingering messages
	// can free up keys properly
	hsRefCnt_SafeUnRef(fDispatch);
	fDispatch = nil;

	// Just before we shut down the registry, page out any keys that still exist.
	// (They shouldn't... they're baaaaaad.)
	IPageOutSceneNodes(true);

	// Shut down the registry (finally!)
	ILockPages();

	PageSet::const_iterator it;
	for (it = fAllPages.begin(); it != fAllPages.end(); it++)
		delete *it;
	fAllPages.clear();
	fLoadedPages.clear();

	IUnlockPages();

	fLastFoundPage = nil;

	kResMgrLog(1, ILog(1, "   ...Shutdown successful!"));

	fInited = false;
}

void plResManager::AddSinglePage(const char* pagePath)
{
	plRegistryPageNode* node = TRACKED_NEW plRegistryPageNode(pagePath);
	AddPage(node);
}

plRegistryPageNode* plResManager::FindSinglePage(const char* path) const
{
	PageSet::const_iterator it;
	for (it = fAllPages.begin(); it != fAllPages.end(); it++)
	{
		if (hsStrCaseEQ((*it)->GetPagePath(), path))
			return *it;
	}

	return nil;
}

void plResManager::RemoveSinglePage(const char* path)
{
	plRegistryPageNode* node = FindSinglePage(path);
	if (node)
	{
		fAllPages.erase(node);
		delete node;
	}
}

plDispatchBase *plResManager::Dispatch()
{
	return fDispatch;
}


void plResManager::LogReadTimes(hsBool logReadTimes)
{
	fLogReadTimes = logReadTimes;
	if (fLogReadTimes)
	{
		plStatusLog::AddLineS("readtimings.log", plStatusLog::kWhite, "Created readtimings log");
	}
}

hsKeyedObject* plResManager::IGetSharedObject(plKeyImp* pKey)
{
	plKeyImp* origKey = (plKeyImp*)pKey->GetCloneOwner();

	// Find the first non-nil key and ask it to clone itself
	UInt32 count = origKey->GetNumClones();
	for (UInt32 i = 0; i < count; i++)
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

	return nil;
}

//// ReadObject /////////////////////////////////////////////////////////////
//	Given a key, goes off and reads in the actual object from its source
hsBool plResManager::ReadObject(plKeyImp* key)
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

	kResMgrLog(4, ILog(4, "   ...Opening page data stream for location 0x%x...", key->GetUoid().GetLocation()));
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

		for (int i = 0; i < children.size(); i++)
		{
			plKey childKey = children[i];
			childKey->VerifyLoaded();
		}
	}

	// we're done loading, and all our children are too, so send the notify
	key->NotifyCreated();

	pageNode->CloseStream();

	return ret;
}

hsBool plResManager::IReadObject(plKeyImp* pKey, hsStream *stream)
{
	static UInt64 totalTime = 0;

	UInt64 startTotalTime = totalTime;
	UInt64 startTime = 0;
	if (fLogReadTimes)
		startTime = hsTimer::GetFullTickCount();

	hsKeyedObject* ko = nil;

	hsAssert(pKey, "Null Key");
	if (pKey->GetUoid().GetLoadMask().DontLoad())
		return nil;

	hsAssert(pKey->GetStartPos() != UInt32(-1), "Missing StartPos");
	hsAssert(pKey->GetDataLen() != UInt32(-1), "Missing Data Length");

	if (pKey->GetStartPos() == UInt32(-1) || pKey->GetDataLen() == UInt32(-1))
		return false; // Try to recover from this by just not reading an object

	kResMgrLog(3, ILog(3, "   Reading object %s::%s", plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()), pKey->GetUoid().GetObjectName()));

	const plUoid& uoid = pKey->GetUoid();

	bool isClone = uoid.IsClone();

	kResMgrLog(4, ILog(4, "   ...is%s a clone", isClone ? "" : " not"));

	// If we're loading the root object of a clone (the object for the key that
	// was actually cloned), set up the global cloning flags so any child objects
	// read in will get them.  Also turn off synching until the object is fully
	// loaded, so we don't send out any partially loaded state.
	bool setClone = false;
	if (isClone && fCurCloneID != uoid.GetCloneID())
	{
		kResMgrLog(4, ILog(4, "   ...fCurCloneID = %d, uoid's cloneID = %d", fCurCloneID, uoid.GetCloneID()));

		if (fCurCloneID != 0)
		{
			hsAssert(false, "Recursive clone");
			kResMgrLog(3, ILog(3, "   ...RECURSIVE CLONE DETECTED. ABORTING READ..."));
			return false;
		}
		fCurClonePlayerID = uoid.GetClonePlayerID();
		fCurCloneID = uoid.GetCloneID();
		setClone = true;

		kResMgrLog(4, ILog(4, "   ...now fCurCloneID = %d, fCurClonePlayerID = %d", fCurCloneID, fCurClonePlayerID));
	}

	// If this is a clone key, try and get the original object to give us a clone
	if (isClone)
	{
		kResMgrLog(4, ILog(4, "   ...Trying to get shared object..."));
		ko = IGetSharedObject(pKey);
		kResMgrLog(4, ILog(4, "   ...IGetSharedObject() %s", (ko != nil) ? "succeeded" : "failed"));
	}

	// If we couldn't share the object, read in a fresh copy
	if (!ko)
	{
		stream->SetPosition(pKey->GetStartPos());
		kResMgrLog(4, ILog(4, "   ...Reading from position %d bytes...", pKey->GetStartPos()));

		plCreatable* cre = ReadCreatable(stream);
		hsAssert(cre, "Could not Create Object");
		if (cre)
		{	
			ko = hsKeyedObject::ConvertNoRef(cre);

			if (ko != nil)
				kResMgrLog(4, ILog(4, "   ...Creatable read and valid"));
			else
				kResMgrLog(3, ILog(3, "   ...Creatable read from stream not keyed object!"));

			if (fProgressProc != nil)
				fProgressProc(plKey::Make(pKey));
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

	kResMgrLog(4, ILog(4, "   ...Read complete for object %s::%s", plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()), pKey->GetUoid().GetObjectName()));

	if (fLogReadTimes)
	{
		UInt64 ourTime = hsTimer::GetFullTickCount() - startTime;
		UInt64 childTime = totalTime - startTotalTime;
		ourTime -= childTime;

		plStatusLog::AddLineS("readtimings.log", plStatusLog::kWhite, "%s, %s, %u, %.1f",
			pKey->GetUoid().GetObjectName(),
			plFactory::GetNameOfClass(pKey->GetUoid().GetClassType()),
			pKey->GetDataLen(),
			hsTimer::FullTicksToMs(ourTime));

		totalTime += (hsTimer::GetFullTickCount() - startTime) - childTime;
	}

	return (ko != nil);
}

//// plPageOutIterator ///////////////////////////////////////////////////////
//	See below function
class plPageOutIterator : public plRegistryPageIterator
{
protected:
	plResManager* fResMgr;
	UInt16		fHint;

public:
	plPageOutIterator(plResManager* resMgr, UInt16 hint) : fResMgr(resMgr), fHint(hint)
	{
		fResMgr->IterateAllPages(this);
	}

	virtual hsBool EatPage(plRegistryPageNode* page)
	{
		fResMgr->UnloadPageObjects(page, fHint);
		return true;
	}
};

#if HS_BUILD_FOR_UNIX
static void	sLeakReportRedirectFn( const char message[] )
{
	hsUNIXStream stream;
	stream.Open( "resMgrMemLeaks.txt", "at" );
	stream.WriteString( message );
	stream.Close();
}
static bool sFirstTime = true;
#endif

// Just the scene nodes (and objects referenced by the node... and so on)
void plResManager::IPageOutSceneNodes(hsBool forceAll)
{
	plSynchEnabler ps(false);	// disable dirty tracking while paging out

#if HS_BUILD_FOR_UNIX
	if (sFirstTime)
	{
		hsUNIXStream stream;
		stream.Open("resMgrMemLeaks.txt", "wt");
		stream.Close();
		sFirstTime = false;
	}
	hsDebugMessageProc oldProc = hsSetStatusMessageProc(sLeakReportRedirectFn);
#endif

	if (forceAll)
	{
		hsStatusMessage( "--- plResManager Object Leak Report (BEGIN) ---" );
		plPageOutIterator iter(this, UInt16(-1));
		hsStatusMessage( "--- plResManager Object Leak Report (END) ---" );
	}
	else
	{
		plPageOutIterator iter(this, fPageOutHint);
	}

#if HS_BUILD_FOR_UNIX
	hsSetStatusMessageProc( oldProc );
#endif
}

//// FindKey /////////////////////////////////////////////////////////////////

inline plKeyImp* IFindKeyLocalized(const plUoid& uoid, plRegistryPageNode* page)
{
	const char* objectName = uoid.GetObjectName();

	// If we're running localized, try to find a localized version first
	if (plLocalization::IsLocalized())
	{
		char localName[256];
		if (plLocalization::GetLocalized(objectName, localName))
		{
			plKeyImp* localKey = page->FindKey(uoid.GetClassType(), localName);
			if (localKey != nil)
				return localKey;
		}
	}

	// Try to find the non-localized version
	return page->FindKey(uoid);
}

plKey plResManager::FindOriginalKey(const plUoid& uoid)
{
	plKey key;
	plKeyImp* foundKey = nil;

	plRegistryPageNode* page = FindPage(uoid.GetLocation());
	if (page == nil)
		return key;

	// Try our find first, without loading
	foundKey = IFindKeyLocalized(uoid, page);
	if (foundKey != nil)
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
		foundKey = TRACKED_NEW plKeyImp(uoid, UInt32(-1), UInt32(0));
		key = plKey::Make(foundKey);
	}

	// OK, find didn't work. Can we load and try again?
	if (!key && !page->IsFullyLoaded())
	{
		// Tell the resManager's helper to load and hold our page keys temporarily
		plResManagerHelper::GetInstance()->LoadAndHoldPageKeys(page);

		// Try again
		foundKey = IFindKeyLocalized(uoid, page);
		if (foundKey != nil)
			key = plKey::Make(foundKey);
	}
	return key;
}

plKey plResManager::FindKey(const plUoid& uoid)
{
	plKey key = FindOriginalKey(uoid);

	// If we're looking for a clone, get the clone instead of the original
	if (key && uoid.IsClone())
		key = ((plKeyImp*)key)->GetClone(uoid.GetClonePlayerID(), uoid.GetCloneID());

	return key;
}

const plLocation& plResManager::FindLocation(const char* age, const char* page) const
{
	static plLocation invalidLoc;

	plRegistryPageNode* pageNode = FindPage(age, page);
	if (pageNode)
		return pageNode->GetPageInfo().GetLocation();

	return invalidLoc;
}

const void plResManager::GetLocationStrings(const plLocation& loc, char* ageBuffer, char* pageBuffer) const
{
	plRegistryPageNode* page = FindPage(loc);
	const plPageInfo& info = page->GetPageInfo();

	// Those buffers better be big enough...
	if (ageBuffer)
		hsStrcpy(ageBuffer, info.GetAge());
	if (pageBuffer)
		hsStrcpy(pageBuffer, info.GetPage());
}

hsBool plResManager::AddViaNotify(plRefMsg* msg, plRefFlags::Type flags)
{
	hsAssert(msg && msg->GetRef() && msg->GetRef()->GetKey(), "Improperly filled out ref message");
	plKey key = msg->GetRef()->GetKey();		// for linux build
	return AddViaNotify(key, msg, flags);
}

hsBool plResManager::AddViaNotify(const plKey &key, plRefMsg* msg, plRefFlags::Type flags)
{
	hsAssert(key, "Can't add without a Key");
	if (!key)
	{
		hsRefCnt_SafeUnRef(msg);
		return false;
	}

	((plKeyImp*)key)->SetupNotify(msg,flags);
	
	if (flags != plRefFlags::kPassiveRef)
	{
		hsKeyedObject* ko = key->ObjectIsLoaded();
		if (!ko)
			Load(key);
	}

	return true;
}

hsBool plResManager::SendRef(hsKeyedObject* ko, plRefMsg* refMsg, plRefFlags::Type flags)
{
	if (!ko)
		return false;
	plKey key = ko->GetKey();
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
hsBool plResManager::SendRef(const plKey& key, plRefMsg* refMsg, plRefFlags::Type flags)
{
	if (!key)
	{
		hsRefCnt_SafeUnRef(refMsg);
		return false;
	}

	plKeyImp* iKey = (plKeyImp*)key;
	iKey->ISetupNotify(refMsg, flags);
	hsRefCnt_SafeUnRef(refMsg);

	if (flags != plRefFlags::kPassiveRef)
		iKey->VerifyLoaded();

	hsKeyedObject* ko = key->ObjectIsLoaded();
	if (!ko)
		return false;

	refMsg->SetRef(ko);
	refMsg->SetTimeStamp(hsTimer::GetSysSeconds());

	for (int i = 0; i < refMsg->GetNumReceivers(); i++)
	{
		hsKeyedObject* rcv = refMsg->GetReceiver(i)->ObjectIsLoaded();
		if (rcv)
			rcv->MsgReceive(refMsg);
	}
	return true;
}

void plResManager::Load(const plKey &key)		// places on list to be loaded
{
	if (fReadingObject)
		fQueuedReads.push_back(key);
	else
		key->VerifyLoaded();	// force Load
}

plKey plResManager::ReadKeyNotifyMe(hsStream* stream, plRefMsg* msg, plRefFlags::Type flags)
{
	plKey key = ReadKey(stream);

	if (!key)
	{
		hsRefCnt_SafeUnRef(msg);
		return nil;
	}
	if(key->GetUoid().GetLoadMask().DontLoad())
	{
		hsStatusMessageF("%s being skipped because of load mask", key->GetName());
		hsRefCnt_SafeUnRef(msg);
		return nil;
	}

	((plKeyImp*)key)->SetupNotify(msg,flags);

	hsKeyedObject* ko = key->ObjectIsLoaded();

	if (!ko)
	{
		Load(key);
	}

	return key;
}

//// NewKey //////////////////////////////////////////////////////////////////
//	Creates a new key and assigns it to the given keyed object, also placing
//	it into the registry.

plKey plResManager::NewKey(const char* name, hsKeyedObject* object, const plLocation& loc, const plLoadMask& m )
{
	hsAssert(name && name[0] != '\0', "No name for new key");
	plUoid newUoid(loc, object->ClassIndex(), name, m);
	return NewKey(newUoid, object);
}

plKey plResManager::NewKey(plUoid& newUoid, hsKeyedObject* object)
{
	hsAssert(fInited, "Attempting to create a new key before we're inited!");

	plKeyImp* newKey = TRACKED_NEW plKeyImp;
	newKey->SetUoid(newUoid);
	AddKey(newKey);

	plKey keyPtr = plKey::Make(newKey);
	object->SetKey(keyPtr);

	return keyPtr;
}

plKey plResManager::ReRegister(const char* nm, const plUoid& oid)
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
			return nil;
		}
	}
	else	//we are cloning
	{
		if (pOrigKey)
		{
			plKey cloneKey = ((plKeyImp*)pOrigKey)->GetClone(fCurClonePlayerID, fCurCloneID);
			if (cloneKey)
				return cloneKey;
		}
	}

	plKeyImp* pKey = TRACKED_NEW plKeyImp;
	if (canClone && pOrigKey)
	{	
		pKey->CopyForClone((plKeyImp*)pOrigKey, fCurClonePlayerID, fCurCloneID);
		((plKeyImp*)pOrigKey)->AddClone(pKey);
	}
	else
	{
		// Make sure key doesn't already exist
		if (pOrigKey)
		{
			hsAssert(false, "Attempting to add duplicate key");
			delete pKey;
			return nil;
		}

		pKey->SetUoid(oid);			// Tell the Key its ID
		AddKey(pKey);
	}

	hsAssert(pKey, "ReRegister: returning nil key?");
	return plKey::Make(pKey);
}

//// ReadKey /////////////////////////////////////////////////////////////////
//	Reads a "key" from the given stream. What we secretly do is read in the
//	plUoid for a key and look up to find the key. Nobody else will know :)

plKey plResManager::ReadKey(hsStream* s)
{
	hsBool nonNil = s->ReadBool();
	if (!nonNil)
		return nil;

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
		if (key == nil)
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
		WriteKey(s, plKey(nil));
}

void	plResManager::WriteKey(hsStream *s, const plKey &key)
{
	s->WriteBool(key != nil);
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
		return nil;
	}

	fCloningCounter++;
	return ICloneKey(objKey->GetUoid(), plNetClientApp::GetInstance()->GetPlayerID(), fCloningCounter);
}

plKey plResManager::ICloneKey(const plUoid& objUoid, UInt32 playerID, UInt32 cloneID)
{
	hsAssert(fCurCloneID == 0, "Recursive clone");
	fCurCloneID = cloneID;
	fCurClonePlayerID = playerID;

	plKey cloneKey = ReRegister("", objUoid);

	fCurClonePlayerID = 0;
	fCurCloneID = 0;

	// Then notify NetClientMgr when object loads
	plObjRefMsg* refMsg = TRACKED_NEW plObjRefMsg(plNetClientApp::GetInstance()->GetKey(), plRefMsg::kOnCreate, 0, 0);
	AddViaNotify(cloneKey, refMsg, plRefFlags::kPassiveRef);	

	return cloneKey;
}

//
// Unregisters (deletes) an object.
// Currently, this means the object is going away permanently.
// When support for paging is added, key->UnRegister() should not clear its notify lists.
// Return true if successful.
//
hsBool plResManager::Unload(const plKey& objKey)
{
	if (objKey)
	{
		((plKeyImp*)objKey)->UnRegister();
		fDispatch->UnRegisterAll(objKey);
		return true;
	}
	return false;
}

plCreatable* plResManager::IReadCreatable(hsStream* s) const
{
	UInt16 hClass = s->ReadSwap16();
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
	Int16 hClass = pCre ? pCre->ClassIndex() : 0x8000;
	hsAssert(pCre == nil || plFactory::IsValidClassIndex(hClass), "Invalid class index on write");
	s->WriteSwap16(hClass);
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
//	Helper object that stores all the keys for an age, to optimize the load
//	process. 

class plResAgeHolder : public hsRefCnt
{
	public:
		hsTArray<plKey>	fKeys;
		std::string		fAge;

		plResAgeHolder() {}
		plResAgeHolder( const char* age ) : fAge( age ) {}
		~plResAgeHolder() { fKeys.Reset(); }
};

//// plResHolderIterator /////////////////////////////////////////////////////

class plResHolderIterator : public plRegistryPageIterator
{
protected:
	hsTArray<plKey>& fKeys;
	const char* fAgeName;
	plResManager* fResMgr;

public:
	plResHolderIterator(const char* age, hsTArray<plKey>& keys, plResManager* resMgr) 
			: fAgeName(age), fKeys(keys), fResMgr(resMgr) {}

	virtual hsBool EatPage(plRegistryPageNode* page)
	{
		if (stricmp(page->GetPageInfo().GetAge(), fAgeName) == 0)
		{
			fResMgr->LoadPageKeys(page);
			plKeyCollector collector(fKeys);
			page->IterateKeys(&collector);
		}

		return true;
	}
};

//// LoadAndHoldAgeKeys //////////////////////////////////////////////////////

void plResManager::LoadAgeKeys(const char* age)
{
	hsAssert(age && age[0] != '\0', "age is nil");
	HeldAgeKeyMap::const_iterator it = fHeldAgeKeys.find(age);
	if (it != fHeldAgeKeys.end())
	{
		kResMgrLog(1, ILog(1, "Reffing age keys for age %s", age));
		hsStatusMessageF("*** Reffing age keys for age %s ***\n", age);
		plResAgeHolder* holder = it->second;
		holder->Ref();
	}
	else
	{
		kResMgrLog(1, ILog(1, "Loading age keys for age %s", age));
		hsStatusMessageF("*** Loading age keys for age %s ***\n", age);

		plResAgeHolder* holder = TRACKED_NEW plResAgeHolder(age);
		fHeldAgeKeys[age] = holder;
		// Go find pages that match this age, load the keys, and ref them all
		plResHolderIterator	iter(age, holder->fKeys, this);
		IterateAllPages(&iter);
	}
}

//// DropAgeKeys /////////////////////////////////////////////////////////////

void plResManager::DropAgeKeys(const char* age)
{
	HeldAgeKeyMap::iterator it = fHeldAgeKeys.find(age);
	if (it != fHeldAgeKeys.end())
	{
		plResAgeHolder* holder = it->second;
		if (holder->RefCnt() == 1)
		{
			// Found it!
			kResMgrLog(1, ILog(1, "Dropping held age keys for age %s", age));
			fHeldAgeKeys.erase(it);
		}
		else
		{
			kResMgrLog(1, ILog(1, "Unreffing age keys for age %s", age));
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
		kResMgrLog(1, ILog(1, "Dropping age keys for age %s", holder->fAge.c_str()));
		while (holder->RefCnt() > 1)
			holder->UnRef();
		holder->UnRef();	// deletes holder
	}
}

//// PageInRoom //////////////////////////////////////////////////////////////
//	Normal finds will have to potentially reload all the keys for a page, but
//	paging in this way will avoid having to reload keys every single find
//	during a load--we load all the keys once, ref them so they're in, then
//	do the entire load and unref when we're done.
//
//	The objClassToRef parameter is a bit tricky. Basically, you assume that
//	there's one object in the page that you're loading based off of (say, a
//	sceneNode). That's the object that'll get reffed by the refMsg passed in.
//	The only reason we abstract it is so we can keep plResManager from being
//	dependent on any particular class type.
//
//	This function is not guaranteed to be synchronous, so you better wait for 
//	the refMsg to be sent before you assume it's done.

class plOurRefferAndFinder : public plRegistryKeyIterator
{
	hsTArray<plKey>	&fRefArray;
	UInt16			fClassToFind;
	plKey			&fFoundKey;

	public:

		plOurRefferAndFinder( hsTArray<plKey> &refArray, UInt16 classToFind, plKey &foundKey ) 
				: fRefArray( refArray ), fClassToFind( classToFind ), fFoundKey( foundKey ) { }

		virtual hsBool	EatKey( const plKey& key )
		{
			// This is cute. Thanks to our new plKey smart pointers, all we have to
			// do is append the key to our ref array. This automatically guarantees us
			// an extra ref on the key, which is what we're trying to do. Go figure.
			fRefArray.Append( key );

			// Also do our find
			if( key->GetUoid().GetClassType() == fClassToFind )
				fFoundKey = key;
			
			return true;
		}
};

void plResManager::PageInRoom(const plLocation& page, UInt16 objClassToRef, plRefMsg* refMsg)
{
	UInt64 readRoomTime = 0;
	if (fLogReadTimes)
		readRoomTime = hsTimer::GetFullTickCount();

	plSynchEnabler ps(false);	// disable dirty tracking while paging in

	kResMgrLog(1, ILog(1, "Paging in room 0x%x...", page.GetSequenceNumber()));

	// Step 0: Find the pageNode
	plRegistryPageNode* pageNode = FindPage(page);
	if (pageNode == nil)
	{
		kResMgrLog(1, ILog(1, "...Page not found!"));
		hsAssert(false, "Invalid location given to PageInRoom()");
		return;
	}

	kResMgrLog(2, ILog(2, "...Found, page is ID'd as %s>%s", pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage()));

	// Step 0.5: Verify the page, just to make sure we really should be loading it
	PageCond cond = pageNode->GetPageCondition();
	if (cond != kPageOk)
	{
		std::string condStr ="Checksum invalid";
		if (cond == kPageTooNew)
			condStr = "Page Version too new";
		else
		if (cond == kPageOutOfDate)
			condStr = "Page Version out of date";
		
		kResMgrLog(1, ILog(1, "...IGNORING pageIn request; verification failed! (%s)", condStr.c_str()));

		std::string msg = xtl::format("Data Problem: Age:%s  Page:%s  Error:%s", 
			pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage(), condStr.c_str());
		hsMessageBox(msg.c_str(), "Error", hsMessageBoxNormal, hsMessageBoxIconError);
		
		hsRefCnt_SafeUnRef(refMsg);
		return;
	}

	// Step 1: We force a load on all the keys in the given page
	kResMgrLog(2, ILog(2, "...Loading page keys..."));
	LoadPageKeys(pageNode);

	// Step 2: Now ref all the keys in that page, every single one. This lets us unref 
	// (and thus potentially delete) them later. Note that we also use this for our find.
	kResMgrLog(2, ILog(2, "...Reffing keys..."));
	plKey objKey;
	hsTArray<plKey> keyRefList;
	plOurRefferAndFinder reffer(keyRefList, objClassToRef, objKey);
	pageNode->IterateKeys(&reffer);

	// Step 3: Do our load
	if (objKey == nil)
	{
		kResMgrLog(1, ILog(1, "...SceneNode not found to base page-in op on. Aborting..."));
		// This is coming up a lot lately; too intrusive to be an assert.
		// hsAssert( false, "No object found on which to base our PageInRoom()" );
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
	keyRefList.Reset();

	// Step 5: Ref the object
	kResMgrLog(2, ILog(2, "...Dispatching refMessage..."));
	AddViaNotify(objKey, refMsg, plRefFlags::kActiveRef);

	// All done!
	kResMgrLog(1, ILog(1, "...Page in complete!"));

	if (fLogReadTimes)
	{
		readRoomTime = hsTimer::GetFullTickCount() - readRoomTime;

		plStatusLog::AddLineS("readtimings.log", plStatusLog::kWhite, "----- Reading page %s>%s took %.1f ms",
			pageNode->GetPageInfo().GetAge(), pageNode->GetPageInfo().GetPage(),
			hsTimer::FullTicksToMs(readRoomTime));
	}
}

class plPageInAgeIter : public plRegistryPageIterator
{
private:
	plKey		fDestKey;
	const char* fAgeName;
	std::vector<plLocation> fLocations;

public:
	plPageInAgeIter(plKey destKey, const char *ageName) : fDestKey(destKey), fAgeName(ageName) {}
	~plPageInAgeIter()
	{
		plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kLoadRoomHold);
		for (int i = 0; i < fLocations.size(); i++)
		{
			pMsg1->AddRoomLoc(fLocations[i]);
		}
		pMsg1->Send(fDestKey);
	}
	virtual hsBool EatPage(plRegistryPageNode* page)
	{
		if (stricmp(page->GetPageInfo().GetAge(), fAgeName) == 0)
		{
			plUoid uoid(page->GetPageInfo().GetLocation(), 0, "");
			fLocations.push_back(uoid.GetLocation());
		}
		return true;
	}
};

// PageInAge is intended for bulk global ages, like GlobalAnimations or GlobalClothing
// that store a lot of data we always want available. (Used to be known as PageInHold)
void plResManager::PageInAge(const char *age)
{
	plSynchEnabler ps(false);	// disable dirty tracking while paging in
	plUoid lu(kClient_KEY);
	plKey clientKey = hsgResMgr::ResMgr()->FindKey(lu);

	// Tell the client to load all the keys for this age, to make the loading process work better
	plClientMsg *loadAgeKeysMsg = TRACKED_NEW plClientMsg(plClientMsg::kLoadAgeKeys);
	loadAgeKeysMsg->SetAgeName(age);
	loadAgeKeysMsg->Send(clientKey);

	// Then iterate through each room in the age. The iterator will send the load message
	// off on destruction.
	plPageInAgeIter	iter(clientKey, age);
	IterateAllPages(&iter);
}

//// VerifyPages /////////////////////////////////////////////////////////////
//	Runs through all the pages and ensures they are all up-to-date in version
//	numbers and that no out-of-date objects exist in them

hsBool plResManager::VerifyPages()
{
	hsTArray<plRegistryPageNode*> invalidPages, newerPages;

	// Step 1: verify major/minor version changes
	if (plResMgrSettings::Get().GetFilterNewerPageVersions() ||
		plResMgrSettings::Get().GetFilterOlderPageVersions())
	{
		PageSet::iterator it = fAllPages.begin();
		while (it != fAllPages.end())
		{
			plRegistryPageNode* page = *it;
			it++;

			if (page->GetPageCondition() == kPageTooNew && plResMgrSettings::Get().GetFilterNewerPageVersions())
			{
				newerPages.Append(page);
				fAllPages.erase(page);
			}
			else if (
				(page->GetPageCondition() == kPageCorrupt ||
				page->GetPageCondition() == kPageOutOfDate)
				&& plResMgrSettings::Get().GetFilterOlderPageVersions())
			{
				invalidPages.Append(page);
				fAllPages.erase(page);
			}
		}
	}

	// Handle all our invalid pages now
	if (invalidPages.GetCount() > 0)
	{
		if (!IDeleteBadPages(invalidPages, false))
			return false;
	}

	// Warn about newer pages
	if (newerPages.GetCount() > 0)
	{
		if (!IWarnNewerPages(newerPages))
			return false;
	}

	// Step 2 of verification: make sure no sequence numbers conflict
	PageSet::iterator it = fAllPages.begin();
	for (; it != fAllPages.end(); it++)
	{
		plRegistryPageNode* page = *it;

		PageSet::iterator itUp = it;
		itUp++;
		for (; itUp != fAllPages.end(); itUp++)
		{
			plRegistryPageNode* upPage = *itUp;
			if (page->GetPageInfo().GetLocation() == upPage->GetPageInfo().GetLocation())
			{
				invalidPages.Append(upPage);
				fAllPages.erase(itUp);
				break;
			}
		}
	}

	// Redo our loaded pages list, since Verify() might force the page's keys to load or unload
	fLoadedPages.clear();
	it = fAllPages.begin();
	while (it != fAllPages.end())
	{
		plRegistryPageNode* page = *it;
		it++;

		if (page->IsLoaded())
			fLoadedPages.insert(page);
	}

	// Handle all our conflicting pages now
	if (invalidPages.GetCount() > 0)
		return IDeleteBadPages(invalidPages, true);

	return true;
}

//// IDeleteBadPages /////////////////////////////////////////////////////////
//	Given an array of pages that are invalid (major version out-of-date or
//	whatnot), asks the user what we should do about them.

static void ICatPageNames(hsTArray<plRegistryPageNode*>& pages, char* buf, int bufSize)
{
	for (int i = 0; i < pages.GetCount(); i++)
	{
		if (i >= 25)
		{
			strcat(buf, "...\n");
			break;
		}

		const char* pagePath = pages[i]->GetPagePath();
		const char* pageFile = plFileUtils::GetFileName(pagePath);

		if (strlen(buf) + strlen(pageFile) > bufSize - 5)
		{
			strcat(buf, "...\n");
			break;
		}

		strcat(buf, pageFile);
		strcat(buf, "\n");
	}
}

hsBool plResManager::IDeleteBadPages(hsTArray<plRegistryPageNode*>& invalidPages, hsBool conflictingSeqNums)
{
#ifndef PLASMA_EXTERNAL_RELEASE
	if (!hsMessageBox_SuppressPrompts)
	{
		char msg[4096];

		// Prompt what to do
		if (conflictingSeqNums)
			strcpy(msg, "The following pages have conflicting sequence numbers. This usually happens when "
						"you copy data files between machines that had random sequence numbers assigned at "
						"export. To avoid crashing, these pages will be deleted:\n\n");
		else
			strcpy(msg, "The following pages are out of date and will be deleted:\n\n");

		ICatPageNames(invalidPages, msg, sizeof(msg));

		hsMessageBox(msg, "Warning", hsMessageBoxNormal);
	}
#endif // PLASMA_EXTERNAL_RELEASE

	// Delete 'em
	for (int i = 0; i < invalidPages.GetCount(); i++)
	{
		invalidPages[i]->DeleteSource();
		delete invalidPages[i];
	}
	invalidPages.Reset();

	fLastFoundPage = nil;

	return true;
}

//// IWarnNewerPages /////////////////////////////////////////////////////////
//	Given an array of pages that are newer (minor or major version are newer
//	than the "current" one), warns the user about them but does nothing to
//	them.

hsBool plResManager::IWarnNewerPages(hsTArray<plRegistryPageNode*> &newerPages)
{
#ifndef PLASMA_EXTERNAL_RELEASE
	if (!hsMessageBox_SuppressPrompts)
	{
		char msg[4096];
		// Prompt what to do
		strcpy(msg, "The following pages have newer version numbers than this client and cannot be \nloaded. "
					"They will be ignored but their files will NOT be deleted:\n\n");

		ICatPageNames(newerPages, msg, sizeof(msg));

		hsMessageBox(msg, "Warning", hsMessageBoxNormal);
	}
#endif // PLASMA_EXTERNAL_RELEASE


	// Not deleting the files, just delete them from memory
	for (int i = 0; i < newerPages.GetCount(); i++)
		delete newerPages[i];
	newerPages.Reset();

	fLastFoundPage = nil;

	return true;
}

//// plOurReffer /////////////////////////////////////////////////////////////
//	Our little reffer key iterator

class plOurReffer : public plRegistryKeyIterator
{
protected:
	hsTArray<plKey>	fRefArray;

public:
	plOurReffer() {}
	virtual ~plOurReffer() { UnRef(); }

	void UnRef() { fRefArray.Reset(); }

	virtual hsBool EatKey(const plKey& key)
	{
		// This is cute. Thanks to our new plKey smart pointers, all we have to
		// do is append the key to our ref array. This automatically guarantees us
		// an extra ref on the key, which is what we're trying to do. Go figure.
		fRefArray.Append(key);

		return true;
	}
};

void plResManager::DumpUnusedKeys(plRegistryPageNode* page) const
{
	plOurReffer reffer;
	page->IterateKeys(&reffer);
}

plRegistryPageNode* plResManager::CreatePage(const plLocation& location, const char* age, const char* page)
{
	plRegistryPageNode* pageNode = TRACKED_NEW plRegistryPageNode(location, age, page, fDataPath.c_str());
	fAllPages.insert(pageNode);

	return pageNode;
}

//// AddPage /////////////////////////////////////////////////////////////////

void plResManager::AddPage(plRegistryPageNode* page)
{
	fAllPages.insert(page);
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
//	Handy tiny function here

static void	sIReportLeak(plKeyImp* key, plRegistryPageNode* page)
{
	class plKeyImpRef : public plKeyImp
	{
	public:
		UInt16 GetRefCnt() const { return fRefCount; }
	};

	static bool alreadyDone = false;
	static plRegistryPageNode* lastPage;

	if (page != nil)
		lastPage = page;

	if (key == nil)
	{
		alreadyDone = false;
		return;
	}

	if (!alreadyDone)
	{
		// Print out page header
		hsStatusMessageF("  Leaks in page %s>%s[%08x]:\n", lastPage->GetPageInfo().GetAge(), lastPage->GetPageInfo().GetPage(), lastPage->GetPageInfo().GetLocation().GetSequenceNumber());
		alreadyDone = true;
	}

	int refsLeft = ((plKeyImpRef*)key)->GetRefCnt() - 1;
	if (refsLeft == 0)
		return;

	char tempStr[256], tempStr2[128];
	if (key->ObjectIsLoaded() == nil)
		sprintf(tempStr2, "(key only, %d refs left)", refsLeft);
	else
		sprintf(tempStr2, "- %d bytes - %d refs left", key->GetDataLen(), refsLeft);

	hsStatusMessageF("    %s: %s %s\n", plFactory::GetNameOfClass(key->GetUoid().GetClassType()), 
												key->GetUoid().StringIze(tempStr), tempStr2);
}

//// UnloadPageObjects ///////////////////////////////////////////////////////
//	Unloads all the objects in a given page. Once this is complete, all 
//	object pointers for every key in the page *should* be nil. Note that we're
//	given a hint class index to start with (like plSceneNode) that should do
//	most of the work for us via unreffing.
//
//	Update 5.20: since there are so many problems with doing this, don't
//				 delete the objects, just print out a memleak report. -mcn

void plResManager::UnloadPageObjects(plRegistryPageNode* pageNode, UInt16 classIndexHint)
{
	if (!pageNode->IsLoaded())
		return;

	class plUnloadObjectsIterator : public plRegistryKeyIterator
	{
	public:
		virtual hsBool EatKey(const plKey& key)
		{
			sIReportLeak((plKeyImp*)key, nil);
			return true;
		}
	};

	sIReportLeak(nil, pageNode);

	plUnloadObjectsIterator	iterator;

	if (classIndexHint != UInt16(-1))
		pageNode->IterateKeys(&iterator, classIndexHint);
	else
		pageNode->IterateKeys(&iterator);
}

//// FindPage ////////////////////////////////////////////////////////////////

plRegistryPageNode* plResManager::FindPage(const plLocation& location) const
{
	// Quick optimization
	if (fLastFoundPage != nil && fLastFoundPage->GetPageInfo().GetLocation() == location)
		return fLastFoundPage;

	PageSet::const_iterator it;
	for (it = fAllPages.begin(); it != fAllPages.end(); it++)
	{
		const plLocation& pageloc = (*it)->GetPageInfo().GetLocation();
		if (pageloc == location)
		{
			fLastFoundPage = *it;
			return fLastFoundPage;
		}
	}

	return nil;
}

//// FindPage ////////////////////////////////////////////////////////////////

plRegistryPageNode* plResManager::FindPage(const char* age, const char* page) const
{
	PageSet::const_iterator it;
	for (it = fAllPages.begin(); it != fAllPages.end(); it++)
	{
		const plPageInfo& info = (*it)->GetPageInfo();
		if (hsStrCaseEQ(info.GetAge(), age) &&
			hsStrCaseEQ(info.GetPage(), page))
			return *it;
	}

	return nil;
}

//////////////////////////////////////////////////////////////////////////////
//// Key Operations //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// AddKey //////////////////////////////////////////////////////////////////
//	Adds a key to the registry. Assumes uoid already set.

void plResManager::AddKey(plKeyImp* key)
{
	plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
	if (page == nil)
		return;

	page->AddKey(key);
	fLoadedPages.insert(page);
}

void plResManager::IKeyReffed(plKeyImp* key)
{
	plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
	if (page == nil)
	{
		hsAssert(0, "Couldn't find page that key belongs to");
		return;
	}

	page->SetKeyUsed(key);
}

void plResManager::IKeyUnreffed(plKeyImp* key)
{
	plRegistryPageNode* page = FindPage(key->GetUoid().GetLocation());
	if (page == nil)
	{
		hsAssert(0, "Couldn't find page that key belongs to");
		return;
	}

	bool removed = page->SetKeyUnused(key);
	hsAssert(removed, "Key wasn't removed from page");

	if (removed)
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
	virtual hsBool EatPage(plRegistryPageNode* keyNode)
	{
		return keyNode->IterateKeys(fIter);
	}
};

//// IterateKeys /////////////////////////////////////////////////////////////

hsBool plResManager::IterateKeys(plRegistryKeyIterator* iterator)
{
	plKeyIterEater myEater(iterator);
	return IteratePages(&myEater, nil);
}

hsBool plResManager::IterateKeys(plRegistryKeyIterator* iterator, const plLocation& pageToRestrictTo)
{
	plRegistryPageNode* page = FindPage(pageToRestrictTo);
	if (page == nil)
	{
		hsAssert(false, "Page not found to iterate through");
		return false;
	}

	plKeyIterEater myEater(iterator);
	return myEater.EatPage(page);
}

//// IteratePages ////////////////////////////////////////////////////////////
//	Iterate through all LOADED pages

hsBool plResManager::IteratePages(plRegistryPageIterator* iterator, const char* ageToRestrictTo)
{
	ILockPages();

	PageSet::const_iterator it;
	for (it = fLoadedPages.begin(); it != fLoadedPages.end(); it++)
	{
		plRegistryPageNode* page = *it;
		if (page->GetPageInfo().GetLocation() == plLocation::kGlobalFixedLoc)
			continue;

		if (!ageToRestrictTo || hsStrCaseEQ(page->GetPageInfo().GetAge(), ageToRestrictTo))
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
//	Iterate through ALL pages

hsBool plResManager::IterateAllPages(plRegistryPageIterator* iterator)
{
	ILockPages();

	PageSet::const_iterator it;
	for (it = fAllPages.begin(); it != fAllPages.end(); it++)
	{
		plRegistryPageNode* page = *it;
		if (page->GetPageInfo().GetLocation() == plLocation::kGlobalFixedLoc)
			continue;

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
//	See, when we iterate through pages, our iterate function might decide to
//	move pages, either explicitly through loads or implicitly through key
//	deletions. So, before we iterate, we lock 'em all so they won't move,
//	then unlock and move them to their proper places at the end.

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

		PageSet::const_iterator it;
		for (it = fAllPages.begin(); it != fAllPages.end(); it++)
		{
			plRegistryPageNode* page = *it;
			if (page->IsLoaded())
				fLoadedPages.insert(page);
		}
	}
}

// Defined here 'cause release build hates it defined in settings.h for some reason
#include "plResMgrSettings.h"
plResMgrSettings& plResMgrSettings::Get()
{
	static plResMgrSettings fSettings;
	return fSettings;
}
