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

#ifndef plTimeMsg_inc
#define plTimeMsg_inc

#include "plMessage.h"
#include "hsStream.h"

class plTimeMsg : public plMessage
{
protected:
	double			fSeconds;
	hsScalar		fDelSecs;

public:
	plTimeMsg();
	plTimeMsg(const plKey &s, 
					const plKey &r, 
					const double* t, const hsScalar* del);
	~plTimeMsg();

	CLASSNAME_REGISTER( plTimeMsg );
	GETINTERFACE_ANY( plTimeMsg, plMessage );

	plTimeMsg& SetSeconds(double s) { fSeconds = s; return *this; }
	plTimeMsg& SetDelSeconds(hsScalar d) { fDelSecs = d; return *this; }

	double			DSeconds() { return fSeconds; }
	hsScalar		DelSeconds() { return fDelSecs; }

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		stream->ReadSwap(&fSeconds);
		stream->ReadSwap(&fDelSecs);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap(fSeconds);
		stream->WriteSwap(fDelSecs);
	}
};

class plEvalMsg : public plTimeMsg
{
public:
	plEvalMsg();
	plEvalMsg(const plKey &s, 
					const plKey &r, 
					const double* t, const hsScalar* del);
	~plEvalMsg();

	CLASSNAME_REGISTER( plEvalMsg );
	GETINTERFACE_ANY( plEvalMsg, plTimeMsg );

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)	{	plTimeMsg::Read(stream, mgr);	}
	void Write(hsStream* stream, hsResMgr* mgr)	{	plTimeMsg::Write(stream, mgr);	}
};

class plTransformMsg : public plTimeMsg
{
public:
	plTransformMsg();
	plTransformMsg(const plKey &s, 
					const plKey &r, 
					const double* t, const hsScalar* del);
	~plTransformMsg();

	CLASSNAME_REGISTER( plTransformMsg );
	GETINTERFACE_ANY( plTransformMsg, plTimeMsg );

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)	{	plTimeMsg::Read(stream, mgr);	}
	void Write(hsStream* stream, hsResMgr* mgr)	{	plTimeMsg::Write(stream, mgr);	}
};

// Does the same thing as plTransformMsg, but as you might guess from the name, it's sent later.
// It's a separate message type so that objects can register for just the delayed message when
// it's broadcast.
class plDelayedTransformMsg : public plTransformMsg
{
public:
	plDelayedTransformMsg() : plTransformMsg() {}
	plDelayedTransformMsg(const plKey &s, const plKey &r, const double* t, const hsScalar* del) : plTransformMsg(s, r, t, del) {}
	
	CLASSNAME_REGISTER( plDelayedTransformMsg );
	GETINTERFACE_ANY( plDelayedTransformMsg, plTransformMsg );
};

#endif // plTimeMsg_inc
