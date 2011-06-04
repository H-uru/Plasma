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
/** \file plAvatarMgr.h
	Gathering place for global animations and miscellaneous avatar data.
	Ideally this stuff will all be migrated into the resource manager. */
#ifndef PLAVATARMGR_INC
#define PLAVATARMGR_INC

#include "hsStlUtils.h"
#include "hsStlSortUtils.h"
#include "hsGeometry3.h"

#include "../pnKeyedObject/hsKeyedObject.h"
#include "../plMessage/plLoadAvatarMsg.h"

// This is still pretty much a hack, but it's a compartmentalized hack instead of the previous
// interwoven spaghetti hack.

class plSeekPointMod;
class plOneShotMod;
class plAGMasterMod;
class plArmatureMod;
class plSpawnModifier;
class plKey;
class plLoadAvatarMsg;
class plMaintainersMarkerModifier;
class plDniCoordinateInfo;
class plAvCoopMsg;
class plNotifyMsg;
class plCoopCoordinator;
class plLoadCloneMsg;
class plStatusLog;

/** \class plAvatarMgr
	Gathering place for global animations and miscellaneous avatar data.
	Ideally this stuff will all be migrated into the resource manager.
	This class is 100% static and must be explictly cleared when
	resetting the scene on shutdown or when rebuilding the scene for export.
*/
class plAvatarMgr : public hsKeyedObject
{
public:
	typedef std::vector<plArmatureMod*> plArmatureModPtrVec;

	enum AvatarTypeMask
	{
		Human = 1,
		Player = 2
	};

	plAvatarMgr();						// can only be constructed by itself (singleton)
	virtual ~plAvatarMgr();

	CLASSNAME_REGISTER( plAvatarMgr );
	GETINTERFACE_ANY( plAvatarMgr, hsKeyedObject );

	// \{
	/** Seek points are alignment points used for aligning
		the avatar before playing a detail interaction animation.
		These are registered by name here primarily for debugging
		and ad-hoc scripting. In final releases, we'll be able to
		do away with this bookeeping entirely. */
	void AddSeekPoint(plSeekPointMod *seekpoint);
	void RemoveSeekPoint(plSeekPointMod *seekpoint);
	plSeekPointMod *FindSeekPoint(const char *name);
	// \}

	// \{
	/** One shots are registered here for debugging and ad-hoc
		scripting only. */
	void AddOneShot(plOneShotMod *oneshot);
	void RemoveOneShot(plOneShotMod *oneshot);
	plOneShotMod *FindOneShot(char *name);
	// \}
	
	plKey LoadPlayer(const char* name, const char *account);
	plKey LoadPlayer(const char* name, const char *account, const char *linkName);
	plKey LoadAvatar(const char *name, const char *accountName, bool isPlayer, plKey spawnPoint, plAvTask *initialTask, const char *userStr = nil);
	/** Unload an avatar - player or npc - both locally and remotely. */
	void UnLoadAvatar(plKey avKey, bool isPlayer);
	/** send our (already loaded) local player to newly-associated clients - used when linking */
	void PropagateLocalPlayer(int spawnPoint = -1);
	/** Unload our local player on other machines because we're leaving this age.
		The player will stay around on our local machine, though. */
	bool UnPropagateLocalPlayer();

	void UnLoadRemotePlayer(plKey playerKey);
	void UnLoadLocalPlayer();

	void AddAvatar(plArmatureMod *avatar);
	void RemoveAvatar(plArmatureMod *instance);

	plArmatureMod *GetLocalAvatar();
	plKey GetLocalAvatarKey();
	static plArmatureMod *FindAvatar(plKey& avatarKey); // Key of the sceneObject
	plArmatureMod *FindAvatarByPlayerID(UInt32 pid);
	plArmatureMod *FindAvatarByModelName(char *name); // Probably only useful for custom NPCs. All players are
													  // either "Male" or "Female".
	void FindAllAvatarsByModelName(const char* name, plArmatureModPtrVec& outVec);
	plArmatureMod *GetFirstRemoteAvatar();

	// \{
	/** Spawn points are potential entry points for the 
		avatar. They're selected pretty randomly right now;
		eventually they'll be selected by script based
		on the book used to enter the scene. */
	void AddSpawnPoint(plSpawnModifier *spawn);
	void RemoveSpawnPoint(plSpawnModifier *spawn);
	const plSpawnModifier *GetSpawnPoint(int index);
	int	NumSpawnPoints() { return fSpawnPoints.size(); }
	int	FindSpawnPoint( const char *name ) const;
	// \}
	static int WarpPlayerToAnother(hsBool iMove, UInt32 remoteID);
	static int WarpPlayerToXYZ(hsScalar x, hsScalar y, hsScalar z);
	static int WarpPlayerToXYZ(int pid, hsScalar x, hsScalar y, hsScalar z);

	static plAvatarMgr *GetInstance();
	static void ShutDown();


	hsBool MsgReceive(plMessage *msg);
	hsBool HandleCoopMsg(plAvCoopMsg *msg);
	hsBool HandleNotifyMsg(plNotifyMsg *msg);
	hsBool IPassMessageToActiveCoop(plMessage *msg, UInt32 id, UInt16 serial);

	// similar to a spawn point, maintainers markers are used 
	// to generate your position in Dni coordinates
	void AddMaintainersMarker(plMaintainersMarkerModifier *mm);
	void RemoveMaintainersMarker(plMaintainersMarkerModifier *mm);
	void PointToDniCoordinate(hsPoint3 pt, plDniCoordinateInfo* ret);
	void GetDniCoordinate(plDniCoordinateInfo* ret);

	static void OfferLinkingBook(plKey hostKey, plKey guestKey, plMessage *linkMsg, plKey replyKey);

	bool IsACoopRunning();
	plStatusLog *GetLog() { return fLog; }

protected:
	/** Dump all internal data. */
	void IReset();
	
	/** Handle an incoming clone message; do any necessary post-processing
		on the avatar. */
	void plAvatarMgr::IFinishLoadingAvatar(plLoadAvatarMsg *cloneMsg);

	/** Handle an incoming clone message which holds an unload request.
	*/
	void plAvatarMgr::IFinishUnloadingAvatar(plLoadAvatarMsg *cloneMsg);
	
	/** When an armature modifier attached to the given scene object is loaded,
		send it the given message.
		We get notified when the avatar's scene object is loaded, but we also need to 
		set some information up for the avatar modifier when it comes in.
		We'll get that notification via the AddAvatar call later. In this function
		we're going to squirrel away an initialization message to pass to the armature
		modifier when it arrives. */
	void plAvatarMgr::IDeferInit(plKey playerSOKey, plMessage *initMsg);
	
	/** See if we have an avatar type message saved for the given avatar and send them. */
	void plAvatarMgr::ISendDeferredInit(plKey playerSOKey);

	static plAvatarMgr*	fInstance;		// the single instance of the avatar manager

	typedef std::map<const char *, plSeekPointMod *, stringISorter> plSeekPointMap;
	plSeekPointMap fSeekPoints;

	typedef std::map<char *, plOneShotMod *, stringISorter> plOneShotMap;
	plOneShotMap fOneShots;

	typedef std::map<plKey, plMessage *> DeferredInits;
	DeferredInits fDeferredInits;

//	typedef std::map<const char *, plArmatureMod *, stringISorter> plAvatarMap;
	typedef std::vector<plKey> plAvatarVec;
	plAvatarVec fAvatars;

	typedef std::vector<const plSpawnModifier*> plSpawnVec;
	plSpawnVec	fSpawnPoints;

	hsTArray<plMaintainersMarkerModifier*> fMaintainersMarkers;

	// we're using a multimap, which is a map which allows multiple entries to
	// share the same key. the key we use is the initiator's player id; in the vast
	// majority of cases, there will only be one coop running for a given initiator's
	// ID. By using a multimap, however, we can still handle a few different coops
	// for the same user by just iterating from the first match forward until
	// we run out of matches.
	typedef std::multimap<UInt32, plCoopCoordinator *> plCoopMap;
	plCoopMap fActiveCoops;

	hsTArray<plLoadCloneMsg*> fCloneMsgQueue;
	plStatusLog *fLog;	
};


#endif // PLAVATARMGR_INC

