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

#ifndef plRenderMsg_inc
#define plRenderMsg_inc

#include "../pnMessage/plMessage.h"

class plPipeline;

class plRenderMsg : public plMessage
{
protected:
	plPipeline*				fPipe;

public:
	plRenderMsg() : plMessage(nil, nil, nil), fPipe(nil) { SetBCastFlag(kBCastByExactType); }
	plRenderMsg(plPipeline* pipe) : plMessage(nil, nil, nil), fPipe(pipe) { SetBCastFlag(kBCastByExactType); }

	~plRenderMsg() {}
	
	CLASSNAME_REGISTER( plRenderMsg );
	GETINTERFACE_ANY( plRenderMsg, plMessage );

	plPipeline* Pipeline() const { return fPipe; }

	virtual void Read(hsStream* s, hsResMgr* mgr) { plMessage::IMsgRead(s, mgr); }
	virtual void Write(hsStream* s, hsResMgr* mgr) { plMessage::IMsgWrite(s, mgr); }
};

class plPreResourceMsg : public plMessage
{
protected:
	plPipeline*				fPipe;

public:
	plPreResourceMsg() : plMessage(nil, nil, nil), fPipe(nil) { SetBCastFlag(kBCastByExactType); }
	plPreResourceMsg(plPipeline* pipe) : plMessage(nil, nil, nil), fPipe(pipe) { SetBCastFlag(kBCastByExactType); }

	~plPreResourceMsg() {}

	CLASSNAME_REGISTER( plPreResourceMsg );
	GETINTERFACE_ANY( plPreResourceMsg, plMessage );

	plPipeline* Pipeline() const { return fPipe; }

	virtual void Read(hsStream* s, hsResMgr* mgr) {}
	virtual void Write(hsStream* s, hsResMgr* mgr) {}
};

#endif // plRenderMsg_inc
