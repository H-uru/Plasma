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
#include "plObjectInBoxConditionalObject.h"
#include "../../PubUtilLib/plPhysical/plDetectorModifier.h"
#include "../../NucleusLib/pnModifier/plLogicModBase.h"
#include "../../PubUtilLib/plMessage/plActivatorMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plFakeOutMsg.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plAvatar/plArmatureMod.h"
#include "../pnSceneObject/plSceneObject.h"

bool plVolumeSensorConditionalObject::makeBriceHappyVar = true;

plObjectInBoxConditionalObject::plObjectInBoxConditionalObject() :
fCurrentTrigger(nil)
{
	SetSatisfied(true);
}

hsBool plObjectInBoxConditionalObject::MsgReceive(plMessage* msg)
{
	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pActivateMsg)
	{
		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
		{
			fInside.Append(pActivateMsg->fHitterObj);
			return true;
		}
		else
		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeExit)
		{
			for (int i = 0; i < fInside.Count(); i++)
			{
				if (fInside[i] == pActivateMsg->fHitterObj)
				{
					fInside.Remove(i);
					if (pActivateMsg->fHitterObj == fCurrentTrigger && fCurrentTrigger && fLogicMod->HasFlag(plLogicModBase::kTriggered) && !IsToggle())
					{
						fCurrentTrigger = nil;
						fLogicMod->GetNotify()->AddContainerEvent( pActivateMsg->fHiteeObj, pActivateMsg->fHitterObj, false );
						fLogicMod->RequestUnTrigger();
					}
				}
			}
			return true;
		}

		return false;
	}
	return plConditionalObject::MsgReceive(msg);
}

hsBool plObjectInBoxConditionalObject::Verify(plMessage* msg)
{
	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pActivateMsg)
	{	
		for (int i = 0; i < fInside.Count(); i++)
		{
			if (pActivateMsg->fHitterObj == fInside[i])
			{
				fLogicMod->GetNotify()->AddContainerEvent( pActivateMsg->fHiteeObj, pActivateMsg->fHitterObj, true );
				fCurrentTrigger = pActivateMsg->fHiteeObj;
				return true;
			}
		}
	}

	plFakeOutMsg* pFakeMsg = plFakeOutMsg::ConvertNoRef(msg);
	if (pFakeMsg && plNetClientApp::GetInstance()->GetLocalPlayerKey())
	{
		for (int i = 0; i < fInside.Count(); i++)
		{
			if (plNetClientApp::GetInstance()->GetLocalPlayerKey() == fInside[i])
				return true;
		}
	}
	return false;
}


//
// volume sensor conditional 
//
plVolumeSensorConditionalObject::plVolumeSensorConditionalObject() :
fTrigNum(-1),
fType(0),
fFirst(false),
fTriggered(false),
fIgnoreExtraEnters(true)
{
	SetSatisfied(true);
}


hsBool plVolumeSensorConditionalObject::MsgReceive(plMessage* msg)
{
	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pActivateMsg)
	{
		// single player hack
 		if (!fLogicMod->HasFlag(plLogicModBase::kRequestingTrigger))
			fLogicMod->GetNotify()->ClearEvents();

		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
		{
			int i;
			for (i = 0; i < fInside.Count(); i++)
			{
				if (fInside[i] == pActivateMsg->fHitterObj)
				{
					if (fIgnoreExtraEnters)
						return false; // this is the "correct" way to handle this situation
					break; // this is for those special situations where, due to some physics oddity, we need to allow the avatar to enter without exiting
				}
			}
			if (i == fInside.Count())
				fInside.Append(pActivateMsg->fHitterObj);
			if (makeBriceHappyVar)
			{
				plSceneObject *pObj = plSceneObject::ConvertNoRef( pActivateMsg->fHitterObj->ObjectIsLoaded() );
				if( pObj )
				{
					//need to check for human vs quabish type things in here				
					int i;
					for( i = 0; i < pObj->GetNumModifiers(); i++ )
					{
						if (plArmatureMod::ConvertNoRef( pObj->GetModifier(i)))
						{	
							if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pActivateMsg->fHitterObj)
							{
								plArmatureMod *am=const_cast<plArmatureMod*>( plArmatureMod::ConvertNoRef(pObj->GetModifier(i)));
								if((am->IsLocalAI())==nil)
								{
									return false;
								}
							}
						}
					}
					plSynchedObject* syncObj = (plSynchedObject*)pObj;
					if (syncObj->IsLocallyOwned() != plSynchedObject::kYes)	
					{
						return false;
					}
				}
			}

			if (fType == kTypeEnter)
			{
				fLogicMod->GetNotify()->AddCollisionEvent(true, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
				fLogicMod->RequestTrigger(false);
			}
			else
			{
				fLogicMod->GetNotify()->AddCollisionEvent(false, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
				fLogicMod->RequestUnTrigger();
			}
			return false;
		}
		else
		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeExit)
		{
			for (int i = 0; i < fInside.Count(); i++)
			{
				if (fInside[i] == pActivateMsg->fHitterObj)
				{
					fInside.Remove(i);
					if (makeBriceHappyVar)
					{
					//need to check for human vs quabish type things in here	
						plSceneObject *pObj = plSceneObject::ConvertNoRef( pActivateMsg->fHitterObj->ObjectIsLoaded() );
						if( pObj )
						{
							int i;
							for( i = 0; i < pObj->GetNumModifiers(); i++ )
							{
								if (plArmatureMod::ConvertNoRef( pObj->GetModifier(i)))
								{	
									if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pActivateMsg->fHitterObj)
									{
										plArmatureMod *am=const_cast<plArmatureMod*>( plArmatureMod::ConvertNoRef(pObj->GetModifier(i)));
										if((am->IsLocalAI())==nil)
										{
											return false;
										}
									}
								}
							}
							plSynchedObject* syncObj = (plSynchedObject*)pObj;
							if (syncObj->IsLocallyOwned() != plSynchedObject::kYes)	
							{
								return false;
							}
						}
					}
					if (fType == kTypeExit)
					{
						fLogicMod->GetNotify()->AddCollisionEvent(false, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
						fLogicMod->RequestTrigger(false);
	
					}
					else
					{
						fLogicMod->GetNotify()->AddCollisionEvent(true, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
						fLogicMod->RequestUnTrigger();
					}
					return false;
				}
			}
		}

		return false;
	}
	return plConditionalObject::MsgReceive(msg);
}

hsBool plVolumeSensorConditionalObject::Satisfied()
{
	if (fType == kTypeExit && fFirst && !fTriggered)
	{
		if (fInside.Count())
			fTriggered = true;
		return true;
	}
	if (fTriggered) 
	{
		if (fInside.Count() == 0)
			fTriggered = false;
		return false;
	}

	if (fTrigNum == -1)
		return true;
	if (fInside.Count() == fTrigNum)
		return true;
	else
		return false;
}

void plVolumeSensorConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Read(stream, mgr);
	fTrigNum = stream->ReadSwap32();
	fType = stream->ReadSwap32();
	fFirst = stream->ReadBool();
}
void plVolumeSensorConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
	plConditionalObject::Write(stream, mgr);
	stream->WriteSwap32(fTrigNum);
	stream->WriteSwap32(fType);
	stream->WriteBool(fFirst);
}
#include "../pnMessage/plPlayerPageMsg.h"
#include "../../NucleusLib/inc/plgDispatch.h"
hsBool plVolumeSensorConditionalObjectNoArbitration::MsgReceive(plMessage* msg)
{
	plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
	if (pActivateMsg)
	{
		// single player hack
 		if (!fLogicMod->HasFlag(plLogicModBase::kRequestingTrigger))
			fLogicMod->GetNotify()->ClearEvents();
		fHittee= pActivateMsg->fHiteeObj;
		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
		{
			int i;
			for (i = 0; i < fInside.Count(); i++)
			{
				if (fInside[i] == pActivateMsg->fHitterObj)
				{
					if (fIgnoreExtraEnters)
						return false; // this is the "correct" way to handle this situation
					break; // this is for those special situations where, due to some physics oddity, we need to allow the avatar to enter without exiting
				}
			}
			if (i == fInside.Count())
				fInside.Append(pActivateMsg->fHitterObj);
			if (makeBriceHappyVar)
			{
				plSceneObject *pObj = plSceneObject::ConvertNoRef( pActivateMsg->fHitterObj->ObjectIsLoaded() );
				if( pObj )
				{
					//need to check for human vs quabish type things in here				
					int i;
					for( i = 0; i < pObj->GetNumModifiers(); i++ )
					{
						if (plArmatureMod::ConvertNoRef( pObj->GetModifier(i)))
						{	
							if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pActivateMsg->fHitterObj)
							{
								plArmatureMod *am=const_cast<plArmatureMod*>( plArmatureMod::ConvertNoRef(pObj->GetModifier(i)));
								if((am->IsLocalAI())==nil)
								{
									return false;
								}
							}
						}
					}
					plSynchedObject* syncObj = (plSynchedObject*)pObj;
					if (syncObj->IsLocallyOwned() != plSynchedObject::kYes)	
					{
						return false;
					}
				}
			}

			if (fType == kTypeEnter)
			{
				fLogicMod->GetNotify()->AddCollisionEvent(true, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
				//fLogicMod->RequestTrigger(false);
				
				if (!Satisfied())
					return false;
				
				fLogicMod->Trigger(false);
			}
		
			return false;
		}
		else
		if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeExit)
		{
			for (int i = 0; i < fInside.Count(); i++)
			{
				if (fInside[i] == pActivateMsg->fHitterObj)
				{
					fInside.Remove(i);
					if (makeBriceHappyVar)
					{
					//need to check for human vs quabish type things in here	
						plSceneObject *pObj = plSceneObject::ConvertNoRef( pActivateMsg->fHitterObj->ObjectIsLoaded() );
						if( pObj )
						{
							int i;
							for( i = 0; i < pObj->GetNumModifiers(); i++ )
							{
								if (plArmatureMod::ConvertNoRef( pObj->GetModifier(i)))
								{	
									if (plNetClientApp::GetInstance()->GetLocalPlayerKey() != pActivateMsg->fHitterObj)
									{
										plArmatureMod *am=const_cast<plArmatureMod*>( plArmatureMod::ConvertNoRef(pObj->GetModifier(i)));
										if((am->IsLocalAI())==nil)
										{
											return false;
										}
									}
								}
							}
							plSynchedObject* syncObj = (plSynchedObject*)pObj;
							if (syncObj->IsLocallyOwned() != plSynchedObject::kYes)	
							{
								return false;
							}
						}
					}
					if (fType == kTypeExit)
					{
						fLogicMod->GetNotify()->AddCollisionEvent(false, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
						//fLogicMod->RequestTrigger(false);
						if (!Satisfied())
							return false;
					
						fLogicMod->Trigger(false);
					}
					return false;
				}
			}
		}

		return false;
	}

	plPlayerPageMsg* page = plPlayerPageMsg::ConvertNoRef(msg);
	if(page && page->fUnload)
	{
		for(int j= 0; j< fInside.Count(); j++)
		{
			if(fInside[j] == page->fPlayer)
			{//this is the one inside
				if(fHittee)
				{
					plSceneObject *so = plSceneObject::ConvertNoRef(fHittee->ObjectIsLoaded());
					if(so && so->IsLocallyOwned())
					{
						if (fType == kTypeExit)
						{
							fLogicMod->GetNotify()->AddCollisionEvent(false, page->fPlayer, fHittee, false);
							//fLogicMod->RequestTrigger(false);
							if (!Satisfied())
								return false;
							fLogicMod->Trigger(false);
						}
					}
				}
				fInside.Remove(j);
			}
		}
	}
		return plConditionalObject::MsgReceive(msg);
}
void plVolumeSensorConditionalObjectNoArbitration::Read(hsStream* stream, hsResMgr* mgr)
{
	plVolumeSensorConditionalObject::Read(stream, mgr);
	plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
}