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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Actual CORE Console Commands and Groups                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleCmd.h"
#include "pnNetBase/Pch.h"

#include <algorithm>

//// DO NOT REMOVE!!!!
//// This is here so Microsoft VC won't decide to "optimize" this file out
PF_CONSOLE_FILE_DUMMY(Core)
//// DO NOT REMOVE!!!!

//
// utility functions
//
//////////////////////////////////////////////////////////////////////////////
void PrintStringF(void pfun(const char *),const char * fmt, ...)
{
    va_list args;

    char buffy[512];
    va_start(args, fmt);
    vsprintf(buffy, fmt, args);
    va_end(args);
    pfun(buffy);
}


/*****************************************************************************
*
*   Server
*
***/

//TODO: Fix Plasma to use OpenSSL's byte order (or better yet, to use OpenSSL),
//      so this hack isn't needed
static void swap_key_bytes(byte keyData[])
{
    for (size_t i = 0; i < (kNetDiffieHellmanKeyBits / 16); ++i)
        std::swap(keyData[i], keyData[ (kNetDiffieHellmanKeyBits / 8) - i - 1 ]);
}

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
    wchar_t *wurl = hsStringToWString((const char *)params[0]);
    SetServerStatusUrl(wurl);
    delete [] wurl;
}

//============================================================================
PF_CONSOLE_CMD(
    Server,
    Signup,
    "string url",
    "Set the server's new user sign-up URL"
) {
    wchar_t *wurl = hsStringToWString((const char *)params[0]);
    SetServerSignupUrl(wurl);
    delete [] wurl;
}

//============================================================================
PF_CONSOLE_CMD(
    Server,
    DispName,
    "string name",
    "Set the displayable server name"
) {
    wchar_t *wname = hsStringToWString((const char *)params[0]);
    SetServerDisplayName(wname);
    delete [] wname;
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
    wchar_t *wHost = hsStringToWString((const char *)params[0]);
    SetFileSrvHostname(wHost);
    delete [] wHost;
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
    wchar_t *wHost = hsStringToWString((const char *)params[0]);
    SetAuthSrvHostname(wHost);
    delete [] wHost;
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    N,
    "string base64Key",
    "Set the Auth Server N key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kAuthDhNData);
    swap_key_bytes(kAuthDhNData);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Auth,
    X,
    "string base64Key",
    "Set the Auth Server X key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kAuthDhXData);
    swap_key_bytes(kAuthDhXData);
}


//============================================================================
// Server.Csr group
PF_CONSOLE_SUBGROUP(Server, Csr)

//============================================================================
PF_CONSOLE_CMD(
    Server_Csr,
    Host,
    "string address",
    "Set the Csr Server address"
) {
    wchar_t *wHost = hsStringToWString((const char *)params[0]);
    SetCsrSrvHostname(wHost);
    delete [] wHost;
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Csr,
    N,
    "string base64Key",
    "Set the Csr Server N key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kCsrDhNData);
    swap_key_bytes(kCsrDhNData);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Csr,
    X,
    "string base64Key",
    "Set the Csr Server X key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kCsrDhXData);
    swap_key_bytes(kCsrDhXData);
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
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kGameDhNData);
    swap_key_bytes(kGameDhNData);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Game,
    X,
    "string base64Key",
    "Set the Game Server X key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kGameDhXData);
    swap_key_bytes(kGameDhXData);
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
    wchar_t *wHost = hsStringToWString((const char *)params[0]);
    SetGateKeeperSrvHostname(wHost);
    delete [] wHost;
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    N,
    "string base64Key",
    "Set the GateKeeper Server N key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kGateKeeperDhNData);
    swap_key_bytes(kGateKeeperDhNData);
}

//============================================================================
PF_CONSOLE_CMD(
    Server_Gate,
    X,
    "string base64Key",
    "Set the GateKeeper Server X key"
) {
    int baseLength = hsStrlen((const char *)params[0]);
    if ((kNetDiffieHellmanKeyBits / 8) != Base64DecodeSize(baseLength, (const char *)params[0])) {
        PrintStringF(PrintString, "Invalid key: should be exactly %u bytes",
                     kNetDiffieHellmanKeyBits / 8);
        return;
    }

    Base64Decode(hsStrlen((const char *)params[0]), (const char *)params[0],
                 kNetDiffieHellmanKeyBits / 8, kGateKeeperDhXData);
    swap_key_bytes(kGateKeeperDhXData);
}
