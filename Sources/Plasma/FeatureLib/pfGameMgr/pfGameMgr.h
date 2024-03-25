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

#ifndef _pfGameMgr_h_
#define _pfGameMgr_h_

#include "HeadSpin.h"
#include "hsRefCnt.h"

#include <atomic>
#include <map>
#include <memory>
#include <type_traits>

#include "pfGameMgrTrans.h"

class pfGameMgr
{
    std::atomic<uint32_t> fTransId;
    std::map<uint32_t, std::unique_ptr<pfGameMgrTrans>> fTransactions;
    std::map<uint32_t, hsRef<pfGameCli>> fGameClis;

    template<typename _MsgT>
    void Recv(const _MsgT& msg) = delete;

    static void RecvGameMgrMsg(struct GameMsgHeader* msg);

protected:
    friend class pfGameCli;

    template<typename _TransT, typename... _ArgsT>
    _TransT* CreateTransaction(_ArgsT&&... args)
    {
        uint32_t transId = fTransId++;
        auto [transIt, happned] = fTransactions.try_emplace(
            transId,
            std::make_unique<_TransT>(std::forward<_ArgsT>(args)...)
        );
        transIt->second->fTransId = transId;
        return (_TransT*)transIt->second.get();
    }

    template<typename _TransT, typename... _ArgsT>
    void SendTransaction(_ArgsT&&... args)
    {
        static_assert(std::is_base_of_v<pfGameMgrTrans, _TransT>, "Transactions must be subclasses of pfGameMgrTrans");

        _TransT* trans = CreateTransaction<_TransT>(std::forward<_ArgsT>(args)...);
        trans->Send();
    }

public:
    pfGameMgr();
    pfGameMgr(const pfGameMgr&) = delete;
    pfGameMgr(pfGameMgr&&) = delete;
    ~pfGameMgr();

public:
    static pfGameMgr* GetInstance();
    static void Shutdown();
};

#endif
