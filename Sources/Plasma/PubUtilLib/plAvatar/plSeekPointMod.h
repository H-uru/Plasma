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
#ifndef PLSEEKPOINTMOD_INC
#define PLSEEKPOINTMOD_INC

#include "../pnModifier/plMultiModifier.h"
#include "../pnMessage/plMessage.h"

// PLSEEKPOINTMOD
// This modifier is something the avatar knows how to go to. (you know, seek)
// It's kind of like a magnet that, when activated, draws the avatar...
// Seen another way, it's a point with a name and a type....
class plSeekPointMod : public plMultiModifier
{
protected:
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) {return true;}
	char * fName;										// public because you can't change it

public:

	plSeekPointMod();
	plSeekPointMod(char *name);
	virtual ~plSeekPointMod();

	const char * GetName() { return fName; };
	void SetName(char * name) { fName = name; };

	CLASSNAME_REGISTER( plSeekPointMod );
	GETINTERFACE_ANY( plSeekPointMod, plMultiModifier );
	
	virtual void AddTarget(plSceneObject* so);

	hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif
