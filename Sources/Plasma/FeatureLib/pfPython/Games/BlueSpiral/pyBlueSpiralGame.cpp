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
#include "pyBlueSpiralGame.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base BlueSpiral game client class
//

pyBlueSpiralGame::pyBlueSpiralGame(): pyGameCli() {}

pyBlueSpiralGame::pyBlueSpiralGame(pfGameCli* client): pyGameCli(client)
{
	if (client && (client->GetGameTypeId() != kGameTypeId_BlueSpiral))
		gameClient = nil; // wrong type, just clear it out
}

bool pyBlueSpiralGame::IsBlueSpiralGame(std::wstring guid)
{
	Uuid gameUuid(guid.c_str());
	return gameUuid == kGameTypeId_BlueSpiral;
}

void pyBlueSpiralGame::JoinCommonBlueSpiralGame(pyKey& callbackKey, unsigned gameID)
{
	BlueSpiral_CreateParam init;
	pfGameMgr::GetInstance()->JoinCommonGame(callbackKey.getKey(), kGameTypeId_BlueSpiral, gameID, sizeof(init), &init);
}

void pyBlueSpiralGame::StartGame()
{
	if (gameClient)
	{
		pfGmBlueSpiral* blueSpiral = pfGmBlueSpiral::ConvertNoRef(gameClient);
		blueSpiral->StartGame();
	}
}

void pyBlueSpiralGame::HitCloth(int clothNum)
{
	if (gameClient)
	{
		pfGmBlueSpiral* blueSpiral = pfGmBlueSpiral::ConvertNoRef(gameClient);
		blueSpiral->HitCloth(clothNum);
	}
}