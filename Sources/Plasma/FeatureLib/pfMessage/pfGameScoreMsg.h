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

#ifndef _pfGameScoreMsg_h_
#define _pfGameScoreMsg_h_

#include "HeadSpin.h"

#include "pnMessage/plMessage.h"
#include "pnNetBase/pnNetBase.h"

#include <string_theory/string>
#include <vector>

class pfGameScore;

class pfGameScoreMsg : public plMessage
{
    ENetError fResult;

public:
    pfGameScoreMsg() : fResult() { }
    pfGameScoreMsg(ENetError result) : fResult(result) { }

    CLASSNAME_REGISTER(pfGameScoreMsg);
    GETINTERFACE_ANY(pfGameScoreMsg, plMessage);

    ENetError GetResult() const { return fResult; }

    void Read(hsStream*, hsResMgr*) override { FATAL("wtf are you doing???"); }
    void Write(hsStream*, hsResMgr*) override { FATAL("wtf are you doing???"); }
};

class pfGameScoreListMsg : public pfGameScoreMsg
{
    std::vector<pfGameScore*> fScores;
    uint32_t fOwnerId;
    ST::string fName;

public:
    pfGameScoreListMsg() : fOwnerId() { }
    pfGameScoreListMsg(ENetError result, uint32_t ownerId, const ST::string& name)
        : pfGameScoreMsg(result), fOwnerId(ownerId), fName(name)
    { }
    pfGameScoreListMsg(ENetError result, std::vector<pfGameScore*> vec, uint32_t ownerId, ST::string name)
        : fScores(std::move(vec)), pfGameScoreMsg(result), fOwnerId(ownerId), fName(std::move(name))
    { }

    ~pfGameScoreListMsg();

    CLASSNAME_REGISTER(pfGameScoreListMsg);
    GETINTERFACE_ANY(pfGameScoreListMsg, pfGameScoreMsg);

    ST::string GetName() const { return fName; }
    uint32_t GetOwnerID() const { return fOwnerId; }
    size_t GetNumScores() const { return fScores.size(); }
    pfGameScore* GetScore(size_t idx) const { return fScores.at(idx); }
};

class pfGameScoreTransferMsg : public pfGameScoreMsg
{
    pfGameScore* fSource;
    pfGameScore* fDestination;

public:
    pfGameScoreTransferMsg() : fSource(), fDestination() { }
    pfGameScoreTransferMsg(ENetError result, pfGameScore* to, pfGameScore* from, int32_t points);

    ~pfGameScoreTransferMsg();

    CLASSNAME_REGISTER(pfGameScoreTransferMsg);
    GETINTERFACE_ANY(pfGameScoreTransferMsg, pfGameScoreMsg);

    pfGameScore* GetDestination() const { return fDestination; }
    pfGameScore* GetSource() const { return fSource; }
};

class pfGameScoreUpdateMsg : public pfGameScoreMsg
{
    pfGameScore* fScore;

public:
    pfGameScoreUpdateMsg() : fScore() { }
    pfGameScoreUpdateMsg(ENetError result, pfGameScore* s, int32_t points);

    ~pfGameScoreUpdateMsg();

    CLASSNAME_REGISTER(pfGameScoreUpdateMsg);
    GETINTERFACE_ANY(pfGameScoreUpdateMsg, pfGameScoreMsg);

    pfGameScore* GetScore() const { return fScore; }
};

#endif // _pfGameScoreMsg_h_
