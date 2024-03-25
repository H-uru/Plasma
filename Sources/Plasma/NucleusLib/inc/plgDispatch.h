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

#ifndef plDispatchBase_inc
#define plDispatchBase_inc

#include "pnFactory/plCreatable.h"

class plMessage;
class plKey;

class plDispatchBase : public plCreatable
{
public:
    CLASSNAME_REGISTER(plDispatchBase);
    GETINTERFACE_ANY(plDispatchBase, plCreatable);

    virtual void RegisterForType(uint16_t hClass, const plKey& receiver) = 0;
    virtual void RegisterForExactType(uint16_t hClass, const plKey& receiver) = 0;

    virtual void UnRegisterForType(uint16_t hClass, const plKey& receiver) = 0;
    virtual void UnRegisterForExactType(uint16_t hClass, const plKey& receiver) = 0;

    virtual void UnRegisterAll(const plKey& receiver) = 0;

    virtual bool MsgSend(plMessage* msg, bool async=false) = 0;

    // Used by other thread to Send Messages, they are handled as soon as
    // Practicable
    virtual void MsgQueue(plMessage* msg)=0;
    virtual void MsgQueueProcess() = 0;

    // Turn on or off Queued Messages, if off, uses MsgSend Immediately (for
    // plugins)
    virtual void MsgQueueOnOff(bool) = 0;

    // On starts deferring msg delivery until buffering is set to off again.
    virtual bool SetMsgBuffering(bool on) = 0;

    virtual void BeginShutdown() = 0;
};

class plgDispatch
{
public:
    static plDispatchBase* Dispatch();
    static bool MsgSend(plMessage* msg, bool async = false) {
        return Dispatch()->MsgSend(msg, async);
    }
};


#endif // plDispatchBase_inc
