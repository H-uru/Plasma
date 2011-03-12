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
#ifndef pyGameCli_h
#define pyGameCli_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyGameCli
//
// PURPOSE: Class wrapper for the game client base class
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../pyGlueHelpers.h"
#include "../pyKey.h"

class pyGameCli
{
protected:
	pfGameCli* gameClient;

	pyGameCli();
	pyGameCli(pfGameCli* client);

public:
	// required functions for PyObject interoperability
	PYTHON_EXPOSE_TYPE; // so we can subclass
	PYTHON_CLASS_NEW_FRIEND(ptGameCli);
	static PyObject* New(pfGameCli* client);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameCli object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameCli); // converts a PyObject to a pyGameCli (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaMethods(std::vector<PyMethodDef>& methods);

	static std::vector<unsigned> GetGameIDs();
	static PyObject* GetGameCli(unsigned gameID); // returns a ptGameCli
	static std::wstring GetGameNameByTypeID(std::wstring typeID);
	static void JoinGame(pyKey& callbackKey, unsigned gameID);

	unsigned GameID() const;
	std::wstring GameTypeID() const;
	std::wstring Name() const;
	unsigned PlayerCount() const;

	void InvitePlayer(unsigned playerID);
	void UninvitePlayer(unsigned playerID);

	void LeaveGame();

	PyObject* UpcastToTTTGame(); // returns ptTTTGame
	PyObject* UpcastToHeekGame(); // returns ptHeekGame
	PyObject* UpcastToMarkerGame(); // returns ptMarkerGame
	PyObject* UpcastToBlueSpiralGame(); // returns ptBlueSpiralGame
	PyObject* UpcastToClimbingWallGame(); // returns ptClimbingWallGame
	PyObject* UpcastToVarSyncGame(); // returns ptVarSyncGame
};

#endif // pyGameCli_h