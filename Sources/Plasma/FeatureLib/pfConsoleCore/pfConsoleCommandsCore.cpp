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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Actual CORE Console Commands and Groups                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleCmd.h"
#include "pnNetBase/pnNetBase.h"
#include <string_theory/codecs>
#include <string_theory/format>

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Core)
//// DO NOT REMOVE!!!!


/*****************************************************************************
*
*   Server
*
***/

//============================================================================
// Server group
PF_CONSOLE_GROUP(Server)

//============================================================================
PF_CONSOLE_CMD(
    Server,
    Status,
    "string url",
    "Set the server's status URL"
) {
    SetServerStatusUrl(params[0]);
}

//============================================================================
PF_CONSOLE_CMD(
    Server,
    Signup,
    "string url",
    "Set the server's new user sign-up URL"
) {
    SetServerSignupUrl(params[0]);
}

//============================================================================
PF_CONSOLE_CMD(
    Server,
    DispName,
    "string name",
    "Set the displayable server name"
) {
    SetServerDisplayName(params[0]);
}

//============================================================================
PF_CONSOLE_CMD(
    Server,
    Port,
    "int port",
    "Set server's port"
) {
    SetClientPort((int)params[0]);
}


//============================================================================
// Server.File group
PF_CONSOLE_SUBGROUP(Server, File)

//============================================================================
PF_CONSOLE_CMD(
    Server_File,
    Host,
    "string address",
    "Set the File Server address"
) {
    SetFileSrvHostname(params[0]);
}


//============================================================================
// Server.Auth group
PF_CONSOLE_SUBGROUP(Server, Auth)

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    Host,
    "string address",
    "Set the Auth Server address"
) {
    SetAuthSrvHostname(params[0]);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    N,
    "string base64Key",
    "Set the Auth Server N key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kAuthDhNData, sizeof(kAuthDhNData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    X,
    "string base64Key",
    "Set the Auth Server X key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kAuthDhXData, sizeof(kAuthDhXData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    G,
    "int GValue",
    "Set the Auth Server G value"
    ) {
    kAuthDhGValue = (int)params[0];
}


//============================================================================
// Server.Game group
PF_CONSOLE_SUBGROUP(Server, Game)

//============================================================================
PF_CONSOLE_CMD(
    Server_Game,
    N,
    "string base64Key",
    "Set the Game Server N key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kGameDhNData, sizeof(kGameDhNData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Game,
    X,
    "string base64Key",
    "Set the Game Server X key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kGameDhXData, sizeof(kGameDhXData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Game,
    G,
    "int GValue",
    "Set the Game Server G value"
    ) {
    kGameDhGValue = (int)params[0];
}


//============================================================================
// Server.Gate group
PF_CONSOLE_SUBGROUP(Server, Gate)

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    Host,
    "string address",
    "Set the GateKeeper Server address"
) {
    SetGateKeeperSrvHostname(params[0]);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    N,
    "string base64Key",
    "Set the GateKeeper Server N key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kGateKeeperDhNData, sizeof(kGateKeeperDhNData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    X,
    "string base64Key",
    "Set the GateKeeper Server X key"
) {
    const ST::string& base64key = params[0];
    ST_ssize_t base64len = ST::base64_decode(base64key, nullptr, 0);
    if ((kNetDiffieHellmanKeyBits / 8) != base64len) {
        PrintString(ST::format("Invalid key: should be exactly {} bytes", kNetDiffieHellmanKeyBits / 8));
        return;
    }

    ST_ssize_t bytes = ST::base64_decode(base64key, kGateKeeperDhXData, sizeof(kGateKeeperDhXData));
    ASSERT(bytes >= 0);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    G,
    "int GValue",
    "Set the GateKeeper Server G value"
    ) {
    kGateKeeperDhGValue = (int)params[0];
}
