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
#include "pfMarkerMgr.h"
#include "../pfMessage/pfMarkerMsg.h"
#include "pfMarkerInfo.h"

#include "../plModifier/plCloneSpawnModifier.h"
#include "../plStatusLog/plStatusLog.h"

#include "../plMessage/plLoadCloneMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plNotifyMsg.h"

#include "../plNetClient/plNetClientMgr.h"
#include "plgDispatch.h"

////////////////////////////////////////////////////////////////////////////////

pfMarkerMgr* pfMarkerMgr::fInstance = nil;
const UInt32 pfMarkerMgr::kNoMarkerSelected = (UInt32)(-1);

pfMarkerMgr* pfMarkerMgr::Instance()
{
	if (!pfMarkerMgr::fInstance)
	{
		pfMarkerMgr::fInstance = TRACKED_NEW pfMarkerMgr;
		pfMarkerMgr::fInstance->IInit();
	}

	return pfMarkerMgr::fInstance;
}

void pfMarkerMgr::Shutdown()
{
	if (pfMarkerMgr::fInstance)
	{
		pfMarkerMgr::fInstance->IShutdown();
		pfMarkerMgr::fInstance = nil;
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
	std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
	while (curMarker != fMarkers.end())
	{
		curMarker->second->Remove();
		DEL(curMarker->second);
		++curMarker;
	}
	fMarkers.clear();

	plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
	UnRegisterAs(kMarkerMgr_KEY);
}

pfMarkerInfo* pfMarkerMgr::IFindMarker(plKey markerKey, UInt32& id)
{
	std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
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
	return nil;
}

void pfMarkerMgr::IUpdate()
{
	// Update all markers
	std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
	while (curMarker != fMarkers.end())
	{
		curMarker->second->Update(hsTimer::GetSeconds());
		++curMarker;
	}
}

void pfMarkerMgr::IMarkerHit(plKey markerKey, plKey playerKey)
{
	if (playerKey != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
		return; // not the local player, abort

	// make sure the marker isn't frozen
	UInt32 id;
	pfMarkerInfo* hitMarker = IFindMarker(markerKey, id);
	if (!hitMarker)
		return; // abort, something weird is going on
	if (hitMarker->IsFrozen())
		return; // marker frozen, abort

	// tell people about it
	pfMarkerMsg* msg = TRACKED_NEW pfMarkerMsg;
	msg->fType = pfMarkerMsg::kMarkerCaptured;
	msg->fMarkerID = id;
	msg->Send();
}

void pfMarkerMgr::AddMarker(double x, double y, double z, UInt32 id, bool justCreated)
{
	if (fMarkers.find(id) != fMarkers.end())
	{
		// delete existing one if we're changing its location
		fMarkers[id]->Remove();
		DEL(fMarkers[id]);
	}

	hsPoint3 pos((hsScalar)x, (hsScalar)y, (hsScalar)z);
	fMarkers[id] = TRACKED_NEW pfMarkerInfo(pos, justCreated);
	fMarkers[id]->Spawn(pfMarkerInfo::kMarkerOpen);
}

void pfMarkerMgr::RemoveMarker(UInt32 id)
{
	if (fMarkers.find(id) == fMarkers.end())
		return;
	fMarkers[id]->Remove();
	DEL(fMarkers[id]);
	fMarkers.erase(id);
}

void pfMarkerMgr::RemoveAllMarkers()
{
	std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
	while (curMarker != fMarkers.end())
	{
		curMarker->second->Remove();
		DEL(curMarker->second);
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

void pfMarkerMgr::SetSelectedMarker(UInt32 id)
{
	ClearSelectedMarker();

	if (id != kNoMarkerSelected)
	{
		if (fMarkers.find(id) != fMarkers.end())
		{
			fMarkers[id]->SetType(pfMarkerInfo::kMarkerLocalSelected);
			fSelectedMarker = id;
		}
	}
}

UInt32 pfMarkerMgr::GetSelectedMarker()
{
	return fSelectedMarker;
}

// for QUEST games (no teams)
void pfMarkerMgr::CaptureMarker(UInt32 id, bool captured)
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
void pfMarkerMgr::CaptureMarker(UInt32 id, int team)
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
		std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
		while (curMarker != fMarkers.end())
			curMarker->second->Show(true);
	}
	else
	{
		std::map<UInt32, pfMarkerInfo*>::iterator curMarker = fMarkers.begin();
		while (curMarker != fMarkers.end())
			curMarker->second->Show(false);
	}
}

bool pfMarkerMgr::AreLocalMarkersShowing()
{
	return fShowingLocalMarkers;
}

hsBool pfMarkerMgr::MsgReceive(plMessage* msg)
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
			UInt32 id;
			pfMarkerInfo* marker = IFindMarker(cloneKey, id);
			marker->InitSpawned(cloneKey);
		}

		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}