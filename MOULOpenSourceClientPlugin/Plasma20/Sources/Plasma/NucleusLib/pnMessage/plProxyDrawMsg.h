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

#ifndef plProxyDrawMsg_inc
#define plProxyDrawMsg_inc

#include "plMessage.h"

class hsStream;
class hsResMgr;

// Proxy Draw Msg's are sent out to tell
// proxies for the visual objects in the world to
// make themselves visible (or reclaim resources used
// to make themselves visible). This message should only
// be sent out by the core system.
class plProxyDrawMsg : public plMessage
{
protected:
	UInt16		fProxyFlags;

public:
	plProxyDrawMsg();
	plProxyDrawMsg(UInt16 flags); // for broadcast
	plProxyDrawMsg(plKey &rcv, UInt16 flags); // send yourself an ack
	~plProxyDrawMsg();

	CLASSNAME_REGISTER( plProxyDrawMsg );
	GETINTERFACE_ANY( plProxyDrawMsg, plMessage );

	enum {
		kCreate			= 0x1,
		kDestroy		= 0x2,
		kDetached		= 0x4,
		kToggle			= 0x8,

		kLight			= 0x10,
		kPhysical		= 0x20,
		kOccluder		= 0x40,
		kAudible		= 0x80,
		kCoordinate		= 0x100,
		kCamera			= 0x200,

		kAllTypes		= kLight 
						| kPhysical 
						| kOccluder
						| kAudible
						| kCoordinate
						| kCamera
	};

	UInt16	GetProxyFlags() const { return fProxyFlags; }
	void	SetProxyFlags(UInt16 f) { fProxyFlags = f; }

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plProxyDrawMsg_inc
