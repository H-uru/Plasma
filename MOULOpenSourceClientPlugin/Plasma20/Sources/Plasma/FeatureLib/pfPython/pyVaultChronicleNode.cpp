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
// pyVaultChronicleNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "pyVaultChronicleNode.h"
#ifndef BUILDING_PYPLASMA
#include "pyVault.h"
#endif

#include "../plVault/plVault.h"

// should only be created from C++ side
pyVaultChronicleNode::pyVaultChronicleNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
, ansiName(nil)
, ansiValue(nil)
{
}

//create from the Python side
pyVaultChronicleNode::pyVaultChronicleNode(int n)
: pyVaultNode(NEWZERO(RelVaultNode))
, ansiName(nil)
, ansiValue(nil)
{
	fNode->SetNodeType(plVault::kNodeType_Chronicle);
}

pyVaultChronicleNode::~pyVaultChronicleNode () {
	FREE(ansiName);
	FREE(ansiValue);
}


//==================================================================
// class RelVaultNode : public plVaultNode
//
void pyVaultChronicleNode::Chronicle_SetName( const char * text )
{
	if (!fNode)
		return;

	wchar * wStr = StrDupToUnicode(text);
	VaultChronicleNode chron(fNode);
	chron.SetEntryName(wStr);
	FREE(wStr);
}

const char * pyVaultChronicleNode::Chronicle_GetName( void )
{
	if (!fNode)
		return "";

	FREE(ansiName);
	VaultChronicleNode chron(fNode);
	ansiName = StrDupToAnsi(chron.entryName);
	
	return ansiName;
}

void pyVaultChronicleNode::Chronicle_SetValue( const char * text )
{
	if (!fNode)
		return;
		
	wchar * wStr = StrDupToUnicode(text);
	VaultChronicleNode chron(fNode);
	chron.SetEntryValue(wStr);
	FREE(wStr);
}

const char * pyVaultChronicleNode::Chronicle_GetValue( void )
{
	if (!fNode)
		return "";
		
	FREE(ansiValue);
	ansiValue = nil;
	
	VaultChronicleNode chron(fNode);
	
	if (!chron.entryValue)
		return "";
		
	ansiValue = StrDupToAnsi(chron.entryValue);
	return ansiValue;
}

void pyVaultChronicleNode::Chronicle_SetType( UInt32 type )
{
	if (!fNode)
		return;

	VaultChronicleNode chron(fNode);
	chron.SetEntryType(type);
}

UInt32 pyVaultChronicleNode::Chronicle_GetType( void )
{
	if (!fNode)
		return 0;

	VaultChronicleNode chron(fNode);
	return chron.entryType;
}
