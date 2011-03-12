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
#ifndef plOneShotMsg_inc
#define plOneShotMsg_inc

#include "plResponderMsg.h"

class plOneShotCallbacks;

class plOneShotMsg : public plResponderMsg
{
public:
	// We can't use a plEventCallbackMsg since we don't know the actual times we
	// want to be called back at.  We need to use a string, then the plAGAnim
	// will figure out the time internally and create a plEventCallbackMsg.
	plOneShotCallbacks *fCallbacks;

	plOneShotMsg();
	~plOneShotMsg();

	CLASSNAME_REGISTER(plOneShotMsg);
	GETINTERFACE_ANY(plOneShotMsg, plResponderMsg);

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);
};

#endif // plOneShotMsg_inc
