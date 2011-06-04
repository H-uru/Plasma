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

#ifndef plDispatchBase_inc
#define plDispatchBase_inc

#include "../pnFactory/plCreatable.h"

class plMessage;
class plKey;

class plDispatchBase : public plCreatable
{
public:
	CLASSNAME_REGISTER( plDispatchBase );
	GETINTERFACE_ANY( plDispatchBase, plCreatable );

	virtual void RegisterForType(UInt16 hClass, const plKey& receiver) = 0;
	virtual void RegisterForExactType(UInt16 hClass, const plKey& receiver) = 0;

	virtual void UnRegisterForType(UInt16 hClass, const plKey& receiver) = 0;
	virtual void UnRegisterForExactType(UInt16 hClass, const plKey& receiver) = 0;

	virtual void UnRegisterAll(const plKey& receiver) = 0;

	virtual hsBool MsgSend(plMessage* msg, hsBool async=false) = 0;
	virtual void	MsgQueue(plMessage* msg)=0;	// Used by other thread to Send Messages, they are handled as soon as Practicable
	virtual void	MsgQueueProcess() = 0;
	virtual void	MsgQueueOnOff(hsBool) = 0;		// Turn on or off Queued Messages, if off, uses MsgSend Immediately (for plugins)

	virtual hsBool	SetMsgBuffering(hsBool on) = 0; // On starts deferring msg delivery until buffering is set to off again.
};

class plgDispatch
{
public:
	static plDispatchBase* Dispatch();
	static hsBool MsgSend(plMessage* msg, hsBool async = false) { return Dispatch()->MsgSend(msg, async); }
};


#endif // plDispatchBase_inc
