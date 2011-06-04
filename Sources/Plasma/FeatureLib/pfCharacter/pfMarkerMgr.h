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
#ifndef pfMarkerMgr_h_inc
#define pfMarkerMgr_h_inc

#include "../pnKeyedObject/hsKeyedObject.h"
#include <map>

class plStatusLog;

////////////////////////////////////////////////////////////////////////////////

class pfMarkerMsg;
class pfMarkerGame;
class pfMarkerInfo;

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
	UInt32 fSelectedMarker;
	static const UInt32 kNoMarkerSelected;
	std::map<UInt32, pfMarkerInfo*> fMarkers; // key is marker id number

	void IInit();
	void IShutdown();

	pfMarkerInfo* IFindMarker(plKey markerKey, UInt32& id);
	void IUpdate();
	void IMarkerHit(plKey markerKey, plKey playerKey);

	pfMarkerMgr();
	~pfMarkerMgr();

public:
	CLASSNAME_REGISTER(pfMarkerMgr);
	GETINTERFACE_ANY(pfMarkerMgr, hsKeyedObject);

	static pfMarkerMgr* Instance();
	static void Shutdown();

	hsBool MsgReceive(plMessage* msg);

	void AddMarker(double x, double y, double z, UInt32 id, bool justCreated);
	void RemoveMarker(UInt32 id);
	void RemoveAllMarkers();

	void ClearSelectedMarker();
	void SetSelectedMarker(UInt32 id);
	UInt32 GetSelectedMarker();

	void SetMarkersRespawn(bool respawn) {fMarkersRespawn = respawn;}
	bool GetMarkersRespawn() {return fMarkersRespawn;}

	void CaptureMarker(UInt32 id, bool captured); // for QUEST games (no teams)
	void CaptureMarker(UInt32 id, int team); // for TEAM games (0 = not captured)

	// Shows your markers locally, so you can see where they are
	void LocalShowMarkers(bool show = true);
	bool AreLocalMarkersShowing();
};

#endif // pfMarkerMgr_h_inc
