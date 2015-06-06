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
#include "plString.h"

/*****************************************************************************
*
*   Local data
*
***/

static plString s_authAddrs[] = { "" };
static plString s_fileAddrs[] = { "" };
static plString s_gateKeeperAddrs[] = { "" };

static unsigned s_clientPort = 14617;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
// Auth
//============================================================================
unsigned GetAuthSrvHostnames (const plString*& addrs) {
    addrs = s_authAddrs;
    return arrsize(s_authAddrs);
}

//============================================================================
void SetAuthSrvHostname (const plString& addr) {
    s_authAddrs[0] = addr;
}

//============================================================================
// File
//============================================================================
unsigned GetFileSrvHostnames (const plString*& addrs) {
    addrs = s_fileAddrs;
    return arrsize(s_fileAddrs);
}

//============================================================================
void SetFileSrvHostname (const plString& addr) {
    s_fileAddrs[0] = addr;
}

//============================================================================
// GateKeeper
//============================================================================
unsigned GetGateKeeperSrvHostnames (const plString*& addrs) {
    addrs = s_gateKeeperAddrs;
    return arrsize(s_gateKeeperAddrs);
}

//============================================================================
void SetGateKeeperSrvHostname (const plString& addr) {
    s_gateKeeperAddrs[0] = addr;
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
static plString s_serverStatusUrl;
static plString s_serverSignupUrl;
static plString s_serverName;

//============================================================================
plString GetServerStatusUrl () {
    return s_serverStatusUrl;
}

//============================================================================
void SetServerStatusUrl (const plString& url) {
    s_serverStatusUrl = url;
}

//============================================================================
plString GetServerSignupUrl () {
    return s_serverSignupUrl;
}

//============================================================================
void SetServerSignupUrl (const plString& url) {
    s_serverSignupUrl = url;
}

//============================================================================
plString GetServerDisplayName () {
    return s_serverName;
}

//============================================================================
void SetServerDisplayName (const plString& name) {
    s_serverName = name;
}
