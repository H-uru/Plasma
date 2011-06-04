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
#include "pyVaultNodeRef.h"

#include "../plVault/plVault.h"
#include <python.h>

// glue functions
// glue functions
PYTHON_CLASS_DEFINITION(ptVaultNodeRef, pyVaultNodeRef);

PYTHON_DEFAULT_NEW_DEFINITION(ptVaultNodeRef, pyVaultNodeRef)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptVaultNodeRef)

PYTHON_NO_INIT_DEFINITION(ptVaultNodeRef)


PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getParent)
{
	return self->fThis->GetParent();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getChild)
{
	return self->fThis->GetChild();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getParentID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetParentID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getChildID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetChildID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getSaver)
{
	return self->fThis->GetSaver();
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, getSaverID)
{
	return PyLong_FromUnsignedLong(self->fThis->GetSaverID());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, beenSeen)
{
	return PyLong_FromUnsignedLong(self->fThis->BeenSeen());
}

PYTHON_METHOD_DEFINITION_NOARGS(ptVaultNodeRef, setSeen)
{
	self->fThis->SetSeen(true);
	PYTHON_RETURN_NONE;
}

PYTHON_START_METHODS_TABLE(ptVaultNodeRef)
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getParent, "Returns a ptVaultNode that is the parent of the reference"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getChild, "Returns a ptVaultNode that is the child of this reference"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getParentID, "Returns id of the parent node"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getChildID, "Returns id of the child node"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getSaver, "Returns a ptVaultPlayerInfoNode of player that created this relationship"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, getSaverID, "Returns id of player that created this relationship"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, beenSeen, "Returns true until we reimplement this"),
	PYTHON_METHOD_NOARGS(ptVaultNodeRef, setSeen, "Does nothing until we reimplement this"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
#define ptVaultNodeRef_COMPARE			PYTHON_NO_COMPARE
#define ptVaultNodeRef_AS_NUMBER		PYTHON_NO_AS_NUMBER
#define ptVaultNodeRef_AS_SEQUENCE		PYTHON_NO_AS_SEQUENCE
#define ptVaultNodeRef_AS_MAPPING		PYTHON_NO_AS_MAPPING
#define ptVaultNodeRef_RICH_COMPARE		PYTHON_NO_RICH_COMPARE
#define ptVaultNodeRef_STR				PYTHON_NO_STR
#define ptVaultNodeRef_GETSET			PYTHON_NO_GETSET
#define ptVaultNodeRef_BASE				PYTHON_NO_BASE
PLASMA_CUSTOM_TYPE(ptVaultNodeRef, "Vault node relationship pseudo class");
PYTHON_EXPOSE_TYPE_DEFINITION(ptVaultNodeRef, pyVaultNodeRef);

// required functions for PyObject interoperability
PyObject *pyVaultNodeRef::New(RelVaultNode * parent, RelVaultNode * child)
{
	ptVaultNodeRef *newObj = (ptVaultNodeRef*)ptVaultNodeRef_type.tp_new(&ptVaultNodeRef_type, NULL, NULL);
	if (newObj->fThis->fParent)
		newObj->fThis->fParent->DecRef();
	if (newObj->fThis->fChild)
		newObj->fThis->fChild->DecRef();
	newObj->fThis->fParent = parent;
	newObj->fThis->fChild = child;
	if (newObj->fThis->fParent)
		newObj->fThis->fParent->IncRef();
	if (newObj->fThis->fChild)
		newObj->fThis->fChild->IncRef();
	return (PyObject*)newObj;
}

PYTHON_CLASS_CHECK_IMPL(ptVaultNodeRef, pyVaultNodeRef)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptVaultNodeRef, pyVaultNodeRef)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyVaultNodeRef::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptVaultNodeRef);
	PYTHON_CLASS_IMPORT_END(m);
}