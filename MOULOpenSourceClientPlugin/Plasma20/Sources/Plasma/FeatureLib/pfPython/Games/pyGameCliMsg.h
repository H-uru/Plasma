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
#ifndef pyGameCliMsg_h
#define pyGameCliMsg_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyGameCliMsg
//
// PURPOSE: Class wrapper for game client messages
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../pyGlueHelpers.h"

class pyGameCliMsg
{
protected:
	pfGameCliMsg* message;

	pyGameCliMsg();
	pyGameCliMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptGameCliMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCliMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCliMsg); // converts a PyObject to a pyGameCliMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	int GetType() const;
	PyObject* GetGameCli() const; // returns ptGameCli

	PyObject* UpcastToFinalGameCliMsg() const; // returns it upcasted to player joined/left/invite/owner change message
	PyObject* UpcastToGameMsg() const; // returns it upcasted to a game's message base class

	// for convenience, we define our own message types, one message type for each game, so that we can have
	// a "base class" for each game's messages. Anything under kCli2Srv_NumGameMsgIds keeps their normal type
	// (and is exposed to python here), but anything above it has the message code look at the game type and
	// return the "message type" indicating what game the message is from
	enum pyGameCliMsgType
	{
		kPyGameCliMsgTypeStart = kCli2Srv_NumGameMsgIds,
		kPyGameCliTTTMsg, // Tick Tack Toe game messages
		kPyGameCliHeekMsg, // Heek game messages
		kPyGameCliMarkerMsg, // Marker game messages
		kPyGameCliBlueSpiralMsg, // Blue Spiral game messages
		kPyGameCliClimbingWallMsg, // Climbing Wall game messages
		kPyGameCliVarSyncMsg, // Var Sync game messages
	};
};

///////////////////////////////////////////////////////////////////////////////
class pyGameCliPlayerJoinedMsg : public pyGameCliMsg
{
protected:
	pyGameCliPlayerJoinedMsg();
	pyGameCliPlayerJoinedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGameCliPlayerJoinedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCliPlayerJoinedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCliPlayerJoinedMsg); // converts a PyObject to a pyGameCliPlayerJoinedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long PlayerID() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyGameCliPlayerLeftMsg : public pyGameCliMsg
{
protected:
	pyGameCliPlayerLeftMsg();
	pyGameCliPlayerLeftMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGameCliPlayerLeftMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCliPlayerLeftMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCliPlayerLeftMsg); // converts a PyObject to a pyGameCliPlayerLeftMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long PlayerID() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyGameCliInviteFailedMsg : public pyGameCliMsg
{
protected:
	pyGameCliInviteFailedMsg();
	pyGameCliInviteFailedMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGameCliInviteFailedMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCliInviteFailedMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCliInviteFailedMsg); // converts a PyObject to a pyGameCliInviteFailedMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);

	unsigned long InviteeID() const;
	unsigned long OperationID() const;
	int Error() const;
};

///////////////////////////////////////////////////////////////////////////////
class pyGameCliOwnerChangeMsg : public pyGameCliMsg
{
protected:
	pyGameCliOwnerChangeMsg();
	pyGameCliOwnerChangeMsg(pfGameCliMsg* msg);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGameCliOwnerChangeMsg);
	static PyObject* New(pfGameCliMsg* msg);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCliOwnerChangeMsg object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCliOwnerChangeMsg); // converts a PyObject to a pyGameCliOwnerChangeMsg (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);

	unsigned long OwnerID() const;
};

#endif // pyGameCliMsg_h