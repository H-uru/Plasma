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

static wchar_t s_authAddrConsole[64] = {0};
static const wchar_t * s_authAddrs[] = {
    s_authAddrConsole
};


static wchar_t s_fileAddrConsole[64] = {0};
static const wchar_t * s_fileAddrs[] = {
    s_fileAddrConsole
};


static wchar_t s_csrAddrConsole[64] = {0};
static const wchar_t * s_csrAddrs[] = {
    s_csrAddrConsole
};


static wchar_t s_gateKeeperAddrConsole[64] = {0};
static const wchar_t * s_gateKeeperAddrs[] = {
    s_gateKeeperAddrConsole
};


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
// Auth
//============================================================================
unsigned GetAuthSrvHostnames (const wchar_t *** addrs) {

    *addrs = s_authAddrs; 
    return arrsize(s_authAddrs);
}

//============================================================================
void SetAuthSrvHostname (const wchar_t addr[]) {

    wcsncpy(s_authAddrConsole, addr, arrsize(s_authAddrConsole));
}

//============================================================================
// File
//============================================================================
unsigned GetFileSrvHostnames (const wchar_t *** addrs) {

    *addrs = s_fileAddrs; 
    return arrsize(s_fileAddrs);
}

//============================================================================
void SetFileSrvHostname (const wchar_t addr[]) {

    wcsncpy(s_fileAddrConsole, addr, arrsize(s_fileAddrConsole));
}

//============================================================================
// Csr
//============================================================================
unsigned GetCsrSrvHostnames (const wchar_t *** addrs) {

    *addrs = s_csrAddrs; 
    return arrsize(s_csrAddrs);
}

//============================================================================
void SetCsrSrvHostname (const wchar_t addr[]) {

    wcsncpy(s_csrAddrConsole, addr, arrsize(s_csrAddrConsole));
}


//============================================================================
// GateKeeper
//============================================================================
unsigned GetGateKeeperSrvHostnames (const wchar_t *** addrs) {

    *addrs = s_gateKeeperAddrs; 
    return arrsize(s_gateKeeperAddrs);
}

//============================================================================
void SetGateKeeperSrvHostname (const wchar_t addr[]) {
    wcsncpy(s_gateKeeperAddrConsole, addr, arrsize(s_gateKeeperAddrConsole));
}


//============================================================================
// User-visible Server
//============================================================================
static wchar_t s_serverStatusUrl[256] = {0};
static wchar_t s_serverSignupUrl[256] = {0};
static wchar_t s_serverName[256] = {0};

//============================================================================
const wchar_t *GetServerStatusUrl () {
    return s_serverStatusUrl;
}

//============================================================================
void SetServerStatusUrl (const wchar_t url[]) {
    wcsncpy(s_serverStatusUrl, url, arrsize(s_serverStatusUrl));
}

//============================================================================
const wchar_t *GetServerSignupUrl () {
    return s_serverSignupUrl;
}

//============================================================================
void SetServerSignupUrl (const wchar_t url[]) {
    wcsncpy(s_serverSignupUrl, url, arrsize(s_serverSignupUrl));
}

//============================================================================
const wchar_t *GetServerDisplayName () {
    return s_serverName;
}

//============================================================================
void SetServerDisplayName (const wchar_t name[]) {
    wcsncpy(s_serverName, name, arrsize(s_serverName));
}
