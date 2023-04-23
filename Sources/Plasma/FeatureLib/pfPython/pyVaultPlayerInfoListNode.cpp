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
// pyVaultPlayerInfoListNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>

#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultFolderNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "plVault/plVault.h"

//create from the Python side
pyVaultPlayerInfoListNode::pyVaultPlayerInfoListNode()
    : pyVaultFolderNode()
{
    fNode->SetNodeType(plVault::kNodeType_PlayerInfoList);
}

//==================================================================
// class RelVaultNode : public plVaultFolderNode
//
bool pyVaultPlayerInfoListNode::HasPlayer( uint32_t playerID )
{
    if (!fNode)
        return false;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode access(&templateNode);
    access.SetPlayerId(playerID);

    hsRef<RelVaultNode> rvn = fNode->GetChildNode(&templateNode, 1);
    return rvn != nullptr;
}

//==================================================================

static void IAddPlayer_NodesFound(ENetError result, void* param, unsigned nodeIdCount, const unsigned nodeIds[])
{
    hsWeakRef<NetVaultNode> parent = static_cast<NetVaultNode*>(param);
    if (nodeIdCount)
        VaultAddChildNode(parent->GetNodeId(), nodeIds[0], VaultGetPlayerId(), nullptr, nullptr);
}

void pyVaultPlayerInfoListNode::AddPlayer( uint32_t playerID )
{
    if (HasPlayer(playerID) || !fNode)
        return;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode access(&templateNode);
    access.SetPlayerId(playerID);

    std::vector<uint32_t> nodeIds;
    VaultLocalFindNodes(&templateNode, &nodeIds);

    // So, if we know about this node, we can take it easy. If not, we lazy load it.
    if (!nodeIds.empty())
        VaultAddChildNode(fNode->GetNodeId(), nodeIds[0], VaultGetPlayerId(), nullptr, nullptr);
    else
        VaultFindNodes(&templateNode, IAddPlayer_NodesFound, fNode.Get());
}

void pyVaultPlayerInfoListNode::RemovePlayer( uint32_t playerID )
{
    if (!fNode)
        return;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode access(&templateNode);
    access.SetPlayerId(playerID);

    if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(&templateNode, 1))
        VaultRemoveChildNode(fNode->GetNodeId(), rvn->GetNodeId(), nullptr, nullptr);
}

PyObject * pyVaultPlayerInfoListNode::GetPlayer( uint32_t playerID )
{
    if (!fNode)
        PYTHON_RETURN_NONE;

    NetVaultNode templateNode;
    templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
    VaultPlayerInfoNode access(&templateNode);
    access.SetPlayerId(playerID);

    if (hsRef<RelVaultNode> rvn = fNode->GetChildNode(&templateNode, 1))
        return pyVaultPlayerInfoNode::New(rvn);
    PYTHON_RETURN_NONE;
}


void pyVaultPlayerInfoListNode::Sort()
{
    hsAssert(false, "eric, port me");
}

