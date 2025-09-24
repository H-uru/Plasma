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

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCLIENTMGRMSG_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCLIENTMGRMSG_H

#include "pnMessage/plMessage.h"


class plNetClientMgrMsg : public plMessage {
public:
    enum {
        kNotifyRcvdAllSDLStates,
        kCmdDisableNet,
    };

    unsigned type;
    char str[256];
    bool yes;

    CLASSNAME_REGISTER(plNetClientMgrMsg);
    GETINTERFACE_ANY(plNetClientMgrMsg, plMessage);

    plNetClientMgrMsg(unsigned _type = 0, bool _yes = false, const char * _str = nullptr)
        : type(_type), yes(_yes)
    {
        if (_str) {
            strncpy(str, _str, std::size(str));
            str[std::size(str)-1] = 0;
        } else {
            memset(str, 0, sizeof(str));
        }
    }

    void Read (hsStream *, hsResMgr *) override { FATAL("plNetClientMgrMsg::Read"); }
    void Write (hsStream *, hsResMgr *) override { FATAL("plNetClientMgrMsg::Write"); }
};


#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLMESSAGE_PLNETCLIENTMGRMSG_H
