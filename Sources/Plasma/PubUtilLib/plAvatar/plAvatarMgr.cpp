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
#include "plAvatarMgr.h"

// local
#include "plAnimStage.h"
#include "plArmatureMod.h"
#include "plAvBrainCoop.h"
#include "plCoopCoordinator.h"
#include "plOneShotMod.h"
#include "plSeekPointMod.h"

// global
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsTimer.h"

// other
#include "pnEncryption/plRandom.h"
#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plKey.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnMessage/plWarpMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAnimation/plAGModifier.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plAvCoopMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plLoadClothingMsg.h"
#include "plMessage/plMemberUpdateMsg.h"
#include "plModifier/plMaintainersMarkerModifier.h"
#include "plModifier/plSpawnModifier.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetTransport/plNetTransport.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plResMgr/plKeyFinder.h"
#include "plStatusLog/plStatusLog.h"
#include "plVault/plDniCoordinateInfo.h"

#include "pfCCR/plCCRMgr.h" // Only included for defined constants. 

#include <algorithm>
#include <cmath>

// The static single instance, allocated on demand by GetInstance()
plAvatarMgr     *plAvatarMgr::fInstance = nullptr;

// CTOR
plAvatarMgr::plAvatarMgr()
{
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Avatar.log", plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kTimestamp);
    fLog->AddLine("Initalized avatar mgr"); 
}

// DTOR
plAvatarMgr::~plAvatarMgr()
{
    IReset();

    delete fLog;
    fLog = nullptr;
}

// GETINSTANCE
plAvatarMgr * plAvatarMgr::GetInstance()
{
    if(!fInstance)
    {
        fInstance = new plAvatarMgr;
        fInstance->RegisterAs(kAvatarMgr_KEY);
        fInstance->Ref();
    }
    return fInstance;
}

// SHUTDOWN
void plAvatarMgr::ShutDown()
{
    if(fInstance)
    {
        fInstance->UnRef();
        if(fInstance)
            fInstance->UnRegister();
        fInstance = nullptr;
    }
}

// RESET
void plAvatarMgr::IReset()
{
    fSeekPoints.clear();

    fOneShots.clear();
    fAvatars.clear();
    fSpawnPoints.clear();
    fMaintainersMarkers.clear();

    plCoopMap::iterator acIt = fActiveCoops.begin();
    while (acIt != fActiveCoops.end())
    {
        plCoopCoordinator* deadCoop = acIt->second;
        delete deadCoop;
        acIt++;
    }
    fActiveCoops.clear();
}

plKey plAvatarMgr::LoadPlayer(const ST::string &name, const ST::string &account)
{
    return LoadAvatar(name, account, true, nullptr, nullptr);
}

plKey plAvatarMgr::LoadPlayer(const ST::string &name, const ST::string &account, const ST::string &linkInName)
{
    // what we'd like to do is turn the linkInName into a spawn point key and
    // put that into the plLoadAvatarMsg, which is already set up to handle
    // initial spawn points.
    // however, that will require that we can handle waiting for our spawn point to load,
    // so we're goin to do this the "old way" for now.
    
    plArmatureMod::SetSpawnPointOverride(linkInName);
    return LoadAvatar(name, account, true, nullptr, nullptr);
}

plKey plAvatarMgr::LoadPlayerFromFile(const ST::string &name, const ST::string &account, const plFileName &clothingFile)
{
    return LoadAvatar(name, account, true, nullptr, nullptr, "", clothingFile);
}

plKey plAvatarMgr::LoadAvatar(ST::string name, const ST::string &accountName, bool isPlayer, const plKey& spawnPoint, plAvTask *initialTask,
                              const ST::string &userStr, const plFileName &clothingFile)
{
    // *** account is currently unused. the idea is that eventually an NPC will
    // *** be able to use a customization account
    plKey result = nullptr;
    plKey requestor = GetKey(); // avatar manager is always the requestor for avatar loads
    plNetClientApp *netMgr = plNetClientApp::GetInstance();

    if(netMgr)      // can't clone without the net manager
    {
        hsAssert(!name.empty(), "name required by LoadPlayer fxn");
        netMgr->DebugMsg("Local: Loading player {}", name);

        // look up player by key name provided by user.
        // this string search should be replaced with some other method of 
        // avatar selection and key lookup.

        // Get the location for the player first
        plKey playerKey = nullptr;
        const plLocation& globalLoc = plKeyFinder::Instance().FindLocation("GlobalAvatars", name);
        const plLocation& maleLoc = plKeyFinder::Instance().FindLocation("GlobalAvatars", "Male");
        const plLocation& custLoc = plKeyFinder::Instance().FindLocation("CustomAvatars", name);

#ifdef PLASMA_EXTERNAL_RELEASE
        // Try global. If that doesn't work, players default to male.
        // If not a player, try custLoc. If that doesn't work, fall back to male
        const plLocation& loc = (globalLoc.IsValid() ? globalLoc : isPlayer ? maleLoc : custLoc.IsValid() ? custLoc : maleLoc);
#else
        // Try global. If that doesn't work try custom. Otherwise fall back to male
        const plLocation& loc = (globalLoc.IsValid() ? globalLoc : custLoc.IsValid() ? custLoc : maleLoc);
#endif

        if (loc == maleLoc)
            name = "Male";

        if (loc.IsValid())
        {
            plUoid uID(loc, plSceneObject::Index(), name);
            plLoadAvatarMsg *cloneMsg = new plLoadAvatarMsg(uID, requestor, 0, isPlayer, spawnPoint, initialTask, userStr);
            if (clothingFile.IsValid())
            {
                plLoadClothingMsg *clothingMsg = new plLoadClothingMsg(clothingFile);
                cloneMsg->SetTriggerMsg(clothingMsg);
            }
            result =  cloneMsg->GetCloneKey();
            
            // the clone message is automatically addressed to the net client manager
            // we'll receive the message back (or a similar message) when the clone is loaded
            cloneMsg->Send();
        }
    }
    return result;
}

void plAvatarMgr::UnLoadAvatar(const plKey& avatarKey, bool isPlayer, bool netPropagate) const
{
    if (avatarKey)
    {
        plKey requestor = GetKey();
        plLoadAvatarMsg* msg = new plLoadAvatarMsg(avatarKey, requestor, 0, isPlayer, false);

        // only netprop if the user has a death wish
        msg->SetBCastFlag(plMessage::kNetPropagate, netPropagate);
        msg->Send();
    }
}

// our player's already loaded locally, but we've just linked into an age and others there need to be
// told about us
void plAvatarMgr::PropagateLocalPlayer(int spawnPoint)
{
    plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    if(playerKey)
    {
        plKey requestor = GetKey();
        bool isPlayer = true;
        bool isLoading = true;
        plLoadAvatarMsg *msg = new plLoadAvatarMsg(playerKey, requestor, 0, isPlayer, isLoading);

        if (spawnPoint >= 0)
        {
            const plSpawnModifier * spawn = GetSpawnPoint(spawnPoint);
            if ( spawn )
            {
                const plSceneObject * spawnObj = spawn->GetTarget(0);
                msg->SetSpawnPoint(spawnObj->GetKey());
            }
        }

        // don't propagate locally. this is only for our peers
        msg->SetBCastFlag(plMessage::kLocalPropagate, false);
        msg->Send();
    } else {
        hsStatusMessage("Tried to propagate non-existent local player.");
    }
}

// UNLOADLOCALPLAYERREMOTELY
bool plAvatarMgr::UnPropagateLocalPlayer()
{
    plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    if(playerKey)
    {
        plKey requestor = GetKey();
        bool isPlayer = true;
        bool isLoading = false;
        plLoadAvatarMsg *msg = new plLoadAvatarMsg(playerKey, requestor, 0, isPlayer, isLoading);
        msg->SetBCastFlag(plMessage::kLocalPropagate, false);
        msg->Send();
        return true;
    }
    return false;
}

// UNLOADLOCALPLAYER
void plAvatarMgr::UnLoadLocalPlayer()
{
    plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
    if(playerKey)
    {
        plKey mgrKey = GetKey();
        bool isPlayer = true;
        bool isLoading = false;
        plLoadAvatarMsg *msg = new plLoadAvatarMsg(playerKey, mgrKey, 0, isPlayer, isLoading);
        msg->Send();
    }
}

// MSGRECEIVE
bool plAvatarMgr::MsgReceive(plMessage *msg)
{
    plLoadAvatarMsg *cloneM = plLoadAvatarMsg::ConvertNoRef(msg);
    if(cloneM)
    {
        // The only way we get clone messages is if we (or our remote counterparts)
        // requested them.
        if(cloneM->GetIsLoading())
        {
            IFinishLoadingAvatar(cloneM);
        } else {
            IFinishUnloadingAvatar(cloneM);
        }
        return true;
    }

    plLoadCloneMsg* pCloneMsg = plLoadCloneMsg::ConvertNoRef(msg);
    if (pCloneMsg)
    {
        pCloneMsg->Ref();
        fCloneMsgQueue.emplace_back(pCloneMsg);
        plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
        return true;
    }

    plEvalMsg* pEval = plEvalMsg::ConvertNoRef(msg);
    if (pEval)
    {
        for (hsSsize_t i = fCloneMsgQueue.size() - 1; i >= 0; i--)
        {
            plArmatureMod* pAvatar = FindAvatarByPlayerID(fCloneMsgQueue[i]->GetUserData());
            if (pAvatar && pAvatar->GetKey()->ObjectIsLoaded())
            {   
                pAvatar->MsgReceive(fCloneMsgQueue[i]);
                fCloneMsgQueue[i]->UnRef();
                fCloneMsgQueue.erase(fCloneMsgQueue.begin() + i);
            }
        }
        if (fCloneMsgQueue.empty())
            plgDispatch::Dispatch()->UnRegisterForExactType(plEvalMsg::Index(), GetKey());
        
        return true;
    }
    plAvCoopMsg *coopM = plAvCoopMsg::ConvertNoRef(msg);
    if(coopM)
    {
        return HandleCoopMsg(coopM);
    }

    plNotifyMsg *notifyM = plNotifyMsg::ConvertNoRef(msg);
    if(notifyM)
    {
        if(proEventData * evt = notifyM->FindEventRecord(proEventData::kCoop))
        {
            proCoopEventData *coopE = static_cast<proCoopEventData *>(evt);
            return IPassMessageToActiveCoop(msg, coopE->fID, coopE->fSerial);
        }
    }

    return false;
}

bool plAvatarMgr::HandleCoopMsg(plAvCoopMsg *msg)
{
    plAvCoopMsg::Command cmd = msg->fCommand;
    
    uint32_t id = msg->fInitiatorID;
    uint16_t serial = msg->fInitiatorSerial;

    if(cmd == plAvCoopMsg::kStartNew)
    {
        // Currently, there's nothing that removes these coop coordinators when
        // they're done.  Since I can't think of a good way to figure out when
        // they're done, I'm just going to clear them every time a new one starts.
        // With the current usage, you should only get one at a time anyway -Colin
        plCoopMap::iterator it = fActiveCoops.begin();
        while (it != fActiveCoops.end())
        {
            plCoopCoordinator* deadCoop = it->second;
            delete deadCoop;
            it++;
        }
        fActiveCoops.clear();

        // start a new coop
        plCoopCoordinator *coord = msg->fCoordinator;
        plCoopMap::value_type newVal(id, coord);
        fActiveCoops.insert(newVal);
        coord->Run();
        return true;
    } else {
        // it's a message for an existing coop...
        return IPassMessageToActiveCoop(msg, id, serial);
    }
}

bool plAvatarMgr::HandleNotifyMsg(plNotifyMsg *msg)
{
    proCoopEventData *ed = static_cast<proCoopEventData *>(msg->FindEventRecord(proEventData::kCoop));
    if(ed)
    {
        uint32_t id = ed->fID;
        uint16_t serial = ed->fSerial;
        return IPassMessageToActiveCoop(msg, id, serial);
    }
    return false;
}

bool plAvatarMgr::IPassMessageToActiveCoop(plMessage *msg, uint32_t id, uint16_t serial)
{
    plCoopMap::iterator i = fActiveCoops.find(id);
    while(i != fActiveCoops.end() && (*i).first == id)
    {
        plCoopCoordinator *coord = (*i).second;
        if(coord->GetInitiatorSerial() == serial && coord->IsActiveForReal() )
        {
            // this is the one
            coord->MsgReceive(msg);
            return true;
        }
        i++;
    }
    return false;
}

bool plAvatarMgr::IsACoopRunning()
{
    bool isRunning = false;
    plCoopMap::iterator it = fActiveCoops.begin();
    while (it != fActiveCoops.end())
    {
        plCoopCoordinator* aCoop = it->second;
        if (aCoop->IsActiveForReal())
            isRunning = true;
        it++;
    }
    return isRunning;
}


void plAvatarMgr::IFinishLoadingAvatar(plLoadAvatarMsg *cloneMsg)
{
    plKey avatarKey = cloneMsg->GetCloneKey();
    plUoid playerUoid = avatarKey->GetUoid();
    const plArmatureMod *armature = FindAvatar(avatarKey);

    // we're going to re-send the clone message to the loaded avatar so he can get
    // any necessary details from it.
    cloneMsg->ClearReceivers();     // don't want it coming back to us
    cloneMsg->Ref();                // or going away

    if(armature)
    {
        cloneMsg->AddReceiver(armature->GetKey());
        cloneMsg->Send();
    } else {
        IDeferInit(avatarKey, cloneMsg);        // we'll send this message when the armature mod loads.
    }

    if( cloneMsg->GetIsPlayer() )
    {
        // notify everyone who cares that a new player has arrived
        // *** might want to move this to the human brain so we can make sure the 
        // *** avatar is sufficiently initialized before anyone accesses him
        bool isLocal = cloneMsg->GetOriginatingPlayerID() == plNetClientMgr::GetInstance()->GetPlayerID();
        plPlayerPageMsg* pageM = new plPlayerPageMsg;
        pageM->SetBCastFlag(plMessage::kBCastByExactType);
        pageM->fLocallyOriginated = isLocal;
        pageM->fPlayer = avatarKey;
        pageM->fUnload = false;
        pageM->fClientID = cloneMsg->GetOriginatingPlayerID();
        pageM->Send();
    }

    // This can probably be replaced by the plPlayerPageMsg:
    // ...keeping for the moment for compatibility
    plMemberUpdateMsg* mu = new plMemberUpdateMsg;
    mu->Send();
}

// IFINISHUNLOADINGAVATAR
void plAvatarMgr::IFinishUnloadingAvatar(plLoadAvatarMsg *cloneMsg)
{
    // Note: in the corresponding FinishLoading, above, we give the incoming avatar
    // a look at the message that spawned him. When unloading, however, he doesn't get
    // that benefit because I don't think he'll actually be around to receive it.
    // *** need to test that theory....but it's not a problem for now.
    if( cloneMsg->GetIsPlayer() )
    {
        plKey avatar = cloneMsg->GetCloneKey();

        bool isLocal = cloneMsg->GetOriginatingPlayerID() == plNetClientMgr::GetInstance()->GetPlayerID();
        plPlayerPageMsg *pageM = new plPlayerPageMsg;
        pageM->SetBCastFlag(plMessage::kBCastByExactType);
        pageM->fLocallyOriginated = isLocal;
        pageM->fPlayer = avatar;
        pageM->fUnload = true;
        pageM->fClientID = cloneMsg->GetOriginatingPlayerID();
        if (plNetClientMgr::GetInstance()->RemotePlayerKeys().size() == 0)
            pageM->fLastOut = true;
        pageM->Send();
    }

    // check on this...can it be subsumed by plPlayerPageMsg ?
    plMemberUpdateMsg *mu = new plMemberUpdateMsg;
    mu->Send();
}


// IDEFERINIT
void plAvatarMgr::IDeferInit(const plKey& playerSOKey, plMessage *initMsg)
{
    plMessage *existing = fDeferredInits[playerSOKey];      // okay to use this form because we're going
                                                            // to do the add either way
    if(existing)
    {
        hsStatusMessage("Avatar was registered twice for init. Discarding initial init message.");
        existing->UnRef();
    }

    fDeferredInits[playerSOKey] = initMsg;
    initMsg->Ref();
}

// ISENDDEFERREDINIT
void plAvatarMgr::ISendDeferredInit(const plKey& avatarSOKey)
{
    // get armaturemod
    const plArmatureMod * armature = FindAvatar(avatarSOKey);

    if(armature)
    {
        DeferredInits::iterator i = fDeferredInits.find(avatarSOKey);
        if(i != fDeferredInits.end())
        {
            plMessage * initMsg = (*i).second;
            hsAssert(initMsg, "Tried to init avatar, but found nil initialization message.");
            
            if(initMsg)
            {
                initMsg->AddReceiver(armature->GetKey());
                initMsg->Send();
            }
        }
    }
}


// ADDSEEKPOINT
void plAvatarMgr::AddSeekPoint(plSeekPointMod *seekPoint)
{
    if(seekPoint)
    {
        ST::string name = seekPoint->GetTarget(0)->GetKey()->GetName();
        plSeekPointMod *alreadyThere = FindSeekPoint(name);

        /// hsAssert( ! alreadyThere, "Tried to add a seek point with duplicate name. Ignoring second seek point.");

        if ( ! alreadyThere)
        {
            fSeekPoints[name] = seekPoint;
        }
    }
}

// REMOVESEEKPOINT
void plAvatarMgr::RemoveSeekPoint(plSeekPointMod *seekPoint)
{
    if(seekPoint)
    {
        ST::string name = seekPoint->GetTarget(0)->GetKey()->GetName();

        plSeekPointMap::iterator found = fSeekPoints.find(name);

        if(found != fSeekPoints.end())
        {
            fSeekPoints.erase(found);
        }
    }
}

// FINDSEEKPOINT
plSeekPointMod * plAvatarMgr::FindSeekPoint(const ST::string &name)
{
    plSeekPointMap::iterator found = fSeekPoints.find(name);
    
    if (found == fSeekPoints.end())
    {
        return nullptr;
    } else {
        return (*found).second;
    }
}

// ADDONESHOT
void plAvatarMgr::AddOneShot(plOneShotMod *oneshot)
{
    if(oneshot)
    {
        ST::string name = oneshot->GetTarget(0)->GetKey()->GetName();
        plOneShotMod *alreadyThere = FindOneShot(name);


        if ( ! alreadyThere)
        {
            fOneShots[name] = oneshot;
        }
    }
}

// REMOVEONESHOT
void plAvatarMgr::RemoveOneShot(plOneShotMod *oneshot)
{
    plOneShotMap::iterator i = fOneShots.begin();

    while (i != fOneShots.end())
    {
        ST::string name = i->first;
        plOneShotMod *thisOneshot = i->second;

        if(oneshot == thisOneshot)
        {
            fOneShots.erase(i++);
        } else {
            ++i;
        }
    }
}

// FINDONESHOT
plOneShotMod *plAvatarMgr::FindOneShot(const ST::string &name)
{
    plOneShotMap::iterator found = fOneShots.find(name);

    if (found == fOneShots.end())
    {
        return nullptr;
    } else {
        return found->second;
    }
}

// ADDAVATAR
void plAvatarMgr::AddAvatar(plArmatureMod *avatar)
{
    // we shouldn't really need to ref this, as every time we access this object we will be checking it, and we don't care too much if it gets
    // pulled out from under us
    fAvatars.push_back(avatar->GetKey());
    plSceneObject *avatarSO = avatar->GetTarget(0);
    hsAssert(avatarSO, "Adding avatar, but it hasn't been attached to a scene object yet.");
    if(avatarSO)
    {
        ISendDeferredInit(avatarSO->GetKey());
    }
}

// REMOVEAVATAR
void plAvatarMgr::RemoveAvatar(plArmatureMod *avatar)
{
    if (avatar)
    {
        plAvatarVec::iterator tail = std::remove(fAvatars.begin(), fAvatars.end(), avatar->GetKey());
        if(tail != fAvatars.end())
            fAvatars.erase(tail);
    }
}

plArmatureMod* plAvatarMgr::GetLocalAvatar()
{
    plNetClientApp * app = plNetClientApp::GetInstance();
    if(app)
    {
        plKey key = app->GetLocalPlayerKey();
        if (key && key->ObjectIsLoaded())
        {
            plSceneObject* so = plSceneObject::ConvertNoRef(key->GetObjectPtr());
            if (so)
                return const_cast<plArmatureMod*>((plArmatureMod*)so->GetModifierByType(plArmatureMod::Index()));
        }
    }

    return nullptr;
}

plKey plAvatarMgr::GetLocalAvatarKey()
{
    plArmatureMod *avatar = GetLocalAvatar();
    if (avatar)
        return avatar->GetKey();

    return nullptr;
}

plArmatureMod *plAvatarMgr::GetFirstRemoteAvatar()
{
    plNetClientApp * app = plNetClientApp::GetInstance();
    if(app)
    {
        plArmatureMod *localAvatar = GetLocalAvatar();
        
        plAvatarVec::iterator it;
        for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
        {
            plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
            if(armature && (armature != localAvatar))
                return armature;
        }
    }
    
    return nullptr;
}

plArmatureMod* plAvatarMgr::FindAvatar(const plKey& avatarKey)
{
    plSceneObject *so = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
    if (so)
        return const_cast<plArmatureMod*>((plArmatureMod*)so->GetModifierByType(plArmatureMod::Index()));
    
    return nullptr;
}

plArmatureMod* plAvatarMgr::FindAvatarByPlayerID(uint32_t pid)
{
    plAvatarVec::iterator it;
    for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
    {
        plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
        if (armature && (armature->GetKey()->GetUoid().GetClonePlayerID() == pid))
            return armature;
    }
    return nullptr;
}

plArmatureMod *plAvatarMgr::FindAvatarByModelName(const ST::string& name)
{
    plAvatarVec::iterator it;
    for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
    {
        plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
        if (armature && (!armature->GetTarget(0)->GetKeyName().compare(name)))
            return armature;
    }
    
    return nullptr;
}

void plAvatarMgr::FindAllAvatarsByModelName(const char* name, plArmatureModPtrVec& outVec)
{
    plAvatarVec::iterator it;
    for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
    {
        plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
        if (armature && (!armature->GetTarget(0)->GetKeyName().compare(name)))
            outVec.push_back(armature);
    }
}

// ADDSPAWNPOINT
void plAvatarMgr::AddSpawnPoint(plSpawnModifier *spawn)
{
    fSpawnPoints.push_back(spawn);
}

// REMOVESPAWNPOINT
void plAvatarMgr::RemoveSpawnPoint(plSpawnModifier *spawn)
{
    plSpawnVec::iterator found = std::find(fSpawnPoints.begin(), fSpawnPoints.end(), spawn);

    if(found != fSpawnPoints.end())
    {
        fSpawnPoints.erase(found);
    }
}

// GETSPAWNPOINT
const plSpawnModifier * plAvatarMgr::GetSpawnPoint(int i)
{
    if(i < fSpawnPoints.size())
    {
        return fSpawnPoints[i];
    }
    else
    {
        return nullptr;
    }
}

int plAvatarMgr::FindSpawnPoint( const char *name ) const
{
    int i;

    for( i = 0; i < fSpawnPoints.size(); i++ )
    {
        if (fSpawnPoints[i] != nullptr &&
            ( fSpawnPoints[ i ]->GetKey()->GetUoid().GetObjectName().contains( name ) ||
              fSpawnPoints[ i ]->GetTarget(0)->GetKeyName().contains( name ) ))
            return i;
    }

    return -1;
}

int plAvatarMgr::WarpPlayerToAnother(bool iMove, uint32_t remoteID)
{
    plNetTransport &mgr = plNetClientMgr::GetInstance()->TransportMgr();
    plNetTransportMember* mbr = mgr.GetMemberByID(remoteID);

    if (!mbr)
        return plCCRError::kCantFindPlayer;
    
    if (!mbr->GetAvatarKey())
        return plCCRError::kPlayerNotInAge;

    plSceneObject *remoteSO = plSceneObject::ConvertNoRef(mbr->GetAvatarKey()->ObjectIsLoaded());
    plSceneObject *localSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());

    if (!remoteSO)
        return plCCRError::kCantFindPlayer;
    if (!localSO)
        return plCCRError::kNilLocalAvatar;

    plWarpMsg *warp = new plWarpMsg(nullptr, (iMove ? localSO->GetKey() : remoteSO->GetKey()),
        plWarpMsg::kFlushTransform, (iMove ? remoteSO->GetLocalToWorld() : localSO->GetLocalToWorld()));
    
    warp->SetBCastFlag(plMessage::kNetPropagate);
    plgDispatch::MsgSend(warp);

    return hsOK;
}

int plAvatarMgr::WarpPlayerToXYZ(float x, float y, float z)
{
    plSceneObject *localSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
    if (!localSO)
        return plCCRError::kNilLocalAvatar;

    hsMatrix44 m = localSO->GetLocalToWorld();
    hsVector3 v(x, y, z);
    m.SetTranslate(&v);

    plWarpMsg *warp = new plWarpMsg(nullptr, localSO->GetKey(), plWarpMsg::kFlushTransform, m);
    warp->SetBCastFlag(plMessage::kNetPropagate);
    plgDispatch::MsgSend(warp);

    return hsOK;
}

int plAvatarMgr::WarpPlayerToXYZ(int pid, float x, float y, float z)
{
    plNetClientMgr* nc=plNetClientMgr::GetInstance();
    plNetTransportMember* mbr = nc->TransportMgr().GetMemberByID(pid);
    plSceneObject *player = plSceneObject::ConvertNoRef(mbr && mbr->GetAvatarKey() ? 
        mbr->GetAvatarKey()->ObjectIsLoaded() : nullptr);
    if (!player)
        return plCCRError::kNilLocalAvatar;

    hsMatrix44 m = player->GetLocalToWorld();
    hsVector3 v(x, y, z);
    m.SetTranslate(&v);

    plWarpMsg *warp = new plWarpMsg(nullptr, player->GetKey(), 0, m);
    warp->SetBCastFlag(plMessage::kNetPropagate);
    plgDispatch::MsgSend(warp);

    return hsOK;
}

// ADD maintainers marker
void plAvatarMgr::AddMaintainersMarker(plMaintainersMarkerModifier *mm)
{
    fMaintainersMarkers.emplace_back(mm);
}

// REMOVE maintainers marker
void plAvatarMgr::RemoveMaintainersMarker(plMaintainersMarkerModifier *mm)
{
    for (auto iter = fMaintainersMarkers.begin(); iter != fMaintainersMarkers.end(); )
    {
        if (*iter == mm)
            iter = fMaintainersMarkers.erase(iter);
        else
            ++iter;
    }
}

void plAvatarMgr::PointToDniCoordinate(hsPoint3 pt, plDniCoordinateInfo* ret)
{
    size_t count = fMaintainersMarkers.size();
    //  plDniCoordinateInfo ret = new plDniCoordinateInfo;
    if (count > 0)
    {

        // find the closest maintainers marker
        size_t nearestIndex = 0;
        if (count > 1)
        {
            for (size_t i = 0; i < fMaintainersMarkers.size(); i++)
            {
                if (fMaintainersMarkers[i]->GetTarget(0))
                {
                    hsVector3 testDist(fMaintainersMarkers[i]->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pt);
                    hsVector3 baseDist(fMaintainersMarkers[nearestIndex]->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pt);  
                    if (testDist.MagnitudeSquared() < baseDist.MagnitudeSquared())
                        nearestIndex = i;
                }
            }
        }
        // convert the marker position to Dni coordinates
        int status = fMaintainersMarkers[nearestIndex]->GetCalibrated();

        switch (status)
        {
        case  plMaintainersMarkerModifier::kBroken:
            {
                plRandom rnd;
                rnd.SetSeed((int)(hsTimer::GetSeconds()));
                rnd.RandRangeI(1,999);
                ret->SetHSpans( rnd.RandRangeI(1,999) );
                ret->SetVSpans( rnd.RandRangeI(1,999) );
                ret->SetTorans( rnd.RandRangeI(1,62500) );
            }
            break;
        case plMaintainersMarkerModifier::kRepaired:
            {
                ret->SetHSpans(0);
                ret->SetVSpans(0);
                ret->SetTorans(0);
            }
            break;
        case plMaintainersMarkerModifier::kCalibrated:
            {
                // this is the real deal here:
                // vertical spans:
                hsPoint3 retPoint = fMaintainersMarkers[nearestIndex]->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
                ret->SetVSpans( ((int)(pt.fZ - retPoint.fZ) / 16) );

                // horizontal spans:

                // zero out the z axis...
                retPoint.fZ = pt.fZ = 0.0f;
                hsVector3 hSpanVec(retPoint - pt);
                ret->SetHSpans( (int)hSpanVec.Magnitude() / 16) ;

                // torans
                hsVector3 zeroVec = fMaintainersMarkers[nearestIndex]->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kView);
                hsVector3 zeroRight = fMaintainersMarkers[nearestIndex]->GetTarget(0)->GetCoordinateInterface()->GetLocalToWorld().GetAxis(hsMatrix44::kRight);
                zeroVec *= -1; // match the zero vectors to the positive X & Y axes in 3DSMax
                zeroRight *= -1;
                hsVector3 retVec(pt - retPoint);
                retVec.Normalize();

                float dotView = retVec * zeroVec;
                float dotRight = retVec * zeroRight;

                float deg = hsRadiansToDegrees(acosf(dotView));
                // account for being > 180
                if (dotRight < 0.0f) 
                {
                    deg = 360.f - deg;
                }
                // convert it to dni radians (torans)
                deg *= 173.61f;
                ret->SetTorans((int)deg);
            }
            break;
        }

    }
}

void plAvatarMgr::GetDniCoordinate(plDniCoordinateInfo* ret)
{
    plSceneObject* localSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
    if (localSO)
    {   
        hsPoint3 pos = localSO->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
        PointToDniCoordinate(pos, ret);
    }
}

// OfferLinkingBook ---------------------------------------------------------------
// ----------
void plAvatarMgr::OfferLinkingBook(const plKey& hostKey, const plKey& guestKey, plMessage *linkMsg, const plKey& replyKey)
{
    if (hostKey != nullptr && guestKey != nullptr)
    {
        const plArmatureMod *hostAv = FindAvatar(hostKey);
        const plArmatureMod *guestAv = FindAvatar(guestKey);

        hsAssert(hostAv && guestAv, "Offering linking book: host or guest missing.");

        if(hostAv && guestAv)
        {

            // make the host brain
            plAvBrainCoop * brainH = new plAvBrainCoop(plAvBrainGeneric::kExitNormal, 3.0, 3.0, plAvBrainGeneric::kMoveRelative, guestKey);

            plAnimStage *hostOffer = new plAnimStage("BookOffer", plAnimStage::kNotifyAdvance);     // autoforward, autoadvance
            // repeats until the guest brain tells us that it's done
            plAnimStage *hostIdle = new plAnimStage("BookOfferIdle", plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                                    plAnimStage::kAdvanceNone, plAnimStage::kRegressNone, -1);

            plAnimStage *hostFinish = new plAnimStage("BookOfferFinish", plAnimStage::kNotifyAdvance);  // autoforward, autoadvance
            
            brainH->AddStage(hostOffer);
            brainH->AddStage(hostIdle);
            brainH->AddStage(hostFinish);

            uint32_t hostID = brainH->GetInitiatorID();
            uint32_t hostSerial = brainH->GetInitiatorSerial();


            // make the guest brain
            plAvBrainCoop * brainG = new plAvBrainCoop(plAvBrainGeneric::kExitNormal, 3.0, 3.0, plAvBrainGeneric::kMoveRelative,
                                                       hostID, (uint16_t)hostSerial, hostKey);

            plAnimStage *guestAccept = new plAnimStage("BookAccept", plAnimStage::kNotifyAdvance);
            plAnimStage *guestAcceptIdle = new plAnimStage("BookAcceptIdle", plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
                                                           plAnimStage::kAdvanceNone, plAnimStage::kRegressNone, -1);
            
            brainG->AddStage(guestAccept);
            brainG->AddStage(guestAcceptIdle);
            plCoopCoordinator *coord = new plCoopCoordinator(hostKey, guestKey, brainH, brainG, "Convergence", 1, 1, linkMsg, true);


            plAvCoopMsg *coMg = new plAvCoopMsg(hostKey, coord);
            coMg->SetBCastFlag(plMessage::kNetPropagate);
            coMg->SetBCastFlag(plMessage::kNetForce);

            coMg->Send();
            brainH->SetRecipient(replyKey);
            brainG->SetRecipient(replyKey);

        }
    }
}
