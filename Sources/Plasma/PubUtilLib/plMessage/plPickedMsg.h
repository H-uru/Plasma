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

#ifndef plPickedMsg_inc
#define plPickedMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsGeometry3.h"


class plPickedMsg : public plMessage
{
protected:

public:
	
	hsBool	fPicked;
	hsPoint3	fHitPoint;		// where in the world the object was picked on
		
	plPickedMsg() : fPicked(true),fHitPoint(0,0,0){SetBCastFlag(plMessage::kPropagateToModifiers);}
	plPickedMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : fPicked(true),fHitPoint(0,0,0) {SetBCastFlag(plMessage::kPropagateToModifiers);}
	~plPickedMsg(){;}

	CLASSNAME_REGISTER( plPickedMsg );
	GETINTERFACE_ANY( plPickedMsg, plMessage );

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fPicked = stream->ReadBool();
		fHitPoint.Read(stream);
	}
	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteBool(fPicked);
		fHitPoint.Write(stream);
	}
	

};




#endif // PickedMsg
