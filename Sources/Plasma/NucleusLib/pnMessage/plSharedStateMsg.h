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
#ifndef plSharedStateMsg_inc
#define plSharedStateMsg_inc

#include "plMessage.h"
#include "../pnNetCommon/plNetSharedState.h"

class hsStream;
class hsResMgr;

//
// Msg from server containing generic sharedState
//
class plSharedStateMsg : public plMessage
{
protected:
	plNetSharedState fSharedState;
public:
	plSharedStateMsg() {}
	plSharedStateMsg(const plKey &s, const plKey &r, const double* t) : plMessage(s,r,t) {}

	CLASSNAME_REGISTER( plSharedStateMsg );
	GETINTERFACE_ANY( plSharedStateMsg, plMessage );

	void CopySharedState(plNetSharedState* ss) { fSharedState.Copy(ss); }
	plNetSharedState* GetSharedState() { return &fSharedState; }

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr) 	{ plMessage::IMsgRead(stream, mgr); fSharedState.Write(stream); }
	void Write(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgWrite(stream, mgr); fSharedState.Read(stream); }
};

#endif // plSharedStateMsg_inc
