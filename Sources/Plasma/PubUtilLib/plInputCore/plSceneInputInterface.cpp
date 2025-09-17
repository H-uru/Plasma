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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plSceneInputInterface                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plSceneInputInterface.h"

#include "plInputInterfaceMgr.h"
#include "plInputManager.h"
#include "plInputDevice.h"

#include "plMessage/plInputEventMsg.h"
#include "plMessage/plLOSRequestMsg.h"
#include "plMessage/plLOSHitMsg.h"
#include "plMessage/plPickedMsg.h"
#include "plMessage/plRenderMsg.h"
#include "plMessage/plInputIfaceMgrMsg.h"
#include "plMessage/plVaultNotifyMsg.h"
#include "pnMessage/plFakeOutMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plRemoteAvatarInfoMsg.h"
#include "pnMessage/plCursorChangeMsg.h"
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrain.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plPhysicalControllerCore.h"
#include "plModifier/plInterfaceInfoModifier.h"
#include "pnModifier/plLogicModBase.h"
#include "plVault/plVault.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClient/plNetLinkingMgr.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetTransport/plNetTransport.h"
#include "plNetTransport/plNetTransportMember.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnKeyedObject/plKey.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnInputCore/plKeyMap.h"
#include "plPhysical.h"

#include "plgDispatch.h"
#include "plPipeline.h"

#include "plModifier/plDetectorLog.h"


#define ID_FIND_CLICKABLE 2
#define ID_FIND_LOCALPLAYER 3
#define ID_FIND_WALKABLE_GROUND 4

#define SHARE_FACING_TOLERANCE -0.70f       // 45 degrees

plSceneInputInterface *plSceneInputInterface::fInstance = nullptr;

bool plSceneInputInterface::fShowLOS = false;


//// Constructor/Destructor //////////////////////////////////////////////////

plSceneInputInterface::plSceneInputInterface()
    : fPipe(), fCurrentCursor(), fButtonState(), fClickability(),
      fCurrClickIsAvatar(), fLastClickIsAvatar(), fFadedLocalAvatar(),
      fPendingLink(), fBookMode(), fOffereeID()
{
    fAgeInstanceGuid.Clear();
    fInstance = this;
    SetEnabled(true);         // Always enabled
}

plSceneInputInterface::~plSceneInputInterface()
{
    ClearClickableMap();
    fIgnoredAvatars.clear();
    fLocalIgnoredAvatars.clear();
    fGUIIgnoredAvatars.clear();
    fInstance = nullptr;
}


//// Init/Shutdown ///////////////////////////////////////////////////////////

void    plSceneInputInterface::Init( plInputInterfaceMgr *manager )
{
    plInputInterface::Init( manager );

    // To get the pipeline
    fPipe = nullptr;
    plgDispatch::Dispatch()->RegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plInputIfaceMgrMsg::Index(), fManager->GetKey() );
    plgDispatch::Dispatch()->RegisterForExactType( plPlayerPageMsg::Index(), fManager->GetKey() );

    fCurrentClickable = nullptr;
    fCurrentClickableLogicMod = nullptr;
    fLastClicked = nullptr;
    fButtonState = 0;
    fClickability = 1;      // Hack for clickable avatars, we need to always do the LOS check
    fLastClickIsAvatar = false;
    fCurrClickIsAvatar = false;
    fFadedLocalAvatar = false;
    fBookMode = kNotOffering;
    fOffereeKey = nullptr;
    fPendingLink = false;

    // register for control messages
    plCmdIfaceModMsg* pModMsg = new plCmdIfaceModMsg;
    pModMsg->SetBCastFlag(plMessage::kBCastByExactType);
    pModMsg->SetSender(fManager->GetKey());
    pModMsg->SetCmd(plCmdIfaceModMsg::kAdd);
    plgDispatch::MsgSend(pModMsg);

}

void    plSceneInputInterface::Shutdown()
{
    if (fPipe == nullptr)
        plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
    else
        fPipe = nullptr;
}

void plSceneInputInterface::ClearClickableMap()
{
    for (clickableTest* pTest : fClickableMap)
        delete pTest;
    fClickableMap.clear();
}

//// IHalfFadeAvatar /////////////////////////////////////////////////////////

void plSceneInputInterface::IHalfFadeAvatar(bool out)
{
    plIfaceFadeAvatarMsg* pMsg = new plIfaceFadeAvatarMsg();
    pMsg->SetSubjectKey(plNetClientMgr::GetInstance()->GetLocalPlayerKey());
    pMsg->SetBCastFlag(plMessage::kBCastByExactType);
    pMsg->SetBCastFlag(plMessage::kNetPropagate, false);
    pMsg->SetFadeOut(out);
    pMsg->Send();
    fFadedLocalAvatar = out;

}


void plSceneInputInterface::ResetClickableState()
{
    if (fLastClicked != nullptr)
        ISetLastClicked(nullptr, {});

    ClearClickableMap();
    fCurrentClickable = nullptr;
    fCurrentClickableLogicMod = nullptr;
    fCurrentCursor = SetCurrentCursorID(kNullCursor);
    fCurrClickIsAvatar = false;
    
}
//// IEval ///////////////////////////////////////////////////////////////////

bool plSceneInputInterface::IEval( double secs, float del, uint32_t dirty )
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
    
//  if (!fCurrClickIsAvatar)
//      fCurrentCursor = SetCurrentCursorID(kNullCursor);
    // ping for possible cursor changes
    for (clickableTest* pTest : fClickableMap)
    {
        plFakeOutMsg *pMsg = new plFakeOutMsg;
        pMsg->SetSender( fManager->GetKey() );
        pMsg->AddReceiver(pTest->key);
        plgDispatch::MsgSend( pMsg );
    }
    // then see if we have any
    bool change = std::any_of(fClickableMap.cbegin(), fClickableMap.cend(),
                              [](clickableTest* pTest) { return pTest->val; });
    if (change)
    {
        if (fLastClicked != nullptr)
            fCurrentCursor = SetCurrentCursorID(kCursorClicked);
        else
            fCurrentCursor = SetCurrentCursorID(kCursorPoised);
    }
    return true;
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    plSceneInputInterface::MsgReceive( plMessage *msg )
{
    plLOSHitMsg *pLOSMsg = plLOSHitMsg::ConvertNoRef( msg );
    if( pLOSMsg )
    {
        if( pLOSMsg->fRequestID == ID_FIND_CLICKABLE )
        {
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
                            plDetectorLog::Special("{}: LOS miss", pObj->GetKeyName());
                        else
                            plDetectorLog::Special("{}: LOS hit", pObj->GetKeyName());
                    }
                    const plInterfaceInfoModifier* pMod = nullptr;
                    for (size_t i = 0; i < pObj->GetNumModifiers(); i++)
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
                                {   // is it the current clickable already?
                                    ClearClickableMap();
                                    fCurrentCursor = SetCurrentCursorID(kNullCursor);
                                    fCurrentClickable = pObj->GetKey();
                                    fCurrentClickableLogicMod = pLogicMod->GetKey();
                                    fCurrentClickPoint = pLOSMsg->fHitPoint;
                                    for (size_t x = 0; x < pMod->GetNumReferencedKeys(); x++)
                                        fClickableMap.emplace_back(new clickableTest(pMod->GetReferencedKey(x)));
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
                            bool amCCR = plNetClientMgr::GetInstance()->GetCCRLevel();
                            
                            // is this person a NPC or CCR?
                            plNetTransportMember* pMbr = plNetClientMgr::GetInstance()->TransportMgr().GetMemberByKey(pObj->GetKey());
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
                                fCurrentClickableLogicMod = nullptr;
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
                                    float viewdot = ourView * theirView;
                                    hsVector3 towards(locPlayer->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate());
                                    towards.Normalize();
                                    float towardsdot = ourView * towards;
                                    if (viewdot > SHARE_FACING_TOLERANCE || towardsdot > SHARE_FACING_TOLERANCE )
                                    {
                                        ResetClickableState();
                                        return true;    // not facing enough... reject
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
                                    
                                    if (fabs(avPt.fZ - objPt.fZ) > 1.0f) // you need to also be in the same plane (some books are on top of rocks you need to jump onto)
                                    {
                                        ResetClickableState();
                                        return true;
                                    }
                                }
                            }
                            // finally - make sure this guy is not in our ignore lists.
                            for (const plKey& avatarKey : fIgnoredAvatars)
                            {
                                if (avatarKey == pObj->GetKey())
                                {
                                    fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
                                    plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
                                    return true;
                                }
                            }
                            for (const plKey& avatarKey : fGUIIgnoredAvatars)
                            {
                                if (avatarKey == pObj->GetKey())
                                {
                                    fCurrentCursor = SetCurrentCursorID(kCursorClickDisabled);
                                    plMouseDevice::AddNameToCursor(plNetClientMgr::GetInstance()->GetPlayerName(pObj->GetKey()));
                                    return true;
                                }
                            }
                            
                            ClearClickableMap();
                            fCurrentClickable = pObj->GetKey();
                            fCurrentClickableLogicMod = nullptr;
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

    plCursorChangeMsg   *fakeReplyMsg = plCursorChangeMsg::ConvertNoRef( msg );
    if (fakeReplyMsg != nullptr)
    {
        bool deniedCurrent = false;
        plKey key = fakeReplyMsg->GetSender();
        for (clickableTest* pTest : fClickableMap)
        {
            if (pTest->key == key)
            {
                if( fakeReplyMsg->fType == plCursorChangeMsg::kNullCursor )
                {
                    // Means not clickable--gotta fix this someday
                    pTest->val = false;
                    if (pTest->key == fCurrentClickableLogicMod)
                    {
                        deniedCurrent = true;
                        break;
                    }
                }
                else
                {
                    // And fix this...
                    pTest->val = true;
                }
            }
        }
        if (deniedCurrent)
            ResetClickableState();
        return true;
    }

    plRenderMsg *rMsg = plRenderMsg::ConvertNoRef( msg );
    if (rMsg != nullptr)
    {
        fPipe = rMsg->Pipeline();
        plgDispatch::Dispatch()->UnRegisterForExactType( plRenderMsg::Index(), fManager->GetKey() );
        return true;
    }
    // reply from coop share book multistage
    plNotifyMsg* pNMsg = plNotifyMsg::ConvertNoRef(msg);
    if (pNMsg)
    {
        for(size_t x = 0; x < pNMsg->GetEventCount(); x++)
        {
            proEventData* pED = pNMsg->GetEventRecord(x);
            if ( pED->fEventType == proEventData::kMultiStage )
            {
                proMultiStageEventData* pMS = (proMultiStageEventData*)pED;
                if (pMS->fAvatar == fOffereeKey) // mojo has linked
                {
                    // do something - they linked out but we are still in the multistage
                    fOffereeKey = nullptr;
                }
                else
                if (pMS->fAvatar == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
                {
                    // do something else
                    if (fBookMode == kNotOffering && fPendingLink == false) // we just linked out
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
            // first, remove this avatar from my list of avatars I ingore for clickable griefing (when the 'ignore avatars' key is pressed)
            for (auto iter = fLocalIgnoredAvatars.cbegin(); iter != fLocalIgnoredAvatars.cend(); )
            {
                if (*iter == pPlayerMsg->fPlayer)
                    iter = fLocalIgnoredAvatars.erase(iter);
                else
                    ++iter;
            }
            // now deal with avatars we are always ignoring because of their current activity
            for (auto iter = fIgnoredAvatars.cbegin(); iter != fIgnoredAvatars.cend(); )
            {
                if (*iter == pPlayerMsg->fPlayer)
                    iter = fIgnoredAvatars.erase(iter);
                else
                    ++iter;
            }
            for (auto iter = fGUIIgnoredAvatars.cbegin(); iter != fGUIIgnoredAvatars.cend(); )
            {
                if (*iter == pPlayerMsg->fPlayer)
                    iter = fGUIIgnoredAvatars.erase(iter);
                else
                    ++iter;
            }
            if (fOffereeKey == pPlayerMsg->fPlayer)
            {
                if (fBookMode == kBookOffered)
                {
                    // and put our own dialog back up...
                    plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
                    ISendOfferNotification(avKey, 0, false);
                    //IManageIgnoredAvatars(fOffereeKey, false);
                    fOffereeKey = nullptr;
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
                fLocalIgnoredAvatars.emplace_back(pPlayerMsg->fPlayer);
            if (fBookMode != kNotOffering)
            {
                // tell them to ignore us
                
                plInputIfaceMgrMsg* pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
                plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
                pMsg->SetAvKey(avKey);
                pMsg->SetBCastFlag(plMessage::kNetPropagate);
                pMsg->SetBCastFlag(plMessage::kNetForce);
                pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
                pMsg->AddNetReceiver( pPlayerMsg->fClientID );
                pMsg->Send();
                
                // and tell them to ignore our victim

                //plInputIfaceMgrMsg* pMsg2 = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
                //pMsg2->SetAvKey(fOffereeKey);
                //pMsg2->SetBCastFlag(plMessage::kNetPropagate);
                //pMsg2->SetBCastFlag(plMessage::kNetForce);
                //pMsg2->SetBCastFlag(plMessage::kLocalPropagate, false);
                //pMsg2->AddNetReceiver( pPlayerMsg->fClientID );
                //pMsg2->Send();
                            
            }
            // tell them to ingore us if we are looking at a GUI
            for (const auto& avatarKey : fGUIIgnoredAvatars)
            {
                if (avatarKey == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
                {
                    plInputIfaceMgrMsg* pMsg3 = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kGUIDisableAvatarClickable);
                    pMsg3->SetAvKey(avatarKey);
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
    if (mgrMsg != nullptr)
    {
        if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kDisableAvatarClickable )
        {
            // ignore if already in list or this is who WE are offering the book to...
            if (mgrMsg->GetAvKey() == fOffereeKey)
                return true;
            for (const plKey& avatarKey : fIgnoredAvatars)
            {
                if (avatarKey == mgrMsg->GetAvKey())
                    return true;
            }
            fIgnoredAvatars.emplace_back(mgrMsg->GetAvKey());
        }
        else
        if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kEnableAvatarClickable )
        {
            for (auto iter = fIgnoredAvatars.cbegin(); iter != fIgnoredAvatars.cend(); )
            {
                if (*iter == mgrMsg->GetAvKey())
                    iter = fIgnoredAvatars.erase(iter);
                else
                    ++iter;
            }
        }
        else
        if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kGUIDisableAvatarClickable )
        {
            // ignore if already in list or this is who WE are offering the book to...
            if (mgrMsg->GetAvKey() == fOffereeKey)
                return true;
            for (const plKey& avatarKey : fGUIIgnoredAvatars)
            {
                if (avatarKey == mgrMsg->GetAvKey())
                    return true;
            }
            fGUIIgnoredAvatars.emplace_back(mgrMsg->GetAvKey());
        }
        else
        if ( mgrMsg->GetCommand() == plInputIfaceMgrMsg::kGUIEnableAvatarClickable )
        {
            for (auto iter = fGUIIgnoredAvatars.cbegin(); iter != fGUIIgnoredAvatars.cend(); )
            {
                if (*iter == mgrMsg->GetAvKey())
                    iter = fGUIIgnoredAvatars.erase(iter);
                else
                    ++iter;
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
            fOffereeKey = nullptr;
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
            if (fOffereeKey != nullptr)
            {
                // notify any offeree that the offer is rescinded
                ISendOfferNotification(fOffereeKey, -999, true);
                //IManageIgnoredAvatars(fOffereeKey, false);
                fOffereeKey = nullptr;
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
                plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
                ISendOfferNotification(avKey, 0, false);
                //IManageIgnoredAvatars(fOffereeKey, false);
                fBookMode = kOfferBook;
                fOffereeKey = nullptr;
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
            fAgeInstanceGuid = plUUID(mgrMsg->GetAgeInstanceGuid());
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

    bool isAgeInstanceGuidSet = fAgeInstanceGuid.IsSet();
    
    plAgeLinkStruct link;
    
    if (isAgeInstanceGuidSet) {
        info.SetAgeInstanceGuid(&fAgeInstanceGuid);
        link.GetAgeInfo()->CopyFrom(&info);

        fAgeInstanceGuid.Clear();
    }
    else if (!VaultGetOwnedAgeLink(&info, &link)) {
    
        // We must have an owned copy of the age before we can offer it, so make one now
        plUUID guid = plUUID::Generate();
        info.SetAgeInstanceGuid(&guid);
        ST::string title, desc;

        ST::string playerName = plNetClientMgr::GetInstance()->GetPlayerName();
        if (playerName.back() == 's' || playerName.back() == 'S') {
            title = ST::format("{}'", playerName);
            desc = ST::format("{}' {}", playerName,
                              link.GetAgeInfo()->GetAgeInstanceName());
        }
        else {
            title = ST::format("{}'s", playerName);
            desc = ST::format("{}'s {}", playerName,
                              link.GetAgeInfo()->GetAgeInstanceName());
        }

        info.SetAgeUserDefinedName(std::move(title));
        info.SetAgeDescription(std::move(desc));

        link.GetAgeInfo()->CopyFrom(&info);
        if (!VaultRegisterOwnedAgeAndWait(&link)) {
            // failed to become an owner of the age for some reason, offer cannot continue
            return;
        }
    }
    else if (hsRef<RelVaultNode> linkNode = VaultGetOwnedAgeLink(&info)) {
        // We have the age in our AgesIOwnFolder. If its volatile, dump it for the new one.
        VaultAgeLinkNode linkAcc(linkNode);
        if (linkAcc.GetVolatile()) {
            if (VaultUnregisterOwnedAge(link.GetAgeInfo())) {
                plUUID guid = plUUID::Generate();
                link.GetAgeInfo()->SetAgeInstanceGuid(&guid);
                VaultRegisterOwnedAgeAndWait(&link);
            }
        }
    }

    if (!fSpawnPoint.empty()) {
        plSpawnPointInfo spawnPoint;
        spawnPoint.SetName(fSpawnPoint);
        link.SetSpawnPoint(std::move(spawnPoint));
    }
    
            
    // We now own the age, offer it

    if (fOfferedAgeFile.compare_i(kPersonalAgeFilename) == 0)
        plNetLinkingMgr::GetInstance()->OfferLinkToPlayer(&link, fOffereeID, fManager->GetKey());
    else
        plNetLinkingMgr::GetInstance()->LinkPlayerToAge(&link, fOffereeID);
        
    if (!fPendingLink && fOfferedAgeFile.compare_i(kPersonalAgeFilename) != 0)
    {   
        // tell our local dialog to pop up again...
        plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
        ISendOfferNotification(avKey, 0, false);
        // make them clickable again(in case they come back?)
        //IManageIgnoredAvatars(fOffereeKey, false);
        
        fBookMode = kNotOffering;
        fOffereeKey = nullptr;
        fPendingLink = false;
    }
    else // this is a yeesha book link, must wait for multistage callbacks
    {
        // commented out until after 0.9
        fBookMode = kOfferLinkPending;
        fPendingLink = true;
//          fBookMode = kNotOffering;
//          fOffereeKey = nullptr;
//          fPendingLink = false;
//          ISendAvatarDisabledNotification(true);
    }
}

//// ISetLastClicked /////////////////////////////////////////////////////////

#define MATT_WAS_HERE

void    plSceneInputInterface::ISetLastClicked( plKey obj, hsPoint3 hitPoint )
{
    if (fBookMode != kNotOffering)
        return;
    
    if (fLastClicked != nullptr)
    {
        // Send an "un-picked" message to it
        if( !fLastClickIsAvatar )
        {
            plPickedMsg *pPickedMsg = new plPickedMsg;
            pPickedMsg->AddReceiver( fLastClicked );
            pPickedMsg->fPicked = false;
            plgDispatch::MsgSend( pPickedMsg );
        }
        else
        {
            plRemoteAvatarInfoMsg *pMsg = new plRemoteAvatarInfoMsg;
            pMsg->SetAvatarKey(nullptr);
            plgDispatch::MsgSend( pMsg );
        }
    }

    fLastClickIsAvatar = (obj == nullptr) ? false : fCurrClickIsAvatar;
    fLastClicked = std::move(obj);

    if (fLastClicked != nullptr)
    {
#ifdef MATT_WAS_HERE
    // now we send pick messages to avatars as well...
    plPickedMsg *pPickedMsg = new plPickedMsg;
    pPickedMsg->AddReceiver( fLastClicked );
    pPickedMsg->fHitPoint = hitPoint;
    plgDispatch::MsgSend( pPickedMsg );

    // if it's an avatar, we also send this thing
    if(fLastClickIsAvatar)
    {                   
        plRemoteAvatarInfoMsg *pMsg = new plRemoteAvatarInfoMsg;
        pMsg->SetAvatarKey( fLastClicked );
        plgDispatch::MsgSend( pMsg );
    }
#else
    // Send a "picked" message to it
    if( !fLastClickIsAvatar )
    {
        plPickedMsg *pPickedMsg = new plPickedMsg;
        pPickedMsg->AddReceiver( fLastClicked );
        pPickedMsg->fHitPoint = hitPoint;
        plgDispatch::MsgSend( pPickedMsg );
    }
    else
    {                   
        plRemoteAvatarInfoMsg *pMsg = new plRemoteAvatarInfoMsg;
        pMsg->SetAvatarKey( fLastClicked );
        plgDispatch::MsgSend( pMsg );
    }
#endif
    }
}

//// InterpretInputEvent /////////////////////////////////////////////////////

bool plSceneInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
    plControlEventMsg* pControlEvent = plControlEventMsg::ConvertNoRef(pMsg);
    if (pControlEvent)
    {
        if (pControlEvent->GetControlCode() == B_CONTROL_IGNORE_AVATARS)
        {
            for (const plKey& avatarKey : fLocalIgnoredAvatars)
            {
                plSceneObject* pObj = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
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
    if (mouseMsg != nullptr)
    {
        // you're suspended when in this mode...
        if (fBookMode == kOfferLinkPending || fBookMode == kOfferAccepted)
            return true;

        if( mouseMsg->GetButton() == kLeftButtonDown )
        {
            if (fCurrentClickable != nullptr && fLastClicked == nullptr && fCurrentCursor != kNullCursor)
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
                fOffereeKey = nullptr;
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
                    plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
                    ISendOfferNotification(avKey, 0, false);
                    //IManageIgnoredAvatars(fOffereeKey, false);
                    fBookMode = kOfferBook;
                    fOffereeKey = nullptr;
                }   
                else
                if (fBookMode == kOfferBook)
                {
                    fBookMode = kNotOffering;
                    fOffereeKey = nullptr;
                    ISendAvatarDisabledNotification(true);
                }
            }
            if (fLastClicked != nullptr)
            {
                fButtonState &= ~kLeftButtonDown;
                ISetLastClicked(nullptr, {});
                
                return true;
            }
        }
    }

    return false;
}


//// ISendOfferNotification ////////////////////////////////////////////////////////
void plSceneInputInterface::IManageIgnoredAvatars(const plKey& offeree, bool add)
{
    // tell everyone else to be able to / not to be able to select this avatar
    plInputIfaceMgrMsg* pMsg = nullptr;
    if (!add)
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableAvatarClickable);
    else
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
    pMsg->SetAvKey(offeree);
    pMsg->SetBCastFlag(plMessage::kNetPropagate);
    pMsg->SetBCastFlag(plMessage::kNetForce);
    pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
    pMsg->Send();
}   

void plSceneInputInterface::ISendOfferNotification(const plKey& offeree, int ID, bool net)
{
    int offereeID = -1;
    if (offeree == plNetClientMgr::GetInstance()->GetLocalPlayerKey())
    {
        offereeID = plNetClientMgr::GetInstance()->GetPlayerID();
    }
    else
    {
        for (plNetTransportMember* mbr : plNetClientMgr::GetInstance()->TransportMgr().GetMemberList())
        {
            if (mbr != nullptr && mbr->GetAvatarKey() == offeree)
            {
                offereeID = mbr->GetPlayerID();
                break;
            }
        }
    }
    plNotifyMsg* pMsg = new plNotifyMsg;
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

void plSceneInputInterface::ISendAvatarDisabledNotification(bool enabled)
{
    plInputIfaceMgrMsg* pMsg = nullptr;
    if (enabled)
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kEnableAvatarClickable);
    else
        pMsg = new plInputIfaceMgrMsg(plInputIfaceMgrMsg::kDisableAvatarClickable);
    plKey avKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    pMsg->SetAvKey(avKey);
    pMsg->SetBCastFlag(plMessage::kNetPropagate);
    pMsg->SetBCastFlag(plMessage::kNetForce);
    pMsg->SetBCastFlag(plMessage::kLocalPropagate, false);
    pMsg->Send();
}


//// IRequestLOSCheck ////////////////////////////////////////////////////////

void    plSceneInputInterface::IRequestLOSCheck( float xPos, float yPos, int ID )
{
    if (fPipe == nullptr)
        return;
    

    int32_t x=(int32_t) ( xPos * fPipe->Width() );
    int32_t y=(int32_t) ( yPos * fPipe->Height() );

    hsPoint3 endPos, startPos;
    
    fPipe->ScreenToWorldPoint( 1,0, &x, &y, 10000, 0, &endPos );
    startPos = fPipe->GetViewPositionWorld();

    // move the start pos out a little to avoid backing up against physical objects...
    hsVector3 view(endPos - startPos);
    view.Normalize();
    startPos = startPos + (view * 0.3f);

    plLOSRequestMsg* pMsg;
    
    if(ID == ID_FIND_CLICKABLE) {
        pMsg = new plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBUIItems, plLOSRequestMsg::kTestClosest );
        pMsg->SetRequestName(ST_LITERAL("Scene Input Interface: Find Clickable"));
        pMsg->SetCullDB(plSimDefs::kLOSDBUIBlockers);
    } else if(ID == ID_FIND_WALKABLE_GROUND) {
        pMsg = new plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBAvatarWalkable, plLOSRequestMsg::kTestClosest);
        pMsg->SetRequestName(ST_LITERAL("Scene Input Interface: Find Terrain"));
    } else {
        pMsg = new plLOSRequestMsg( fManager->GetKey(), startPos, endPos, plSimDefs::kLOSDBLocalAvatar, plLOSRequestMsg::kTestClosest);
        pMsg->SetRequestName(ST_LITERAL("Scene Input Interface: Find Local Avatar"));
    }
    pMsg->SetReportType( plLOSRequestMsg::kReportHitOrMiss );

    pMsg->SetRequestID( ID );

    plgDispatch::MsgSend( pMsg );

    fLastStartPt = startPos;
    fLastEndPt = endPos;
}
        
//// IWorldPosMovedSinceLastLOSCheck /////////////////////////////////////////

bool    plSceneInputInterface::IWorldPosMovedSinceLastLOSCheck()
{
    if (fPipe == nullptr)
        return false;

    int32_t x=(int32_t) ( plMouseDevice::Instance()->GetCursorX() * fPipe->Width() );
    int32_t y=(int32_t) ( plMouseDevice::Instance()->GetCursorY() * fPipe->Height() );

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

uint32_t plSceneInputInterface::SetCurrentCursorID(uint32_t id)
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
