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
// local
#include "plAvBrain.h"
#include "plAvBehaviors.h"
#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plAvatarTasks.h"

// global
#include "hsGeometry3.h"
#include "hsQuat.h"

// other
#include "../pnSceneObject/plSceneObject.h"
#include "../plPipeline/plDebugText.h"

// messages
#include "../plMessage/plAvatarMsg.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

plArmatureBrain::plArmatureBrain() :
	fCurTask(nil),
	fArmature(nil),
	fAvMod(nil)
{
}

plArmatureBrain::~plArmatureBrain()
{
	while (fTaskQueue.size() > 0)
	{
		plAvTask *task = fTaskQueue.front();
		delete task;
		fTaskQueue.pop_front();
	}
	if (fCurTask)
		delete fCurTask;	
}

hsBool plArmatureBrain::Apply(double timeNow, hsScalar elapsed)
{
	IProcessTasks(timeNow, elapsed);
	fArmature->ApplyAnimations(timeNow, elapsed);
	
	return true;
}

void plArmatureBrain::Activate(plArmatureModBase *armature)
{ 
	fArmature = armature;
	fAvMod = plArmatureMod::ConvertNoRef(armature);
}

void plArmatureBrain::QueueTask(plAvTask *task)
{
	if (task)
		fTaskQueue.push_back(task);
}

hsBool plArmatureBrain::LeaveAge()
{
	if (fCurTask)
		fCurTask->LeaveAge(plArmatureMod::ConvertNoRef(fArmature));
	
	plAvTaskQueue::iterator i = fTaskQueue.begin();
	for (; i != fTaskQueue.end(); i++)
	{
		plAvTask *task = *i;
		task->LeaveAge(plArmatureMod::ConvertNoRef(fArmature)); // Give it a chance to do something before we nuke it.
		delete task;
	}
	fTaskQueue.clear();
	return true;
}
	
hsBool plArmatureBrain::IsRunningTask() const
{
	if (fCurTask)
		return true;
	if(fTaskQueue.size() > 0)
		return true;

	return false;
}

// Nothing for this class to read/write. These methods exist
// for backwards compatability with plAvBrain and plAvBrainUser
void plArmatureBrain::Write(hsStream *stream, hsResMgr *mgr)
{
	plCreatable::Write(stream, mgr);
	
	// plAvBrain
	stream->WriteSwap32(0);
	stream->WriteBool(false);

	// plAvBrainUser
	stream->WriteSwap32(0);
	stream->WriteSwapScalar(0.f);
	stream->WriteSwapDouble(0.f);	
}

void plArmatureBrain::Read(hsStream *stream, hsResMgr *mgr)
{
	plCreatable::Read(stream, mgr);

	// plAvBrain
	stream->ReadSwap32();
	if (stream->ReadBool()) 
		mgr->ReadKey(stream);

	// plAvBrainUser
	stream->ReadSwap32();
	stream->ReadSwapScalar();
	stream->ReadSwapDouble();
}

// MSGRECEIVE
hsBool plArmatureBrain::MsgReceive(plMessage * msg)
{
	plAvTaskMsg *taskMsg = plAvTaskMsg::ConvertNoRef(msg);
	if (taskMsg)
	{
		return IHandleTaskMsg(taskMsg);
	}
	return false;
}

void plArmatureBrain::IProcessTasks(double time, hsScalar elapsed)
{
	if (!fCurTask || !fCurTask->Process(plArmatureMod::ConvertNoRef(fArmature), this, time, elapsed))
	{
		if (fCurTask)
		{
			fCurTask->Finish(plArmatureMod::ConvertNoRef(fArmature), this, time, elapsed);
			delete fCurTask;
			fCurTask = nil;
		}
		
		// need a new task
		if (fTaskQueue.size() > 0)
		{
			plAvTask *newTask = fTaskQueue.front();
			if (newTask && newTask->Start(plArmatureMod::ConvertNoRef(fArmature), this, time, elapsed))
			{
				fCurTask = newTask;
				fTaskQueue.pop_front();
			}
			// if we couldn't start the task, we'll keep trying until we can.
		}
	}
}

hsBool plArmatureBrain::IHandleTaskMsg(plAvTaskMsg *msg)
{
	plAvTask *task = msg->GetTask();
	QueueTask(task);
	return true;
}