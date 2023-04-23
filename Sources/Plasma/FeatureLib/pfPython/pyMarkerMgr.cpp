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
//////////////////////////////////////////////////////////////////////
//
// pyMarkerMgr   - a wrapper class to provide interface to the pfMarkerMgr stuff
//
//////////////////////////////////////////////////////////////////////


#include "pyMarkerMgr.h"
#include "pfCharacter/pfMarkerMgr.h"

#include "pyGeometry3.h"

void pyMarkerMgr::AddMarker(pyPoint3* pos, uint32_t id, bool justCreated)
{
    pfMarkerMgr* mmi = pfMarkerMgr::Instance();
    if (mmi)
        mmi->AddMarker(pos->fPoint, id, justCreated);
}

void pyMarkerMgr::RemoveMarker(uint32_t id)
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

void pyMarkerMgr::SetSelectedMarker(uint32_t markerID)
{
    pfMarkerMgr* mmi = pfMarkerMgr::Instance();
    if (mmi)
        mmi->SetSelectedMarker(markerID);
}

uint32_t pyMarkerMgr::GetSelectedMarker()
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

void pyMarkerMgr::CaptureQuestMarker(uint32_t id, bool captured)
{
    pfMarkerMgr* mmi = pfMarkerMgr::Instance();
    if (mmi)
        mmi->CaptureMarker(id, captured);
}

void pyMarkerMgr::CaptureTeamMarker(uint32_t id, int team)
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
