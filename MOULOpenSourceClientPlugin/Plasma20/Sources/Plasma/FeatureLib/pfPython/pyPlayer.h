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
#ifndef pyPlayer_h
#define pyPlayer_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyPlayer
//
// PURPOSE: Class wrapper for Python to the player data
//

#include "hsStlUtils.h"
#include "pyKey.h"

#include <python.h>
#include "pyGlueHelpers.h"

class plKey;

class pyPlayer
{
protected:
	plKey			fAvatarKey;
	std::string		fPlayerName;
	UInt32			fPlayerID;
	hsScalar		fDistSq;			// from local player, temp
	hsBool			fIsCCR;
	hsBool			fIsServer;

	pyPlayer(); // only used by python glue, do NOT call
	pyPlayer(pyKey& avKey, const char* pname, UInt32 pid, hsScalar distsq);
	pyPlayer(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq);
	// another way to create a player with just a name and number
	pyPlayer(const char* pname, UInt32 pid);
public:
	void Init(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq); // used by python glue, do NOT call

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptPlayer);
	static PyObject *New(pyKey& avKey, const char* pname, UInt32 pid, hsScalar distsq);
	static PyObject *New(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq);
	static PyObject *New(const char* pname, UInt32 pid);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyPlayer object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyPlayer); // converts a PyObject to a pyPlayer (throws error if not correct type)

	static void AddPlasmaClasses(PyObject *m);

	// override the equals to operator
	hsBool operator==(const pyPlayer &player) const
	{
		// only thing that needs testing is the playerid, which is unique for all
		if ( ((pyPlayer*)this)->GetPlayerID() == player.GetPlayerID() )
			return true;
		else
			return false;
	}
	hsBool operator!=(const pyPlayer &player) const { return !(player == *this);	}

	// for C++ access
	plKey GetKey() { return fAvatarKey; }

	// for python access
	const char * GetPlayerName() const { return fPlayerName.c_str();}
	UInt32 GetPlayerID() const 
	{
		return fPlayerID;
	}

	hsScalar GetDistSq() const { return fDistSq; }

	void SetCCRFlag(hsBool state);
	hsBool IsCCR();

	void SetServerFlag(hsBool state);
	hsBool IsServer();

};

#endif  // pyPlayer_h
