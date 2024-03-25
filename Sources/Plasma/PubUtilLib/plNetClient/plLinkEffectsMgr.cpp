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
#include "plLinkEffectsMgr.h"

#include "plNetLinkingMgr.h"
#include "plNetClientMgr.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarTasks.h"
#include "plAnimation/plAGAnim.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plLinkToAgeMsg.h"
#include "plMessage/plLoadAgeMsg.h"
#include "plMessage/plTransitionMsg.h"
#include "plNetTransport/plNetTransportMember.h"

plLinkEffectsMgr::plLinkEffectsMgr()
{
}

plLinkEffectsMgr::~plLinkEffectsMgr()
{
    for (plLinkEffectsTriggerMsg* msg : fLinks)
    {
        hsRefCnt_SafeUnRef(msg);
    }
    for (plLinkEffectsTriggerMsg* msg : fWaitlist)
    {
        hsRefCnt_SafeUnRef(msg);
    }
    for (plLinkEffectsTriggerMsg* msg : fDeadlist)
    {
        hsRefCnt_SafeUnRef(msg);
    }
}

void plLinkEffectsMgr::Init()
{
    plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plPseudoLinkEffectMsg::Index(), GetKey());
}   

plLinkEffectsTriggerMsg *plLinkEffectsMgr::IFindLinkTriggerMsg(const plKey& linkKey)
{
    for (plLinkEffectsTriggerMsg* msg : fLinks)
    {
        if (msg->GetLinkKey() == linkKey)
            return msg;
    }
    return nullptr;
}

void plLinkEffectsMgr::IAddLink(plLinkEffectsTriggerMsg *msg)
{
    hsRefCnt_SafeRef(msg);
    fLinks.emplace_back(msg);
}

void plLinkEffectsMgr::IAddWait(plLinkEffectsTriggerMsg *msg)
{
    hsRefCnt_SafeRef(msg);
    fWaitlist.emplace_back(msg);
}

void plLinkEffectsMgr::IAddDead(plLinkEffectsTriggerMsg *msg)
{
    hsRefCnt_SafeRef(msg);
    fDeadlist.emplace_back(msg);
}

void plLinkEffectsMgr::IAddPseudo(plPseudoLinkEffectMsg *msg)
{
    hsRefCnt_SafeRef(msg);
    fPseudolist.emplace_back(msg);
}

bool plLinkEffectsMgr::IHuntWaitlist(plLinkEffectsTriggerMsg *msg)
{
    bool found = false;
    for (hsSsize_t i = fWaitlist.size() - 1; i >= 0; i--)
    {
        if (fWaitlist[i] == msg)
        {
            found = true;
            hsRefCnt_SafeUnRef(fWaitlist[i]);
            fWaitlist.erase(fWaitlist.begin() + i);
            plNetApp::GetInstance()->DebugMsg("Received backup LinkEffectsTriggerMsg. Never got remote trigger!\n");            
        }
    }
    
    return found || IHuntWaitlist(msg->GetLinkKey());
}

bool plLinkEffectsMgr::IHuntWaitlist(const plKey& linkKey)
{
    bool found = false;
    for (hsSsize_t i = fWaitlist.size() - 1; i >= 0; i--)
    {
        if (fWaitlist[i]->GetLinkKey() == linkKey)
        {
            found = true;
            IAddDead(fWaitlist[i]);

            hsRefCnt_SafeUnRef(fWaitlist[i]);
            fWaitlist.erase(fWaitlist.begin() + i);
            plNetApp::GetInstance()->DebugMsg("Received remote LinkEffectsTriggerMsg. Awaiting backup.\n");
        }
    }

    return found;
}

bool plLinkEffectsMgr::IHuntDeadlist(plLinkEffectsTriggerMsg *msg)
{
    bool found = false;
    for (hsSsize_t i = fDeadlist.size() - 1; i >= 0; i--)
    {
        if (fDeadlist[i] == msg)
        {
            found = true;
            hsRefCnt_SafeUnRef(fDeadlist[i]);
            fDeadlist.erase(fDeadlist.begin() + i);
            plNetApp::GetInstance()->DebugMsg("Received backup LinkEffectsTriggerMsg. Cleanly ignoring since we received remote trigger.\n");
        }
    }

    return found;
}



void plLinkEffectsMgr::ISendAllReadyCallbacks()
{
    for (hsSsize_t i = fLinks.size() - 1; i >= 0; i--)
    {
        if (fLinks[i]->fEffects <= 0)
        {
            if (fLinks[i]->IsLeavingAge())
            {
                if (fLinks[i]->GetLinkKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
                {
                    plLinkOutUnloadMsg* lam = new plLinkOutUnloadMsg;   // derived from LoadAgeMsg
                    lam->SetAgeFilename( NetCommGetAge()->ageDatasetName );
                    lam->AddReceiver(plNetClientMgr::GetInstance()->GetKey());
                    lam->SetPlayerID(plNetClientMgr::GetInstance()->GetPlayerID());
                    lam->Send();
                }
            }
            else
            {
                plLinkInDoneMsg* lid = new plLinkInDoneMsg;
                lid->AddReceiver(fLinks[i]->GetLinkKey());
                lid->SetBCastFlag(plMessage::kPropagateToModifiers);
                lid->Send();                    

                if (fLinks[i]->GetLinkKey() == plNetClientApp::GetInstance()->GetLocalPlayerKey())
                {
                    plLinkInDoneMsg* lid = new plLinkInDoneMsg;
                    lid->AddReceiver(plNetClientMgr::GetInstance()->GetKey());
                    lid->Send();
                }
            }

            hsRefCnt_SafeUnRef(fLinks[i]);
            fLinks.erase(fLinks.begin() + i);

            hsStatusMessage("Done - removing link FX msg\n");
        }
    }
}

bool plLinkEffectsMgr::MsgReceive(plMessage *msg)
{
    plNetClientMgr* nc = plNetClientMgr::GetInstance();
    plNetLinkingMgr* lm = plNetLinkingMgr::GetInstance();

    plPseudoLinkEffectMsg* pseudoMsg = plPseudoLinkEffectMsg::ConvertNoRef(msg);
    if (pseudoMsg)
    {
        // verify valid avatar and "link" objects
        if (!pseudoMsg->fAvatarKey)
            return true;
        if (!pseudoMsg->fLinkObjKey)
            return true;
        if (!pseudoMsg->fLinkObjKey->ObjectIsLoaded())
            return true;
        if (!plNetClientMgr::GetInstance()->IsAPlayerKey(pseudoMsg->fAvatarKey))
            return true;
        // send the trigger message to the avatar...
        plPseudoLinkAnimTriggerMsg* pMsg = new plPseudoLinkAnimTriggerMsg(true, pseudoMsg->fAvatarKey);
        pMsg->SetSender(GetKey());
        pMsg->Send();
        IAddPseudo(pseudoMsg);
    }

    plPseudoLinkAnimCallbackMsg* pseudoCallback = plPseudoLinkAnimCallbackMsg::ConvertNoRef(msg);
    if (pseudoCallback)
    {
        // warp the avatar to his new position
        plPseudoLinkEffectMsg* pMsg = IFindPseudo(pseudoCallback->fAvatarKey);
        if (pMsg)
        {
            plSceneObject* pObj = plSceneObject::ConvertNoRef(pMsg->fLinkObjKey->ObjectIsLoaded());
            if (pObj && pObj->GetCoordinateInterface())
            {
                hsMatrix44 mat = pObj->GetCoordinateInterface()->GetLocalToWorld();
                // create message
                plWarpMsg* pMsg = new plWarpMsg(mat);
                pMsg->SetWarpFlags(plWarpMsg::kFlushTransform);
                pMsg->AddReceiver(pseudoCallback->fAvatarKey);
                plUoid U(kVirtualCamera1_KEY);
                plKey pCamKey = hsgResMgr::ResMgr()->FindKey(U);
                if (pCamKey)
                {
                    pMsg->AddReceiver(pCamKey);
                }
                plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
                // now make him re-appear
                plPseudoLinkAnimTriggerMsg* pTrigMsg = new plPseudoLinkAnimTriggerMsg(false, pseudoCallback->fAvatarKey);
                pTrigMsg->SetSender(GetKey());
                pTrigMsg->Send();
                IRemovePseudo(pseudoCallback->fAvatarKey);
                
            }
        }
    }
    plLinkEffectsTriggerPrepMsg *tpMsg = plLinkEffectsTriggerPrepMsg::ConvertNoRef(msg);
    if (tpMsg)
    {
        plNetApp::GetInstance()->DebugMsg("Received LinkEffectsTriggerPREPMsg\n");
        IAddWait(tpMsg->GetTrigger());
        plLinkEffectPrepBCMsg *bcpMsg = new plLinkEffectPrepBCMsg;
        bcpMsg->fLeavingAge = tpMsg->fLeavingAge;
        bcpMsg->fLinkKey = tpMsg->fLinkKey;
        bcpMsg->Send();

        return true;
    }

    plLinkEffectsTriggerMsg* pTriggerMsg = plLinkEffectsTriggerMsg::ConvertNoRef(msg);
    if (pTriggerMsg)
    {
        plNetApp::GetInstance()->DebugMsg("Received LinkEffectsTriggerMsg, local={}, linkingIn={}, stealth={}",
            !msg->HasBCastFlag(plMessage::kNetNonLocal),
            !pTriggerMsg->IsLeavingAge(), pTriggerMsg->GetInvisLevel());

        plKey linkKey = pTriggerMsg->GetLinkKey();
        if (linkKey == nullptr)
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
        if (avatar == nullptr)
        {
            plNetApp::GetInstance()->DebugMsg("Can't find avatar, mod={}\n", linkKey->GetName());
            return true;
        }

        // This is not the right place to catch this problem.
//      if (IFindLinkTriggerMsg(linkKey) != nullptr)
//      { 
//          hsAssert(false, "Trying to link an Avatar already in the process of linking.");
//          return true;
//      }

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
        
        plLinkEffectBCMsg *BCMsg = new plLinkEffectBCMsg();
        BCMsg->fLinkKey = linkKey;
        BCMsg->SetLinkFlag(plLinkEffectBCMsg::kLeavingAge, pTriggerMsg->IsLeavingAge());
        BCMsg->SetLinkFlag(plLinkEffectBCMsg::kSendCallback, true);
        BCMsg->SetLinkFlag(plLinkEffectBCMsg::kMute, pTriggerMsg->MuteLinkSfx());
        
        // Check if you have a Yeesha book, and mute sound if you don't.
        // 'CleftSolved' gets set when you click on the linking panel in the cleft,
        // so we use that instead of checking KILevel.
        // Also, check if you're going to/from the ACA, or through the fissure, and mute sound if you are.
        if (linkKey == nc->GetLocalPlayerKey())
        {
            if(lm) {
                ST::string ageName = lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename();
                ST::string prevAgeName = lm->GetPrevAgeLink()->GetAgeInfo()->GetAgeFilename();

                bool linkToStartup = ageName.compare_i(kStartUpAgeFilename) == 0;      // To Startup
                bool linkFromStartup = prevAgeName.compare_i(kStartUpAgeFilename) == 0;   // Leaving Startup

                bool linkToACA = ageName.compare_i(kAvCustomizationFilename) == 0;
                bool linkFromACA = prevAgeName.compare_i(kAvCustomizationFilename) == 0;

                bool linkToFissureDrop = lm && 
                                        lm->GetAgeLink()->HasSpawnPt() &&
                                        !lm->GetAgeLink()->SpawnPoint().GetName().compare_i(kCleftAgeLinkInPointFissureDrop);
                bool linkToDsntFromShell = lm && 
                                        lm->GetAgeLink()->HasSpawnPt() &&
                                        !lm->GetAgeLink()->SpawnPoint().GetTitle().compare_i(kDescentLinkFromShell);
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
            plATCAnim *linkInAnim = nullptr;
            plKey linkInAnimKey;
            const plArmatureMod *avMod = plArmatureMod::ConvertNoRef(avatar->GetModifierByType(plArmatureMod::Index()));
            if (pTriggerMsg->HasBCastFlag(plMessage::kNetNonLocal))
            {
                // Remote trigger, they should tell us how they linked in.
                linkInAnimKey = pTriggerMsg->GetLinkInAnimKey();
            }
            else
            {
                // this is our backup trigger we send ourselves. We've already received the remote player's SDL.
                linkInAnimKey = avMod ? avMod->GetLinkInAnimKey() : nullptr;
            }
            linkInAnim = plATCAnim::ConvertNoRef(linkInAnimKey ? linkInAnimKey->ObjectIsLoaded() : nullptr);
            
            if (avMod && linkInAnim)
            {   
                plAvOneShotTask *task = new plAvOneShotTask(linkInAnim->GetName(), false, false, nullptr);
                task->fBackwards = true;
                task->fDisableLooping = true;
                task->fDisablePhysics = false;
                (new plAvTaskMsg(GetKey(), avMod->GetKey(), task))->Send();
            }
                
        }
        
        IAddLink(pTriggerMsg); // refs the avatarMod
        
        // Dummy msg sent after the broadcast. This guarantees we have a callback to actually trigger the
        // link, plus we know any effect broadcast messages will have processed before this (and therefore
        // have told us to wait for them.)
        pTriggerMsg->fEffects++;
        plLinkCallbackMsg *dummyMsg = new plLinkCallbackMsg();
        dummyMsg->AddReceiver(GetKey());
        dummyMsg->fLinkKey = linkKey;
        plgDispatch::MsgSend(dummyMsg);

        return true;
    }

    // callbacks from linkout events
    plLinkCallbackMsg* pLinkCallbackMsg = plLinkCallbackMsg::ConvertNoRef(msg);
    if (pLinkCallbackMsg)
    {
        plNetApp::GetInstance()->DebugMsg("Received pLinkCallbackMsg, localmsg={}\n",
            !msg->HasBCastFlag(plMessage::kNetNonLocal));

        plLinkEffectsTriggerMsg *pTriggerMsg = IFindLinkTriggerMsg(pLinkCallbackMsg->fLinkKey);
        if (pTriggerMsg == nullptr)
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
            plNetApp::GetInstance()->DebugMsg("Too many link callbacks received for avatar {}. Ignoring extras.\n",
                    pTriggerMsg->GetLinkKey()->GetName());
        }
        else
        {
            plNetApp::GetInstance()->DebugMsg("{} link callbacks left until avatar {} links...\n",
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

        const float kMaxTimeForLinkTrigger = 30.f;

        // If we're not loading state, we're in the age. So this avatar coming in must be linking in.
        // If the player is us, no prep is necessary.
        if (!plNetClientApp::GetInstance()->IsLoadingInitialAgeState() && 
            (pageMsg->fPlayer != nc->GetLocalPlayerKey()))
        {
            plLinkEffectsTriggerMsg *trigMsg = new plLinkEffectsTriggerMsg;
            trigMsg->SetLeavingAge(false);
            trigMsg->SetLinkKey(pageMsg->fPlayer);

            // Send off the prep message right away
            plLinkEffectsTriggerPrepMsg *trigPrepMsg = new plLinkEffectsTriggerPrepMsg;
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

void plLinkEffectsMgr::WaitForEffect(plKey linkKey, float time)
{
    plLinkEffectsTriggerMsg *msg = IFindLinkTriggerMsg(linkKey);
    if (msg == nullptr)
    {
        hsAssert(true, "Request to wait on an effect for an avatar that isn't linking.");
        return;
    }

    msg->fEffects++;
    plLinkCallbackMsg *callback = new plLinkCallbackMsg();
    callback->fEvent = kStop;
    callback->fRepeats = 0;
    callback->fLinkKey = std::move(linkKey);
    double timeToDeliver = hsTimer::GetSysSeconds() + time;
    callback->SetTimeStamp( timeToDeliver );
    callback->Send( GetKey() );
}

plMessage *plLinkEffectsMgr::WaitForEffect(plKey linkKey)
{
    plLinkEffectsTriggerMsg *msg = IFindLinkTriggerMsg(linkKey);
    if (msg == nullptr)
    {
        hsAssert(true, "Request to wait on an effect for an avatar that isn't linking.");
        return nullptr;
    }

    msg->fEffects++;

    plLinkCallbackMsg *callback = new plLinkCallbackMsg();
    callback->fEvent = kStop;
    callback->fRepeats = 0;
    callback->fLinkKey = std::move(linkKey);
    callback->AddReceiver( GetKey() );
    return callback;
}

void plLinkEffectsMgr::WaitForPseudoEffect(plKey linkKey, float time)
{
    plPseudoLinkEffectMsg* msg = IFindPseudo(linkKey);
    if (msg == nullptr)
    {
        hsAssert(true, "Request to wait on an fake effect for an avatar that isn't fake linking.");
        return;
    }

    plPseudoLinkAnimCallbackMsg* callback = new plPseudoLinkAnimCallbackMsg();
    callback->fAvatarKey = std::move(linkKey);
    double timeToDeliver = hsTimer::GetSysSeconds() + time;
    callback->SetTimeStamp( timeToDeliver );
    callback->Send( GetKey() );
}

plPseudoLinkEffectMsg* plLinkEffectsMgr::IFindPseudo(const plKey& avatarKey)
{
    for (plPseudoLinkEffectMsg* msg : fPseudolist)
    {
        if (msg->fAvatarKey == avatarKey)
            return msg;
    }
    return nullptr;

}

void plLinkEffectsMgr::IRemovePseudo(const plKey& avatarKey)
{
    for (auto iter = fPseudolist.cbegin(); iter != fPseudolist.cend(); ++iter)
    {
        if ((*iter)->fAvatarKey == avatarKey)
        {
            fPseudolist.erase(iter);
            return;
        }
    }

}
