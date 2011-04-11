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
#include "pyAgeVault.h"
#include "pyAgeInfoStruct.h"
#include "pySDL.h"

#include <python.h>

// glue functions
PYTHON_CLASS_DEFINITION(ptAgeVault, pyAgeVault);

PYTHON_DEFAULT_NEW_DEFINITION(ptAgeVault, pyAgeVault)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptAgeVault)

PYTHON_INIT_DEFINITION(ptAgeVault, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getAgeInfo)
{
	return self->fThis->GetAgeInfo();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getAgeDevicesFolder)
{
	return self->fThis->GetAgeDevicesFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getSubAgesFolder)
{
	return self->fThis->GetSubAgesFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getChronicleFolder)
{
	return self->fThis->GetChronicleFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getAgesIOwnFolder)
{
	return self->fThis->GetBookshelfFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getBookshelfFolder)
{
	return self->fThis->GetBookshelfFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getPeopleIKnowAboutFolder)
{
	return self->fThis->GetPeopleIKnowAboutFolder();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getPublicAgesFolder)
{
	return self->fThis->GetPublicAgesFolder();
}

PYTHON_METHOD_DEFINITION(ptAgeVault, getSubAgeLink, args)
{
	PyObject* ageInfoObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getSubAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	if (!pyAgeInfoStruct::Check(ageInfoObj))
	{
		PyErr_SetString(PyExc_TypeError, "getSubAgeLink expects a ptAgeInfoStruct");
		PYTHON_RETURN_ERROR;
	}
	pyAgeInfoStruct* ageInfo = pyAgeInfoStruct::ConvertFrom(ageInfoObj);
	return self->fThis->GetSubAgeLink(*ageInfo);
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getAgeGuid)
{
	return PyString_FromString(self->fThis->GetAgeGuid());
}

PYTHON_METHOD_DEFINITION(ptAgeVault, addDevice, args)
{
	char* name;
	PyObject* cbObj = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "s|Ol", &name, &cbObj, &context))
	{
		PyErr_SetString(PyExc_TypeError, "addDevice expects a string, an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddDevice(name, cbObj, context);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAgeVault, removeDevice, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "removeDevice expects a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->RemoveDevice(name);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAgeVault, hasDevice, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "hasDevice expects a string");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->HasDevice(name));
}

PYTHON_METHOD_DEFINITION(ptAgeVault, getDevice, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "getDevice expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetDevice(name);
}

PYTHON_METHOD_DEFINITION(ptAgeVault, setDeviceInbox, args)
{
	char* name;
	char* inboxName;
	PyObject* cb = NULL;
	unsigned long context = 0;
	if (!PyArg_ParseTuple(args, "ss|Ol", &name, &inboxName, &cb, &context))
	{
		PyErr_SetString(PyExc_TypeError, "setDeviceInbox expects two strings, an optional object, and an optional unsigned long");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->SetDeviceInbox(name, inboxName, cb, context);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION(ptAgeVault, getDeviceInbox, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "getDeviceInbox expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetDeviceInbox(name);
}

PYTHON_METHOD_DEFINITION(ptAgeVault, findChronicleEntry, args)
{
	char* name;
	if (!PyArg_ParseTuple(args, "s", &name))
	{
		PyErr_SetString(PyExc_TypeError, "findChronicleEntry expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->FindChronicleEntry(name);
}

PYTHON_METHOD_DEFINITION(ptAgeVault, addChronicleEntry, args)
{
	char* name;
	unsigned long entryType;
	char* val;
	if (!PyArg_ParseTuple(args, "sls", &name, &entryType, &val))
	{
		PyErr_SetString(PyExc_TypeError, "addChronicleEntry expects a string, an unsigned long, and a string");
		PYTHON_RETURN_ERROR;
	}
	self->fThis->AddChronicleEntry(name, entryType, val);
	PYTHON_RETURN_NONE;
}

PYTHON_METHOD_DEFINITION_NOARGS(ptAgeVault, getAgeSDL)
{
	return self->fThis->GetAgeSDL();
}

PYTHON_METHOD_DEFINITION(ptAgeVault, updateAgeSDL, args)
{
	PyObject* recordObj = NULL;
	if (!PyArg_ParseTuple(args, "O", &recordObj))
	{
		PyErr_SetString(PyExc_TypeError, "updateAgeSDL expects a ptSDLStateDataRecord");
		PYTHON_RETURN_NONE;
	}
	if (!pySDLStateDataRecord::Check(recordObj))
	{
		PyErr_SetString(PyExc_TypeError, "updateAgeSDL expects a ptSDLStateDataRecord");
		PYTHON_RETURN_NONE;
	}
	pySDLStateDataRecord* record = pySDLStateDataRecord::ConvertFrom(recordObj);
	self->fThis->UpdateAgeSDL(*record);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptAgeVault)
	PYTHON_METHOD_NOARGS(ptAgeVault, getAgeInfo, "Returns a ptVaultAgeInfoNode of the this Age"),
	PYTHON_METHOD_NOARGS(ptAgeVault, getAgeDevicesFolder, "Returns a ptVaultFolderNode of the inboxes for the devices in this Age."),
	PYTHON_METHOD_NOARGS(ptAgeVault, getSubAgesFolder, "Returns a ptVaultFolderNode of sub Age's folder."),
	PYTHON_METHOD_NOARGS(ptAgeVault, getChronicleFolder, "Returns a ptVaultFolderNode"),
	PYTHON_METHOD_NOARGS(ptAgeVault, getAgesIOwnFolder, "(depreciated, use getBookshelfFolder) Returns a ptVaultFolderNode that contain the Ages I own"),
	PYTHON_METHOD_NOARGS(ptAgeVault, getBookshelfFolder, "Personal age only: Returns a ptVaultFolderNode that contains the owning player's AgesIOwn age list"),
	PYTHON_METHOD_NOARGS(ptAgeVault, getPeopleIKnowAboutFolder, "Returns a ptVaultPlayerInfoListNode of the players the Age knows about(?)."),
	PYTHON_METHOD_NOARGS(ptAgeVault, getPublicAgesFolder, "Returns a ptVaultFolderNode that contains all the public Ages"),
	PYTHON_METHOD(ptAgeVault, getSubAgeLink, "Params: ageInfo\nReturns a ptVaultAgeLinkNode to 'ageInfo' (a ptAgeInfoStruct) for this Age."),
	PYTHON_METHOD_NOARGS(ptAgeVault, getAgeGuid, "Returns the current Age's guid as a string."),
	PYTHON_METHOD(ptAgeVault, addDevice, "Params: deviceName,cb=None,cbContext=0\nAdds a device to the age"),
	PYTHON_METHOD(ptAgeVault, removeDevice, "Params: deviceName\nRemoves a device from the age"),
	PYTHON_METHOD(ptAgeVault, hasDevice, "Params: deviceName\nDoes a device with this name exist?"),
	PYTHON_METHOD(ptAgeVault, getDevice, "Params: deviceName\nReturns the specified device (ptVaultTextNoteNode)"),
	PYTHON_METHOD(ptAgeVault, setDeviceInbox, "Params: deviceName,inboxName,cb=None,cbContext=0\nSet's the device's inbox"),
	PYTHON_METHOD(ptAgeVault, getDeviceInbox, "Params: deviceName\nReturns a ptVaultFolderNode of the inbox for the named device in this age."),
	PYTHON_METHOD(ptAgeVault, findChronicleEntry, "Params: entryName\nReturns the named ptVaultChronicleNode"),
	PYTHON_METHOD(ptAgeVault, addChronicleEntry, "Params: name,type,value\nAdds a chronicle entry with the specified type and value"),
	PYTHON_METHOD_NOARGS(ptAgeVault, getAgeSDL, "Returns the age's SDL (ptSDLStateDataRecord)"),
	PYTHON_METHOD(ptAgeVault, updateAgeSDL, "Params: pyrec\nUpdates the age's SDL"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptAgeVault, "Accessor class to the Age's vault");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptAgeVault, pyAgeVault)

PYTHON_CLASS_CHECK_IMPL(ptAgeVault, pyAgeVault)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptAgeVault, pyAgeVault)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyAgeVault::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptAgeVault);
	PYTHON_CLASS_IMPORT_END(m);
}