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
#ifndef plAnimEventModifier_h_inc
#define plAnimEventModifier_h_inc

#include "../pnModifier/plSingleModifier.h"
#include "hsTemplates.h"

//
// Detects an anim event (marker, begin, end) and sends out a notification.
//
class plAnimEventModifier : public plSingleModifier
{
protected:
	hsTArray<plKey> fReceivers;// Keys to notify when the anim event happens
	plMessage* fCallback;		// The callback setup message we send when the anim loads

	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) { return false; }

	void ISendNotify(bool triggered);
	hsBool fDisabled;
public:
	plAnimEventModifier();
	virtual ~plAnimEventModifier();

	CLASSNAME_REGISTER(plAnimEventModifier);
	GETINTERFACE_ANY(plAnimEventModifier, plSingleModifier);

	virtual void Read(hsStream* stream, hsResMgr* mgr);
	virtual void Write(hsStream* stream, hsResMgr* mgr);

	virtual hsBool MsgReceive(plMessage* msg);

	// Export only
	void SetReceivers(hsTArray<plKey>& receivers) { fReceivers = receivers; }
	void SetCallback(plMessage* callback) { fCallback = callback; }
};

#endif // plAnimEventModifier_h_inc
