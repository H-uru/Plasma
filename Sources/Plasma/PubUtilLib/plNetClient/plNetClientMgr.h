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
#ifndef PL_NET_CLIENT_inc
#define PL_NET_CLIENT_inc

#include "HeadSpin.h"
#include "hsStlUtils.h"

#include "plNetClientGroup.h"
#include "plNetVoiceList.h"
#include "plNetClientMsgHandler.h"
#include "plNetClientMsgScreener.h"

#include "pnNetCommon/plNetApp.h"

#include "plNetTransport/plNetTransport.h"
#include "pnEncryption/plChecksum.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetClientComm/plNetClientComm.h"
#include "plUnifiedTime/plUnifiedTime.h"
#pragma warning(disable: 4284)

////////////////////////////////////////////////////////////////////

class plUoid;
class hsStream;
class plKey;
class plNetMessage;
class plSynchedObject;
struct DistSqInfo;
class plStatusLog;
class plOperationProgress;
class plIDataServer;
class plPlate;
class plLoadCloneMsg;
class plPlayerPageMsg;
class plNetClientRecorder;
class plVaultPlayerNode;
class plVaultAgeNode;
class plNetVoiceListMsg;
class plStateDataRecord;
class plCCRPetitionMsg;
class plNetMsgPagingRoom;


struct plNetClientCommMsgHandler : plNetClientComm::MsgHandler {
    int HandleMessage( plNetMessage* msg ); 
};

class plNetClientMgr : public plNetClientApp
{
private:
    typedef std::vector<plKey> plKeyVec;
public:

    enum NetChannels
    {
        kNetChanDefault,
        kNetChanVoice,
        kNetChanListenListUpdate,
        kNetChanDirectedMsg,
        kNetNumChannels
    };

    enum DirectedSendFlags
    {
        kInterAgeMsg = 0x1
    };
    
    enum ListenListMode
    {
        kListenList_Distance    = 0,
        kListenList_Forced,
        kListenList_End,
    };

    enum RefContext
    {
        kVaultImage = 0,
        kAgeSDLHook = 1
    };

    struct PendingLoad
    {
        // must be set by user
        plStateDataRecord* fSDRec;  // the sdl data record
        plUoid  fUoid;              // the object it's meant for
        uint32_t  fPlayerID;          // the player that originally sent the state

        // set by NetClient
        plKey fKey;                 // the key of the object it's meant for
        double fQueuedTime;
        int     fQueueTimeResets;

        PendingLoad() : fSDRec(nil),fPlayerID(0),fKey(nil),fQueuedTime(0.0),fQueueTimeResets(0) {}
        ~PendingLoad();
    };

private:
    // plOperationProgress  *fProgressBar;
    plOperationProgress *fTaskProgBar;
    
    typedef std::list<PendingLoad*> PendingLoadsList;
    PendingLoadsList fPendingLoads;
            
    // pending room page msgs
    std::vector<plNetMsgPagingRoom*>    fPendingPagingRoomMsgs;

    plNetTransport fTransport;

    // groups of objects in the game.  Each group is mastered by a single client.
    plNetClientGroups       fNetGroups;

    // cached char info
    plKey       fLocalPlayerKey;
    plKeyVec    fRemotePlayerKeys;
    plKeyVec    fNPCKeys;
    
    class plNetClientMgrMsg *   fDisableMsg;

    // ini info
    std::string         fIniAccountName;
    std::string         fIniAccountPass;
    std::string         fIniAuthServer;
    uint32_t              fIniPlayerID;   // the player we want to load from vault.
    std::string         fSPDesiredPlayerName;   // SP: the player we want to load from vault.
    
    // server info
    double              fServerTimeOffset;      // diff between our unified time and server's unified time
    uint32_t              fTimeSamples;
    double              fLastTimeUpdate;

    uint8_t               fJoinOrder;         // returned by the server
    
    // voice lists
    int     fListenListMode;            // how we are generating our listen list    
    plNetListenList fListenList;        // other players I'm listening to
    plNetTalkList fTalkList;            // other players I'm talking to

    plNetClientMsgHandler   fMsgHandler;
    plNetClientMsgScreener  fScreener;

    // recorder support
    plNetClientRecorder* fMsgRecorder;
    std::vector<plNetClientRecorder*> fMsgPlayers;

    plKey   fAgeSDLObjectKey;
    uint8_t fExperimentalLevel;
    uint8_t   fPingServerType;        // non-zero if we're pinging someone
    float   fOverrideAgeTimeOfDayPercent;   // for console debugging

    int fNumInitialSDLStates;
    int fRequiredNumInitialSDLStates;

    // simplification of object ownership...one player owns all non-physical objects in the world
    // physical objects are owned by whoever touched them most recently (or the "owner" if nobody
    // has touched it yet)
    bool fIsOwner;

    //
    void ICheckPendingStateLoad(double secs);
    int IDeduceLocallyOwned(const plUoid& loc) const;
    bool IHandlePlayerPageMsg(plPlayerPageMsg *playerMsg);  // *** 

    void IShowLists();
    void IShowRooms();
    void IShowAvatars();
    void IShowRelevanceRegions();
    
    int ISendDirtyState(double secs);
    int ISendMembersListRequest();
    int ISendRoomsReset();
    void ISendCCRPetition(plCCRPetitionMsg* petMsg);    
    void ISendCameraReset(bool bEnteringAge);
    
    bool IUpdateListenList(double secs);
    void IHandleNetVoiceListMsg(plNetVoiceListMsg* msg);
    bool IApplyNewListenList(std::vector<DistSqInfo>& newListenList, bool forceSynch);
    int IPrepMsg(plNetMessage* msg);
    void IPlayerChangeAge(bool exiting, int32_t spawnPt);   
    
    void IAddCloneRoom();
    void IRemoveCloneRoom();

    void IUnloadRemotePlayers();
    void IUnloadNPCs();
    
    plKey ILoadClone(plLoadCloneMsg *cloneMsg);

    bool IFindModifier(plSynchedObject* obj, int16_t classIdx);
    void IClearPendingLoads();

    // recorder
    bool IIsRecordableMsg(plNetMessage* msg);
    void IPlaybackMsgs();

    void IRequestAgeState();

    void    IDumpOSVersionInfo() const;

    int ISendGameMessage(plMessage* msg);
    void IDisableNet ();

public:
    plNetClientMgr();
    ~plNetClientMgr();

    CLASSNAME_REGISTER( plNetClientMgr );
    GETINTERFACE_ANY( plNetClientMgr, plNetClientApp );

    static plNetClientMgr* GetInstance() { return plNetClientMgr::ConvertNoRef(fInstance); }

    void StartLinkOutFX();
    void StartLinkInFX();

    bool MsgReceive(plMessage* msg);
    void Shutdown();
    int  Init();

    void QueueDisableNet (bool showDlg, const char msg[]);

    int SendMsg(plNetMessage* msg);
    int Update(double secs);
    int IsLocallyOwned(const plSynchedObject* obj) const;   // returns yes/no/maybe
    int IsLocallyOwned(const plUoid&) const;        // for special cases, like sceneNodes. returns yes/no/maybe 
    plNetGroupId GetEffectiveNetGroup(const plSynchedObject*& obj) const;
    plNetGroupId SelectNetGroup(plSynchedObject* objIn, plKey groupKey);

    void SendLocalPlayerAvatarCustomizations();
    void SendApplyAvatarCustomizationsMsg(const plKey msgReceiver, bool netPropagate=true, bool localPropagate=true);

    // plLoggable
    bool Log(const char* str) const;

    // setters
    void SetIniAuthServer(const char * value)  { fIniAuthServer=value;}
    void SetIniAccountName(const char * value) { fIniAccountName=value;}
    void SetIniAccountPass(const char * value) { fIniAccountPass=value;}
    void SetIniPlayerID(uint32_t value)          { fIniPlayerID=value;}

    void SetSPDesiredPlayerName( const char * value ) { fSPDesiredPlayerName=value;}
    const char * GetSPDesiredPlayerName() const { return fSPDesiredPlayerName.c_str(); }

    void SetLocalPlayerKey(plKey l, bool pageOut=false);
    void SetNullSend(bool on);        // turn null send on/off
    void SetPingServer(uint8_t serverType) { fPingServerType = serverType; }
    
    // getters
    uint32_t            GetPlayerID( void ) const;
    plString            GetPlayerName( const plKey avKey=nil ) const;
    plString            GetPlayerNameById (unsigned playerId) const;
    unsigned            GetPlayerIdByName(const plString & name) const;

    uint8_t GetJoinOrder()              const { return fJoinOrder; }    // only valid at join time

    plKey GetLocalPlayerKey()           const { return fLocalPlayerKey; }
    plSynchedObject* GetLocalPlayer(bool forceLoad=false) const;
    
    bool IsPeerToPeer()               const { return false; }
    bool IsConnected()                const { return true; }

    void IncNumInitialSDLStates();
    void ResetNumInitialSDLStates() { fNumInitialSDLStates=0; }
    int GetNumInitialSDLStates() const { return fNumInitialSDLStates; }
    void SetRequiredNumInitialSDLStates( int v ) { fRequiredNumInitialSDLStates=v; }
    int GetRequiredNumInitialSDLStates() const { return fRequiredNumInitialSDLStates; }

    // Linking progress
    void    StartTaskProgress( const char *msg, int numSteps );
    void    IncTaskProgress( const char *msg );

    // avatar vault actions
    int UploadPlayerVault(uint32_t vaultFlags);

    // npc clones
    const plKeyVec& NPCKeys() const { return fNPCKeys; }
    plSynchedObject* GetNPC(uint32_t i) const;
    void AddNPCKey(const plKey& npc);
    bool IsNPCKey(const plKey& npc, int* idx=nil) const;
    
    // remote players
    const plKeyVec& RemotePlayerKeys() const { return fRemotePlayerKeys;  }
    plSynchedObject* GetRemotePlayer(int i) const;
    void AddRemotePlayerKey(plKey p);
    bool IsRemotePlayerKey(const plKey p, int* idx=nil);
    bool IsAPlayerKey(const plKey pKey) { return (pKey==GetLocalPlayerKey() || IsRemotePlayerKey(pKey));    }

    void SetConsoleOutput( bool b ) { SetFlagsBit(kConsoleOutput, b); }
    bool GetConsoleOutput() const { return GetFlagsBit(kConsoleOutput); }
    
    // Net groups
    const plNetClientGroups* GetNetGroups()     const { return &fNetGroups; }
    plNetClientGroups* GetNetGroups()   { return &fNetGroups;   }

    // Voice Lists
    plNetListenList* GetListenList() { return &fListenList; }
    plNetTalkList* GetTalkList() { return &fTalkList; }
    void SetListenListMode (int i); 
    void SynchTalkList();
    int GetListenListMode() { return fListenListMode; }
        
    // network activity-generated events, passed to current task
    bool CanSendMsg(plNetMessage * msg);

    const plNetTransport& TransportMgr() const { return fTransport; }
    plNetTransport& TransportMgr() { return fTransport; }
    
    bool ObjectInLocalAge(const plSynchedObject* obj) const;
    
    // time converters
    plUnifiedTime GetServerTime() const;
    const char* GetServerLogTimeAsString(std::string& ts) const;
    double GetCurrentAgeElapsedSeconds() const;
    float GetCurrentAgeTimeOfDayPercent() const;

    bool RecordMsgs(const char* recType, const char* recName);
    bool PlaybackMsgs(const char* recName);

    void MakeCCRInvisible(plKey avKey, int level);
    bool CCRVaultConnected() const { return GetFlagsBit(kCCRVaultConnected); }

    uint8_t GetExperimentalLevel() const { return fExperimentalLevel; }

    void AddPendingLoad(PendingLoad *pl);
    const plKey& GetAgeSDLObjectKey() const { return fAgeSDLObjectKey; }
    plUoid GetAgeSDLObjectUoid(const char* ageName) const;
    plNetClientComm& GetNetClientComm()  { return fNetClientComm; }
    const char* GetNextAgeFilename();
    void SetOverrideAgeTimeOfDayPercent(float f) { fOverrideAgeTimeOfDayPercent=f;  }

    void AddPendingPagingRoomMsg( plNetMsgPagingRoom * msg );
    void MaybeSendPendingPagingRoomMsgs();
    void SendPendingPagingRoomMsgs();
    void ClearPendingPagingRoomMsgs();
    
    void NotifyRcvdAllSDLStates();

    plOperationProgress* GetTaskProgBar() { return fTaskProgBar;    }
    
    bool DebugMsgV(const char* fmt, va_list args) const;
    bool ErrorMsgV(const char* fmt, va_list args) const; 
    bool WarningMsgV(const char* fmt, va_list args) const; 
    bool AppMsgV(const char* fmt, va_list args) const;

    bool IsObjectOwner();
    void SetObjectOwner(bool own);

    void StoreSDLState(const plStateDataRecord* sdRec, const plUoid& uoid, uint32_t sendFlags, uint32_t writeOptions);

    void UpdateServerTimeOffset(plNetMessage* msg);
    void ResetServerTimeOffset(bool delayed=false);

private:
    plNetClientComm             fNetClientComm;
    plNetClientCommMsgHandler   fNetClientCommMsgHandler;
    
    int IInitNetClientComm();
    int IDeInitNetClientComm();
    void INetClientCommOpStarted(uint32_t context);
    void INetClientCommOpComplete(uint32_t context, int resultCode);

    friend struct plNCAgeJoiner;
    friend struct plNCAgeLeaver;
    friend class plNetDniInfoSource;
    friend class plNetTalkList;
    friend class plNetClientMsgHandler;
    friend struct plNetClientCommMsgHandler;
};

#define plCheckNetMgrResult_VoidReturn(r,s)     if (hsFailed(r)) { ErrorMsg(s); hsAssert(false,s); return; }
// returns int
#define plCheckNetMgrResult_ValReturn(r,s)      if (hsFailed(r)) { ErrorMsg(s); hsAssert(false,s); return r; }
// returns bool
#define plCheckNetMgrResult_BoolReturn(r,s)     if (hsFailed(r)) { ErrorMsg(s); hsAssert(false,s); return false; }

#endif  // PL_NET_CLIENT_inc




