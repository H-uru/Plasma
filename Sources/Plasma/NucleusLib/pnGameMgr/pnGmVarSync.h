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

#ifndef _pnGmVarSync_h_
#define _pnGmVarSync_h_

#include "pnGameMgr.h"

/*****************************************************************************
*
*   Var Sync
*
***/

//============================================================================
//  Game type id
//============================================================================

const plUUID kGameTypeId_VarSync("475c2e9b-a245-4106-a047-9b25d41ff333");

//============================================================================
//  Network message ids
//============================================================================

// Cli2Srv message ids
enum
{
    kCli2Srv_VarSync_SetStringVar = kCli2Srv_NumGameMsgIds,
    kCli2Srv_VarSync_SetNumericVar,
    kCli2Srv_VarSync_RequestAllVars,
    kCli2Srv_VarSync_CreateStringVar,
    kCli2Srv_VarSync_CreateNumericVar,
};

// Srv2Cli message ids
enum
{
    kSrv2Cli_VarSync_StringVarChanged = kSrv2Cli_NumGameMsgIds,
    kSrv2Cli_VarSync_NumericVarChanged,
    kSrv2Cli_VarSync_AllVarsSent,
    kSrv2Cli_VarSync_StringVarCreated,
    kSrv2Cli_VarSync_NumericVarCreated,
};


//============================================================================
// Begin networked data structures
#pragma pack(push, 1)
//============================================================================

    //========================================================================
    // Message parameters
    //========================================================================
    struct VarSync_CreateParam
    {
    };

    //========================================================================
    // VarSync message structures
    //========================================================================

    // Cli2Srv
    struct Cli2Srv_VarSync_SetStringVar : GameMsgHeader
    {
        uint32_t    varID;
        char16_t    varValue[256];
    };

    struct Cli2Srv_VarSync_SetNumericVar : GameMsgHeader
    {
        uint32_t        varID;
        double          varValue;
    };

    struct Cli2Srv_VarSync_RequestAllVars : GameMsgHeader
    {
    };

    struct Cli2Srv_VarSync_CreateStringVar : GameMsgHeader
    {
        char16_t       varName[256];
        char16_t       varValue[256];
    };

    struct Cli2Srv_VarSync_CreateNumericVar : GameMsgHeader
    {
        char16_t       varName[256];
        double         varValue;
    };

    // Srv2Cli
    struct Srv2Cli_VarSync_StringVarChanged : GameMsgHeader
    {
        uint32_t       varID;
        char16_t       varValue[256];
    };

    struct Srv2Cli_VarSync_NumericVarChanged : GameMsgHeader
    {
        uint32_t       varID;
        double         varValue;
    };

    struct Srv2Cli_VarSync_AllVarsSent : GameMsgHeader
    {
    };

    struct Srv2Cli_VarSync_StringVarCreated : GameMsgHeader
    {
        char16_t       varName[256];
        uint32_t       varID;
        char16_t       varValue[256];
    };

    struct Srv2Cli_VarSync_NumericVarCreated : GameMsgHeader
    {
        char16_t       varName[256];
        uint32_t       varID;
        double         varValue;
    };

//============================================================================
// End networked data structures
#pragma pack(pop)
//============================================================================

#endif
