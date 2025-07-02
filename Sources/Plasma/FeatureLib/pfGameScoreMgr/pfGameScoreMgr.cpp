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

#include <string_theory/string>

#include "pnNetProtocol/pnNetProtocol.h"

#include "plNetGameLib/plNetGameLib.h"

#include "pfMessage/pfGameScoreMsg.h"

void pfGameScore::SetPoints(int32_t value, plKey rcvr)
{
    Ref(); // netcode holds us
    NetCliAuthScoreSetPoints(
        fScoreId, value,
        [this, value, rcvr = std::move(rcvr)](auto result) {
            pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, this, value);
            msg->Send(rcvr);
        }
    );
}

void pfGameScore::AddPoints(int32_t add, plKey rcvr)
{
    Ref(); // netcode holds us
    NetCliAuthScoreAddPoints(
        fScoreId, add,
        [this, newValue = fValue + add, rcvr = std::move(rcvr)](auto result) {
            pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, this, newValue);
            msg->Send(rcvr);
        }
    );
}

void pfGameScore::Delete()
{
    NetCliAuthScoreDelete(fScoreId, [](auto result) {}); // who cares about getting a notify here?
    UnRef(); // kthxbai
}

void pfGameScore::TransferPoints(pfGameScore* to, int32_t points, plKey rcvr)
{
    this->Ref(); to->Ref(); // netcode holds us
    NetCliAuthScoreTransferPoints(
        this->fScoreId, to->fScoreId, points,
        [this, to, points, rcvr = std::move(rcvr)](auto result) {
            pfGameScoreTransferMsg* msg = new pfGameScoreTransferMsg(result, to, this, points);
            msg->Send(rcvr);
        }
    );
}

void pfGameScore::Create(uint32_t ownerId, const ST::string& name, uint32_t type, int32_t value, const plKey& rcvr)
{
    NetCliAuthScoreCreate(
        ownerId, name, type, value,
        [ownerId, name, type, value, rcvr](auto result, auto scoreId, auto createdTime) {
            pfGameScore* score = new pfGameScore(scoreId, ownerId, name, type, value);
            pfGameScoreUpdateMsg* msg = new pfGameScoreUpdateMsg(result, score, value);
            msg->Send(rcvr);
        }
    );
}

static void OnScoreFound(
    uint32_t            ownerId,
    const ST::string&   name,
    const plKey&        rcvr,
    ENetError           result,
    const std::vector<NetGameScore>& scores
) {
    std::vector<pfGameScore*> vec(scores.size());
    for (size_t i = 0; i < scores.size(); ++i) {
        const NetGameScore& ngs = scores[i];
        vec[i] = new pfGameScore(ngs.scoreId, ngs.ownerId, ngs.gameName, ngs.gameType, ngs.value);
    }

    pfGameScoreListMsg* msg = new pfGameScoreListMsg(result, vec, ownerId, name);
    msg->Send(rcvr);
}

void pfGameScore::Find(uint32_t ownerId, const ST::string& name, const plKey& rcvr)
{
    NetCliAuthScoreGetScores(
        ownerId, name,
        [ownerId, name, rcvr](auto result, const auto& scores) {
            OnScoreFound(ownerId, name, rcvr, result, scores);
        }
    );
}

void pfGameScore::FindHighScores(uint32_t ageId, uint32_t maxScores, const ST::string& name, const plKey& rcvr)
{
    // This functionality is only supported by next-gen servers
    if (NetCliAuthCheckCap(kCapsScoreLeaderBoards)) {
        NetCliAuthScoreGetHighScores(
            ageId, maxScores, name,
            [ageId, name, rcvr](auto result, const auto& scores) {
                OnScoreFound(ageId, name, rcvr, result, scores);
            }
        );
    } else {
        pfGameScoreListMsg* msg = new pfGameScoreListMsg(kNetErrNotSupported, ageId, name);
        msg->Send(rcvr);
    }
}
