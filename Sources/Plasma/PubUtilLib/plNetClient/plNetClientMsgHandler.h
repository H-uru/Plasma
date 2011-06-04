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
#ifndef plNetClientMsgHandler_inc
#define plNetClientMsgHandler_inc

#include "../plNetCommon/plNetMsgHandler.h"
#include "hsStlUtils.h"

///////////////////////////////////////////////////////////////////

//
// define msg handler fxn for netClient msgs
//
class plNetClientMgr;
class plNetMsgMemberInfoHelper;
class plNetTransportMember;

///////////////////////////////////////////////////////////////////

class plNetClientMsgHandler : public plNetMsgHandler
{
//protected:
	enum // SendOrTimeOut return values and meanings
	{
		kTimedOutNoRetry	= -1,
		kDoNothing	=  0,
		kSend		=  1,
	};
	plNetClientMgr * IGetNetClientMgr();
	void IFillInTransportMember(const plNetMsgMemberInfoHelper* mbi, plNetTransportMember* mbr);
public:
	plNetClientMsgHandler(plNetClientMgr * mgr);
	~plNetClientMsgHandler();

	int		ReceiveMsg(plNetMessage *& netMsg);	
	int		PeekMsg(plNetMessage * netMsg);	// return msgsize on success. -1 on error.

	MSG_HANDLER_DECL(plNetMsgTerminated)
	MSG_HANDLER_DECL(plNetMsgGroupOwner)
	MSG_HANDLER_DECL(plNetMsgSDLState)
	MSG_HANDLER_DECL(plNetMsgGameMessage)
	MSG_HANDLER_DECL(plNetMsgVoice)
	MSG_HANDLER_DECL(plNetMsgMembersList)
	MSG_HANDLER_DECL(plNetMsgMemberUpdate)
	MSG_HANDLER_DECL(plNetMsgListenListUpdate)
	MSG_HANDLER_DECL(plNetMsgInitialAgeStateSent)
};

#endif	//  plNetClientMsgHandler_inc
