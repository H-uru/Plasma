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

#include <Python.h>
#pragma hdrstop

#include "pyVarSyncMsg.h"
#include "pfGameMgr/pfGameMgr.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base VarSync msg class
//

pyVarSyncMsg::pyVarSyncMsg(): pyGameCliMsg() {}

pyVarSyncMsg::pyVarSyncMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
    if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_VarSync))
        message = nil; // wrong type, just clear it out
}

int pyVarSyncMsg::GetVarSyncMsgType() const
{
    if (message)
        return message->netMsg->messageId;
    return -1;
}

PyObject* pyVarSyncMsg::UpcastToFinalVarSyncMsg() const
{
    if (!message)
        PYTHON_RETURN_NONE;
    switch (message->netMsg->messageId)
    {
    case kSrv2Cli_VarSync_StringVarChanged:
        return pyVarSyncStringVarChangedMsg::New(message);
    case kSrv2Cli_VarSync_NumericVarChanged:
        return pyVarSyncNumericVarChangedMsg::New(message);
    case kSrv2Cli_VarSync_AllVarsSent:
        return pyVarSyncAllVarsSentMsg::New(message);
    case kSrv2Cli_VarSync_StringVarCreated:
        return pyVarSyncStringVarCreatedMsg::New(message);
    case kSrv2Cli_VarSync_NumericVarCreated:
        return pyVarSyncNumericVarCreatedMsg::New(message);
    default:
        PYTHON_RETURN_NONE;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyVarSyncStringVarChangedMsg::pyVarSyncStringVarChangedMsg(): pyVarSyncMsg() {}

pyVarSyncStringVarChangedMsg::pyVarSyncStringVarChangedMsg(pfGameCliMsg* msg): pyVarSyncMsg(msg)
{
    if (message && (message->netMsg->messageId != kSrv2Cli_VarSync_StringVarChanged))
        message = nil; // wrong type, just clear it out
}

unsigned long pyVarSyncStringVarChangedMsg::ID() const
{
    if (message)
    {
        const Srv2Cli_VarSync_StringVarChanged* gmMsg = (const Srv2Cli_VarSync_StringVarChanged*)message->netMsg;
        return gmMsg->varID;
    }
    return 0;
}

std::wstring pyVarSyncStringVarChangedMsg::Value() const
{
    if (message)
    {
        const Srv2Cli_VarSync_StringVarChanged* gmMsg = (const Srv2Cli_VarSync_StringVarChanged*)message->netMsg;
        return gmMsg->varValue;
    }
    return L"";
}

///////////////////////////////////////////////////////////////////////////////
pyVarSyncNumericVarChangedMsg::pyVarSyncNumericVarChangedMsg(): pyVarSyncMsg() {}

pyVarSyncNumericVarChangedMsg::pyVarSyncNumericVarChangedMsg(pfGameCliMsg* msg): pyVarSyncMsg(msg)
{
    if (message && (message->netMsg->messageId != kSrv2Cli_VarSync_NumericVarChanged))
        message = nil; // wrong type, just clear it out
}

unsigned long pyVarSyncNumericVarChangedMsg::ID() const
{
    if (message)
    {
        const Srv2Cli_VarSync_NumericVarChanged* gmMsg = (const Srv2Cli_VarSync_NumericVarChanged*)message->netMsg;
        return gmMsg->varID;
    }
    return 0;
}

double pyVarSyncNumericVarChangedMsg::Value() const
{
    if (message)
    {
        const Srv2Cli_VarSync_NumericVarChanged* gmMsg = (const Srv2Cli_VarSync_NumericVarChanged*)message->netMsg;
        return gmMsg->varValue;
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
pyVarSyncAllVarsSentMsg::pyVarSyncAllVarsSentMsg(): pyVarSyncMsg() {}

pyVarSyncAllVarsSentMsg::pyVarSyncAllVarsSentMsg(pfGameCliMsg* msg): pyVarSyncMsg(msg)
{
    if (message && (message->netMsg->messageId != kSrv2Cli_VarSync_AllVarsSent))
        message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////
pyVarSyncStringVarCreatedMsg::pyVarSyncStringVarCreatedMsg(): pyVarSyncMsg() {}

pyVarSyncStringVarCreatedMsg::pyVarSyncStringVarCreatedMsg(pfGameCliMsg* msg): pyVarSyncMsg(msg)
{
    if (message && (message->netMsg->messageId != kSrv2Cli_VarSync_StringVarCreated))
        message = nil; // wrong type, just clear it out
}

std::wstring pyVarSyncStringVarCreatedMsg::Name() const
{
    if (message)
    {
        const Srv2Cli_VarSync_StringVarCreated* gmMsg = (const Srv2Cli_VarSync_StringVarCreated*)message->netMsg;
        return gmMsg->varName;
    }
    return L"";
}

unsigned long pyVarSyncStringVarCreatedMsg::ID() const
{
    if (message)
    {
        const Srv2Cli_VarSync_StringVarCreated* gmMsg = (const Srv2Cli_VarSync_StringVarCreated*)message->netMsg;
        return gmMsg->varID;
    }
    return 0;
}

std::wstring pyVarSyncStringVarCreatedMsg::Value() const
{
    if (message)
    {
        const Srv2Cli_VarSync_StringVarCreated* gmMsg = (const Srv2Cli_VarSync_StringVarCreated*)message->netMsg;
        return gmMsg->varValue;
    }
    return L"";
}

///////////////////////////////////////////////////////////////////////////////
pyVarSyncNumericVarCreatedMsg::pyVarSyncNumericVarCreatedMsg(): pyVarSyncMsg() {}

pyVarSyncNumericVarCreatedMsg::pyVarSyncNumericVarCreatedMsg(pfGameCliMsg* msg): pyVarSyncMsg(msg)
{
    if (message && (message->netMsg->messageId != kSrv2Cli_VarSync_NumericVarCreated))
        message = nil; // wrong type, just clear it out
}

std::wstring pyVarSyncNumericVarCreatedMsg::Name() const
{
    if (message)
    {
        const Srv2Cli_VarSync_NumericVarCreated* gmMsg = (const Srv2Cli_VarSync_NumericVarCreated*)message->netMsg;
        return gmMsg->varName;
    }
    return L"";
}

unsigned long pyVarSyncNumericVarCreatedMsg::ID() const
{
    if (message)
    {
        const Srv2Cli_VarSync_NumericVarCreated* gmMsg = (const Srv2Cli_VarSync_NumericVarCreated*)message->netMsg;
        return gmMsg->varID;
    }
    return 0;
}

double pyVarSyncNumericVarCreatedMsg::Value() const
{
    if (message)
    {
        const Srv2Cli_VarSync_NumericVarCreated* gmMsg = (const Srv2Cli_VarSync_NumericVarCreated*)message->netMsg;
        return gmMsg->varValue;
    }
    return 0;
}