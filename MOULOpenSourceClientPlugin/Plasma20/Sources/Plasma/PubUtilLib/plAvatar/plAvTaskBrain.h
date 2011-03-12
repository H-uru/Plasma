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
#ifndef plAvTaskBrain_h
#define plAvTaskBrain_h

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
#include "plAvTask.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

/** \class plAvTaskBrain
	Push a new brain onto the avatar or pull an old brain off.
	If a brain is supplied, it's a push. Otherwise it's a pull.
*/
class plAvTaskBrain : public plAvTask
{
public:
	/** Default constructor. Used as is, will function as a pop brain message. */
	plAvTaskBrain();
	/** Constructor for a push configuration. If the brain is non, nil, it 
		will be pushed onto the receiving avatar when the task is dequeued.
		If the brain is nil, the current brain will be popped. */
	plAvTaskBrain(plArmatureBrain *brain);
	virtual ~plAvTaskBrain();

	// task protocol
	virtual hsBool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);
	virtual void Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);
		
	/** dump descriptive stuff to the given debug text */
	virtual void DumpDebug(const char *name, int &x, int&y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	plArmatureBrain *GetBrain();

	CLASSNAME_REGISTER( plAvTaskBrain );
	GETINTERFACE_ANY( plAvTaskBrain, plAvTask );

	/** Read the task from a stream. Not all tasks need to read/write, so the base implementation
		gives a warning to expose tasks that are being read/written unexpectedly. */
	virtual void Read(hsStream *stream, hsResMgr *mgr);

	/** Write the task to a stream. Not all tasks need to read/write, so the base implementation
		gives a warning to expose tasks that are being read/written unexpectedly. */
	virtual void Write(hsStream *stream, hsResMgr *mgr);
protected:
	plArmatureBrain *fBrain;
};

#endif