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
#include "plAgeLoader.h"
#include "hsStream.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
//#include "hsTimer.h"
#include "plResPatcher.h"
#include "plBackgroundDownloader.h"
#include "process.h"	// for getpid()

#include "../pnProduct/pnProduct.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnNetCommon/plNetApp.h"

#include "../plScene/plRelevanceMgr.h"
#include "../plResMgr/plKeyFinder.h"
#include "../plAgeDescription/plAgeDescription.h"
#include "../plSDL/plSDL.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plResManager.h"
#include "../plFile/plEncryptedStream.h"

/// TEMP HACK TO LOAD CONSOLE INIT FILES ON AGE LOAD
#include "../plMessage/plConsoleMsg.h"
#include "../plMessage/plLoadAvatarMsg.h"
#include "../plMessage/plAgeLoadedMsg.h"


extern	hsBool	gDataServerLocal;
extern	hsBool	gUseBackgroundDownloader;

// static 
plAgeLoader* plAgeLoader::fInstance=nil;

//
// CONSTRUCT
//
plAgeLoader::plAgeLoader() :
		fInitialAgeState(nil),
		fFlags(0)
{
}

//
// DESTRUCT
//
plAgeLoader::~plAgeLoader()
{
	delete fInitialAgeState;
	fInitialAgeState=nil;

	if ( PendingAgeFniFiles().size() )
		plNetClientApp::StaticErrorMsg( "~plAgeLoader(): %d pending age fni files", PendingAgeFniFiles().size() );
	if ( PendingPageOuts().size() )
		plNetClientApp::StaticErrorMsg( "~plAgeLoader(): %d pending page outs", PendingPageOuts().size() );

	ClearPageExcludeList();		// Clear our debugging exclude list, just to be tidy
	
	if (fInstance==this)
		SetInstance(nil);
}

void plAgeLoader::Shutdown()
{

}

void plAgeLoader::Init()
{
	RegisterAs( kAgeLoader_KEY );
	plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plClientMsg::Index(), GetKey());

	if (!gDataServerLocal && gUseBackgroundDownloader)
		plBackgroundDownloader::StartThread();
}

//
// STATIC
//
plAgeLoader* plAgeLoader::GetInstance()
{
	return fInstance;
}

//
// STATIC
//
void plAgeLoader::SetInstance(plAgeLoader* inst)
{
	fInstance=inst;
}

//
// Plasma Msg Handler
//
hsBool plAgeLoader::MsgReceive(plMessage* msg)
{
	plInitialAgeStateLoadedMsg *stateMsg = plInitialAgeStateLoadedMsg::ConvertNoRef( msg );
	if( stateMsg != nil )
	{
		// done receiving the initial state of the age from the server
		return true;
	}

	plClientMsg* clientMsg = plClientMsg::ConvertNoRef(msg);
	if (clientMsg && clientMsg->GetClientMsgFlag()==plClientMsg::kInitComplete)
	{
		ExecPendingAgeFniFiles();	// exec age-specific fni files
		return true;
	}

	
	return plReceiver::MsgReceive(msg);
}

//
// read in the age desc file and page in/out the rooms belonging to the specified age.
// return false on error
//
//============================================================================
bool plAgeLoader::LoadAge(const char ageName[])
{
	return ILoadAge(ageName);
}

//============================================================================
bool plAgeLoader::UpdateAge(const char ageName[])
{
	bool result = true;

	if (!gDataServerLocal)
	{
		plResPatcher myPatcher(ageName);
		result = myPatcher.Update();
	}

	return result;
}

//============================================================================
void plAgeLoader::NotifyAgeLoaded( bool loaded )
{
	if ( loaded )
		fFlags &= ~kLoadingAge;
	else
		fFlags &= ~kUnLoadingAge;

	plAgeLoadedMsg * msg = TRACKED_NEW plAgeLoadedMsg;
	msg->fLoaded = loaded;
	msg->Send();
}


//// ILoadAge ////////////////////////////////////////////////////////////////
//	Does the loading-specific stuff for queueing an age to load

bool plAgeLoader::ILoadAge(const char ageName[])
{
	plNetClientApp* nc = plNetClientApp::GetInstance();
	ASSERT(!nc->GetFlagsBit(plNetClientApp::kPlayingGame));

	StrCopy(fAgeName, ageName, arrsize(fAgeName));
	
	nc->DebugMsg( "Net: Loading age %s", fAgeName);

	if ((fFlags & kLoadMask) != 0)
		ErrorFatal(__LINE__, __FILE__, "Fatal Error:\nAlready loading or unloading an age.\n%S will now exit.", ProductShortName());
		
	fFlags |= kLoadingAge;
	
	plAgeBeginLoadingMsg* ageBeginLoading = TRACKED_NEW plAgeBeginLoadingMsg();
	ageBeginLoading->Send();

	///////////////////////////////////////////////////////


	/// Step 1: Update all of the dat files for this age
	/*
	UpdateAge(fAgeName);
	*/

	/// Step 2: Load the keys for this age, so we can find sceneNodes for them
	// exec age .fni file when data is done loading
 	char consoleIniName[ 256 ];
	sprintf( consoleIniName, "dat\\%s.fni", fAgeName);
	fPendingAgeFniFiles.push_back( consoleIniName );

	char csvName[256];
	sprintf(csvName, "dat\\%s.csv", fAgeName);
	fPendingAgeCsvFiles.push_back(csvName);
	
	plSynchEnabler p( false );	// turn off dirty tracking while in this function	

	hsStream* stream=GetAgeDescFileStream(fAgeName);
	if (!stream)
	{
		nc->ErrorMsg("Failed loading age.  Age desc file %s has nil stream", fAgeName);
		fFlags &= ~kLoadingAge;
		return false;
	}

	plAgeDescription ad;
	ad.Read(stream);
	ad.SetAgeName(fAgeName);
	stream->Close();
	delete stream;
	ad.SeekFirstPage();
	
	plAgePage *page;
	plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

	// Copy, exclude pages we want excluded, and collect our scene nodes
	fCurAgeDescription.CopyFrom(ad);
	while( ( page = ad.GetNextPage() ) != nil )
	{
		if( IsPageExcluded( page, fAgeName) )
			continue;

		plKey roomKey = plKeyFinder::Instance().FindSceneNodeKey( fAgeName, page->GetName() );
		if( roomKey != nil )
			AddPendingPageInRoomKey( roomKey );
	}
	ad.SeekFirstPage();


	// Tell the client to load-and-hold all the keys for this age, to make the loading process work better
	plClientMsg *loadAgeKeysMsg = TRACKED_NEW plClientMsg( plClientMsg::kLoadAgeKeys );
	loadAgeKeysMsg->SetAgeName( fAgeName);
	loadAgeKeysMsg->Send( clientKey );

	//
	// Load the Age's SDL Hook object (and it's python modifier)
	//	
	plUoid oid=nc->GetAgeSDLObjectUoid(fAgeName);
	plKey ageSDLObjectKey = hsgResMgr::ResMgr()->FindKey(oid);
	if (ageSDLObjectKey)
		hsgResMgr::ResMgr()->AddViaNotify(ageSDLObjectKey, TRACKED_NEW plGenRefMsg(nc->GetKey(), plRefMsg::kOnCreate, -1, 
		plNetClientMgr::kAgeSDLHook), plRefFlags::kActiveRef);
	
	int nPages = 0;

	plClientMsg* pMsg1 = TRACKED_NEW plClientMsg(plClientMsg::kLoadRoom);
	pMsg1->SetAgeName(fAgeName);

	// Loop and ref!
	while( ( page = ad.GetNextPage() ) != nil )
	{
		if( IsPageExcluded( page, fAgeName) )
		{
			nc->DebugMsg( "\tExcluding page %s\n", page->GetName() );
			continue;
		}

		nPages++;

		pMsg1->AddRoomLoc(ad.CalcPageLocation(page->GetName()));
		nc->DebugMsg("\tPaging in room %s\n", page->GetName());
	}

	pMsg1->Send(clientKey);

	// Send the client a message to let go of the extra keys it was holding on to
	plClientMsg	*dumpAgeKeys = TRACKED_NEW plClientMsg( plClientMsg::kReleaseAgeKeys );
	dumpAgeKeys->SetAgeName( fAgeName);
	dumpAgeKeys->Send( clientKey );

	if ( nPages==0 )
	{
		// age is done loading because it has no pages?
		fFlags &= ~kLoadingAge;
	}

	return true;
}

//// plUnloadAgeCollector ////////////////////////////////////////////////////
//	Registry page iterator to collect all the loaded pages of a given age
//	Note: we have to do an IterateAllPages(), since we want to also catch
//	pages that are partially loaded, which are skipped in the vanilla 
//	IteratePages() call.

class plUnloadAgeCollector : public plRegistryPageIterator
{
	public:
		hsTArray<plRegistryPageNode *>	fPages;
		const char						*fAge;
		
		plUnloadAgeCollector( const char *a ) : fAge( a ) {}

		virtual hsBool EatPage( plRegistryPageNode *page )
		{
			if( fAge && stricmp( page->GetPageInfo().GetAge(), fAge ) == 0 )
			{
				fPages.Append( page );
			}

			return true;
		}
};

//// IUnloadAge //////////////////////////////////////////////////////////////
//	Does the UNloading-specific stuff for queueing an age to unload.
//	Far simpler that ILoadAge :)

bool	plAgeLoader::IUnloadAge()
{
	plNetClientApp* nc = plNetClientApp::GetInstance();
	nc->DebugMsg( "Net: Unloading age %s", fAgeName);

	hsAssert( (fFlags & kLoadMask)==0, "already loading or unloading an age?");	
	fFlags |= kUnLoadingAge;
	
	plAgeBeginLoadingMsg* msg = TRACKED_NEW plAgeBeginLoadingMsg();
	msg->fLoading = false;
	msg->Send();

	// Note: instead of going from the .age file, we just want a list of what
	// is REALLY paged in for this age. So ask the resMgr!
	plUnloadAgeCollector collector( fAgeName);
	// WARNING: unsafe cast here, but it's ok, until somebody is mean and makes a non-plResManager resMgr
	( (plResManager *)hsgResMgr::ResMgr() )->IterateAllPages( &collector );

	// Dat was easy...
	plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

	// Build up a list of all the rooms we're going to page out
	plKeyVec newPageOuts;

	int i;
	for( i = 0; i < collector.fPages.GetCount(); i++ )
	{
		plRegistryPageNode *page = collector.fPages[ i ];

		plKey roomKey = plKeyFinder::Instance().FindSceneNodeKey( page->GetPageInfo().GetLocation() );
		if( roomKey != nil && roomKey->ObjectIsLoaded() )
		{
			nc->DebugMsg( "\tPaging out room %s\n", page->GetPageInfo().GetPage() );
			newPageOuts.push_back(roomKey);
		}
	}

	// Put them in our pending page outs
	for( i = 0; i < newPageOuts.size(); i++ )
		fPendingPageOuts.push_back(newPageOuts[i]);

	// ...then send the unload messages.  That way we ensure the list is complete
	// before any messages get processed
	for( i = 0; i < newPageOuts.size(); i++ )
	{
		plClientMsg *pMsg1 = TRACKED_NEW plClientMsg( plClientMsg::kUnloadRoom );
		pMsg1->AddRoomLoc(newPageOuts[i]->GetUoid().GetLocation());
		pMsg1->Send( clientKey );
	}
	
	if ( newPageOuts.size()==0 )
	{
		// age is done unloading because it has no pages?
		NotifyAgeLoaded( false );
	}

	return true;
}


void plAgeLoader::ExecPendingAgeFniFiles()
{
	int i;
	for (i=0;i<PendingAgeFniFiles().size(); i++)
	{
		plConsoleMsg	*cMsg = TRACKED_NEW plConsoleMsg( plConsoleMsg::kExecuteFile, fPendingAgeFniFiles[i].c_str() );
		plgDispatch::MsgSend( cMsg );
	}
	fPendingAgeFniFiles.clear();
}

void plAgeLoader::ExecPendingAgeCsvFiles()
{
	int i;
	for (i=0;i<PendingAgeCsvFiles().size(); i++)
	{
		hsStream* stream = plEncryptedStream::OpenEncryptedFile(fPendingAgeCsvFiles[i].c_str());
		if (stream)
		{
			plRelevanceMgr::Instance()->ParseCsvInput(stream);
			stream->Close();
			delete stream;
		}
	}
	fPendingAgeCsvFiles.clear();
}

//
// return alloced stream or nil
// static
//
hsStream* plAgeLoader::GetAgeDescFileStream(const char* ageName)
{
	if (!ageName)
		return nil;

 	char ageDescFileName[256];
	sprintf(ageDescFileName, "dat\\%s.age", ageName);

	hsStream* stream = plEncryptedStream::OpenEncryptedFile(ageDescFileName);
	if (!stream)
	{
		char str[256];
		sprintf(str, "Can't find age desc file %s", ageDescFileName);
		hsAssert(false, str);
		return nil;
	}

	return stream;
}

//
// sent from server with joinAck
//
void plAgeLoader::ISetInitialAgeState(plStateDataRecord* s)
{
	hsAssert(fInitialAgeState != s, "duplicate initial age state");
	delete fInitialAgeState;
	fInitialAgeState=s;
}
