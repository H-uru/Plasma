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

#ifndef plServerReplyMsg_inc
#define plServerReplyMsg_inc

#include "plMessage.h"

class hsStream;
class hsResMgr;


//
// Sent by server to confirm/deny a sharedState lock attempt.
// Used for trigger arbitration.
// Should not set the NetPropagate bcast flag, since it originates on the gameServer.
//
class plServerReplyMsg : public plMessage
{
	int fType;
public:

	enum
	{
		kUnInit = -1,
		kDeny,
		kAffirm,
	};

	void SetType(int t) { fType = t; }
	int GetType() { return fType; }

	plServerReplyMsg() : fType(kUnInit) { }
	plServerReplyMsg(const plKey &s, const plKey &r, const double* t) : plMessage(s,r,t), fType(kUnInit) { }

	CLASSNAME_REGISTER( plServerReplyMsg );
	GETINTERFACE_ANY( plServerReplyMsg, plMessage );

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

#endif // plServerReplyMsg_inc
