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
#ifndef plNetMsgHandler_inc
#define plNetMsgHandler_inc

#include "hsConfig.h"
#include "hsTypes.h"	// for nil

class plNetMessage;
class plNetApp;

#define kMsgSendInterval		0.25f	// time between message re-send
#define kNetOperationTimeout	10.f	// amount of time we try a network operation before complaining

//
// Base msg handler class
//
class plNetMsgHandler
{
protected:
	plNetApp* fNetApp;
public:
	plNetMsgHandler() : fNetApp(nil) {}
	virtual ~plNetMsgHandler() {}

	void SetNetApp(plNetApp* na) { fNetApp=na; }
	plNetApp* GetNetApp() { return fNetApp; }

	// return -1 on error, 0 if ok.	
	virtual int ReceiveMsg(plNetMessage*& netMsg) = 0;
};

#define MSG_HANDLER(msgClassName) msgClassName##HandleMsg

//
// Use to declare msg handler fxns in your MsgHandler .h class header
//
#define MSG_HANDLER_DECL(msgClassName) \
virtual int MSG_HANDLER(msgClassName)(plNetMessage*& netMsg); 

//
// Use to define msg handler fxns in your MsgHandler .cpp file
//
#define MSG_HANDLER_DEFN(handlerClassName, msgClassName) \
int handlerClassName::MSG_HANDLER(msgClassName)(plNetMessage*& netMsg)

//
// Use in the switch statement in your ReceiveMsg function
//
#define MSG_HANDLER_CASE(msgClassName) \
case CLASS_INDEX_SCOPED(msgClassName): \
	{ \
		return MSG_HANDLER(msgClassName)(netMsg); \
	} 


#endif	//  plNetMsgHandler_inc
