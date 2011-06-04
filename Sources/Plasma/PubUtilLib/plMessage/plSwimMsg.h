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
#ifndef plSwimMsg_h
#define plSwimMsg_h

#include "../pnMessage/plMessage.h"

/** \class plSwimMsg
	You're either entering the pool, or leaving. Those are the only swim messages right now.
*/	
class plSwimMsg : public plMessage
{
public:

	// tors
	plSwimMsg();
	plSwimMsg(const plKey &sender, const plKey &receiver, bool entering, plKey regionKey);


	bool GetIsEntering();
	bool GetIsLeaving();

	// plasma protocol
	CLASSNAME_REGISTER( plSwimMsg );
	GETINTERFACE_ANY( plSwimMsg, plMessage );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	plKey fSwimRegionKey;
	
private:
	bool fIsEntering;	// right now, if you're not entering, you're leaving
						// that might not be so simple later, so we hide it
						// behind a getter.
};


#endif
