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
#ifndef plMessageWithCB_inc
#define plMessageWithCB_inc

#include "plMessage.h"
#include "hsTemplates.h"
#include "plEventCallbackMsg.h"
//
// Base class for messages which contain callbacks
//

class plSynchedObject;
class plEventCallbackMsg;
class hsStream;
class hsResMgr;
class plMessageWithCallbacks : public plMessage
{
private:
	hsTArray<plMessage*>		fCallbacks;	
public:
	plMessageWithCallbacks() {}
	plMessageWithCallbacks(const plKey &s, const plKey &r, const double* t) : plMessage(s,r,t) {}
	~plMessageWithCallbacks();

	CLASSNAME_REGISTER( plMessageWithCallbacks );
	GETINTERFACE_ANY( plMessageWithCallbacks, plMessage );
	
	void Clear();
	
	void				AddCallback(plMessage* e); // will RefCnt the message e.
	int					GetNumCallbacks() const { return fCallbacks.GetCount(); }
	plMessage*			GetCallback(int i) const { return fCallbacks[i]; }
	plEventCallbackMsg*	GetEventCallback(int i) const { return plEventCallbackMsg::ConvertNoRef(fCallbacks[i]); }
	void SendCallbacks();
	
#if 0
	// returns true if ok to send in a networked situations
	static hsBool	NetOKToSend(plSynchedObject* sender, plEventCallbackMsg* cbmsg);	
#endif

	// IO
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

#endif	// plMessageWithCB_inc
