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
#ifndef pyScoreMgr_h
#define pyScoreMgr_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: pyScoreMgr
//
// PURPOSE: a wrapper class to provide an interface to the scoring system
//

#include "hsTypes.h"
#include "hsStlUtils.h"

#include <python.h>
#include "pyGlueHelpers.h"


class pyScoreMgr
{
public:
	pyScoreMgr();
	~pyScoreMgr();

	// required functions for PyObject interoperability
	PYTHON_CLASS_NEW_FRIEND(ptScoreMgr);
	PYTHON_CLASS_NEW_DEFINITION;
	PYTHON_CLASS_CHECK_DEFINITION; // returns true if the PyObject is a pyScoreMgr object
	PYTHON_CLASS_CONVERT_FROM_DEFINITION(pyScoreMgr); // converts a PyObject to a pyScoreMgr (throws error if not correct type)

	static void		AddPlasmaClasses(PyObject *m);
	static void		AddPlasmaConstantsClasses(PyObject *m);

	bool DeleteScore(unsigned scoreId);

	PyObject* CreateGlobalScore(
		const char *	gameName,
		unsigned		gameType,
		int				value
	);
	PyObject* GetGlobalScores(const char* gameName);

	PyObject* CreatePlayerScore(
		const char *	gameName,
		unsigned		gameType,
		int				value
	);
	PyObject* GetPlayerScores(const char* gameName);

	PyObject* CreateNeighborhoodScore(
		const char *	gameName,
		unsigned		gameType,
		int				value
	);
	PyObject* GetNeighborhoodScores(const char* gameName);

	PyObject* CreateCurrentAgeScore(
		const char *	gameName,
		unsigned		gameType,
		int				value
	);
	PyObject* GetCurrentAgeScores(const char* gameName);

	PyObject * GetRankList(
		unsigned		scoreGroup,
		unsigned		parentFolderId,
		const char *	gameName,
		unsigned		timePeriod,
		unsigned		numResults,
		unsigned		pageNumber,
		bool			sortDesc
	);
};

#endif