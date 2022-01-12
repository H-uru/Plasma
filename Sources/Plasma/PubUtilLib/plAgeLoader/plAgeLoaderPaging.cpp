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

#include "hsTimer.h"
#include "hsResMgr.h"
#include "plgDispatch.h"

#include <algorithm>

#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnKeyedObject/plKey.h"

#include "plMessage/plAgeLoadedMsg.h"
#include "plNetMessage/plNetMessage.h"
#include "plSDL/plSDL.h"
#include "plResMgr/plResManager.h"

#include "plNetClient/plNetClientMgr.h"

//
// if room is reserved or for animations, don't report it to the server.
// the server only cares about rooms which have real, networked objects in them.
// The server already assigns a global room to everyone for things like avatar and builtin state .
//
bool ReportRoomToServer(const plKey &key)
{
    plLocation keyLoc=key->GetUoid().GetLocation();
    bool skip=(keyLoc.IsReserved() || keyLoc.IsVirtual() ||
                // HACK ALERT - replace with new uoid type flags
                (!key->GetName().empty() &&
                    (!key->GetName().compare_ni("global", 6) ||
                    key->GetName().contains("_Male") ||
                    key->GetName().contains("_Female")
                    )
                )
            );  
    
    if (skip)
        hsLogEntry(plNetApp::StaticDebugMsg("Not reporting room {} to server, reserved={}, virtual={}",
            key->GetName(), keyLoc.IsReserved(), keyLoc.IsVirtual()));
    
    return !skip;
}

//
// call when finished paging in a room.
//
void plAgeLoader::FinishedPagingInRoom(plKey* rmKey, int numRms)
{
    if (numRms==0)
        return;
        
    unsigned pendingPageIns = PendingPageIns().size();

    plNetClientApp* nc = plNetClientApp::GetInstance();

    // Send a msg to the server indicating that we have this room paged in
    plNetMsgPagingRoom * pagingMsg = new plNetMsgPagingRoom;
    pagingMsg->SetNetProtocol(kNetProtocolCli2Game);
    int i;
    for(i=0;i<numRms;i++)
    {
        plKey key=rmKey[i];
        if (!RemovePendingPageInRoomKey(key))   // room is done paging in
            continue;   // we didn't queue this room
        
        if (!ReportRoomToServer(key))
            continue;

        pagingMsg->AddRoom(key);        
        hsLogEntry(nc->DebugMsg("\tSending PageIn/RequestState msg, room={}\n", key->GetName()));
    }
    if( pagingMsg->GetNumRooms() > 0 )  // all rooms were reserved
    {
        plNetClientMgr * mgr = plNetClientMgr::GetInstance();
        mgr->AddPendingPagingRoomMsg( pagingMsg );
    }
    else
        delete pagingMsg;

    // If any of these rooms were queued for load by us, then we may be done loading the age.
    if (pendingPageIns != PendingPageIns().size())
    {
        bool ageLoaded = (PendingPageIns().size()==0) && (fFlags & kLoadingAge);
        if (ageLoaded)
        {
            plAgeLoaded2Msg * msg = new plAgeLoaded2Msg;
            msg->Send();
            // join task will call NotifyAgeLoaded for us later
        }
    }
}

//
// called by the client when a room is finished paging out
//
void plAgeLoader::FinishedPagingOutRoom(plKey* rmKey, int numRms)
{
    plNetClientApp* nc = plNetClientApp::GetInstance();
    nc->StayAlive(hsTimer::GetSysSeconds());    // alive

    int i;
    for(i=0;i<numRms;i++)
    {
        plKeyVec::iterator found = std::find( fPendingPageOuts.begin(), fPendingPageOuts.end(), rmKey[ i ] );
        if( found != fPendingPageOuts.end() )
        {
            fPendingPageOuts.erase( found );
            nc->DebugMsg("Finished paging out room {}", rmKey[i]->GetName());
        }
    }

    if (PendingPageOuts().size() == 0  && (fFlags & kUnLoadingAge))
    {
        NotifyAgeLoaded( false );
    }

}


//
// call when starting to page out a room.
// allows server to transfer ownership of room objects to someone else.
//
void plAgeLoader::StartPagingOutRoom(plKey* rmKey, int numRms)
{
    plNetClientApp* nc = plNetClientApp::GetInstance();

    plNetMsgPagingRoom pagingMsg;
    pagingMsg.SetNetProtocol(kNetProtocolCli2Game);
    pagingMsg.SetPagingOut(true);
    int i;
    for(i=0;i<numRms;i++)
    {
        plKey key=rmKey[i];
        if (!ReportRoomToServer(key))
            continue;
    
        pagingMsg.AddRoom(rmKey[i]);
        nc->DebugMsg("\tSending PageOut msg, room={}", rmKey[i]->GetName());
    }

    if (!pagingMsg.GetNumRooms())   // all rooms were reserved
        return;

    nc->SendMsg(&pagingMsg);
}

// Client telling us that this page isn't going to get the start/finish combo
// on page out, most likely because it was a load-and-hold, not a load. So take
// it from our pending list but don't actually process it
// Note: right now it's just a dup of FinishPagingOutRoom(), but since the latter
// might change later, we go ahead and dup to avoid unnecessary bugs later
void plAgeLoader::IgnorePagingOutRoom(plKey* rmKey, int numRms)
{
    plNetClientApp* nc = plNetClientApp::GetInstance();
    nc->StayAlive(hsTimer::GetSysSeconds());    // alive

    int i;
    for(i=0;i<numRms;i++)
    {
        plKeyVec::iterator found = std::find( fPendingPageOuts.begin(), fPendingPageOuts.end(), rmKey[ i ] );
        if( found != fPendingPageOuts.end() )
        {
            fPendingPageOuts.erase( found );
            nc->DebugMsg("Ignoring paged out room {}", rmKey[i]->GetName());
        }
    }

    if (PendingPageOuts().size() == 0  && (fFlags & kUnLoadingAge))
    {
        NotifyAgeLoaded( false );
    }
}

///////////////////////////////////

bool plAgeLoader::IsPendingPageInRoomKey(const plKey& pKey, int *idx) 
{
    if (pKey)
    {
        plKeyVec::iterator result=std::find(fPendingPageIns.begin(), fPendingPageIns.end(), pKey);
        bool found = result!=fPendingPageIns.end();
        if (idx)
            *idx = found ? (int)(result-fPendingPageIns.begin()) : -1;
        return found;
    }
    return false;
}

void plAgeLoader::AddPendingPageInRoomKey(plKey pKey)
{
    if (!IsPendingPageInRoomKey(pKey))
    {
        fPendingPageIns.emplace_back(std::move(pKey));
    }
}

bool plAgeLoader::RemovePendingPageInRoomKey(const plKey& pKey)
{
    int idx;
    if (IsPendingPageInRoomKey(pKey, &idx))
    {
        fPendingPageIns.erase(fPendingPageIns.begin()+idx); // remove key from list
        return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////
//// Page Exclusion //////////////////////////////////////////////////////////
//                                                                          //
// Fun debugging exclude commands (to prevent certain pages from loading)   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

class plExcludePage
{
    public:
        ST::string fPageName;
        ST::string fAgeName;

        plExcludePage() { }
        plExcludePage(const ST::string& p, const ST::string& a)
            : fPageName(p), fAgeName(a)
        { }
};

static std::vector<plExcludePage>  sExcludeList;

void    plAgeLoader::ClearPageExcludeList()
{
    sExcludeList.clear();
}

void    plAgeLoader::AddExcludedPage( const ST::string& pageName, const ST::string& ageName )
{
    sExcludeList.emplace_back(pageName, ageName);
}

bool    plAgeLoader::IsPageExcluded( const plAgePage *page, const ST::string& ageName )
{
    // check page flags
    if (page->GetFlags() & plAgePage::kPreventAutoLoad)
        return true;

    // check exclude list
    ST::string pageName = page->GetName();
    for (const plExcludePage& exclude : sExcludeList)
    {
        if (pageName.compare_i(exclude.fPageName) == 0)
        {
            if (ageName.empty() || exclude.fAgeName.empty() ||
                ageName.compare_i(exclude.fAgeName) == 0)
            {
                return true;
            }
        }
    }

    // Check if pages are excluded due to age SDL vars
    if (page->GetFlags() & plAgePage::kLoadIfSDLPresent) 
    {
        if (IGetInitialAgeState())
        {
            plSimpleStateVariable* sdVar = IGetInitialAgeState()->FindVar(pageName);
            if (!sdVar)
                return true;    // no sdl var, exclude
            
            bool value;
            sdVar->Get(&value);
            return value ? false : true;    // exclude if var is false
        }
        else
            return true;    // no age state, exclude
    }

    return false;
}

