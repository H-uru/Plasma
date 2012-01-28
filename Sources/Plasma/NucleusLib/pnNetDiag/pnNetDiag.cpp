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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetDiag/pnNetDiag.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


namespace ND {

/*****************************************************************************
*
*   Local data
*
***/


/*****************************************************************************
*
*   Module data
*
***/

HMODULE     g_lib;
//const wchar_t   g_version[] = L"miasma";
//const wchar_t   g_version[] = L"ectomorph";
const wchar_t g_version[] = L"solvent";

} // namespace ND


/*****************************************************************************
*
*   NetDiag
*
***/

//============================================================================
NetDiag::~NetDiag () {

    for (unsigned srv = 0; srv < kNumDiagSrvs; ++srv)
        free(hosts[srv]);
}

//============================================================================
void NetDiag::SetHost (unsigned srv, const wchar_t host[]) {

    critsect.Enter();
    {       
        free(hosts[srv]);
        
        if (host)
            hosts[srv] = StrDup(host);
        else
            hosts[srv] = nil;
            
        nodes[srv] = 0;
    }
    critsect.Leave();
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetDiagInitialize () {
    
    g_lib = LoadLibrary("Iphlpapi.dll");

    SysStartup();
    DnsStartup();
    IcmpStartup();
    TcpStartup();
    
}

//============================================================================
void NetDiagDestroy () {

    TcpShutdown();
    IcmpShutdown();
    DnsShutdown();
    SysShutdown();

    if (g_lib) {
        FreeLibrary(g_lib);
        g_lib = nil;
    }
}

//============================================================================
NetDiag * NetDiagCreate () {

    NetDiag * diag = NEWZERO(NetDiag);
    diag->IncRef("Lifetime");
    return diag;
}

//============================================================================
void NetDiagDelete (NetDiag * diag) {

    ASSERT(!diag->destroyed);
    diag->destroyed = true;
    diag->DecRef("Lifetime");
}

//============================================================================
void NetDiagSetHost (
    NetDiag *       diag,
    ENetProtocol    protocol,
    const wchar_t     host[]
) {
    ASSERT(diag);
    
    unsigned srv = NetProtocolToSrv(protocol);
    if (srv == kNumDiagSrvs)
        return;
        
    diag->SetHost(srv, host);
}
