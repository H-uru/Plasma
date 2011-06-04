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
#ifndef pyHeekMsg_h
#define pyHeekMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyHeekMsg
//
// PURPOSE: Class wrapper for Heek game messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCliMsg.h"

class pyHeekMsg : public pyGameCliMsg
{
protected:
	pyHeekMsg();
	pyHeekMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptHeekMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekMsg); // converts a PyObject to a pyHeekMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetHeekMsgType() const;

	PyObject* UpcastToFinalHeekMsg() const; // returns the heek message this really is
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekPlayGameMsg : public pyHeekMsg
{
protected:
	pyHeekPlayGameMsg();
	pyHeekPlayGameMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekPlayGameMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekPlayGameMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekPlayGameMsg); // converts a PyObject to a pyHeekPlayGameMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool IsPlaying() const;
	bool IsSinglePlayer() const;
	bool EnableButtons() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekGoodbyeMsg : public pyHeekMsg
{
protected:
	pyHeekGoodbyeMsg();
	pyHeekGoodbyeMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekGoodbyeMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekGoodbyeMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekGoodbyeMsg); // converts a PyObject to a pyHeekGoodbyeMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekWelcomeMsg : public pyHeekMsg
{
protected:
	pyHeekWelcomeMsg();
	pyHeekWelcomeMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekWelcomeMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekWelcomeMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekWelcomeMsg); // converts a PyObject to a pyHeekWelcomeMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long Points() const;
	unsigned long Rank() const;
	std::wstring Name() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekDropMsg : public pyHeekMsg
{
protected:
	pyHeekDropMsg();
	pyHeekDropMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekDropMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekDropMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekDropMsg); // converts a PyObject to a pyHeekDropMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int Position() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekSetupMsg : public pyHeekMsg
{
protected:
	pyHeekSetupMsg();
	pyHeekSetupMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekSetupMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekSetupMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekSetupMsg); // converts a PyObject to a pyHeekSetupMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int Position() const;
	bool ButtonState() const;
	std::vector<bool> LightOn() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekLightStateMsg : public pyHeekMsg
{
protected:
	pyHeekLightStateMsg();
	pyHeekLightStateMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekLightStateMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekLightStateMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekLightStateMsg); // converts a PyObject to a pyHeekLightStateMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int LightNum() const;
	int State() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekInterfaceStateMsg : public pyHeekMsg
{
protected:
	pyHeekInterfaceStateMsg();
	pyHeekInterfaceStateMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekInterfaceStateMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekInterfaceStateMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekInterfaceStateMsg); // converts a PyObject to a pyHeekInterfaceStateMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool ButtonsEnabled() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekCountdownStateMsg : public pyHeekMsg
{
protected:
	pyHeekCountdownStateMsg();
	pyHeekCountdownStateMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekCountdownStateMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekCountdownStateMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekCountdownStateMsg); // converts a PyObject to a pyHeekCountdownStateMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int State() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekWinLoseMsg : public pyHeekMsg
{
protected:
	pyHeekWinLoseMsg();
	pyHeekWinLoseMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekWinLoseMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekWinLoseMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekWinLoseMsg); // converts a PyObject to a pyHeekWinLoseMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool Win() const;
	int Choice() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekGameWinMsg : public pyHeekMsg
{
protected:
	pyHeekGameWinMsg();
	pyHeekGameWinMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekGameWinMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekGameWinMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekGameWinMsg); // converts a PyObject to a pyHeekGameWinMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	int Choice() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyHeekPointUpdateMsg : public pyHeekMsg
{
protected:
	pyHeekPointUpdateMsg();
	pyHeekPointUpdateMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekPointUpdateMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekPointUpdateMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekPointUpdateMsg); // converts a PyObject to a pyHeekPointUpdateMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	bool DisplayUpdate() const;
	unsigned long Points() const;
	unsigned long Rank() const;
};

#endif // pyHeekMsg_h