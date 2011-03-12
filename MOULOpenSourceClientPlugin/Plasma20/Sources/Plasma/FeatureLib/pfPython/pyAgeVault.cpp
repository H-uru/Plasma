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
// pyAgeVault   - a wrapper class to provide interface to the plVaultAgeNode
//
//////////////////////////////////////////////////////////////////////

#include "pyAgeVault.h"
#include "pyVault.h"
#include "pyVaultNodeRef.h"
#include "pyVaultFolderNode.h"
#include "pyVaultPlayerInfoListNode.h"
#include "pyVaultPlayerInfoNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultTextNoteNode.h"
#include "pyNetLinkingMgr.h"
#include "pyAgeInfoStruct.h"
#include "pySDL.h"


#include "../plVault/plVault.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plNetTransport/plNetTransport.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plSDL/plSDL.h"
#include "../pnNetCommon/plNetApp.h"

pyAgeVault::pyAgeVault() {
}

pyAgeVault::~pyAgeVault() {
}

//////////////////////////////////////////////////

PyObject* pyAgeVault::GetAgeInfo()
{
	RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef();
	if (rvn) {
		PyObject * result = pyVaultAgeInfoNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetAgeDevicesFolder( void )
{
	RelVaultNode * rvn = VaultGetAgeDevicesFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetSubAgesFolder( void )
{
	RelVaultNode * rvn = VaultGetAgeSubAgesFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetChronicleFolder( void )
{
	RelVaultNode * rvn = VaultGetAgeChronicleFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetBookshelfFolder ( void )
{
	RelVaultNode * rvn = VaultAgeGetBookshelfFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetPeopleIKnowAboutFolder( void )
{
	RelVaultNode * rvn = VaultGetAgePeopleIKnowAboutFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}


PyObject* pyAgeVault::GetPublicAgesFolder(void)
{
	RelVaultNode * rvn = VaultGetAgePublicAgesFolderIncRef();
	if (rvn) {
		PyObject * result = pyVaultFolderNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetSubAgeLink( const pyAgeInfoStruct & info )
{
	RelVaultNode * rvn = VaultFindAgeSubAgeLinkIncRef(info.GetAgeInfo());
	if (rvn) {
		PyObject * result = pyVaultAgeLinkNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	// just return a None object
	PYTHON_RETURN_NONE;
}

const char* pyAgeVault::GetAgeGuid( void )
{
	RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef();
	if (rvn) {
		VaultAgeInfoNode ageInfo(rvn);
		GuidToString(ageInfo.ageInstUuid, fAgeGuid, arrsize(fAgeGuid));
		rvn->DecRef();
	}
	else {
		fAgeGuid[0] = 0;
	}
	return fAgeGuid;
}


///////////////
// Chronicle
PyObject* pyAgeVault::FindChronicleEntry( const char * entryName )
{
	wchar wEntryName[kMaxVaultNodeStringLength];
	StrToUnicode(wEntryName, entryName, arrsize(wEntryName));
	
	if (RelVaultNode * rvn = VaultFindAgeChronicleEntryIncRef(wEntryName)) {
		PyObject * result = pyVaultChronicleNode::New(rvn);
		rvn->DecRef();
		return result;
	}
	
	// just return a None object
	PYTHON_RETURN_NONE;
}

void pyAgeVault::AddChronicleEntry( const char * name, UInt32 type, const char * value )
{
	wchar * wEntryName = StrDupToUnicode(name);
	wchar * wEntryValue = StrDupToUnicode(value);
	
	VaultAddAgeChronicleEntry(wEntryName, type, wEntryValue);
	
	FREE(wEntryName);
	FREE(wEntryValue);
}

// AGE DEVICES. AKA IMAGERS, WHATEVER.
// Add a new device.
void pyAgeVault::AddDevice( const char * deviceName, PyObject * cbObject, UInt32 cbContext )
{
	pyVaultNode::pyVaultNodeOperationCallback * cb = NEWZERO(pyVaultNode::pyVaultNodeOperationCallback)( cbObject );
	cb->VaultOperationStarted( cbContext );

	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, deviceName, arrsize(wStr));

	if (RelVaultNode * rvn = VaultAgeAddDeviceAndWaitIncRef(wStr)) {
		cb->SetNode(rvn);
		rvn->DecRef();
	}

	cb->VaultOperationComplete( cbContext, cb->GetNode() ? hsOK : hsFail);	// cbHolder deletes itself here.
}

// Remove a device.
void pyAgeVault::RemoveDevice( const char * deviceName )
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, deviceName, arrsize(wStr));

	VaultAgeRemoveDevice(wStr);
}

// True if device exists in age.
bool pyAgeVault::HasDevice( const char * deviceName )
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, deviceName, arrsize(wStr));

	return VaultAgeHasDevice(wStr);
}

PyObject * pyAgeVault::GetDevice( const char * deviceName )
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, deviceName, arrsize(wStr));

	if (RelVaultNode * rvn = VaultAgeGetDeviceIncRef(wStr)) {
		PyObject * result = pyVaultTextNoteNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	PYTHON_RETURN_NONE;
}

// Sets the inbox associated with a device.
void pyAgeVault::SetDeviceInbox( const char * deviceName, const char * inboxName, PyObject * cbObject, UInt32 cbContext )
{
	pyVaultNode::pyVaultNodeOperationCallback * cb = NEWZERO(pyVaultNode::pyVaultNodeOperationCallback)( cbObject );
	cb->VaultOperationStarted( cbContext );

	wchar wDev[MAX_PATH];
	StrToUnicode(wDev, deviceName, arrsize(wDev));
	wchar wInb[MAX_PATH];
	StrToUnicode(wInb, inboxName, arrsize(wInb));
	
	if (RelVaultNode * rvn = VaultAgeSetDeviceInboxAndWaitIncRef(wDev, wInb)) {
		cb->SetNode(rvn);
		rvn->DecRef();
	}

	cb->VaultOperationComplete( cbContext, cb->GetNode() ? hsOK : hsFail );	// cbHolder deletes itself here.
}

PyObject * pyAgeVault::GetDeviceInbox( const char * deviceName )
{
	wchar wStr[MAX_PATH];
	StrToUnicode(wStr, deviceName, arrsize(wStr));

	if (RelVaultNode * rvn = VaultAgeGetDeviceInboxIncRef(wStr)) {
		PyObject * result = pyVaultTextNoteNode::New(rvn);
		rvn->DecRef();
		return result;
	}

	PYTHON_RETURN_NONE;
}

PyObject * pyAgeVault::GetAgeSDL() const
{
	plStateDataRecord * rec = NEWZERO(plStateDataRecord);
	if (!VaultAgeGetAgeSDL(rec)) {
		DEL(rec);
		PYTHON_RETURN_NONE;
	}
	else {
		return pySDLStateDataRecord::New( rec );
	}	
}

void pyAgeVault::UpdateAgeSDL( pySDLStateDataRecord & pyrec )
{
	plStateDataRecord * rec = pyrec.GetRec();
	if ( !rec )
		return;
		
	VaultAgeUpdateAgeSDL(rec);
}

PyObject* pyAgeVault::FindNode( pyVaultNode* templateNode ) const
{
	if (RelVaultNode * rvn = VaultGetAgeNodeIncRef()) {
		RelVaultNode * find = rvn->GetChildNodeIncRef(templateNode->fNode, 1);
		rvn->DecRef();
		if (find) {
			PyObject * result = pyVaultNode::New(find);
			find->DecRef();
			return result;
		}
	}

	PYTHON_RETURN_NONE;
}

