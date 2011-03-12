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
#include "plAnimationEventConditionalObject.h"
#include "../pnInputCore/plKeyDef.h"
#include "../plModifier/plSimpleModifier.h"
#include "../../NucleusLib/pnModifier/plLogicModBase.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../plMessage/plAnimCmdMsg.h"

plAnimationEventConditionalObject::plAnimationEventConditionalObject(plKey pTargetModifier) :
fTarget(pTargetModifier),
fAction(kEventEnd)
{
}

hsBool plAnimationEventConditionalObject::MsgReceive(plMessage* msg)
{
	plEventCallbackMsg* pMsg = plEventCallbackMsg::ConvertNoRef(msg);
	if (pMsg)
	{
		SetSatisfied(true);
//		fLogicMod->RequestTrigger();
		return true;
	}
	return plConditionalObject::MsgReceive(msg);
}
	

void plAnimationEventConditionalObject::SetEvent(const CallbackEvent b, hsScalar time)
{
	plAnimCmdMsg* pMsg = TRACKED_NEW plAnimCmdMsg;
	pMsg->AddReceiver(fTarget);
	pMsg->SetSender( GetKey() );

	plEventCallbackMsg* cb = TRACKED_NEW plEventCallbackMsg(GetKey(), b, 0, time);

	pMsg->AddCallback( cb );
	hsRefCnt_SafeUnRef(cb);
	pMsg->SetCmd( plAnimCmdMsg::kAddCallbacks );
	
	plgDispatch::MsgSend( pMsg );
}

void plAnimationEventConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Read(stream, mgr);
	fTarget = mgr->ReadKey(stream);
	fAction = (CallbackEvent)stream->ReadSwap32();
}

void plAnimationEventConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Write(stream, mgr);
	mgr->WriteKey(stream, fTarget);
	stream->WriteSwap32(fAction); 
}
	
