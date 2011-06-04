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
#ifndef pyClimbingWallGame_h
#define pyClimbingWallGame_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyClimbingWallGame
//
// PURPOSE: Class wrapper for the climbing wall game client
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCli.h"
#include "../../pyKey.h"

class pyClimbingWallGame : public pyGameCli
{
protected:
	pyClimbingWallGame();
	pyClimbingWallGame(pfGameCli* client);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptClimbingWallGame);
	static PyObject* New(pfGameCli* client);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyClimbingWallGame object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyClimbingWallGame); // converts a PyObject to a pyClimbingWallGame (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaMethods(std::vector<PyMethodDef>& methods);
	static void AddPlasmaConstantsClasses(PyObject* m);

	static bool IsClimbingWallGame(std::wstring guid);
	static void JoinCommonClimbingWallGame(pyKey& callbackKey, unsigned gameID);

	void ChangeNumBlockers(int amountToAdjust);
	void Ready(unsigned readyType, unsigned teamNumber);
	void ChangeBlocker(unsigned teamNumber, unsigned blockerNumber, bool added);
	void Reset();
	void PlayerEntered(unsigned teamNumber);
	void FinishedGame();
	void Panic();
};

#endif // pyClimbingWallGame_h