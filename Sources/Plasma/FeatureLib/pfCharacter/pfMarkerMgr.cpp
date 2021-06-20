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

#include "pfMarkerMgr.h"
#include "pfMarkerInfo.h"

#include "plgDispatch.h"
#include "hsTimer.h"

#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plTimeMsg.h"

#include "plModifier/plCloneSpawnModifier.h"
#include "plMessage/plLoadCloneMsg.h"
#include "plNetClient/plNetClientMgr.h"
#include "plStatusLog/plStatusLog.h"

#include "pfMessage/pfMarkerMsg.h"

////////////////////////////////////////////////////////////////////////////////

pfMarkerMgr* pfMarkerMgr::fInstance = nullptr;
const uint32_t pfMarkerMgr::kNoMarkerSelected = (uint32_t)(-1);

pfMarkerMgr* pfMarkerMgr::Instance()
{
    if (!pfMarkerMgr::fInstance)
    {
        pfMarkerMgr::fInstance = new pfMarkerMgr;
        pfMarkerMgr::fInstance->IInit();
    }

    return pfMarkerMgr::fInstance;
}

void pfMarkerMgr::Shutdown()
{
    if (pfMarkerMgr::fInstance)
    {
        pfMarkerMgr::fInstance->IShutdown();
        pfMarkerMgr::fInstance = nullptr;
    }
}

////////////////////////////////////////////////////////////////////////////////

pfMarkerMgr::pfMarkerMgr(): fSelectedMarker(kNoMarkerSelected), fShowingLocalMarkers(false), fMarkersRespawn(false)
{
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(20, "Marker.log", plStatusLog::kAlignToTop | plStatusLog::kFilledBackground);
    pfMarkerInfo::Init();
}

pfMarkerMgr::~pfMarkerMgr()
{
    delete fLog;
}

void pfMarkerMgr::IInit()
{
    fMyKey = RegisterAs(kMarkerMgr_KEY);
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
}

void pfMarkerMgr::IShutdown()
{
    std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
    while (curMarker != fMarkers.end())
    {
        curMarker->second->Remove();
        delete curMarker->second;
        ++curMarker;
    }
    fMarkers.clear();

    plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
    UnRegisterAs(kMarkerMgr_KEY);
}

pfMarkerInfo* pfMarkerMgr::IFindMarker(const plKey& markerKey, uint32_t& id)
{
    std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
    while (curMarker != fMarkers.end())
    {
        if (curMarker->second->GetKey() == markerKey)
        {
            id = curMarker->first;
            return curMarker->second;
        }
        ++curMarker;
    }
    id = kNoMarkerSelected;
    return nullptr;
}

void pfMarkerMgr::IUpdate()
{
    // Update all markers
    std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
    while (curMarker != fMarkers.end())
    {
        curMarker->second->Update(hsTimer::GetSeconds());
        ++curMarker;
    }
}

void pfMarkerMgr::IMarkerHit(const plKey& markerKey, const plKey& playerKey)
{
    if (playerKey != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
        return; // not the local player, abort

    // make sure the marker isn't frozen
    uint32_t id;
    pfMarkerInfo* hitMarker = IFindMarker(markerKey, id);
    if (!hitMarker)
        return; // abort, something weird is going on
    if (hitMarker->IsFrozen())
        return; // marker frozen, abort

    // tell people about it
    pfMarkerMsg* msg = new pfMarkerMsg;
    msg->fType = pfMarkerMsg::kMarkerCaptured;
    msg->fMarkerID = id;
    msg->Send();
}

void pfMarkerMgr::AddMarker(const hsPoint3& pos, uint32_t id, bool justCreated)
{
    if (fMarkers.find(id) != fMarkers.end())
    {
        // delete existing one if we're changing its location
        fMarkers[id]->Remove();
        delete fMarkers[id];
    }

    fMarkers[id] = new pfMarkerInfo(pos, justCreated);
    fMarkers[id]->Spawn(pfMarkerInfo::kMarkerOpen);
}

void pfMarkerMgr::RemoveMarker(uint32_t id)
{
    if (fMarkers.find(id) == fMarkers.end())
        return;
    fMarkers[id]->Remove();
    delete fMarkers[id];
    fMarkers.erase(id);
}

void pfMarkerMgr::RemoveAllMarkers()
{
    std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
    while (curMarker != fMarkers.end())
    {
        curMarker->second->Remove();
        delete curMarker->second;
        ++curMarker;
    }
    fMarkers.clear();
}

void pfMarkerMgr::ClearSelectedMarker()
{
    if (fSelectedMarker != kNoMarkerSelected)
    {
        if (fMarkers.find(fSelectedMarker) != fMarkers.end())
            fMarkers[fSelectedMarker]->SetType(pfMarkerInfo::kMarkerOpen);
        fSelectedMarker = kNoMarkerSelected;
    }
}

void pfMarkerMgr::SetSelectedMarker(uint32_t id)
{
    ClearSelectedMarker();

    if (id != kNoMarkerSelected)
    {
        fSelectedMarker = id;
        if (fMarkers.find(id) != fMarkers.end())
        {
            fMarkers[id]->SetType(pfMarkerInfo::kMarkerLocalSelected);
        }
    }
}

uint32_t pfMarkerMgr::GetSelectedMarker()
{
    return fSelectedMarker;
}

// for QUEST games (no teams)
void pfMarkerMgr::CaptureMarker(uint32_t id, bool captured)
{
    if (fMarkers.find(id) == fMarkers.end())
        return;

    if (fMarkersRespawn)
        fMarkers[id]->SetFrozen(hsTimer::GetSeconds());
    else
        fMarkers[id]->Show(!captured);

    fMarkers[id]->PlayHitSound();
    fMarkers[id]->SetType(captured ? pfMarkerInfo::kMarkerGreen : pfMarkerInfo::kMarkerOpen);
}

// for TEAM games (0 = not captured)
void pfMarkerMgr::CaptureMarker(uint32_t id, int team)
{
    if (fMarkers.find(id) == fMarkers.end())
        return;

    if (fMarkersRespawn)
        fMarkers[id]->SetFrozen(hsTimer::GetSeconds());
    else
        fMarkers[id]->Show(team == 0); // 0 = uncaptured

    fMarkers[id]->PlayHitSound();
    if (team == 0)
        fMarkers[id]->SetType(pfMarkerInfo::kMarkerOpen);
    else if (team == 1)
        fMarkers[id]->SetType(pfMarkerInfo::kMarkerGreen);
    else
        fMarkers[id]->SetType(pfMarkerInfo::kMarkerRed);
}

void pfMarkerMgr::LocalShowMarkers(bool show)
{
    fShowingLocalMarkers = show;
    if (show)
    {
        std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
        while (curMarker != fMarkers.end())
            curMarker->second->Show(true);
    }
    else
    {
        std::map<uint32_t, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
        while (curMarker != fMarkers.end())
            curMarker->second->Show(false);
    }
}

bool pfMarkerMgr::AreLocalMarkersShowing()
{
    return fShowingLocalMarkers;
}

bool pfMarkerMgr::MsgReceive(plMessage* msg)
{
    plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
    if (evalMsg)
    {
        IUpdate();
        return true;
    }

    // Somebody hit a marker
    plNotifyMsg* notify = plNotifyMsg::ConvertNoRef(msg);
    if (notify)
    {
        proCollisionEventData* cEvent = (proCollisionEventData*)notify->FindEventRecord(proEventData::kCollision);
        if (cEvent)
        {
            plKey markerKey = cEvent->fHittee;
            plKey playerKey = cEvent->fHitter;
            if (plNetClientMgr::GetInstance()->IsAPlayerKey(cEvent->fHittee))
            {
                // swap the above, since the hittee is actually the player
                playerKey = cEvent->fHittee;
                markerKey = cEvent->fHitter;
            }

            IMarkerHit(markerKey, playerKey);
        }

        return true;
    }

    plLoadCloneMsg* cloneMsg = plLoadCloneMsg::ConvertNoRef(msg);
    if (cloneMsg)
    {
        plKey cloneKey = cloneMsg->GetCloneKey();
        if (cloneMsg->GetIsLoading() && cloneKey)
        {
            uint32_t id;
            pfMarkerInfo* marker = IFindMarker(cloneKey, id);
            marker->InitSpawned(cloneKey);
        }

        return true;
    }

    return hsKeyedObject::MsgReceive(msg);
}
