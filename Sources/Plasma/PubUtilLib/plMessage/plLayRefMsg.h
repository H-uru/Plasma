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

#ifndef plLayRefMsg_inc
#define plLayRefMsg_inc

#include "../pnMessage/plRefMsg.h"

class hsStream;
class hsResMgr;

class plLayRefMsg : public plRefMsg
{
public:
	enum {
		kTexture		= 1,
		kUnderLay		= 2,
		kVertexShader	= 3,
		kPixelShader	= 4
	};

	plLayRefMsg() : fType(-1), fWhich(-1) {}
	plLayRefMsg(const plKey &r, UInt8 f, Int8 which, Int8 type) : plRefMsg(r, f), fWhich(which), fType(type) {}

	CLASSNAME_REGISTER( plLayRefMsg );
	GETINTERFACE_ANY( plLayRefMsg, plRefMsg );

	Int8		fType;
	Int8		fWhich;

	// IO - not really applicable to ref msgs, but anyway
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plLayRefMsg_inc
