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

#ifndef plIntRefMsg_inc
#define plIntRefMsg_inc

#include "plRefMsg.h"
#include "hsStream.h"

class hsResMgr;


class plIntRefMsg : public plRefMsg
{
public:
	enum {
		kOwner			= 1,
		kTarget			= 2,
		kChild			= 3,
		kDrawable		= 4,
		kPhysical		= 5,
		kAudible		= 6,
		kChildObject	= 7,

		kNumRefTypes
	};

	plIntRefMsg() : fType(-1), fWhich(-1), fIdx(-1) {}
	plIntRefMsg(const plKey &r, UInt8 flags, Int32 which, Int8 type, Int8 idx=-1) : plRefMsg(r, flags), fWhich((Int16)which), fType(type), fIdx(idx) {}

	CLASSNAME_REGISTER( plIntRefMsg );
	GETINTERFACE_ANY( plIntRefMsg, plRefMsg );

	Int8		fType;
	Int8		fIdx;
	Int16		fWhich;

	// IO - not really applicable to ref msgs, but anyway
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Read(stream, mgr);
		stream->ReadSwap(&fType);
		stream->ReadSwap(&fWhich);
		stream->ReadSwap(&fIdx);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plRefMsg::Write(stream, mgr);
		stream->WriteSwap(fType);
		stream->WriteSwap(fWhich);
		stream->WriteSwap(fIdx);
	}
};

#endif // plIntRefMsg_inc
