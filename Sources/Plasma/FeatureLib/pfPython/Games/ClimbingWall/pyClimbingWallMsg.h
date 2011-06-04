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
#ifndef pyClimbingWallMsg_h
#define pyClimbingWallMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyClimbingWallMsg
//
// PURPOSE: Class wrapper for ClimbingWall game messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyClimbingWallMsg : public pyGameCliMsg
{
protected:
	pyClimbingWallMsg();
	pyClimbingWallMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallMsg); // converts a PyObject to a pyClimbingWallMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetClimbingWallMsgType() const;

	PyObject* UpcastToFinalClimbingWallMsg() const; // returns the climbing wall message that this really is
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallNumBlockersChangedMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallNumBlockersChangedMsg();
	pyClimbingWallNumBlockersChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallNumBlockersChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallNumBlockersChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallNumBlockersChangedMsg); // converts a PyObject to a pyClimbingWallNumBlockersChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int NewBlockerCount() const;
	bool LocalOnly() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallReadyMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallReadyMsg();
	pyClimbingWallReadyMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallReadyMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallReadyMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallReadyMsg); // converts a PyObject to a pyClimbingWallReadyMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int ReadyType() const;
	bool Team1Ready() const;
	bool Team2Ready() const;
	bool LocalOnly() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallBlockersChangedMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallBlockersChangedMsg();
	pyClimbingWallBlockersChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallBlockersChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallBlockersChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallBlockersChangedMsg); // converts a PyObject to a pyClimbingWallBlockersChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int TeamNumber() const;
	std::vector<int> BlockersSet() const;
	bool LocalOnly() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallPlayerEnteredMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallPlayerEnteredMsg();
	pyClimbingWallPlayerEnteredMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallPlayerEnteredMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallPlayerEnteredMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallPlayerEnteredMsg); // converts a PyObject to a pyClimbingWallPlayerEnteredMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallSuitMachineLockedMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallSuitMachineLockedMsg();
	pyClimbingWallSuitMachineLockedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallSuitMachineLockedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallSuitMachineLockedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallSuitMachineLockedMsg); // converts a PyObject to a pyClimbingWallSuitMachineLockedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool Team1MachineLocked() const;
	bool Team2MachineLocked() const;
	bool LocalOnly() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyClimbingWallGameOverMsg : public pyClimbingWallMsg
{
protected:
	pyClimbingWallGameOverMsg();
	pyClimbingWallGameOverMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallGameOverMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallGameOverMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallGameOverMsg); // converts a PyObject to a pyClimbingWallGameOverMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int TeamWon() const;
	std::vector<int> Team1Blockers() const;
	std::vector<int> Team2Blockers() const;
	bool LocalOnly() const;
};

#endif // pyClimbingWallMsg_h