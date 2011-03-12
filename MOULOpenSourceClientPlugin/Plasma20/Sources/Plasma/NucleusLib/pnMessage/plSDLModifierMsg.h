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
#ifndef plSDLModifierMsg_INC
#define plSDLModifierMsg_INC

#include "../pnMessage/plMessage.h"
#include "hsUtils.h"

//
// A msg sent to an SDL modifier to tell it send or recv state.
//
class hsStream;
class hsResMgr;
class plStateDataRecord;
class plSDLModifierMsg : public plMessage
{
public:
	enum Action
	{
		kActionNone	= 0,
		kRecv,
		kSendToServer,
		kSendToServerAndClients
	};

protected:
	char* fSDLName;			// the state descriptor name (ie. "physical")
	Action fAction;
	plStateDataRecord* fState;		// for recving state
	bool fManageStateMem;			// delete fState?
	UInt32 fPlayerID;
	UInt32 fFlags;

public:
	plSDLModifierMsg(const char* sdlName=nil, Action a=kActionNone);
	~plSDLModifierMsg();

	CLASSNAME_REGISTER( plSDLModifierMsg );
	GETINTERFACE_ANY( plSDLModifierMsg, plMessage );

	UInt32 GetFlags() const { return fFlags; }
	void SetFlags(UInt32 f) { fFlags = f; }

	Action GetAction() const { return fAction; }
	void SetAction(Action t) { fAction=t; }

	plStateDataRecord* GetState(bool unManageState=false) { if ( unManageState ) fManageStateMem=false; return fState; }
	void SetState(plStateDataRecord* s, bool manageState) { fState=s; fManageStateMem=manageState; }

	const char* GetSDLName() const { return fSDLName; }
	void SetSDLName(const char* s) { delete [] fSDLName; fSDLName=hsStrcpy(s); }
		
	UInt32 GetPlayerID() const { return fPlayerID;	}
	void SetPlayerID(UInt32 p) { fPlayerID=p;	}
	
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "local only msg"); }
	void Write(hsStream* stream, hsResMgr* mgr) { hsAssert(false, "local only msg"); }
};

#endif	// plSDLModifierMsg_INC
