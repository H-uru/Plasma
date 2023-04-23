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
//////////////////////////////////////////////////////////////////////
//
// pyVaultPlayerInfoNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////


#include "pyVaultPlayerInfoNode.h"

#include <string_theory/string>

#include "plVault/plVault.h"
#include "pnUUID/pnUUID.h"

//create from the Python side
pyVaultPlayerInfoNode::pyVaultPlayerInfoNode()
    : pyVaultNode()
{
    fNode->SetNodeType(plVault::kNodeType_PlayerInfo);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultPlayerInfoNode::Player_SetPlayerID( uint32_t plyrid )
{
    if (!fNode)
        return;

    VaultPlayerInfoNode playerInfo(fNode);      
    playerInfo.SetPlayerId(plyrid);
}

uint32_t pyVaultPlayerInfoNode::Player_GetPlayerID()
{
    if (!fNode)
        return 0;

    VaultPlayerInfoNode playerInfo(fNode);      
    return playerInfo.GetPlayerId();
}

void pyVaultPlayerInfoNode::Player_SetPlayerName(const ST::string& name)
{
    if (fNode) {
        VaultPlayerInfoNode playerInfo(fNode);
        playerInfo.SetPlayerName(name);
    }
}

ST::string pyVaultPlayerInfoNode::Player_GetPlayerName() const
{
    if (fNode) {
        VaultPlayerInfoNode playerInfo(fNode);
        return playerInfo.GetPlayerName();
    }
    return ST::string();
}

// age the player is currently in, if any.
void pyVaultPlayerInfoNode::Player_SetAgeInstanceName(const ST::string& name)
{
    if (fNode) {
        VaultPlayerInfoNode playerInfo(fNode);
        playerInfo.SetAgeInstName(name);
    }
}

ST::string pyVaultPlayerInfoNode::Player_GetAgeInstanceName() const
{
    if (fNode) {
        VaultPlayerInfoNode playerInfo(fNode);
        return playerInfo.GetAgeInstName();
    }
    return ST::string();
}

void pyVaultPlayerInfoNode::Player_SetAgeGuid(const ST::string& guidtext)
{
    if (!fNode)
        return;

    plUUID ageInstId(guidtext);
    VaultPlayerInfoNode playerInfo(fNode);
    playerInfo.SetAgeInstUuid(ageInstId);
}

plUUID pyVaultPlayerInfoNode::Player_GetAgeGuid() const
{
    if (fNode) {
        VaultPlayerInfoNode playerInfo(fNode);
        return playerInfo.GetAgeInstUuid();
    }
    return kNilUuid;
}

// online status
void pyVaultPlayerInfoNode::Player_SetOnline( bool b )
{
    if (!fNode)
        return;

    VaultPlayerInfoNode playerInfo(fNode);
    playerInfo.SetOnline(b);
}

bool pyVaultPlayerInfoNode::Player_IsOnline()
{
    if (!fNode)
        return false;

    VaultPlayerInfoNode playerInfo(fNode);
    return playerInfo.GetOnline();
}

int pyVaultPlayerInfoNode::Player_GetCCRLevel()
{
    if (!fNode)
        return 0;

    VaultPlayerInfoNode playerInfo(fNode);
    return playerInfo.GetCCRLevel();
}
