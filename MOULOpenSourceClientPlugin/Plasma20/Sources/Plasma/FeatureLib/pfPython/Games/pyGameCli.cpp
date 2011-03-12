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
#include "pyGameCli.h"

#include "TicTacToe/pyTTTGame.h"
#include "Heek/pyHeekGame.h"
#include "Marker/pyMarkerGame.h"
#include "BlueSpiral/pyBlueSpiralGame.h"
#include "ClimbingWall/pyClimbingWallGame.h"
#include "VarSync/pyVarSyncGame.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base game client class
//

pyGameCli::pyGameCli(): gameClient(nil) {}

pyGameCli::pyGameCli(pfGameCli* client): gameClient(client) {}

std::vector<unsigned> pyGameCli::GetGameIDs()
{
	ARRAY(unsigned) gameIDs;
	std::vector<unsigned> retVal;
	pfGameMgr::GetInstance()->GetGameIds(&gameIDs);
	for (unsigned i = 0; i < gameIDs.Count(); ++i)
		retVal.push_back(gameIDs[i]);
	return retVal;
}

PyObject* pyGameCli::GetGameCli(unsigned gameID)
{
	pfGameCli* client = pfGameMgr::GetInstance()->GetGameCli(gameID);
	if (client)
		return pyGameCli::New(client);
	PYTHON_RETURN_NONE;
}

std::wstring pyGameCli::GetGameNameByTypeID(std::wstring typeID)
{
	Uuid gameUuid(typeID.c_str());
	return pfGameMgr::GetInstance()->GetGameNameByTypeId(gameUuid);
}

void pyGameCli::JoinGame(pyKey& callbackKey, unsigned gameID)
{
	pfGameMgr::GetInstance()->JoinGame(callbackKey.getKey(), gameID);
}

unsigned pyGameCli::GameID() const
{
	if (gameClient)
		return gameClient->GetGameId();
	return 0;
}

std::wstring pyGameCli::GameTypeID() const
{
	if (gameClient)
	{
		wchar_t guidStr[256];
		GuidToString(gameClient->GetGameTypeId(), guidStr, arrsize(guidStr));
		return guidStr;
	}
	return L"";
}

std::wstring pyGameCli::Name() const
{
	if (gameClient)
		return gameClient->GetName();
	return L"";
}

unsigned pyGameCli::PlayerCount() const
{
	if (gameClient)
		return gameClient->GetPlayerCount();
	return 0;
}

void pyGameCli::InvitePlayer(unsigned playerID)
{
	if (gameClient)
		gameClient->InvitePlayer(playerID);
}

void pyGameCli::UninvitePlayer(unsigned playerID)
{
	if (gameClient)
		gameClient->UninvitePlayer(playerID);
}

void pyGameCli::LeaveGame()
{
	if (gameClient)
		gameClient->LeaveGame();
}

PyObject* pyGameCli::UpcastToTTTGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_TicTacToe))
		return pyTTTGame::New(gameClient);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCli::UpcastToHeekGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_Heek))
		return pyHeekGame::New(gameClient);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCli::UpcastToMarkerGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_Marker))
		return pyMarkerGame::New(gameClient);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCli::UpcastToBlueSpiralGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_BlueSpiral))
		return pyBlueSpiralGame::New(gameClient);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCli::UpcastToClimbingWallGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_ClimbingWall))
		return pyClimbingWallGame::New(gameClient);
	PYTHON_RETURN_NONE;
}

PyObject* pyGameCli::UpcastToVarSyncGame()
{
	if (gameClient && (gameClient->GetGameTypeId() == kGameTypeId_VarSync))
		return pyVarSyncGame::New(gameClient);
	PYTHON_RETURN_NONE;
}