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
#include "pyGameMgrMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base game mgr msg class
//

pyGameMgrMsg::pyGameMgrMsg(): message(nil) {}

pyGameMgrMsg::pyGameMgrMsg(pfGameMgrMsg* msg): message(msg) {}

int pyGameMgrMsg::GetType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyGameMgrMsg::UpcastToInviteReceivedMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	if (message->netMsg->messageId != kSrv2Cli_GameMgr_InviteReceived)
		PYTHON_RETURN_NONE;
	return pyGameMgrInviteReceivedMsg::New(message);
}

PyObject* pyGameMgrMsg::UpcastToInviteRevokedMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	if (message->netMsg->messageId != kSrv2Cli_GameMgr_InviteRevoked)
		PYTHON_RETURN_NONE;
	return pyGameMgrInviteRevokedMsg::New(message);
}

///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyGameMgrInviteReceivedMsg::pyGameMgrInviteReceivedMsg(): pyGameMgrMsg() {}

pyGameMgrInviteReceivedMsg::pyGameMgrInviteReceivedMsg(pfGameMgrMsg* msg): pyGameMgrMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_GameMgr_InviteReceived))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameMgrInviteReceivedMsg::InviterID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteReceived* gmMsg = (const Srv2Cli_GameMgr_InviteReceived*)message->netMsg;
		return gmMsg->inviterId;
	}
	return 0;
}

std::wstring pyGameMgrInviteReceivedMsg::GameTypeID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteReceived* gmMsg = (const Srv2Cli_GameMgr_InviteReceived*)message->netMsg;
		wchar_t buffer[256];
		GuidToString(gmMsg->gameTypeId, buffer, arrsize(buffer));
		return buffer;
	}
	return L"";
}

unsigned long pyGameMgrInviteReceivedMsg::NewGameID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteReceived* gmMsg = (const Srv2Cli_GameMgr_InviteReceived*)message->netMsg;
		return gmMsg->newGameId;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
pyGameMgrInviteRevokedMsg::pyGameMgrInviteRevokedMsg(): pyGameMgrMsg() {}

pyGameMgrInviteRevokedMsg::pyGameMgrInviteRevokedMsg(pfGameMgrMsg* msg): pyGameMgrMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_GameMgr_InviteRevoked))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameMgrInviteRevokedMsg::InviterID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteRevoked* gmMsg = (const Srv2Cli_GameMgr_InviteRevoked*)message->netMsg;
		return gmMsg->inviterId;
	}
	return 0;
}

std::wstring pyGameMgrInviteRevokedMsg::GameTypeID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteRevoked* gmMsg = (const Srv2Cli_GameMgr_InviteRevoked*)message->netMsg;
		wchar_t buffer[256];
		GuidToString(gmMsg->gameTypeId, buffer, arrsize(buffer));
		return buffer;
	}
	return L"";
}

unsigned long pyGameMgrInviteRevokedMsg::NewGameID() const
{
	if (message)
	{
		const Srv2Cli_GameMgr_InviteRevoked* gmMsg = (const Srv2Cli_GameMgr_InviteRevoked*)message->netMsg;
		return gmMsg->newGameId;
	}
	return 0;
}