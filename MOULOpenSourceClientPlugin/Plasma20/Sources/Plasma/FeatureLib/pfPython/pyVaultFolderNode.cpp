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
// pyVaultFolderNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultFolderNode.h"

#include "hsUtils.h"
#include "../plVault/plVault.h"

// should only be created from C++ side
pyVaultFolderNode::pyVaultFolderNode( RelVaultNode* nfsNode )
: pyVaultNode( nfsNode )
{
}

//create from the Python side
pyVaultFolderNode::pyVaultFolderNode(int n)
: pyVaultNode(NEWZERO(RelVaultNode))
{
	fNode->SetNodeType(plVault::kNodeType_Folder);
}

pyVaultFolderNode::~pyVaultFolderNode () {
}

//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultFolderNode::Folder_SetType( int type )
{
	if (!fNode)
		return;
		
	VaultFolderNode folder(fNode);
	folder.SetFolderType(type);
}

int pyVaultFolderNode::Folder_GetType( void )
{
	if (!fNode)
		return 0;
		
	VaultFolderNode folder(fNode);
	return folder.folderType;
}

void pyVaultFolderNode::Folder_SetName( std::string name )
{
	if (!fNode)
		return;
		
	wchar_t* wName = hsStringToWString(name.c_str());
	VaultFolderNode folder(fNode);
	folder.SetFolderName(wName);
	delete [] wName;
}

void pyVaultFolderNode::Folder_SetNameW( std::wstring name )
{
	if (!fNode)
		return;

	VaultFolderNode folder(fNode);
	folder.SetFolderName(name.c_str());
}

std::string pyVaultFolderNode::Folder_GetName( void )
{
	if (!fNode)
		return "";
		
	VaultFolderNode folder(fNode);
	if (!folder.folderName)
		return "";

	std::string retVal;
	char* sName = hsWStringToString(folder.folderName);
	retVal = sName;
	delete [] sName;
	return retVal;
}

std::wstring pyVaultFolderNode::Folder_GetNameW( void )
{
	if (!fNode)
		return L"";

	VaultFolderNode folder(fNode);
	if (!folder.folderName)
		return L"";

	return folder.folderName;
}
