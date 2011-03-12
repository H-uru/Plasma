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

#ifndef plActivatorMsg_inc
#define plActivatorMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsGeometry3.h"

class hsStream;
class hsResMgr;


class plActivatorMsg : public plMessage
{

	void IReset() { fPickedObj=fHiteeObj=fHitterObj=nil; fTriggerType=0; fHitPoint.Set(0,0,0); }
public:
	plActivatorMsg() { IReset(); }
	plActivatorMsg(const plKey &s, 
					const plKey &r, 
					const double* t) { IReset(); }
	~plActivatorMsg() { }

	CLASSNAME_REGISTER( plActivatorMsg );
	GETINTERFACE_ANY( plActivatorMsg, plMessage );
	
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fTriggerType = stream->ReadSwap32();
		fHitPoint.Read(stream);
	}

	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		stream->WriteSwap32( fTriggerType );
		fHitPoint.Write(stream);
	}

	hsBool TriggerType() { return fTriggerType; }
	void SetTriggerType(int n) { fTriggerType = n; }

	enum 
	{
		kUndefined	= 0,
		kPickedTrigger,
		kUnPickedTrigger,
		kCollideEnter,
		kCollideExit,
		kCollideContact,
		kCollideUnTrigger,
		kRoomEntryTrigger,
		kLogicMod,
		kVolumeEnter,
		kVolumeExit,
		kEnterUnTrigger,
		kExitUnTrigger,
	};

	UInt32	fTriggerType;
	plKey	fPickedObj;
	plKey	fHiteeObj;
	plKey	fHitterObj;

	hsPoint3	fHitPoint;		// currently only useful for Picked events


};

#endif // plActivatorMsg_inc
