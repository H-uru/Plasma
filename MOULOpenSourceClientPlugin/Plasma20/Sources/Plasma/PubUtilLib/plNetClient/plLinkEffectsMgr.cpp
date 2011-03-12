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
#include "../pnKeyedObject/plKey.h"
#include "hsTemplates.h"
#include "hsStream.h"
#include "plLinkEffectsMgr.h"
#include "../pnMessage/plEventCallbackMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plPlayerPageMsg.h"
#include "../plMessage/plLinkToAgeMsg.h"
#include "../plMessage/plTransitionMsg.h"
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "../pnNetCommon/plNetApp.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plVault/plVault.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plAgeLoader/plAgeLoader.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnKeyedObject/plFixedKey.h"

// chronicle var
#define kCleftSolved					L"CleftSolved"

#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvatarTasks.h"
#include "../plAvatar/plAGAnim.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plLoadAgeMsg.h"

plLinkEffectsMgr::plLinkEffectsMgr()
{
}

plLinkEffectsMgr::~plLinkEffectsMgr()
{
	int	i;
	for( i = 0; i < fLinks.GetCount(); i++ )
	{
		hsRefCnt_SafeUnRef(fLinks[i]);
	}
	for( i = 0; i < fWaitlist.GetCount(); i++ )
	{
		hsRefCnt_SafeUnRef(fWaitlist[i]);
	}
	for( i = 0; i < fDeadlist.GetCount(); i++ )
	{
		hsRefCnt_SafeUnRef(fDeadlist[i]);
	}
}

void plLinkEffectsMgr::Init()
{
	plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
	plgDispatch::Dispatch()->RegisterForExactType(plPseudoLinkEffectMsg::Index(), GetKey());
}	

plLinkEffectsTriggerMsg *plLinkEffectsMgr::IFindLinkTriggerMsg(plKey linkKey)
{
	int i;
	for (i = 0; i < fLinks.GetCount(); i++)
	{
		if (fLinks[i]->GetLinkKey() == linkKey)
			return fLinks[i];
	}
	return nil;
}

void plLinkEffectsMgr::IAddLink(plLinkEffectsTriggerMsg *msg)
{
	hsRefCnt_SafeRef(msg);
	fLinks.Append(msg);
}

void plLinkEffectsMgr::IAddWait(plLinkEffectsTriggerMsg *msg)
{
	hsRefCnt_SafeRef(msg);
	fWaitlist.Append(msg);
}

void plLinkEffectsMgr::IAddDead(plLinkEffectsTriggerMsg *msg)
{
	hsRefCnt_SafeRef(msg);
	fDeadlist.Append(msg);
}

void plLinkEffectsMgr::IAddPsuedo(plPseudoLinkEffectMsg *msg)
{
	hsRefCnt_SafeRef(msg);
	fPseudolist.Append(msg);
}

hsBool plLinkEffectsMgr::IHuntWaitlist(plLinkEffectsTriggerMsg *msg)
{
	int i;
	hsBool found = false;
	for (i = fWaitlist.GetCount() - 1; i >= 0; i--)
	{
		if (fWaitlist[i] == msg)
		{
			found = true;
			hsRefCnt_SafeUnRef(fWaitlist[i]);			
			fWaitlist.Remove(i);
			plNetApp::GetInstance()->DebugMsg("Received backup LinkEffectsTriggerMsg. Never got remote trigger!\n");			
		}
	}
	
	return found || IHuntWaitlist(msg->GetLinkKey());
}

hsBool plLinkEffectsMgr::IHuntWaitlist(plKey linkKey)
{
	int i;
	hsBool found = false;
	for (i = fWaitlist.GetCount() - 1; i >= 0; i--)
	{
		if (fWaitlist[i]->GetLinkKey() == linkKey)
		{
			found = true;
			IAddDead(fWaitlist[i]);

			hsRefCnt_SafeUnRef(fWaitlist[i]);
			fWaitlist.Remove(i);
			plNetApp::GetInstance()->DebugMsg("Received remote LinkEffectsTriggerMsg. Awaiting backup.\n");
		}
	}

	return found;
}

hsBool plLinkEffectsMgr::IHuntDeadlist(plLinkEffectsTriggerMsg *msg)
{
	int i;
	hsBool found = false;
	for (i = fDeadlist.GetCount() - 1; i >= 0; i--)
	{
		if (fDeadlist[i] == msg)
		{
			found = true;
			hsRefCnt_SafeUnRef(fDeadlist[i]);
			fDeadlist.Remove(i);
			plNetApp::GetInstance()->DebugMsg("Received backup LinkEffectsTriggerMsg. Cleanly ignoring since we received remote trigger.\n");
		}
	}

	return found;
}



void plLinkEffectsMgr::ISendAllReadyCallbacks()
{
	int i;
	for (i = fLinks.GetCount() - 1; i >= 0; i--)
	{
		if (fLinks[i]->fEffects <= 0)
		{
			if (fLinks[i]->IsLeavingAge())
			{
				if (fLinks[i]->GetLinkKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
				{
					plLinkOutUnloadMsg* lam = TRACKED_NEW plLinkOutUnloadMsg;	// derived from LoadAgeMsg
					lam->SetAgeFilename( NetCommGetAge()->ageDatasetName );
					lam->AddReceiver(plNetClientMgr::GetInstance()->GetKey());
					lam->SetPlayerID(plNetClientMgr::GetInstance()->GetPlayerID());
					lam->Send();
				}
			}
			else
			{
				plLinkInDoneMsg* lid = TRACKED_NEW plLinkInDoneMsg;
				lid->AddReceiver(fLinks[i]->GetLinkKey());
				lid->SetBCastFlag(plMessage::kPropagateToModifiers);
				lid->Send();					

				if (fLinks[i]->GetLinkKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
				{
					plLinkInDoneMsg* lid = TRACKED_NEW plLinkInDoneMsg;
					lid->AddReceiver(plNetClientMgr::GetInstance()->GetKey());
					lid->Send();
				}
			}

			hsRefCnt_SafeUnRef(fLinks[i]);
			fLinks.Remove(i);

			hsStatusMessage("Done - removing link FX msg\n");
		}
	}
}

hsBool plLinkEffectsMgr::MsgReceive(plMessage *msg)
{
	plNetClientMgr* nc = plNetClientMgr::GetInstance();
	plNetLinkingMgr* lm = plNetLinkingMgr::GetInstance();

	plPseudoLinkEffectMsg* pSeudoMsg = plPseudoLinkEffectMsg::ConvertNoRef(msg);
	if (pSeudoMsg)
	{
		// verify valid avatar and "link" objects
		if (!pSeudoMsg->fAvatarKey) 
			return true;
		if (!pSeudoMsg->fLinkObjKey)
			return true;
		if (!pSeudoMsg->fLinkObjKey->ObjectIsLoaded())
			return true;
		if (!plNetClientMgr::GetInstance()->IsAPlayerKey(pSeudoMsg->fAvatarKey))
			return true;
		// send the trigger message to the avatar...
		plPseudoLinkAnimTriggerMsg* pMsg = TRACKED_NEW plPseudoLinkAnimTriggerMsg(true, pSeudoMsg->fAvatarKey);
		pMsg->SetSender(GetKey());
		pMsg->Send();
		IAddPsuedo(pSeudoMsg);
	}

	plPseudoLinkAnimCallbackMsg* pSeudoCallback = plPseudoLinkAnimCallbackMsg::ConvertNoRef(msg);
	if (pSeudoCallback)
	{
		// warp the avatar to his new position
		plPseudoLinkEffectMsg* pMsg = IFindPseudo(pSeudoCallback->fAvatarKey);
		if (pMsg)
		{
			plSceneObject* pObj = plSceneObject::ConvertNoRef(pMsg->fLinkObjKey->ObjectIsLoaded());
			if (pObj && pObj->GetCoordinateInterface())
			{
				hsMatrix44 mat = pObj->GetCoordinateInterface()->GetLocalToWorld();
				// create message
				plWarpMsg* pMsg = TRACKED_NEW plWarpMsg(mat);
				pMsg->SetWarpFlags(plWarpMsg::kFlushTransform);
				pMsg->AddReceiver(pSeudoCallback->fAvatarKey);
				plUoid U(kVirtualCamera1_KEY);
				plKey pCamKey = hsgResMgr::ResMgr()->FindKey(U);
				if (pCamKey)
				{
					pMsg->AddReceiver(pCamKey);
				}
				plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
				// now make him re-appear
				plPseudoLinkAnimTriggerMsg* pTrigMsg = TRACKED_NEW plPseudoLinkAnimTriggerMsg(false, pSeudoCallback->fAvatarKey);
				pTrigMsg->SetSender(GetKey());
				pTrigMsg->Send();
				IRemovePseudo(pSeudoCallback->fAvatarKey);
				
			}
		}
	}
	plLinkEffectsTriggerPrepMsg *tpMsg = plLinkEffectsTriggerPrepMsg::ConvertNoRef(msg);
	if (tpMsg)
	{
		plNetApp::GetInstance()->DebugMsg("Received LinkEffectsTriggerPREPMsg\n");
		IAddWait(tpMsg->GetTrigger());
		plLinkEffectPrepBCMsg *bcpMsg = TRACKED_NEW plLinkEffectPrepBCMsg;
		bcpMsg->fLeavingAge = tpMsg->fLeavingAge;
		bcpMsg->fLinkKey = tpMsg->fLinkKey;
		bcpMsg->Send();

		return true;
	}

	plLinkEffectsTriggerMsg* pTriggerMsg = plLinkEffectsTriggerMsg::ConvertNoRef(msg);
	if (pTriggerMsg)
	{
		plNetApp::GetInstance()->DebugMsg("Received LinkEffectsTriggerMsg, local=%d, linkingIn=%d, stealth=%d", 
			!msg->HasBCastFlag(plMessage::kNetNonLocal),
			!pTriggerMsg->IsLeavingAge(), pTriggerMsg->GetInvisLevel());

		plKey linkKey = pTriggerMsg->GetLinkKey();
		if (linkKey == nil)
			return true;

		if ((linkKey != nc->GetLocalPlayerKey()) &&
			(!pTriggerMsg->IsLeavingAge()))
		{
			if (IHuntDeadlist(pTriggerMsg)) // Just an obselete safety trigger
				return true;
			
			if (!IHuntWaitlist(pTriggerMsg))
			{
				plNetApp::GetInstance()->DebugMsg("Unexpected linkEffectsTriggerMsg. Ignoring\n");
				return true;
			}
		}
		
		plSceneObject *avatar = plSceneObject::ConvertNoRef(linkKey->ObjectIsLoaded());
		if (avatar == nil)
		{
			plNetApp::GetInstance()->DebugMsg("Can't find avatar, mod=%s\n", linkKey->GetName());
			return true;
		}

		// This is not the right place to catch this problem.
//		if (IFindLinkTriggerMsg(linkKey) != nil)
//		{ 
//			hsAssert(false, "Trying to link an Avatar already in the process of linking.");
//			return true;
//		}
		
		if (pTriggerMsg->GetInvisLevel() && linkKey != nc->GetLocalPlayerKey())
		{
#ifdef PLASMA_EXTERNAL_RELEASE
			// Verify that the server told us that the invisible avatar is a CCR
			plNetTransportMember* mbr=nc->TransportMgr().GetMember(nc->TransportMgr().FindMember(linkKey));
			if (!mbr || mbr->GetCCRLevel()<pTriggerMsg->GetInvisLevel())
			{
				plNetApp::StaticErrorMsg("Remote Avatar trying to be stealthy - REJECTING since he's not a CCR");
			}
			else
#endif
			{
				plNetApp::StaticDebugMsg("Remote Avatar is in stealth mode - making invisible");
				nc->MakeCCRInvisible(pTriggerMsg->GetLinkKey(), pTriggerMsg->GetInvisLevel());
			}
		}
		
		if (pTriggerMsg->IsLeavingAge())
			hsStatusMessage("Starting LinkOut FX\n");
		else
			hsStatusMessage("Starting LinkIn FX\n");
		
		plLinkEffectBCMsg *BCMsg = TRACKED_NEW plLinkEffectBCMsg();
		BCMsg->fLinkKey = linkKey;
		BCMsg->SetLinkFlag(plLinkEffectBCMsg::kLeavingAge, pTriggerMsg->IsLeavingAge());
		BCMsg->SetLinkFlag(plLinkEffectBCMsg::kSendCallback, true);
		
		// Check if you have a Yeesha book, and mute sound if you don't.
		// 'CleftSolved' gets set when you click on the linking panel in the cleft,
		// so we use that instead of checking KILevel.
		// Also, check if you're going to/from the ACA, or through the fissure, and mute sound if you are.
		if (linkKey == nc->GetLocalPlayerKey())
		{
			if(lm) {
				const char *ageName = lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename();
				const char *prevAgeName = lm->GetPrevAgeLink()->GetAgeInfo()->GetAgeFilename();

				bool linkToStartup = ageName && !stricmp(ageName, kStartUpAgeFilename	);		// To Startup
				bool linkFromStartup = prevAgeName && !stricmp(prevAgeName,	kStartUpAgeFilename);	// Leaving Startup

				bool cleftSolved = VaultHasChronicleEntry( kCleftSolved );

				bool linkToACA = ageName &&	!stricmp(ageName, kAvCustomizationFilename);
				bool linkFromACA = prevAgeName && !stricmp(prevAgeName,	kAvCustomizationFilename);

				bool linkToFissureDrop = lm && 
										lm->GetAgeLink()->HasSpawnPt() && 
										lm->GetAgeLink()->SpawnPoint().GetName() &&
										!stricmp(lm->GetAgeLink()->SpawnPoint().GetName(), kCleftAgeLinkInPointFissureDrop);
				bool linkToDsntFromShell = lm && 
										lm->GetAgeLink()->HasSpawnPt() && 
										lm->GetAgeLink()->SpawnPoint().GetTitle() &&
										!stricmp(lm->GetAgeLink()->SpawnPoint().GetTitle(), kDescentLinkFromShell);
				if ( linkToACA || linkFromACA || linkToStartup || linkFromStartup || linkToFissureDrop || linkToDsntFromShell)
				{
					BCMsg->SetLinkFlag(plLinkEffectBCMsg::kMute);
				}			
			}
		}
		
		BCMsg->SetSender(GetKey());
		if (msg->HasBCastFlag(plMessage::kNetNonLocal))
			// terminate the remote cascade and start a new (local) cascade, since the rcvr is localOnly and will reject remote msgs
			BCMsg->SetBCastFlag(plMessage::kNetStartCascade);	
		plgDispatch::MsgSend(BCMsg);
		
		if (!pTriggerMsg->IsLeavingAge()) // Avatar is currently entering a new age
		{
			plATCAnim *linkInAnim = nil;
			plKey linkInAnimKey = nil;
			const plArmatureMod *avMod = plArmatureMod::ConvertNoRef(avatar->GetModifierByType(plArmatureMod::Index()));
			if (pTriggerMsg->HasBCastFlag(plMessage::kNetNonLocal))
			{
				// Remote trigger, they should tell us how they linked in.
				linkInAnimKey = pTriggerMsg->GetLinkInAnimKey();
			}
			else
			{
				// this is our backup trigger we send ourselves. We've already received the remote player's SDL.
				linkInAnimKey = avMod ? avMod->GetLinkInAnimKey() : nil;
			}
			linkInAnim = plATCAnim::ConvertNoRef(linkInAnimKey ? linkInAnimKey->ObjectIsLoaded() : nil);
			
			if (avMod && linkInAnim)
			{	
				plAvOneShotTask *task = TRACKED_NEW plAvOneShotTask(linkInAnim->GetName(), false, false, nil);
				task->fBackwards = true;
				task->fDisableLooping = true;
				task->fDisablePhysics = false;
				(TRACKED_NEW plAvTaskMsg(GetKey(), avMod->GetKey(), task))->Send();
			}
				
		}
		
		IAddLink(pTriggerMsg); // refs the avatarMod
		
		// Dummy msg sent after the broadcast. This guarantees we have a callback to actually trigger the
		// link, plus we know any effect broadcast messages will have processed before this (and therefore
		// have told us to wait for them.)
		pTriggerMsg->fEffects++;
		plLinkCallbackMsg *dummyMsg = TRACKED_NEW plLinkCallbackMsg();
		dummyMsg->AddReceiver(GetKey());
		dummyMsg->fLinkKey = linkKey;
		plgDispatch::MsgSend(dummyMsg);

		return true;
	}

	// callbacks from linkout events
	plLinkCallbackMsg* pLinkCallbackMsg = plLinkCallbackMsg::ConvertNoRef(msg);
	if (pLinkCallbackMsg)
	{
		plNetApp::GetInstance()->DebugMsg("Received pLinkCallbackMsg, localmsg=%d\n", 
			!msg->HasBCastFlag(plMessage::kNetNonLocal));

		static char str[ 128 ];
		plLinkEffectsTriggerMsg *pTriggerMsg = IFindLinkTriggerMsg(pLinkCallbackMsg->fLinkKey);
		if (pTriggerMsg == nil)
		{
			hsAssert(true, "Received a callback for an avatar that isn't linking.");
			return true;
		}

		if (--pTriggerMsg->fEffects == 0)
		{
			plNetApp::GetInstance()->DebugMsg("All link callbacks received.\n" );
			plgDispatch::Dispatch()->RegisterForExactType(plTimeMsg::Index(), GetKey());
		}
		else if (pTriggerMsg->fEffects < 0 )
		{
			plNetApp::GetInstance()->DebugMsg("Too many link callbacks received for avatar %s. Ignoring extras.\n",
					pTriggerMsg->GetLinkKey()->GetName());			
		}
		else
		{
			plNetApp::GetInstance()->DebugMsg("%d link callbacks left until avatar %s links...\n", 
					 pTriggerMsg->fEffects, pTriggerMsg->GetLinkKey()->GetName());
		}
		return true;
	}

	plTimeMsg *time = plTimeMsg::ConvertNoRef(msg);
	if (time) // This is how we know we're out of the render function, and it's safe to pageIn/Out nodes
	{
		plgDispatch::Dispatch()->UnRegisterForExactType(plTimeMsg::Index(), GetKey());
		ISendAllReadyCallbacks();

		return true;
	}

	plPlayerPageMsg *pageMsg = plPlayerPageMsg::ConvertNoRef(msg);
	if (pageMsg)
	{
		if (pageMsg->fUnload)
		{
			IHuntWaitlist(pageMsg->fPlayer);
			return true;
		}

		const hsScalar kMaxTimeForLinkTrigger = 30.f;

		// If we're not loading state, we're in the age. So this avatar coming in must be linking in.
		// If the player is us, no prep is necessary.
		if (!plNetClientApp::GetInstance()->IsLoadingInitialAgeState() && 
			(pageMsg->fPlayer != nc->GetLocalPlayerKey()))
		{
			plLinkEffectsTriggerMsg *trigMsg = TRACKED_NEW plLinkEffectsTriggerMsg;
			trigMsg->SetLeavingAge(false);
			trigMsg->SetLinkKey(pageMsg->fPlayer);

			// Send off the prep message right away
			plLinkEffectsTriggerPrepMsg *trigPrepMsg = TRACKED_NEW plLinkEffectsTriggerPrepMsg;
			trigPrepMsg->fLinkKey = pageMsg->fPlayer;
			trigPrepMsg->SetTrigger(trigMsg);
			trigPrepMsg->Send(GetKey());

			// Send off a delayed safety trigger. If things are going along properly,
			// we'll get a trigger from the player linking in before this message is
			// received, and we'll ignore it.
			double timeToDeliver = hsTimer::GetSysSeconds() + kMaxTimeForLinkTrigger;
			trigMsg->SetTimeStamp(timeToDeliver);
			trigMsg->Send(GetKey());
		}
		
		return true;
	}

	return hsKeyedObject::MsgReceive(msg);
}

void plLinkEffectsMgr::WaitForEffect(plKey linkKey, hsScalar time)
{
	plLinkEffectsTriggerMsg *msg = IFindLinkTriggerMsg(linkKey);
	if (msg == nil)
	{
		hsAssert(true, "Request to wait on an effect for an avatar that isn't linking.");
		return;
	}

	msg->fEffects++;
	plLinkCallbackMsg *callback = TRACKED_NEW plLinkCallbackMsg();
	callback->fEvent = kStop;
	callback->fRepeats = 0;
	callback->fLinkKey = linkKey;
	double timeToDeliver = hsTimer::GetSysSeconds() + time;
	callback->SetTimeStamp( timeToDeliver );
	callback->Send( GetKey() );
}

plMessage *plLinkEffectsMgr::WaitForEffect(plKey linkKey)
{
	plLinkEffectsTriggerMsg *msg = IFindLinkTriggerMsg(linkKey);
	if (msg == nil)
	{
		hsAssert(true, "Request to wait on an effect for an avatar that isn't linking.");
		return nil;
	}

	msg->fEffects++;

	plLinkCallbackMsg *callback = TRACKED_NEW plLinkCallbackMsg();
	callback->fEvent = kStop;
	callback->fRepeats = 0;
	callback->fLinkKey = linkKey;
	callback->AddReceiver( GetKey() );
	return callback;
}

void plLinkEffectsMgr::WaitForPseudoEffect(plKey linkKey, hsScalar time)
{
	plPseudoLinkEffectMsg* msg = IFindPseudo(linkKey);
	if (msg == nil)
	{
		hsAssert(true, "Request to wait on an fake effect for an avatar that isn't fake linking.");
		return;
	}

	plPseudoLinkAnimCallbackMsg* callback = TRACKED_NEW plPseudoLinkAnimCallbackMsg();
	callback->fAvatarKey = linkKey;
	double timeToDeliver = hsTimer::GetSysSeconds() + time;
	callback->SetTimeStamp( timeToDeliver );
	callback->Send( GetKey() );
}

plPseudoLinkEffectMsg* plLinkEffectsMgr::IFindPseudo(plKey avatarKey)
{
	int i;
	for (i = 0; i < fPseudolist.GetCount(); i++)
	{
		if (fPseudolist[i]->fAvatarKey == avatarKey)
			return fPseudolist[i];
	}
	return nil;

}

void plLinkEffectsMgr::IRemovePseudo(plKey avatarKey)
{
	int i;
	for (i = 0; i < fPseudolist.GetCount(); i++)
	{
		if (fPseudolist[i]->fAvatarKey == avatarKey)
		{	
			fPseudolist.Remove(i);
			return;
		}
	}

}