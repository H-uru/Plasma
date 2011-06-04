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
#ifndef plOneShotMod_INC
#define plOneShotMod_INC

#include "../pnModifier/plMultiModifier.h"
#include "../pnMessage/plMessage.h"

// PLONESHOTMOD
// A one shot
// - suspends user input
// - moves the avatar to a specific position
// - plays a specific animation
// - returns control to the user from the final position of the animation
// This modifier holds the information that the avatar needs to do that.
class plOneShotMod : public plMultiModifier
{
protected:
	virtual hsBool IEval(double secs, hsScalar del, UInt32 dirty) {return true;}
	char * fAnimName;		// the name of the animation associated with this one-shot
	hsBool fDrivable;		// whether the user can control the position of the animation
	hsBool fReversable;		// whether the user can back up the animation (fDrivable must be true as well)
	float fSeekDuration;	// how long to take to get to the seek point (??? should this be speed instead?)
	float fSmartSeek;		// use smart seek to walk to the seek point?
	hsBool fNoSeek;
public:
	plOneShotMod();
	plOneShotMod(const char *animName, hsBool drivable, hsBool reversable, float seekDuration, hsBool smartSeek,hsBool noSeek = false);
	virtual ~plOneShotMod();
	
	void Init(const char *animName, hsBool drivable, hsBool reversable, float seekDuration, hsBool smartSeek, hsBool noSeek = false);

	CLASSNAME_REGISTER( plOneShotMod );
	GETINTERFACE_ANY( plOneShotMod, plMultiModifier );
	
	virtual void AddTarget(plSceneObject* so);
	hsBool MsgReceive(plMessage* msg);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif
