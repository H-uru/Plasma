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
//////////////////////////////////////////////////////////////////////
//
// pyVaultSDLNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultSDLNode.h"
#include "pySDL.h"

#include "../plVault/plVault.h"

#include "../plSDL/plSDL.h"

// should only be created from C++ side
pyVaultSDLNode::pyVaultSDLNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultSDLNode::pyVaultSDLNode()
: pyVaultNode(NEWZERO(RelVaultNode))
{
	fNode->SetNodeType(plVault::kNodeType_SDL);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultSDLNode::SetIdent( int v )
{
	if (!fNode)
		return;
	
	VaultSDLNode sdl(fNode);
	sdl.SetSdlIdent(v);
}

int pyVaultSDLNode::GetIdent() const
{
	if (!fNode)
		return 0;
	
	VaultSDLNode sdl(fNode);
	return sdl.sdlIdent;
}

PyObject * pyVaultSDLNode::GetStateDataRecord() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;
		
	VaultSDLNode sdl(fNode);
	plStateDataRecord * rec = NEWZERO(plStateDataRecord);
	if (sdl.GetStateDataRecord(rec))
		return pySDLStateDataRecord::New(rec);
	else
		DEL(rec);

	PYTHON_RETURN_NONE;
}

void pyVaultSDLNode::InitStateDataRecord( const char* agename, int flags)
{
	if (!fNode)
		return;

	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, agename, arrsize(wStr));	
	VaultSDLNode sdl(fNode);
	sdl.InitStateDataRecord(wStr, flags);
}

void pyVaultSDLNode::SetStateDataRecord( const pySDLStateDataRecord & rec, int writeOptions/*=0 */)
{
	if (!fNode)
		return;
	
	VaultSDLNode sdl(fNode);
	sdl.SetStateDataRecord(rec.GetRec(), writeOptions);
}

