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

#ifndef plEventCallbackMsg_inc
#define plEventCallbackMsg_inc

#include "hsStream.h"
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
	hsScalar		fEventTime;	// the time for time events
	CallbackEvent	fEvent;		// the event	
	Int16			fIndex;		// the index of the object we want the event to come from
								// (where applicable, required for sounds)
	Int16			fRepeats;	// -1 for infinite repeats, 0 for one call, no repeats
	Int16			fUser;		// User defined data, useful for keeping track of multiple callbacks

	plEventCallbackMsg() : fEventTime(0.0f), fEvent((CallbackEvent)0), fRepeats(-1), fUser(0), fIndex(0) {;}
	plEventCallbackMsg (const plKey &s, 
					const plKey &r, 
					const double* t) : 
						plMessage(s, r, t),
						fEventTime(0.0f), fEvent((CallbackEvent)0), fRepeats(-1), fUser(0), fIndex(0) {;}
	
	plEventCallbackMsg(const plKey &receiver, CallbackEvent e, int idx=0, hsScalar t=0, Int16 repeats=-1, UInt16 user=0) :
						plMessage(nil, receiver, nil), fEvent(e), fIndex(idx), fEventTime(t), fRepeats(repeats), fUser(user) {}

	~plEventCallbackMsg(){;}

	CLASSNAME_REGISTER( plEventCallbackMsg );
	GETINTERFACE_ANY( plEventCallbackMsg, plMessage );

	// IO 
	virtual void Read(hsStream* stream, hsResMgr* mgr) 
	{
		plMessage::IMsgRead(stream, mgr);	
		fEventTime = stream->ReadSwapFloat();
		fEvent = (CallbackEvent)stream->ReadSwap16();
		fIndex = stream->ReadSwap16();
		fRepeats = stream->ReadSwap16();
		fUser = stream->ReadSwap16();
	}
	virtual void Write(hsStream* stream, hsResMgr* mgr)	
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwapFloat(fEventTime);
		stream->WriteSwap16((Int16)fEvent);
		stream->WriteSwap16(fIndex);
		stream->WriteSwap16(fRepeats);
		stream->WriteSwap16(fUser);
	}
};

// For when you want to send callbacks, but someone other than the sender/receiver
// needs to modify them along the way.
class plEventCallbackInterceptMsg : public plEventCallbackMsg
{
protected:
	plMessage *fMsg;

public:
	plMessage *GetMessage() { return fMsg; }
	void SetMessage(plMessage *msg) { fMsg = msg; hsRefCnt_SafeRef(msg); }
	void SendMessage() { fMsg->SendAndKeep(); }

	plEventCallbackInterceptMsg() : plEventCallbackMsg(), fMsg(nil) {}
	~plEventCallbackInterceptMsg() { hsRefCnt_SafeUnRef(fMsg); fMsg = nil; }

	CLASSNAME_REGISTER( plEventCallbackInterceptMsg );
	GETINTERFACE_ANY( plEventCallbackInterceptMsg, plEventCallbackMsg );

	virtual void Read(hsStream *stream, hsResMgr *mgr) { plEventCallbackMsg::Read(stream, mgr); }
	virtual void Write(hsStream *stream, hsResMgr *mgr) { plEventCallbackMsg::Write(stream, mgr); }
};


#endif // plEventCallbackMsg_inc
