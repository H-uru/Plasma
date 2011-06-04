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
#ifndef plArmatureEffectMsg_inc
#define plArmatureEffectMsg_inc

#include "../pnMessage/plEventCallbackMsg.h"

class plArmatureEffectMsg : public plEventCallbackMsg
{
public:
	plArmatureEffectMsg() : plEventCallbackMsg(), fTriggerIdx(-1) {}
	plArmatureEffectMsg(const plKey &receiver, CallbackEvent e, int idx=0, hsScalar t=0, Int16 repeats=-1, UInt16 user=0) :
		plEventCallbackMsg(receiver, e, idx, t, repeats, user), fTriggerIdx(-1) {}

	CLASSNAME_REGISTER( plArmatureEffectMsg );
	GETINTERFACE_ANY( plArmatureEffectMsg, plEventCallbackMsg );

	// These aren't meant to go across the net, so no IO necessary.
	void Read(hsStream* stream, hsResMgr* mgr) {}
	void Write(hsStream* stream, hsResMgr* mgr) {}

	Int8 fTriggerIdx;
};

class plArmatureEffectStateMsg : public plMessage
{
public:
	plArmatureEffectStateMsg();
	~plArmatureEffectStateMsg();
	
	CLASSNAME_REGISTER( plArmatureEffectStateMsg );
	GETINTERFACE_ANY( plArmatureEffectStateMsg, plMessage );	

	virtual void Read(hsStream* stream, hsResMgr* mgr); 
	virtual void Write(hsStream* stream, hsResMgr* mgr); 

	Int8 fSurface;
	hsBool fAddSurface;
};
	

#endif