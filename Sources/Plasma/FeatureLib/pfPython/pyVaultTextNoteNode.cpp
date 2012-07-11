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
#pragma hdrstop

#include "pyVaultTextNoteNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultFolderNode.h"
#include "plVault/plVault.h"
#ifndef BUILDING_PYPLASMA
#   include "pyVault.h"
#endif

// should only be created from C++ side
pyVaultTextNoteNode::pyVaultTextNoteNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultTextNoteNode::pyVaultTextNoteNode()
: pyVaultNode(NEWZERO(RelVaultNode))
{
    fNode->SetNodeType(plVault::kNodeType_TextNote);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultTextNoteNode::Note_SetTitle( const char * text )
{
    if (!fNode)
        return;

    wchar_t wStr[MAX_PATH] = L"";
    if (text)
        StrToUnicode(wStr, text, arrsize(wStr));
    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteTitle(wStr);
}

void pyVaultTextNoteNode::Note_SetTitleW( const wchar_t * text )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteTitle(text);
}

std::string pyVaultTextNoteNode::Note_GetTitle( void )
{
    if (!fNode)
        return "";

    char * aStr = nil;
    VaultTextNoteNode textNote(fNode);
    if (textNote.noteTitle) {
        std::string result;
        aStr = StrDupToAnsi(textNote.noteTitle);
        result = aStr;
        free(aStr);
        return result;
    }
    return "";
}

std::wstring pyVaultTextNoteNode::Note_GetTitleW( void )
{
    if (!fNode)
        return L"";

    VaultTextNoteNode textNote(fNode);
    if (textNote.noteTitle)
        return textNote.noteTitle;
    return L"";
}

void pyVaultTextNoteNode::Note_SetText( const char * text )
{
    if (!fNode)
        return;

    wchar_t * wStr = nil;
    if (text)
        wStr = StrDupToUnicode(text);
    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteText(wStr);
    free(wStr);
}

void pyVaultTextNoteNode::Note_SetTextW( const wchar_t * text )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteText(text);
}

std::string pyVaultTextNoteNode::Note_GetText( void )
{
    if (!fNode)
        return "";

    char * aStr = nil;
    VaultTextNoteNode textNote(fNode);
    if (textNote.noteText) {
        std::string result;
        aStr = StrDupToAnsi(textNote.noteText);
        result = aStr;
        free(aStr);
        return result;
    }
    return "";
}

std::wstring pyVaultTextNoteNode::Note_GetTextW( void )
{
    if (!fNode)
        return L"";

    VaultTextNoteNode textNote(fNode);
    if (textNote.noteText)
        return textNote.noteText;
    return L"";
}

void pyVaultTextNoteNode::Note_SetType( int32_t type )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteType(type);
}

int32_t pyVaultTextNoteNode::Note_GetType( void )
{
    if (!fNode)
        return 0;
        
    VaultTextNoteNode textNote(fNode);
    return textNote.noteType;
}

void pyVaultTextNoteNode::Note_SetSubType( int32_t type )
{
    if (!fNode)
        return;

    VaultTextNoteNode textNote(fNode);
    textNote.SetNoteSubType(type);
}

int32_t pyVaultTextNoteNode::Note_GetSubType( void )
{
    if (!fNode)
        return 0;
        
    VaultTextNoteNode textNote(fNode);
    return textNote.noteSubType;
}

PyObject * pyVaultTextNoteNode::GetDeviceInbox() const
{
    if (!fNode)
        PYTHON_RETURN_NONE;

    hsAssert(false, "eric, port me");
        PYTHON_RETURN_NONE;
}

void pyVaultTextNoteNode::SetDeviceInbox( const char * devName, PyObject * cbObject, uint32_t cbContext )
{
    if (!fNode)
        return;

    pyVaultNode::pyVaultNodeOperationCallback * cb = NEWZERO(pyVaultNode::pyVaultNodeOperationCallback)( cbObject );
    cb->VaultOperationStarted( cbContext );

    wchar_t wDev[MAX_PATH];
    StrToUnicode(wDev, devName, arrsize(wDev));
    
    if (RelVaultNode * rvn = VaultAgeSetDeviceInboxAndWaitIncRef(wDev, DEFAULT_DEVICE_INBOX)) {
        cb->SetNode(rvn);
        rvn->DecRef();
    }

    cb->VaultOperationComplete( cbContext, cb->GetNode() ? hsOK : hsFail ); // cbHolder deletes itself here.
}
