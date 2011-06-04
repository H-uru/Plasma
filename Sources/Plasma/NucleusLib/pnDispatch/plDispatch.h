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
#include "../pnKeyedObject/hsKeyedObject.h"

#pragma warning(disable: 4284)

class hsResMgr;
class plMessage;
class plKey;

class plTypeFilter
{
public:
	plTypeFilter() : fHClass(0) {}

	UInt16				fHClass;
	hsTArray<plKey>		fReceivers;
};

class plMsgWrap;

typedef void (*MsgRecieveCallback)();

class plDispatch : public plDispatchBase
{
protected:

	hsKeyedObject*					fOwner;

	plMsgWrap*						fFutureMsgQueue;
	static Int32					fNumBufferReq;
	static plMsgWrap*				fMsgCurrent;
	static hsMutex					fMsgCurrentMutex; // mutex for above
	static hsMutex					fMsgDispatchLock;	// mutex for IMsgDispatch
	static plMsgWrap*				fMsgHead;
	static plMsgWrap*				fMsgTail;
	static hsBool					fMsgActive;
	static hsTArray<plMessage*>		fMsgWatch;
	static MsgRecieveCallback		fMsgRecieveCallback;

	hsTArray<plTypeFilter*>			fRegisteredExactTypes;
	std::list<plMessage*>			fQueuedMsgList;
	hsMutex							fQueuedMsgListMutex; // mutex for above
	hsBool							fQueuedMsgOn;		// Turns on or off Queued Messages, Plugins need them off

	hsKeyedObject*					IGetOwner() { return fOwner; }
	plKey							IGetOwnerKey() { return IGetOwner() ? IGetOwner()->GetKey() : nil; }
	int								IFindType(UInt16 hClass);
	int								IFindSender(const plKey& sender);
	hsBool							IUnRegisterForExactType(int idx, const plKey& receiver);

	static plMsgWrap*				IInsertToQueue(plMsgWrap** back, plMsgWrap* isert);
	static plMsgWrap*				IDequeue(plMsgWrap** head, plMsgWrap** tail);

	hsBool							IMsgNetPropagate(plMessage* msg);

	static void						IMsgDispatch();
	static void						IMsgEnqueue(plMsgWrap* msgWrap, hsBool async);

	hsBool							ISortToDeferred(plMessage* msg);
	void							ICheckDeferred(double stamp);
	hsBool							IListeningForExactType(UInt16 hClass);

	void							ITrashUndelivered(); // Just pitches them, doesn't try to deliver.

public:
	plDispatch();
	virtual ~plDispatch();

	CLASSNAME_REGISTER( plDispatch );
	GETINTERFACE_ANY( plDispatch, plCreatable );

	virtual void RegisterForType(UInt16 hClass, const plKey& receiver);
	virtual void RegisterForExactType(UInt16 hClass, const plKey& receiver);

	virtual void UnRegisterForType(UInt16 hClass, const plKey& receiver);
	virtual void UnRegisterForExactType(UInt16 hClass, const plKey& receiver);

	virtual void UnRegisterAll(const plKey& receiver);

	virtual hsBool	MsgSend(plMessage* msg, hsBool async=false);
	virtual void	MsgQueue(plMessage* msg);	// Used by other thread to Send Messages, they are handled as soon as Practicable
	virtual void	MsgQueueProcess();
	virtual void	MsgQueueOnOff(hsBool );		// Turn on or off Queued Messages, if off, uses MsgSend Immediately

	virtual hsBool	SetMsgBuffering(hsBool on); // On starts deferring msg delivery until buffering is set to off again.

	static void SetMsgRecieveCallback(MsgRecieveCallback callback) { fMsgRecieveCallback = callback; }
};

class plNullDispatch : public plDispatch
{
public:

	virtual void RegisterForExactType(UInt16 hClass, const plKey& receiver) {}
	virtual void RegisterForType(UInt16 hClass, const plKey& receiver) {}

	virtual void UnRegisterForExactType(UInt16 hClass, const plKey& receiver) {}
	virtual void UnRegisterForType(UInt16 hClass, const plKey& receiver) {}


	virtual hsBool MsgSend(plMessage* msg) {}
	virtual void	MsgQueue(plMessage* msg){}
	virtual void	MsgQueueProcess(){}

};

#endif // plDispatch_inc
