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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "../../pyKey.h"
#pragma hdrstop

#include "pyVarSyncGame.h"
#include "pfGameMgr/pfGameMgr.h"

///////////////////////////////////////////////////////////////////////////////
//
// Base VarSync game client class
//

const unsigned kGameId = 255; // random number that should be high enough to avoid collisions with other global games

pyVarSyncGame::pyVarSyncGame(): pyGameCli() {}

pyVarSyncGame::pyVarSyncGame(pfGameCli* client): pyGameCli(client)
{
    if (client && (client->GetGameTypeId() != kGameTypeId_VarSync))
        gameClient = nil; // wrong type, just clear it out
}

bool pyVarSyncGame::IsVarSyncGame(plString& guid)
{
    plUUID gameUuid(guid);
    return gameUuid == kGameTypeId_VarSync;
}

void pyVarSyncGame::JoinCommonVarSyncGame(pyKey& callbackKey)
{
    // NOTE: We don't let the player specify the game ID, because there should only be one of these in an age, ever
    VarSync_CreateParam init;
    pfGameMgr::GetInstance()->JoinCommonGame(callbackKey.getKey(), kGameTypeId_VarSync, kGameId, sizeof(init), &init);
}

void pyVarSyncGame::SetStringVar(unsigned long id, std::wstring val)
{
    if (gameClient)
    {
        pfGmVarSync* vsync = pfGmVarSync::ConvertNoRef(gameClient);
        vsync->SetStringVar(id, val.c_str());
    }
}

void pyVarSyncGame::SetNumericVar(unsigned long id, double val)
{
    if (gameClient)
    {
        pfGmVarSync* vsync = pfGmVarSync::ConvertNoRef(gameClient);
        vsync->SetNumericVar(id, val);
    }
}

void pyVarSyncGame::RequestAllVars()
{
    if (gameClient)
    {
        pfGmVarSync* vsync = pfGmVarSync::ConvertNoRef(gameClient);
        vsync->RequestAllVars();
    }
}

void pyVarSyncGame::CreateStringVar(std::wstring name, std::wstring val)
{
    if (gameClient)
    {
        pfGmVarSync* vsync = pfGmVarSync::ConvertNoRef(gameClient);
        vsync->CreateStringVar(name.c_str(), val.c_str());
    }
}

void pyVarSyncGame::CreateNumericVar(std::wstring name, double val)
{
    if (gameClient)
    {
        pfGmVarSync* vsync = pfGmVarSync::ConvertNoRef(gameClient);
        vsync->CreateNumericVar(name.c_str(), val);
    }
}
