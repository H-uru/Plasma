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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/Marker/pfGmMarker.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_MARKER_PFGMMARKER_H
#error "Header $/Plasma20/Sources/Plasma/FeatureLib/pfGameMgr/Marker/pfGmMarker.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_FEATURELIB_PFGAMEMGR_MARKER_PFGMMARKER_H


/*****************************************************************************
*
*   pfGmMarker interface
*
***/
class pfGmMarker : public pfGameCli {

	// Encapsulate all implementation details such as member fields
	// in an opaque friend class, in this case that's IMarker.
	friend struct IMarker;
	struct IMarker * internal;

	//========================================================================
	// Required subclass methods
	//--------------------------
	void Recv			(GameMsgHeader * msg, void * param);
	void OnPlayerJoined	(const Srv2Cli_Game_PlayerJoined & msg);
	void OnPlayerLeft	(const Srv2Cli_Game_PlayerLeft & msg);
	void OnInviteFailed	(const Srv2Cli_Game_InviteFailed & msg);
	void OnOwnerChange	(const Srv2Cli_Game_OwnerChange & msg);
	//========================================================================

public:
	#pragma warning(push, 0)
	// These macros produce warnings on W4
	CLASSNAME_REGISTER(pfGmMarker);
	GETINTERFACE_ANY(pfGmMarker, pfGameCli);
	#pragma warning(pop)
	
	pfGmMarker (unsigned gameId, plKey receiver);
	~pfGmMarker ();
	
	//========================================================================
	// Game methods
	//-------------
	void StartGame ();
	void PauseGame ();
	void ResetGame ();
	void ChangeGameName	(const wchar name[]);
	void ChangeTimeLimit (unsigned long timeLimit);
	void DeleteGame ();
	void AddMarker (double x, double y, double z, const wchar name[], const wchar age[]);
	void DeleteMarker (unsigned long markerID);
	void ChangeMarkerName (unsigned long markerID, const wchar name[]);
	void CaptureMarker (unsigned long markerID);
	//========================================================================
};

