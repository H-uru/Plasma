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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	plSceneInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsConfig.h"
#include "hsWindows.h"

#include "hsTypes.h"
#include "plSceneInputInterface.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "../plPhysical/plPickingDetector.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plLOSRequestMsg.h"
#include "../plMessage/plLOSHitMsg.h"
#include "../plMessage/plPickedMsg.h"
#include "../plMessage/plRenderMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../plMessage/plVaultNotifyMsg.h"
#include "../pnMessage/plFakeOutMsg.h"
#include "../pnMessage/plNotifyMsg.h"
#include "../pnMessage/plRemoteAvatarInfoMsg.h"
#include "../pnMessage/plCursorChangeMsg.h"
#include "../pnMessage/plCameraMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnMessage/plCmdIfaceModMsg.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvBrain.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plAvCallbackAction.h"
#include "../plModifier/plInterfaceInfoModifier.h"
#include "../pnModifier/plLogicModBase.h"
#include "../plVault/plVault.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plNetCommon/plNetServerSessionInfo.h"
#include "../plNetTransport/plNetTransport.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnInputCore/plKeyMap.h"
#include "plPhysical.h"

#include "plgDispatch.h"
#include "plPipeline.h"

#include "../plModifier/plDetectorLog.h"


#define ID_FIND_CLICKABLE 2
#define ID_FIND_LOCALPLAYER 3
#define ID_FIND_WALKABLE_GROUND 4

#define SHARE_FACING_TOLERANCE -0.70f		// 45 degrees

plSceneInputInterface *plSceneInputInterface::fInstance = nil;

hsBool plSceneInputInterface::fShowLOS = false;


//// Constructor/Destructor //////////////////////////////////////////////////

plSceneInputInterface::plSceneInputInterface()
{
	fPipe = nil;
	fSpawnPoint = nil;
	GuidClear(&fAgeInstanceGuid);
	fInstance = this;
	SetEnabled( true );			// Always enabled
}

plSceneInputInterface::~plSceneInputInterface()
{
	ClearClickableMap();
	fIgnoredAvatars.Reset();
	fLocalIgnoredAvatars.Reset();
	fGUIIgnoredAvatars.Reset();
	fInstance = nil;
}


//// Init/Shutdown ///////////////////////////////////////////////////////////

void	plSceneInputInterface::Init( plInputInterfaceMgr *manager )
{
	plInputInterface::Init( manager );

	// To get the pipeline
	fPipe = nil;
	plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plInputIfaceMgrMsg::Index(), fManager->GetKey() );
	plgDispatch::Dispatch()->RegisterForExactType( plPlayerPageMsg::Index(), fManager->GetKey() );

	fCurrentClickable = nil;
	fCurrentClickableLogicMod = nil;
	fLastClicked = nil;
	fButtonState = 0;
	fClickability = 1;		// Hack for clickable avatars, we need to always do the LOS check
	fLastClickIsAvatar = false;
	fCurrClickIsAvatar = false;
	fFadedLocalAvatar = false;
	fBookMode = kNotOffering;
	fOffereeKey = nil;
	fPendingLink = false;

	// register for control messages
	plCmdIfaceModMsg* pModMsg = TRACKED_NEW plCmdIfaceModMsg;
	pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
	pModMsg->SetSender(fManager->GetKey());
	pModMsg->SetCmd(plCmdIfaceModMsg::kAdd);
	plgDispatch::MsgSend(pModMsg);

}

void	plSceneInputInterface::Shutdown( void )
{
	if( fPipe == nil )
		plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
	else
		fPipe = nil;
}

void plSceneInputInterface::ClearClickableMap()
{
	for (int i = 0; i < fClickableMap.Count(); i++)
	{
		clickableTest* pTest = fClickableMap[i];
		delete(pTest);
	}
	fClickableMap.SetCountAndZero(0);
}

//// IHalfFadeAvatar /////////////////////////////////////////////////////////

void plSceneInputInterface::IHalfFadeAvatar(hsBool out)
{
	plIfaceFadeAvatarMsg* pMsg = TRACKED_NEW plIfaceFadeAvatarMsg();
	pMsg->SetSubjectKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	pMsg->SetBCastFlag(plMessage::kBCastByExactType);
	pMsg->SetBCastFlag(plMessage::kNetPropagate, FALSE);
	pMsg->SetFadeOut(out);
	pMsg->Send();
	fFadedLocalAvatar = out;
	
}


void plSceneInputInterface::ResetClickableState()
{
	if( fLastClicked != nil )
		ISetLastClicked( nil, hsPoint3(0,0,0) );
	
	ClearClickableMap();
	fCurrentClickable = nil;
	fCurrentClickableLogicMod = nil;
	fCurrentCursor = SetCurrentCursorID(kNullCursor);
	fCurrClickIsAvatar = false;
	
}
//// IEval ///////////////////////////////////////////////////////////////////

hsBool plSceneInputInterface::IEval( double secs, hsScalar del, UInt32 dirty )
{
	// this needs to always go no matter what...
	// ...unless we have cliclability disabled (as in the case of certain multistage behaviors)
	if (plMouseDevice::Instance()->GetCursorOpacity() > 0.f && !plMouseDevice::Instance()->GetHideCursor())
	{
		IRequestLOSCheck( plMouseDevice::Instance()->GetCursorX(), plMouseDevice::Instance()->GetCursorY(), ID_FIND_LOCALPLAYER );
		if (fClickability)
			IRequestLOSCheck( plMouseDevice::Instance()->GetCursorX(), plMouseDevice::Instance()->GetCursorY(), ID_FIND_CLICKABLE );
	}
	else
	if (fFadedLocalAvatar)
		IHalfFadeAvatar(false);
	
//	if (!fCurrClickIsAvatar)
//		fCurrentCursor = SetCurrentCursorID(kNullCursor);
	// ping for possible cursor changes
	int i;
	for (i=0; i < fClickableMap.Count(); i++)
	{
		plFakeOutMsg *pMsg = TRACKED_NEW plFakeOutMsg;
		pMsg->SetSender( fManager->GetKey() );
		pMsg->AddReceiver( fClickableMap[i]->key );
		plgDispatch::MsgSend( pMsg );
	}
	// then see if we have any
	hsBool change = false;
	for (i=0; i < fClickableMap.Count(); i++)
	{
		if( fClickableMap[i]->val )
		{
			change = true;
			break;
		}
	}
	if (change)
	{
		if( fLastClicked != nil )
			fCurrentCursor = SetCurrentCursorID(kCursorClicked);
		else
			fCurrentCursor = SetCurrentCursorID(kCursorPoised);
	}
	return true;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	plSceneInputInterface::MsgReceive( plMessage *msg )
{
	plLOSHitMsg *pLOSMsg = plLOSHitMsg::ConvertNoRef( msg );
	if( pLOSMsg )
	{
		if( pLOSMsg->fRequestID == ID_FIND_CLICKABLE )
		{
			hsBool clearCursor = false;
			if (!fClickability)
				return true;
			if( pLOSMsg->fObj )
			{
				// is this object clickable?
				plSceneObject *pObj = plSceneObject::ConvertNoRef( pLOSMsg->fObj->ObjectIsLoaded() );
				if( pObj )
				{
					if (fShowLOS)
					{
						if (pLOSMsg->fNoHit)
							DetectorLogSpecial("%s: LOS miss", pObj->GetKeyName());
						else
							DetectorLogSpecial("%s: LOS hit", pObj->GetKeyName());
					}
					int i;
					const plInterfaceInfoModifier* pMod = 0;
					for( i = 0; i < pObj->GetNumModifiers(); i++ )
					{
						if (fBookMode == kNotOffering) // when sharing a book we don't care about other clickables
						{
							pMod = plInterfaceInfoModifier::ConvertNoRef( pObj->GetModifier(i) );
							if (pMod) // we found our list, stop here
							{
								plLogicModBase* pLogicMod = (plLogicModBase*)pObj->GetModifierByType(plLogicModBase::Index());
								if (!pLogicMod)
									return true;

								if (fCurrentClickable != pObj->GetKey())
								{	// is it the current clickable already?
									ClearClickableMap();
									fCurrentCursor = SetCurrentCursorID(kNullCursor);
									fCurrentClickable = pObj->GetKey();
									fCurrentClickableLogicMod = pLogicMod->GetKey();
									fCurrentClickPoint = pLOSMsg->fHitPoint;
									for (int x = 0; x < pMod->GetNumReferencedKeys(); x++)
										fClickableMap.Append( TRACKED_NEW clickableTest(pMod->GetReferencedKey(x)));
								}
								else
								{
									// even if this is still the same clickable object, the cursor could be
									// ...at a different spot on the clickable, so save that
									fCurrentClickPoint = pLOSMsg->fHitPoint;
								}
								fCurrClickIsAvatar = false;
								return true;
							}
						}
						
						// see if it is an avatar 
						plArmatureMod* armMod = (plArmatureMod*)plArmatureMod::ConvertNoRef( pObj->GetModifier(i));
						if (armMod)
						{
							if (armMod->IsMidLink())
								return true;

							// okay, are we a CCR?
							hsBool amCCR = plNetClientMgr::GetInstance()->GetCCRLevel();
							
							// is this person a NPC or CCR?
							int mbrIdx=plNetClientMgr::GetInstance()->TransportMgr().FindMember(pObj->GetKey());
							plNetTransportMember* pMbr = plNetClientMgr::GetInstance()->TransportMgr().GetMember(mbrIdx);
							if (!pMbr) // whoops - it's a freakin' NPC !
								return true;
							
							if (pMbr->IsCCR())
							{
								if (amCCR)
								{
									// we can click on them 
									plMouseDevice::AddCCRToCursor();
								}
								else
								{
									// nope
									return true;
								}
								
							}
							// now, if I am a CCR, let me click on anyone at any time
							if (amCCR)
							{
								ClearClickableMap();
								fCurrentClickable = pObj->GetKey();
								fCurrentClickableLogicMod = nil;
								fCurrClickIsAvatar = true;
								fCurrentCursor = SetCurrentCursorID(kCursorPoised);
								// not sure why we need to point on the avatar...
								// ...but maybe something in the future will need this
								fCurrentClickPoint = pLOSMsg->fHitPoint;
								plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(fCurrentClickable));
								// also add their player ID to the cursor
								plMouseDevice::AddIDNumToCursor(pMbr->GetPlayerID());
								return true;
							}
							// otherwise, cull people as necessary
							// also make sure that they are not in our ignore list
							else if (VaultAmIgnoringPlayer( pMbr->GetPlayerID()))
								return true;
							// further, if we are offering a book, only allow clicks on the person
							// whom we've already offered it to (to cancel it)
							else if (fBookMode == kBookOffered && pObj->GetKey() != fOffereeKey)
								return true;
							// within distance
							// also... make sure they aren't off climbing a ladder or looking at their KI
							else if (fBookMode == kOfferBook)
							{
								plArmatureBrain* curBrain = armMod->GetCurrentBrain();
								if (curBrain)
								{
									if (curBrain->IsRunningTask())
									{
										fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
										plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
										return true;
									}
								}
								plAvatarMgr* aMgr = plAvatarMgr::GetInstance();
								if (aMgr)
								{
									if (aMgr->IsACoopRunning())
									{
										fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
										plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
										return true;
									}
								}
								plSceneObject* locPlayer = (plSceneObject*)plNetClientMgr::GetInstance()->GetLocalPlayer();
								// make sure that they are facing each other
								if ( locPlayer )
								{
									hsVector3 ourView = locPlayer->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
									hsVector3 theirView = pObj->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
									hsScalar viewdot = ourView * theirView;
									hsVector3 towards(locPlayer->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate());
									towards.Normalize();
									hsScalar towardsdot = ourView * towards;
									if (viewdot > SHARE_FACING_TOLERANCE || towardsdot > SHARE_FACING_TOLERANCE )
									{
										ResetClickableState();
										return true;	// not facing enough... reject
									}
								}
								//otherwise make sure that they are close enough to click on
								if (locPlayer)
								{
									hsPoint3 avPt = locPlayer->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
									hsPoint3 objPt = pObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
									hsVector3 dist(avPt - objPt);
									if ( dist.MagnitudeSquared() >= 16.0f ) // you are too far away
									{
										ResetClickableState();
										return true;
									}
									
									if (hsABS(avPt.fZ - objPt.fZ) > 1.0f) // you need to also be in the same plane (some books are on top of rocks you need to jump onto)
									{
										ResetClickableState();
										return true;
									}
								}
							}
							// finally - make sure this guy is not in our ignore lists.
							int x;
							for (x = 0; x < fIgnoredAvatars.Count(); x++)
							{
								if (fIgnoredAvatars[x] == pObj->GetKey())
								{	
									fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
									plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
									return true;
								}
							}
							for (x = 0; x < fGUIIgnoredAvatars.Count(); x++)
							{
								if (fGUIIgnoredAvatars[x] == pObj->GetKey())
								{	
									fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
									plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
									return true;
								}
							}
							
							ClearClickableMap();
							fCurrentClickable = pObj->GetKey();
							fCurrentClickableLogicMod = nil;
							fCurrClickIsAvatar = true;
							fCurrentCursor = SetCurrentCursorID(kCursorPoised);
							// not sure why we need to point on the avatar...
							// ...but maybe something in the future will need this
							fCurrentClickPoint = pLOSMsg->fHitPoint;
							plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(fCurrentClickable));
							return true;
						}
					}
					// here! it's an object which is not clickable
					// no object, or not clickable or avatar
					
					fCurrentClickPoint = pLOSMsg->fHitPoint;
					ResetClickableState();
					return false;
				}
			}
			// no object, or not clickable or avatar
			ResetClickableState();
		}
		else
		if( pLOSMsg->fRequestID == ID_FIND_LOCALPLAYER )
		{
			bool result = false;
			if( pLOSMsg->fObj )
			{
				// is this object clickable?
				plSceneObject *pObj = plSceneObject::ConvertNoRef( pLOSMsg->fObj->ObjectIsLoaded() );
				if( pObj )
				{	
					if (pObj == plNetClientMgr::GetInstance()->GetLocalPlayer())
						result = true;
				}	
			}
			if (result && !fFadedLocalAvatar)
			{	
				IHalfFadeAvatar(true);
				return true;
			}
			else
			if (!result && fFadedLocalAvatar)
			{
				IHalfFadeAvatar(false);
				return true;
			}
		}
		if( pLOSMsg->fRequestID == ID_FIND_WALKABLE_GROUND )
		{
			if (!pLOSMsg->fNoHit)
			{
				plAvatarMgr::GetInstance()->GetLocalAvatar()->TurnToPoint(pLOSMsg->fHitPoint);
			}
			return true;
		}
			
		return true;
	}

	plCursorChangeMsg	*fakeReplyMsg = plCursorChangeMsg::ConvertNoRef( msg );
	if( fakeReplyMsg != nil )
	{
		hsBool deniedCurrent = false;
		plKey key = fakeReplyMsg->GetSender();
		for (int i = 0; i < fClickableMap.Count(); i++)
		{
			if (fClickableMap[i]->key == key)
			{
				if( fakeReplyMsg->fType == plCursorChangeMsg::kNullCursor )
				{
					// Means not clickable--gotta fix this someday
					fClickableMap[i]->val = false;
					if (fClickableMap[i]->key == fCurrentClickableLogicMod)
					{
						deniedCurrent = true;
						break;
					}
				}
				else
				{
					// And fix this...
					fClickableMap[i]->val = true;
				}
			}
		}
		if (deniedCurrent)
			ResetClickableState();
		return true;
	}

	plRenderMsg *rMsg = plRenderMsg::ConvertNoRef( msg );
	if( rMsg != nil )
	{
		fPipe = rMsg->Pipeline();
		plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
		return true;
	}
	// reply from coop share book multistage
	plNotifyMsg* pNMsg = plNotifyMsg::ConvertNoRef(msg);
	if (pNMsg)
	{
		for(int x=0; x < pNMsg->GetEventCount();x++)
		{
			proEventData* pED = pNMsg->GetEventRecord(0);
			if ( pED->fEventType == proEventData::kMultiStage )
			{
				proMultiStageEventData* pMS = (proMultiStageEventData*)pED;
				if (pMS->fAvatar == fOffereeKey) // mojo has linked
				{
					// do something - they linked out but we are still in the multistage
					fOffereeKey = nil;
				}
				else
				if (pMS->fAvatar == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
				{
					// do something else
					if (fBookMode = kNotOffering && fPendingLink == false) // we just linked out
					{
						// make me clickable again
						ISendAvatarDisabledNotification(true);
					}
					else // we put the book back after our target linked out
					{
						fBookMode = kNotOffering;
						fPendingLink = false;
						// make ME clickable again
						ISendAvatarDisabledNotification(true);
					}
				}	
				return true;
			}
		}
		return false;
				
	}
	// if someone pages out / in, remove them from our ignore list or notify them to ignore us
	plPlayerPageMsg* pPlayerMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pPlayerMsg)
	{
		if (pPlayerMsg->fUnload)
		{
			int x;
			// first, remove this avatar from my list of avatars I ingore for clickable griefing (when the 'ignore avatars' key is pressed)
			for(x = 0; x < fLocalIgnoredAvatars.Count(); x++)
			{
				if (fLocalIgnoredAvatars[x] == pPlayerMsg->fPlayer)
					fLocalIgnoredAvatars.RemoveItem(pPlayerMsg->fPlayer);
			}
			// now deal with avatars we are always ignoring because of their current activity
			for(x = 0; x < fIgnoredAvatars.Count(); x++)
			{
				if (fIgnoredAvatars[x] == pPlayerMsg->fPlayer)
					fIgnoredAvatars.RemoveItem(pPlayerMsg->fPlayer);
			}
			for(x = 0; x < fGUIIgnoredAvatars.Count(); x++)
			{
				if (fGUIIgnoredAvatars[x] == pPlayerMsg->fPlayer)
					fGUIIgnoredAvatars.RemoveItem(pPlayerMsg->fPlayer);
			}
			if (fOffereeKey == pPlayerMsg->fPlayer)
			{
				if (fBookMode == kBookOffered)
				{
					// and put our own dialog back up...
					ISendOfferNotification(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), 0, false);
					//IManageIgnoredAvatars(fOffereeKey, false);
					fOffereeKey = nil;
					fBookMode = kNotOffering;
					ISendAvatarDisabledNotification(true);
				}
			}
		}
		else
		{	
			// add them to the list we keep of everyone here:
			// but DO NOT add the local avatar
			if (pPlayerMsg->fPlayer != plNetClientMgr::GetInstance()->GetLocalPlayerKey())
				fLocalIgnoredAvatars.Append(pPlayerMsg->fPlayer);
			if (fBookMode != kNotOffering)
			{
				// tell them to ignore us
				
				plInputIfaceMgrMsg* pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
				pMsg->SetAvKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
				pMsg->SetBCastFlag(plMessage::kNetPropagate);
				pMsg->SetBCastFlag(plMessage::kNetForce);
				pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
				pMsg->AddNetReceiver( pPlayerMsg->fClientID );
				pMsg->Send();
				
				// and tell them to ignore our victim

				//plInputIfaceMgrMsg* pMsg2 = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
				//pMsg2->SetAvKey(fOffereeKey);
				//pMsg2->SetBCastFlag(plMessage::kNetPropagate);
				//pMsg2->SetBCastFlag(plMessage::kNetForce);
				//pMsg2->SetBCastFlag(plMessage::kLocalPropagate, false);
				//pMsg2->AddNetReceiver( pPlayerMsg->fClientID );
				//pMsg2->Send();
							
			}
			// tell them to ingore us if we are looking at a GUI
			for(int x = 0; x < fGUIIgnoredAvatars.Count(); x++)
			{
				if (fGUIIgnoredAvatars[x] == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
				{
					plInputIfaceMgrMsg* pMsg3 = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
					pMsg3->SetAvKey(fGUIIgnoredAvatars[x]);
					pMsg3->SetBCastFlag(plMessage::kNetPropagate);
					pMsg3->SetBCastFlag(plMessage::kNetForce);
					pMsg3->SetBCastFlag(plMessage::kLocalPropagate, false);
					pMsg3->AddNetReceiver( pPlayerMsg->fClientID );
					pMsg3->Send();
					return true;
				}
			}
		}
	}
	
	plInputIfaceMgrMsg *mgrMsg = plInputIfaceMgrMsg::ConvertNoRef( msg );
	if( mgrMsg != nil )
	{
		if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kDisableAvatarClickable )
		{
			// ignore if already in list or this is who WE are offering the book to...
			if (mgrMsg->GetAvKey() == fOffereeKey)
				return true;
			for(int x = 0; x < fIgnoredAvatars.Count(); x++)
			{
				if (fIgnoredAvatars[x] == mgrMsg->GetAvKey())
					return true;
			}
			fIgnoredAvatars.Append(mgrMsg->GetAvKey());
		}
		else
		if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kEnableAvatarClickable )
		{
			for(int x = 0; x < fIgnoredAvatars.Count(); x++)
			{
				if (fIgnoredAvatars[x] == mgrMsg->GetAvKey())
					fIgnoredAvatars.RemoveItem(mgrMsg->GetAvKey());
			}
		}
		else
		if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kGUIDisableAvatarClickable )
		{
			// ignore if already in list or this is who WE are offering the book to...
			if (mgrMsg->GetAvKey() == fOffereeKey)
				return true;
			for(int x = 0; x < fGUIIgnoredAvatars.Count(); x++)
			{
				if (fGUIIgnoredAvatars[x] == mgrMsg->GetAvKey())
					return true;
			}
			fGUIIgnoredAvatars.Append(mgrMsg->GetAvKey());
		}
		else
		if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kGUIEnableAvatarClickable )
		{
			for(int x = 0; x < fGUIIgnoredAvatars.Count(); x++)
			{
				if (fGUIIgnoredAvatars[x] == mgrMsg->GetAvKey())
					fGUIIgnoredAvatars.RemoveItem(mgrMsg->GetAvKey());
			}
		}
		else
		if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kEnableClickables )
		{
			fClickability = true;
			return true;
		}
		else if( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kDisableClickables )
		{
			fClickability = false;
			ResetClickableState();
			return true;
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kSetOfferBookMode )
		{
			fBookMode = kOfferBook;
			fOffereeKey = nil;
			fBookKey = mgrMsg->GetSender();
			fOfferedAgeInstance = mgrMsg->GetAgeName();
			fOfferedAgeFile = mgrMsg->GetAgeFileName();
			ISendAvatarDisabledNotification(false);

		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kClearOfferBookMode )
		{
			if (fBookMode == kOfferAccepted || fBookMode == kOfferLinkPending)
			{
				fPendingLink = true;
			}
			else
			if (fOffereeKey != nil)
			{
				// notify any offeree that the offer is rescinded
				ISendOfferNotification(fOffereeKey, -999, true);
				//IManageIgnoredAvatars(fOffereeKey, false);
				fOffereeKey = nil;
			}
			// shut down offer book mode
			fBookMode = kNotOffering;
			ISendAvatarDisabledNotification(true);
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kNotifyOfferRejected)
		{
			if (fBookMode == kBookOffered)
			{
				// and put our own dialog back up...
				ISendOfferNotification(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), 0, false);
				//IManageIgnoredAvatars(fOffereeKey, false);
				fBookMode = kOfferBook;
				fOffereeKey = nil;
			}
			else
			if (mgrMsg->GetSender() == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
			{
				fBookMode = kNotOffering;
				ISendAvatarDisabledNotification(true);
			}
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kNotifyOfferAccepted && fBookMode == kBookOffered)
		{
			fBookMode = kOfferAccepted;
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kNotifyOfferCompleted )
		{
			// must have actually offered the book...
			if (!fPendingLink)
			{
				if (fBookMode == kOfferBook || fBookMode == kBookOffered)
				return true;
			}
			if (!plNetClientMgr::GetInstance())
				return true;

			fOffereeID = mgrMsg->GetPageID();
			ILinkOffereeToAge();
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kSetShareSpawnPoint )
		{
			fSpawnPoint = mgrMsg->GetSpawnPoint();
		}
		else if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kSetShareAgeInstanceGuid )
		{
			fAgeInstanceGuid = mgrMsg->GetAgeInstanceGuid();
		}
	}
	plVaultNotifyMsg* pVaultMsg = plVaultNotifyMsg::ConvertNoRef(msg);
	if (pVaultMsg && pVaultMsg->GetType()==plNetCommon::VaultTasks::kRegisterOwnedAge )
	{
		//sanity check -
		if (fBookMode != kOfferLinkPending && fPendingLink == false)
			return true;
		// stop looking for this message and reset interface to 'offer book' mode again
		plgDispatch::Dispatch()->UnRegisterForExactType(plVaultNotifyMsg::Index(), fManager->GetKey());
		ILinkOffereeToAge();
		return true;
	}
	return false;
}


///// ILinkOffereeToAge

void plSceneInputInterface::ILinkOffereeToAge()
{
	// check vault to see if we've got an instance of the offered age now, if not create one and wait until we get a reply...
	plAgeInfoStruct info;
	info.SetAgeFilename(fOfferedAgeFile);
	info.SetAgeInstanceName(fOfferedAgeInstance);

	bool isAgeInstanceGuidSet = !GuidIsNil(fAgeInstanceGuid);
	
	plAgeLinkStruct link;
	
	if (isAgeInstanceGuidSet) {
		info.SetAgeInstanceGuid(&plUUID(fAgeInstanceGuid));
		link.GetAgeInfo()->CopyFrom(&info);

		GuidClear(&fAgeInstanceGuid);
	}
	else if (!VaultGetOwnedAgeLink(&info, &link)) {
	
		// We must have an owned copy of the age before we can offer it, so make one now
		info.SetAgeInstanceGuid(&plUUID(GuidGenerate()));
		std::string title;
		std::string desc;
		
		unsigned nameLen = StrLen(plNetClientMgr::GetInstance()->GetPlayerName());
		if (plNetClientMgr::GetInstance()->GetPlayerName()[nameLen - 1] == 's' || plNetClientMgr::GetInstance()->GetPlayerName()[nameLen - 1] == 'S') {
			xtl::format( title, "%s'", plNetClientMgr::GetInstance()->GetPlayerName() );
			xtl::format( desc, "%s' %s", plNetClientMgr::GetInstance()->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName() );
		}
		else {
			xtl::format( title, "%s's", plNetClientMgr::GetInstance()->GetPlayerName() );
			xtl::format( desc, "%s's %s", plNetClientMgr::GetInstance()->GetPlayerName(), link.GetAgeInfo()->GetAgeInstanceName() );
		}
		
		info.SetAgeUserDefinedName( title.c_str() );
		info.SetAgeDescription( desc.c_str() );

		link.GetAgeInfo()->CopyFrom(&info);
		if (!VaultRegisterOwnedAgeAndWait(&link)) {
			// failed to become an owner of the age for some reason, offer cannot continue
			return;
		}
	}
	else if (RelVaultNode * linkNode = VaultGetOwnedAgeLinkIncRef(&info)) {
		// We have the age in our AgesIOwnFolder. If its volatile, dump it for the new one.
		VaultAgeLinkNode linkAcc(linkNode);
		if (linkAcc.volat) {
			if (VaultUnregisterOwnedAgeAndWait(link.GetAgeInfo())) {
				link.GetAgeInfo()->SetAgeInstanceGuid(&plUUID(GuidGenerate()));
				VaultRegisterOwnedAgeAndWait(&link);
			}
		}
		linkNode->DecRef();
	}

	if (fSpawnPoint) {
		plSpawnPointInfo spawnPoint;
		spawnPoint.SetName(fSpawnPoint);
		link.SetSpawnPoint(spawnPoint);
	}
	
			
	// We now own the age, offer it

	if (0 == stricmp(fOfferedAgeFile, kPersonalAgeFilename))
		plNetLinkingMgr::GetInstance()->OfferLinkToPlayer(&link, fOffereeID, fManager->GetKey());
	else
		plNetLinkingMgr::GetInstance()->LinkPlayerToAge(&link, fOffereeID);
		
	if (!fPendingLink && stricmp(fOfferedAgeFile, kPersonalAgeFilename))
	{	
		// tell our local dialog to pop up again...
		ISendOfferNotification(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), 0, false);
		// make them clickable again(in case they come back?)
		//IManageIgnoredAvatars(fOffereeKey, false);
		
		fBookMode = kNotOffering;
		fOffereeKey = nil;
		fPendingLink = false;
	}
	else // this is a yeesha book link, must wait for multistage callbacks
	{
		// commented out until after 0.9
		fBookMode = kOfferLinkPending;
		fPendingLink = true;
//			fBookMode = kNotOffering;
//			fOffereeKey = nil;
//			fPendingLink = false;
//			ISendAvatarDisabledNotification(true);
	}
}

//// ISetLastClicked /////////////////////////////////////////////////////////

#define MATT_WAS_HERE

void	plSceneInputInterface::ISetLastClicked( plKey obj, hsPoint3 hitPoint )
{
	if (fBookMode != kNotOffering)
		return;
	
	if( fLastClicked != nil )
	{
		// Send an "un-picked" message to it
		if( !fLastClickIsAvatar )
		{
			plPickedMsg *pPickedMsg = TRACKED_NEW plPickedMsg;
			pPickedMsg->AddReceiver( fLastClicked );
			pPickedMsg->fPicked = false;
			plgDispatch::MsgSend( pPickedMsg );
		}
		else
		{
			plRemoteAvatarInfoMsg *pMsg = TRACKED_NEW plRemoteAvatarInfoMsg;
			pMsg->SetAvatarKey( nil );
			plgDispatch::MsgSend( pMsg );
		}
	}

	fLastClicked = obj;
	fLastClickIsAvatar = ( obj == nil ) ? false : fCurrClickIsAvatar;

	if( fLastClicked != nil )
	{
#ifdef MATT_WAS_HERE
	// now we send pick messages to avatars as well...
	plPickedMsg *pPickedMsg = TRACKED_NEW plPickedMsg;
	pPickedMsg->AddReceiver( fLastClicked );
	pPickedMsg->fHitPoint = hitPoint;
	plgDispatch::MsgSend( pPickedMsg );

	// if it's an avatar, we also send this thing
	if(fLastClickIsAvatar)
	{					
		plRemoteAvatarInfoMsg *pMsg = TRACKED_NEW plRemoteAvatarInfoMsg;
		pMsg->SetAvatarKey( fLastClicked );
		plgDispatch::MsgSend( pMsg );
	}
#else
	// Send a "picked" message to it
	if( !fLastClickIsAvatar )
	{
		plPickedMsg *pPickedMsg = TRACKED_NEW plPickedMsg;
		pPickedMsg->AddReceiver( fLastClicked );
		pPickedMsg->fHitPoint = hitPoint;
		plgDispatch::MsgSend( pPickedMsg );
	}
	else
	{					
		plRemoteAvatarInfoMsg *pMsg = TRACKED_NEW plRemoteAvatarInfoMsg;
		pMsg->SetAvatarKey( fLastClicked );
		plgDispatch::MsgSend( pMsg );
	}
#endif
	}
}

//// InterpretInputEvent /////////////////////////////////////////////////////

hsBool plSceneInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
	plControlEventMsg* pControlEvent = plControlEventMsg::ConvertNoRef(pMsg);
	if (pControlEvent)
	{
		if (pControlEvent->GetControlCode() == B_CONTROL_IGNORE_AVATARS)
		{
			for (int i = 0; i < fLocalIgnoredAvatars.Count(); i++)
			{
				plSceneObject* pObj = plSceneObject::ConvertNoRef(fLocalIgnoredAvatars[i]->ObjectIsLoaded());
				if (!pObj)
					continue;

				const plArmatureMod* pArm = (const plArmatureMod*)pObj->GetModifierByType(plArmatureMod::Index());
				if (!pArm)
					continue;

				plPhysicalControllerCore* controller = pArm->GetController();
				if (controller)
				{
 					if (pControlEvent->ControlActivated())
 						controller->SetLOSDB(plSimDefs::kLOSDBNone);
 					else
 						controller->SetLOSDB(plSimDefs::kLOSDBUIItems);
				}
			}
			return true;
		}
		return false;
	}

	plMouseEventMsg *mouseMsg = plMouseEventMsg::ConvertNoRef( pMsg );
	if( mouseMsg != nil )
	{
		// you're suspended when in this mode...
		if (fBookMode == kOfferLinkPending || fBookMode == kOfferAccepted)
			return true;

		if( mouseMsg->GetButton() == kLeftButtonDown )
		{
			if( fCurrentClickable != nil && fLastClicked == nil && fCurrentCursor != kNullCursor )
			{
				fButtonState |= kLeftButtonDown;
				ISetLastClicked( fCurrentClickable, fCurrentClickPoint );
				fCurrentCursor = SetCurrentCursorID(kCursorClicked);
				return true;
			}
			// right here
			if (fBookMode == kOfferBook)
			{
				fBookMode = kNotOffering;
				fOffereeKey = nil;
				ISendAvatarDisabledNotification(true);
			}
		}
		else if( mouseMsg->GetButton() == kLeftButtonUp )
		{
			if (fBookMode != kNotOffering)
			{
				if (fBookMode == kOfferBook && fCurrClickIsAvatar)
				{
					// send the avatar a message to put up his appropriate book
					ISendOfferNotification(fCurrentClickable, 999, true);
					//IManageIgnoredAvatars(fCurrentClickable, true);
					fBookMode = kBookOffered;
					fOffereeKey = fCurrentClickable;
				}
				else
				if (fBookMode == kBookOffered && fCurrClickIsAvatar)
				{
					// and put our own dialog back up...
					ISendOfferNotification(fOffereeKey, -999, true);
					ISendOfferNotification(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), 0, false);
					//IManageIgnoredAvatars(fOffereeKey, false);
					fBookMode = kOfferBook;
					fOffereeKey = nil;
				} 	
				else
				if (fBookMode == kOfferBook)
				{
					fBookMode = kNotOffering;
					fOffereeKey = nil;
					ISendAvatarDisabledNotification(true);
				}
			}
			if( fLastClicked != nil )
			{
				fButtonState &= ~kLeftButtonDown;
				ISetLastClicked( nil, hsPoint3(0,0,0) );
				
				return true;
			}
		}
	}

	return false;
}


//// ISendOfferNotification ////////////////////////////////////////////////////////
void plSceneInputInterface::IManageIgnoredAvatars(plKey& offeree, hsBool add)
{
	// tell everyone else to be able to / not to be able to select this avatar
	plInputIfaceMgrMsg* pMsg = 0;
	if (!add)
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableAvatarClickable);
	else
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
	pMsg->SetAvKey(offeree);
	pMsg->SetBCastFlag(plMessage::kNetPropagate);
	pMsg->SetBCastFlag(plMessage::kNetForce);
	pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
	pMsg->Send();
}	

void plSceneInputInterface::ISendOfferNotification(plKey& offeree, int ID, hsBool net)
{
	int offereeID = -1;
	if (offeree == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
	{
		offereeID = plNetClientMgr::GetInstance()->GetPlayerID();
	}
	else
	{
		plNetTransportMember **members = nil;
		plNetClientMgr::GetInstance()->TransportMgr().GetMemberListDistSorted( members );
		if( members != nil)
		{
			for(int i = 0; i < plNetClientMgr::GetInstance()->TransportMgr().GetNumMembers(); i++ )
			{
				plNetTransportMember *mbr = members[ i ];

				if( mbr != nil && mbr->GetAvatarKey() == offeree)
				{	
					offereeID = mbr->GetPlayerID();
					break;
				}
			}
		}

		delete [] members;

	}
	plNotifyMsg* pMsg = TRACKED_NEW plNotifyMsg;
	pMsg->AddOfferBookEvent(plNetClientMgr::GetInstance()->GetLocalPlayerKey(), ID, offereeID);
	pMsg->AddReceiver(fBookKey);
	if (net)
	{	
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		pMsg->SetBCastFlag(plMessage::kNetForce);
		pMsg->SetBCastFlag(plMessage::kLocalPropagate,false);
		pMsg->AddNetReceiver( offereeID );
		pMsg->Send();
	}
	else
	{
		pMsg->SetBCastFlag(plMessage::kNetPropagate, false); // don't deliver networked!
		pMsg->Send();
	}

}

void plSceneInputInterface::ISendAvatarDisabledNotification(hsBool enabled)
{
	plInputIfaceMgrMsg* pMsg = 0;
	if (enabled)
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableAvatarClickable);
	else
		pMsg = TRACKED_NEW plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
	pMsg->SetAvKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
	pMsg->SetBCastFlag(plMessage::kNetPropagate);
	pMsg->SetBCastFlag(plMessage::kNetForce);
	pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
	pMsg->Send();
}


//// IRequestLOSCheck ////////////////////////////////////////////////////////

void	plSceneInputInterface::IRequestLOSCheck( hsScalar xPos, hsScalar yPos, int ID )
{
	if( fPipe == nil )
		return;
	

	Int32 x=(Int32) ( xPos * fPipe->Width() );
	Int32 y=(Int32) ( yPos * fPipe->Height() );

	hsPoint3 endPos, startPos;
	
	fPipe->ScreenToWorldPoint( 1,0, &x, &y, 10000, 0, &endPos );
	startPos = fPipe->GetViewPositionWorld();

	// move the start pos out a little to avoid backing up against physical objects...
	hsVector3 view(endPos - startPos);
	view.Normalize();
	startPos = startPos + (view * 0.3f);

	plLOSRequestMsg* pMsg;
	
	if(ID == ID_FIND_CLICKABLE) {
		pMsg = TRACKED_NEW plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBUIItems, plLOSRequestMsg::kTestClosest );
		pMsg->SetCullDB(plSimDefs::kLOSDBUIBlockers);
	} else if(ID == ID_FIND_WALKABLE_GROUND) {
		pMsg = TRACKED_NEW plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBAvatarWalkable, plLOSRequestMsg::kTestClosest);
	} else
		pMsg = TRACKED_NEW plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBLocalAvatar, plLOSRequestMsg::kTestClosest);
	
	pMsg->SetReportType( plLOSRequestMsg::kReportHitOrMiss );

	pMsg->SetRequestID( ID );

	plgDispatch::MsgSend( pMsg );

	fLastStartPt = startPos;
	fLastEndPt = endPos;
}
		
//// IWorldPosMovedSinceLastLOSCheck /////////////////////////////////////////

hsBool	plSceneInputInterface::IWorldPosMovedSinceLastLOSCheck( void )
{
	if( fPipe == nil )
		return false;

	Int32 x=(Int32) ( plMouseDevice::Instance()->GetCursorX() * fPipe->Width() );
	Int32 y=(Int32) ( plMouseDevice::Instance()->GetCursorY() * fPipe->Height() );

	hsPoint3 endPos, startPos;
	
	startPos = fPipe->GetViewPositionWorld();
	if( !( startPos == fLastStartPt ) )
		return true;

	fPipe->ScreenToWorldPoint( 1,0, &x, &y, 10000, 0, &endPos );
	if( !( endPos == fLastEndPt ) )
		return true;

	return false;
}

//// GetCurrentCursorID ///////////////////////////////////////////////////////

UInt32 plSceneInputInterface::SetCurrentCursorID(UInt32 id)
{ 
	if (fBookMode == kOfferBook || fBookMode == kBookOffered)
	{
		switch(id)
		{
		case kCursorPoised:
			return kCursorOfferBookHilite;
		case kNullCursor:
			return kCursorOfferBook;
		case kCursorClicked:
			return kCursorOfferBookClicked;
		}
	}
	else
	if (fBookMode == kOfferAccepted || fBookMode == kOfferLinkPending)
		return kCursorOfferBook;
	
	return id; 
}

void plSceneInputInterface::RequestAvatarTurnToPointLOS()
{
	IRequestLOSCheck( plMouseDevice::Instance()->GetCursorX(), plMouseDevice::Instance()->GetCursorY(), ID_FIND_WALKABLE_GROUND );	
}