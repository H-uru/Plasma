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
#include "pyGameScore.h"

#include "../pfGameScoreMgr/pfGameScoreMgr.h"

pyGameScore::pyGameScore() : fScore(nil)
{
}

pyGameScore::pyGameScore(pfGameScore * score) : fScore(score)
{
	fScore->IncRef();
}

pyGameScore::~pyGameScore()
{
	if (fScore)
		fScore->DecRef();
}

int pyGameScore::GetScoreID()
{
	if (fScore)
		return fScore->scoreId;

	return 0;
}

UInt32 pyGameScore::GetCreatedTime()
{
	if (fScore)
		return fScore->createdTime;

	return 0;
}

int pyGameScore::GetOwnerID()
{
	if (fScore)
		return fScore->ownerId;

	return 0;
}

int pyGameScore::GetGameType()
{
	if (fScore)
		return fScore->gameType;

	return 0;
}

int pyGameScore::GetValue()
{
	if (fScore)
		return fScore->value;

	return 0;
}

const char* pyGameScore::GetGameName()
{
	if (fScore)
		return fScore->gameName;

	return "";
}

bool pyGameScore::AddPoints(int numPoints)
{
	ENetError result = kNetErrScoreWrongType;

	if (fScore && fScore->gameType != kScoreTypeFixed)
		result = pfGameScoreMgr::GetInstance()->AddPoints(fScore->scoreId, numPoints);

	return IS_NET_SUCCESS(result);
}

bool pyGameScore::TransferPoints(unsigned destination, int numPoints)
{
	ENetError result = kNetErrScoreWrongType;

	if (fScore && fScore->gameType != kScoreTypeFixed)
		result = pfGameScoreMgr::GetInstance()->TransferPoints(fScore->scoreId, destination, numPoints);

	return IS_NET_SUCCESS(result);
}

bool pyGameScore::SetPoints(int numPoints)
{
	ENetError result = kNetErrScoreWrongType;

	if (fScore && fScore->gameType != kScoreTypeFixed)
		result = pfGameScoreMgr::GetInstance()->SetPoints(fScore->scoreId, numPoints);

	return IS_NET_SUCCESS(result);
}
