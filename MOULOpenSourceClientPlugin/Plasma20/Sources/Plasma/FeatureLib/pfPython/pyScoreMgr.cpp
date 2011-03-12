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

#include "../pfGameScoreMgr/pfGameScoreMgr.h"
#include "../plVault/plVault.h"
#include "../plNetCommon/plNetCommon.h"
#include "pyGameScore.h"

pyScoreMgr::pyScoreMgr()
{
}

pyScoreMgr::~pyScoreMgr()
{
}

bool pyScoreMgr::DeleteScore(unsigned scoreId)
{
	return IS_NET_SUCCESS(pfGameScoreMgr::GetInstance()->DeleteScore(scoreId));
}

PyObject* pyScoreMgr::CreateGlobalScore(
	const char *	gameName,
	unsigned		gameType,
	int				value
) {
	pfGameScore * score = NEWZERO(pfGameScore);
	score->IncRef();

	pfGameScoreMgr::GetInstance()->CreateScore(0, gameName, gameType, value, *score);

	if (score)
	{
		if (score->scoreId > 0)
		{
			PyObject* pyScore = pyGameScore::New(score);
			score->DecRef();

			return pyScore;
		}

		score->DecRef();
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::GetGlobalScores(const char* gameName)
{
	pfGameScore**	scoreList		= nil;
	int				scoreListCount	= 0;
	ENetError result = pfGameScoreMgr::GetInstance()->GetScoresIncRef(0, gameName, scoreList, scoreListCount);

	if (IS_NET_SUCCESS(result) && scoreListCount > 0)
	{
		PyObject* pyScoreList = PyList_New(scoreListCount);
		for (int i = 0; i < scoreListCount; ++i)
		{
			PyObject* pyScore = pyGameScore::New(scoreList[i]);
			PyList_SetItem(pyScoreList, i, pyScore);
			scoreList[i]->DecRef();
		}

		delete [] scoreList;

		return pyScoreList;
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::CreatePlayerScore(
	const char *	gameName,
	unsigned		gameType,
	int				value
) {
	pfGameScore * score = nil;

	if (RelVaultNode * rvn = VaultGetPlayerInfoNodeIncRef()) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		score = NEWZERO(pfGameScore);
		score->IncRef();
		pfGameScoreMgr::GetInstance()->CreateScore(ownerId, gameName, gameType, value, *score);
	}

	if (score)
	{
		if (score->scoreId > 0)
		{
			PyObject* pyScore = pyGameScore::New(score);
			score->DecRef();

			return pyScore;
		}

		score->DecRef();
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::GetPlayerScores(const char* gameName)
{
	if (RelVaultNode * rvn = VaultGetPlayerInfoNodeIncRef()) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		pfGameScore**	scoreList		= nil;
		int				scoreListCount	= 0;
		ENetError result = pfGameScoreMgr::GetInstance()->GetScoresIncRef(ownerId, gameName, scoreList, scoreListCount);

		if (IS_NET_SUCCESS(result) && scoreListCount > 0)
		{
			PyObject* pyScoreList = PyList_New(scoreListCount);
			for (int i = 0; i < scoreListCount; ++i)
			{
				PyObject* pyScore = pyGameScore::New(scoreList[i]);
				PyList_SetItem(pyScoreList, i, pyScore);
				scoreList[i]->DecRef();
			}

			delete [] scoreList;

			return pyScoreList;
		}
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::CreateNeighborhoodScore(
	const char *	gameName,
	unsigned		gameType,
	int				value
) {
	pfGameScore * score = nil;

	plAgeInfoStruct info;
	info.SetAgeFilename(kNeighborhoodAgeFilename);

	if (RelVaultNode * rvn = VaultGetOwnedAgeInfoIncRef(&info)) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		score = NEWZERO(pfGameScore);
		score->IncRef();
		pfGameScoreMgr::GetInstance()->CreateScore(ownerId, gameName, gameType, value, *score);
	}

	if (score)
	{
		if (score->scoreId > 0)
		{
			PyObject* pyScore = pyGameScore::New(score);
			score->DecRef();

			return pyScore;
		}

		score->DecRef();
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::GetNeighborhoodScores(const char* gameName)
{
	plAgeInfoStruct info;
	info.SetAgeFilename(kNeighborhoodAgeFilename);

	if (RelVaultNode * rvn = VaultGetOwnedAgeInfoIncRef(&info)) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		pfGameScore**	scoreList		= nil;
		int				scoreListCount	= 0;
		ENetError result = pfGameScoreMgr::GetInstance()->GetScoresIncRef(ownerId, gameName, scoreList, scoreListCount);

		if (IS_NET_SUCCESS(result) && scoreListCount > 0)
		{
			PyObject* pyScoreList = PyList_New(scoreListCount);
			for (int i = 0; i < scoreListCount; ++i)
			{
				PyObject* pyScore = pyGameScore::New(scoreList[i]);
				PyList_SetItem(pyScoreList, i, pyScore);
				scoreList[i]->DecRef();
			}

			delete [] scoreList;

			return pyScoreList;
		}
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::CreateCurrentAgeScore(
	const char *	gameName,
	unsigned		gameType,
	int				value
) {
	pfGameScore * score = nil;

	if (RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef()) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		score = NEWZERO(pfGameScore);
		score->IncRef();
		pfGameScoreMgr::GetInstance()->CreateScore(ownerId, gameName, gameType, value, *score);
	}

	if (score)
	{
		if (score->scoreId > 0)
		{
			PyObject* pyScore = pyGameScore::New(score);
			score->DecRef();

			return pyScore;
		}

		score->DecRef();
	}

	PYTHON_RETURN_NONE;
}

PyObject* pyScoreMgr::GetCurrentAgeScores(const char* gameName)
{
	if (RelVaultNode * rvn = VaultGetAgeInfoNodeIncRef()) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		pfGameScore**	scoreList		= nil;
		int				scoreListCount	= 0;
		ENetError result = pfGameScoreMgr::GetInstance()->GetScoresIncRef(ownerId, gameName, scoreList, scoreListCount);

		if (IS_NET_SUCCESS(result) && scoreListCount > 0)
		{
			PyObject* pyScoreList = PyList_New(scoreListCount);
			for (int i = 0; i < scoreListCount; ++i)
			{
				PyObject* pyScore = pyGameScore::New(scoreList[i]);
				PyList_SetItem(pyScoreList, i, pyScore);
				scoreList[i]->DecRef();
			}

			delete [] scoreList;

			return pyScoreList;
		}
	}

	PYTHON_RETURN_NONE;
}

PyObject * pyScoreMgr::GetRankList(
	unsigned		scoreGroup,
	unsigned		parentFolderId,
	const char *	gameName,
	unsigned		timePeriod,
	unsigned		numResults,
	unsigned		pageNumber,
	bool			sortDesc
) {
	if (RelVaultNode * rvn = VaultGetPlayerInfoNodeIncRef()) {
		unsigned ownerId = rvn->nodeId;
		rvn->DecRef();

		NetGameRank**	rankList		= nil;
		int				rankListCount	= 0;
		ENetError result = pfGameScoreMgr::GetInstance()->GetRankList(
			ownerId,
			scoreGroup,
			parentFolderId,
			gameName,
			timePeriod,
			numResults,
			pageNumber,
			sortDesc,
			rankList,
			rankListCount
		);

		if (IS_NET_SUCCESS(result) && rankListCount > 0)
		{
			PyObject* pyRankList = PyList_New(rankListCount);
			for (int i = 0; i < rankListCount; ++i)
			{
				char tempStr[kMaxPlayerNameLength];
				StrToAnsi(tempStr, rankList[i]->name, arrsize(tempStr));

				PyObject* pyRank = PyTuple_New(3);
				PyTuple_SetItem(pyRank, 0, PyInt_FromLong(rankList[i]->rank));
				PyTuple_SetItem(pyRank, 1, PyString_FromString(tempStr));
				PyTuple_SetItem(pyRank, 2, PyInt_FromLong(rankList[i]->score));

				PyList_SetItem(pyRankList, i, pyRank);

				delete rankList[i];
			}

			delete [] rankList;

			return pyRankList;
		}
	}

	PYTHON_RETURN_NONE;
}
