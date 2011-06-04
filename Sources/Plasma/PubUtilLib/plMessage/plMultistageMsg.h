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
#ifndef plMultistageMsg_inc
#define plMultistageMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsBitVector.h"

// Messages sent to a MultistageModifier.

class plMultistageModMsg : public plMessage
{
protected:
	hsBitVector fCmds;

public:
	enum // Commands
	{
		kSetLoopCount,
	};

	UInt8 fStageNum;
	UInt8 fNumLoops;

	plMultistageModMsg() : fStageNum(0), fNumLoops(1) {}
	plMultistageModMsg(const plKey &sender, const plKey &receiver) : plMessage(sender,receiver,nil),fStageNum(0),fNumLoops(1) {}

	hsBool GetCommand(UInt8 cmd) { return fCmds.IsBitSet(cmd); }
	void SetCommand(UInt8 cmd, hsBool val = true) { fCmds.SetBit(cmd, val); }
	
	// plasma protocol
	CLASSNAME_REGISTER( plMultistageModMsg );
	GETINTERFACE_ANY( plMultistageModMsg, plMessage );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif