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
#ifndef pyBlueSpiralMsg_h
#define pyBlueSpiralMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyBlueSpiralMsg
//
// PURPOSE: Class wrapper for BlueSpiral game messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyBlueSpiralMsg : public pyGameCliMsg
{
protected:
	pyBlueSpiralMsg();
	pyBlueSpiralMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralMsg); // converts a PyObject to a pyBlueSpiralMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetBlueSpiralMsgType() const;

	PyObject* UpcastToFinalBlueSpiralMsg() const; // returns this message as the blue spiral message it is
};

///////////////////////////////////////////////////////////////////////////////
class pyBlueSpiralClothOrderMsg : public pyBlueSpiralMsg
{
protected:
	pyBlueSpiralClothOrderMsg();
	pyBlueSpiralClothOrderMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralClothOrderMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralClothOrderMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralClothOrderMsg); // converts a PyObject to a pyBlueSpiralClothOrderMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	std::vector<int> Order();
};

///////////////////////////////////////////////////////////////////////////////
class pyBlueSpiralSuccessfulHitMsg : public pyBlueSpiralMsg
{
protected:
	pyBlueSpiralSuccessfulHitMsg();
	pyBlueSpiralSuccessfulHitMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralSuccessfulHitMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralSuccessfulHitMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralSuccessfulHitMsg); // converts a PyObject to a pyBlueSpiralSuccessfulHitMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyBlueSpiralGameWonMsg : public pyBlueSpiralMsg
{
protected:
	pyBlueSpiralGameWonMsg();
	pyBlueSpiralGameWonMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralGameWonMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralGameWonMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralGameWonMsg); // converts a PyObject to a pyBlueSpiralGameWonMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyBlueSpiralGameOverMsg : public pyBlueSpiralMsg
{
protected:
	pyBlueSpiralGameOverMsg();
	pyBlueSpiralGameOverMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralGameOverMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralGameOverMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralGameOverMsg); // converts a PyObject to a pyBlueSpiralGameOverMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyBlueSpiralGameStartedMsg : public pyBlueSpiralMsg
{
protected:
	pyBlueSpiralGameStartedMsg();
	pyBlueSpiralGameStartedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptBlueSpiralGameStartedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyBlueSpiralGameStartedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyBlueSpiralGameStartedMsg); // converts a PyObject to a pyBlueSpiralGameStartedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool StartSpin();
};

#endif // pyBlueSpiralMsg_h