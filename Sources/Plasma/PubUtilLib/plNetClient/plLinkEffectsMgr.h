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
#ifndef plLinkEffectsMgr_inc
#define plLinkEffectsMgr_inc

#include "../pnKeyedObject/hsKeyedObject.h"

class plLinkEffectsTriggerMsg;
class plPseudoLinkEffectMsg;

class plLinkEffectsMgr : public hsKeyedObject
{
protected:
	// Collection of links in progress (in or out)
	hsTArray<plLinkEffectsTriggerMsg *> fLinks;

	// Players we know exist, but aren't ready to link yet
	hsTArray<plLinkEffectsTriggerMsg *> fWaitlist; 

	// Queue of delayed messages from people linking in that 
	// we haven't received yet but are no longer necessary, 
	// because we either received the trigger from them, or
	// they're no longer in the age.
	hsTArray<plLinkEffectsTriggerMsg *> fDeadlist;
	
	// queue of pseudo link messages
	hsTArray<plPseudoLinkEffectMsg *> fPseudolist;

	plLinkEffectsTriggerMsg *IFindLinkTriggerMsg(plKey avatarKey);
	void IAddLink(plLinkEffectsTriggerMsg *msg);
	void IAddWait(plLinkEffectsTriggerMsg *msg);
	void IAddDead(plLinkEffectsTriggerMsg *msg);
	void IAddPsuedo(plPseudoLinkEffectMsg *msg);
	void IRemovePseudo(plKey avatarKey);
	plPseudoLinkEffectMsg* IFindPseudo(plKey avatarKey);

	hsBool IHuntWaitlist(plLinkEffectsTriggerMsg *msg);
	hsBool IHuntWaitlist(plKey linkKey);
	hsBool IHuntDeadlist(plLinkEffectsTriggerMsg *msg);
	void ISendAllReadyCallbacks();
	
public:
	plLinkEffectsMgr();
	~plLinkEffectsMgr();
	void Init();

	CLASSNAME_REGISTER( plLinkEffectsMgr );
	GETINTERFACE_ANY( plLinkEffectsMgr, hsKeyedObject );

	void WaitForEffect(plKey linkKey, hsScalar time);
	void WaitForPseudoEffect(plKey linkKey, hsScalar time);
	
	plMessage *WaitForEffect(plKey linkKey);

	virtual hsBool MsgReceive(plMessage *msg);
};

#endif // plLinkEffectsMgr_inc
