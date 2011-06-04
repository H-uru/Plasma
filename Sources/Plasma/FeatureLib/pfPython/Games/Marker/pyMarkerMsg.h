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
#ifndef pyMarkerMsg_h
#define pyMarkerMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyMarkerMsg
//
// PURPOSE: Class wrapper for Marker game messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyMarkerMsg : public pyGameCliMsg
{
protected:
	pyMarkerMsg();
	pyMarkerMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptMarkerMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMsg); // converts a PyObject to a pyMarkerMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetMarkerMsgType() const;

	PyObject* UpcastToFinalMarkerMsg() const; // returns this message as the marker message it really is
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerTemplateCreatedMsg : public pyMarkerMsg
{
protected:
	pyMarkerTemplateCreatedMsg();
	pyMarkerTemplateCreatedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerTemplateCreatedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerTemplateCreatedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerTemplateCreatedMsg); // converts a PyObject to a pyMarkerTemplateCreatedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	std::wstring TemplateID() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerTeamAssignedMsg : public pyMarkerMsg
{
protected:
	pyMarkerTeamAssignedMsg();
	pyMarkerTeamAssignedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerTeamAssignedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerTeamAssignedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerTeamAssignedMsg); // converts a PyObject to a pyMarkerTeamAssignedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int TeamNumber() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameTypeMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameTypeMsg();
	pyMarkerGameTypeMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameTypeMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameTypeMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameTypeMsg); // converts a PyObject to a pyMarkerGameTypeMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int GameType() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameStartedMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameStartedMsg();
	pyMarkerGameStartedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameStartedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameStartedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameStartedMsg); // converts a PyObject to a pyMarkerGameStartedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGamePausedMsg : public pyMarkerMsg
{
protected:
	pyMarkerGamePausedMsg();
	pyMarkerGamePausedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGamePausedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGamePausedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGamePausedMsg); // converts a PyObject to a pyMarkerGamePausedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long TimeLeft() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameResetMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameResetMsg();
	pyMarkerGameResetMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameResetMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameResetMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameResetMsg); // converts a PyObject to a pyMarkerGameResetMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameOverMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameOverMsg();
	pyMarkerGameOverMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameOverMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameOverMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameOverMsg); // converts a PyObject to a pyMarkerGameOverMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameNameChangedMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameNameChangedMsg();
	pyMarkerGameNameChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameNameChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameNameChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameNameChangedMsg); // converts a PyObject to a pyMarkerGameNameChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	std::wstring Name() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerTimeLimitChangedMsg : public pyMarkerMsg
{
protected:
	pyMarkerTimeLimitChangedMsg();
	pyMarkerTimeLimitChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerTimeLimitChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerTimeLimitChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerTimeLimitChangedMsg); // converts a PyObject to a pyMarkerTimeLimitChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long TimeLimit() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerGameDeletedMsg : public pyMarkerMsg
{
protected:
	pyMarkerGameDeletedMsg();
	pyMarkerGameDeletedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerGameDeletedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerGameDeletedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerGameDeletedMsg); // converts a PyObject to a pyMarkerGameDeletedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool Failed() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerMarkerAddedMsg : public pyMarkerMsg
{
protected:
	pyMarkerMarkerAddedMsg();
	pyMarkerMarkerAddedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerMarkerAddedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMarkerAddedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMarkerAddedMsg); // converts a PyObject to a pyMarkerMarkerAddedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	double X() const;
	double Y() const;
	double Z() const;
	unsigned long MarkerId() const;
	std::wstring Name() const;
	std::wstring Age() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerMarkerDeletedMsg : public pyMarkerMsg
{
protected:
	pyMarkerMarkerDeletedMsg();
	pyMarkerMarkerDeletedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerMarkerDeletedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMarkerDeletedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMarkerDeletedMsg); // converts a PyObject to a pyMarkerMarkerDeletedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long MarkerId() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerMarkerNameChangedMsg : public pyMarkerMsg
{
protected:
	pyMarkerMarkerNameChangedMsg();
	pyMarkerMarkerNameChangedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerMarkerNameChangedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMarkerNameChangedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMarkerNameChangedMsg); // converts a PyObject to a pyMarkerMarkerNameChangedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long MarkerId() const;
	std::wstring Name() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyMarkerMarkerCapturedMsg : public pyMarkerMsg
{
protected:
	pyMarkerMarkerCapturedMsg();
	pyMarkerMarkerCapturedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptMarkerMarkerCapturedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyMarkerMarkerCapturedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyMarkerMarkerCapturedMsg); // converts a PyObject to a pyMarkerMarkerCapturedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long MarkerId() const;
	unsigned int Team() const;
};

#endif // pyMarkerMsg_h