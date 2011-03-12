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
//////////////////////////////////////////////////////////////////////
//
// pyMarkerMgr   - a wrapper class to provide interface to the pfMarkerMgr stuff
//
//////////////////////////////////////////////////////////////////////

#include "pyMarkerMgr.h"

#include "../pfCharacter/pfMarkerMgr.h"

void pyMarkerMgr::AddMarker(double x, double y, double z, UInt32 id, bool justCreated)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->AddMarker(x, y, z, id, justCreated);
}

void pyMarkerMgr::RemoveMarker(UInt32 id)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->RemoveMarker(id);
}

void pyMarkerMgr::RemoveAllMarkers()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->RemoveAllMarkers();
}

void pyMarkerMgr::SetSelectedMarker(UInt32 markerID)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->SetSelectedMarker(markerID);
}

UInt32 pyMarkerMgr::GetSelectedMarker()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		return mmi->GetSelectedMarker();
	return 0;
}

void pyMarkerMgr::ClearSelectedMarker()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->ClearSelectedMarker();
}

void pyMarkerMgr::SetMarkersRespawn(bool respawn)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->SetMarkersRespawn(respawn);
}

bool pyMarkerMgr::GetMarkersRespawn()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		return mmi->GetMarkersRespawn();
	return false;
}

void pyMarkerMgr::CaptureQuestMarker(UInt32 id, bool captured)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->CaptureMarker(id, captured);
}

void pyMarkerMgr::CaptureTeamMarker(UInt32 id, int team)
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->CaptureMarker(id, team);
}

// Shows your markers locally, so you can see where they are
void pyMarkerMgr::ShowMarkersLocal()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->LocalShowMarkers(true);
}

void pyMarkerMgr::HideMarkersLocal()
{
	pfMarkerMgr* mmi = pfMarkerMgr::Instance();
	if (mmi)
		mmi->LocalShowMarkers(false);
}

bool pyMarkerMgr::AreLocalMarkersShowing()
{
	return pfMarkerMgr::Instance()->AreLocalMarkersShowing();
}