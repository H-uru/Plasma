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
// pyVaultTextNoteNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>

#include "pyVaultTextNoteNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultFolderNode.h"
#include "pnNetBase/pnNbError.h"
#include "plVault/plVault.h"
#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#endif

//create from the Python side
pyVaultTextNoteNode::pyVaultTextNoteNode()
    : pyVaultNode()
{
    fNode->SetNodeType(plVault::kNodeType_TextNote);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultTextNoteNode::Note_SetTitle(const ST::string& text)
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteTitle(text);
}

ST::string pyVaultTextNoteNode::Note_GetTitle() const
{
    if (fNode) {
        VaultTextNoteNode note(fNode);
        return note.GetNoteTitle();
    }
    return ST::string();
}

void pyVaultTextNoteNode::Note_SetText(const ST::string& text)
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteText(text);
}

ST::string pyVaultTextNoteNode::Note_GetText() const
{
    if (fNode) {
        VaultTextNoteNode note(fNode);
        return note.GetNoteText();
    }
    return ST::string();
}

void pyVaultTextNoteNode::Note_SetType( int32_t type )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteType(type);
}

int32_t pyVaultTextNoteNode::Note_GetType()
{
    if (!fNode)
        return 0;

    VaultTextNoteNode textNote(fNode);
    return textNote.GetNoteType();
}

void pyVaultTextNoteNode::Note_SetSubType( int32_t type )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteSubType(type);
}

int32_t pyVaultTextNoteNode::Note_GetSubType()
{
    if (!fNode)
        return 0;

    VaultTextNoteNode textNote(fNode);
    return textNote.GetNoteSubType();
}

PyObject * pyVaultTextNoteNode::GetDeviceInbox() const
{
    if (!fNode)
        PYTHON_RETURN_NONE;

    hsAssert(false, "eric, port me");
        PYTHON_RETURN_NONE;
}

void pyVaultTextNoteNode::SetDeviceInbox(const ST::string& devName, PyObject * cbObject, uint32_t cbContext)
{
    if (!fNode)
        return;

    pyVaultNode::pyVaultNodeOperationCallback * cb = new pyVaultNode::pyVaultNodeOperationCallback( cbObject );
    cb->VaultOperationStarted( cbContext );

    if (hsRef<RelVaultNode> rvn = VaultAgeSetDeviceInboxAndWait(devName, DEFAULT_DEVICE_INBOX))
        cb->SetNode(rvn);

    // cb deletes itself here.
    cb->VaultOperationComplete(cbContext, cb->GetNode() ? kNetSuccess : kNetErrInternalError);
}
