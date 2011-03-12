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
#pragma warning(disable: 4503 4786)

#include <algorithm>

#include "plAvatarMgr.h"

// local
#include "plArmatureMod.h"
#include "plSeekPointMod.h"
#include "plOneShotMod.h"
#include "plArmatureMod.h"
#include "plAGModifier.h"
#include "plAnimStage.h"
#include "plCoopCoordinator.h"
#include "plAvBrainCoop.h"

// global
#include "hsResMgr.h"
#include "../pnNetCommon/plNetApp.h"
#include "plgDispatch.h"
#include "hsTimer.h"

// other
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plResMgr/plKeyFinder.h"
#include "../pfCCR/plCCRMgr.h" // Only included for defined constants. 
#include "../plNetTransport/plNetTransport.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plModifier/plSpawnModifier.h"
#include "../plModifier/plMaintainersMarkerModifier.h"
#include "../plVault/plDniCoordinateInfo.h"
#include "../plMath/plRandom.h"

#include "../pnMessage/plPlayerPageMsg.h"
#include "../pnMessage/plWarpMsg.h"
#include "../pnMessage/plNotifyMsg.h"

#include "../plMessage/plMemberUpdateMsg.h"
#include "../plMessage/plAvatarMsg.h"
#include "../plMessage/plAvCoopMsg.h"
#include "../pnMessage/plTimeMsg.h"
#include "../plStatusLog/plStatusLog.h"

// The static single instance, allocated on demand by GetInstance()
plAvatarMgr		*plAvatarMgr::fInstance = nil;

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
	fLog = nil;
}

// GETINSTANCE
plAvatarMgr * plAvatarMgr::GetInstance()
{
	if(!fInstance)
	{
		fInstance = TRACKED_NEW plAvatarMgr;
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
		fInstance = nil;
	}
}

// RESET
void plAvatarMgr::IReset()
{
	fSeekPoints.clear();

	// Oneshots have copies of strings in their maps. I'm assuming the others should be the same, but until
	// I hear otherwise...
	for( plOneShotMap::iterator it = fOneShots.begin(); it != fOneShots.end(); it++ )
		delete [] (char *)it->first;
	fOneShots.clear();
	fAvatars.clear();
	fSpawnPoints.clear();
	fMaintainersMarkers.SetCountAndZero(0);

	plCoopMap::iterator acIt = fActiveCoops.begin();
	while (acIt != fActiveCoops.end())
	{
		plCoopCoordinator* deadCoop = acIt->second;
		delete deadCoop;
		acIt++;
	}
	fActiveCoops.clear();
}

plKey plAvatarMgr::LoadPlayer(const char *name, const char *account)
{
	return LoadAvatar(name, account, true, nil, nil);
}

plKey plAvatarMgr::LoadPlayer(const char *name, const char *account, const char *linkInName)
{
	// what we'd like to do is turn the linkInName into a spawn point key and
	// put that into the plLoadAvatarMsg, which is already set up to handle
	// initial spawn points.
	// however, that will require that we can handle waiting for our spawn point to load,
	// so we're goin to do this the "old way" for now.
	
	plArmatureMod::SetSpawnPointOverride(linkInName);
	return LoadAvatar(name, account, true, nil, nil);
}


plKey plAvatarMgr::LoadAvatar(const char *name, const char *accountName, bool isPlayer, plKey spawnPoint, plAvTask *initialTask, const char *userStr /*=nil*/)
{
	// *** account is currently unused. the idea is that eventually an NPC will
	// *** be able to use a customization account
	plKey result = nil;
	plKey requestor = GetKey();	// avatar manager is always the requestor for avatar loads
	plNetClientMgr *netMgr = plNetClientMgr::GetInstance();

	if(netMgr)		// can't clone without the net manager
	{
		hsAssert(name, "name required by LoadPlayer fxn");
		netMgr->DebugMsg("Local: Loading player %s", name);	

		// look up player by key name provided by user.
		// this string search should be replaced with some other method of 
		// avatar selection and key lookup.

		// Get the location for the player first
		plKey playerKey = nil;
		const plLocation& globalLoc = plKeyFinder::Instance().FindLocation("GlobalAvatars", name);
		const plLocation& maleLoc = plKeyFinder::Instance().FindLocation("GlobalAvatars", "Male");
		const plLocation& custLoc = plKeyFinder::Instance().FindLocation("CustomAvatars", name);

		// Silliness to make the compiler happy with const references.
		// and don't allow players to use custom avatars
		const plLocation& loc = (globalLoc.IsValid() ? globalLoc : isPlayer ? maleLoc : custLoc);

		const char* theName = name;
		if ( isPlayer && !globalLoc.IsValid() )
			theName = "Male";

		if (loc.IsValid())
		{
			plUoid uID(loc, plSceneObject::Index(), theName);
			plLoadAvatarMsg *cloneMsg = TRACKED_NEW plLoadAvatarMsg (uID, requestor, 0, isPlayer, spawnPoint, initialTask, userStr);
			result =  cloneMsg->GetCloneKey();
			
			// the clone message is automatically addressed to the net client manager
			// we'll receive the message back (or a similar message) when the clone is loaded
			cloneMsg->Send();
		}
	}
	return result;
}

void plAvatarMgr::UnLoadAvatar(plKey avatarKey, bool isPlayer)
{
	hsBool isLoading = false;
	plLoadAvatarMsg *msg = TRACKED_NEW plLoadAvatarMsg(avatarKey, GetKey(), 0, isPlayer, isLoading);
	msg->Send();
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
		hsBool isLoading = true;
		plLoadAvatarMsg *msg = TRACKED_NEW plLoadAvatarMsg(playerKey, requestor, 0, isPlayer, isLoading);

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
		hsBool isLoading = false;
		plLoadAvatarMsg *msg = TRACKED_NEW plLoadAvatarMsg(playerKey, requestor, 0, isPlayer, isLoading);
		msg->SetBCastFlag(plMessage::kLocalPropagate, false);
		msg->Send();
		return true;
	}
	return false;
}

// UNLOADREMOTEPLAYER
void plAvatarMgr::UnLoadRemotePlayer(plKey remotePlayer)
{
	if(remotePlayer)
	{
		plKey requestor = GetKey();
		bool isPlayer = true;
		hsBool isLoading = false;
		plLoadAvatarMsg * msg = TRACKED_NEW plLoadAvatarMsg(remotePlayer, requestor, 0, isPlayer, isLoading);

		// don't propagate over the network. this is just for removing our local version
		msg->SetBCastFlag(plMessage::kNetPropagate, false);
		msg->Send();
	}
}

// UNLOADLOCALPLAYER
void plAvatarMgr::UnLoadLocalPlayer()
{
	plKey playerKey = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if(playerKey)
	{
		plKey mgrKey = GetKey();
		bool isPlayer = true;
		hsBool isLoading = false;
		plLoadAvatarMsg *msg = TRACKED_NEW plLoadAvatarMsg(playerKey, mgrKey, 0, isPlayer, isLoading);
		msg->Send();
	}
}

// MSGRECEIVE
hsBool plAvatarMgr::MsgReceive(plMessage *msg)
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
		fCloneMsgQueue.Append(pCloneMsg);
		plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
		return true;
	}

	plEvalMsg* pEval = plEvalMsg::ConvertNoRef(msg);
	if (pEval)
	{
		for (int i = fCloneMsgQueue.Count() - 1; i > -1; i--)
		{
			plArmatureMod* pAvatar = FindAvatarByPlayerID(fCloneMsgQueue[i]->GetUserData());
			if (pAvatar && pAvatar->GetKey()->ObjectIsLoaded())
			{	
				pAvatar->MsgReceive(fCloneMsgQueue[i]);
				fCloneMsgQueue[i]->UnRef();
				fCloneMsgQueue.Remove(i);
			}
		}
		if (fCloneMsgQueue.Count() == 0)
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

hsBool plAvatarMgr::HandleCoopMsg(plAvCoopMsg *msg)
{
	plAvCoopMsg::Command cmd = msg->fCommand;
	
	UInt32 id = msg->fInitiatorID;
	UInt16 serial = msg->fInitiatorSerial;

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

hsBool plAvatarMgr::HandleNotifyMsg(plNotifyMsg *msg)
{
	proCoopEventData *ed = static_cast<proCoopEventData *>(msg->FindEventRecord(proEventData::kCoop));
	if(ed)
	{
		UInt32 id = ed->fID;
		UInt16 serial = ed->fSerial;
		return IPassMessageToActiveCoop(msg, id, serial);
	}
	return false;
}

hsBool plAvatarMgr::IPassMessageToActiveCoop(plMessage *msg, UInt32 id, UInt16 serial)
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
	cloneMsg->ClearReceivers();		// don't want it coming back to us
	cloneMsg->Ref();				// or going away

	if(armature)
	{
		cloneMsg->AddReceiver(armature->GetKey());
		cloneMsg->Send();
	} else {
		IDeferInit(avatarKey, cloneMsg);		// we'll send this message when the armature mod loads.
	}

	if( cloneMsg->GetIsPlayer() )
	{
		// notify everyone who cares that a new player has arrived
		// *** might want to move this to the human brain so we can make sure the 
		// *** avatar is sufficiently initialized before anyone accesses him
		bool isLocal = cloneMsg->GetOriginatingPlayerID() == plNetClientMgr::GetInstance()->GetPlayerID();
		plPlayerPageMsg* pageM = TRACKED_NEW plPlayerPageMsg;
		pageM->SetBCastFlag(plMessage::kBCastByExactType);
		pageM->fLocallyOriginated = isLocal;
		pageM->fPlayer = avatarKey;
		pageM->fUnload = false;
		pageM->fClientID = cloneMsg->GetOriginatingPlayerID();
		pageM->Send();
	}

	// This can probably be replaced by the plPlayerPageMsg:
	// ...keeping for the moment for compatibility
	plMemberUpdateMsg* mu = TRACKED_NEW plMemberUpdateMsg;
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
		plPlayerPageMsg *pageM = TRACKED_NEW plPlayerPageMsg;
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
	plMemberUpdateMsg *mu = TRACKED_NEW plMemberUpdateMsg;
	mu->Send();
}


// IDEFERINIT
void plAvatarMgr::IDeferInit(plKey playerSOKey, plMessage *initMsg)
{
	plMessage *existing = fDeferredInits[playerSOKey];		// okay to use this form because we're going
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
void plAvatarMgr::ISendDeferredInit(plKey avatarSOKey)
{
	// get armaturemod
	const plArmatureMod * armature = FindAvatar(avatarSOKey);

	if(armature)
	{
		DeferredInits::iterator i = fDeferredInits.find(avatarSOKey);
		bool found = (i != fDeferredInits.end());

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
		const char *name = seekPoint->GetTarget(0)->GetKey()->GetName();
		char *ourName = hsStrcpy(name);
		plSeekPointMod *alreadyThere = FindSeekPoint(name);
		
		/// hsAssert( ! alreadyThere, "Tried to add a seek point with duplicate name. Ignoring second seek point.");

		if ( ! alreadyThere)
		{
			fSeekPoints[ourName] = seekPoint;
		}
	}
}

// REMOVESEEKPOINT
void plAvatarMgr::RemoveSeekPoint(plSeekPointMod *seekPoint)
{
	if(seekPoint)
	{
		const char *name = seekPoint->GetTarget(0)->GetKey()->GetName();

		plSeekPointMap::iterator found = fSeekPoints.find(name);

		if(found != fSeekPoints.end())
		{
			const char *oldName = (*found).first;
			fSeekPoints.erase(found);
			delete[] const_cast<char *>(oldName);	// retarded language, this is...
		}
	}
}

// FINDSEEKPOINT
plSeekPointMod * plAvatarMgr::FindSeekPoint(const char *name)
{
	plSeekPointMap::iterator found = fSeekPoints.find(name);
	
	if (found == fSeekPoints.end())
	{
		return nil;
	} else {
		return (*found).second;
	}
}

// ADDONESHOT
void plAvatarMgr::AddOneShot(plOneShotMod *oneshot)
{
	if(oneshot)
	{
		// allocate a copy of the target name to use as a key
		char * name = hsStrcpy(oneshot->GetTarget(0)->GetKey()->GetName());
		plOneShotMod *alreadyThere = FindOneShot(name);
		

		if ( ! alreadyThere)
		{
			fOneShots[name] = oneshot;
		}
		else
			delete [] name;
	}
}

// REMOVEONESHOT
void plAvatarMgr::RemoveOneShot(plOneShotMod *oneshot)
{
	plOneShotMap::iterator i = fOneShots.begin();

	while (i != fOneShots.end())
	{
		char * name = (*i).first;
		plOneShotMod *thisOneshot = (*i).second;

		if(oneshot == thisOneshot)
		{
			i = fOneShots.erase(i);
			// destroy our copy of the target name
			delete[] name;
		} else {
			i++;
		}
	}
}

// FINDONESHOT
plOneShotMod *plAvatarMgr::FindOneShot(char *name)
{
	plOneShotMap::iterator found = fOneShots.find(name);
	
	if (found == fOneShots.end())
	{
		return nil;
	} else {
		return (*found).second;
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
		plKey soKey = avatarSO->GetKey();
		ISendDeferredInit(soKey);
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

	return nil;
}

plKey plAvatarMgr::GetLocalAvatarKey()
{
	plArmatureMod *avatar = GetLocalAvatar();
	if (avatar)
		return avatar->GetKey();

	return nil;
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
	
	return nil;
}

plArmatureMod* plAvatarMgr::FindAvatar(plKey& avatarKey)
{
	plSceneObject *so = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
	if (so)
		return const_cast<plArmatureMod*>((plArmatureMod*)so->GetModifierByType(plArmatureMod::Index()));
	
	return nil;
}

plArmatureMod* plAvatarMgr::FindAvatarByPlayerID(UInt32 pid)
{
	plAvatarVec::iterator it;
	for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
	{
		plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
		if (armature && (armature->GetKey()->GetUoid().GetClonePlayerID() == pid))
			return armature;
	}
	return nil;
}

plArmatureMod *plAvatarMgr::FindAvatarByModelName(char *name)
{
	plAvatarVec::iterator it;
	for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
	{
		plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
		if (armature && (!strcmp(armature->GetTarget(0)->GetKeyName(), name)))
			return armature;
	}
	
	return nil;
}

void plAvatarMgr::FindAllAvatarsByModelName(const char* name, plArmatureModPtrVec& outVec)
{
	plAvatarVec::iterator it;
	for (it = fAvatars.begin(); it != fAvatars.end(); ++it)
	{
		plArmatureMod* armature = plArmatureMod::ConvertNoRef((*it)->ObjectIsLoaded());
		if (armature && (!strcmp(armature->GetTarget(0)->GetKeyName(), name)))
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
	} else return nil;
}

int	plAvatarMgr::FindSpawnPoint( const char *name ) const
{
	int i;

	for( i = 0; i < fSpawnPoints.size(); i++ )
	{
		if( fSpawnPoints[ i ] != nil && 
			(strstr( fSpawnPoints[ i ]->GetKey()->GetUoid().GetObjectName(), name ) != nil ||
			 strstr( fSpawnPoints[i]->GetTarget(0)->GetKeyName(), name) != nil))
			return i;
	}

	return -1;
}

int plAvatarMgr::WarpPlayerToAnother(hsBool iMove, UInt32 remoteID)
{
	plNetTransport &mgr = plNetClientMgr::GetInstance()->TransportMgr();
	plNetTransportMember *mbr = mgr.GetMember(mgr.FindMember(remoteID));

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

	plWarpMsg *warp = TRACKED_NEW plWarpMsg(nil, (iMove ? localSO->GetKey() : remoteSO->GetKey()), 
		plWarpMsg::kFlushTransform, (iMove ? remoteSO->GetLocalToWorld() : localSO->GetLocalToWorld()));
	
	warp->SetBCastFlag(plMessage::kNetPropagate);
	plgDispatch::MsgSend(warp);

	return hsOK;
}

int plAvatarMgr::WarpPlayerToXYZ(hsScalar x, hsScalar y, hsScalar z)
{
	plSceneObject *localSO = plSceneObject::ConvertNoRef(plNetClientMgr::GetInstance()->GetLocalPlayer());
	if (!localSO)
		return plCCRError::kNilLocalAvatar;

	hsMatrix44 m = localSO->GetLocalToWorld();
	hsVector3 v(x, y, z);
	m.SetTranslate(&v);

	plWarpMsg *warp = TRACKED_NEW plWarpMsg(nil, localSO->GetKey(), plWarpMsg::kFlushTransform, m);
	warp->SetBCastFlag(plMessage::kNetPropagate);
	plgDispatch::MsgSend(warp);

	return hsOK;
}

int plAvatarMgr::WarpPlayerToXYZ(int pid, hsScalar x, hsScalar y, hsScalar z)
{
	plNetClientMgr* nc=plNetClientMgr::GetInstance();
	plNetTransportMember* mbr=nc->TransportMgr().GetMember(nc->TransportMgr().FindMember(pid));
	plSceneObject *player = plSceneObject::ConvertNoRef(mbr && mbr->GetAvatarKey() ? 
		mbr->GetAvatarKey()->ObjectIsLoaded() : nil);
	if (!player)
		return plCCRError::kNilLocalAvatar;

	hsMatrix44 m = player->GetLocalToWorld();
	hsVector3 v(x, y, z);
	m.SetTranslate(&v);

	plWarpMsg *warp = TRACKED_NEW plWarpMsg(nil, player->GetKey(), 0, m);
	warp->SetBCastFlag(plMessage::kNetPropagate);
	plgDispatch::MsgSend(warp);

	return hsOK;
}

// ADD maintainers marker
void plAvatarMgr::AddMaintainersMarker(plMaintainersMarkerModifier *mm)
{
	fMaintainersMarkers.Append(mm);
}

// REMOVE maintainers marker
void plAvatarMgr::RemoveMaintainersMarker(plMaintainersMarkerModifier *mm)
{
	for (int i = 0; i < fMaintainersMarkers.Count(); i++)
	{
		if (fMaintainersMarkers[i] == mm)
			fMaintainersMarkers.Remove(i);
	}
}

void plAvatarMgr::PointToDniCoordinate(hsPoint3 pt, plDniCoordinateInfo* ret)
{
	int count = fMaintainersMarkers.Count();
	//	plDniCoordinateInfo ret = TRACKED_NEW plDniCoordinateInfo;
	if (count > 0)
	{	

		// find the closest maintainers marker
		int nearestIndex = 0;
		if (count > 1)
		{
			for (int i = 0; i < fMaintainersMarkers.Count(); i++)
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

				hsScalar dotView = retVec * zeroVec;
				hsScalar dotRight = retVec * zeroRight;

				hsScalar deg = acosf(dotView);
				deg*=(180/3.141592);
				// account for being > 180
				if (dotRight < 0.0f) 
				{
					deg = 360.f - deg;
				}
				// convert it to dni radians (torans)
				deg*=173.61;
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
void plAvatarMgr::OfferLinkingBook(plKey hostKey, plKey guestKey, plMessage *linkMsg, plKey replyKey)
{
	if(hostKey != nil && guestKey != nil)
	{
		const plArmatureMod *hostAv = FindAvatar(hostKey);
		const plArmatureMod *guestAv = FindAvatar(guestKey);

		hsAssert(hostAv && guestAv, "Offering linking book: host or guest missing.");

		if(hostAv && guestAv)
		{

			// make the host brain
			plAvBrainCoop * brainH = TRACKED_NEW plAvBrainCoop(plAvBrainGeneric::kExitNormal, 3.0, 3.0, plAvBrainGeneric::kMoveRelative, guestKey);

			plAnimStage *hostOffer = TRACKED_NEW plAnimStage("BookOffer", plAnimStage::kNotifyAdvance);		// autoforward, autoadvance
			// repeats until the guest brain tells us that it's done
			plAnimStage *hostIdle = TRACKED_NEW plAnimStage("BookOfferIdle", plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
													plAnimStage::kAdvanceNone, plAnimStage::kRegressNone, -1);

			plAnimStage *hostFinish = TRACKED_NEW plAnimStage("BookOfferFinish", plAnimStage::kNotifyAdvance);	// autoforward, autoadvance
			
			brainH->AddStage(hostOffer);
			brainH->AddStage(hostIdle);
			brainH->AddStage(hostFinish);

			UInt32 hostID = brainH->GetInitiatorID();
			UInt32 hostSerial = brainH->GetInitiatorSerial();


			// make the guest brain
			plAvBrainCoop * brainG = TRACKED_NEW plAvBrainCoop(plAvBrainGeneric::kExitNormal, 3.0, 3.0, plAvBrainGeneric::kMoveRelative,
													   hostID, (UInt16)hostSerial, hostKey);

			plAnimStage *guestAccept = TRACKED_NEW plAnimStage("BookAccept", plAnimStage::kNotifyAdvance);
			plAnimStage *guestAcceptIdle = TRACKED_NEW plAnimStage("BookAcceptIdle", plAnimStage::kNotifyEnter, plAnimStage::kForwardAuto, plAnimStage::kBackNone,
														   plAnimStage::kAdvanceNone, plAnimStage::kRegressNone, -1);
			
			brainG->AddStage(guestAccept);
			brainG->AddStage(guestAcceptIdle);
			plCoopCoordinator *coord = TRACKED_NEW plCoopCoordinator(hostKey, guestKey, brainH, brainG, "Convergence", 1, 1, linkMsg, true);


			plAvCoopMsg *coMg = TRACKED_NEW plAvCoopMsg(hostKey, coord);
			coMg->SetBCastFlag(plMessage::kNetPropagate);
			coMg->SetBCastFlag(plMessage::kNetForce);

			coMg->Send();
			brainH->SetRecipient(replyKey);
			brainG->SetRecipient(replyKey);

		}
	}
}
