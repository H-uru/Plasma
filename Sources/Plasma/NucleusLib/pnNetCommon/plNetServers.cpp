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

#include "plNetServers.h"


// Server Defns

const char* plNetServerConstants::ServerPrograms[] =
{
#ifndef HS_DEBUGGING
#if HS_BUILD_FOR_WIN32
        "plNetServerAgent.exe",
        "plNetLobbyServer.exe",
        "plNetGameServer.exe",
        "plNetVaultServer.exe",
        "plNetAuthServer.exe",
        "plNetAdminServer.exe",
        "plNetLookupServer.exe",
#elif HS_BUILD_FOR_UNIX
        "plNetServerAgent",
        "plNetLobbyServer",
        "plNetGameServer",
        "plNetVaultServer",
        "plNetAuthServer",
        "plNetAdminServer",
        "plNetLookupServer",
#else
#error "No servers work on this Platform!"
#endif
#else // Debug
#if HS_BUILD_FOR_WIN32
        "plNetServerAgent_dbg.exe",
        "plNetLobbyServer_dbg.exe",
        "plNetGameServer_dbg.exe",
        "plNetVaultServer_dbg.exe",
        "plNetAuthServer_dbg.exe",
        "plNetAdminServer_dbg.exe",
        "plNetLookupServer_dbg.exe",
#elif HS_BUILD_FOR_UNIX
        "plNetServerAgent.dbg",
        "plNetLobbyServer.dbg",
        "plNetGameServer.dbg",
        "plNetVaultServer.dbg",
        "plNetAuthServer.dbg",
        "plNetAdminServer.dbg",
        "plNetLookupServer.dbg",
#else
#error "No servers work on this Platform!"
#endif
#endif
};

//
// STATIC
//
const char* plNetServerConstants::GetServerName(int type) 
{ 
    switch(type)
    {
    default:
//      hsAssert(false, "unknown type"); // not the right place to catch this problem.
        return "UNKNOWN";
    case kAgent:
        return plNetServerAgentConstants::GetName();
    case kLobby:
        return plNetLobbyServerConstants::GetName();
    case kGame:
        return plNetGameServerConstants::GetName();
    case kVault:
        return plNetVaultServerConstants::GetName();
    case kAuth:
        return plNetAuthServerConstants::GetName();
    case kAdmin:
        return plNetAdminServerConstants::GetName();
    case kLookup:
        return plNetLookupServerConstants::GetName();
    case kClient:
        return "plClient";
    }
}

uint16_t plNetServerConstants::GetPort(int type)
{ 
    switch(type)
    {
    default:
//      hsAssert(false, "unknown type"); // not the right place to catch this problem.
        return 0;
    case kGame:
        return 0;
    case kAgent:
        return plNetServerAgentConstants::GetPort();
    case kLobby:
        return plNetLobbyServerConstants::GetPort();
    case kVault:
        return plNetVaultServerConstants::GetPort();
    case kAuth:
        return plNetAuthServerConstants::GetPort();
    case kAdmin:
        return plNetAdminServerConstants::GetPort();
    case kLookup:
        return plNetLookupServerConstants::GetPort();
    }
}
