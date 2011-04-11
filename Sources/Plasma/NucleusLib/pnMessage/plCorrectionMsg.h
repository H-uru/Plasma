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

#ifndef plCorrectionMsg_inc
#define plCorrectionMsg_inc

#include "plMessage.h"
#include "hsMatrix44.h"

class plCorrectionMsg : public plMessage
{
public:
	plCorrectionMsg() : plMessage(nil, nil, nil) { }
	plCorrectionMsg(plKey &r, const hsMatrix44& l2w, const hsMatrix44& w2l, hsBool dirtySynch = false)
		: plMessage(nil, r, nil),
		  fLocalToWorld(l2w),
		  fWorldToLocal(w2l),
		  fDirtySynch(dirtySynch)
	{ }

	CLASSNAME_REGISTER( plCorrectionMsg );
	GETINTERFACE_ANY( plCorrectionMsg, plMessage );

	hsMatrix44		fLocalToWorld;
	hsMatrix44		fWorldToLocal;
	
	hsBool			fDirtySynch;

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fLocalToWorld.Read(stream);
		fWorldToLocal.Read(stream);
	}
	
	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		fLocalToWorld.Write(stream);
		fWorldToLocal.Write(stream);
	}


};

#endif // plCorrectionMsg_inc
