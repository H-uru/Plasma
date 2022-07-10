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

#ifndef _pnGmBlueSpiral_h_
#define _pnGmBlueSpiral_h_

#include "pnGameMgr.h"

/*****************************************************************************
*
*   BlueSpiral
*
***/

//============================================================================
//  Game type id
//============================================================================

const plUUID kGameTypeId_BlueSpiral("5ff98165-913e-4fd1-a2c2-9c7f31be2cc8");

//============================================================================
//  Network message ids
//============================================================================

// Cli2Srv message ids
enum
{
    kCli2Srv_BlueSpiral_StartGame = kCli2Srv_NumGameMsgIds,
    kCli2Srv_BlueSpiral_HitCloth,
};

// Srv2Cli message ids
enum
{
    kSrv2Cli_BlueSpiral_ClothOrder = kSrv2Cli_NumGameMsgIds,
    kSrv2Cli_BlueSpiral_SuccessfulHit,
    kSrv2Cli_BlueSpiral_GameWon,
    kSrv2Cli_BlueSpiral_GameOver, // sent on time out and incorrect entry
    kSrv2Cli_BlueSpiral_GameStarted,
};


//============================================================================
// Begin networked data scructures
#pragma pack(push, 1)
//============================================================================

    //========================================================================
    // Message parameters
    //========================================================================
    struct BlueSpiral_CreateParam
    {
        // empty
    };

    //========================================================================
    // Tic-Tac-Toe message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_BlueSpiral_StartGame : GameMsgHeader
    {
        // empty
    };

    struct Cli2Srv_BlueSpiral_HitCloth : GameMsgHeader
    {
        uint8_t clothNum; // the cloth we hit, 0..6
    };

    // Srv2Cli
    struct Srv2Cli_BlueSpiral_ClothOrder : GameMsgHeader
    {
        uint8_t order[7]; // each value is the cloth to hit, 0..6, the order is the order in the array
    };

    struct Srv2Cli_BlueSpiral_SuccessfulHit : GameMsgHeader
    {
        // empty
    };

    struct Srv2Cli_BlueSpiral_GameWon : GameMsgHeader
    {
        // empty
    };

    struct Srv2Cli_BlueSpiral_GameOver : GameMsgHeader
    {
        // empty
    };

    struct Srv2Cli_BlueSpiral_GameStarted : GameMsgHeader
    {
        bool startSpin; // if true, start spinning the door thingy
    };

//============================================================================
// End networked data structures
#pragma pack(pop)
//============================================================================

#endif
