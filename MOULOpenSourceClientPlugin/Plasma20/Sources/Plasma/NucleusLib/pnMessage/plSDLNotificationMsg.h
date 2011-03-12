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
#ifndef plSDLNotificationMsg_inc
#define plSDLNotificationMsg_inc

#include "../pnMessage/plMessage.h"

class plSimpleStateVariable;
class plSDLNotificationMsg : public plMessage
{
public:
	float fDelta;				// change threshold
	const plSimpleStateVariable* fVar;
	std::string fSDLName;				// name of state descriptor
	int fPlayerID;				// pid of the player who changed the data
	std::string fHintString;	// hint from the player who changed the data
	
	plSDLNotificationMsg() : fDelta(0), fVar(nil), fPlayerID(0) {}
	~plSDLNotificationMsg() { }

	CLASSNAME_REGISTER( plSDLNotificationMsg );
	GETINTERFACE_ANY( plSDLNotificationMsg, plMessage );

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "NA: LocalOnly msg");	}
	void Write(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "NA: LocalOnly msg");	}
};

#endif	// plSDLNotificationMsg_in

