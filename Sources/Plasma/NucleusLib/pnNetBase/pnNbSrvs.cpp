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

#include "pnNbSrvs.h"

#if !HS_BUILD_FOR_WIN32
#    include <wchar.h>
#endif


/*****************************************************************************
*
*   Local data
*
***/

static char s_authAddrConsole[64] = {0};
static const char* s_authAddrs[] = {
    s_authAddrConsole
};


static char s_fileAddrConsole[64] = {0};
static const char* s_fileAddrs[] = {
    s_fileAddrConsole
};


static char s_gateKeeperAddrConsole[64] = {0};
static const char* s_gateKeeperAddrs[] = {
    s_gateKeeperAddrConsole
};

static unsigned s_clientPort = 14617;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
// Auth
//============================================================================
unsigned GetAuthSrvHostnames (const char*** addrs) {

    *addrs = s_authAddrs; 
    return arrsize(s_authAddrs);
}

//============================================================================
void SetAuthSrvHostname (const char addr[]) {

    strncpy(s_authAddrConsole, addr, arrsize(s_authAddrConsole));
}

//============================================================================
// File
//============================================================================
unsigned GetFileSrvHostnames (const char*** addrs) {

    *addrs = s_fileAddrs; 
    return arrsize(s_fileAddrs);
}

//============================================================================
void SetFileSrvHostname (const char addr[]) {

    strncpy(s_fileAddrConsole, addr, arrsize(s_fileAddrConsole));
}

//============================================================================
// GateKeeper
//============================================================================
unsigned GetGateKeeperSrvHostnames (const char*** addrs) {

    *addrs = s_gateKeeperAddrs; 
    return arrsize(s_gateKeeperAddrs);
}

//============================================================================
void SetGateKeeperSrvHostname (const char addr[]) {
    strncpy(s_gateKeeperAddrConsole, addr, arrsize(s_gateKeeperAddrConsole));
}

//============================================================================
// Client Port
//============================================================================
unsigned GetClientPort() {
    return s_clientPort;
}

//============================================================================
void SetClientPort(unsigned port) {
    s_clientPort = port;
}


//============================================================================
// User-visible Server
//============================================================================
static char s_serverStatusUrl[256] = {0};
static char s_serverSignupUrl[256] = {0};
static char s_serverName[256] = {0};

//============================================================================
const char* GetServerStatusUrl () {
    return s_serverStatusUrl;
}

//============================================================================
void SetServerStatusUrl (const char url[]) {
    strncpy(s_serverStatusUrl, url, arrsize(s_serverStatusUrl));
}

//============================================================================
const char* GetServerSignupUrl () {
    return s_serverSignupUrl;
}

//============================================================================
void SetServerSignupUrl (const char url[]) {
    strncpy(s_serverSignupUrl, url, arrsize(s_serverSignupUrl));
}

//============================================================================
const char* GetServerDisplayName () {
    return s_serverName;
}

//============================================================================
void SetServerDisplayName (const char name[]) {
    strncpy(s_serverName, name, arrsize(s_serverName));
}
