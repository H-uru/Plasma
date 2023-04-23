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
// pyAgeVault   - a wrapper class to provide interface to the plVaultAgeNode
//
//////////////////////////////////////////////////////////////////////

#include <Python.h>
#include <string_theory/string>

#include "pnNetBase/pnNbError.h"

#include "pyAgeVault.h"
#include "pyVaultFolderNode.h"
#include "pyVaultAgeInfoNode.h"
#include "pyVaultAgeLinkNode.h"
#include "pyVaultChronicleNode.h"
#include "pyVaultTextNoteNode.h"
#include "pyAgeInfoStruct.h"
#include "pySDL.h"


#include "plVault/plVault.h"
#include "plSDL/plSDL.h"

pyAgeVault::pyAgeVault() {
}

//////////////////////////////////////////////////

PyObject* pyAgeVault::GetAgeInfo()
{
    hsRef<RelVaultNode> rvn = VaultGetAgeInfoNode();
    if (rvn)
        return pyVaultAgeInfoNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetAgeDevicesFolder()
{
    hsRef<RelVaultNode> rvn = VaultGetAgeDevicesFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetSubAgesFolder()
{
    hsRef<RelVaultNode> rvn = VaultGetAgeSubAgesFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetChronicleFolder()
{
    hsRef<RelVaultNode> rvn = VaultGetAgeChronicleFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetBookshelfFolder ()
{
    hsRef<RelVaultNode> rvn = VaultAgeGetBookshelfFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetPeopleIKnowAboutFolder()
{
    hsRef<RelVaultNode> rvn = VaultGetAgePeopleIKnowAboutFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}


PyObject* pyAgeVault::GetPublicAgesFolder()
{
    hsRef<RelVaultNode> rvn = VaultGetAgePublicAgesFolder();
    if (rvn)
        return pyVaultFolderNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

PyObject* pyAgeVault::GetSubAgeLink( const pyAgeInfoStruct & info )
{
    hsRef<RelVaultNode> rvn = VaultFindAgeSubAgeLink(info.GetAgeInfo());
    if (rvn)
        return pyVaultAgeLinkNode::New(rvn);

    // just return a None object
    PYTHON_RETURN_NONE;
}

plUUID pyAgeVault::GetAgeGuid()
{
    hsRef<RelVaultNode> rvn = VaultGetAgeInfoNode();
    if (rvn) {
        VaultAgeInfoNode ageInfo(rvn);
        return ageInfo.GetAgeInstanceGuid();
    }
    return kNilUuid;
}


///////////////
// Chronicle
PyObject* pyAgeVault::FindChronicleEntry( const ST::string& entryName )
{
    if (hsRef<RelVaultNode> rvn = VaultFindAgeChronicleEntry(entryName))
        return pyVaultChronicleNode::New(rvn);
    
    // just return a None object
    PYTHON_RETURN_NONE;
}

void pyAgeVault::AddChronicleEntry( const ST::string& name, uint32_t type, const ST::string& value )
{
    VaultAddAgeChronicleEntry(name, type, value);
}

// AGE DEVICES. AKA IMAGERS, WHATEVER.
// Add a new device.
void pyAgeVault::AddDevice(const ST::string& deviceName, PyObject * cbObject, uint32_t cbContext)
{
    pyVaultNode::pyVaultNodeOperationCallback * cb = new pyVaultNode::pyVaultNodeOperationCallback( cbObject );
    cb->VaultOperationStarted( cbContext );

    if (hsRef<RelVaultNode> rvn = VaultAgeAddDeviceAndWait(deviceName))
        cb->SetNode(rvn);

    // cb deletes itself here.
    cb->VaultOperationComplete(cbContext, cb->GetNode() ? kNetSuccess : kNetErrInternalError);
}

// Remove a device.
void pyAgeVault::RemoveDevice(const ST::string& deviceName)
{
    VaultAgeRemoveDevice(deviceName);
}

// True if device exists in age.
bool pyAgeVault::HasDevice(const ST::string& deviceName)
{
    return VaultAgeHasDevice(deviceName);
}

PyObject * pyAgeVault::GetDevice(const ST::string& deviceName)
{
    if (hsRef<RelVaultNode> rvn = VaultAgeGetDevice(deviceName))
        return pyVaultTextNoteNode::New(rvn);

    PYTHON_RETURN_NONE;
}

// Sets the inbox associated with a device.
void pyAgeVault::SetDeviceInbox(const ST::string& deviceName, const ST::string& inboxName, PyObject * cbObject, uint32_t cbContext)
{
    pyVaultNode::pyVaultNodeOperationCallback * cb = new pyVaultNode::pyVaultNodeOperationCallback( cbObject );
    cb->VaultOperationStarted( cbContext );

    if (hsRef<RelVaultNode> rvn = VaultAgeSetDeviceInboxAndWait(deviceName, inboxName))
        cb->SetNode(rvn);

    // cb deletes itself here.
    cb->VaultOperationComplete(cbContext, cb->GetNode() ? kNetSuccess : kNetErrInternalError);
}

PyObject * pyAgeVault::GetDeviceInbox(const ST::string& deviceName)
{
    if (hsRef<RelVaultNode> rvn = VaultAgeGetDeviceInbox(deviceName))
        return pyVaultTextNoteNode::New(rvn);

    PYTHON_RETURN_NONE;
}

PyObject * pyAgeVault::GetAgeSDL() const
{
    plStateDataRecord * rec = new plStateDataRecord;
    if (!VaultAgeGetAgeSDL(rec)) {
        delete rec;
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
    if (hsRef<RelVaultNode> rvn = VaultGetAgeNode()) {
        hsWeakRef<NetVaultNode> node(templateNode->fNode);
        hsRef<RelVaultNode> find = rvn->GetChildNode(node, 1);
        if (find)
            return pyVaultNode::New(find);
    }

    PYTHON_RETURN_NONE;
}

