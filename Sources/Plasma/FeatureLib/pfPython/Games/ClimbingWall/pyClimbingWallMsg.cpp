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
#include "pyClimbingWallMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base climbing wall msg class
//

pyClimbingWallMsg::pyClimbingWallMsg(): pyGameCliMsg() {}

pyClimbingWallMsg::pyClimbingWallMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_ClimbingWall))
		message = nil; // wrong type, just clear it out
}

int pyClimbingWallMsg::GetClimbingWallMsgType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyClimbingWallMsg::UpcastToFinalClimbingWallMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_ClimbingWall_NumBlockersChanged:
		return pyClimbingWallNumBlockersChangedMsg::New(message);
	case kSrv2Cli_ClimbingWall_Ready:
		return pyClimbingWallReadyMsg::New(message);
	case kSrv2Cli_ClimbingWall_BlockersChanged:
		return pyClimbingWallBlockersChangedMsg::New(message);
	case kSrv2Cli_ClimbingWall_PlayerEntered:
		return pyClimbingWallPlayerEnteredMsg::New(message);
	case kSrv2Cli_ClimbingWall_SuitMachineLocked:
		return pyClimbingWallSuitMachineLockedMsg::New(message);
	case kSrv2Cli_ClimbingWall_GameOver:
		return pyClimbingWallGameOverMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}


///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyClimbingWallNumBlockersChangedMsg::pyClimbingWallNumBlockersChangedMsg(): pyClimbingWallMsg() {}

pyClimbingWallNumBlockersChangedMsg::pyClimbingWallNumBlockersChangedMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_NumBlockersChanged))
		message = nil; // wrong type, just clear it out
}

int pyClimbingWallNumBlockersChangedMsg::NewBlockerCount() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_NumBlockersChanged* gmMsg = (const Srv2Cli_ClimbingWall_NumBlockersChanged*)message->netMsg;
		return gmMsg->newBlockerCount;
	}
	return 0;
}

bool pyClimbingWallNumBlockersChangedMsg::LocalOnly() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_Ready* gmMsg = (const Srv2Cli_ClimbingWall_Ready*)message->netMsg;
		return gmMsg->localOnly;
	}
	return true; // safe-guard so we don't screw up other's state if the python does something stupid
}

///////////////////////////////////////////////////////////////////////////////

pyClimbingWallReadyMsg::pyClimbingWallReadyMsg(): pyClimbingWallMsg() {}

pyClimbingWallReadyMsg::pyClimbingWallReadyMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_Ready))
		message = nil; // wrong type, just clear it out
}

int pyClimbingWallReadyMsg::ReadyType() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_Ready* gmMsg = (const Srv2Cli_ClimbingWall_Ready*)message->netMsg;
		return gmMsg->readyType;
	}
	return 0;
}

bool pyClimbingWallReadyMsg::Team1Ready() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_Ready* gmMsg = (const Srv2Cli_ClimbingWall_Ready*)message->netMsg;
		return gmMsg->team1Ready;
	}
	return false;
}

bool pyClimbingWallReadyMsg::Team2Ready() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_Ready* gmMsg = (const Srv2Cli_ClimbingWall_Ready*)message->netMsg;
		return gmMsg->team2Ready;
	}
	return false;
}

bool pyClimbingWallReadyMsg::LocalOnly() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_Ready* gmMsg = (const Srv2Cli_ClimbingWall_Ready*)message->netMsg;
		return gmMsg->localOnly;
	}
	return true; // safe-guard so we don't screw up other's state if the python does something stupid
}

///////////////////////////////////////////////////////////////////////////////

pyClimbingWallBlockersChangedMsg::pyClimbingWallBlockersChangedMsg(): pyClimbingWallMsg() {}

pyClimbingWallBlockersChangedMsg::pyClimbingWallBlockersChangedMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_BlockersChanged))
		message = nil; // wrong type, just clear it out
}

int pyClimbingWallBlockersChangedMsg::TeamNumber() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_BlockersChanged* gmMsg = (const Srv2Cli_ClimbingWall_BlockersChanged*)message->netMsg;
		return gmMsg->teamNumber;
	}
	return 0;
}

std::vector<int> pyClimbingWallBlockersChangedMsg::BlockersSet() const
{
	std::vector<int> retVal;
	if (message)
	{
		const Srv2Cli_ClimbingWall_BlockersChanged* gmMsg = (const Srv2Cli_ClimbingWall_BlockersChanged*)message->netMsg;
		for (unsigned i = 0; i < kClimbingWallMaxBlockers; ++i)
		{
			if (gmMsg->blockersSet[i] != kClimbingWallNoBlocker)
				retVal.push_back(gmMsg->blockersSet[i]);
		}
	}
	return retVal;
}

bool pyClimbingWallBlockersChangedMsg::LocalOnly() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_BlockersChanged* gmMsg = (const Srv2Cli_ClimbingWall_BlockersChanged*)message->netMsg;
		return gmMsg->localOnly;
	}
	return true; // safe-guard so we don't screw up other's state if the python does something stupid
}

///////////////////////////////////////////////////////////////////////////////

pyClimbingWallPlayerEnteredMsg::pyClimbingWallPlayerEnteredMsg(): pyClimbingWallMsg() {}

pyClimbingWallPlayerEnteredMsg::pyClimbingWallPlayerEnteredMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_PlayerEntered))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////

pyClimbingWallSuitMachineLockedMsg::pyClimbingWallSuitMachineLockedMsg(): pyClimbingWallMsg() {}

pyClimbingWallSuitMachineLockedMsg::pyClimbingWallSuitMachineLockedMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_SuitMachineLocked))
		message = nil; // wrong type, just clear it out
}

bool pyClimbingWallSuitMachineLockedMsg::Team1MachineLocked() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_SuitMachineLocked* gmMsg = (const Srv2Cli_ClimbingWall_SuitMachineLocked*)message->netMsg;
		return gmMsg->team1MachineLocked;
	}
	return true; // err on the side of caution
}

bool pyClimbingWallSuitMachineLockedMsg::Team2MachineLocked() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_SuitMachineLocked* gmMsg = (const Srv2Cli_ClimbingWall_SuitMachineLocked*)message->netMsg;
		return gmMsg->team2MachineLocked;
	}
	return true; // err on the side of caution
}

bool pyClimbingWallSuitMachineLockedMsg::LocalOnly() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_SuitMachineLocked* gmMsg = (const Srv2Cli_ClimbingWall_SuitMachineLocked*)message->netMsg;
		return gmMsg->localOnly;
	}
	return true; // safe-guard so we don't screw up other's state if the python does something stupid
}

///////////////////////////////////////////////////////////////////////////////

pyClimbingWallGameOverMsg::pyClimbingWallGameOverMsg(): pyClimbingWallMsg() {}

pyClimbingWallGameOverMsg::pyClimbingWallGameOverMsg(pfGameCliMsg* msg): pyClimbingWallMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_ClimbingWall_GameOver))
		message = nil; // wrong type, just clear it out
}

int pyClimbingWallGameOverMsg::TeamWon() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_GameOver* gmMsg = (const Srv2Cli_ClimbingWall_GameOver*)message->netMsg;
		return gmMsg->teamWon;
	}
	return 0;
}

std::vector<int> pyClimbingWallGameOverMsg::Team1Blockers() const
{
	std::vector<int> retVal;
	if (message)
	{
		const Srv2Cli_ClimbingWall_GameOver* gmMsg = (const Srv2Cli_ClimbingWall_GameOver*)message->netMsg;
		for (unsigned i = 0; i < kClimbingWallMaxBlockers; ++i)
		{
			if (gmMsg->team1Blockers[i] != kClimbingWallNoBlocker)
				retVal.push_back(gmMsg->team1Blockers[i]);
		}
	}
	return retVal;
}

std::vector<int> pyClimbingWallGameOverMsg::Team2Blockers() const
{
	std::vector<int> retVal;
	if (message)
	{
		const Srv2Cli_ClimbingWall_GameOver* gmMsg = (const Srv2Cli_ClimbingWall_GameOver*)message->netMsg;
		for (unsigned i = 0; i < kClimbingWallMaxBlockers; ++i)
		{
			if (gmMsg->team2Blockers[i] != kClimbingWallNoBlocker)
				retVal.push_back(gmMsg->team2Blockers[i]);
		}
	}
	return retVal;
}

bool pyClimbingWallGameOverMsg::LocalOnly() const
{
	if (message)
	{
		const Srv2Cli_ClimbingWall_GameOver* gmMsg = (const Srv2Cli_ClimbingWall_GameOver*)message->netMsg;
		return gmMsg->localOnly;
	}
	return true; // safe-guard so we don't screw up other's state if the python does something stupid
}