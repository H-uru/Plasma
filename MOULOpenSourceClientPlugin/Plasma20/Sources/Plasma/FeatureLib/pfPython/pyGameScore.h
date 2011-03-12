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
#ifndef pyGameScore_h
#define pyGameScore_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyGameScore
//
// PURPOSE: a wrapper class to provide access to a game score
//

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"

struct pfGameScore;

class pyGameScore
{
private:
	pfGameScore * fScore;

public:
	pyGameScore();
	pyGameScore(pfGameScore * score);
	~pyGameScore();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptGameScore);
	static PyObject* New(pfGameScore* score);
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyGameScore object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyGameScore); // converts a PyObject to a pyGameScore (throws error if not correct type)

	static void		AddPlasmaClasses(PyObject *m);

	int				GetScoreID();
	UInt32			GetCreatedTime();
	int				GetOwnerID();
	int				GetGameType();
	int				GetValue();
	const char*		GetGameName();

	bool			AddPoints(int numPoints);
	bool			TransferPoints(unsigned destination, int numPoints);
	bool			SetPoints(int numPoints);
};

#endif