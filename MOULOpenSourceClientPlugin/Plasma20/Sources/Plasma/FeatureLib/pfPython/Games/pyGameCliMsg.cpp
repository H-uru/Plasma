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
#include "pyGameCliMsg.h"
#include "pyGameCli.h"

#include "TicTacToe\pyTTTMsg.h"
#include "Heek\pyHeekMsg.h"
#include "Marker\pyMarkerMsg.h"
#include "BlueSpiral\pyBlueSpiralMsg.h"
#include "ClimbingWall\pyClimbingWallMsg.h"
#include "VarSync/pyVarSyncMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base game cli msg class
//

pyGameCliMsg::pyGameCliMsg(): message(nil) {}

pyGameCliMsg::pyGameCliMsg(pfGameCliMsg* msg): message(msg) {}

int pyGameCliMsg::GetType() const
{
	if (message)
	{
		switch (message->netMsg->messageId)
		{
		case kSrv2Cli_Game_PlayerJoined:
		case kSrv2Cli_Game_PlayerLeft:
		case kSrv2Cli_Game_InviteFailed:
		case kSrv2Cli_Game_OwnerChange:
			return message->netMsg->messageId; // just return the type straight up
		}
		
		// if we get here, it's probably a game message, check the game guid
		if (message->gameCli->GetGameTypeId() == kGameTypeId_TicTacToe)
			return kPyGameCliTTTMsg;

		if (message->gameCli->GetGameTypeId() == kGameTypeId_Heek)
			return kPyGameCliHeekMsg;

		if (message->gameCli->GetGameTypeId() == kGameTypeId_Marker)
			return kPyGameCliMarkerMsg;

		if (message->gameCli->GetGameTypeId() == kGameTypeId_BlueSpiral)
			return kPyGameCliBlueSpiralMsg;

		if (message->gameCli->GetGameTypeId() == kGameTypeId_ClimbingWall)
			return kPyGameCliClimbingWallMsg;

		if (message->gameCli->GetGameTypeId() == kGameTypeId_VarSync)
			return kPyGameCliVarSyncMsg;
	}
	return -1;
}

PyObject* pyGameCliMsg::GetGameCli() const
{
	if (message && (message->gameCli))
		return pyGameCli::New(message->gameCli);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCliMsg::UpcastToFinalGameCliMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_Game_PlayerJoined:
		return pyGameCliPlayerJoinedMsg::New(message);
	case kSrv2Cli_Game_PlayerLeft:
		return pyGameCliPlayerLeftMsg::New(message);
	case kSrv2Cli_Game_InviteFailed:
		return pyGameCliInviteFailedMsg::New(message);
	case kSrv2Cli_Game_OwnerChange:
		return pyGameCliOwnerChangeMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}

PyObject* pyGameCliMsg::UpcastToGameMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;

	const Uuid& gameTypeId = message->gameCli->GetGameTypeId();
	if (gameTypeId == kGameTypeId_TicTacToe)
		return pyTTTMsg::New(message);
	else if (gameTypeId == kGameTypeId_Heek)
		return pyHeekMsg::New(message);
	else if (gameTypeId == kGameTypeId_Marker)
		return pyMarkerMsg::New(message);
	else if (gameTypeId == kGameTypeId_BlueSpiral)
		return pyBlueSpiralMsg::New(message);
	else if (gameTypeId == kGameTypeId_ClimbingWall)
		return pyClimbingWallMsg::New(message);
	else if (gameTypeId == kGameTypeId_VarSync)
		return pyVarSyncMsg::New(message);
	else
		PYTHON_RETURN_NONE;
}

///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyGameCliPlayerJoinedMsg::pyGameCliPlayerJoinedMsg(): pyGameCliMsg() {}

pyGameCliPlayerJoinedMsg::pyGameCliPlayerJoinedMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Game_PlayerJoined))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameCliPlayerJoinedMsg::PlayerID() const
{
	if (message)
	{
		const Srv2Cli_Game_PlayerJoined* gmMsg = (const Srv2Cli_Game_PlayerJoined*)message->netMsg;
		return gmMsg->playerId;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
pyGameCliPlayerLeftMsg::pyGameCliPlayerLeftMsg(): pyGameCliMsg() {}

pyGameCliPlayerLeftMsg::pyGameCliPlayerLeftMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Game_PlayerLeft))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameCliPlayerLeftMsg::PlayerID() const
{
	if (message)
	{
		const Srv2Cli_Game_PlayerLeft* gmMsg = (const Srv2Cli_Game_PlayerLeft*)message->netMsg;
		return gmMsg->playerId;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
pyGameCliInviteFailedMsg::pyGameCliInviteFailedMsg(): pyGameCliMsg() {}

pyGameCliInviteFailedMsg::pyGameCliInviteFailedMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Game_InviteFailed))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameCliInviteFailedMsg::InviteeID() const
{
	if (message)
	{
		const Srv2Cli_Game_InviteFailed* gmMsg = (const Srv2Cli_Game_InviteFailed*)message->netMsg;
		return gmMsg->inviteeId;
	}
	return 0;
}

unsigned long pyGameCliInviteFailedMsg::OperationID() const
{
	if (message)
	{
		const Srv2Cli_Game_InviteFailed* gmMsg = (const Srv2Cli_Game_InviteFailed*)message->netMsg;
		return gmMsg->operationId;
	}
	return 0;
}

int pyGameCliInviteFailedMsg::Error() const
{
	if (message)
	{
		const Srv2Cli_Game_InviteFailed* gmMsg = (const Srv2Cli_Game_InviteFailed*)message->netMsg;
		return gmMsg->error;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
pyGameCliOwnerChangeMsg::pyGameCliOwnerChangeMsg(): pyGameCliMsg() {}

pyGameCliOwnerChangeMsg::pyGameCliOwnerChangeMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Game_OwnerChange))
		message = nil; // wrong type, just clear it out
}

unsigned long pyGameCliOwnerChangeMsg::OwnerID() const
{
	if (message)
	{
		const Srv2Cli_Game_OwnerChange* gmMsg = (const Srv2Cli_Game_OwnerChange*)message->netMsg;
		return gmMsg->ownerId;
	}
	return 0;
}