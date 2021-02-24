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

#ifndef plEventCallbackMsg_inc
#define plEventCallbackMsg_inc

#include "plMessage.h"

enum CallbackEvent
{
    kStart = 0,
    kStop,
    kReverse,
    kTime,
    kLoop,
    kBegin,
    kEnd,
    kEventEnd,
    kSingleFrameAdjust,
    kSingleFrameEval
};

class plEventCallbackMsg : public plMessage
{
protected:

public:
    float           fEventTime; // the time for time events
    CallbackEvent   fEvent;     // the event    
    int16_t         fIndex;     // the index of the object we want the event to come from
                                // (where applicable, required for sounds)
    int16_t         fRepeats;   // -1 for infinite repeats, 0 for one call, no repeats
    int16_t         fUser;      // User defined data, useful for keeping track of multiple callbacks

    plEventCallbackMsg() : fEventTime(0.0f), fEvent((CallbackEvent)0), fRepeats(-1), fUser(0), fIndex(0) { }
    plEventCallbackMsg (const plKey &s, 
                    const plKey &r, 
                    const double* t) : 
                        plMessage(s, r, t),
                        fEventTime(0.0f), fEvent((CallbackEvent)0), fRepeats(-1), fUser(0), fIndex(0) { }

    plEventCallbackMsg(const plKey &receiver, CallbackEvent e, int idx=0, float t=0, int16_t repeats=-1, uint16_t user=0) :
                        plMessage(nullptr, receiver, nullptr), fEvent(e), fIndex(idx), fEventTime(t), fRepeats(repeats), fUser(user) { }

    ~plEventCallbackMsg() { }

    CLASSNAME_REGISTER(plEventCallbackMsg);
    GETINTERFACE_ANY(plEventCallbackMsg, plMessage);

    // IO
    void Read(hsStream* stream, hsResMgr* mgr) override;
    void Write(hsStream* stream, hsResMgr* mgr) override;
};

// For when you want to send callbacks, but someone other than the sender/receiver
// needs to modify them along the way.
class plEventCallbackInterceptMsg : public plEventCallbackMsg
{
protected:
    plMessage *fMsg;

public:
    plMessage* GetMessageNoRef() const { return fMsg; }
    void SetMessageRef(plMessage *msg)
    {
        hsRefCnt_SafeUnRef(fMsg);
        hsRefCnt_SafeRef(msg);
        fMsg = msg;
    }

    void SendMessageAndKeep()
    {
        if (fMsg)
            fMsg->SendAndKeep();
    }

    plEventCallbackInterceptMsg() : plEventCallbackMsg(), fMsg() { }
    ~plEventCallbackInterceptMsg() { hsRefCnt_SafeUnRef(fMsg); fMsg = nullptr; }

    CLASSNAME_REGISTER(plEventCallbackInterceptMsg);
    GETINTERFACE_ANY(plEventCallbackInterceptMsg, plEventCallbackMsg);
};


#endif // plEventCallbackMsg_inc
