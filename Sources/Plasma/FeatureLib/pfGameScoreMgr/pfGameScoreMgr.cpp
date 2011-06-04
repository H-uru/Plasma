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
#include "pfGameScoreMgr.h"

#include "../pnUtils/pnUtils.h"
#include "../plNetGameLib/plNetGameLib.h"
#include "../pnNetProtocol/pnNetProtocol.h"

//============================================================================
pfGameScore::pfGameScore()
{
}

pfGameScore::~pfGameScore()
{
	pfGameScoreMgr::GetInstance()->RemoveCachedScore(scoreId);
}

void pfGameScore::Init(
	unsigned sid,
	unsigned oid,
	UInt32 createTime,
	const char gname[],
	unsigned gType,
	int val
) {
	scoreId		= sid;
	ownerId		= oid;
	createdTime	= createTime;
	gameType	= gType;
	value		= val;

	StrCopy(gameName, gname, arrsize(gameName));
	pfGameScoreMgr::GetInstance()->AddCachedScore(this);
}

void pfGameScore::CopyFrom(
	const pfGameScore* score
) {
	scoreId		= score->scoreId;
	ownerId		= score->ownerId;
	createdTime	= score->createdTime;
	gameType	= score->gameType;
	value		= score->value;

	StrCopy(gameName, score->gameName, arrsize(gameName));
}

//============================================================================
pfGameScoreMgr::pfGameScoreMgr()
{
}

pfGameScoreMgr* pfGameScoreMgr::GetInstance()
{
	static pfGameScoreMgr s_instance;
	return &s_instance;
}

void pfGameScoreMgr::AddCachedScore(pfGameScore * score)
{
	GameScoreLink * scoreLink = fScores.Find(score->scoreId);
	if (scoreLink == nil)
	{
		GameScoreLink * link = TRACKED_NEW GameScoreLink(score);
		fScores.Add(link);
	}
	else
		scoreLink->score->CopyFrom(score);
}

void pfGameScoreMgr::RemoveCachedScore(unsigned scoreId)
{
	if (GameScoreLink * link = fScores.Find(scoreId))
	{
		DEL(link);
	}
}

//============================================================================
struct NetWaitOp
{
	ENetError	result;
	bool		complete;
};

static void WaitOpCallback(
	ENetError	result,
	void *		param
) {
	NetWaitOp * op = (NetWaitOp *)param;

	op->result = result;
	op->complete = true;
}

//============================================================================
// CreateScore
//============================================================================
struct CreateScoreOp : NetWaitOp
{
	pfGameScore * score;
};

static void CreateScoreCallback(
	ENetError	result,
	void *		param,
	unsigned	scoreId,
	UInt32		createdTime,
	unsigned	ownerId,
	const char*	gameName,
	unsigned	gameType,
	int			value
) {
	CreateScoreOp * op = (CreateScoreOp*)param;
	op->result = result;

	if (IS_NET_SUCCESS(result)) {
		op->score->Init(
			scoreId,
			ownerId,
			createdTime,
			gameName,
			gameType,
			value
		);
	}
	else
		op->score->scoreId = 0;

	op->complete = true;
}

ENetError pfGameScoreMgr::CreateScore(
	unsigned		ownerId,
	const char*		gameName,
	unsigned		gameType,
	int				value,
	pfGameScore&	score
) {
	CreateScoreOp param;
	MemZero(&param, sizeof(CreateScoreOp));
	param.score = &score;

	NetCliAuthScoreCreate(
		ownerId,
		gameName,
		gameType,
		value,
		CreateScoreCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	return param.result;
}

//============================================================================
// DeleteScore
//============================================================================
ENetError pfGameScoreMgr::DeleteScore(
	unsigned scoreId
) {
	NetWaitOp param;
	MemZero(&param, sizeof(NetWaitOp));

	NetCliAuthScoreDelete(
		scoreId,
		WaitOpCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	return param.result;
}

//============================================================================
// GetScores
//============================================================================
struct GetScoresOp : NetWaitOp
{
	pfGameScore***	scores;
	int*			scoreCount;
};

static void GetScoresCallback(
	ENetError			result,
	void *				param,
	const NetGameScore	scores[],
	unsigned			scoreCount
) {
	GetScoresOp * op = (GetScoresOp*)param;
	op->result = result;

	if (IS_NET_SUCCESS(result)) {
		*(op->scores) = TRACKED_NEW pfGameScore*[scoreCount];
		*(op->scoreCount) = scoreCount;

		for (int i = 0; i < scoreCount; ++i) {
			pfGameScore* score = TRACKED_NEW pfGameScore();
			score->IncRef();

			char tempGameName[kMaxGameScoreNameLength];
			StrToAnsi(tempGameName, scores[i].gameName, arrsize(tempGameName));

			score->Init(
				scores[i].scoreId,
				scores[i].ownerId,
				scores[i].createdTime,
				tempGameName,
				scores[i].gameType,
				scores[i].value
			);

			(*op->scores)[i] = score;
		}
	}
	else {
		*(op->scores) = nil;
		op->scoreCount = 0;
	}

	op->complete = true;
}

ENetError pfGameScoreMgr::GetScoresIncRef(
	unsigned		ownerId,
	const char*		gameName,
	pfGameScore**&	outScoreList,
	int&			outScoreListCount
) {
	GetScoresOp param;
	MemZero(&param, sizeof(GetScoresOp));
	param.scores		= &outScoreList;
	param.scoreCount	= &outScoreListCount;

	NetCliAuthScoreGetScores(
		ownerId,
		gameName,
		GetScoresCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	return param.result;
}

//============================================================================
// AddPoints
//============================================================================
ENetError pfGameScoreMgr::AddPoints(
	unsigned	scoreId,
	int			numPoints
) {
	NetWaitOp param;
	MemZero(&param, sizeof(NetWaitOp));

	NetCliAuthScoreAddPoints(
		scoreId,
		numPoints,
		WaitOpCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	if (IS_NET_SUCCESS(param.result)) {
		if (GameScoreLink * link = fScores.Find(scoreId)) {
			link->score->value += numPoints;
		}
	}

	return param.result;
}

//============================================================================
// TransferPoints
//============================================================================
ENetError pfGameScoreMgr::TransferPoints(
	unsigned	srcScoreId,
	unsigned	destScoreId,
	int			numPoints
) {
	NetWaitOp param;
	MemZero(&param, sizeof(NetWaitOp));

	NetCliAuthScoreTransferPoints(
		srcScoreId,
		destScoreId,
		numPoints,
		WaitOpCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	if (IS_NET_SUCCESS(param.result)) {
		if (GameScoreLink * link = fScores.Find(srcScoreId)) {
			link->score->value -= numPoints;
		}
		if (GameScoreLink * link = fScores.Find(destScoreId)) {
			link->score->value += numPoints;
		}
	}

	return param.result;
}

//============================================================================
// SetPoints
//============================================================================
ENetError pfGameScoreMgr::SetPoints(
	unsigned	scoreId,
	int			numPoints
) {
	NetWaitOp param;
	MemZero(&param, sizeof(NetWaitOp));

	NetCliAuthScoreSetPoints(
		scoreId,
		numPoints,
		WaitOpCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	if (IS_NET_SUCCESS(param.result)) {
		if (GameScoreLink * link = fScores.Find(scoreId)) {
			link->score->value = numPoints;
		}
	}

	return param.result;
}

//============================================================================
// GetRankList
//============================================================================

struct GetRanksOp : NetWaitOp
{
	NetGameRank***	ranks;
	int*			rankCount;
};

static void GetRanksCallback(
	ENetError			result,
	void *				param,
	const NetGameRank	ranks[],
	unsigned			rankCount
) {
	GetRanksOp * op = (GetRanksOp*)param;
	op->result = result;

	if (IS_NET_SUCCESS(result)) {
		*(op->ranks) = TRACKED_NEW NetGameRank*[rankCount];
		*(op->rankCount) = rankCount;

		for (int i = 0; i < rankCount; ++i) {
			NetGameRank * rank = TRACKED_NEW NetGameRank;

			rank->rank	= ranks[i].rank;
			rank->score	= ranks[i].score;
			StrCopy(rank->name, ranks[i].name, arrsize(rank->name));

			(*op->ranks)[i] = rank;
		}
	}
	else {
		*(op->ranks) = nil;
		op->rankCount = 0;
	}

	op->complete = true;
}

ENetError pfGameScoreMgr::GetRankList(
	unsigned		ownerId,
	unsigned		scoreGroup,
	unsigned		parentFolderId,
	const char *	gameName,
	unsigned		timePeriod,
	unsigned		numResults,
	unsigned		pageNumber,
	bool			sortDesc,
	NetGameRank**&	outRankList,
	int&			outRankListCount
) {
	GetRanksOp param;
	MemZero(&param, sizeof(GetRanksOp));
	param.ranks		= &outRankList;
	param.rankCount	= &outRankListCount;

	NetCliAuthScoreGetRankList(
		ownerId,
		scoreGroup,
		parentFolderId,
		gameName,
		timePeriod,
		numResults,
		pageNumber,
		sortDesc,
		GetRanksCallback,
		&param
	);

	while (!param.complete) {
		NetClientUpdate();
		AsyncSleep(10);
	}

	return param.result;
}
