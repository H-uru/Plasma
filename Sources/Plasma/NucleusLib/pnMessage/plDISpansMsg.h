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
#ifndef plDISpansMsg_inc
#define plDISpansMsg_inc

#include "plMessage.h"
#include "hsStream.h"

class hsKeyedObject;

class plDISpansMsg : public plMessage
{
public:
	enum {
		kAddingSpan,
		kRemovingSpan
	};
	UInt8					fType;

	enum {
		kLeaveEmptyDrawable = 0x1
	};
	UInt8					fFlags;

	Int32					fIndex;

	plDISpansMsg() : plMessage(), fType(0), fFlags(0), fIndex(-1) {}
	plDISpansMsg(const plKey &r, UInt8 type, int index, int flags) : plMessage(nil, r, nil), fType(type), fIndex(index), fFlags(flags) {}	
	
	CLASSNAME_REGISTER( plDISpansMsg );
	GETINTERFACE_ANY( plDISpansMsg, plMessage );
	
	void Read(hsStream* stream, hsResMgr* mgr) {}
	void Write(hsStream* stream, hsResMgr* mgr) {}
};

#endif // plDISpansMsg_inc
