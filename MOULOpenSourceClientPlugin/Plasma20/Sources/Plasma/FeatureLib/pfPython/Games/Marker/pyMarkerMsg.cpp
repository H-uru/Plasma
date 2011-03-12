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
#include "pyMarkerMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base Marker msg class
//

pyMarkerMsg::pyMarkerMsg(): pyGameCliMsg() {}

pyMarkerMsg::pyMarkerMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_Marker))
		message = nil; // wrong type, just clear it out
}

int pyMarkerMsg::GetMarkerMsgType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyMarkerMsg::UpcastToFinalMarkerMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_Marker_TemplateCreated:
		return pyMarkerTemplateCreatedMsg::New(message);
	case kSrv2Cli_Marker_TeamAssigned:
		return pyMarkerTeamAssignedMsg::New(message);
	case kSrv2Cli_Marker_GameType:
		return pyMarkerGameTypeMsg::New(message);
	case kSrv2Cli_Marker_GameStarted:
		return pyMarkerGameStartedMsg::New(message);
	case kSrv2Cli_Marker_GamePaused:
		return pyMarkerGamePausedMsg::New(message);
	case kSrv2Cli_Marker_GameReset:
		return pyMarkerGameResetMsg::New(message);
	case kSrv2Cli_Marker_GameOver:
		return pyMarkerGameOverMsg::New(message);
	case kSrv2Cli_Marker_GameNameChanged:
		return pyMarkerGameNameChangedMsg::New(message);
	case kSrv2Cli_Marker_TimeLimitChanged:
		return pyMarkerTimeLimitChangedMsg::New(message);
	case kSrv2Cli_Marker_GameDeleted:
		return pyMarkerGameDeletedMsg::New(message);
	case kSrv2Cli_Marker_MarkerAdded:
		return pyMarkerMarkerAddedMsg::New(message);
	case kSrv2Cli_Marker_MarkerDeleted:
		return pyMarkerMarkerDeletedMsg::New(message);
	case kSrv2Cli_Marker_MarkerNameChanged:
		return pyMarkerMarkerNameChangedMsg::New(message);
	case kSrv2Cli_Marker_MarkerCaptured:
		return pyMarkerMarkerCapturedMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyMarkerTemplateCreatedMsg::pyMarkerTemplateCreatedMsg(): pyMarkerMsg() {}

pyMarkerTemplateCreatedMsg::pyMarkerTemplateCreatedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_TemplateCreated))
		message = nil; // wrong type, just clear it out
}

std::wstring pyMarkerTemplateCreatedMsg::TemplateID() const
{
	if (message)
	{
		const Srv2Cli_Marker_TemplateCreated* gmMsg = (const Srv2Cli_Marker_TemplateCreated*)message->netMsg;
		return gmMsg->templateID;
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerTeamAssignedMsg::pyMarkerTeamAssignedMsg(): pyMarkerMsg() {}

pyMarkerTeamAssignedMsg::pyMarkerTeamAssignedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_TeamAssigned))
		message = nil; // wrong type, just clear it out
}

int pyMarkerTeamAssignedMsg::TeamNumber() const
{
	if (message)
	{
		const Srv2Cli_Marker_TeamAssigned* gmMsg = (const Srv2Cli_Marker_TeamAssigned*)message->netMsg;
		return gmMsg->teamNumber;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGameTypeMsg::pyMarkerGameTypeMsg(): pyMarkerMsg() {}

pyMarkerGameTypeMsg::pyMarkerGameTypeMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameType))
		message = nil; // wrong type, just clear it out
}

int pyMarkerGameTypeMsg::GameType() const
{
	if (message)
	{
		const Srv2Cli_Marker_GameType* gmMsg = (const Srv2Cli_Marker_GameType*)message->netMsg;
		return gmMsg->gameType;
	}
	return 0;
}
 
///////////////////////////////////////////////////////////////////////////////

pyMarkerGameStartedMsg::pyMarkerGameStartedMsg(): pyMarkerMsg() {}

pyMarkerGameStartedMsg::pyMarkerGameStartedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameStarted))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGamePausedMsg::pyMarkerGamePausedMsg(): pyMarkerMsg() {}

pyMarkerGamePausedMsg::pyMarkerGamePausedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GamePaused))
		message = nil; // wrong type, just clear it out
}

unsigned long pyMarkerGamePausedMsg::TimeLeft() const
{
	if (message)
	{
		const Srv2Cli_Marker_GamePaused* gmMsg = (const Srv2Cli_Marker_GamePaused*)message->netMsg;
		return gmMsg->timeLeft;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGameResetMsg::pyMarkerGameResetMsg(): pyMarkerMsg() {}

pyMarkerGameResetMsg::pyMarkerGameResetMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameReset))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGameOverMsg::pyMarkerGameOverMsg(): pyMarkerMsg() {}

pyMarkerGameOverMsg::pyMarkerGameOverMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameOver))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGameNameChangedMsg::pyMarkerGameNameChangedMsg(): pyMarkerMsg() {}

pyMarkerGameNameChangedMsg::pyMarkerGameNameChangedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameNameChanged))
		message = nil; // wrong type, just clear it out
}

std::wstring pyMarkerGameNameChangedMsg::Name() const
{
	if (message)
	{
		const Srv2Cli_Marker_GameNameChanged* gmMsg = (const Srv2Cli_Marker_GameNameChanged*)message->netMsg;
		return gmMsg->newName;
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerTimeLimitChangedMsg::pyMarkerTimeLimitChangedMsg(): pyMarkerMsg() {}

pyMarkerTimeLimitChangedMsg::pyMarkerTimeLimitChangedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_TimeLimitChanged))
		message = nil; // wrong type, just clear it out
}

unsigned long pyMarkerTimeLimitChangedMsg::TimeLimit() const
{
	if (message)
	{
		const Srv2Cli_Marker_TimeLimitChanged* gmMsg = (const Srv2Cli_Marker_TimeLimitChanged*)message->netMsg;
		return gmMsg->newTimeLimit;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerGameDeletedMsg::pyMarkerGameDeletedMsg(): pyMarkerMsg() {}

pyMarkerGameDeletedMsg::pyMarkerGameDeletedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_GameDeleted))
		message = nil; // wrong type, just clear it out
}

bool pyMarkerGameDeletedMsg::Failed() const
{
	if (message)
	{
		const Srv2Cli_Marker_GameDeleted* gmMsg = (const Srv2Cli_Marker_GameDeleted*)message->netMsg;
		return gmMsg->failed;
	}
	return true; // assume it failed if we have a problem
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerMarkerAddedMsg::pyMarkerMarkerAddedMsg(): pyMarkerMsg() {}

pyMarkerMarkerAddedMsg::pyMarkerMarkerAddedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_MarkerAdded))
		message = nil; // wrong type, just clear it out
}

double pyMarkerMarkerAddedMsg::X() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->x;
	}
	return 0;
}

double pyMarkerMarkerAddedMsg::Y() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->y;
	}
	return 0;
}

double pyMarkerMarkerAddedMsg::Z() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->z;
	}
	return 0;
}

unsigned long pyMarkerMarkerAddedMsg::MarkerId() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->markerID;
	}
	return 0;
}

std::wstring pyMarkerMarkerAddedMsg::Name() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->name;
	}
	return L"";
}

std::wstring pyMarkerMarkerAddedMsg::Age() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerAdded* gmMsg = (const Srv2Cli_Marker_MarkerAdded*)message->netMsg;
		return gmMsg->age;
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerMarkerDeletedMsg::pyMarkerMarkerDeletedMsg(): pyMarkerMsg() {}

pyMarkerMarkerDeletedMsg::pyMarkerMarkerDeletedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_MarkerDeleted))
		message = nil; // wrong type, just clear it out
}

unsigned long pyMarkerMarkerDeletedMsg::MarkerId() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerDeleted* gmMsg = (const Srv2Cli_Marker_MarkerDeleted*)message->netMsg;
		return gmMsg->markerID;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerMarkerNameChangedMsg::pyMarkerMarkerNameChangedMsg(): pyMarkerMsg() {}

pyMarkerMarkerNameChangedMsg::pyMarkerMarkerNameChangedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_MarkerNameChanged))
		message = nil; // wrong type, just clear it out
}

unsigned long pyMarkerMarkerNameChangedMsg::MarkerId() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerNameChanged* gmMsg = (const Srv2Cli_Marker_MarkerNameChanged*)message->netMsg;
		return gmMsg->markerID;
	}
	return 0;
}

std::wstring pyMarkerMarkerNameChangedMsg::Name() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerNameChanged* gmMsg = (const Srv2Cli_Marker_MarkerNameChanged*)message->netMsg;
		return gmMsg->newName;
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////

pyMarkerMarkerCapturedMsg::pyMarkerMarkerCapturedMsg(): pyMarkerMsg() {}

pyMarkerMarkerCapturedMsg::pyMarkerMarkerCapturedMsg(pfGameCliMsg* msg): pyMarkerMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Marker_MarkerCaptured))
		message = nil; // wrong type, just clear it out
}

unsigned long pyMarkerMarkerCapturedMsg::MarkerId() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerCaptured* gmMsg = (const Srv2Cli_Marker_MarkerCaptured*)message->netMsg;
		return gmMsg->markerID;
	}
	return 0;
}

unsigned int pyMarkerMarkerCapturedMsg::Team() const
{
	if (message)
	{
		const Srv2Cli_Marker_MarkerCaptured* gmMsg = (const Srv2Cli_Marker_MarkerCaptured*)message->netMsg;
		return gmMsg->team;
	}
	return 0;
}