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

#ifndef plDispatch_inc
#define plDispatch_inc
#include "hsTemplates.h"
#include "hsStlUtils.h"
#include "plgDispatch.h"
#include "hsThread.h"
#include "pnKeyedObject/hsKeyedObject.h"

#pragma warning(disable: 4284)

class hsResMgr;
class plMessage;
class plKey;

class plTypeFilter
{
public:
    plTypeFilter() : fHClass(0) {}

    uint16_t              fHClass;
    hsTArray<plKey>     fReceivers;
};

class plMsgWrap;

typedef void (*MsgRecieveCallback)();

class plDispatch : public plDispatchBase
{
protected:

    hsKeyedObject*                  fOwner;

    plMsgWrap*                      fFutureMsgQueue;
    static int32_t                    fNumBufferReq;
    static plMsgWrap*               fMsgCurrent;
    static hsMutex                  fMsgCurrentMutex; // mutex for above
    static hsMutex                  fMsgDispatchLock;   // mutex for IMsgDispatch
    static plMsgWrap*               fMsgHead;
    static plMsgWrap*               fMsgTail;
    static bool                     fMsgActive;
    static hsTArray<plMessage*>     fMsgWatch;
    static MsgRecieveCallback       fMsgRecieveCallback;

    hsTArray<plTypeFilter*>         fRegisteredExactTypes;
    std::list<plMessage*>           fQueuedMsgList;
    hsMutex                         fQueuedMsgListMutex; // mutex for above
    bool                            fQueuedMsgOn;       // Turns on or off Queued Messages, Plugins need them off

    hsKeyedObject*                  IGetOwner() { return fOwner; }
    plKey                           IGetOwnerKey() { return IGetOwner() ? IGetOwner()->GetKey() : nil; }
    int                             IFindType(uint16_t hClass);
    int                             IFindSender(const plKey& sender);
    bool                            IUnRegisterForExactType(int idx, const plKey& receiver);

    static plMsgWrap*               IInsertToQueue(plMsgWrap** back, plMsgWrap* isert);
    static plMsgWrap*               IDequeue(plMsgWrap** head, plMsgWrap** tail);

    bool                            IMsgNetPropagate(plMessage* msg);

    static void                     IMsgDispatch();
    static void                     IMsgEnqueue(plMsgWrap* msgWrap, bool async);

    bool                            ISortToDeferred(plMessage* msg);
    void                            ICheckDeferred(double stamp);
    bool                            IListeningForExactType(uint16_t hClass);

    void                            ITrashUndelivered(); // Just pitches them, doesn't try to deliver.

public:
    plDispatch();
    virtual ~plDispatch();

    CLASSNAME_REGISTER( plDispatch );
    GETINTERFACE_ANY( plDispatch, plCreatable );

    virtual void RegisterForType(uint16_t hClass, const plKey& receiver);
    virtual void RegisterForExactType(uint16_t hClass, const plKey& receiver);

    virtual void UnRegisterForType(uint16_t hClass, const plKey& receiver);
    virtual void UnRegisterForExactType(uint16_t hClass, const plKey& receiver);

    virtual void UnRegisterAll(const plKey& receiver);

    virtual bool    MsgSend(plMessage* msg, bool async=false);
    virtual void    MsgQueue(plMessage* msg);   // Used by other thread to Send Messages, they are handled as soon as Practicable
    virtual void    MsgQueueProcess();
    virtual void    MsgQueueOnOff(bool );     // Turn on or off Queued Messages, if off, uses MsgSend Immediately

    virtual bool    SetMsgBuffering(bool on); // On starts deferring msg delivery until buffering is set to off again.

    static void SetMsgRecieveCallback(MsgRecieveCallback callback) { fMsgRecieveCallback = callback; }
};

class plNullDispatch : public plDispatch
{
public:

    virtual void RegisterForExactType(uint16_t hClass, const plKey& receiver) {}
    virtual void RegisterForType(uint16_t hClass, const plKey& receiver) {}

    virtual void UnRegisterForExactType(uint16_t hClass, const plKey& receiver) {}
    virtual void UnRegisterForType(uint16_t hClass, const plKey& receiver) {}


    virtual bool MsgSend(plMessage* msg) { return true; }
    virtual void    MsgQueue(plMessage* msg) {}
    virtual void    MsgQueueProcess() {}

};

#endif // plDispatch_inc
