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
#ifndef pyVarSyncMsg_h
#define pyVarSyncMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyVarSyncMsg
//
// PURPOSE: Class wrapper for VarSync game messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyVarSyncMsg : public pyGameCliMsg
{
protected:
	pyVarSyncMsg();
	pyVarSyncMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncMsg); // converts a PyObject to a pyVarSyncMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetVarSyncMsgType() const;

	PyObject* UpcastToFinalVarSyncMsg() const; // returns the VarSync message that this really is
};

///////////////////////////////////////////////////////////////////////////////
class pyVarSyncStringVarChangedMsg : public pyVarSyncMsg
{
protected:
	pyVarSyncStringVarChangedMsg();
	pyVarSyncStringVarChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncStringVarChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncStringVarChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncStringVarChangedMsg); // converts a PyObject to a pyVarSyncStringVarChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long ID() const;
	std::wstring Value() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyVarSyncNumericVarChangedMsg : public pyVarSyncMsg
{
protected:
	pyVarSyncNumericVarChangedMsg();
	pyVarSyncNumericVarChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncNumericVarChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncNumericVarChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncNumericVarChangedMsg); // converts a PyObject to a pyVarSyncNumericVarChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long ID() const;
	double Value() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyVarSyncAllVarsSentMsg : public pyVarSyncMsg
{
protected:
	pyVarSyncAllVarsSentMsg();
	pyVarSyncAllVarsSentMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncAllVarsSentMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncAllVarsSentMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncAllVarsSentMsg); // converts a PyObject to a pyVarSyncAllVarsSentMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyVarSyncStringVarCreatedMsg : public pyVarSyncMsg
{
protected:
	pyVarSyncStringVarCreatedMsg();
	pyVarSyncStringVarCreatedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncStringVarCreatedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncStringVarCreatedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncStringVarCreatedMsg); // converts a PyObject to a pyVarSyncStringVarCreatedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	std::wstring Name() const;
	unsigned long ID() const;
	std::wstring Value() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyVarSyncNumericVarCreatedMsg : public pyVarSyncMsg
{
protected:
	pyVarSyncNumericVarCreatedMsg();
	pyVarSyncNumericVarCreatedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptVarSyncNumericVarCreatedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyVarSyncNumericVarCreatedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyVarSyncNumericVarCreatedMsg); // converts a PyObject to a pyVarSyncNumericVarCreatedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	std::wstring Name() const;
	unsigned long ID() const;
	double Value() const;
};

#endif // pyVarSyncMsg_h