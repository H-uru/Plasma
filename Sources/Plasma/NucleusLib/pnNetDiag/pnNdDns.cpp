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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetDiag/pnNdDns.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Internal types
*
***/

struct DNSParam {
    NetDiag *               diag;
    FNetDiagDumpProc        dump;
    FNetDiagTestCallback    callback;
    void *                  param;
    ENetProtocol            protocol;
    unsigned                srv;
};


/*****************************************************************************
*
*   Internal functions
*
***/

//============================================================================
static void LookupCallback (
    void *              param,
    const wchar_t         name[],
    unsigned            addrCount,
    const NetAddress    addrs[]
) {
    DNSParam * p = (DNSParam *)param;
    if (addrCount) {
        unsigned node = NetAddressGetNode(addrs[0]);
        p->diag->critsect.Enter();
        {
            p->diag->nodes[p->srv] = node;
        }
        p->diag->critsect.Leave();
        wchar_t nodeStr[64];
        NetAddressNodeToString(p->diag->nodes[p->srv], nodeStr, arrsize(nodeStr));
        p->dump(L"[DNS] Success. %s --> %s", name, nodeStr);
        p->callback(
            p->diag,
            p->protocol,
            kNetSuccess,
            p->param
        );
    }
    else {
        p->diag->critsect.Enter();
        {
            // if the hostname still matches, then clear the node
            if (p->diag->hosts[p->srv] && 0 == StrCmp(p->diag->hosts[p->srv], name))
                p->diag->nodes[p->srv] = 0;
        }
        p->diag->critsect.Leave();
        p->dump(L"[DNS] Failed to resolve hostname %s", name);
        p->callback(
            p->diag,
            p->protocol,
            kNetErrNameLookupFailed,
            p->param
        );
    }
    p->diag->DecRef("DNS");
    delete p;
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void DnsStartup () {
}

//============================================================================
void DnsShutdown () {
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetDiagDns (
    NetDiag *               diag,
    ENetProtocol            protocol,
    FNetDiagDumpProc        dump,
    FNetDiagTestCallback    callback,
    void *                  param
) {
    ASSERT(diag);
    ASSERT(dump);
    ASSERT(callback);
    
    unsigned srv = NetProtocolToSrv(protocol);
    if (srv == kNumDiagSrvs) {
        dump(L"[DNS] Unsupported protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetErrNotSupported, param);
        return;
    }

    wchar_t * host = nil;
    diag->critsect.Enter();
    {
        if (diag->hosts[srv])
            host = StrDup(diag->hosts[srv]);
    }
    diag->critsect.Leave();

    if (!host) {
        dump(L"[DNS] No hostname set for protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetSuccess, param);
        return;
    }
            
    diag->IncRef("DNS");
    dump(L"[DNS] Looking up %s...", host);

    DNSParam * dnsParam = NEWZERO(DNSParam);
    dnsParam->diag      = diag;
    dnsParam->srv       = srv;
    dnsParam->protocol  = protocol;
    dnsParam->dump      = dump;
    dnsParam->callback  = callback;
    dnsParam->param     = param;

    AsyncCancelId cancelId;
    AsyncAddressLookupName(
        &cancelId,
        LookupCallback,
        host,
        0,
        dnsParam
    );
    free(host);
}
