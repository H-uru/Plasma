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
/** \file plAvatarMgr.h
    Gathering place for global animations and miscellaneous avatar data.
    Ideally this stuff will all be migrated into the resource manager. */
#ifndef PLAVATARMGR_INC
#define PLAVATARMGR_INC

#include "HeadSpin.h"
#include <map>
#include <vector>

#include "plFileSystem.h"
#include "hsGeometry3.h"

#include "pnKeyedObject/hsKeyedObject.h"

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
class plAvTask;

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

    plAvatarMgr();                      // can only be constructed by itself (singleton)
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
    plSeekPointMod *FindSeekPoint(const ST::string &name);
    // \}

    // \{
    /** One shots are registered here for debugging and ad-hoc
        scripting only. */
    void AddOneShot(plOneShotMod *oneshot);
    void RemoveOneShot(plOneShotMod *oneshot);
    plOneShotMod *FindOneShot(const ST::string &name);
    // \}
    
    plKey LoadPlayer(const ST::string &name, const ST::string &account);
    plKey LoadPlayer(const ST::string &name, const ST::string &account, const ST::string &linkName);
    plKey LoadPlayerFromFile(const ST::string &name, const ST::string &account, const plFileName &clothingFile);
    plKey LoadAvatar(ST::string name, const ST::string &accountName, bool isPlayer, const plKey& spawnPoint, plAvTask *initialTask,
                     const ST::string &userStr = {}, const plFileName &clothingFile = {});

    /**
     * Unload an avatar clone
     *
     * This unloads the clone of an avatar (remote player or NPC) from our local game.
     * The avatar clone can be unloaded globally by setting netPropagate; however, this
     * is highly discouraged.
     */
    void UnLoadAvatar(const plKey& avKey, bool isPlayer, bool netPropagate=false) const;
    /** send our (already loaded) local player to newly-associated clients - used when linking */
    void PropagateLocalPlayer(int spawnPoint = -1);
    /** Unload our local player on other machines because we're leaving this age.
        The player will stay around on our local machine, though. */
    bool UnPropagateLocalPlayer();

    void UnLoadLocalPlayer();

    void AddAvatar(plArmatureMod *avatar);
    void RemoveAvatar(plArmatureMod *instance);

    plArmatureMod *GetLocalAvatar();
    plKey GetLocalAvatarKey();
    static plArmatureMod *FindAvatar(const plKey& avatarKey); // Key of the sceneObject
    plArmatureMod *FindAvatarByPlayerID(uint32_t pid);
    plArmatureMod *FindAvatarByModelName(const ST::string& name); // Probably only useful for custom NPCs. All players are
                                                      // either "Male" or "Female".
    void FindAllAvatarsByModelName(const ST::string& name, plArmatureModPtrVec& outVec);
    plArmatureMod *GetFirstRemoteAvatar();

    // \{
    /** Spawn points are potential entry points for the 
        avatar. They're selected pretty randomly right now;
        eventually they'll be selected by script based
        on the book used to enter the scene. */
    void AddSpawnPoint(plSpawnModifier *spawn);
    void RemoveSpawnPoint(plSpawnModifier *spawn);
    const plSpawnModifier *GetSpawnPoint(int index);
    int NumSpawnPoints() { return fSpawnPoints.size(); }
    int FindSpawnPoint( const char *name ) const;
    // \}
    static bool WarpPlayerToAnother(bool iMove, uint32_t remoteID);
    static bool WarpPlayerToXYZ(float x, float y, float z);
    static bool WarpPlayerToXYZ(int pid, float x, float y, float z);

    static plAvatarMgr *GetInstance();
    static void ShutDown();


    bool MsgReceive(plMessage *msg) override;
    bool HandleCoopMsg(plAvCoopMsg *msg);
    bool HandleNotifyMsg(plNotifyMsg *msg);
    bool IPassMessageToActiveCoop(plMessage *msg, uint32_t id, uint16_t serial);

    // similar to a spawn point, maintainers markers are used 
    // to generate your position in Dni coordinates
    void AddMaintainersMarker(plMaintainersMarkerModifier *mm);
    void RemoveMaintainersMarker(plMaintainersMarkerModifier *mm);
    void PointToDniCoordinate(hsPoint3 pt, plDniCoordinateInfo* ret);
    void GetDniCoordinate(plDniCoordinateInfo* ret);

    static void OfferLinkingBook(const plKey& hostKey, const plKey& guestKey, plMessage *linkMsg, const plKey& replyKey);

    bool IsACoopRunning();
    plStatusLog *GetLog() { return fLog; }

protected:
    /** Dump all internal data. */
    void IReset();
    
    /** Handle an incoming clone message; do any necessary post-processing
        on the avatar. */
    void IFinishLoadingAvatar(plLoadAvatarMsg *cloneMsg);

    /** Handle an incoming clone message which holds an unload request.
    */
    void IFinishUnloadingAvatar(plLoadAvatarMsg *cloneMsg);
    
    /** When an armature modifier attached to the given scene object is loaded,
        send it the given message.
        We get notified when the avatar's scene object is loaded, but we also need to 
        set some information up for the avatar modifier when it comes in.
        We'll get that notification via the AddAvatar call later. In this function
        we're going to squirrel away an initialization message to pass to the armature
        modifier when it arrives. */
    void IDeferInit(const plKey& playerSOKey, plMessage *initMsg);
    
    /** See if we have an avatar type message saved for the given avatar and send them. */
    void ISendDeferredInit(const plKey& playerSOKey);

    static plAvatarMgr* fInstance;      // the single instance of the avatar manager

    typedef std::map<ST::string, plSeekPointMod *, ST::less_i> plSeekPointMap;
    plSeekPointMap fSeekPoints;

    typedef std::map<ST::string, plOneShotMod *, ST::less_i> plOneShotMap;
    plOneShotMap fOneShots;

    typedef std::map<plKey, plMessage *> DeferredInits;
    DeferredInits fDeferredInits;

//  typedef std::map<const char *, plArmatureMod *, stringISorter> plAvatarMap;
    typedef std::vector<plKey> plAvatarVec;
    plAvatarVec fAvatars;

    typedef std::vector<const plSpawnModifier*> plSpawnVec;
    plSpawnVec  fSpawnPoints;

    std::vector<plMaintainersMarkerModifier*> fMaintainersMarkers;

    // we're using a multimap, which is a map which allows multiple entries to
    // share the same key. the key we use is the initiator's player id; in the vast
    // majority of cases, there will only be one coop running for a given initiator's
    // ID. By using a multimap, however, we can still handle a few different coops
    // for the same user by just iterating from the first match forward until
    // we run out of matches.
    typedef std::multimap<uint32_t, plCoopCoordinator *> plCoopMap;
    plCoopMap fActiveCoops;

    std::vector<plLoadCloneMsg*> fCloneMsgQueue;
    plStatusLog *fLog;  
};


#endif // PLAVATARMGR_INC

