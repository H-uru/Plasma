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
#ifndef pyVaultPlayerNode_h_
#define pyVaultPlayerNode_h_

//////////////////////////////////////////////////////////////////////
//
// pyVaultPlayerNode   - a wrapper class to provide interface to the RelVaultNode
//
//////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

#include "../pfPython/pyVaultNode.h"
#include "../pfPython/pyVaultPlayerInfoNode.h"

class pyAgeInfoStruct;
struct RelVaultNode;

class pyVaultPlayerNode : public pyVaultNode
{
protected:
	// should only be created from C++ side
	pyVaultPlayerNode(RelVaultNode *nfsNode);

	//create from the Python side
	pyVaultPlayerNode();

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVaultPlayerNode);
	PYTHON_CLASS_NEW_DEFINITION;
	static PyObject *New(RelVaultNode *nfsNode);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVaultPlayerNode object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVaultPlayerNode); // converts a PyObject to a pyVaultPlayerNode (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

//==================================================================
// class plVaultPlayerInfoNode : public plVaultNode
//
	PyObject *GetInbox(); // returns pyVaultFolderNode
	PyObject *GetPlayerInfo(); // returns pyVaultPlayerInfoNode
	PyObject *GetAvatarOutfitFolder(); // returns pyVaultFolderNode
	PyObject *GetAvatarClosetFolder(); // returns pyVaultFolderNode
	PyObject *GetChronicleFolder(); // returns pyVaultFolderNode
	PyObject *GetAgeJournalsFolder(); // returns pyVaultFolderNode
	PyObject *GetIgnoreListFolder(); // returns pyVaultPlayerInfoListNode
	PyObject *GetBuddyListFolder(); // returns pyVaultPlayerInfoListNode
	PyObject *GetPeopleIKnowAboutFolder(); // returns pyVaultPlayerInfoListNode
	PyObject *GetAgesICanVisitFolder(); // returns pyVaultFolderNode
	PyObject *GetAgesIOwnFolder(); // returns pyVaultFolderNode

	PyObject *GetLinkToMyNeighborhood(); // returns pyVaultAgeLinkNode
	PyObject *GetLinkToCity(); // returns pyVaultAgeLinkNode

	PyObject *GetOwnedAgeLink(const pyAgeInfoStruct *info); // returns pyVaultAgeLinkNode
	void RemoveOwnedAgeLink(const char* guid);

	PyObject *GetVisitAgeLink(const pyAgeInfoStruct *info); // returns pyVaultAgeLinkNode
	void RemoveVisitAgeLink(const char* guid);

	PyObject *FindChronicleEntry(const char *entryName); // returns pyVaultChronicleNode

	void SetPlayerName(const char *value);
	std::string GetPlayerName();

	void SetAvatarShapeName(const char *value);
	std::string GetAvatarShapeName();

	void SetDisabled(bool value);
	bool IsDisabled();

	void SetOnlineTime(UInt32 value);
	UInt32 GetOnlineTime();

	void	SetExplorer (bool b);
	hsBool	IsExplorer ();
};

#endif	// pyVaultPlayerNode_h_