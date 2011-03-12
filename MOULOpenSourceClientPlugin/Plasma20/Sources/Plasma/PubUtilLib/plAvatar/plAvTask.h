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
#ifndef plAvTask_h
#define plAvTask_h

/////////////////////////////////////////////////////////////////////////////////////////
//
// INCLUDES
//
/////////////////////////////////////////////////////////////////////////////////////////
// base class
#include "../pnFactory/plCreatable.h"

/////////////////////////////////////////////////////////////////////////////////////////
//
// DECLARATIONS
//
/////////////////////////////////////////////////////////////////////////////////////////
class plArmatureMod;
class plArmatureBrain;
class plDebugText;

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEFINITIONS
//
/////////////////////////////////////////////////////////////////////////////////////////
/** \class plAvTask
	Abstract class for avatar tasks
	A task is an activity which completely occupies the avatar from beginning to end.
	Only one task can run at a time.
	Tasks are queued within a given avatar brain; you can queue several at once, and they'll
	run in succession, with each waiting for the former to complete before running.
	Some tasks are instantaneous (like attaching an animation,) and only use the task model for
	sequencing effect. (i.e., walk here, attach this animation, etc.)
	If your task requires user intervention or control, you probably should
	be using an avatar brain instead. Tasks are meant to sort of run on their own.
*/
class plAvTask : public plCreatable
{
public:
	plAvTask();

	/** Start the task: set up initial conditions or wait for resources to become available.
		Start will be called repeatedly until it returns true, indicating the task has begun. */
	virtual hsBool Start(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);

	/** Run the task. Start is guaranteed to have returned true before Process() is called even once.
		Returns false when the task has finished and epilogue code has been executed. */
	virtual hsBool Process(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);

	/** Clean up the task. This is guaranteed to be called when Process returns false. */
	virtual void Finish(plArmatureMod *avatar, plArmatureBrain *brain, double time, hsScalar elapsed);

	virtual void LeaveAge(plArmatureMod *avatar) {}
	
	/** dump descriptive stuff to the given debug text */
	virtual void DumpDebug(const char *name, int &x, int&y, int lineHeight, char *strBuf, plDebugText &debugTxt);

	// plasma protocol
	CLASSNAME_REGISTER( plAvTask );
	GETINTERFACE_ANY( plAvTask, plCreatable );

	/** Read the task from a stream. Not all tasks need to read/write, so the base implementation
		gives a warning to expose tasks that are being read/written unexpectedly. */
	virtual void Read(hsStream *stream, hsResMgr *mgr);

	/** Write the task to a stream. Not all tasks need to read/write, so the base implementation
		gives a warning to expose tasks that are being read/written unexpectedly. */
	virtual void Write(hsStream *stream, hsResMgr *mgr);
	
protected:
	virtual void ILimitPlayersInput(plArmatureMod *avatar);
	virtual void IUndoLimitPlayersInput(plArmatureMod *avatar);
};

#endif