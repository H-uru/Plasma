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
// pyVaultNodeRef   - a wrapper class to provide interface to the plVaultNodeRef
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultNodeRef.h"

#include "plVault/plVault.h"

#include "pyGlueHelpers.h"
#include "pyVaultNode.h"
#include "pyVaultPlayerInfoNode.h"

//////////////////////////////////////////////////////////////////////


// should only be created from C++ side
pyVaultNodeRef::pyVaultNodeRef(hsRef<RelVaultNode> parent, hsRef<RelVaultNode> child)
    : fParent(std::move(parent)), fChild(std::move(child))
{ }

pyVaultNodeRef::pyVaultNodeRef(std::nullptr_t)
{ }

hsRef<RelVaultNode> pyVaultNodeRef::GetParentNode() const
{
    return fParent;
}

hsRef<RelVaultNode> pyVaultNodeRef::GetChildNode() const
{
    return fChild;
}


///////////////////////////////////////////////////////////////////////////

PyObject* pyVaultNodeRef::GetParent ()
{
    return pyVaultNode::New(fParent);
}

PyObject* pyVaultNodeRef::GetChild()
{
    return pyVaultNode::New(fChild);
}

unsigned pyVaultNodeRef::GetParentID () {
    if (!fParent)
        return 0;
    return fParent->GetNodeId();
}

unsigned pyVaultNodeRef::GetChildID () {
    if (!fChild)
        return 0;
    return fChild->GetNodeId();
}

unsigned pyVaultNodeRef::GetSaverID () {
    if (!fParent || !fChild)
        return 0;

    unsigned saverId = 0;
    if (hsRef<RelVaultNode> child = VaultGetNode(fChild->GetNodeId()))
        saverId = child->GetRefOwnerId(fParent->GetNodeId());
    return saverId;
}

PyObject * pyVaultNodeRef::GetSaver () {
    if (!fParent || !fChild)
        return nullptr;

    hsRef<RelVaultNode> saver;
    if (hsRef<RelVaultNode> child = VaultGetNode(fChild->GetNodeId())) {
        if (unsigned saverId = child->GetRefOwnerId(fParent->GetNodeId())) {
            // Find the player info node representing the saver
            NetVaultNode templateNode;
            templateNode.SetNodeType(plVault::kNodeType_PlayerInfo);
            VaultPlayerInfoNode access(&templateNode);
            access.SetPlayerId(saverId);
            saver = VaultGetNode(&templateNode);

            if (!saver) {
                std::vector<unsigned> nodeIds;
                VaultFindNodesAndWait(&templateNode, &nodeIds);
                if (!nodeIds.empty()) {
                    VaultFetchNodesAndWait(nodeIds.data(), nodeIds.size());
                    saver = VaultGetNode(nodeIds[0]);
                }
            }
        }
    }
    if (!saver)
        PYTHON_RETURN_NONE;
        
    return pyVaultPlayerInfoNode::New(saver);
}

bool pyVaultNodeRef::BeenSeen () {
    return false;
}

void pyVaultNodeRef::SetSeen (bool v) {
    if (!fParent || !fChild)
        return;

    fParent->SetSeen(fChild->GetNodeId(), v);
}
