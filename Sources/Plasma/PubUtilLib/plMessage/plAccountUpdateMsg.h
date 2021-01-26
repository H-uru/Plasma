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
#ifndef plAccountUpdateMsg_inc
#define plAccountUpdateMsg_inc

#include "pnMessage/plMessage.h"

class hsStream;
class hsResMgr;

class plAccountUpdateMsg : public plMessage
{
public:
    // If you update this enum, please update the python enum
    // located at the bottom of cyAccountManagementGlue.cpp
    enum
    {
        kCreatePlayer = 1,
        kDeletePlayer,
        kUpgradePlayer,
        kActivePlayer,
        kChangePassword,
    };

    plAccountUpdateMsg() : fUpdateType(0), fResult(0), fPlayerInt(0) { }
    plAccountUpdateMsg(uint32_t updateType) : fUpdateType(updateType), fResult(0), fPlayerInt(0) { }

    CLASSNAME_REGISTER(plAccountUpdateMsg);
    GETINTERFACE_ANY(plAccountUpdateMsg, plMessage);

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;

    uint32_t    GetUpdateType() const { return fUpdateType; }
    void        SetUpdateType(uint32_t type) { fUpdateType = type; }

    uint32_t    GetResult() const { return fResult; }
    void        SetResult(uint32_t result) { fResult = result; }

    uint32_t    GetPlayerInt() const { return fPlayerInt; }
    void        SetPlayerInt(uint32_t playerInt) { fPlayerInt = playerInt; }

private:
    uint32_t fUpdateType;
    uint32_t fResult;
    uint32_t fPlayerInt;
};

#endif // plAccountUpdateMsg_inc
