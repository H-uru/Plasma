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
#include "pyBlueSpiralMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base BlueSpiral msg class
//

pyBlueSpiralMsg::pyBlueSpiralMsg(): pyGameCliMsg() {}

pyBlueSpiralMsg::pyBlueSpiralMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_BlueSpiral))
		message = nil; // wrong type, just clear it out
}

int pyBlueSpiralMsg::GetBlueSpiralMsgType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyBlueSpiralMsg::UpcastToFinalBlueSpiralMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_BlueSpiral_ClothOrder:
		return pyBlueSpiralClothOrderMsg::New(message);
	case kSrv2Cli_BlueSpiral_SuccessfulHit:
		return pyBlueSpiralSuccessfulHitMsg::New(message);
	case kSrv2Cli_BlueSpiral_GameWon:
		return pyBlueSpiralGameWonMsg::New(message);
	case kSrv2Cli_BlueSpiral_GameOver:
		return pyBlueSpiralGameOverMsg::New(message);
	case kSrv2Cli_BlueSpiral_GameStarted:
		return pyBlueSpiralGameStartedMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyBlueSpiralClothOrderMsg::pyBlueSpiralClothOrderMsg(): pyBlueSpiralMsg() {}

pyBlueSpiralClothOrderMsg::pyBlueSpiralClothOrderMsg(pfGameCliMsg* msg): pyBlueSpiralMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_BlueSpiral_ClothOrder))
		message = nil; // wrong type, just clear it out
}

std::vector<int> pyBlueSpiralClothOrderMsg::Order()
{
	std::vector<int> retVal;
	if (message)
	{
		const Srv2Cli_BlueSpiral_ClothOrder* gmMsg = (const Srv2Cli_BlueSpiral_ClothOrder*)message->netMsg;
		for (int i = 0; i < arrsize(gmMsg->order); ++i)
			retVal.push_back(gmMsg->order[i]);
	}
	return retVal;
}

///////////////////////////////////////////////////////////////////////////////
pyBlueSpiralSuccessfulHitMsg::pyBlueSpiralSuccessfulHitMsg(): pyBlueSpiralMsg() {}

pyBlueSpiralSuccessfulHitMsg::pyBlueSpiralSuccessfulHitMsg(pfGameCliMsg* msg): pyBlueSpiralMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_BlueSpiral_SuccessfulHit))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////
pyBlueSpiralGameWonMsg::pyBlueSpiralGameWonMsg(): pyBlueSpiralMsg() {}

pyBlueSpiralGameWonMsg::pyBlueSpiralGameWonMsg(pfGameCliMsg* msg): pyBlueSpiralMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_BlueSpiral_GameWon))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////
pyBlueSpiralGameOverMsg::pyBlueSpiralGameOverMsg(): pyBlueSpiralMsg() {}

pyBlueSpiralGameOverMsg::pyBlueSpiralGameOverMsg(pfGameCliMsg* msg): pyBlueSpiralMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_BlueSpiral_GameOver))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////
pyBlueSpiralGameStartedMsg::pyBlueSpiralGameStartedMsg(): pyBlueSpiralMsg() {}

pyBlueSpiralGameStartedMsg::pyBlueSpiralGameStartedMsg(pfGameCliMsg* msg): pyBlueSpiralMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_BlueSpiral_GameStarted))
		message = nil; // wrong type, just clear it out
}

bool pyBlueSpiralGameStartedMsg::StartSpin()
{
	if (message)
	{
		const Srv2Cli_BlueSpiral_GameStarted* gmMsg = (const Srv2Cli_BlueSpiral_GameStarted*)message->netMsg;
		return gmMsg->startSpin;
	}
	return false;
}
