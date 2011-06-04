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
#ifndef pyTTTGame_h
#define pyTTTGame_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyTTTGame
//
// PURPOSE: Class wrapper for the TTT game client
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCli.h"
#include "../../pyKey.h"

class pyTTTGame : public pyGameCli
{
protected:
	pyTTTGame();
	pyTTTGame(pfGameCli* client);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptTTTGame);
	static PyObject* New(pfGameCli* client);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyTTTGame object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyTTTGame); // converts a PyObject to a pyTTTGame (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);
	static void AddPlasmaMethods(std::vector<PyMethodDef>& methods);

	static bool IsTTTGame(std::wstring guid);
	static void CreateTTTGame(pyKey& callbackKey, unsigned numPlayers);
	static void JoinCommonTTTGame(pyKey& callbackKey, unsigned gameID, unsigned numPlayers);

	void MakeMove(unsigned row, unsigned col);
	void ShowBoard();
};

#endif // pyTTTGame_h