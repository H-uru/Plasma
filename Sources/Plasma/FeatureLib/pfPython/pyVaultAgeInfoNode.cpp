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
// pyVaultAgeInfoNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsStlUtils.h"

#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeInfoListNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultSDLNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"

#include "../plVault/plVault.h"

// should only be created from C++ side
pyVaultAgeInfoNode::pyVaultAgeInfoNode(RelVaultNode* nfsNode)
: pyVaultNode(nfsNode)
{
}

//create from the Python side
pyVaultAgeInfoNode::pyVaultAgeInfoNode(int n)
: pyVaultNode(NEWZERO(RelVaultNode))
{
	fNode->SetNodeType(plVault::kNodeType_AgeInfo);
}

//============================================================================
/*
static PyObject * GetChildFolder (RelVaultNode * node, unsigned type) {
	PyObject * result = nil;
	if (RelVaultNode * rvn = node->GetChildFolderNodeIncRef(type, 1)) {
		result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
	}
	return result;
}
*/

//============================================================================
static PyObject * GetChildPlayerInfoList (RelVaultNode * node, unsigned type) {
	PyObject * result = nil;
	if (RelVaultNode * rvn = node->GetChildPlayerInfoListNodeIncRef(type, 1)) {
		result = pyVaultPlayerInfoListNode::New(rvn);
		rvn->DecRef();
	}
	return result;
}

//============================================================================
static PyObject * GetChildAgeInfoList (RelVaultNode * node, unsigned type) {
	PyObject * result = nil;
	if (RelVaultNode * rvn = node->GetChildAgeInfoListNodeIncRef(type, 1)) {
		result = pyVaultAgeInfoListNode::New(rvn);
		rvn->DecRef();
	}
	return result;
}

//==================================================================
// class RelVaultNode : public plVaultNode
//

PyObject * pyVaultAgeInfoNode::GetAgeOwnersFolder() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	if (PyObject * result = GetChildPlayerInfoList(fNode, plVault::kAgeOwnersFolder))
		return result;

	// just return a None object.
	PYTHON_RETURN_NONE;
}

PyObject * pyVaultAgeInfoNode::GetCanVisitFolder() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	if (PyObject * result = GetChildPlayerInfoList(fNode, plVault::kCanVisitFolder))
		return result;

	// just return a None object.
	PYTHON_RETURN_NONE;
}

PyObject* pyVaultAgeInfoNode::GetChildAgesFolder( void )
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	if (PyObject * result = GetChildAgeInfoList(fNode, plVault::kChildAgesFolder))
		return result;

	// just return a None object.
	PYTHON_RETURN_NONE;
}


PyObject * pyVaultAgeInfoNode::GetAgeSDL() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	hsAssert(false, "eric, port me");
	// just return a None object.
	PYTHON_RETURN_NONE
}

PyObject * pyVaultAgeInfoNode::GetCzar() const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	hsAssert(false, "eric, port me");

	// just return a None object.
	PYTHON_RETURN_NONE
}

PyObject * pyVaultAgeInfoNode::GetParentAgeLink () const
{
	if (!fNode)
		PYTHON_RETURN_NONE;

	if (RelVaultNode * rvn = fNode->GetParentAgeLinkIncRef()) {
		PyObject * result = pyVaultAgeLinkNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object.
	PYTHON_RETURN_NONE
}


const char * pyVaultAgeInfoNode::GetAgeFilename() const
{
	if (fNode) {
		char str[MAX_PATH];
		VaultAgeInfoNode access(fNode);
		StrToAnsi(str, access.ageFilename, arrsize(str));
		fAgeFilename = str;
	}
	return fAgeFilename.c_str();
}

void pyVaultAgeInfoNode::SetAgeFilename( const char * v )
{
}

const char * pyVaultAgeInfoNode::GetAgeInstanceName() const
{
	if (fNode) {
		char str[MAX_PATH];
		VaultAgeInfoNode access(fNode);
		StrToAnsi(str, access.ageInstName, arrsize(str));
		fAgeInstName = str;
	}
	return fAgeInstName.c_str();
}

void pyVaultAgeInfoNode::SetAgeInstanceName( const char * v )
{
}

const char * pyVaultAgeInfoNode::GetAgeUserDefinedName() const
{
	if (fNode) {
		char str[MAX_PATH];
		VaultAgeInfoNode access(fNode);
		StrToAnsi(str, access.ageUserDefinedName, arrsize(str));
		fAgeUserName = str;
	}
	return fAgeUserName.c_str();
}

void pyVaultAgeInfoNode::SetAgeUserDefinedName( const char * v )
{
}

const char * pyVaultAgeInfoNode::GetAgeInstanceGuid() const
{
	fAgeInstGuid[0] = 0;
	
	if (fNode) {
		VaultAgeInfoNode access(fNode);
		GuidToString(access.ageInstUuid, fAgeInstGuid, arrsize(fAgeInstGuid));
	}
	return fAgeInstGuid;
}

void pyVaultAgeInfoNode::SetAgeInstanceGuid( const char * sguid )
{
}

const char * pyVaultAgeInfoNode::GetAgeDescription() const
{
	if (fNode) {
		char str[MAX_PATH];
		memset(str, 0, sizeof(str));
		VaultAgeInfoNode access(fNode);
		StrToAnsi(str, access.ageDescription, arrsize(str));
		fAgeDescription = str;
	}
	return fAgeDescription.c_str();
}

void pyVaultAgeInfoNode::SetAgeDescription( const char * v )
{
}

Int32 pyVaultAgeInfoNode::GetSequenceNumber() const
{
	if (!fNode)
		return -1;
		
	VaultAgeInfoNode access(fNode);
	return access.ageSequenceNumber;
}

void pyVaultAgeInfoNode::SetSequenceNumber( Int32 v )
{
}

Int32 pyVaultAgeInfoNode::GetAgeLanguage() const
{
	if (!fNode)
		return -1;
		
	VaultAgeInfoNode access(fNode);
	return access.ageLanguage;
}

void pyVaultAgeInfoNode::SetAgeLanguage( Int32 v )
{
}

UInt32 pyVaultAgeInfoNode::GetAgeID() const
{
	return 0;
}

void pyVaultAgeInfoNode::SetAgeID( UInt32 v )
{
}

UInt32 pyVaultAgeInfoNode::GetCzarID() const
{
	hsAssert(false, "eric, port me");
	return 0;
}


bool pyVaultAgeInfoNode::IsPublic() const
{
	if (fNode) {
		VaultAgeInfoNode access(fNode);
		return access.ageIsPublic;
	}
	return false;
}

const char * pyVaultAgeInfoNode::GetDisplayName() const
{
	if (fNode) {
		char str[MAX_PATH];
		VaultAgeInfoNode access(fNode);
		if (access.ageSequenceNumber > 0)
			StrPrintf(str, arrsize(str), "%S(%d) %S", access.ageUserDefinedName, access.ageSequenceNumber, access.ageInstName);
		else
			StrPrintf(str, arrsize(str), "%S %S", access.ageUserDefinedName, access.ageInstName);
		fAgeDispName = str;
	}
	return fAgeDispName.c_str();
}

PyObject * pyVaultAgeInfoNode::AsAgeInfoStruct() const
{
	plAgeInfoStruct info;
	VaultAgeInfoNode access(fNode);
	access.CopyTo(&info);
	return pyAgeInfoStruct::New(&info);
}
