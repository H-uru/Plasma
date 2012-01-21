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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetDiag/pnNdIcmp.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Local types
*
***/

typedef HANDLE (PASCAL FAR * FIcmpCreateFile) ();
typedef DWORD (PASCAL FAR * FIcmpSendEcho) (
    HANDLE                  icmpHandle,
    DWORD                   destinationAddress,
    LPVOID                  requestData,
    WORD                    requestSize,
    PIP_OPTION_INFORMATION  options,
    LPVOID                  replyBuffer,
    DWORD                   replySize,
    DWORD                   timeoutMs
);

struct PingParam {
    NetDiag *               diag;
    FNetDiagDumpProc        dump;
    FNetDiagTestCallback    callback;
    void *                  param;
    ENetProtocol            protocol;
    unsigned                srv;
};



/*****************************************************************************
*
*   Local data
*
***/

static const unsigned   kPingCount      = 5;
static const unsigned   kPayloadBytes   = 32;

static FIcmpCreateFile  IcmpCreateFile;
static FIcmpSendEcho    IcmpSendEcho;

static uint8_t             s_payload[kPayloadBytes];


/*****************************************************************************
*
*   Local functions
*
***/

//============================================================================
static const wchar_t * IpStatusToString (ULONG status) {

    switch (status) {
        case IP_SUCCESS:                return L"IP_SUCCESS";
        case IP_BUF_TOO_SMALL:          return L"IP_BUF_TOO_SMALL";
        case IP_DEST_NET_UNREACHABLE:   return L"IP_DEST_NET_UNREACHABLE";
        case IP_DEST_HOST_UNREACHABLE:  return L"IP_DEST_HOST_UNREACHABLE";
        case IP_DEST_PROT_UNREACHABLE:  return L"IP_DEST_PROT_UNREACHABLE";
        case IP_DEST_PORT_UNREACHABLE:  return L"IP_DEST_PORT_UNREACHABLE";
        case IP_NO_RESOURCES:           return L"IP_NO_RESOURCES";
        case IP_BAD_OPTION:             return L"IP_BAD_OPTION";
        case IP_HW_ERROR:               return L"IP_HW_ERROR";
        case IP_PACKET_TOO_BIG:         return L"IP_PACKET_TOO_BIG";
        case IP_REQ_TIMED_OUT:          return L"IP_REQ_TIMED_OUT";
        case IP_BAD_REQ:                return L"IP_BAD_REQ";
        case IP_BAD_ROUTE:              return L"IP_BAD_ROUTE";
        case IP_TTL_EXPIRED_TRANSIT:    return L"IP_TTL_EXPIRED_TRANSIT";
        case IP_TTL_EXPIRED_REASSEM:    return L"IP_TTL_EXPIRED_REASSEM";
        case IP_PARAM_PROBLEM:          return L"IP_PARAM_PROBLEM";
        case IP_SOURCE_QUENCH:          return L"IP_SOURCE_QUENCH";
        case IP_OPTION_TOO_BIG:         return L"IP_OPTION_TOO_BIG";
        case IP_BAD_DESTINATION:        return L"IP_BAD_DESTINATION";
        default:                        return L"Unknown error";
    }
}


//============================================================================
static void __cdecl PingThreadProc (void * param) {

    PingParam * p = (PingParam *)param;

    HANDLE icmp = IcmpCreateFile();
    if (!icmp) {
        p->dump(L"[ICMP] Failed to create ICMP handle");
        p->callback(p->diag, p->protocol, kNetErrFileNotFound, p->param);
        return;
    }

    char addr[64];  
    wchar_t waddr[64];
    NetAddressNodeToString(p->diag->nodes[p->srv], waddr, arrsize(waddr));
    StrToAnsi(addr, waddr, arrsize(addr));
    
    ENetError result = kNetSuccess;

    uint8_t reply[kPayloadBytes + sizeof(ICMP_ECHO_REPLY)];
    
    for (unsigned i = 0; i < kPingCount; ++i) {
        DWORD retval = IcmpSendEcho(
            icmp,
            inet_addr(addr),
            s_payload,
            sizeof(s_payload), 
            NULL,
            reply, 
            sizeof(reply),
            4000
        );
        
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)reply;
        
        if (retval) {
            p->dump(L"[ICMP] Reply from %s. ms=%u", waddr, pEchoReply->RoundTripTime);
        }
        else {
            result = kNetErrConnectFailed;
            p->dump(L"[ICMP] No reply from %s. %s", waddr, IpStatusToString(pEchoReply->Status));
        }
    }
    
    p->callback(p->diag, p->protocol, result, p->param);
    p->diag->DecRef("ICMP");
    delete p;
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void IcmpStartup () {

    if (g_lib) {
        IcmpCreateFile = (FIcmpCreateFile)GetProcAddress(g_lib, "IcmpCreateFile");
        IcmpSendEcho = (FIcmpSendEcho)GetProcAddress(g_lib, "IcmpSendEcho");
    }
    memset(s_payload, (uint8_t)((uintptr_t)&s_payload >> 4), arrsize(s_payload));
}

//============================================================================
void IcmpShutdown () {

    IcmpCreateFile = nil;
    IcmpSendEcho = nil;
}


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetDiagIcmp (
    NetDiag *               diag,
    ENetProtocol            protocol,
    FNetDiagDumpProc        dump,
    FNetDiagTestCallback    callback,
    void *                  param
) {
    ASSERT(diag);
    ASSERT(dump);
    ASSERT(callback);

    if (!IcmpCreateFile || !IcmpSendEcho) {
        dump(L"[ICMP] Failed to load IP helper API");
        callback(diag, protocol, kNetErrNotSupported, param);
        return;
    }

    unsigned srv = NetProtocolToSrv(protocol);
    if (srv == kNumDiagSrvs) {
        dump(L"[ICMP] Unsupported protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetErrNotSupported, param);
        return;
    }

    unsigned node = 0;
    diag->critsect.Enter();
    {
        node = diag->nodes[srv];
    }
    diag->critsect.Leave(); 
    
    if (!node) {
        dump(L"[ICMP] No address set for protocol: %s", NetProtocolToString(protocol));
        callback(diag, protocol, kNetSuccess, param);
        return;
    }

    wchar_t nodeStr[64];
    NetAddressNodeToString(node, nodeStr, arrsize(nodeStr));        
    dump(L"[ICMP] Pinging %s with %u bytes of data...", nodeStr, kPayloadBytes);

    PingParam * pingParam   = NEWZERO(PingParam);
    pingParam->diag         = diag;
    pingParam->srv          = srv;
    pingParam->protocol     = protocol;
    pingParam->dump         = dump;
    pingParam->callback     = callback;
    pingParam->param        = param;
    
    diag->IncRef("ICMP");
    _beginthread(PingThreadProc, 0, pingParam);
}
