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

#include "plNetClientMgr.h"

#include "plNetLinkingMgr.h"

#include "plCreatableIndex.h"
#include "plgDispatch.h"
#include "plPhysical.h"
#include "plProduct.h"
#include "hsSystemInfo.h"
#include "hsTimer.h"

#include "pnMessage/plClientMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnMessage/plTimeMsg.h"
#include "pnNetCommon/pnNetCommon.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plAvatar/plAvatarClothing.h"
#include "plAvatar/plAvatarMgr.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plCCRMsg.h"
#include "plMessage/plLoadAvatarMsg.h"
#include "plMessage/plNetClientMgrMsg.h"
#include "plMessage/plNetVoiceListMsg.h"
#include "plMessage/plResPatcherMsg.h"
#include "plMessage/plSDLModifierStateMsg.h"
#include "plMessage/plSynchEnableMsg.h"
#include "plMessage/plVaultNotifyMsg.h"
#include "plModifier/plResponderModifier.h"
#include "plNetClientRecorder/plNetClientRecorder.h"
#include "plNetCommon/plNetObjectDebugger.h"
#include "plNetMessage/plNetMessage.h"
#include "plNetTransport/plNetTransportMember.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plResMgr/plKeyFinder.h"
#include "plResMgr/plPageInfo.h"
#include "plScene/plSceneNode.h"
#include "plSDL/plSDL.h"
#include "plStatusLog/plStatusLog.h"
#include "plSurface/plLayerAnimation.h"
#include "plVault/plVault.h"

#include "pfMessage/pfKIMsg.h"  // Move this to PubUtil level



////////////////////////////////////////////////////////////////////

plNetClientMgr::PendingLoad::~PendingLoad()
{
    delete fSDRec;
}

////////////////////////////////////////////////////////////////////

//
// CONSTRUCT
//
plNetClientMgr::plNetClientMgr()
    : fLocalPlayerKey(), fMsgHandler(this), fJoinOrder(), fTaskProgBar(),
      fMsgRecorder(), fServerTimeOffset(), fTimeSamples(), fLastTimeUpdate(),
      fListenListMode(kListenList_Distance), fAgeSDLObjectKey(), fExperimentalLevel(),
      fOverrideAgeTimeOfDayPercent(-1.f), fNumInitialSDLStates(), fRequiredNumInitialSDLStates(),
      fDisableMsg(), fIsOwner(true), fIniPlayerID(), fPingServerType()
{   
#ifndef HS_DEBUGGING
    // release code will timeout inactive players on servers by default
    SetFlagsBit(kAllowTimeOut);
#endif
    SetFlagsBit(kAllowAuthTimeOut);

//  fPlayerVault.SetPlayerName("SinglePlayer"); // in a MP game, this will be replaced with a player name like 'Atrus'
    fTransport.SetNumChannels(kNetNumChannels); 
}

//
// DESTRUCT
//
plNetClientMgr::~plNetClientMgr()
{
    plSynchedObject::PushSynchDisabled(true);       // disable dirty tracking

    IDeInitNetClientComm(); 

    if (this==GetInstance())
        SetInstance(nullptr);       // we're going down boys
    IClearPendingLoads();
    delete fTaskProgBar;
}

//
// App is exiting
//
void plNetClientMgr::Shutdown()
{
    ISendDirtyState(hsTimer::GetSysSeconds());

    plNetLinkingMgr::GetInstance()->LeaveAge(true);

    // release all avatar clones
    IUnloadRemotePlayers();
    IUnloadNPCs();

    // Finally, pump the dispatch system so all the new refs get delivered.
    plgDispatch::Dispatch()->MsgQueueProcess();

    if (fMsgRecorder)
    {
        delete fMsgRecorder;
        fMsgRecorder = nullptr;
    }
    for (int i = 0; i < fMsgPlayers.size(); i++)
        delete fMsgPlayers[i];
    fMsgPlayers.clear();

    IRemoveCloneRoom();

    VaultDestroy();

    // commit hara-kiri
    UnRegisterAs(kNetClientMgr_KEY);
    SetInstance(nullptr);
}

//
// Clear any pending state, probably joining a new age
//
void plNetClientMgr::IClearPendingLoads()
{
    std::for_each( fPendingLoads.begin(), fPendingLoads.end(),
        [](PendingLoad *pl) { delete pl; }
    );
    fPendingLoads.clear();
}

//
// manually add/remove the cloned object room in plClient
//
void plNetClientMgr::IAddCloneRoom()
{
    plSceneNode *cloneRoom = new plSceneNode();
    cloneRoom->RegisterAs(kNetClientCloneRoom_KEY);
    plKey clientKey=hsgResMgr::ResMgr()->FindKey(kClient_KEY);
    plClientRefMsg *pMsg1 = new plClientRefMsg(clientKey, plRefMsg::kOnCreate, -1, plClientRefMsg::kManualRoom);
    hsgResMgr::ResMgr()->AddViaNotify(cloneRoom->GetKey(), pMsg1, plRefFlags::kPassiveRef);
}

//
// manually add/remove the cloned object room in plClient
//
void plNetClientMgr::IRemoveCloneRoom()
{
    plKey cloneRoomKey = hsgResMgr::ResMgr()->FindKey(kNetClientCloneRoom_KEY);
    plSceneNode *cloneRoom = cloneRoomKey ? plSceneNode::ConvertNoRef(cloneRoomKey->ObjectIsLoaded()) : nullptr;
    if (cloneRoom)
    {
        cloneRoom->UnRegisterAs(kNetClientCloneRoom_KEY);
        cloneRoom = nullptr;
    }
}       

//
// turn null send on/off.  Null send does everything except actually send the msg out on the socket
//
void plNetClientMgr::SetNullSend(bool on) 
{
}

//
// returns server time in the form "[m/d/y h:m:s]"
//
const char* plNetClientMgr::GetServerLogTimeAsString(ST::string& timestamp) const
{
    const plUnifiedTime st=GetServerTime();
    timestamp = ST::format("{{{02}/{02} {02}:{02}:{02}}",
        st.GetMonth(), st.GetDay(), st.GetHour(), st.GetMinute(), st.GetSecond());
    return timestamp.c_str();
}

//
// support for a single leading tab in statusLog strings
//
const char* ProcessTab(const char* fmt)
{
    static ST::string s;
    if (fmt && *fmt=='\t')
    {
        s = ST::format("  {}", fmt);
        return s.c_str();
    }
    return fmt;
}

//
// create StatusLog if necessary
//
void plNetClientMgr::ICreateStatusLog() const
{
    if (!fStatusLog)
    {
        fStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "network.log",
            plStatusLog::kTimestamp | plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | 
            plStatusLog::kServerTimestamp);
    }
}

//
// override for plLoggable
//
bool plNetClientMgr::Log(const ST::string& str) const
{
    if (str.empty()) {
        return true;
    }

    // prepend raw time
    ST::string buf2 = ST::format("{.2f} {}", hsTimer::GetSeconds(), ProcessTab(str.c_str()));

    if ( GetConsoleOutput() )
        hsStatusMessage(buf2.c_str());

    GetLog();

    plNetObjectDebugger::GetInstance()->LogMsgIfMatch(buf2);

    if (fStatusLog) {
        return fStatusLog->AddLine(buf2);
    }

    return true;
}

//
// Display OS version info for log
//

void plNetClientMgr::IDumpOSVersionInfo() const
{
    DebugMsg("*** OS Info");
    DebugMsg(hsSystemInfo::GetOperatingSystem());
}

//
// initialize net client.
//
void plNetClientMgr::Init()
{
    hsLogEntry( DebugMsg("*** plNetClientMgr::Init GMT:{}", plUnifiedTime::GetCurrent().Print()) );
    
    IDumpOSVersionInfo();
    
    if (GetFlagsBit(kNullSend))
        SetNullSend(true);

    VaultInitialize();

    IAddCloneRoom();

    fNetGroups.Reset();

    plgDispatch::Dispatch()->RegisterForExactType(plNetClientMgrMsg::Index(), GetKey());    
    plgDispatch::Dispatch()->RegisterForExactType(plAgeLoadedMsg::Index(), GetKey());   
    plgDispatch::Dispatch()->RegisterForExactType(plAgeLoaded2Msg::Index(), GetKey());  
    plgDispatch::Dispatch()->RegisterForExactType(plCCRPetitionMsg::Index(), GetKey()); 
    plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plInitialAgeStateLoadedMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plNetVoiceListMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plClientMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());

    plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plNetCommActivePlayerMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plNetCommLinkToAgeMsg::Index(), GetKey());

    // We need plVaultNotifyMsgs for the NetLinkingMgr
    plgDispatch::Dispatch()->RegisterForType(plVaultNotifyMsg::Index(), GetKey());
    plgDispatch::Dispatch()->RegisterForExactType(plResPatcherMsg::Index(), GetKey());

    IInitNetClientComm();
}

//
// Prepare to send.
// Update p2p transport groups and rcvrs list in some msgs
// Returns channel.
//
int plNetClientMgr::IPrepMsg(plNetMessage* msg)
{
    // pick channel, prepare msg
    int channel=kNetChanDefault;
    plNetMsgVoice* v=plNetMsgVoice::ConvertNoRef(msg);
    if (v)
    {       // VOICE MSG
        channel=kNetChanVoice;
        
        // compute new transport group (from talkList) if necessary
        GetTalkList()->UpdateTransportGroup(this);  
        
        // update receivers list in voice msg based on talk list
        v->Receivers()->Clear();
        int i;
        for(i=0;i<GetTalkList()->GetNumMembers(); i++)
        {
            v->Receivers()->AddReceiverPlayerID(GetTalkList()->GetMember(i)->GetPlayerID());
        }

        if (msg->IsBitSet(plNetMessage::kEchoBackToSender))
            v->Receivers()->AddReceiverPlayerID(GetPlayerID());
    }
    else if (plNetMsgListenListUpdate::ConvertNoRef(msg))
    {   // LISTEN LIST UPDATE MSG
        channel=kNetChanListenListUpdate;

        // update transport group from rcvrs list, add p2p mbrs to trasnport group
        fTransport.ClearChannelGrp(kNetChanListenListUpdate);
        if (IsPeerToPeer())
        {
            plNetMsgReceiversListHelper* rl = plNetMsgReceiversListHelper::ConvertNoRef(msg);
            hsAssert(rl, "error converting msg to rcvrs list?");
            int i;
            for(i=0;i<rl->GetNumReceivers();i++)
            {
                plNetTransportMember* tm = fTransport.GetMemberByID(rl->GetReceiverPlayerID(i));
                hsAssert(tm, "error finding transport mbr");
                if (tm->IsPeerToPeer())
                    fTransport.SubscribeToChannelGrp(tm, kNetChanListenListUpdate);
            }
        }
    }
    else if( plNetMsgGameMessageDirected::ConvertNoRef( msg ) )
    {
        channel = kNetChanDirectedMsg;
    }
    return channel;
}

//
// unload other player since we're leaving the age
//
void plNetClientMgr::IUnloadRemotePlayers()
{
    for (size_t i = fRemotePlayerKeys.size(); i > 0; --i)
        plAvatarMgr::GetInstance()->UnLoadAvatar(fRemotePlayerKeys[i-1], true);
    hsAssert(fRemotePlayerKeys.empty(), "Still remote players left when linking out");
}

//
// unload NPCs since we're leaving the age
//
void plNetClientMgr::IUnloadNPCs()
{
    for (size_t i = fNPCKeys.size(); i > 0; --i)
        plAvatarMgr::GetInstance()->UnLoadAvatar(fNPCKeys[i-1], false);
    hsAssert(fNPCKeys.empty(), "Still npcs left when linking out");
}

//
// compute the difference between our clock and the server's in unified time
//
void plNetClientMgr::UpdateServerTimeOffset(plNetMessage* msg)
{
    if ((hsTimer::GetSysSeconds() - fLastTimeUpdate) > 5)
    {
        fLastTimeUpdate = hsTimer::GetSysSeconds();

        const plUnifiedTime& msgSentUT = msg->GetTimeSent();
        if (!msgSentUT.AtEpoch())
        {
            double diff = plUnifiedTime::GetTimeDifference(msgSentUT, plUnifiedTime::GetCurrent());

            if (fServerTimeOffset == 0)
            {
                fServerTimeOffset = diff;
            }
            else
            {
                fServerTimeOffset = fServerTimeOffset + ((diff - fServerTimeOffset) / ++fTimeSamples);
            }

            DebugMsg("Setting server time offset to {f}", fServerTimeOffset);
        }
    }
}

void plNetClientMgr::ResetServerTimeOffset(bool delayed)
{
    if (!delayed)
        fServerTimeOffset = 0;
    fTimeSamples = 0;
    fLastTimeUpdate = 0;
}

//
// return the gameservers time
//
plUnifiedTime plNetClientMgr::GetServerTime() const 
{ 
    if ( fServerTimeOffset==0 )     // offline mode or before connecting/calibrating to a server
        return plUnifiedTime::GetCurrent();
    
    plUnifiedTime serverUT;
    if (fServerTimeOffset<0)
        return plUnifiedTime::GetCurrent() - plUnifiedTime(fabs(fServerTimeOffset));
    else
        return  plUnifiedTime::GetCurrent() + plUnifiedTime(fServerTimeOffset);
}

//
// How old is the age
//
double plNetClientMgr::GetCurrentAgeElapsedSeconds() const
{
    return plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeElapsedSeconds(GetServerTime());
}

//
// Get the time in the current age (0-1), taking into account the age's day length
//
float plNetClientMgr::GetCurrentAgeTimeOfDayPercent() const
{
    if (fOverrideAgeTimeOfDayPercent>=0)
        return fOverrideAgeTimeOfDayPercent;

    return plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeTimeOfDayPercent(GetServerTime());
}

//
// main update fxn for net client code
//
void plNetClientMgr::Update(double secs)
{
    if (GetFlagsBit(kDisableOnNextUpdate)) {
        SetFlagsBit(kDisableOnNextUpdate, false);
        IDisableNet();
    }
    
    // Pump net messages
    NetCommUpdate();

    static double lastUpdateTime=0;
    double curTime=hsTimer::GetSeconds();
    if (curTime-lastUpdateTime > 1.f)
    {
        DebugMsg("NetClient hasn't updated for {f} secs", curTime-lastUpdateTime);
    }
    lastUpdateTime=curTime;

    if (GetFlagsBit(kPlayingGame) )
    {
        MaybeSendPendingPagingRoomMsgs();
        ICheckPendingStateLoad(secs);
        ISendDirtyState(secs);
        IUpdateListenList(secs);
        if (GetFlagsBit(plNetClientApp::kShowLists))
            IShowLists();
        if (GetFlagsBit(plNetClientApp::kShowRooms))
            IShowRooms();
        if (GetFlagsBit(plNetClientApp::kShowAvatars))
            IShowAvatars();
        if (GetFlagsBit(plNetClientApp::kShowRelevanceRegions))
            IShowRelevanceRegions();
    }

    // Send dirty nodes, deliver vault callbacks, etc.
    VaultUpdate();

    plNetLinkingMgr::GetInstance()->Update();
}

//
// See if there is state recvd from the network that needsto be delivered
//
void plNetClientMgr::ICheckPendingStateLoad(double secs)
{
    // We only care if we're in an age or loading state
    if (!(GetFlagsBit(kPlayingGame) || (GetFlagsBit(kLoadingInitialAgeState) && !GetFlagsBit(kNeedInitialAgeStateCount))))
        return;

    for (auto it = fPendingLoads.begin(); it != fPendingLoads.end();)
    {
        PendingLoad* load = *it;

        // If no key has been cached, we need to find it.
        if (!load->fKey)
        {
            load->fKey = hsgResMgr::ResMgr()->FindKey(load->fUoid);

            // By this point, we should have all the age's keys downloaded from filesrv
            // So, if fKey is null at this point, this state is garbage
            if (!load->fKey)
            {
                ErrorMsg("Key `{}` not found. Discarding state for `{}`",
                          load->fUoid.GetObjectName(),
                          load->fSDRec->GetDescriptor()->GetName());
                it = fPendingLoads.erase(it);
                delete load;
                continue;
            }
        }

        // Time to deliver the state!
        plSynchedObject* synchObj = plSynchedObject::ConvertNoRef(load->fKey->ObjectIsLoaded());
        if (synchObj && synchObj->IsFinal())
        {
            plSDLModifierStateMsg* msg = new plSDLModifierStateMsg(load->fSDRec->GetDescriptor()->GetName(), plSDLModifierMsg::kRecv);
            msg->SetState(load->fSDRec, true);
            load->fSDRec = nullptr;
            msg->SetPlayerID(load->fPlayerID);

#ifdef HS_DEBUGGING
            if (plNetObjectDebugger::GetInstance()->IsDebugObject(synchObj))
            {
                DebugMsg("Delivering SDL State '{}' to {} owned key {}",
                    msg->GetState()->GetDescriptor()->GetName(),
                    (synchObj->IsLocallyOwned() == plSynchedObject::kYes) ? "locally" : "remote",
                    load->fUoid.StringIze());
            }
#endif
            msg->Send(load->fKey);
            it = fPendingLoads.erase(it);
            delete load;
        }
        else if (GetFlagsBit(kPlayingGame))
        {
            // If we're playing the game and object isn't loaded/final, then this state is probably
            // never going to be useful (it's from some paged in hack or something that's been deleted)
            // Throw it away.
            it = fPendingLoads.erase(it);
            delete load;
        }
        else
            ++it;
    }
}

//
// Determine the net group for a given object (based on paging unit)
//
plNetGroupId plNetClientMgr::SelectNetGroup(plSynchedObject* objIn, const plKey& roomKey)
{
    if( objIn->GetSynchFlags() & plSynchedObject::kHasConstantNetGroup )
        return objIn->GetNetGroup();

    return plNetGroupId(roomKey->GetUoid().GetLocation(), 0);
}

//
// Get the net group that an object belongs to, taking into acct
// that objects may derive their net group from another object they belong to.
//
plNetGroupId plNetClientMgr::GetEffectiveNetGroup(const plSynchedObject*& obj) const
{
   plNetGroupId netGroup = plNetGroup::kNetGroupUnknown;
    // the object is an interface, so consider the owner instead 
    const plObjInterface* oInt=plObjInterface::ConvertNoRef(obj);
    if (oInt)
    {
        // the object is an interface, so consider the owner instead 
        if (oInt->GetOwner())
            obj = oInt->GetOwner();
    }   
    else
    {
        const plModifier* mod=plModifier::ConvertNoRef(obj);
        if (mod)
        {
            // the object is a modifier, so consider the first target instead 
            if (mod->GetNumTargets() && mod->GetTarget(0))
                obj = mod->GetTarget(0);
        }
        else
        {
            const plLayerInterface* li = plLayerInterface::ConvertNoRef(obj);
            const plClothingOutfit* cl = plClothingOutfit::ConvertNoRef(obj);
            if (li || cl)
            {
                // the object is a layer interface or clothing object
                plUoid u = obj->GetKey()->GetUoid();
                if (u.GetLocation().IsReserved())
                {
                    // layer interface is associated with an avatar, find which one
                    int cloneID = u.GetCloneID();
                    int clonePlayerID = u.GetClonePlayerID();
                    for(int i = -1; i < RemotePlayerKeys().size(); i++)
                    {
                        plKey pKey = (i==-1) ? GetLocalPlayerKey() : RemotePlayerKeys()[i];
                        if (pKey && pKey->GetUoid().GetCloneID()==cloneID && pKey->GetUoid().GetClonePlayerID()==clonePlayerID)
                        {
                            obj = plSynchedObject::ConvertNoRef(pKey->ObjectIsLoaded());
                            break;
                        }
                    }
                }
                else
                {
                    // layer interface is on a non-avatar object
                    netGroup = u.GetLocation();
                }
            }
            else
            {
                const plPhysical* ph = plPhysical::ConvertNoRef(obj);
                if (ph)
                {
                    // the object is a physical
                    obj = plSynchedObject::ConvertNoRef(ph->GetObjectKey()->ObjectIsLoaded());
                }
            }
        }
    }
    
    if (obj)
        netGroup = obj->GetNetGroup();  
    
    return netGroup;
}

//
// If we don't know for sure if something is locallyOwned, check if it' related to the remote or 
// local avatar.  if not, fallback to join order.
//
int plNetClientMgr::IDeduceLocallyOwned(const plUoid& uoid) const
{
    // check against local/remote avatars
    if (fLocalPlayerKey)
        if (uoid.GetLocation() == fLocalPlayerKey->GetUoid().GetLocation() &&
            uoid.GetClonePlayerID() == fLocalPlayerKey->GetUoid().GetClonePlayerID())
            return plSynchedObject::kYes;   // this object's location is the same as the local player's

    if (!fRemotePlayerKeys.empty())
        if (uoid.GetLocation() == fRemotePlayerKeys.back()->GetUoid().GetLocation() &&
            uoid.GetClonePlayerID() == fRemotePlayerKeys.back()->GetUoid().GetClonePlayerID())
            return plSynchedObject::kNo;    // this object's location is the same as the currently loading remote player's

    return fIsOwner;
}


//
// returns yes/no
// for special cases, like sceneNodes.
//
int plNetClientMgr::IsLocallyOwned(const plUoid& uoid) const
{
    // we're non-networked
    if ( GetFlagsBit(kDisabled))
        return plSynchedObject::kYes;

    plNetGroupId netGroup = uoid.GetLocation();
    int answer=GetNetGroups()->IsGroupLocal(netGroup);
    
    if (answer==-1)
        answer = IDeduceLocallyOwned(uoid);

    hsAssert(answer==plSynchedObject::kYes || answer==plSynchedObject::kNo, "invalid answer to IsLocallyOwned()");
    return answer;
}


//
// check if this object is in a group which is owned (mastered) by this client.
// returns yes/no
//
int plNetClientMgr::IsLocallyOwned(const plSynchedObject* obj) const
{
    // we're non-networked
    if ( GetFlagsBit(kDisabled) )
        return plSynchedObject::kYes;

    if( !obj )
        return plSynchedObject::kNo;

    // object is flagged as local only
    if (!obj->IsNetSynched())
        return plSynchedObject::kYes;

    plNetGroupId netGroup = GetEffectiveNetGroup(obj);
    int answer=GetNetGroups()->IsGroupLocal(netGroup);

    if (answer==-1)     // not sure
        answer = IDeduceLocallyOwned(obj->GetKey()->GetUoid());

    hsAssert(answer==plSynchedObject::kYes || answer==plSynchedObject::kNo, "invalid answer to IsLocallyOwned()");
    return answer;
}

//
// return localPlayer ptr
//
plSynchedObject* plNetClientMgr::GetLocalPlayer(bool forceLoad) const
{ 
    if (forceLoad)
        return fLocalPlayerKey ? plSynchedObject::ConvertNoRef(fLocalPlayerKey->GetObjectPtr()) : nullptr;
    else
        return fLocalPlayerKey ?
            plSynchedObject::ConvertNoRef(fLocalPlayerKey->ObjectIsLoaded()) : nullptr;
}

plSynchedObject* plNetClientMgr::GetNPC(uint32_t i) const
{
    if (i >= fNPCKeys.size())
        return nullptr;
    return plSynchedObject::ConvertNoRef(fNPCKeys[i]->ObjectIsLoaded()); 
}

void plNetClientMgr::AddNPCKey(const plKey& npc)
{
    // note: npc keys have little sanity checking...
    hsAssert(npc, "adding nil npc key? naughty, naughty...");
    fNPCKeys.push_back(npc);
}

bool plNetClientMgr::IsNPCKey(const plKey& npc, int* idx) const
{
    if (npc)
    {
        plKeyVec::const_iterator it = std::find(fNPCKeys.begin(), fNPCKeys.end(), npc);
        bool found = it != fNPCKeys.end();
        if (idx)
            *idx = found ? (int)(it - fNPCKeys.begin()) : -1;
        return found;
    }
    return false;
}

//
// return a ptr to a remote player
//
plSynchedObject* plNetClientMgr::GetRemotePlayer(int i) const
{ 
    return fRemotePlayerKeys[i] ? plSynchedObject::ConvertNoRef(fRemotePlayerKeys[i]->ObjectIsLoaded()) : nullptr;
}

//
// check if a key si a remote player
//
bool plNetClientMgr::IsRemotePlayerKey(const plKey& pKey, int *idx)
{
    if (pKey)
    {
        plKeyVec::iterator result=std::find(fRemotePlayerKeys.begin(), fRemotePlayerKeys.end(), pKey);
        bool found = result!=fRemotePlayerKeys.end();
        if (idx)
            *idx = found ? (int)(result-fRemotePlayerKeys.begin()) : -1;
        return found;
    }
    return false;
}

//
// add remote char if not already there
//
void plNetClientMgr::AddRemotePlayerKey(plKey pKey)
{
    hsAssert(pKey, "adding nil remote player key");
    if (!IsRemotePlayerKey(pKey))
    {
        hsAssert(pKey != fLocalPlayerKey, "setting remote player to the local player?");
        fRemotePlayerKeys.emplace_back(std::move(pKey));
    }
}

//
// MsgReceive handler for plasma messages
//
bool plNetClientMgr::MsgReceive( plMessage* msg )
{
    if (plNetLinkingMgr::GetInstance()->MsgReceive( msg ))
        return true;

    plEvalMsg* evalMsg = plEvalMsg::ConvertNoRef(msg);
    if (evalMsg)
    {
        IPlaybackMsgs();

        if ( GetFlagsBit( kNeedToSendAgeLoadedMsg ) )
        {
            SetFlagsBit( kNeedToSendAgeLoadedMsg, false );
            plAgeLoader::GetInstance()->NotifyAgeLoaded( true );
        }

        if ( GetFlagsBit( kNeedToSendInitialAgeStateLoadedMsg ) )
        {
            SetFlagsBit(kNeedToSendInitialAgeStateLoadedMsg, false);
            plInitialAgeStateLoadedMsg* m = new plInitialAgeStateLoadedMsg;
            m->Send();
        }

        return true;
    }

    plGenRefMsg* ref = plGenRefMsg::ConvertNoRef(msg);
    if (ref)
    {
        if( ref->fType == kVaultImage )
        {
            // Ignore, we just use it for reffing, don't care about the actual pointer
            return true;
        }

        hsAssert(ref->fType==kAgeSDLHook, "unknown ref msg context");
        if (ref->GetContext()==plRefMsg::kOnCreate)
        {
            hsAssert(fAgeSDLObjectKey == nullptr, "already have a ref to age sdl hook");
            fAgeSDLObjectKey = ref->GetRef()->GetKey();
            DebugMsg("Age SDL hook object created, uoid={}", fAgeSDLObjectKey->GetUoid().StringIze());
        }
        else
        {
            fAgeSDLObjectKey = nullptr;
            DebugMsg("Age SDL hook object destroyed");
        }
        return true;
    }

    if (plNetClientMgrMsg * ncmMsg = plNetClientMgrMsg::ConvertNoRef(msg)) {
        if (ncmMsg->type == plNetClientMgrMsg::kCmdDisableNet) {
            SetFlagsBit(kDisableOnNextUpdate);
            hsRefCnt_SafeUnRef(fDisableMsg);
            fDisableMsg = ncmMsg;
            fDisableMsg->Ref();
        }
        return true;
    }
    
    if (plNetCommAuthMsg * authMsg = plNetCommAuthMsg::ConvertNoRef(msg)) {
        if (IS_NET_ERROR(authMsg->result)) {
            char str[256];
            snprintf(str, std::size(str), "Authentication failed: %S", NetErrorToString(authMsg->result));
            QueueDisableNet(true, str);
            return false;   // @@@ TODO: Handle this failure better
        }

        return true;
    }

    if (plNetCommActivePlayerMsg * activePlrMsg = plNetCommActivePlayerMsg::ConvertNoRef(msg)) {
        if (IS_NET_ERROR(activePlrMsg->result)) {
            char str[256];
            snprintf(str, std::size(str), "SetActivePlayer failed: %S", NetErrorToString(activePlrMsg->result));
            QueueDisableNet(true, str);
            return false;   // @@@ TODO: Handle this failure better.
        }
            
        return true;
    }

    plPlayerPageMsg *playerPageMsg = plPlayerPageMsg::ConvertNoRef(msg);
    if(playerPageMsg)
    {
        IHandlePlayerPageMsg(playerPageMsg);
        return true;    // handled
    }

    plLoadCloneMsg* pCloneMsg = plLoadCloneMsg::ConvertNoRef(msg);
    if(pCloneMsg)
    {
        ILoadClone(pCloneMsg);
        return true;    // handled
    }

    // player is petitioning a CCR
    plCCRPetitionMsg* petMsg=plCCRPetitionMsg::ConvertNoRef(msg);
    if (petMsg)
    {
        ISendCCRPetition(petMsg);
        return true;
    }

    // a remote CCR is turning invisible
    plCCRInvisibleMsg* invisMsg=plCCRInvisibleMsg::ConvertNoRef(msg);
    if (invisMsg)
    {
        DebugMsg("plNetClientMgr::MsgReceive - Got plCCRInvisibleMsg");
        MakeCCRInvisible(invisMsg->fAvKey, invisMsg->fInvisLevel);
        return true;
    }
    
    plCCRBanLinkingMsg* banLinking = plCCRBanLinkingMsg::ConvertNoRef(msg);
    if (banLinking)
    {
        DebugMsg("Setting BanLinking to {}", banLinking->fBan);
        SetFlagsBit(kBanLinking, banLinking->fBan);
        return true;
    }

    plCCRSilencePlayerMsg* silence = plCCRSilencePlayerMsg::ConvertNoRef(msg);
    if (silence)
    {
        DebugMsg("Setting Silence to {}", silence->fSilence);
        SetFlagsBit(kSilencePlayer, silence->fSilence);
        return true;
    }

    plNetVoiceListMsg* voxList = plNetVoiceListMsg::ConvertNoRef(msg);
    if (voxList)
    {
        IHandleNetVoiceListMsg(voxList);
        return true;
    }

    plSynchEnableMsg* synchEnable = plSynchEnableMsg::ConvertNoRef(msg);
    if (synchEnable)
    {
        if (synchEnable->fPush)
        {
            plSynchedObject::PushSynchDisabled(!synchEnable->fEnable);
        }
        else
        {
            plSynchedObject::PopSynchDisabled();
        }
        return true;
    }

    plClientMsg* clientMsg = plClientMsg::ConvertNoRef(msg);
    if (clientMsg && clientMsg->GetClientMsgFlag()==plClientMsg::kInitComplete)
    {
        // add 1 debug object for age sdl
        if (plNetObjectDebugger::GetInstance())
        {
            plNetObjectDebugger::GetInstance()->RemoveDebugObject(ST_LITERAL("AgeSDLHook"));
            plNetObjectDebugger::GetInstance()->AddDebugObject(ST_LITERAL("AgeSDLHook"));
        }

        // if we're linking to startup we don't need (or want) a player set
        ST::string ageName = NetCommGetStartupAge()->ageDatasetName;
        if (ageName.empty())
            ageName = ST_LITERAL("StartUp");
        if (ageName.compare_i("StartUp") == 0)
            NetCommSetActivePlayer(0, nullptr);

        plAgeLinkStruct link;
        link.GetAgeInfo()->SetAgeFilename(NetCommGetStartupAge()->ageDatasetName);
        link.SetLinkingRules(plNetCommon::LinkingRules::kOriginalBook);
        plNetLinkingMgr::GetInstance()->LinkToAge(&link);

        return true;
    }
    
    return plNetClientApp::MsgReceive(msg);
}

void plNetClientMgr::IncNumInitialSDLStates()
{
    fNumInitialSDLStates++;
    DebugMsg( "Received {} initial SDL states", fNumInitialSDLStates );
    if ( GetFlagsBit( plNetClientApp::kNeedInitialAgeStateCount ) )
    {
        DebugMsg( "Need initial SDL state count" );
        return;
    }

    if ( GetNumInitialSDLStates()>=GetRequiredNumInitialSDLStates() )
    {
        ICheckPendingStateLoad(hsTimer::GetSysSeconds());
        NotifyRcvdAllSDLStates();
    }
}

void plNetClientMgr::NotifyRcvdAllSDLStates() {
    DebugMsg( "Got all initial SDL states" );
    plNetClientMgrMsg * msg = new plNetClientMgrMsg(plNetClientMgrMsg::kNotifyRcvdAllSDLStates);
    msg->SetBCastFlag(plMessage::kBCastByType);
    msg->Send();
}

//
// return true if we should send this msg
//
bool plNetClientMgr::CanSendMsg(plNetMessage * msg)
{
    // TEMP
    if (GetFlagsBit(kSilencePlayer))
    {
        if (plNetMsgVoice::ConvertNoRef(msg))
            return false;

        plNetMsgGameMessage* gm=plNetMsgGameMessage::ConvertNoRef(msg);
        if (gm && gm->StreamInfo()->GetStreamType()==pfKIMsg::Index())
        {
            hsReadOnlyStream stream(gm->StreamInfo()->GetStreamLen(), gm->StreamInfo()->GetStreamBuf());
            pfKIMsg* kiMsg = pfKIMsg::ConvertNoRef(hsgResMgr::ResMgr()->ReadCreatable(&stream));
            if (kiMsg && kiMsg->GetCommand()==pfKIMsg::kHACKChatMsg)
            {
                delete kiMsg;
                return false;
            }
            delete kiMsg;
        }
    }

    return true;
}

//
// Return the net client (account) name of the player whose avatar
// key is provided.  If avKey is nil, returns local client name.
//
ST::string plNetClientMgr::GetPlayerName(const plKey avKey) const
{
    // local case
    if (!avKey || avKey == GetLocalPlayerKey())
        return NetCommGetPlayer()->playerName;

    plNetTransportMember* mbr = TransportMgr().GetMemberByKey(avKey);
    return mbr ? mbr->GetPlayerName() : ST::string();
}

ST::string plNetClientMgr::GetPlayerNameById (unsigned playerId) const {
    // local case
    if (NetCommGetPlayer()->playerInt == playerId)
        return NetCommGetPlayer()->playerName;

    plNetTransportMember* mbr = TransportMgr().GetMemberByID(playerId);
    return mbr ? mbr->GetPlayerName() : ST::string();
}

unsigned plNetClientMgr::GetPlayerIdByName (const ST::string & name) const {
    // local case
    if (name.compare_i(NetCommGetPlayer()->playerName) == 0)
        return NetCommGetPlayer()->playerInt;

    size_t n = TransportMgr().GetNumMembers();
    for (size_t i = 0; i < n; ++i)
        if (plNetTransportMember * member = TransportMgr().GetMember(i))
            if (0 == name.compare(member->GetPlayerName()))
                return member->GetPlayerID();
    return 0;
}

uint32_t plNetClientMgr::GetPlayerID() const
{
    return NetCommGetPlayer()->playerInt;
}

//
// return true if the age is the set of local ages
//
static const char* gLocalAges[]={"GUI", "AvatarCustomization", ""};
static bool IsLocalAge(const char* ageName)
{
    int i=0;
    while(strlen(gLocalAges[i]))
    {
        if (!stricmp(gLocalAges[i], ageName))
            return true;
        i++;
    }
    return false;
}

//
// is the object in a local age?
//
bool plNetClientMgr::ObjectInLocalAge(const plSynchedObject* obj) const
{
    plLocation loc = obj->GetKey()->GetUoid().GetLocation();
    const plPageInfo* pageInfo = plKeyFinder::Instance().GetLocationInfo(loc);
    bool ret= IsLocalAge(pageInfo->GetAge().c_str());
    return ret;
}

//
// the next age we are going to
//
ST::string plNetClientMgr::GetNextAgeFilename() const
{ 
    // set when we start linking to an age.
    plNetLinkingMgr * lm = plNetLinkingMgr::GetInstance();
    return lm->GetAgeLink()->GetAgeInfo()->GetAgeFilename();
}

//
// a remote ccr is turning [in]visible
//
void plNetClientMgr::MakeCCRInvisible(plKey avKey, int level)
{
    if (!avKey)
    {
        ErrorMsg("Failed to make remote CCR Invisible, nil avatar key");
        return;
    }
        
    if (level<0)
    {
        ErrorMsg("Failed to make remote CCR Invisible, negative level");
        return;
    }
    
    if (!avKey->ObjectIsLoaded() || !avKey->ObjectIsLoaded()->IsFinal())
    {
        hsAssert(false, "avatar not loaded or final");
        return;
    }
    
    plAvatarStealthModeMsg *msg = new plAvatarStealthModeMsg();
    msg->SetSender(std::move(avKey));
    msg->fLevel = level;

    if (GetCCRLevel()<level)    // I'm a lower level than him, so he's invisible to me
        msg->fMode = plAvatarStealthModeMsg::kStealthCloaked;       
    else
    {
        // he's visible to me
        if (AmCCR() && level > 0)
            msg->fMode = plAvatarStealthModeMsg::kStealthCloakedButSeen;    // draw as semi-invis
        else
            msg->fMode = plAvatarStealthModeMsg::kStealthVisible;
    }

    DebugMsg("Handled MakeCCRInvisible - sending stealth msg");;

    // This fxn is called when avatar SDL state is received from the server,
    // so this msg will inherit the 'non-local' status of the parent sdl msg.
    // That means that the avatar linkSound won't receive it, since the linkSound
    // is set as localOnly.
    // So, terminate the remote cascade and start a new (local) cascade.
    msg->SetBCastFlag(plMessage::kNetStartCascade); 
    
    msg->Send();
}

void plNetClientMgr::QueueDisableNet (bool showDlg, const char str[]) {

    plNetClientMgrMsg * msg = new plNetClientMgrMsg(plNetClientMgrMsg::kCmdDisableNet,
                                                    showDlg, str);

#if 0
    msg->Send(GetKey());
#else
    msg->Ref();
    fDisableMsg = msg;
    IDisableNet();
#endif
}

void plNetClientMgr::IDisableNet () {
    ASSERT(fDisableMsg);
    
    if (!GetFlagsBit(kDisabled)) {
        SetFlagsBit(kDisabled);
        // cause subsequent net operations to fail immediately, but don't block
        // waiting for net subsystem to shutdown (we'll do that later)
        NetCommEnableNet(false, false);

        // display a msg to the player
        if ( fDisableMsg->yes )
        {
            if (!GetFlagsBit(plNetClientApp::kPlayingGame))
            {
                // KI may not be loaded
                ST::string title = ST::format("{} Error", plProduct::CoreName());
                hsMessageBox(fDisableMsg->str, title.c_str(), hsMessageBoxNormal, hsMessageBoxIconError );
                plClientMsg *quitMsg = new plClientMsg(plClientMsg::kQuit);
                quitMsg->Send(hsgResMgr::ResMgr()->FindKey(kClient_KEY));
            }
            else
            {
                pfKIMsg *msg = new pfKIMsg(pfKIMsg::kKIOKDialog);
                msg->SetString(fDisableMsg->str);
                msg->Send();
            }
        }
    }

    hsRefCnt_SafeUnRef(fDisableMsg);
    fDisableMsg = nullptr;
}

bool plNetClientMgr::IHandlePlayerPageMsg(plPlayerPageMsg *playerMsg)
{
    bool result = false;
    plKey playerKey = playerMsg->fPlayer;

    if(playerMsg->fUnload)
    {
        if (GetLocalPlayerKey() == playerKey)
        {
            fLocalPlayerKey = nullptr;
            DebugMsg("Net: Unloading local player {}", playerKey->GetName());

            // notify server - NOTE: he might not still be around to get this...
            plNetMsgPlayerPage npp (playerKey->GetUoid(), playerMsg->fUnload);
            npp.SetNetProtocol(kNetProtocolCli2Game);
            SendMsg(&npp);
        }
        else if (int idx; IsRemotePlayerKey(playerKey, &idx))
        {
            fRemotePlayerKeys.erase(fRemotePlayerKeys.begin()+idx); // remove key from list
            DebugMsg("Net: Unloading remote player {}", playerKey->GetName());
        }
    }
    else
    {
        plSceneObject *playerSO = plSceneObject::ConvertNoRef(playerKey->ObjectIsLoaded());
        if (!playerSO)
        {
            hsStatusMessageF("Ignoring player page message for non-existant player.");
        }
        else
        if(playerMsg->fPlayer)
        {
            if (playerMsg->fLocallyOriginated)
            {
                hsAssert(!GetLocalPlayerKey() ||
                    GetLocalPlayerKey() == playerKey,
                    "Different local player already loaded");

                hsLogEntry(DebugMsg("Adding LOCAL player {}\n", playerKey->GetName()));
                playerSO->SetNetGroupConstant(plNetGroup::kNetGroupLocalPlayer);

                // don't save avatar state permanently on server
                playerSO->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);               
                const plCoordinateInterface* co = playerSO->GetCoordinateInterface();
                if (co)
                {
                    for (size_t i = 0; i < co->GetNumChildren(); i++)
                    {
                        if (co->GetChild(i) && co->GetChild(i)->GetOwner())
                                const_cast<plSceneObject*>(co->GetChild(i)->GetOwner())->SetSynchFlagsBit(plSynchedObject::kAllStateIsVolatile);
                    }
                }

                // notify server
                plNetMsgPlayerPage npp (playerKey->GetUoid(), playerMsg->fUnload);
                npp.SetNetProtocol(kNetProtocolCli2Game);
                SendMsg(&npp);
            }
            else
            {
                hsLogEntry(DebugMsg("Adding REMOTE player {}\n", playerKey->GetName()));
                playerSO->SetNetGroupConstant(plNetGroup::kNetGroupRemotePlayer);
                plNetTransportMember* mbr = fTransport.GetMemberByID(playerMsg->fClientID);
                if (mbr)
                {
                    hsAssert(playerKey, "NIL KEY?");
                    hsAssert(!playerKey->GetName().empty(), "UNNAMED KEY");
                    mbr->SetAvatarKey(playerKey);
                }
                else
                {
                    hsLogEntry(DebugMsg("Ignoring player page msg (player not found in member list) : {}\n", playerKey->GetName()));
                }
            }

            hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plAvatarSDLModifier)), "avatar missing avatar SDL modifier");
            hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plClothingSDLModifier)), "avatar missing clothing SDL modifier");
            hsAssert(IFindModifier(playerSO, CLASS_INDEX_SCOPED(plAGMasterSDLModifier)), "avatar missing AGMaster SDL modifier");
            result = true;
        }
    }
    return result;
}

// for debugging purposes
bool plNetClientMgr::IFindModifier(plSynchedObject* obj, int16_t classIdx)
{
    plLayerAnimation* layer = plLayerAnimation::ConvertNoRef(obj);
    if (layer)
        return (layer->GetSDLModifier() != nullptr);

    plResponderModifier* resp = plResponderModifier::ConvertNoRef(obj);
    if (resp)
        return (resp->GetSDLModifier() != nullptr);

    int cnt=0;
    plSceneObject* sceneObj=plSceneObject::ConvertNoRef(obj);
    if (sceneObj)
    {
        for (size_t i = 0; i < sceneObj->GetNumModifiers(); i++)
            if (sceneObj->GetModifier(i) && sceneObj->GetModifier(i)->ClassIndex()==classIdx)
                cnt++;
    }

    hsAssert(cnt<2, ST::format("Object {} has multiple SDL modifiers of the same kind ({})?",
             obj->GetKeyName(), plFactory::GetNameOfClass(classIdx)).c_str());
    return cnt==0 ? false : true;
}

plUoid plNetClientMgr::GetAgeSDLObjectUoid(const ST::string& ageName) const
{
    hsAssert(!ageName.empty(), "nil ageName");

    // if age sdl hook is loaded
    if (fAgeSDLObjectKey)
        return fAgeSDLObjectKey->GetUoid();

    // if age is loaded
    plLocation loc = plKeyFinder::Instance().FindLocation(ageName,plAgeDescription::GetCommonPage(plAgeDescription::kGlobal));
    if (!loc.IsValid())
    {
        // check current age des
        if (plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName() == ageName)
            loc=plAgeLoader::GetInstance()->GetCurrAgeDesc().CalcPageLocation("BuiltIn");

        if (!loc.IsValid())
        {
            // try to load age desc
            std::unique_ptr<hsStream> stream = plAgeLoader::GetAgeDescFileStream(ageName);
            if (stream)
            {
                plAgeDescription ad;
                ad.Read(stream.get());
                loc=ad.CalcPageLocation("BuiltIn");
            }
        }
    }

    return plUoid(loc, plSceneObject::Index(), plSDL::kAgeSDLObjectName);
}

//
// Add a state update to the pending queue
//
void plNetClientMgr::AddPendingLoad(PendingLoad *pl) 
{ 
    // find corresponding key
    pl->fKey = hsgResMgr::ResMgr()->FindKey(pl->fUoid);

#ifdef HS_DEBUGGING
    // check for age SDL state
    if (!pl->fUoid.GetObjectName().empty() && !pl->fUoid.GetObjectName().compare(plSDL::kAgeSDLObjectName))
    {
        DebugMsg("Recv SDL state for age hook object, uoid={}", pl->fUoid.StringIze());
        if (!pl->fKey)
            WarningMsg("Can't find age hook object, nil key!");
        else
            DebugMsg("Found age hook object");
    }
#endif

    // check if object is ready
    if (pl->fKey)
    {
        if (!pl->fKey->ObjectIsLoaded())
        {
            WarningMsg("Object {} not loaded, withholding SDL state",
                pl->fKey->GetUoid().StringIze());
        }
        else if (!pl->fKey->ObjectIsLoaded()->IsFinal())
        {
            WarningMsg("Object {} is not FINAL, withholding SDL state",
                pl->fKey->GetUoid().StringIze());
        }
    }

    // add entry
    fPendingLoads.push_back(pl);
}

void plNetClientMgr::AddPendingPagingRoomMsg( plNetMsgPagingRoom * msg )
{
    fPendingPagingRoomMsgs.push_back( msg );
}

void plNetClientMgr::MaybeSendPendingPagingRoomMsgs()
{
    if ( GetFlagsBit( kPlayingGame ) )
        SendPendingPagingRoomMsgs();
}

void plNetClientMgr::SendPendingPagingRoomMsgs()
{
    for ( int i=0; i<fPendingPagingRoomMsgs.size(); i++ )
        SendMsg( fPendingPagingRoomMsgs[i] );
    ClearPendingPagingRoomMsgs();
}

void plNetClientMgr::ClearPendingPagingRoomMsgs()
{
    std::for_each( fPendingPagingRoomMsgs.begin(), fPendingPagingRoomMsgs.end(),
        [](plNetMsgPagingRoom *pr) { delete pr; }
    );
    fPendingPagingRoomMsgs.clear();
}

void plNetClientMgr::BeginTask()
{
    fTaskProgBar = plProgressMgr::GetInstance()->RegisterOverallOperation(0.f);
}

void plNetClientMgr::EndTask()
{
    delete fTaskProgBar;
    fTaskProgBar = nullptr;
}

bool plNetClientMgr::IsObjectOwner()
{
    return fIsOwner;
}

void plNetClientMgr::SetObjectOwner(bool own)
{
    fIsOwner = own;
}
