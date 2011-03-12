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
#ifndef pyHeekGame_h
#define pyHeekGame_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyHeekGame
//
// PURPOSE: Class wrapper for the Heek game client
//

#include "../pfGameMgr/pfGameMgr.h"

#include <python.h>
#include "../../pyGlueHelpers.h"
#include "../pyGameCli.h"
#include "../../pyKey.h"

class pyHeekGame : public pyGameCli
{
protected:
	pyHeekGame();
	pyHeekGame(pfGameCli* client);

public:
	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptHeekGame);
	static PyObject* New(pfGameCli* client);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyHeekGame object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyHeekGame); // converts a PyObject to a pyHeekGame (throws error if not correct type)

	static void AddPlasmaClasses(PyObject* m);
	static void AddPlasmaConstantsClasses(PyObject* m);
	static void AddPlasmaMethods(std::vector<PyMethodDef>& methods);

	static bool IsHeekGame(std::wstring guid);
	static void JoinCommonHeekGame(pyKey& callbackKey, unsigned gameID);

	void PlayGame(int position, UInt32 points, std::wstring name);
	void LeaveGame();
	void Choose(int choice);
	void SequenceFinished(int seq);
};

#endif // pyHeekGame_h