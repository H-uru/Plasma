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

#include "plNglCore.h"

#include <utility>

#include "Intern.h"

namespace Ngl {
/*****************************************************************************
*
*   Private
*
***/


struct ReportNetErrorTrans : NetNotifyTrans {
    ENetProtocol        m_errProtocol;
    ENetError           m_errError;

    ReportNetErrorTrans (
        ENetProtocol    errProtocol,
        ENetError       errError
    );

    void Post() override;
};


/*****************************************************************************
*
*   Private data
*
***/

static NetClientErrorFunc   s_errorFunc;
static std::atomic<long>    s_initCount;


/*****************************************************************************
*
*   Local functions
*
***/


/*****************************************************************************
*
*   Transactions
*
***/

//============================================================================
// NetNotifyTrans
//============================================================================
NetNotifyTrans::NetNotifyTrans (ETransType transType)
:   NetTrans(kNetProtocolNil, transType)
{
}

//============================================================================
// ReportNetErrorTrans
//============================================================================
ReportNetErrorTrans::ReportNetErrorTrans (
    ENetProtocol    errProtocol,
    ENetError       errError
) : NetNotifyTrans(kReportNetErrorTrans)
,   m_errProtocol(errProtocol)
,   m_errError(errError)
{ }

//============================================================================
void ReportNetErrorTrans::Post () {
    if (s_errorFunc)
        s_errorFunc(m_errProtocol, m_errError);
}


/*****************************************************************************
*
*   Module functions
*
***/

//============================================================================
void ReportNetError (ENetProtocol protocol, ENetError error) {
    ReportNetErrorTrans * trans = new ReportNetErrorTrans(protocol, error);
    NetTransSend(trans);
}

} using namespace Ngl;


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
void NetClientInitialize () {
    
    if (0 == s_initCount.fetch_add(1)) {
        NetTransInitialize();
        AuthInitialize();
        GameInitialize();
        FileInitialize();
        GateKeeperInitialize();
    }
}

//============================================================================
void NetClientCancelAllTrans () {
    NetTransCancelAll(kNetErrTimeout);
}

//============================================================================
void NetClientDestroy (bool wait) {

    if (1 == s_initCount.fetch_sub(1)) {
        s_errorFunc = nullptr;

        GateKeeperDestroy(false);
        FileDestroy(false);
        GameDestroy(false);
        AuthDestroy(false);
        NetTransDestroy(false);
        if (wait) {
            GateKeeperDestroy(true);
            FileDestroy(true);
            GameDestroy(true);
            AuthDestroy(true);
            NetTransDestroy(true);
        }
    }
}

//============================================================================
void NetClientUpdate () {
    NetTransUpdate();
}

//============================================================================
void NetClientSetTransTimeoutMs (unsigned ms) {
    NetTransSetTimeoutMs(ms);
}

//============================================================================
void NetClientPingEnable (bool enable) {
    AuthPingEnable(enable);
    GamePingEnable(enable);
}

//============================================================================
void NetClientSetErrorHandler(NetClientErrorFunc errorFunc) {
    s_errorFunc = std::move(errorFunc);
}
