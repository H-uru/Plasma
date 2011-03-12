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
#ifndef pfMarkerMsg_h_inc
#define pfMarkerMsg_h_inc

#include "../pnMessage/plMessage.h"
struct hsPoint3;

class pfMarkerMsg : public plMessage
{
public:
	enum Type
	{
		// Sent by yourself when you hit a marker
		// - fMarkerID is the id number of the marker we hit
		kMarkerCaptured,
	};
	Type fType;

	UInt32 fMarkerID;
	
	pfMarkerMsg();
	virtual ~pfMarkerMsg();

	void PrintDebug(char* buf);

	CLASSNAME_REGISTER(pfMarkerMsg);
	GETINTERFACE_ANY(pfMarkerMsg, plMessage);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // pfMarkerMsg_h_inc
