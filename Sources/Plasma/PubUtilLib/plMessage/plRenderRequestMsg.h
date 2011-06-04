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

#ifndef plRenderRequestMsg_inc
#define plRenderRequestMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsRefCnt.h"

class plRenderRequest;
class hsStream;
class hsResMgr;

// This is a little StoneAge, using the old HeadSpin ref count here.
// It's a perfect spot for a smart pointer, but we ain't got none.
// 
// Basic issue is that we hand a pointer to this request off to the
// client (via the plRenderRequestMsg), who will at some later point this frame
// hand it to the pipeline to be processed. So if I want to hand it off and
// forget about it, the client needs to delete it. If I want to reuse it every
// frame and destroy it when I'm done, I need to delete it. Sounds just like
// a smart pointer, or for the cro-magnons a RefCnt.

class plRenderRequestBase : public hsRefCnt
{
};

class plRenderRequestMsg : public plMessage
{
protected:

	plRenderRequestBase*		fReq;

public:
	// Argumentless constructor useless (except for compiling).
	plRenderRequestMsg();
	// non-nil sender will get an ack when render is "done".
	plRenderRequestMsg(plKey sender, plRenderRequestBase* req);
	virtual ~plRenderRequestMsg();

	CLASSNAME_REGISTER( plRenderRequestMsg );
	GETINTERFACE_ANY( plRenderRequestMsg, plMessage );

	// These aren't really implemented. Read/Write/Transmission of
	// these messages doesn't currently make sense.
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	plRenderRequest* Request() const { return (plRenderRequest*)fReq; }

};

class plRenderRequestAck: public plMessage
{
protected:
	UInt32			fUserData;
	UInt32			fNumDrawn; // number of objects drawn.
public:
	// Argumentless constructor useless (except for compiling).
	plRenderRequestAck();
	plRenderRequestAck(plKey r, UInt32 userData=0);
	~plRenderRequestAck() {}

	CLASSNAME_REGISTER( plRenderRequestAck );
	GETINTERFACE_ANY( plRenderRequestAck, plMessage );

	void		SetNumDrawn(UInt32 n) { fNumDrawn = n; }
	UInt32		GetNumDrawn() const { return fNumDrawn; }

	// These aren't really implemented. Read/Write/Transmission of
	// these messages doesn't currently make sense.
	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

};

#endif // plRenderRequestMsg_inc
