/*==LICENSE==*

CyanWorlds.com Engine - MMOG client, server and tools
Copyright (C) 2011 Cyan Worlds, Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.

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

#ifndef _pnGmClimbingWall_h_
#define _pnGmClimbingWall_h_

#include "pnGameMgr.h"
#include "pnGmClimbingWallConst.h"

/*****************************************************************************
 *
 *   Climbing Wall
 *
 ***/

//============================================================================
//	Game type id
//============================================================================

const plUUID kGameTypeId_ClimbingWall("6224cdf4-3556-4740-b7cd-d637562d07be");

//============================================================================
//	Network message ids
//============================================================================

// Cli2Srv message ids
enum
{
    kCli2Srv_ClimbingWall_ChangeNumBlockers = kCli2Srv_NumGameMsgIds,
    kCli2Srv_ClimbingWall_Ready,
    kCli2Srv_ClimbingWall_BlockerChanged,
    kCli2Srv_ClimbingWall_Reset,
    kCli2Srv_ClimbingWall_PlayerEntered,
    kCli2Srv_ClimbingWall_FinishedGame,
    kCli2Srv_ClimbingWall_Panic,
};

// Srv2Cli message ids
enum
{
    kSrv2Cli_ClimbingWall_NumBlockersChanged = kSrv2Cli_NumGameMsgIds,
    kSrv2Cli_ClimbingWall_Ready,
    kSrv2Cli_ClimbingWall_BlockersChanged,
    kSrv2Cli_ClimbingWall_PlayerEntered,
    kSrv2Cli_ClimbingWall_SuitMachineLocked,
    kSrv2Cli_ClimbingWall_GameOver,
};

//============================================================================
// Begin networked data structures
#pragma pack(push, 1)
//============================================================================

    //========================================================================
    // Message parameters
    //========================================================================
    struct ClimbingWall_CreateParam
    {
        // no params
    };

    //========================================================================
    // Climbing Wall message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_ClimbingWall_ChangeNumBlockers : GameMsgHeader
    {
        int32_t amountToAdjust; // + or - value to adjust the number of blockers by
    };

    struct Cli2Srv_ClimbingWall_Ready : GameMsgHeader
    {
        uint8_t readyType;  // the type of ready this message represents (EClimbingWallReadyType)
        uint8_t teamNumber; // the team that you are saying is ready (1 or 2)
    };

    struct Cli2Srv_ClimbingWall_BlockerChanged : GameMsgHeader
    {
        uint8_t teamNumber;    // the team that is adjusting their blockers
        uint8_t blockerNumber; // the number of the blocker that was added/removed
        bool    added;         // was the blocker added, or removed?
    };

    struct Cli2Srv_ClimbingWall_Reset : GameMsgHeader
    {
        // <no data>
    };

    struct Cli2Srv_ClimbingWall_PlayerEntered : GameMsgHeader
    {
        uint8_t teamNumber; // the team this player is playing for
    };

    struct Cli2Srv_ClimbingWall_FinishedGame : GameMsgHeader
    {
        // <no data>
    };

    struct Cli2Srv_ClimbingWall_Panic : GameMsgHeader
    {
        // <no data>
    };

    // Srv2Cli
    struct Srv2Cli_ClimbingWall_NumBlockersChanged : GameMsgHeader
    {
        uint8_t newBlockerCount; // the new number of blocker we are playing with
        bool    localOnly;       // only adjust your local display, don't net prop
    };

    struct Srv2Cli_ClimbingWall_Ready : GameMsgHeader
    {
        uint8_t readyType; // the type of ready this message represents (EClimbingWallReadyType)
        bool    team1Ready;
        bool    team2Ready;
        bool    localOnly; // only adjust your local display, don't net prop
    };

    struct Srv2Cli_ClimbingWall_BlockersChanged : GameMsgHeader
    {
        uint8_t teamNumber;                            // the team this set of blockers is for
        int32_t blockersSet[kClimbingWallMaxBlockers]; // which blockers are set
        bool    localOnly;                             // only adjust your local display, don't net prop
    };

    struct Srv2Cli_ClimbingWall_PlayerEntered : GameMsgHeader
    {
        // <no data>
    };

    struct Srv2Cli_ClimbingWall_SuitMachineLocked : GameMsgHeader
    {
        bool team1MachineLocked;
        bool team2MachineLocked;
        bool localOnly; // only adjust your local display, don't net prop
    };

    struct Srv2Cli_ClimbingWall_GameOver : GameMsgHeader
    {
        uint8_t teamWon; // which team won the game
        int32_t team1Blockers[kClimbingWallMaxBlockers];
        int32_t team2Blockers[kClimbingWallMaxBlockers];
        bool    localOnly; // only adjust your local display, don't net prop
    };

//============================================================================
// End networked data structures
#pragma pack(pop)
//============================================================================

#endif
