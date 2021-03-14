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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
// local
#include "plAvBrain.h"

#include "plArmatureMod.h"
#include "plAvatarTasks.h"
#include "plAvBehaviors.h"

// other
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plAvatarMsg.h"
#include "plPipeline/plDebugText.h"

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

plArmatureBrain::plArmatureBrain() :
    fCurTask(),
    fArmature(),
    fAvMod()
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

bool plArmatureBrain::Apply(double timeNow, float elapsed)
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

bool plArmatureBrain::LeaveAge()
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
    
bool plArmatureBrain::IsRunningTask() const
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
    stream->WriteLE32(0);
    stream->WriteBool(false);

    // plAvBrainUser
    stream->WriteLE32(0);
    stream->WriteLEFloat(0.f);
    stream->WriteLEDouble(0.0);
}

void plArmatureBrain::Read(hsStream *stream, hsResMgr *mgr)
{
    plCreatable::Read(stream, mgr);

    // plAvBrain
    (void)stream->ReadLE32();
    if (stream->ReadBool()) 
        (void)mgr->ReadKey(stream);

    // plAvBrainUser
    (void)stream->ReadLE32();
    (void)stream->ReadLEFloat();
    (void)stream->ReadLEDouble();
}

// MSGRECEIVE
bool plArmatureBrain::MsgReceive(plMessage * msg)
{
    plAvTaskMsg *taskMsg = plAvTaskMsg::ConvertNoRef(msg);
    if (taskMsg)
    {
        return IHandleTaskMsg(taskMsg);
    }
    return false;
}

void plArmatureBrain::IProcessTasks(double time, float elapsed)
{
    if (!fCurTask || !fCurTask->Process(plArmatureMod::ConvertNoRef(fArmature), this, time, elapsed))
    {
        if (fCurTask)
        {
            fCurTask->Finish(plArmatureMod::ConvertNoRef(fArmature), this, time, elapsed);
            delete fCurTask;
            fCurTask = nullptr;
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

bool plArmatureBrain::IHandleTaskMsg(plAvTaskMsg *msg)
{
    plAvTask *task = msg->GetTask();
    QueueTask(task);
    return true;
}