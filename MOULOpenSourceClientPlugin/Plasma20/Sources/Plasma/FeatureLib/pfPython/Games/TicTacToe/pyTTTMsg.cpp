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
#include "pyTTTMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base TTT msg class
//

pyTTTMsg::pyTTTMsg(): pyGameCliMsg() {}

pyTTTMsg::pyTTTMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_TicTacToe))
		message = nil; // wrong type, just clear it out
}

int pyTTTMsg::GetTTTMsgType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyTTTMsg::UpcastToFinalTTTMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_TTT_GameStarted:
		return pyTTTGameStartedMsg::New(message);
	case kSrv2Cli_TTT_GameOver:
		return pyTTTGameOverMsg::New(message);
	case kSrv2Cli_TTT_MoveMade:
		return pyTTTMoveMadeMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyTTTGameStartedMsg::pyTTTGameStartedMsg(): pyTTTMsg() {}

pyTTTGameStartedMsg::pyTTTGameStartedMsg(pfGameCliMsg* msg): pyTTTMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_TTT_GameStarted))
		message = nil; // wrong type, just clear it out
}

bool pyTTTGameStartedMsg::YourTurn() const
{
	if (message)
	{
		const Srv2Cli_TTT_GameStarted* gmMsg = (const Srv2Cli_TTT_GameStarted*)message->netMsg;
		return gmMsg->yourTurn;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
pyTTTGameOverMsg::pyTTTGameOverMsg(): pyTTTMsg() {}

pyTTTGameOverMsg::pyTTTGameOverMsg(pfGameCliMsg* msg): pyTTTMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_TTT_GameOver))
		message = nil; // wrong type, just clear it out
}

int pyTTTGameOverMsg::Result() const
{
	if (message)
	{
		const Srv2Cli_TTT_GameOver* gmMsg = (const Srv2Cli_TTT_GameOver*)message->netMsg;
		return gmMsg->result;
	}
	return false;
}

unsigned long pyTTTGameOverMsg::WinnerID() const
{
	if (message)
	{
		const Srv2Cli_TTT_GameOver* gmMsg = (const Srv2Cli_TTT_GameOver*)message->netMsg;
		return gmMsg->winnerId;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
pyTTTMoveMadeMsg::pyTTTMoveMadeMsg(): pyTTTMsg() {}

pyTTTMoveMadeMsg::pyTTTMoveMadeMsg(pfGameCliMsg* msg): pyTTTMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_TTT_MoveMade))
		message = nil; // wrong type, just clear it out
}

unsigned long pyTTTMoveMadeMsg::PlayerID() const
{
	if (message)
	{
		const Srv2Cli_TTT_MoveMade* gmMsg = (const Srv2Cli_TTT_MoveMade*)message->netMsg;
		return gmMsg->playerId;
	}
	return false;
}

int pyTTTMoveMadeMsg::Row() const
{
	if (message)
	{
		const Srv2Cli_TTT_MoveMade* gmMsg = (const Srv2Cli_TTT_MoveMade*)message->netMsg;
		return gmMsg->row;
	}
	return false;
}

int pyTTTMoveMadeMsg::Col() const
{
	if (message)
	{
		const Srv2Cli_TTT_MoveMade* gmMsg = (const Srv2Cli_TTT_MoveMade*)message->netMsg;
		return gmMsg->col;
	}
	return false;
}