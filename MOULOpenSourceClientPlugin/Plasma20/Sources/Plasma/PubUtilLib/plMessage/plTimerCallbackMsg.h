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
#ifndef plTimerCallbackMsg_inc
#define plTimerCallbackMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsStream.h"

class hsResMgr;

class plTimerCallbackMsg : public plMessage
{
public:
	plTimerCallbackMsg(){;}
	plTimerCallbackMsg(const plKey &s, const plKey &r, const double* t){;}
	plTimerCallbackMsg(const plKey &r, UInt32 id = 0) { AddReceiver(r); fID = id;}
	~plTimerCallbackMsg(){;}

	CLASSNAME_REGISTER( plTimerCallbackMsg );
	GETINTERFACE_ANY( plTimerCallbackMsg, plMessage );
	
	UInt32 fID;
	hsScalar fTime;

	virtual void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fID = stream->ReadSwap32();
		fTime = stream->ReadSwapScalar();
	}

	virtual void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap32(fID);
		stream->WriteSwapScalar(fTime);
	}
};

#endif // plTimerCallbackMsg_inc
