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
#ifndef plClimbMsg_h
#define plClimbMsg_h

#include "../pnMessage/plMessage.h"

/** \class plClimbMsg
	Things you can say with a climb message:
	"mount the climbing wall with an upward/downward/leftward/rightward mount"
	"dis-/enable climbing in the up/down/left/right direction"
	"dis-/enable dismounting in the up/down/left/right direction"
*/	
class plClimbMsg : public plMessage
{
public:
	enum Direction {
		kUp =		0x01,
		kDown =		0x02,
		kLeft =		0x04,
		kRight =	0x08,
		kCenter =	0x10
	};

	enum Command {
		kNoCommand =		0x0,
		kEnableClimb =		0x1,
		kEnableDismount =	0x2,
		kFallOff =			0x4,
		kRelease =			0x8,
		kStartClimbing =	0x8
	};

	// tors
	plClimbMsg();
	plClimbMsg(const plKey &sender, const plKey &receiver, Command command = kNoCommand, Direction direction = kCenter, hsBool status = false, plKey target = nil);

	// plasma protocol
	CLASSNAME_REGISTER( plClimbMsg );
	GETINTERFACE_ANY( plClimbMsg, plMessage );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);

	Command fCommand;
	Direction fDirection;
	hsBool fStatus;
	plKey fTarget;	// used for seeking to mount points
private:
};

#endif
