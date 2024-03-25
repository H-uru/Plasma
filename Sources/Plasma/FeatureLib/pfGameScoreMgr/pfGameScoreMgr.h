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

#ifndef _pfGameScoreMgr_h_
#define _pfGameScoreMgr_h_

#include "HeadSpin.h"
#include "hsRefCnt.h"

#include <string_theory/string>

#include "pnKeyedObject/plKey.h"
#include "pnNetBase/pnNetBase.h"

// TODO: Rank List (seems to be unused in regular gameplay though...)
//       That's some strange stuff...

/**
 * Plasma Game Score
 * Mid-level class that encapsulates game scores and sends pfGameScoreMsg notifications
 * via the dispatcher when network operations complete.
 */
class pfGameScore : public hsRefCnt
{
    uint32_t fScoreId;
    uint32_t fOwnerId;
    ST::string fName;
    uint32_t fGameType; // EGameScoreTypes
    int32_t  fValue;

    friend class pfGameScoreTransferMsg;
    friend class pfGameScoreUpdateMsg;

public:
    pfGameScore(uint32_t scoreId, uint32_t owner, const ST::string& name, uint32_t type, int32_t value = 0)
        : fScoreId(scoreId), fOwnerId(owner), fName(name), fGameType(type), fValue(value)
    { }

    ST::string GetGameName() const { return fName; }
    uint32_t GetGameType() const { return fGameType; }
    uint32_t GetOwner() const { return fOwnerId; }
    int32_t  GetPoints() const { return fValue; }
    void     SetPoints(int32_t value, plKey rcvr = {});

    void AddPoints(int32_t add, plKey rcvr = {});
    void Delete();
    void TransferPoints(pfGameScore* to, plKey rcvr = {}) { TransferPoints(to, fValue, std::move(rcvr)); }
    void TransferPoints(pfGameScore* to, int32_t points, plKey rcvr = {});

    static void Create(uint32_t ownerId, const ST::string& name, uint32_t type, int32_t value, const plKey& rcvr);
    static void Find(uint32_t ownerId, const ST::string& name, const plKey& rcvr);
    static void FindHighScores(uint32_t ageId, uint32_t maxScores, const ST::string& name, const plKey& rcvr);
};

#endif // _pfGameScoreMgr_h_
