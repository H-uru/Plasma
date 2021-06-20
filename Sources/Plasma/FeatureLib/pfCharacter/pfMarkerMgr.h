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
#ifndef pfMarkerMgr_h_inc
#define pfMarkerMgr_h_inc

#include "HeadSpin.h"

#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/hsKeyedObject.h"

#include <map>

class plStatusLog;

////////////////////////////////////////////////////////////////////////////////

class pfMarkerGame;
class pfMarkerInfo;
class pfMarkerMsg;

class pfMarkerMgr : public hsKeyedObject
{
protected:
    friend class pfMarkerInfo;
    friend class pfMarkerInfoOwned;

    // Because for some reason if I ask for my key, the refs hit zero when I release it (0 initial refs?)
    plKey fMyKey;

    plStatusLog* fLog;

    static pfMarkerMgr* fInstance;

    bool fShowingLocalMarkers;
    bool fMarkersRespawn;
    uint32_t fSelectedMarker;
    static const uint32_t kNoMarkerSelected;
    std::map<uint32_t, pfMarkerInfo*> fMarkers; // key is marker id number

    void IInit();
    void IShutdown();

    pfMarkerInfo* IFindMarker(const plKey& markerKey, uint32_t& id);
    void IUpdate();
    void IMarkerHit(const plKey& markerKey, const plKey& playerKey);

    pfMarkerMgr();
    ~pfMarkerMgr();

public:
    CLASSNAME_REGISTER(pfMarkerMgr);
    GETINTERFACE_ANY(pfMarkerMgr, hsKeyedObject);

    static pfMarkerMgr* Instance();
    static void Shutdown();

    bool MsgReceive(plMessage* msg) override;

    void AddMarker(const struct hsPoint3& pos, uint32_t id, bool justCreated);
    void RemoveMarker(uint32_t id);
    void RemoveAllMarkers();

    void ClearSelectedMarker();
    void SetSelectedMarker(uint32_t id);
    uint32_t GetSelectedMarker();

    void SetMarkersRespawn(bool respawn) {fMarkersRespawn = respawn;}
    bool GetMarkersRespawn() {return fMarkersRespawn;}

    void CaptureMarker(uint32_t id, bool captured); // for QUEST games (no teams)
    void CaptureMarker(uint32_t id, int team); // for TEAM games (0 = not captured)

    // Shows your markers locally, so you can see where they are
    void LocalShowMarkers(bool show = true);
    bool AreLocalMarkersShowing();
};

#endif // pfMarkerMgr_h_inc
