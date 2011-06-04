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
#include "pyHeekMsg.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base Heek msg class
//

pyHeekMsg::pyHeekMsg(): pyGameCliMsg() {}

pyHeekMsg::pyHeekMsg(pfGameCliMsg* msg): pyGameCliMsg(msg)
{
	if (message && (message->gameCli->GetGameTypeId() != kGameTypeId_Heek))
		message = nil; // wrong type, just clear it out
}

int pyHeekMsg::GetHeekMsgType() const
{
	if (message)
		return message->netMsg->messageId;
	return -1;
}

PyObject* pyHeekMsg::UpcastToFinalHeekMsg() const
{
	if (!message)
		PYTHON_RETURN_NONE;
	switch (message->netMsg->messageId)
	{
	case kSrv2Cli_Heek_PlayGame:
		return pyHeekPlayGameMsg::New(message);
	case kSrv2Cli_Heek_Goodbye:
		return pyHeekGoodbyeMsg::New(message);
	case kSrv2Cli_Heek_Welcome:
		return pyHeekWelcomeMsg::New(message);
	case kSrv2Cli_Heek_Drop:
		return pyHeekDropMsg::New(message);
	case kSrv2Cli_Heek_Setup:
		return pyHeekSetupMsg::New(message);
	case kSrv2Cli_Heek_LightState:
		return pyHeekLightStateMsg::New(message);
	case kSrv2Cli_Heek_InterfaceState:
		return pyHeekInterfaceStateMsg::New(message);
	case kSrv2Cli_Heek_CountdownState:
		return pyHeekCountdownStateMsg::New(message);
	case kSrv2Cli_Heek_WinLose:
		return pyHeekWinLoseMsg::New(message);
	case kSrv2Cli_Heek_GameWin:
		return pyHeekGameWinMsg::New(message);
	case kSrv2Cli_Heek_PointUpdate:
		return pyHeekPointUpdateMsg::New(message);
	default:
		PYTHON_RETURN_NONE;
	}
}

///////////////////////////////////////////////////////////////////////////////
//
// The different messages we can receive
//

pyHeekPlayGameMsg::pyHeekPlayGameMsg(): pyHeekMsg() {}

pyHeekPlayGameMsg::pyHeekPlayGameMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_PlayGame))
		message = nil; // wrong type, just clear it out
}

bool pyHeekPlayGameMsg::IsPlaying() const
{
	if (message)
	{
		const Srv2Cli_Heek_PlayGame* gmMsg = (const Srv2Cli_Heek_PlayGame*)message->netMsg;
		return gmMsg->isPlaying;
	}
	return false;
}

bool pyHeekPlayGameMsg::IsSinglePlayer() const
{
	if (message)
	{
		const Srv2Cli_Heek_PlayGame* gmMsg = (const Srv2Cli_Heek_PlayGame*)message->netMsg;
		return gmMsg->isSinglePlayer;
	}
	return false;
}

bool pyHeekPlayGameMsg::EnableButtons() const
{
	if (message)
	{
		const Srv2Cli_Heek_PlayGame* gmMsg = (const Srv2Cli_Heek_PlayGame*)message->netMsg;
		return gmMsg->enableButtons;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekGoodbyeMsg::pyHeekGoodbyeMsg(): pyHeekMsg() {}

pyHeekGoodbyeMsg::pyHeekGoodbyeMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_Goodbye))
		message = nil; // wrong type, just clear it out
}

///////////////////////////////////////////////////////////////////////////////
pyHeekWelcomeMsg::pyHeekWelcomeMsg(): pyHeekMsg() {}

pyHeekWelcomeMsg::pyHeekWelcomeMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_Welcome))
		message = nil; // wrong type, just clear it out
}

unsigned long pyHeekWelcomeMsg::Points() const
{
	if (message)
	{
		const Srv2Cli_Heek_Welcome* gmMsg = (const Srv2Cli_Heek_Welcome*)message->netMsg;
		return gmMsg->points;
	}
	return 0;
}

unsigned long pyHeekWelcomeMsg::Rank() const
{
	if (message)
	{
		const Srv2Cli_Heek_Welcome* gmMsg = (const Srv2Cli_Heek_Welcome*)message->netMsg;
		return gmMsg->rank;
	}
	return 0;
}

std::wstring pyHeekWelcomeMsg::Name() const
{
	if (message)
	{
		const Srv2Cli_Heek_Welcome* gmMsg = (const Srv2Cli_Heek_Welcome*)message->netMsg;
		return gmMsg->name;
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////
pyHeekDropMsg::pyHeekDropMsg(): pyHeekMsg() {}

pyHeekDropMsg::pyHeekDropMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_Drop))
		message = nil; // wrong type, just clear it out
}

int pyHeekDropMsg::Position() const
{
	if (message)
	{
		const Srv2Cli_Heek_Drop* gmMsg = (const Srv2Cli_Heek_Drop*)message->netMsg;
		return gmMsg->position;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekSetupMsg::pyHeekSetupMsg(): pyHeekMsg() {}

pyHeekSetupMsg::pyHeekSetupMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_Welcome))
		message = nil; // wrong type, just clear it out
}

int pyHeekSetupMsg::Position() const
{
	if (message)
	{
		const Srv2Cli_Heek_Setup* gmMsg = (const Srv2Cli_Heek_Setup*)message->netMsg;
		return gmMsg->position;
	}
	return -1;
}

bool pyHeekSetupMsg::ButtonState() const
{
	if (message)
	{
		const Srv2Cli_Heek_Setup* gmMsg = (const Srv2Cli_Heek_Setup*)message->netMsg;
		return gmMsg->buttonState;
	}
	return false;
}

std::vector<bool> pyHeekSetupMsg::LightOn() const
{
	std::vector<bool> retVal;
	if (message)
	{
		const Srv2Cli_Heek_Setup* gmMsg = (const Srv2Cli_Heek_Setup*)message->netMsg;
		int numLights = arrsize(gmMsg->lightOn);
		for (int i = 0; i < numLights; ++i)
			retVal.push_back(gmMsg->lightOn[i]);
		return retVal;
	}
	return retVal;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekLightStateMsg::pyHeekLightStateMsg(): pyHeekMsg() {}

pyHeekLightStateMsg::pyHeekLightStateMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_LightState))
		message = nil; // wrong type, just clear it out
}

int pyHeekLightStateMsg::LightNum() const
{
	if (message)
	{
		const Srv2Cli_Heek_LightState* gmMsg = (const Srv2Cli_Heek_LightState*)message->netMsg;
		return gmMsg->lightNum;
	}
	return -1;
}

int pyHeekLightStateMsg::State() const
{
	if (message)
	{
		const Srv2Cli_Heek_LightState* gmMsg = (const Srv2Cli_Heek_LightState*)message->netMsg;
		return gmMsg->state;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekInterfaceStateMsg::pyHeekInterfaceStateMsg(): pyHeekMsg() {}

pyHeekInterfaceStateMsg::pyHeekInterfaceStateMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_InterfaceState))
		message = nil; // wrong type, just clear it out
}

bool pyHeekInterfaceStateMsg::ButtonsEnabled() const
{
	if (message)
	{
		const Srv2Cli_Heek_InterfaceState* gmMsg = (const Srv2Cli_Heek_InterfaceState*)message->netMsg;
		return gmMsg->buttonsEnabled;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekCountdownStateMsg::pyHeekCountdownStateMsg(): pyHeekMsg() {}

pyHeekCountdownStateMsg::pyHeekCountdownStateMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_CountdownState))
		message = nil; // wrong type, just clear it out
}

int pyHeekCountdownStateMsg::State() const
{
	if (message)
	{
		const Srv2Cli_Heek_CountdownState* gmMsg = (const Srv2Cli_Heek_CountdownState*)message->netMsg;
		return gmMsg->state;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekWinLoseMsg::pyHeekWinLoseMsg(): pyHeekMsg() {}

pyHeekWinLoseMsg::pyHeekWinLoseMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_WinLose))
		message = nil; // wrong type, just clear it out
}

bool pyHeekWinLoseMsg::Win() const
{
	if (message)
	{
		const Srv2Cli_Heek_WinLose* gmMsg = (const Srv2Cli_Heek_WinLose*)message->netMsg;
		return gmMsg->win;
	}
	return false;
}

int pyHeekWinLoseMsg::Choice() const
{
	if (message)
	{
		const Srv2Cli_Heek_WinLose* gmMsg = (const Srv2Cli_Heek_WinLose*)message->netMsg;
		return gmMsg->choice;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekGameWinMsg::pyHeekGameWinMsg(): pyHeekMsg() {}

pyHeekGameWinMsg::pyHeekGameWinMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_GameWin))
		message = nil; // wrong type, just clear it out
}

int pyHeekGameWinMsg::Choice() const
{
	if (message)
	{
		const Srv2Cli_Heek_GameWin* gmMsg = (const Srv2Cli_Heek_GameWin*)message->netMsg;
		return gmMsg->choice;
	}
	return -1;
}

///////////////////////////////////////////////////////////////////////////////
pyHeekPointUpdateMsg::pyHeekPointUpdateMsg(): pyHeekMsg() {}

pyHeekPointUpdateMsg::pyHeekPointUpdateMsg(pfGameCliMsg* msg): pyHeekMsg(msg)
{
	if (message && (message->netMsg->messageId != kSrv2Cli_Heek_PointUpdate))
		message = nil; // wrong type, just clear it out
}

bool pyHeekPointUpdateMsg::DisplayUpdate() const
{
	if (message)
	{
		const Srv2Cli_Heek_PointUpdate* gmMsg = (const Srv2Cli_Heek_PointUpdate*)message->netMsg;
		return gmMsg->displayUpdate;
	}
	return false;
}

unsigned long pyHeekPointUpdateMsg::Points() const
{
	if (message)
	{
		const Srv2Cli_Heek_PointUpdate* gmMsg = (const Srv2Cli_Heek_PointUpdate*)message->netMsg;
		return gmMsg->points;
	}
	return 0;
}

unsigned long pyHeekPointUpdateMsg::Rank() const
{
	if (message)
	{
		const Srv2Cli_Heek_PointUpdate* gmMsg = (const Srv2Cli_Heek_PointUpdate*)message->netMsg;
		return gmMsg->rank;
	}
	return 0;
}