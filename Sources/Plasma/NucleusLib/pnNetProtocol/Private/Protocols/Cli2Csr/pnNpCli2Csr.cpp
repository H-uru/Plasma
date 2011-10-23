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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2Csr/pnNpCli2Csr.cpp
*   
***/

#define USES_PROTOCOL_CLI2CSR
#include "../../../Pch.h"
#pragma hdrstop


namespace Cli2Csr {
/*****************************************************************************
*
*   Cli2Csr message field definitions
*
***/

static const NetMsgField kPingRequestFields[] = {
    kNetMsgFieldTransId,                    // transId
    kNetMsgFieldTimeMs,                     // pingTimeMs
    NET_MSG_FIELD_VAR_COUNT(1, 64 * 1024),  // payloadBytes
    NET_MSG_FIELD_VAR_PTR(),                // payload
};

static const NetMsgField kRegisterRequestFields[] = {
    kNetMsgFieldTransId,                    // transId
};

static const NetMsgField kLoginRequestFields[] = {
    kNetMsgFieldTransId,                    // transId
    NET_MSG_FIELD_DWORD(),                  // clientChallenge
    kNetMsgFieldAccountName,                // csrName
    kNetMsgFieldShaDigest,                  // challenge
};


/*****************************************************************************
*
*   Csr2Cli message field definitions
*
***/

static const NetMsgField kPingReplyFields[] = {
    kNetMsgFieldTransId,                    // transId
    kNetMsgFieldTimeMs,                     // pingTimeMs
    NET_MSG_FIELD_VAR_COUNT(1, 64 * 1024),  // payloadBytes
    NET_MSG_FIELD_VAR_PTR(),                // payload
};

static const NetMsgField kRegisterReplyFields[] = {
    kNetMsgFieldTransId,                    // transId
    NET_MSG_FIELD_DWORD(),                  // serverChallenge
    NET_MSG_FIELD_DWORD(),                  // latestBuildId
};

static const NetMsgField kLoginReplyFields[] = {
    kNetMsgFieldTransId,                    // transId
    kNetMsgFieldENetError,                  // result
    kNetMsgFieldUuid,                       // csrId
    NET_MSG_FIELD_DWORD(),                  // csrFlags
};


} using namespace Cli2Csr;


/*****************************************************************************
*
*   Exports
*
***/

const NetMsg kNetMsg_Cli2Csr_PingRequest            = NET_MSG(kCli2Csr_PingRequest,         kPingRequestFields);
const NetMsg kNetMsg_Cli2Csr_RegisterRequest        = NET_MSG(kCli2Csr_RegisterRequest,     kRegisterRequestFields);
const NetMsg kNetMsg_Cli2Csr_LoginRequest           = NET_MSG(kCli2Csr_LoginRequest,        kLoginRequestFields);

const NetMsg kNetMsg_Csr2Cli_PingReply              = NET_MSG(kCsr2Cli_PingReply,           kPingReplyFields);
const NetMsg kNetMsg_Csr2Cli_RegisterReply          = NET_MSG(kCsr2Cli_RegisterReply,       kRegisterReplyFields);
const NetMsg kNetMsg_Csr2Cli_LoginReply             = NET_MSG(kCsr2Cli_LoginReply,          kLoginReplyFields);

