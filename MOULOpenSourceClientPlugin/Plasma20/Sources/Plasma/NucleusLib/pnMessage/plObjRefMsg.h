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

#ifndef plObjRefMsg_inc
#define plObjRefMsg_inc

#include "plRefMsg.h"
#include "hsStream.h"

class hsResMgr;


class plObjRefMsg : public plRefMsg
{

public:
	enum {
		kModifier		= 0,
		kInterface		= 4
	};

	plObjRefMsg(): fType(-1), fWhich(-1) {};

	plObjRefMsg(const plKey &r, UInt8 refMsgFlags, Int8 which , Int8 type)
		: plRefMsg(r, refMsgFlags), fType(type), fWhich(which) {}


	CLASSNAME_REGISTER( plObjRefMsg );
	GETINTERFACE_ANY( plObjRefMsg, plRefMsg );

	Int8					fType;
	Int8					fWhich;

	// IO - not really applicable to ref msgs, but anyway
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Read(stream, mgr);
		stream->ReadSwap(&fType);
		stream->ReadSwap(&fWhich);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Write(stream, mgr);
		stream->WriteSwap(fType);
		stream->WriteSwap(fWhich);
	}
};

#endif // plObjRefMsg_inc
