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

#include "pyGameScoreMsg.h"

#include <string_theory/string>

#include "pfMessage/pfGameScoreMsg.h"
#include "pyGameScore.h"

PyObject* pyGameScoreMsg::CreateFinal(pfGameScoreMsg* msg)
{
    if (pfGameScoreListMsg* l = pfGameScoreListMsg::ConvertNoRef(msg))
        return pyGameScoreListMsg::New(l);
    if (pfGameScoreTransferMsg* t = pfGameScoreTransferMsg::ConvertNoRef(msg))
        return pyGameScoreTransferMsg::New(t);
    if (pfGameScoreUpdateMsg* u = pfGameScoreUpdateMsg::ConvertNoRef(msg))
        return pyGameScoreUpdateMsg::New(u);
    return nullptr;
}

ST::string pyGameScoreMsg::GetError() const
{
    if (fMsg)
        return ST::string::from_wchar(NetErrorToString(fMsg->GetResult()));
    return ST_LITERAL("pfGameScoreMsg is NULL");
}

bool pyGameScoreMsg::IsValid() const
{
    if (fMsg)
        return IS_NET_SUCCESS(fMsg->GetResult());
    return false;
}

ST::string pyGameScoreListMsg::GetName() const
{
    if (pfGameScoreListMsg* pList = pfGameScoreListMsg::ConvertNoRef(fMsg))
        return pList->GetName();
    return ST::string();
}

uint32_t pyGameScoreListMsg::GetOwnerID() const
{
    if (pfGameScoreListMsg* pList = pfGameScoreListMsg::ConvertNoRef(fMsg))
        return pList->GetOwnerID();
    return 0;
}

size_t pyGameScoreListMsg::GetNumScores() const
{
    if (pfGameScoreListMsg* pList = pfGameScoreListMsg::ConvertNoRef(fMsg))
        return pList->GetNumScores();
    return 0;
}

PyObject* pyGameScoreListMsg::GetScore(size_t idx) const
{
    if (pfGameScoreListMsg* pList = pfGameScoreListMsg::ConvertNoRef(fMsg))
        return pyGameScore::New(pList->GetScore(idx));
    return nullptr;
}

PyObject* pyGameScoreTransferMsg::GetDestinationScore() const
{
    if (pfGameScoreTransferMsg* pTrans = pfGameScoreTransferMsg::ConvertNoRef(fMsg))
        return pyGameScore::New(pTrans->GetDestination());
    return nullptr;
}

PyObject* pyGameScoreTransferMsg::GetSourceScore() const
{
    if (pfGameScoreTransferMsg* pTrans = pfGameScoreTransferMsg::ConvertNoRef(fMsg))
        return pyGameScore::New(pTrans->GetSource());
    return nullptr;
}

PyObject* pyGameScoreUpdateMsg::GetScore() const
{
    if (pfGameScoreUpdateMsg* pUp = pfGameScoreUpdateMsg::ConvertNoRef(fMsg))
        return pyGameScore::New(pUp->GetScore());
    return nullptr;
}
