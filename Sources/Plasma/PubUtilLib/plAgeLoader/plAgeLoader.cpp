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
#include "plAgeLoader.h"
#include "hsStream.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
//#include "hsTimer.h"
#include "plResPatcher.h"
#if HS_BUILD_FOR_WIN32
#    include "process.h"    // for getpid()
#else
#    include <unistd.h>
#endif

#include "plProduct.h"

#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnMessage/plClientMsg.h"
#include "pnNetCommon/plNetApp.h"

#include "plAgeDescription/plAgeDescription.h"
#include "plFile/plEncryptedStream.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plConsoleMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plResPatcherMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plScene/plRelevanceMgr.h"
#include "plSDL/plSDL.h"
#include "plResMgr/plKeyFinder.h"
#include "plResMgr/plRegistryHelpers.h"
#include "plResMgr/plRegistryNode.h"
#include "plResMgr/plResManager.h"

// static 
plAgeLoader* plAgeLoader::fInstance = nullptr;

//
// CONSTRUCT
//
plAgeLoader::plAgeLoader() :
        fInitialAgeState(),
        fFlags()
{
}

//
// DESTRUCT
//
plAgeLoader::~plAgeLoader()
{
    delete fInitialAgeState;
    fInitialAgeState = nullptr;

    if ( PendingAgeFniFiles().size() )
        plNetClientApp::StaticErrorMsg( "~plAgeLoader(): {} pending age fni files", PendingAgeFniFiles().size() );
    if ( PendingPageOuts().size() )
        plNetClientApp::StaticErrorMsg( "~plAgeLoader(): {} pending page outs", PendingPageOuts().size() );

    ClearPageExcludeList();     // Clear our debugging exclude list, just to be tidy
    
    if (fInstance==this)
        SetInstance(nullptr);
}

void plAgeLoader::Shutdown()
{
    plResPatcher::GetInstance()->Shutdown();
    UnRegisterAs(kAgeLoader_KEY);
    SetInstance(nullptr);
}

void plAgeLoader::Init()
{
    RegisterAs( kAgeLoader_KEY );
    plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plClientMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plResPatcherMsg::Index(), GetKey());
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
bool plAgeLoader::MsgReceive(plMessage* msg)
{
    plInitialAgeStateLoadedMsg *stateMsg = plInitialAgeStateLoadedMsg::ConvertNoRef( msg );
    if (stateMsg != nullptr)
    {
        // done receiving the initial state of the age from the server
        return true;
    }

    plClientMsg* clientMsg = plClientMsg::ConvertNoRef(msg);
    if (clientMsg && clientMsg->GetClientMsgFlag()==plClientMsg::kInitComplete)
    {
        ExecPendingAgeFniFiles();   // exec age-specific fni files
        return true;
    }

    // sadface thread protection
    if (plResPatcherMsg::ConvertNoRef(msg)) {
        delete plResPatcher::GetInstance()->fProgress;
        plResPatcher::GetInstance()->fProgress = nullptr;
    }

    return plReceiver::MsgReceive(msg);
}

//
// read in the age desc file and page in/out the rooms belonging to the specified age.
// return false on error
//
//============================================================================
bool plAgeLoader::LoadAge(const ST::string& ageName)
{
    return ILoadAge(ageName);
}

//============================================================================
void plAgeLoader::UpdateAge(const ST::string& ageName)
{
    plResPatcher::GetInstance()->Update(ageName);
}

//============================================================================
void plAgeLoader::NotifyAgeLoaded( bool loaded )
{
    if ( loaded )
        fFlags &= ~kLoadingAge;
    else
        fFlags &= ~kUnLoadingAge;

    plAgeLoadedMsg * msg = new plAgeLoadedMsg;
    msg->fLoaded = loaded;
    msg->Send();
}


//// ILoadAge ////////////////////////////////////////////////////////////////
//  Does the loading-specific stuff for queueing an age to load

bool plAgeLoader::ILoadAge(const ST::string& ageName)
{
    plNetClientApp* nc = plNetClientApp::GetInstance();
    ASSERT(!nc->GetFlagsBit(plNetClientApp::kPlayingGame));

    fAgeName = ageName;

    nc->DebugMsg( "Net: Loading age {}", fAgeName);

    if ((fFlags & kLoadMask) != 0)
        ErrorAssert(__LINE__, __FILE__, "Fatal Error:\nAlready loading or unloading an age.\n%s will now exit.",
                                        plProduct::ShortName().c_str());

    fFlags |= kLoadingAge;
    
    plAgeBeginLoadingMsg* ageBeginLoading = new plAgeBeginLoadingMsg();
    ageBeginLoading->Send();

    ///////////////////////////////////////////////////////


    /// Step 1: Update all of the dat files for this age
    /*
    UpdateAge(fAgeName);
    */

    /// Step 2: Load the keys for this age, so we can find sceneNodes for them
    // exec age .fni file when data is done loading
    fPendingAgeFniFiles.emplace_back(plFileName::Join("dat", ST::format("{}.fni", fAgeName)));
    fPendingAgeCsvFiles.emplace_back(plFileName::Join("dat", ST::format("{}.csv", fAgeName)));

    plSynchEnabler p( false );  // turn off dirty tracking while in this function   

    hsStream* stream=GetAgeDescFileStream(fAgeName);
    if (!stream)
    {
        nc->ErrorMsg("Failed loading age.  Age desc file {} has nil stream", fAgeName);
        fFlags &= ~kLoadingAge;
        return false;
    }

    plAgeDescription ad;
    ad.Read(stream);
    ad.SetAgeName(fAgeName);
    delete stream;
    ad.SeekFirstPage();
    
    plAgePage *page;
    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

    // Copy, exclude pages we want excluded, and collect our scene nodes
    fCurAgeDescription.CopyFrom(ad);
    while ((page = ad.GetNextPage()) != nullptr)
    {
        if( IsPageExcluded( page, fAgeName) )
            continue;

        plKey roomKey = plKeyFinder::Instance().FindSceneNodeKey( fAgeName, page->GetName() );
        if (roomKey != nullptr)
            AddPendingPageInRoomKey( roomKey );
    }
    ad.SeekFirstPage();


    // Tell the client to load-and-hold all the keys for this age, to make the loading process work better
    plClientMsg *loadAgeKeysMsg = new plClientMsg( plClientMsg::kLoadAgeKeys );
    loadAgeKeysMsg->SetAgeName( fAgeName);
    loadAgeKeysMsg->Send( clientKey );

    //
    // Load the Age's SDL Hook object (and it's python modifier)
    //  
    plUoid oid=nc->GetAgeSDLObjectUoid(fAgeName);
    plKey ageSDLObjectKey = hsgResMgr::ResMgr()->FindKey(oid);
    if (ageSDLObjectKey)
        hsgResMgr::ResMgr()->AddViaNotify(ageSDLObjectKey, new plGenRefMsg(nc->GetKey(), plRefMsg::kOnCreate, -1, 
        plNetClientMgr::kAgeSDLHook), plRefFlags::kActiveRef);
    
    int nPages = 0;

    plClientMsg* pMsg1 = new plClientMsg(plClientMsg::kLoadRoom);
    pMsg1->SetAgeName(fAgeName);

    // Loop and ref!
    while ((page = ad.GetNextPage()) != nullptr)
    {
        if( IsPageExcluded( page, fAgeName) )
        {
            nc->DebugMsg("\tExcluding page {}\n", page->GetName());
            continue;
        }

        nPages++;

        pMsg1->AddRoomLoc(ad.CalcPageLocation(page->GetName()));
        nc->DebugMsg("\tPaging in room {}\n", page->GetName());
    }

    pMsg1->Send(clientKey);

    // Send the client a message to let go of the extra keys it was holding on to
    plClientMsg *dumpAgeKeys = new plClientMsg( plClientMsg::kReleaseAgeKeys );
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
//  Registry page iterator to collect all the loaded pages of a given age
//  Note: we have to do an IterateAllPages(), since we want to also catch
//  pages that are partially loaded, which are skipped in the vanilla 
//  IteratePages() call.

class plUnloadAgeCollector : public plRegistryPageIterator
{
    public:
        std::vector<plRegistryPageNode *> fPages;
        const ST::string                fAge;

        plUnloadAgeCollector(const ST::string& a) : fAge( a ) {}

        bool EatPage(plRegistryPageNode *page) override
        {
            if ( !fAge.empty() && page->GetPageInfo().GetAge().compare_i(fAge) == 0 )
            {
                fPages.emplace_back(page);
            }

            return true;
        }
};

//// IUnloadAge //////////////////////////////////////////////////////////////
//  Does the UNloading-specific stuff for queueing an age to unload.
//  Far simpler that ILoadAge :)

bool    plAgeLoader::IUnloadAge()
{
    plNetClientApp* nc = plNetClientApp::GetInstance();
    nc->DebugMsg( "Net: Unloading age {}", fAgeName);

    hsAssert( (fFlags & kLoadMask)==0, "already loading or unloading an age?"); 
    fFlags |= kUnLoadingAge;
    
    plAgeBeginLoadingMsg* msg = new plAgeBeginLoadingMsg();
    msg->fLoading = false;
    msg->Send();

    // Note: instead of going from the .age file, we just want a list of what
    // is REALLY paged in for this age. So ask the resMgr!
    plUnloadAgeCollector collector(fAgeName);
    // WARNING: unsafe cast here, but it's ok, until somebody is mean and makes a non-plResManager resMgr
    ( (plResManager *)hsgResMgr::ResMgr() )->IterateAllPages( &collector );

    // Dat was easy...
    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

    // Build up a list of all the rooms we're going to page out
    plKeyVec newPageOuts;

    for (plRegistryPageNode *page : collector.fPages)
    {
        plKey roomKey = plKeyFinder::Instance().FindSceneNodeKey( page->GetPageInfo().GetLocation() );
        if (roomKey != nullptr && roomKey->ObjectIsLoaded())
        {
            nc->DebugMsg( "\tPaging out room {}\n", page->GetPageInfo().GetPage() );
            newPageOuts.push_back(roomKey);
        }
    }

    // Put them in our pending page outs
    for (const plKey& poKey : newPageOuts)
        fPendingPageOuts.push_back(poKey);

    // ...then send the unload messages.  That way we ensure the list is complete
    // before any messages get processed
    for (const plKey& poKey : newPageOuts)
    {
        plClientMsg *pMsg1 = new plClientMsg( plClientMsg::kUnloadRoom );
        pMsg1->AddRoomLoc(poKey->GetUoid().GetLocation());
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
        plConsoleMsg    *cMsg = new plConsoleMsg( plConsoleMsg::kExecuteFile, fPendingAgeFniFiles[i].AsString() );
        plgDispatch::MsgSend( cMsg );
    }
    fPendingAgeFniFiles.clear();
}

void plAgeLoader::ExecPendingAgeCsvFiles()
{
    int i;
    for (i=0;i<PendingAgeCsvFiles().size(); i++)
    {
        hsStream* stream = plEncryptedStream::OpenEncryptedFile(fPendingAgeCsvFiles[i].AsString());
        if (stream)
        {
            plRelevanceMgr::Instance()->ParseCsvInput(stream);
            delete stream;
        }
    }
    fPendingAgeCsvFiles.clear();
}

//
// return alloced stream or nullptr
// static
//
hsStream* plAgeLoader::GetAgeDescFileStream(const ST::string& ageName)
{
    if (ageName.empty())
        return nullptr;

    plFileName ageDescFileName = plFileName::Join("dat", ST::format("{}.age", ageName));

    hsStream* stream = plEncryptedStream::OpenEncryptedFile(ageDescFileName);
    if (!stream)
    {
        hsAssert(false, ST::format("Can't find age desc file {}", ageDescFileName).c_str());
        return nullptr;
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
