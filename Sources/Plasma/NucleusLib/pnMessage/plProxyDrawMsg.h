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

#ifndef plProxyDrawMsg_inc
#define plProxyDrawMsg_inc

#include "plMessage.h"

class hsStream;
class hsResMgr;

// Proxy Draw Msg's are sent out to tell
// proxies for the visual objects in the world to
// make themselves visible (or reclaim resources used
// to make themselves visible). This message should only
// be sent out by the core system.
class plProxyDrawMsg : public plMessage
{
protected:
    uint16_t      fProxyFlags;

public:
    plProxyDrawMsg();
    plProxyDrawMsg(uint16_t flags); // for broadcast
    plProxyDrawMsg(plKey &rcv, uint16_t flags); // send yourself an ack
    ~plProxyDrawMsg();

    CLASSNAME_REGISTER(plProxyDrawMsg);
    GETINTERFACE_ANY(plProxyDrawMsg, plMessage);

    enum {
        kCreate         = 0x1,
        kDestroy        = 0x2,
        kDetached       = 0x4,
        kToggle         = 0x8,

        kLight          = 0x10,
        kPhysical       = 0x20,
        kOccluder       = 0x40,
        kAudible        = 0x80,
        kCoordinate     = 0x100,
        kCamera         = 0x200,

        kAllTypes       = kLight
                        | kPhysical
                        | kOccluder
                        | kAudible
                        | kCoordinate
                        | kCamera
    };

    uint16_t  GetProxyFlags() const { return fProxyFlags; }
    void    SetProxyFlags(uint16_t f) { fProxyFlags = f; }

    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

#endif // plProxyDrawMsg_inc
