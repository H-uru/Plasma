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

#include "cyAvatar.h"

#include <string_theory/format>

#include "plFileSystem.h"
#include "plgDispatch.h"
#include "plPhysical.h"

#include "pnMessage/plNotifyMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plAGAnim.h" // to get the BodyUsage enum
#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarClothing.h"
#include "plAvatar/plAvatarMgr.h"
#include "plAvatar/plAvBrainHuman.h" // needed to call the emote
#include "plAvatar/plClothingLayout.h"
#include "plAvatar/plMultistageBehMod.h"
#include "plAvatar/plOneShotMod.h"
#include "plDrawable/plMorphSequence.h"
#include "plDrawable/plSharedMesh.h"
#include "plInputCore/plAvatarInputInterface.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plMultistageMsg.h"
#include "plMessage/plOneShotMsg.h"
#include "plMessage/plSimStateMsg.h"
#include "plVault/plVault.h"

#include "pyColor.h"
#include "pyGlueHelpers.h"
#include "pyImage.h"
#include "pyKey.h"
#include "pySceneObject.h"

///////////////////////////////////////////////////////////////////////////
//
// LOCAL FORWARD DECLs
//
///////////////////////////////////////////////////////////////////////////
bool IEnterGenericMode(const ST::string& enterAnim, const ST::string& idleAnim, const ST::string& exitAnim, bool autoExit, plAGAnim::BodyUsage bodyUsage,
                       plAvBrainGeneric::BrainType = plAvBrainGeneric::kGeneric);
bool IExitTopmostGenericMode();


cyAvatar::cyAvatar(plKey sender, plKey recvr)
{
    SetSender(std::move(sender));
    AddRecvr(std::move(recvr));
    fNetForce = false;
}

// setters
void cyAvatar::SetSender(plKey sender)
{
    fSender = std::move(sender);
}

void cyAvatar::AddRecvr(plKey recvr)
{
    if (recvr != nullptr)
        fRecvr.emplace_back(std::move(recvr));
}

void cyAvatar::SetSenderKey(const pyKey& pKey)
{
    SetSender(pKey.getKey());
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindArmatureModKey
//  PARAMETERS : avObj  - avatar sceneobject
//
//  PURPOSE    : find the armature mod for this sceneoabject (if its an avatar)
//
const plArmatureMod* cyAvatar::IFindArmatureMod(const plKey& avKey)
{
    plSceneObject* avObj = plSceneObject::ConvertNoRef(avKey->ObjectIsLoaded());
    if ( avObj )
    {
        // search through its modifiers to see if one of them is an avatar modifier
        for (size_t i = 0; i < avObj->GetNumModifiers(); i++)
        {
            const plModifier* mod = avObj->GetModifier(i);
            // see if it is an avatar mod base class
            const plArmatureMod* avmod = plArmatureMod::ConvertNoRef(mod);
            if ( avmod )
                return avmod;
        }
    }
    // otherwise we didn't find anything
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IFindArmatureModKey
//  PARAMETERS : avObj  - avatar sceneobject
//
//  PURPOSE    : find the armature mod for this sceneoabject (if its an avatar)
//
plKey cyAvatar::IFindArmatureModKey(const plKey& avKey)
{
    const plArmatureMod* avatar = IFindArmatureMod(avKey);
    if ( avatar )
        return avatar->GetKey();
    // otherwise we didn't find anything
    return nullptr;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : oneShot
//  PARAMETERS : 
//
//  PURPOSE    : oneShot Avatar (must already be there)
//
void cyAvatar::OneShot(pyKey &seekKey, float duration, bool usePhysics,
               const ST::string &animName, bool drivable, bool reversible)
{
    if (!fRecvr.empty())
    {
        // create message
        plAvOneShotMsg* pMsg = new plAvOneShotMsg(
            (plKey )fSender,
            nullptr,
            seekKey.getKey(),   // Mark D told me to do it ...paulg
            duration,  
            usePhysics,  
            animName,
            drivable, 
            reversible);

        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        // must have a receiver!
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunBehavior
//  PARAMETERS : 
//
//  PURPOSE    : Run Behavior, could be single or multi-stage shot
//
void cyAvatar::RunBehavior(pyKey &behKey, bool netForce, bool netProp)
{
    // first there is someone to send to and make sure that we an avatar to send this to
    if (behKey.getKey() && !fRecvr.empty())
    {
        // must determine if the behKey is pointing to Single or Multi Shot behavior
        if (plOneShotMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()))
        {
            // create a message OneShotMessage
            plOneShotMsg* pMsg = new plOneShotMsg;
            pMsg->SetBCastFlag(plMessage::kNetPropagate, netProp || netForce);
            pMsg->SetBCastFlag(plMessage::kNetForce, netForce);
            pMsg->SetSender(fSender);
            pMsg->AddReceiver(behKey.getKey());
            for (const plKey& rcKey : fRecvr) {
                // make sure there is an avatar to set
                if (rcKey) {
                    pMsg->fPlayerKey = rcKey;
                    pMsg->SendAndKeep(); // gotta keep the message so we can keep sending it
                                         // there should really only be one avatar, though...
                }
            }
            pMsg->UnRef(); // done with our reference
        }
        // else if it is a Multistage guy
        else if (plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nullptr)
        {
            // its a multistage thingy... need to send it a plNotifyMsg
            // create new notify message to do the actual send with
            plNotifyMsg* pNMsg = new plNotifyMsg;

            // set whether this should be forced over the network (ignoring net-cascading)
            if (netProp)
            {
                pNMsg->SetBCastFlag(plMessage::kNetPropagate);
            }
            else
            {
                pNMsg->SetBCastFlag(plMessage::kNetPropagate, false);
            }

            if ( netForce )
            {
                pNMsg->SetBCastFlag(plMessage::kNetPropagate);
                pNMsg->SetBCastFlag(plMessage::kNetForce);
            }
            else
            {
                pNMsg->SetBCastFlag(plMessage::kNetForce, false);
            }

            // copy data and event records to new NotifyMsg
            pNMsg->fState = 1.0;
            // need to recreate all the events in the new message by Adding them
            if (!fRecvr.empty() && fRecvr[0] != nullptr)
            {
                pNMsg->AddPickEvent((plKey)fRecvr[0], nullptr, true, {});
            }

            // add receivers
            // loop though adding the ones that want to be notified of the change
            pNMsg->AddReceiver(behKey.getKey());
            pNMsg->SetSender(fSender);
            plgDispatch::MsgSend( pNMsg );
        }

    }
}
/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunBehaviorAndReply
//  PARAMETERS : 
//
//  PURPOSE    : Run Behavior, multistage only, reply to specified key'd object
//
void cyAvatar::RunBehaviorAndReply(pyKey& behKey, pyKey& replyKey, bool netForce, bool netProp)
{
    plMultistageBehMod* pMod = plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr());   
    if ( pMod )
    {
        // its a multistage thingy... need to send it a plNotifyMsg
        // create new notify message to do the actual send with
        plNotifyMsg* pNMsg = new plNotifyMsg;

        // set whether this should be forced over the network (ignoring net-cascading)
        if (netProp)
        {
            pNMsg->SetBCastFlag(plMessage::kNetPropagate);
        }
        else
        {
            pNMsg->SetBCastFlag(plMessage::kNetPropagate, false);
        }

        if (netForce)
        {
            // set the network propagate flag to make sure it gets to the other clients
            pNMsg->SetBCastFlag(plMessage::kNetPropagate);
            pNMsg->SetBCastFlag(plMessage::kNetForce);
        }
        else
        {
            pNMsg->SetBCastFlag(plMessage::kNetForce, false);
        }

        // copy data and event records to new NotifyMsg
        pNMsg->fState = 1.0;
        // need to recreate all the events in the new message by Adding them
        if (!fRecvr.empty() && fRecvr[0] != nullptr)
        {
            pNMsg->AddPickEvent((plKey)fRecvr[0], nullptr, true, {});
        }

        // add receivers
        // loop though adding the ones that want to be notified of the change
        pNMsg->AddReceiver(behKey.getKey());
        pNMsg->SetSender(replyKey.getKey());
        plgDispatch::MsgSend( pNMsg );
    }

}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RunCoopAnim
//  PARAMETERS : targetKey - target avatar pyKey
//               activeAvatarAnim - animation name
//               targetAvatarAnim - animation name
//               range - how far away are we allowed to be? (default in glue: 6)
//               dist - how close shall the avatar move? (default in glue: 3)
//               move - shall he move at all? (default in glue: true)
//
//  PURPOSE    : Seek near another avatar and run animations on both
//
bool cyAvatar::RunCoopAnim(pyKey& targetKey, ST::string activeAvatarAnim, ST::string targetAvatarAnim, float range, float dist, bool move)
{
    if (!fRecvr.empty() && fRecvr[0]) {
        // get the participating avatars
        plArmatureMod* activeAv = plAvatarMgr::FindAvatar(fRecvr[0]);
        plArmatureMod* targetAv = plAvatarMgr::FindAvatar(targetKey.getKey());

        if (activeAv && targetAv) {
            // build the gender-specific animation name
            activeAvatarAnim = activeAv->MakeAnimationName(activeAvatarAnim);
            targetAvatarAnim = targetAv->MakeAnimationName(targetAvatarAnim);

            // set seek position and rotation of the avatars
            hsPoint3 avPos, targetPos;
            activeAv->GetPositionAndRotationSim(&avPos, nullptr);
            targetAv->GetPositionAndRotationSim(&targetPos, nullptr);
            hsVector3 av2target(&targetPos, &avPos); //targetPos - avPos
            if (av2target.Magnitude() > range)
                return false;
            av2target.Normalize();
            if (move)
                avPos = targetPos - dist * av2target;

            // create the messages and let one task queue the next
            const int bcastToNetMods = plMessage::kNetPropagate | plMessage::kNetForce | plMessage::kPropagateToModifiers;
            plAvOneShotMsg *avAnim = new plAvOneShotMsg(nullptr, fRecvr[0], fRecvr[0], 0.f, true, activeAvatarAnim, false, false);
            avAnim->SetBCastFlag(bcastToNetMods);

            plAvOneShotMsg *targetAnim = new plAvOneShotMsg(nullptr, targetKey.getKey(), targetKey.getKey(), 0.f, true, targetAvatarAnim, false, false);
            targetAnim->SetBCastFlag(bcastToNetMods);
            targetAnim->fFinishMsg = avAnim;

            plAvSeekMsg *targetSeek = new plAvSeekMsg(nullptr, targetKey.getKey(), nullptr, 0.f, true);
            targetSeek->SetBCastFlag(bcastToNetMods);
            targetSeek->fTargetPos = targetPos;
            targetSeek->fTargetLookAt = avPos;
            targetSeek->fFinishMsg = targetAnim;

            plAvSeekMsg *avSeek = new plAvSeekMsg(nullptr, fRecvr[0], nullptr, 0.f, true);
            avSeek->SetBCastFlag(bcastToNetMods);
            avSeek->fTargetPos = avPos;
            avSeek->fTargetLookAt = targetPos;
            avSeek->fFinishMsg = targetSeek;

            // start the circus, messages are processed "backwards"
            avSeek->Send();
            return true;
        }
    }
    return false;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : NextStage
//  PARAMETERS : behKey  - behavior pyKey
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to the next stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::NextStage(pyKey &behKey, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce)
{
    // first there is someone to send to and make sure that we an avatar to send this to
    if (behKey.getKey() != nullptr && !fRecvr.empty())
    {
        // if it is a Multistage guy
        if (plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nullptr)
        {
            plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
            if ( avKey )
            {
                // create the message
                plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg((plKey)fSender, avKey,
                    plAvBrainGenericMsg::kNextStage, 0, setTime, newTime,
                    setDirection, isForward, transTime);

                if ( netForce )
                    pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

                plgDispatch::MsgSend( pMsg );
            }
        }

    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PreviousStage
//  PARAMETERS : behKey  - behavior pyKey
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to the previous stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::PreviousStage(pyKey &behKey, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce)
{
    // first there is someone to send to and make sure that we an avatar to send this to
    if (behKey.getKey() != nullptr && !fRecvr.empty())
    {
        // if it is a Multistage guy
        if (plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nullptr)
        {
            plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
            if ( avKey )
            {
                // create the message
                plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg((plKey)fSender, avKey,
                    plAvBrainGenericMsg::kPrevStage, 0, setTime, newTime,
                    setDirection, isForward, transTime);

                if ( netForce )
                    pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

                plgDispatch::MsgSend( pMsg );
            }
        }

    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GotoStage
//  PARAMETERS : behKey  - behavior pyKey
//             : stage   - stage number to go to
//             : transTime  - the transition time to the next stage
//   (optional): rewind   - whether to rewind to the front of the next stage
//
//  PURPOSE    : Go to a particular stage in a multi-stage behavior
//
// NOTE: only works with multi-stage behaviors
//
void cyAvatar::GoToStage(pyKey &behKey, int32_t stage, float transTime, bool setTime, float newTime,
                        bool setDirection, bool isForward, bool netForce)
{
    // first there is someone to send to and make sure that we an avatar to send this to
    if (behKey.getKey() != nullptr && !fRecvr.empty())
    {
        // if it is a Multistage guy
        if (plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nullptr)
        {
            plKey avKey = IFindArmatureModKey( (plKey)fRecvr[0] );
            if ( avKey )
            {
                // create the message
                plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg((plKey)fSender, avKey,
                    plAvBrainGenericMsg::kGotoStage, stage, setTime, newTime,
                    setDirection, isForward, transTime);

                if ( netForce )
                    pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

                plgDispatch::MsgSend( pMsg );
            }
        }

    }
}


void cyAvatar::SetLoopCount(pyKey &behKey, int32_t stage, int32_t loopCount, bool netForce)
{
    // if it is a Multistage guy
    if (plMultistageBehMod::ConvertNoRef(behKey.getKey()->GetObjectPtr()) != nullptr)
    {
        plMultistageModMsg* pMsg = new plMultistageModMsg({}, behKey.getKey());
        pMsg->SetCommand(plMultistageModMsg::kSetLoopCount);
        pMsg->fStageNum = (uint8_t)stage;
        pMsg->fNumLoops = (uint8_t)loopCount;

        if ( netForce )
            pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

        plgDispatch::MsgSend( pMsg );
    }
} 


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : seek
//  PARAMETERS : 
//
//  PURPOSE    : seek Avatar (must already be there)
//

/* Unsupported. Ask Bob if you want it back.

void cyAvatar::Seek(pyKey &seekKey, float duration, bool usePhysics)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAvSeekMsg* pMsg = new plAvSeekMsg(
            (plKey)fSender, nullptr, seekKey.getKey(), duration, usePhysics);

        // check if this needs to be network forced to all clients
        if (fNetForce )
        {
            // set the network propagate flag to make sure it gets to the other clients
            pMsg->SetBCastFlag(plMessage::kNetPropagate);
            pMsg->SetBCastFlag(plMessage::kNetForce);
        }
        pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
        // add all our receivers to the message receiver list
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}
*/

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAvatarClothingGroup
//  PARAMETERS : 
//
//  PURPOSE    : Return what clothing group the avatar is in
//
int32_t cyAvatar::GetAvatarClothingGroup()
{
    // find the avatar's armature modifier
    const plArmatureMod *avMod = nullptr;

    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if ( avMod )
            {
                return avMod->GetClothingOutfit()->fGroup;
            }
        }
    }
    return -1;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClosetClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<ST::string> cyAvatar::GetEntireClothingList(int32_t clothing_type)
{
    // Currently, just all the clothing available will be returned
    const std::vector<plClothingItem*> &clothingList = plClothingMgr::GetClothingMgr()->GetItemList();

    // create the string list to send to python...
    std::vector<ST::string> retVal;
    retVal.reserve(clothingList.size());
    for (plClothingItem* item : clothingList)
        retVal.emplace_back(item->GetName());

    return retVal;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClosetClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<PyObject*> cyAvatar::GetClosetClothingList(int32_t clothing_type)
{
    std::vector<PyObject*> retVal;

    // find the avatar's armature modifier
    const plArmatureMod *avMod = nullptr;

    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if ( avMod )
            {
                // Get all the clothes that we can wear
                std::vector<plClothingItem*> clothingList;
                plClothingMgr::GetClothingMgr()->GetItemsByGroup(avMod->GetClothingOutfit()->fGroup, clothingList);
                // create the string list to send to python... as a python object
                for (plClothingItem* item : clothingList)
                {
                    if ( clothing_type == -1 || item->fType == clothing_type )
                    {
                        // add this event record to the main event list (lists within a list)
                        // create list
                        PyObject* clothingItem = PyList_New(5);

                        // [0] = clothing name
                        PyList_SetItem(clothingItem, 0, PyUnicode_FromSTString(item->GetName()));
                        
                        // [1] = clothing type
                        PyList_SetItem(clothingItem, 1, PyLong_FromLong(item->fType));

                        // [2] = description
                        PyList_SetItem(clothingItem, 2, PyUnicode_FromSTString(item->fDescription));

                        // [3] = ptImage of icon
                        if (item->fThumbnail != nullptr)
                            // create a ptImage
                            PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
                        else
                            PyList_SetItem(clothingItem, 3, PyLong_FromLong(0));

                        // [4] = fCustomText
                        PyList_SetItem(clothingItem, 4, PyUnicode_FromSTString(item->fCustomText));

                        retVal.push_back(clothingItem);
                    }
                }
            }
        }
    }
    return retVal;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAvatarClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of the wearable items for this avatar of that clothing_type
//
std::vector<PyObject*> cyAvatar::GetAvatarClothingList()
{
    std::vector<PyObject*> retVal;
    // find the avatar's armature modifier
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if ( avMod )
            {
                // Currently, just all the clothing available will be returned
                const std::vector<plClothingItem*> &clothingList = avMod->GetClothingOutfit()->GetItemList();
                // create the string list to send to python... as a python object
                for (plClothingItem* item : clothingList)
                {
                    // add this event record to the main event list (lists within a list)
                    // create list
                    PyObject* clothingItem = PyList_New(5);

                    // [0] = clothing name
                    PyList_SetItem(clothingItem, 0, PyUnicode_FromSTString(item->GetName()));

                    // [1] = clothing type
                    PyList_SetItem(clothingItem, 1, PyLong_FromLong(item->fType));

                    // [2] = description
                    PyList_SetItem(clothingItem, 2, PyUnicode_FromSTString(item->fDescription));

                    // [3] = ptImage of icon
                    if (item->fThumbnail != nullptr)
                        // create a ptImage
                        PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
                    else
                        PyList_SetItem(clothingItem, 3, PyLong_FromLong(0));

                    // [4] = fCustomText
                    PyList_SetItem(clothingItem, 4, PyUnicode_FromSTString(item->fCustomText));

                    retVal.push_back(clothingItem);
                }
            }
        }
    }
    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetWardrobeClothingList
//  PARAMETERS : 
//
//  PURPOSE    : Return a list of items that are in the avatars closet
//
std::vector<PyObject*> cyAvatar::GetWardrobeClothingList()
{
    std::vector<PyObject*> retVal;
    std::vector<plClosetItem> closetList;
    plClothingMgr::GetClothingMgr()->GetClosetItems(closetList);
    // create the string list to send to python... as a python object
    for (const plClosetItem& item : closetList)
    {
        // add this event record to the main event list (lists within a list)
        // create list
        PyObject* closetItem = PyList_New(7);

        // [0] = clothing name
        PyList_SetItem(closetItem, 0, PyUnicode_FromSTString(item.fItem->GetName()));
        
        // [1] = clothing type
        PyList_SetItem(closetItem, 1, PyLong_FromLong(item.fItem->fType));
        
        // [2] = description
        PyList_SetItem(closetItem, 2, PyUnicode_FromSTString(item.fItem->fDescription));

        // [3] = ptImage of icon
        if (item.fItem->fThumbnail != nullptr)
            // create a ptImage
            PyList_SetItem(closetItem, 3, pyImage::New(item.fItem->fThumbnail->GetKey()));
        else
            PyList_SetItem(closetItem, 3, PyLong_FromLong(0));

        // [4] = fCustomText
        PyList_SetItem(closetItem, 4, PyUnicode_FromSTString(item.fItem->fCustomText));

        // [5] = fTint1
        PyList_SetItem(closetItem, 5, pyColor::New(item.fOptions.fTint1));
        
        // [6] = fTint2
        PyList_SetItem(closetItem, 6, pyColor::New(item.fOptions.fTint2));

        retVal.push_back(closetItem);
    }
    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : AddWardrobeClothingItem
//  PARAMETERS : clothing_name - the name of the clothing item to add to your wardrobe
//             : tint1 - layer one color
//             : tint2 - layer two color
//
//  PURPOSE    : To add a clothing item to the avatar's wardrobe (closet)
//
void cyAvatar::AddWardrobeClothingItem(const ST::string& clothing_name,pyColor& tint1,pyColor& tint2)
{
    plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);
    if ( item )
    {
        std::vector<plClosetItem> items(1);
        items[0].fItem = item;
        items[0].fOptions.fTint1.Set(tint1.getRed(), tint1.getGreen(), tint1.getBlue(), 1.f);
        items[0].fOptions.fTint2.Set(tint2.getRed(), tint2.getGreen(), tint2.getBlue(), 1.f);

        plClothingMgr::GetClothingMgr()->AddItemsToCloset(items);
    }
}   


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetUniqueMeshList
//  PARAMETERS : clothing_type - the type of clothing to get
//
//  PURPOSE    : Return a list of unique clothing items (each has a different mesh)
//             : that belong to the specific type
//
std::vector<PyObject*> cyAvatar::GetUniqueMeshList(int32_t clothing_type)
{
    std::vector<PyObject*> retVal;

    // find the avatar's armature modifier
    const plArmatureMod *avMod = nullptr;

    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if ( avMod )
            {
                // Get all the clothes that we can wear
                std::vector<plClothingItem*> clothingList;
                plClothingMgr::GetClothingMgr()->GetItemsByGroup(avMod->GetClothingOutfit()->fGroup, clothingList);
                plClothingMgr::GetClothingMgr()->FilterUniqueMeshes(clothingList); // filter all redundant meshes
                // create the string list to send to python... as a python object
                for (plClothingItem* item : clothingList)
                {
                    if ( clothing_type == -1 || item->fType == clothing_type )
                    {
                        // add this event record to the main event list (lists within a list)
                        // create list
                        PyObject* clothingItem = PyList_New(5);

                        // [0] = clothing name
                        PyList_SetItem(clothingItem, 0, PyUnicode_FromSTString(item->GetName()));

                        // [1] = clothing type
                        PyList_SetItem(clothingItem, 1, PyLong_FromLong(item->fType));

                        // [2] = description
                        PyList_SetItem(clothingItem, 2, PyUnicode_FromSTString(item->fDescription));

                        // [3] = ptImage of icon
                        if (item->fThumbnail != nullptr)
                            // create a ptImage
                            PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
                        else
                            PyList_SetItem(clothingItem, 3, PyLong_FromLong(0));

                        // [4] = fCustomText
                        PyList_SetItem(clothingItem, 4, PyUnicode_FromSTString(item->fCustomText));

                        retVal.push_back(clothingItem);
                    }
                }
            }
        }
    }
    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetAllWithSameMesh
//  PARAMETERS : clothing_name - the name of the mesh to get the textures of
//
//  PURPOSE    : Return a list of clothing items that have the same mesh as
//             : the item passed in
//
std::vector<PyObject*> cyAvatar::GetAllWithSameMesh(const ST::string& clothing_name)
{
    std::vector<PyObject*> retVal;

    // find the avatar's armature modifier
    const plArmatureMod *avMod = nullptr;

    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if ( avMod )
            {
                // Get all clothes with the same mesh as the one passed in
                std::vector<plClothingItem*> clothingList;
                plClothingMgr::GetClothingMgr()->GetAllWithSameMesh(plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name), clothingList);
                // create the string list to send to python... as a python object
                for (plClothingItem* item : clothingList)
                {
                    // add this event record to the main event list (lists within a list)
                    // create list
                    PyObject* clothingItem = PyList_New(5);

                    // [0] = clothing name
                    PyList_SetItem(clothingItem, 0, PyUnicode_FromSTString(item->GetName()));

                    // [1] = clothing type
                    PyList_SetItem(clothingItem, 1, PyLong_FromLong(item->fType));

                    // [2] = description
                    PyList_SetItem(clothingItem, 2, PyUnicode_FromSTString(item->fDescription));

                    // [3] = ptImage of icon
                    if (item->fThumbnail != nullptr)
                        // create a ptImage
                        PyList_SetItem(clothingItem, 3, pyImage::New(item->fThumbnail->GetKey()));
                    else
                        PyList_SetItem(clothingItem, 3, PyLong_FromLong(0));

                    // [4] = fCustomText
                    PyList_SetItem(clothingItem, 4, PyUnicode_FromSTString(item->fCustomText));

                    retVal.push_back(clothingItem);
                }
            }
        }
    }
    return retVal;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetMatchingClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Return the clothing item that matches this one
//             : If no match then returns the number 0
//
PyObject* cyAvatar::GetMatchingClothingItem(const ST::string& clothing_name)
{
    // Get all the clothes that we can wear
    plClothingItem* match = plClothingMgr::GetClothingMgr()->GetLRMatch(plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name));
    if ( match )
    {
        // create list
        PyObject* clothingItem = PyList_New(5);
        
        // [0] = clothing name
        PyList_SetItem(clothingItem, 0, PyUnicode_FromSTString(match->GetName()));

        // [1] = clothing type
        PyList_SetItem(clothingItem, 1, PyLong_FromLong(match->fType));

        // [2] = description
        PyList_SetItem(clothingItem, 2, PyUnicode_FromSTString(match->fDescription));

        // [3] = ptImage of icon
        if (match->fThumbnail != nullptr)
            // create a ptImage
            PyList_SetItem(clothingItem, 3, pyImage::New(match->fThumbnail->GetKey()));
        else
            PyList_SetItem(clothingItem, 3, PyLong_FromLong(0));

        // [4] = fCustomText
        PyList_SetItem(clothingItem, 4, PyUnicode_FromSTString(match->fCustomText));

        return clothingItem;
    }
    else
        return PyLong_FromLong(0);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : WearClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns 0, if clothing item was not found
//
bool cyAvatar::WearClothingItem(const ST::string& clothing_name)
{
    return WearClothingItemU(clothing_name,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RemoveClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns false, if clothing item was not found
//
bool cyAvatar::RemoveClothingItem(const ST::string& clothing_name)
{
    return RemoveClothingItemU(clothing_name,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
bool cyAvatar::TintClothingItem(const ST::string& clothing_name, pyColor& tint)
{
    return TintClothingItemU(clothing_name,tint,true);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemLayer
//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
//             : tint   - what color to change it to
//             : layer  - which layer to change (1 or 2)
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
bool cyAvatar::TintClothingItemLayer(const ST::string& clothing_name, pyColor& tint, uint8_t layer)
{
    return TintClothingItemLayerU(clothing_name,tint,layer,true);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : WearClothingItem
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns 0, if clothing item was not found
//
bool cyAvatar::WearClothingItemU(const ST::string& clothing_name, bool update)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                if ( fNetForce )
                    avMod->GetClothingOutfit()->AddItem(item, update, true, true);
                else
                    avMod->GetClothingOutfit()->AddItem(item, update);
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RemoveClothingItemU
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Wear a particular piece of clothing based on name of clothing item
//             : returns false, if clothing item was not found
//
bool cyAvatar::RemoveClothingItemU(const ST::string& clothing_name, bool update)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                if ( fNetForce )
                    avMod->GetClothingOutfit()->RemoveItem(item,update,true);
                else
                    avMod->GetClothingOutfit()->RemoveItem(item,update);
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemU
//  PARAMETERS : --- with update flag
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
bool cyAvatar::TintClothingItemU(const ST::string& clothing_name, pyColor& tint, bool update)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                avMod->GetClothingOutfit()->TintItem(item, tint.getRed(),tint.getGreen(),tint.getBlue(),update,true,fNetForce,true,plClothingElement::kLayerTint1);
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintClothingItemLayer
//  PARAMETERS : clothing_name   - name of the clothing item to change the color of
//             : tint   - what color to change it to
//             : layer  - which layer to change (1 or 2)
//
//  PURPOSE    : Tint a clothing item, i.e. change the color of it
//
bool cyAvatar::TintClothingItemLayerU(const ST::string& clothing_name, pyColor& tint, uint8_t layer, bool update)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                // Convert Python layer number to clothing code...
                if (layer == 2)
                    layer = plClothingElement::kLayerTint2;
                else 
                    layer = plClothingElement::kLayerTint1;
                avMod->GetClothingOutfit()->TintItem(item, tint.getRed(),tint.getGreen(),tint.getBlue(),update,true,fNetForce,true,layer);
                return true;
            }
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetClothingItemParameterString
//  PARAMETERS : 
//
//  PURPOSE    : Get the custom parameter string for a clothing item
//
ST::string cyAvatar::GetClothingItemParameterString(const ST::string& clothing_name)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                return item->fCustomText;
            }
        }
    }

    return "";
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
//
PyObject* cyAvatar::GetTintClothingItem(const ST::string& clothing_name)
{
    return GetTintClothingItemL(clothing_name, 1);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintClothingItem
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint a clothing item, i.e. change the color of it
//
PyObject* cyAvatar::GetTintClothingItemL(const ST::string& clothing_name, uint8_t layer)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());

            plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);

            if (avMod && item)
            {
                // Convert Python layer number to clothing code...
                if (layer == 2)
                    layer = plClothingElement::kLayerTint2;
                else 
                    layer = plClothingElement::kLayerTint1;
                hsColorRGBA tint = avMod->GetClothingOutfit()->GetItemTint(item,layer);
                return pyColor::New(tint);
            }
        }
    }

    ST::string errmsg = ST::format("Cannot find clothing item {} to find out what tint it is", clothing_name);
    PyErr_SetString(PyExc_KeyError, errmsg.c_str());
    // returning nullptr means an error occurred
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintSkin
//  PARAMETERS : 
//
//  PURPOSE    : Tint the skin of the player's avatar
//
void cyAvatar::TintSkin(pyColor& tint)
{
    TintSkinU(tint,true);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TintSkinU
//  PARAMETERS : 
//
//  PURPOSE    : Tint the skin of the player's avatar with optional update flag
//
void cyAvatar::TintSkinU(pyColor& tint, bool update)
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            avMod->GetClothingOutfit()->TintSkin(tint.getRed(),tint.getGreen(),tint.getBlue(),update,true);
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetTintSkin
//  PARAMETERS : 
//
//  PURPOSE    : Get the tint of the skin of the player's avatar
//
PyObject* cyAvatar::GetTintSkin()
{
    const plArmatureMod *avMod = nullptr;
    // we can really only talk to one avatar, so just get the first one (which is probably the only one)
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            hsColorRGBA tint = avMod->GetClothingOutfit()->fSkinTint;
            // now create the ptColor Python object
            return pyColor::New(tint);
        }
    }

    PyErr_SetString(PyExc_KeyError, "Cannot find the skin of the player. Whatever that means!");
    // returning nullptr means an error occurred
    return nullptr;
}

plMorphSequence* cyAvatar::LocalMorphSequence()
{
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if (!avMod)
        return nullptr;
    

    const plSceneObject *so = avMod->GetClothingSO(0); // grabbing the high LOD node
    if (!so)
        return nullptr;

    const plModifier* constSeq = nullptr;
    for (size_t i = 0; i < so->GetNumModifiers(); i++)
    {
        constSeq = so->GetModifier(i);
        if (constSeq && plMorphSequence::ConvertNoRef(constSeq))
        {
            return (plMorphSequence*)constSeq; // safe cast, we've already checked type (plus we're const_cast'ing).
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetMorph
//  PARAMETERS : clothing_name - the name of the clothing to morph
//             : layer - the layer to affect
//             : value - what the new value should be (clipped between -1 and 1)
//
//  PURPOSE    : Set the morph value of a specific layer of clothing
//
void cyAvatar::SetMorph(const ST::string& clothing_name, uint8_t layer, float value)
{
    plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);
    if( !item )
    {
        PyErr_SetString(PyExc_KeyError, "Item not found");
        return;
    }

    float wgtPlus;
    float wgtMinus;
    
    if(value > 1.0) value = 1.0;
    if(value < -1.0) value = -1.0;
    
    if (value > 0)
    {
        wgtPlus = value;
        wgtMinus = 0;
    }
    else
    {
        wgtMinus = -value;
        wgtPlus = 0;
    }
    
    if (!fRecvr.empty() && fRecvr[0] != nullptr)
    {
        plSceneObject *so = plSceneObject::ConvertNoRef(fRecvr[0]->GetObjectPtr());
        if (so != nullptr)
        {
            const plArmatureMod *avMod = (plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
            if (avMod && avMod->GetClothingOutfit())
            {
                avMod->GetClothingOutfit()->MorphItem(item, layer, 0, wgtPlus, true);
                avMod->GetClothingOutfit()->MorphItem(item, layer, 1, wgtMinus, true);
            }
        }
    }
            

}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetMorph
//  PARAMETERS : clothing_name - the name of the clothing to get the value from
//             : layer - the layer to get the value from
//
//  PURPOSE    : Returns the current morph value of the specific layer of clothing
//
float cyAvatar::GetMorph(const ST::string& clothing_name, uint8_t layer)
{
    plMorphSequence* seq = LocalMorphSequence();
    if( !seq )
    {
        PyErr_SetString(PyExc_KeyError, "Sequence not found");
        return 0;
    }

    plClothingItem *item = plClothingMgr::GetClothingMgr()->FindItemByName(clothing_name);
    if( !item )
    {
        PyErr_SetString(PyExc_KeyError, "Item not found");
        return 0;
    }

    plKey meshKey = item->fMeshes[0]->GetKey();

    if (layer >= seq->GetNumLayers(meshKey))
    {
        PyErr_SetString(PyExc_KeyError, "Layer index too high");
        return 0;
    }   

    float wgtPlus;
    float wgtMinus;

    wgtPlus = seq->GetWeight(layer,0,meshKey);
    wgtMinus = seq->GetWeight(layer,1,meshKey);

    if (wgtPlus > 0)
        return wgtPlus;
    else
        return -wgtMinus;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetSkinBlend
//  PARAMETERS : layer - the layer to affect
//             : value - what the new value should be (clipped between 0 and 1)
//
//  PURPOSE    : Set the skin blend for the specified layer
//
void cyAvatar::SetSkinBlend(uint8_t layer, float value)
{
    if (value < 0.0) value = 0.0;
    if (value > 1.0) value = 1.0;

    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    
    if (avMod)
    {
        avMod->GetClothingOutfit()->SetSkinBlend(value, (int)layer + plClothingElement::kLayerSkinBlend1 - 1);
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : GetSkinBlend
//  PARAMETERS : layer - the layer to get the blend for
//
//  PURPOSE    : Returns the current layer's skin blend
//
float cyAvatar::GetSkinBlend(uint8_t layer)
{
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    
    if (avMod)
    {
        return avMod->GetClothingOutfit()->GetSkinBlend((int)layer + plClothingElement::kLayerSkinBlend1 - 1);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SaveClothing
//  PARAMETERS :
//
//  PURPOSE    : Saves the current clothing to the vault (including morphs)
//
void cyAvatar::SaveClothing()
{
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    
    if (avMod)
        avMod->GetClothingOutfit()->SaveCustomizations();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterSubWorld
//  PARAMETERS : object  - a sceneobject that is in the subworld
//
//  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
//
void cyAvatar::EnterSubWorld(pySceneObject& object)
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            // get the sceneobject that we will use to find the subworld
            plKey SOkey = object.getObjKey();
            if ( SOkey )
            {
                plSceneObject *SO = plSceneObject::ConvertNoRef(SOkey->ObjectIsLoaded());
                if(SO)
                {
                    plSubWorldMsg *swMsg = new plSubWorldMsg(nullptr, avatar->GetKey(), SOkey);
                    swMsg->Send();
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitSubWorld
//  PARAMETERS : (none)
//
//  PURPOSE    : Exit the avatar from the subworld, back into the ... <whatever> world
//
void cyAvatar::ExitSubWorld()
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            // we're going to the main (null) subworld
            plSubWorldMsg *swMsg = new plSubWorldMsg(nullptr, avatar->GetKey(), nullptr);
            swMsg->Send();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PlaySimpleAnimation
//  PARAMETERS : object  - a sceneobject that is in the subworld
//
//  PURPOSE    : Place the Avatar into the subworld of the sceneobject specified
//
void cyAvatar::PlaySimpleAnimation(const ST::string& animName)
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            avatar->PlaySimpleAnim(animName);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SaveClothingToFile
//  PARAMETERS : filename - file to save to
//
//  PURPOSE    : Save the avatar's clothing to a file. If only a filename is
//               given, it will write to UserData/Avatars.
//
bool cyAvatar::SaveClothingToFile(plFileName filename)
{
    if (!fRecvr.empty()) {
        plArmatureMod* avatar = plAvatarMgr::FindAvatar(fRecvr[0]);
        if (avatar) {
            plClothingOutfit* cl = avatar->GetClothingOutfit();
            if (cl) {
                // Save file in UserData/Avatars if only a filename is given
                if (!filename.StripFileName().IsValid()) {
                    plFileName path = plFileName::Join(plFileSystem::GetUserDataPath(), "Avatars");
                    plFileSystem::CreateDir(path, true);
                    filename = plFileName::Join(path, filename);
                }
                return cl->WriteToFile(filename);
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : LoadClothingFromFile
//  PARAMETERS : filename - file to load from
//
//  PURPOSE    : Load the avatar's clothing from a file. If only a filename is
//               given, it will read from UserData/Avatars.
//
bool cyAvatar::LoadClothingFromFile(plFileName filename)
{
    if (!fRecvr.empty()) {
        plArmatureMod* avatar = plAvatarMgr::FindAvatar(fRecvr[0]);
        if (avatar) {
            plClothingOutfit* cl = avatar->GetClothingOutfit();
            if (cl) {
                // Search for file in UserData/Avatars if only a filename is given
                if (!filename.StripFileName().IsValid())
                    filename = plFileName::Join(plFileSystem::GetUserDataPath(), "Avatars", filename);
                cl->SetClothingFile(filename);
                return cl->ReadClothing();
            }
        }
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : FindBone
//  PARAMETERS : bonename - the name of the bone to find
//
//  PURPOSE    : Returns a reference to the scene object for the bone with the
//               given name.
//
PyObject* cyAvatar::FindBone(const ST::string& boneName)
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty()) {
        // find the armature modifier
        const plArmatureMod* avatar = IFindArmatureMod(fRecvr[0]);
        if (avatar) {
            const plSceneObject* bone = avatar->FindBone(boneName);
            if (bone) {
                return pySceneObject::New(bone->GetKey());
            }
        }
    }

    ST::string errmsg = ST::format("Bone {} not found", boneName);
    PyErr_SetString(PyExc_NameError, errmsg.c_str());
    return nullptr; // return nullptr cause we errored
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ChangeAvatar
//  PARAMETERS : gender name  - is a string of the name of the gender to go to
//
//  PURPOSE    : Change the local avatar's gender.
//
//  Valid genders:
//    Male
//    Female
//
void cyAvatar::ChangeAvatar(const ST::string& genderName)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    plClothingMgr::ChangeAvatar(genderName);

    hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode();
    if (rvnPlr) {
        VaultPlayerNode plr(rvnPlr);
        plr.SetAvatarShapeName(genderName);
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ChangePlayerName
//  PARAMETERS : name  - is a string of the new name for the player
//
//  PURPOSE    : Change the local player's avatar name
//
void cyAvatar::ChangePlayerName(const ST::string& playerName)
{
    hsRef<RelVaultNode> rvnPlr = VaultGetPlayerNode();
    if (rvnPlr) {
        VaultPlayerNode plr(rvnPlr);
        plr.SetPlayerName(playerName);
    } 
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Emote
//  PARAMETERS : emoteName - name of the emote to play on the avatar
//
//  PURPOSE    : plays an emote on a the local avatar (net propagated)
//
bool cyAvatar::Emote(const ST::string& emoteName)
{
    // can we find an emote of this name?
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

    return AvatarEmote(avatar, emoteName);
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Sit
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar sit down on the ground where they are.
//               The avatar will automatically stand when the user tries to move.
//
bool cyAvatar::Sit()
{
    return IEnterGenericMode(ST_LITERAL("SitDownGround"), ST_LITERAL("SitIdleGround"), ST_LITERAL("SitStandGround"), true, plAGAnim::kBodyLower, plAvBrainGeneric::kSitOnGround);
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterKiMode
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar appear to be using the ki.
//
bool cyAvatar::EnterKiMode()
{
    return IEnterGenericMode(ST_LITERAL("KiBegin"), ST_LITERAL("KiUse"), ST_LITERAL("KiEnd"), false, plAGAnim::kBodyFull);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitKiMode
//  PARAMETERS : none
//
//  PURPOSE    : Makes the avatar stop appearing to use the ki.
//               May cause problems if EnterKiMode() was not called earlier.
//
bool cyAvatar::ExitKiMode()
{
    return IExitTopmostGenericMode();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterAFKMode
//  PARAMETERS : none
//
//  PURPOSE    : Tell the avatar to enter the AFK mode (sitting, head down)
//
bool cyAvatar::EnterAFKMode()
{
    return IEnterGenericMode(ST_LITERAL("AFKEnter"), ST_LITERAL("AFKIdle"), ST_LITERAL("AFKExit"), true, plAGAnim::kBodyFull, plAvBrainGeneric::kAFK);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitAFKMode
//  PARAMETERS : none
//
//  PURPOSE    : Tell the avatar to exit the AFK mode
//               May cause problems if EnterKiMode() was not called earlier.
//
bool cyAvatar::ExitAFKMode()
{
    return IExitTopmostGenericMode();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterPBMode
//  PARAMETERS : none
//
//  PURPOSE    : Enter the personal book mode...stay until further notice.
//
bool cyAvatar::EnterPBMode()
{
    return IEnterGenericMode(ST_LITERAL("PersonalBookEnter"), ST_LITERAL("PersonalBookIdle"), ST_LITERAL("PersonalBookExit"), false, plAGAnim::kBodyFull);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ExitPBMode
//  PARAMETERS : none
//
//  PURPOSE    : Leave the personal book mode. Currently leaves any mode; will become
//             : more specific in future version
//
bool cyAvatar::ExitPBMode()
{
    return IExitTopmostGenericMode();
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : EnterAnimMode
//  PARAMETERS : animName - string
//
//  PURPOSE    : Makes the avatar enter a custom anim loop.
//
bool cyAvatar::EnterAnimMode(const ST::string& animName)
{
    plArmatureMod* fAvMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    return PushRepeatEmote(fAvMod, animName);
}


int cyAvatar::GetCurrentMode()
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            return avatar->GetCurrentGenericType();
        }
    }
    return plAvBrainGeneric::kNonGeneric;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : disable movement controls
//  PARAMETERS : 
//
//  PURPOSE    : something tells me python shouldn't do this this way
//


void cyAvatar::DisableMovementControls()
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if (avatar)
    {   
        if (!avatar->IsInputSuspended())
            avatar->SuspendInput();
    }
}

void cyAvatar::EnableMovementControls()
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();   
    if (avatar)
    {
        if (avatar->IsInputSuspended())
            avatar->ResumeInput();
    }
}

void cyAvatar::DisableMouseMovement()
{
    plAvatarInputInterface::GetInstance()->SuspendMouseMovement();
}

void cyAvatar::EnableMouseMovement()
{
    plAvatarInputInterface::GetInstance()->EnableMouseMovement();
}

void cyAvatar::EnableAvatarJump()
{
    plAvatarInputInterface::GetInstance()->EnableJump(true);
}

void cyAvatar::DisableAvatarJump()
{
    plAvatarInputInterface::GetInstance()->EnableJump(false);
}

void cyAvatar::EnableForwardMovement()
{
    plAvatarInputInterface::GetInstance()->EnableForwardMovement(true);
}

void cyAvatar::DisableForwardMovement()
{
    plAvatarInputInterface::GetInstance()->EnableForwardMovement(false);
}

bool cyAvatar::LocalAvatarRunKeyDown()
{
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if (avMod)
        return avMod->FastKeyDown();
    return false;
}

bool cyAvatar::LocalAvatarIsMoving()
{
    plArmatureMod *avMod = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if (avMod)
        return avMod->ForwardKeyDown() || avMod->BackwardKeyDown() || avMod->StrafeRightKeyDown() ||
            avMod->StrafeLeftKeyDown() || avMod->TurnRightKeyDown() || avMod->TurnLeftKeyDown() ||
            avMod->JumpKeyDown();
    return false;
}

void cyAvatar::SetMouseTurnSensitivity(float val)
{
    plArmatureMod::SetMouseTurnSensitivity(val);
}

float cyAvatar::GetMouseTurnSensitivity()
{
    return plArmatureMod::GetMouseTurnSensitivity();
}

void cyAvatar::SpawnNext()
{
    static int whichSpawn = 0;
    plAvatarMgr *mgr = plAvatarMgr::GetInstance();
    int max = mgr->NumSpawnPoints();

    whichSpawn = ++whichSpawn < max ? whichSpawn : 0;
    
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if(avatar)
    {
        double fakeTime = 0.0f;
        avatar->SpawnAt(whichSpawn, fakeTime);
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : RegisterForBehaviorNotify()
//  PARAMETERS : none
//
//  PURPOSE    : To register for notifies from the avatar for any kind of behavior notify
//
void cyAvatar::RegisterForBehaviorNotify(pyKey &selfKey)
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            avatar->RegisterForBehaviorNotify(selfKey.getKey());
        }
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : UnRegisterForBehaviorNotify()
//  PARAMETERS : none
//
//  PURPOSE    : To remove the registeration for notifies from the avatar
//
void cyAvatar::UnRegisterForBehaviorNotify(pyKey &selfKey)
{
    // make sure that there is atleast one avatar scene object attached (should be)
    if (!fRecvr.empty())
    {
        // find the armature modifier
        plArmatureMod* avatar = (plArmatureMod*)IFindArmatureMod((plKey)fRecvr[0]);
        if(avatar)
        {
            avatar->UnRegisterForBehaviorNotify(selfKey.getKey());
        }
    }
}






/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IEnterGenericMode
//  PARAMETERS : none
//
//  PURPOSE    : Three-stage multistage animations (sit down, sit, get up) are really common.
//             : This does the basic setup.
//
bool IEnterGenericMode(const ST::string& enterAnim, const ST::string& idleAnim, const ST::string& exitAnim, bool autoExit, plAGAnim::BodyUsage bodyUsage, 
                       plAvBrainGeneric::BrainType type /* = kGeneric */)
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    return PushSimpleMultiStage(avatar, enterAnim, idleAnim, exitAnim, true, autoExit, bodyUsage, type);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IExitTopmostGenericMode
//  PARAMETERS : none
//
//  PURPOSE    : Exits whatever multistage animation you're in. We currently don't discriminate;
//             : that will be added later.
//
bool IExitTopmostGenericMode()
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();

    plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg(nullptr, avatar->GetKey(),
        plAvBrainGenericMsg::kGotoStage, 2, false, 0.0,
        false, false, 0.0);

    pMsg->SetBCastFlag(plMessage::kNetForce | plMessage::kNetPropagate);

    plgDispatch::MsgSend( pMsg );
    return true;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : IsCurrentBrainHuman
//  PARAMETERS : none
//
//  PURPOSE    : Returns whether the top most brain is a human brain
//
bool cyAvatar::IsCurrentBrainHuman()
{
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    if (avatar)
    {
        plArmatureBrain *brain = avatar->GetCurrentBrain();
        plAvBrainHuman *human = plAvBrainHuman::ConvertNoRef(brain);
        if (human)
            return true;
    }
    return false;
}

void cyAvatar::SetDontPanicLink(bool value)
{
    if (!fRecvr.empty()) {
        plArmatureMod* mod = plAvatarMgr::FindAvatar(fRecvr[0]);
        if (mod)
            mod->SetDontPanicLinkFlag(value);
    }
}
