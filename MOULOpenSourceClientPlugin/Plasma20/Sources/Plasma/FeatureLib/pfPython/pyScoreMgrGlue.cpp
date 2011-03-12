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
#include "pyScoreMgr.h"

#include "pyEnum.h"
#include "../pfGameScoreMgr/pfGameScoreMgr.h"

// glue functions
PYTHON_CLASS_DEFINITION(ptScoreMgr, pyScoreMgr);

PYTHON_DEFAULT_NEW_DEFINITION(ptScoreMgr, pyScoreMgr)
PYTHON_DEFAULT_DEALLOC_DEFINITION(ptScoreMgr)

PYTHON_INIT_DEFINITION(ptScoreMgr, args, keywords)
{
	PYTHON_RETURN_INIT_OK;
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, deleteScore, args)
{
	int scoreId;
	if (!PyArg_ParseTuple(args, "i", &scoreId))
	{
		PyErr_SetString(PyExc_TypeError, "deleteScore expects an int");
		PYTHON_RETURN_ERROR;
	}
	PYTHON_RETURN_BOOL(self->fThis->DeleteScore(scoreId));
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, createGlobalScore, args)
{
	char* gameName;
	int gameType;
	int value;
	if (!PyArg_ParseTuple(args, "sii", &gameName, &gameType, &value))
	{
		PyErr_SetString(PyExc_TypeError, "createGlobalScore expects a string, an int, and an int");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->CreateGlobalScore(gameName, gameType, value);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, getGlobalScores, args)
{
	char* gameName;
	if (!PyArg_ParseTuple(args, "s", &gameName))
	{
		PyErr_SetString(PyExc_TypeError, "getGlobalScores expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetGlobalScores(gameName);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, createPlayerScore, args)
{
	char* gameName;
	int gameType;
	int value;
	if (!PyArg_ParseTuple(args, "sii", &gameName, &gameType, &value))
	{
		PyErr_SetString(PyExc_TypeError, "createPlayerScore expects a string, an int, and an int");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->CreatePlayerScore(gameName, gameType, value);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, getPlayerScores, args)
{
	char* gameName;
	if (!PyArg_ParseTuple(args, "s", &gameName))
	{
		PyErr_SetString(PyExc_TypeError, "getPlayerScores expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetPlayerScores(gameName);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, createNeighborhoodScore, args)
{
	char* gameName;
	int gameType;
	int value;
	if (!PyArg_ParseTuple(args, "sii", &gameName, &gameType, &value))
	{
		PyErr_SetString(PyExc_TypeError, "createNeighborhoodScore expects a string, an int, and an int");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->CreateNeighborhoodScore(gameName, gameType, value);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, getNeighborhoodScores, args)
{
	char* gameName;
	if (!PyArg_ParseTuple(args, "s", &gameName))
	{
		PyErr_SetString(PyExc_TypeError, "getNeighborhoodScores expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetNeighborhoodScores(gameName);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, createCurrentAgeScore, args)
{
	char* gameName;
	int gameType;
	int value;
	if (!PyArg_ParseTuple(args, "sii", &gameName, &gameType, &value))
	{
		PyErr_SetString(PyExc_TypeError, "createCurrentAgeScore expects a string, an int, and an int");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->CreateCurrentAgeScore(gameName, gameType, value);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, getCurrentAgeScores, args)
{
	char* gameName;
	if (!PyArg_ParseTuple(args, "s", &gameName))
	{
		PyErr_SetString(PyExc_TypeError, "getCurrentAgeScores expects a string");
		PYTHON_RETURN_ERROR;
	}
	return self->fThis->GetCurrentAgeScores(gameName);
}

PYTHON_METHOD_DEFINITION(ptScoreMgr, getRankList, args)
{
	int scoreGroup;
	int parentFolderId;
	char* gameName;
	int timePeriod;
	int numResults;
	int pageNumber;
	int sortDesc;
	if (!PyArg_ParseTuple(args, "iisiiii", &scoreGroup, &parentFolderId, &gameName, &timePeriod, &numResults, &pageNumber, &sortDesc))
	{
		PyErr_SetString(PyExc_TypeError, "getRankList expects two ints, a string, and four more ints");
		PYTHON_RETURN_ERROR;
	}

	return self->fThis->GetRankList(scoreGroup, parentFolderId, gameName, timePeriod, numResults, pageNumber, sortDesc != 0);
}

PYTHON_START_METHODS_TABLE(ptScoreMgr)
	PYTHON_METHOD(ptScoreMgr, deleteScore, "Params: scoreId\nDeletes the specified score"),
	PYTHON_METHOD(ptScoreMgr, createGlobalScore, "Params: gameName, gameType, value\nCreates a score and returns it"),
	PYTHON_METHOD(ptScoreMgr, getGlobalScores, "Params: gameName\nReturns a list of scores associated with the specified game."),
	PYTHON_METHOD(ptScoreMgr, createPlayerScore, "Params: gameName, gameType, value\nCreates a score and returns it"),
	PYTHON_METHOD(ptScoreMgr, getPlayerScores, "Params: gameName\nReturns a list of scores associated with the specified game."),
	PYTHON_METHOD(ptScoreMgr, createNeighborhoodScore, "Params: gameName, gameType, value\nCreates a score and returns it"),
	PYTHON_METHOD(ptScoreMgr, getNeighborhoodScores, "Params: gameName\nReturns a list of scores associated with the specified game."),
	PYTHON_METHOD(ptScoreMgr, createCurrentAgeScore, "Params: gameName, gameType, value\nCreates a score and returns it"),
	PYTHON_METHOD(ptScoreMgr, getCurrentAgeScores, "Params: gameName\nReturns a list of scores associated with the specified game."),
	PYTHON_METHOD(ptScoreMgr, getRankList, "Params: ownerInfoId, scoreGroup, parentFolderId, gameName, timePeriod, numResults, pageNumber, sortDesc\nReturns a list of scores and rank"),
PYTHON_END_METHODS_TABLE;

// Type structure definition
PLASMA_DEFAULT_TYPE(ptScoreMgr, "Game score manager");

// required functions for PyObject interoperability
PYTHON_CLASS_NEW_IMPL(ptScoreMgr, pyScoreMgr)

PYTHON_CLASS_CHECK_IMPL(ptScoreMgr, pyScoreMgr)
PYTHON_CLASS_CONVERT_FROM_IMPL(ptScoreMgr, pyScoreMgr)

///////////////////////////////////////////////////////////////////////////
//
// AddPlasmaClasses - the python module definitions
//
void pyScoreMgr::AddPlasmaClasses(PyObject *m)
{
	PYTHON_CLASS_IMPORT_START(m);
	PYTHON_CLASS_IMPORT(m, ptScoreMgr);
	PYTHON_CLASS_IMPORT_END(m);
}

void pyScoreMgr::AddPlasmaConstantsClasses(PyObject *m)
{
	PYTHON_ENUM_START(PtGameScoreTypes);
	PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kFixed,				kScoreTypeFixed);
	PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kAccumulative,		kScoreTypeAccumulative);
	PYTHON_ENUM_ELEMENT(PtGameScoreTypes, kAccumAllowNegative,	kScoreTypeAccumAllowNegative);
	PYTHON_ENUM_END(m, PtGameScoreTypes);

	PYTHON_ENUM_START(PtScoreRankGroups);
	PYTHON_ENUM_ELEMENT(PtScoreRankGroups, kIndividual,			kScoreRankGroupIndividual);
	PYTHON_ENUM_ELEMENT(PtScoreRankGroups, kNeighborhood,		kScoreRankGroupNeighborhood);
	PYTHON_ENUM_END(m, PtScoreRankGroups);

	PYTHON_ENUM_START(PtScoreTimePeriods);
	PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kOverall,			kScoreTimePeriodOverall);
	PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kYear,				kScoreTimePeriodYear);
	PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kMonth,				kScoreTimePeriodMonth);
	PYTHON_ENUM_ELEMENT(PtScoreTimePeriods, kDay,				kScoreTimePeriodDay);
	PYTHON_ENUM_END(m, PtScoreTimePeriods);
}
