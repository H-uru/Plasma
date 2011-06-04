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
#include "pyMarkerGame.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base Marker game client class
//

pyMarkerGame::pyMarkerGame(): pyGameCli() {}

pyMarkerGame::pyMarkerGame(pfGameCli* client): pyGameCli(client)
{
	if (client && (client->GetGameTypeId() != kGameTypeId_Marker))
		gameClient = nil; // wrong type, just clear it out
}

bool pyMarkerGame::IsMarkerGame(std::wstring guid)
{
	Uuid gameUuid(guid.c_str());
	return gameUuid == kGameTypeId_Marker;
}

void pyMarkerGame::CreateMarkerGame(pyKey& callbackKey, unsigned gameType, std::wstring gameName, unsigned long timeLimit, std::wstring templateId)
{
	Marker_CreateParam init;
	init.gameType = gameType;
	StrCopy(init.gameName, gameName.c_str(), arrsize(init.gameName));
	init.timeLimit = timeLimit;
	StrCopy(init.templateID, templateId.c_str(), arrsize(init.templateID));
	pfGameMgr::GetInstance()->CreateGame(callbackKey.getKey(), kGameTypeId_Marker, 0, sizeof(init), &init);
}

void pyMarkerGame::StartGame()
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->StartGame();
	}
}

void pyMarkerGame::PauseGame()
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->PauseGame();
	}
}

void pyMarkerGame::ResetGame()
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->ResetGame();
	}
}

void pyMarkerGame::ChangeGameName(std::wstring newName)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->ChangeGameName(newName.c_str());
	}
}

void pyMarkerGame::ChangeTimeLimit(unsigned long timeLimit)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->ChangeTimeLimit(timeLimit);
	}
}

void pyMarkerGame::DeleteGame()
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->DeleteGame();
	}
}

void pyMarkerGame::AddMarker(double x, double y, double z, std::wstring name, std::wstring age)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->AddMarker(x, y, z, name.c_str(), age.c_str());
	}
}

void pyMarkerGame::DeleteMarker(unsigned long markerId)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->DeleteMarker(markerId);
	}
}

void pyMarkerGame::ChangeMarkerName(unsigned long markerId, std::wstring newName)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->ChangeMarkerName(markerId, newName.c_str());
	}
}

void pyMarkerGame::CaptureMarker(unsigned long markerId)
{
	if (gameClient)
	{
		pfGmMarker* marker = pfGmMarker::ConvertNoRef(gameClient);
		marker->CaptureMarker(markerId);
	}
}