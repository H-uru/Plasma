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

#include "hsTypes.h"
#include "plSimpleModifier.h"
#include "plgDispatch.h"

#include "hsStream.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plRefMsg.h"
#include "hsTimer.h"
// #include "../pfConditional/plAnimationEventConditionalObject.h"
#include "../plMessage/plAnimCmdMsg.h"

plSimpleModifier::plSimpleModifier()
: 
	fTarget(nil)
{
	fTimeConvert.SetOwner(this);
}

plSimpleModifier::~plSimpleModifier()
{
	if( !fTimeConvert.IsStopped() )
		IEnd();
}

void plSimpleModifier::Read(hsStream* s, hsResMgr* mgr)
{
	plModifier::Read(s, mgr);

	fTimeConvert.Read(s, mgr);

	if( !fTimeConvert.IsStopped() )
		IBegin();		// TEMP TILL Message causes IBEGIN
}

void plSimpleModifier::Write(hsStream* s, hsResMgr* mgr)
{
	plModifier::Write(s, mgr);

	fTimeConvert.Write(s, mgr);
}

void plSimpleModifier::AddTarget(plSceneObject* o)
{
	fTarget = o;
	if( !fTimeConvert.IsStopped() )
		IBegin();		// TEMP TILL Message causes IBEGIN
}

void plSimpleModifier::RemoveTarget(plSceneObject* o)
{
	hsAssert(o == fTarget, "Removing target I don't have");
	fTarget = nil;
}

void plSimpleModifier::IBegin()
{
	if( fTarget )
	{
		fTimeConvert.Start();
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
		
	}
}

void plSimpleModifier::IEnd()
{
	fTimeConvert.Stop();
	if( fTarget )
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
	}
}

hsBool plSimpleModifier::IEval(double secs, hsScalar del, UInt32 dirty)
{
	return IHandleTime(secs, del);
}

hsBool plSimpleModifier::MsgReceive(plMessage* msg)
{
	plRefMsg* refMsg = plRefMsg::ConvertNoRef(msg);
	if( refMsg )
	{
		return IHandleRef(refMsg);
	}
	plAnimCmdMsg* modMsg = plAnimCmdMsg::ConvertNoRef(msg);
	if( modMsg )
	{
		return IHandleCmd(modMsg);
	}
	return plModifier::MsgReceive(msg);
}

hsBool plSimpleModifier::IHandleRef(plRefMsg* refMsg)
{
	if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
		AddTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));
	else
		RemoveTarget(plSceneObject::ConvertNoRef(refMsg->GetRef()));

	return true;
}

hsBool plSimpleModifier::IHandleCmd(plAnimCmdMsg* modMsg)
{
	hsBool wasStopped = fTimeConvert.IsStopped();

	fTimeConvert.HandleCmd(modMsg);

	hsBool isStopped = fTimeConvert.IsStopped();

	if( wasStopped != isStopped )
	{
		if( isStopped )
		{
			IEnd();
		}
		else
		{
			IBegin();
		}
	}

#if 0	// debug
	char str[256];
		sprintf(str, "ModHandleCmd: time=%f, ts=%f FWD=%d, BWD=%d, SpeedChange=%d sp=%f, CONT=%d, STOP=%d\n",
			hsTimer::GetSysSeconds(),
			modMsg->GetTimeStamp(),
			modMsg->Cmd(plAnimCmdMsg::kSetForewards),
			modMsg->Cmd(plAnimCmdMsg::kSetBackwards),
			modMsg->Cmd(plAnimCmdMsg::kSetSpeed),
			modMsg->fSpeed,
			modMsg->Cmd(plAnimCmdMsg::kContinue),
			modMsg->Cmd(plAnimCmdMsg::kStop));
		hsStatusMessage(str);
#endif
	return true;
}	

hsBool plSimpleModifier::IHandleTime(double wSecs, hsScalar del)
{

	if( !fTarget )
		return true;

	hsScalar secs = fTimeConvert.WorldToAnimTime(wSecs);
	
	if( secs != fCurrentTime )
	{
		fCurrentTime = secs;

		IApplyDynamic();
	}

	return true;
}


