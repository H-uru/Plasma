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

#pragma hdrstop

#include "pyVaultPlayerInfoNode.h"
#include "plVault/plVault.h"
#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#endif

// should only be created from C++ side
pyVaultPlayerInfoNode::pyVaultPlayerInfoNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
, ansiPlayerName(nil)
, ansiAgeInstName(nil)
{
}

//create from the Python side
pyVaultPlayerInfoNode::pyVaultPlayerInfoNode()
: pyVaultNode(NEWZERO(RelVaultNode))
, ansiPlayerName(nil)
, ansiAgeInstName(nil)
{
    fNode->SetNodeType(plVault::kNodeType_PlayerInfo);
}

pyVaultPlayerInfoNode::~pyVaultPlayerInfoNode () {
    free(ansiPlayerName);
    free(ansiAgeInstName);
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

uint32_t pyVaultPlayerInfoNode::Player_GetPlayerID( void )
{
    if (!fNode)
        return 0;

    VaultPlayerInfoNode playerInfo(fNode);      
    return playerInfo.playerId;
}

void pyVaultPlayerInfoNode::Player_SetPlayerName( const char * name )
{
    if (!fNode)
        return;

    wchar_t * wStr = StrDupToUnicode(name);
    VaultPlayerInfoNode playerInfo(fNode);      
    playerInfo.SetPlayerName(wStr);
    free(wStr);
}

const char * pyVaultPlayerInfoNode::Player_GetPlayerName( void )
{
    if (!fNode)
        return "";
        
    VaultPlayerInfoNode playerInfo(fNode);      
    if (!playerInfo.playerName)
        return "";

    free(ansiPlayerName);
    ansiPlayerName = StrDupToAnsi(playerInfo.playerName);
    return ansiPlayerName;
}

// age the player is currently in, if any.
void pyVaultPlayerInfoNode::Player_SetAgeInstanceName( const char * agename )
{
    if (!fNode)
        return;

    wchar_t * wStr = StrDupToUnicode(agename);
    VaultPlayerInfoNode playerInfo(fNode);      
    playerInfo.SetAgeInstName(wStr);
    free(wStr);
}

const char * pyVaultPlayerInfoNode::Player_GetAgeInstanceName( void )
{
    if (!fNode)
        return "";
        
    VaultPlayerInfoNode playerInfo(fNode);
    if (!playerInfo.ageInstName)
        return "";
        
    free(ansiAgeInstName);
    ansiAgeInstName = StrDupToAnsi(playerInfo.ageInstName);
    return ansiAgeInstName;
}

void pyVaultPlayerInfoNode::Player_SetAgeGuid( const char * guidtext)
{
    if (!fNode)
        return;

    Uuid ageInstId;
    GuidFromString(guidtext, &ageInstId);
    VaultPlayerInfoNode playerInfo(fNode);
    playerInfo.SetAgeInstUuid(ageInstId);       
}

const char * pyVaultPlayerInfoNode::Player_GetAgeGuid( void )
{
    if (!fNode)
        return "";

    VaultPlayerInfoNode playerInfo(fNode);
    GuidToString(playerInfo.ageInstUuid, ansiAgeInstUuid, arrsize(ansiAgeInstUuid));
    return ansiAgeInstUuid;
}

// online status
void pyVaultPlayerInfoNode::Player_SetOnline( bool b )
{
    if (!fNode)
        return;

    VaultPlayerInfoNode playerInfo(fNode);
    playerInfo.SetOnline(b);
}

bool pyVaultPlayerInfoNode::Player_IsOnline( void )
{
    if (!fNode)
        return false;

    VaultPlayerInfoNode playerInfo(fNode);
    return playerInfo.online;
}

int pyVaultPlayerInfoNode::Player_GetCCRLevel( void )
{
    if (!fNode)
        return 0;

    VaultPlayerInfoNode playerInfo(fNode);
    return playerInfo.ccrLevel;
}
