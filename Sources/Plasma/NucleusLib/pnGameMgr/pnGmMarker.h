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

#ifndef _pnGmMarker_h_
#define _pnGmMarker_h_

#include "pnGameMgr.h"
#include "pnGmMarkerConst.h"

/*****************************************************************************
*
*   Marker
*
***/

//============================================================================
//  Game type id
//============================================================================

const plUUID kGameTypeId_Marker("000b2c39-0319-4be1-b06c-7a105b160fcf");

//============================================================================
//  Network message ids
//============================================================================

// Cli2Srv message ids
enum
{
    kCli2Srv_Marker_StartGame = kCli2Srv_NumGameMsgIds,
    kCli2Srv_Marker_PauseGame,
    kCli2Srv_Marker_ResetGame,
    kCli2Srv_Marker_ChangeGameName,
    kCli2Srv_Marker_ChangeTimeLimit,
    kCli2Srv_Marker_DeleteGame,
    kCli2Srv_Marker_AddMarker,
    kCli2Srv_Marker_DeleteMarker,
    kCli2Srv_Marker_ChangeMarkerName,
    kCli2Srv_Marker_CaptureMarker,
};

// Srv2Cli message ids
enum
{
    kSrv2Cli_Marker_TemplateCreated = kSrv2Cli_NumGameMsgIds,
    kSrv2Cli_Marker_TeamAssigned,
    kSrv2Cli_Marker_GameType,
    kSrv2Cli_Marker_GameStarted,
    kSrv2Cli_Marker_GamePaused,
    kSrv2Cli_Marker_GameReset,
    kSrv2Cli_Marker_GameOver,
    kSrv2Cli_Marker_GameNameChanged,
    kSrv2Cli_Marker_TimeLimitChanged,
    kSrv2Cli_Marker_GameDeleted,
    kSrv2Cli_Marker_MarkerAdded,
    kSrv2Cli_Marker_MarkerDeleted,
    kSrv2Cli_Marker_MarkerNameChanged,
    kSrv2Cli_Marker_MarkerCaptured,
};


//============================================================================
// Begin networked data structures
#pragma pack(push, 1)
//============================================================================

    //========================================================================
    // Message parameters
    //========================================================================
    struct Marker_CreateParam
    {
        EMarkerGameType gameType;       // member of EMarkerGameType
        char16_t        gameName[256];
        uint32_t        timeLimit;
        char16_t        templateID[80]; // empty if creating a new game, guid if a quest game and we need to grab the data from the state server
    };

    //========================================================================
    // Tic-Tac-Toe message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_Marker_StartGame : GameMsgHeader
    {
        // nothing
    };

    struct Cli2Srv_Marker_PauseGame : GameMsgHeader
    {
        // nothing
    };

    struct Cli2Srv_Marker_ResetGame : GameMsgHeader
    {
        // nothing
    };

    struct Cli2Srv_Marker_ChangeGameName : GameMsgHeader
    {
        char16_t       gameName[256];
    };

    struct Cli2Srv_Marker_ChangeTimeLimit : GameMsgHeader
    {
        uint32_t       timeLimit;
    };

    struct Cli2Srv_Marker_DeleteGame : GameMsgHeader
    {
        // nothing
    };

    struct Cli2Srv_Marker_AddMarker : GameMsgHeader
    {
        double         x;
        double         y;
        double         z;
        char16_t       name[256];
        char16_t       age[80];
    };

    struct Cli2Srv_Marker_DeleteMarker : GameMsgHeader
    {
        uint32_t       markerID;
    };

    struct Cli2Srv_Marker_ChangeMarkerName : GameMsgHeader
    {
        uint32_t       markerID;
        char16_t       markerName[256];
    };

    struct Cli2Srv_Marker_CaptureMarker : GameMsgHeader
    {
        uint32_t       markerID;
    };

    // Srv2Cli
    struct Srv2Cli_Marker_TemplateCreated : GameMsgHeader
    {
        char16_t       templateID[80];
    };

    struct Srv2Cli_Marker_TeamAssigned : GameMsgHeader
    {
        uint8_t        teamNumber; // 1 or 2
    };

    struct Srv2Cli_Marker_GameType : GameMsgHeader
    {
        EMarkerGameType gameType;
    };

    struct Srv2Cli_Marker_GameStarted : GameMsgHeader
    {
        // nothing
    };

    struct Srv2Cli_Marker_GamePaused : GameMsgHeader
    {
        uint32_t       timeLeft;   // 0 if quest game, since they don't have a timer
    };

    struct Srv2Cli_Marker_GameReset : GameMsgHeader
    {
        // nothing
    };

    struct Srv2Cli_Marker_GameOver : GameMsgHeader
    {
        // nothing
    };

    struct Srv2Cli_Marker_GameNameChanged : GameMsgHeader
    {
        char16_t       newName[256];
    };

    struct Srv2Cli_Marker_TimeLimitChanged : GameMsgHeader
    {
        uint32_t       newTimeLimit;
    };

    struct Srv2Cli_Marker_GameDeleted : GameMsgHeader
    {
        bool           failed; // did the delete fail?
    };

    struct Srv2Cli_Marker_MarkerAdded : GameMsgHeader
    {
        double         x;
        double         y;
        double         z;
        uint32_t       markerID;
        char16_t       name[256];
        char16_t       age[80];
    };

    struct Srv2Cli_Marker_MarkerDeleted : GameMsgHeader
    {
        uint32_t       markerID;
    };

    struct Srv2Cli_Marker_MarkerNameChanged : GameMsgHeader
    {
        uint32_t       markerID;
        char16_t       newName[256];
    };

    struct Srv2Cli_Marker_MarkerCaptured : GameMsgHeader
    {
        uint32_t       markerID;
        uint8_t        team;       // 0 for no team, or for quest games
    };


//============================================================================
// End networked data structures
#pragma pack(pop)
//============================================================================

#endif
