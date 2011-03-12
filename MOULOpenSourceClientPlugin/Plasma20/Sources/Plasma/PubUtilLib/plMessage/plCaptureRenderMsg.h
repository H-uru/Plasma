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

#ifndef plCaptureRenderMsg_inc
#define plCaptureRenderMsg_inc

#include "../pnMessage/plMessage.h"

class plMipmap;

class plCaptureRenderMsg : public plMessage
{
public:
	plCaptureRenderMsg() : plMessage(), fMipmap(nil) {}
	plCaptureRenderMsg(const plKey &r, plMipmap* mipmap) : plMessage(nil, r, nil), fMipmap(mipmap) {}

	virtual ~plCaptureRenderMsg();
	
	CLASSNAME_REGISTER( plCaptureRenderMsg );
	GETINTERFACE_ANY( plCaptureRenderMsg, plMessage );

	// Mipmap will be unref'd on destruction of message. If you plan to
	// hang onto it, you need to ref it when you receive this message.
	// You can either use hsRefCnt or use the ResMgr to give it a new key.
	plMipmap* GetMipmap() const { return fMipmap; }
	plMipmap*		fMipmap;

	virtual void Read(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "Transient message"); }
	virtual void Write(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "Transient message"); }
};

#endif // plCaptureRenderMsg_inc
