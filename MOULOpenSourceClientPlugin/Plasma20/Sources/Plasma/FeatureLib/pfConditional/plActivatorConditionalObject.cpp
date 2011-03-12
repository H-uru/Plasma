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
#include "plActivatorConditionalObject.h"
#include "../plPhysical/plDetectorModifier.h"
#include "../../NucleusLib/pnModifier/plLogicModBase.h"
#include "../plMessage/plActivatorMsg.h"
#include "../pnMessage/plNotifyMsg.h"


plActivatorConditionalObject::plActivatorConditionalObject()
{
	SetFlag(kLocalElement);		// since it relies on user input
}

hsBool plActivatorConditionalObject::MsgReceive(plMessage* msg)
{
	plActivatorMsg* pDetectorMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pDetectorMsg)
	{
		if (pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeEnter || pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeExit)
			return plConditionalObject::MsgReceive(msg);

		for (int i = 0; i < fActivators.Count(); i++)
		{
			if (pDetectorMsg && pDetectorMsg->GetSender() == fActivators[i])
			{
				if (pDetectorMsg->fTriggerType == plActivatorMsg::kUnPickedTrigger ||
					pDetectorMsg->fTriggerType == plActivatorMsg::kExitUnTrigger   ||
					pDetectorMsg->fTriggerType == plActivatorMsg::kEnterUnTrigger  ||
					pDetectorMsg->fTriggerType == plActivatorMsg::kCollideUnTrigger )
					
				{
					// is this a toggle activator?
					if (IsToggle())
						return true;

					// are we in a triggered state?
					if (fLogicMod->HasFlag(plLogicModBase::kTriggered))
					{
						if (pDetectorMsg->fTriggerType == plActivatorMsg::kUnPickedTrigger)
							fLogicMod->GetNotify()->AddPickEvent(pDetectorMsg->fHitterObj, pDetectorMsg->fPickedObj, false, pDetectorMsg->fHitPoint);

						if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideExit) ||
							(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeExit)  )
							fLogicMod->GetNotify()->AddCollisionEvent(true, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
					
						if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideEnter) ||
							(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeEnter) )
							fLogicMod->GetNotify()->AddCollisionEvent(false, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);

						fLogicMod->RequestUnTrigger();
					}
				}
				else
				{
					// are we a toggle that has been triggered?
					// are we in a triggered state?
					if (fLogicMod->HasFlag(plLogicModBase::kTriggered) && IsToggle())
					{
						if (pDetectorMsg->fTriggerType == plActivatorMsg::kUnPickedTrigger)
							fLogicMod->GetNotify()->AddPickEvent(pDetectorMsg->fHitterObj, pDetectorMsg->fPickedObj, false, pDetectorMsg->fHitPoint);

						if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideExit) ||
							(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeExit)  )
							fLogicMod->GetNotify()->AddCollisionEvent(true, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
					
						if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideEnter) ||
							(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeEnter) )
							fLogicMod->GetNotify()->AddCollisionEvent(false, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);

						fLogicMod->RequestUnTrigger();
						return true;
					}


					if (!fLogicMod->VerifyConditions( pDetectorMsg ))
						return false;
					// this is used as part of a picked detector sometimes...
					if (pDetectorMsg->fTriggerType == plActivatorMsg::kPickedTrigger)
						fLogicMod->GetNotify()->AddPickEvent(pDetectorMsg->fHitterObj, pDetectorMsg->fPickedObj, true, pDetectorMsg->fHitPoint);

					if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideExit) ||
						(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeExit)  )
					{
						fLogicMod->GetNotify()->AddCollisionEvent(false, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
					}
					if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideEnter) ||
						(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeEnter) )
					{
						fLogicMod->GetNotify()->AddCollisionEvent(true, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
					}
					SetSatisfied(true);
					//hsBool netRequest = msg->HasBCastFlag(plMessage::kNetNonLocal);
					//fLogicMod->RequestTrigger(netRequest);
					fLogicMod->RequestTrigger(false);
				}
				return true;
			}
		}
	}
	return plConditionalObject::MsgReceive(msg);
}

void plActivatorConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Read(stream, mgr);
	fActivators.Reset();
	int n = stream->ReadSwap32();
	for (int i = 0; i < n; i++)
		fActivators.Append(mgr->ReadKey(stream));
}
void plActivatorConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Write(stream, mgr);
	stream->WriteSwap32(fActivators.Count());
	for (int i = 0; i < fActivators.Count(); i++)
		mgr->WriteKey(stream, fActivators[i]);
}

void plActivatorConditionalObject::SetActivatorKey(plKey k)
{
	fActivators.Append(k);
}

//
// plActivatorActivatorConditional
//

hsBool plActivatorActivatorConditionalObject::MsgReceive(plMessage* msg)
{
	plNotifyMsg* pDetectorMsg = plNotifyMsg::ConvertNoRef(msg);
	if (pDetectorMsg)
	{
		if (!pDetectorMsg->fState && fLogicMod->HasFlag(plLogicModBase::kTriggered))
		{
			fLogicMod->RequestUnTrigger();
		}
		else
		if (pDetectorMsg->fState && !fLogicMod->HasFlag(plLogicModBase::kTriggered))
		{
			if (!fLogicMod->VerifyConditions( pDetectorMsg ))
				return false;

			SetSatisfied(true);
			fLogicMod->RequestTrigger(false);
		}
		return true;
	}
	return plConditionalObject::MsgReceive(msg);
}


hsBool plVolActivatorConditionalObject::MsgReceive(plMessage* msg)
{
	plActivatorMsg* pDetectorMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pDetectorMsg)
	{
		for (int i = 0; i < fActivators.Count(); i++)
		{
			if (!fLogicMod->VerifyConditions( pDetectorMsg ))
				return false;

			if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideExit) ||
				(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeExit)  )
			{
				fLogicMod->GetNotify()->AddCollisionEvent(false, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
			}
			if ((pDetectorMsg->fTriggerType == plActivatorMsg::kCollideEnter) ||
				(pDetectorMsg->fTriggerType == plActivatorMsg::kVolumeEnter) )
			{
				fLogicMod->GetNotify()->AddCollisionEvent(true, pDetectorMsg->fHitterObj, pDetectorMsg->fHiteeObj);
			}
			SetSatisfied(true);
			fLogicMod->RequestTrigger(false);
		}
		return true;
	}
	return plConditionalObject::MsgReceive(msg);
}


