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
#ifndef PLAVBRAIN_INC
#define PLAVBRAIN_INC

#include "plAGModifier.h"
#include "hsTypes.h"
#include "hsTemplates.h"
#include "hsBitVector.h"
#include "hsGeometry3.h"
#include "hsResMgr.h"

#include "../pnNetCommon/plSynchedObject.h"

#pragma warning(disable: 4284)
#include <deque>

class plArmatureModBase;
class plArmatureBehavior;
class plHKAction;
class plAvTask;
class plAvTaskMsg;
class plDebugText;

class plArmatureBrain : public plCreatable
{
public:
	plArmatureBrain();
	virtual ~plArmatureBrain();
	
	CLASSNAME_REGISTER( plArmatureBrain );
	GETINTERFACE_ANY( plArmatureBrain, plCreatable );	
	
	virtual hsBool Apply(double timeNow, hsScalar elapsed);
	virtual void Activate(plArmatureModBase *armature);
	virtual void Deactivate() {}
	virtual void Suspend() {}
	virtual void Resume() {}
	virtual void Spawn(double timeNow) {}
	virtual void OnBehaviorStop(UInt8 index) {}
	virtual hsBool LeaveAge();
	virtual hsBool IsRunningTask() const;
	virtual void QueueTask(plAvTask *task);
	virtual void DumpToDebugDisplay(int &x, int &y, int lineHeight, char *strBuf, plDebugText &debugTxt) {}
	
	virtual void Write(hsStream *stream, hsResMgr *mgr);
	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual hsBool MsgReceive(plMessage *msg);
	
protected:
	virtual void IProcessTasks(double time, hsScalar elapsed);
	virtual hsBool IHandleTaskMsg(plAvTaskMsg *msg);
		
	typedef std::deque<plAvTask *> plAvTaskQueue;
	plAvTaskQueue			fTaskQueue;						// FIFO queue of tasks we're working on
	plAvTask				*fCurTask;						// the task we're working on right now
	
	plArmatureModBase *fArmature;
	plArmatureMod *fAvMod;
	hsTArray<plArmatureBehavior*> fBehaviors;
};


#endif
