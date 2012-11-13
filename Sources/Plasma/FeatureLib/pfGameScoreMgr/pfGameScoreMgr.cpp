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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
#include "pfGameScoreMgr.h"
#include "pfMessage/pfGameScoreMsg.h"
#include "plNetGameLib/plNetGameLib.h"
#include "pnNetProtocol/pnNetProtocol.h"

struct ScoreFindParam
{
    uint32_t fOwnerId;
    plString fName;
    plKey fReceiver; // because plKey as a void* isn't cool

    ScoreFindParam(uint32_t ownerId, plString name, plKey r)
        : fOwnerId(ownerId), fName(name), fReceiver(r)
    { }
};

struct ScoreTransferParam
{
    pfGameScore* fTo;
    pfGameScore* fFrom;
    int32_t      fPoints;
    plKey        fReceiver;

    ScoreTransferParam(pfGameScore* to, pfGameScore* from, int32_t points, plKey r)
        : fTo(to), fFrom(from), fPoints(points), fReceiver(r)
    { }
};

struct ScoreUpdateParam
{
    pfGameScore* fParent;
    plKey        fReceiver;
    int32_t      fPoints; // reset points to this if update op

    ScoreUpdateParam(pfGameScore* s, plKey r, int32_t points = 0)
        : fParent(s), fReceiver(r), fPoints(points)
    { }
};

//======================================
static void OnScoreSet(ENetError result, void* param)
{
    ScoreUpdateParam* p = (ScoreUpdateParam*)param;
    pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, p->fParent, p->fPoints);
    msg->Send(p->fReceiver);
    delete p;
}

void pfGameScore::SetPoints(int32_t value, plKey rcvr)
{
    Ref(); // netcode holds us
    NetCliAuthScoreSetPoints(fScoreId, value, OnScoreSet, new ScoreUpdateParam(this, rcvr, value));
}

//======================================
static void OnScoreUpdate(ENetError result, void* param)
{
    ScoreUpdateParam* p = (ScoreUpdateParam*)param;
    pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, p->fParent, p->fPoints);
    msg->Send(p->fReceiver);
    delete p;
}

void pfGameScore::AddPoints(int32_t add, plKey rcvr)
{
    Ref(); // netcode holds us
    NetCliAuthScoreAddPoints(fScoreId, add, OnScoreUpdate, new ScoreUpdateParam(this, rcvr, fValue + add));
}

//======================================
void pfGameScore::Delete()
{
    NetCliAuthScoreDelete(fScoreId, nil, nil); // who cares about getting a notify here?
    UnRef(); // kthxbai
}

//======================================
static void OnScoreTransfer(ENetError result, void* param)
{
    ScoreTransferParam* p = (ScoreTransferParam*)param;
    pfGameScoreTransferMsg* msg = new pfGameScoreTransferMsg(result, p->fTo, p->fFrom, p->fPoints);
    msg->Send(p->fReceiver);
    delete p;
}

void pfGameScore::TransferPoints(pfGameScore* to, int32_t points, plKey recvr)
{
    this->Ref(); to->Ref(); // netcode holds us
    NetCliAuthScoreTransferPoints(this->fScoreId, to->fScoreId, points,
        OnScoreTransfer, new ScoreTransferParam(to, this, points, recvr));
}

//======================================
static void OnScoreCreate(
    ENetError   result,
    void *      param,
    uint32_t    scoreId,
    uint32_t    createdTime, // ignored
    uint32_t    ownerId,
    const char* gameName,
    uint32_t    gameType,
    int32_t     value
) {
    ScoreUpdateParam* p = (ScoreUpdateParam*)param;
    pfGameScore* score = new pfGameScore(scoreId, ownerId, gameName, gameType, value);
    pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, score, value);
    msg->Send(p->fReceiver);
    delete p;
}

void pfGameScore::Create(uint32_t ownerId, const plString& name, uint32_t type, int32_t value, plKey rcvr)
{
    NetCliAuthScoreCreate(ownerId, name.c_str(), type, value, OnScoreCreate, new ScoreUpdateParam(nil, rcvr));
}

//======================================
static void OnScoreFound(
    ENetError           result,
    void *              param,
    const NetGameScore  scores[],
    uint32_t            scoreCount
) {
    std::vector<pfGameScore*> vec(scoreCount);
    for (uint32_t i = 0; i < scoreCount; ++i)
    {
        const NetGameScore ngs = scores[i];
        vec[i] = new pfGameScore(ngs.scoreId, ngs.ownerId, plString::FromWchar(ngs.gameName), ngs.gameType, ngs.value);
    }

    ScoreFindParam* p = (ScoreFindParam*)param;
    pfGameScoreListMsg* msg = new pfGameScoreListMsg(result, vec, p->fOwnerId, p->fName);
    msg->Send(p->fReceiver);
    delete p;
}

void pfGameScore::Find(uint32_t ownerId, const plString& name, plKey rcvr)
{
    NetCliAuthScoreGetScores(ownerId, name.c_str(), OnScoreFound, new ScoreFindParam(ownerId, name, rcvr));
}
