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
#include "hsConfig.h"
#include "../plSDL/plSDL.h"
#include "../plVault/plVaultCache.h"
#include "../pyNetClientComm/pyNetClientComm.h"
#include "../pyVault/pyVNodeMgr.h"
#include "../pfPython/pyNetServerSessionInfo.h"
#include "../pfPython/pyVault.h"
#include "../pfPython/pyVaultNodeRef.h"
#include "../pfPython/pyVaultAgeInfoListNode.h"
#include "../pfPython/pyVaultAgeInfoNode.h"
#include "../pfPython/pyVaultAgeLinkNode.h"
#include "../pfPython/pyVaultChronicleNode.h"
#include "../pfPython/pyVaultFolderNode.h"
#include "../pfPython/pyVaultImageNode.h"
#include "../pfPython/pyVaultMarkerNode.h"
#include "../pfPython/pyVaultPlayerInfoListNode.h"
#include "../pfPython/pyVaultPlayerInfoNode.h"
#include "../pfPython/pyVaultPlayerNode.h"
#include "../pfPython/pySDL.h"
#include "../pfPython/pyVaultSDLNode.h"
#include "../pfPython/pyVaultTextNoteNode.h"
#include "../pfPython/pySpawnPointInfo.h"
#include "../pfPython/pyAgeInfoStruct.h"
#include "../pfPython/pyAgeLinkStruct.h"
#include "../pfPython/pyDniCoordinates.h"
#include "../pfPython/pyImage.h"
#include "../pfPython/pyNetLinkingMgr.h"
#include "../pfPython/pyStatusLog.h"
#include "../pfPython/pyColor.h"

#include "../pfPython/pyEnum.h"
#include "../pfPython/pyGlueHelpers.h"

#include <python.h>

extern "C" __declspec(dllexport) void PyInit_pyPlasma(void)
{
	// If a glue function (AddPlasmaClasses, AddPlasmaMethds, etc) is commented out, it is included
	// in the source in this project... but the original version of this function did not call the
	// function. So in order to keep the module identical, those specified classes/functions are not
	// added, but can be un-commented in the future if needed

	std::vector<PyMethodDef> methods; // this is temporary, for easy addition of new methods
	//pyImage::AddPlasmaMethods(methods);
	pySpawnPointInfo::AddPlasmaMethods(methods);

	// now copy the data to our real method definition structure
	PyMethodDef* plasmaMethods = new PyMethodDef[methods.size() + 1];
	for (int curMethod = 0; curMethod < methods.size(); curMethod++)
		plasmaMethods[curMethod] = methods[curMethod];
	PyMethodDef terminator = {NULL};
	plasmaMethods[methods.size()] = terminator; // add the terminator

	// Init the module
	PyObject *m = Py_InitModule("pyPlasma", plasmaMethods);

	// Inits
	plSDLMgr::GetInstance()->Init();
	plVaultCache::GetInstance()->SetEnabled( true );

	// Enum
	pyEnum::AddPlasmaConstantsClasses(m);

	// Classes
	pyAgeInfoStruct::AddPlasmaClasses(m);
	pyAgeInfoStructRef::AddPlasmaClasses(m);
	pyAgeLinkStruct::AddPlasmaClasses(m);
	pyAgeLinkStructRef::AddPlasmaClasses(m);
	pyColor::AddPlasmaClasses(m);
	//pyDniCoordinates::AddPlasmaClasses(m);
	//pyPoint3::AddPlasmaClasses(m);
	//pyVector3::AddPlasmaClasses(m);
	//pyImage::AddPlasmaClasses(m);
	//pyMatrix44::AddPlasmaClasses(m);
	pyNetClientComm::AddPlasmaClasses(m);
	pyNetServerSessionInfo::AddPlasmaClasses(m);
	pyNetServerSessionInfoRef::AddPlasmaClasses(m);
	pySDLStateDataRecord::AddPlasmaClasses(m);
	pySimpleStateVariable::AddPlasmaClasses(m);
	pySpawnPointInfo::AddPlasmaClasses(m);
	pySpawnPointInfoRef::AddPlasmaClasses(m);
	pyStatusLog::AddPlasmaClasses(m);

	pyVNodeMgr::AddPlasmaClasses(m);
	pyAdminVNodeMgr::AddPlasmaClasses(m);
	pyAgeVNodeMgr::AddPlasmaClasses(m);
	pyPlayerVNodeMgr::AddPlasmaClasses(m);

	pyVaultNode::AddPlasmaClasses(m);
	pyVaultNodeRef::AddPlasmaClasses(m);
	pyVaultFolderNode::AddPlasmaClasses(m);

	pyVaultAgeInfoListNode::AddPlasmaClasses(m);
	pyVaultAgeInfoNode::AddPlasmaClasses(m);
	pyVaultAgeLinkNode::AddPlasmaClasses(m);
	pyVaultChronicleNode::AddPlasmaClasses(m);
	pyVaultImageNode::AddPlasmaClasses(m);
	//pyVaultMarkerListNode::AddPlasmaClasses(m);
	pyVaultMarkerNode::AddPlasmaClasses(m);
	pyVaultPlayerInfoListNode::AddPlasmaClasses(m);
	pyVaultPlayerInfoNode::AddPlasmaClasses(m);
	pyVaultPlayerNode::AddPlasmaClasses(m);
	pyVaultSDLNode::AddPlasmaClasses(m);
	//pyVaultSystemNode::AddPlasmaClasses(m);
	pyVaultTextNoteNode::AddPlasmaClasses(m);

	// Constants
	pyNetLinkingMgr::AddPlasmaConstantsClasses(m);
	pySDL::AddPlasmaConstantsClasses(m);
	pyStatusLog::AddPlasmaConstantsClasses(m);
	pyVault::AddPlasmaConstantsClasses(m);

	delete [] plasmaMethods; // cleanup
}