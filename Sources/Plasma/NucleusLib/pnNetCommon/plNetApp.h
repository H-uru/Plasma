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
#ifndef plNetApp_h
#define plNetApp_h

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "plNetGroup.h"

#include "plLoggable.h"

#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plUoid.h"

#define plVerifyConditionRet(NetApp,cond,ret,str)   \
    do {    \
        if (!(cond)) {  \
            char * _str_ = str; \
            (NetApp)->ErrorMsg(_str_);  \
            hsAssert(cond,_str_);   \
            return ret; \
        }   \
    } while (0)

#define plVerifyCondition(NetApp,cond,str)  \
    plVerifyConditionRet(NetApp,cond,hsFail,str)


class plNetMember;
class plSynchedObject;
class plKey;
class plNetMessage;

typedef std::vector<plNetMember*>       plNetMemberList;
typedef std::vector<uint32_t>             plNetPlayerIDList;

//
// Common baseclasses for client and server net apps
//

class plNetApp : public hsKeyedObject, public plLoggable        // so it can recv messages
{
protected:
    static plNetApp*        fInstance;
    hsBitVector fFlagsVec;
public:
    enum FlagBits   // common to both client and server
    {
        kNullSend=0,
        kNetCoreSingleThreaded,
        kScreenMessages,          // filter out illegal net game messages, used by gameserver&client

        FLAG_CEILING        = 10  // stay below this or conflict with client/server specific flags
    };

    static plNetApp* GetInstance();
    static void SetInstance(plNetApp* app);

    plNetApp() {}
    virtual ~plNetApp() {}

    CLASSNAME_REGISTER(plNetApp);
    GETINTERFACE_ANY(plNetApp, hsKeyedObject);

    virtual void Shutdown() {}

    void SetFlagsBit(int b, bool on=true) { fFlagsVec.SetBit(b, on); }
    bool GetFlagsBit(int b) const { return fFlagsVec.IsBitSet(b) ? true : false; }

    static bool StaticErrorMsg(const ST::string& msg);
    static bool StaticWarningMsg(const ST::string& msg);
    static bool StaticAppMsg(const ST::string& msg);
    static bool StaticDebugMsg(const ST::string& msg);

    static bool StaticErrorMsg(const char* msg)
    {
        return StaticErrorMsg(ST::string(msg));
    }

    static bool StaticWarningMsg(const char* msg)
    {
        return StaticWarningMsg(ST::string(msg));
    }

    static bool StaticAppMsg(const char* msg)
    {
        return StaticAppMsg(ST::string(msg));
    }

    static bool StaticDebugMsg(const char* msg)
    {
        return StaticDebugMsg(ST::string(msg));
    }

    template <typename... _Args>
    static bool StaticErrorMsg(const char* fmt, _Args... args)
    {
        return StaticErrorMsg(ST::format(fmt, args...));
    }

    template <typename... _Args>
    static bool StaticWarningMsg(const char* fmt, _Args... args)
    {
        return StaticWarningMsg(ST::format(fmt, args...));
    }

    template <typename... _Args>
    static bool StaticAppMsg(const char* fmt, _Args... args)
    {
        return StaticAppMsg(ST::format(fmt, args...));
    }

    template <typename... _Args>
    static bool StaticDebugMsg(const char* fmt, _Args... args)
    {
        return StaticDebugMsg(ST::format(fmt, args...));
    }
};

//
// base netApp class specific to client code
//
class plVaultPlayerNode;
class plNetClientApp : public plNetApp
{
private:
    int fCCRLevel;  // 0 for players, 1-4 for CCRs

    friend class plDispatch;

    virtual hsError ISendGameMessage(plMessage* msg) { hsAssert(false, "stub"); return hsFail; }

public:
    enum ClientFlagBits
    {
        kDeferMsgs          = FLAG_CEILING, // currently ignoring most msgs because the game is not active
        kDisabled,                          // client. networking has been disabled
        kDisableOnNextUpdate,               // client. disable networking in next update call
        kLocalTriggers,                     // marks all triggers as local only
        kEchoVoice,                         // send voice packets back to the speaker
        kRequestP2P,                        // wants to play peer to peer
        kIndirectMember,                    // behind a firewall, can't player P2P
        kSendingVoice,                      // sending a voice packet
        kSendingActions,                    // sending a game message
        kAllowTimeOut,                      // allow clients to timeout and be kicked off the server
        kAllowAuthTimeOut,                  // allow clients to timeout while authenticating
        kPlayingGame,                       // set when client is actively part of an age.
        kShowLists,                         // debug info on-screen 
        kShowRooms,                         // debug info on-screen 
        kShowAvatars,                       // Avatar position/orientation info
        kShowRelevanceRegions,              // debug info on-screen 
        kConnectedToVault,                  // initial connection to vault achieved
        kBanLinking,                        // player is not allowed to link
        kSilencePlayer,                     // player's outbound communication is shutoff
        kConsoleOutput,                     // net log output is echoed to console
        kLoadingInitialAgeState,            // set when we first link in to an age and are recving its initial state
        kLaunchedFromSetup,                 // set if we were launched from the setup program
        kCCRVaultConnected,                 // set if we've connected to the CCR vault 
        kNetClientCommInited,               // set if the netClientComm interface has been initialized
        kNeedToSendInitialAgeStateLoadedMsg,// internal use only, when we need to send plInitialAgeStateLoadedMsg
        kNeedToSendAgeLoadedMsg,
        kDemoMode,                          // set if this is a demo - limited play
        kNeedInitialAgeStateCount,          // the server must tell us how many age states to expect
        kLinkingToOfflineAge,               // set if we're linking to the startup age
    };

    CLASSNAME_REGISTER(plNetClientApp);
    GETINTERFACE_ANY(plNetClientApp, plNetApp);

    plNetClientApp();
    
    // statics
    static plNetClientApp* GetInstance() { return plNetClientApp::ConvertNoRef(fInstance); }
    static const plNetClientApp* GetConstInstance() { return plNetClientApp::ConvertNoRef(fInstance); }
    static void InheritNetMsgFlags(const plMessage* parentMsg, plMessage* childMsg, bool startCascade);
    static void InheritNetMsgFlags(uint32_t parentMsgFlags, uint32_t* childMsgFlags, bool startCascade);
    static void UnInheritNetMsgFlags(plMessage* msg);

    // functions that all net client apps should implement
    virtual hsError SendMsg(plNetMessage* msg) = 0;
    virtual uint32_t GetPlayerID() const = 0;
    virtual ST::string GetPlayerName(const plKey avKey={}) const = 0;

    // commonly used net client app functions
    virtual float GetCurrentAgeTimeOfDayPercent() const { hsAssert(false, "stub"); return 0.; }
    virtual bool ObjectInLocalAge(const plSynchedObject* obj) const { hsAssert(false, "stub"); return false; }
    virtual uint8_t GetJoinOrder() const { hsAssert(false, "stub"); return 0; }
    virtual bool IsRemotePlayerKey(const plKey& p, int* idx=nullptr) { hsAssert(false, "stub"); return false; }
    virtual plKey GetLocalPlayerKey()   const { hsAssert(false, "stub"); return nullptr; }
    virtual plSynchedObject* GetLocalPlayer(bool forceLoad=false) const { hsAssert(false, "stub"); return nullptr; }
    virtual plNetGroupId SelectNetGroup(plSynchedObject* objIn, const plKey& groupKey) { hsAssert(false, "stub"); return plNetGroup::kNetGroupUnknown; }
    virtual int IsLocallyOwned(const plSynchedObject* obj) const { hsAssert(false, "stub"); return 0; }
    virtual int IsLocallyOwned(const plUoid&) const { hsAssert(false, "stub"); return 0; }  
    virtual plNetGroupId GetEffectiveNetGroup(const plSynchedObject* obj) const { hsAssert(false, "stub"); return plNetGroup::kNetGroupUnknown; }
    virtual hsError Update(double secs) { return hsOK; }
    virtual const char* GetServerLogTimeAsString(ST::string& ts) const { hsAssert(false, "stub"); return nullptr; }
    virtual plUoid GetAgeSDLObjectUoid(const ST::string& ageName) const { hsAssert(false, "stub"); return plUoid(); }
    virtual void StayAlive(double secs) {}
    virtual void QueueDisableNet( bool showDlg, const char msg[] ) {}

    bool IsEnabled() const { return !GetFlagsBit(kDisabled); }
    bool InDemoMode() const { return GetFlagsBit(kDemoMode); }
    bool IsLoadingInitialAgeState() const { return GetFlagsBit(kLoadingInitialAgeState); }
    void  SetLaunchedFromSetup(bool b) { SetFlagsBit(kLaunchedFromSetup, b);    }
    bool GetLaunchedFromSetup() const { return GetFlagsBit(kLaunchedFromSetup); }
    
    // CCR stuff
#ifdef PLASMA_EXTERNAL_RELEASE
    void SetCCRLevel(int level) {   }
    int GetCCRLevel() const { return 0; }
    bool AmCCR() const { return false; }
#else
    void SetCCRLevel(int level) { fCCRLevel=level;  }
    int GetCCRLevel() const { return fCCRLevel; }
    bool AmCCR() const { return (fCCRLevel>0); }
#endif
};

//
// base netApp class specific to server code
//
class plNetServerApp : public plNetApp
{
public:
    enum ServerFlagBits
    {
        kLastFlagBitsValue  = FLAG_CEILING, // get past plNetApp flags
        kDone,                              // exit update loop.
        kDumpStats,                         // dump stats to log file
        kDisableStateLogging,               // used by gameserver
        kGameStateIsDirty,                  // used by gameserver
        kDumpConfigDoc,                     // dump config options queries to log file
        kProtectedServer,                   // set by a protected lobby 
        kRequireProtectedCCRs,              // CCRS must have logged in thru a protected lobby, used by gameserver
        kProcessedPendingMsgs,              // Used by front-end server
    };

    CLASSNAME_REGISTER(plNetServerApp);
    GETINTERFACE_ANY(plNetServerApp, plNetApp);

    virtual int SendMsg(plNetMessage* msg) = 0;
};

//
// abstract base class for net client object debugger
//
class plNetObjectDebuggerBase
{
private:
    static plNetObjectDebuggerBase* fInstance;
public:
    static plNetObjectDebuggerBase* GetInstance() { return fInstance;   }
    static void SetInstance(plNetObjectDebuggerBase* i) { fInstance=i;  }
    virtual bool IsDebugObject(const hsKeyedObject* obj) const = 0;
    virtual void LogMsgIfMatch(const ST::string& msg) const = 0;      // write to status log if there's a string match
    virtual void LogMsg(const ST::string& msg) const = 0;
    
    virtual bool GetDebugging() const = 0;
    virtual void SetDebugging(bool b) = 0;
};

#endif  // plNetApp_h
