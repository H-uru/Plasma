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
#ifndef plNetLinkingMgr_h_inc
#define plNetLinkingMgr_h_inc

#include "HeadSpin.h"
#include "hsBitVector.h"
#include "plNetCommon/plNetServerSessionInfo.h"
#include "plNetCommon/plNetCommon.h"
#include "plMessage/plLinkToAgeMsg.h"

class plMessage;
struct plNCAgeJoiner;
struct plNCAgeLeaver;
class plVaultNotifyMsg;

class plNetLinkingMgr
{
    static void NCAgeJoinerCallback (
        plNCAgeJoiner *         joiner,
        unsigned                type,
        void *                  notify,
        void *                  userState
    );
    
    static void NCAgeLeaverCallback (
        plNCAgeLeaver *         leaver,
        unsigned                type,
        void *                  notify,
        void *                  userState
    );

    friend struct NCAgeJoinerCallback;
    friend struct NCAgeLeaverCallback;
    
    static void ExecNextOp ();


    plNetLinkingMgr()
        : fLinkingEnabled(true), fLinkedIn(), fDeferredLink()
    { }
    plNetLinkingMgr(const plNetLinkingMgr &) { }
    ~plNetLinkingMgr();

    enum Cmds
    {
        kNilCmd,
        // Sent to a player to have them call us back with info for linking to their age.
        kLinkPlayerHere,
        // Offer link to player.
        kOfferLinkToPlayer,
        // Offer link to another player from a public linking book (to my instance of the age w/out going through personal age)
        kOfferLinkFromPublicBook,
        // Sent to a player to have them link to their last age 
        kLinkPlayerToPrevAge
    };

    plLinkToAgeMsg* fDeferredLink;

    enum PreProcessResult {
        // Old style IPreProcessLink style "false" result
        kLinkFailed,
        // Old style IPreProcessLink style "true" result
        kLinkImmediately,
        // Defer the link until later, don't trash the structs
        kLinkDeferred,
    };

    uint8_t IPreProcessLink();
    void IPostProcessLink();
    bool IProcessLinkingMgrMsg( plLinkingMgrMsg * msg );
    bool IProcessLinkToAgeMsg( plLinkToAgeMsg * msg );
    void IDoLink(plLinkToAgeMsg* link);
    bool IProcessVaultNotifyMsg(plVaultNotifyMsg* msg);

    bool IDispatchMsg( plMessage* msg, uint32_t playerID );


public:
    static plNetLinkingMgr * GetInstance();
    bool MsgReceive( plMessage *msg );    // TODO: Make this a hsKeyedObject so we can really handle messages.
    void Update();

    bool IsEnabled() const { return fLinkingEnabled;}
    void SetEnabled( bool b );
    
    bool LinkedIn () const { return  fLinkedIn &&  fLinkingEnabled; }
    bool Linking () const  { return !fLinkedIn && !fLinkingEnabled; }

    // Link to an age.
    void LinkToAge( plAgeLinkStruct * link, bool linkInSfx=true, bool linkOutSfx=true, uint32_t playerID=kInvalidPlayerID );
    void LinkToAge( plAgeLinkStruct * link, const ST::string& linkAnim, bool linkInSfx=true, bool linkOutSfx=true, uint32_t playerID=kInvalidPlayerID );

    // Link to my last age.
    void LinkToPrevAge( uint32_t playerID=kInvalidPlayerID );

    // Link to my Personal Age
    void LinkToMyPersonalAge( uint32_t playerID=kInvalidPlayerID );

    // Link to my Neighborhood Age
    void LinkToMyNeighborhoodAge( uint32_t playerID=kInvalidPlayerID );

    // Link a player here.
    void LinkPlayerHere( uint32_t playerID );

    // Link player to specified age
    void LinkPlayerToAge( plAgeLinkStruct * link, uint32_t playerID );

    // Link player back to his last age
    void LinkPlayerToPrevAge( uint32_t playerID );

    // Link us to a players age.
    void LinkToPlayersAge( uint32_t playerID );

    // Offer a link to player.
    void OfferLinkToPlayer( const plAgeLinkStruct * info, uint32_t playerID, const plKey& replyKey );
    void OfferLinkToPlayer( const plAgeInfoStruct * info, uint32_t playerID );
    void OfferLinkToPlayer( const plAgeLinkStruct * info, uint32_t playerID );

    // Leave the current age
    void LeaveAge (bool quitting);

    // link info
    plAgeLinkStruct * GetAgeLink() { return &fAgeLink; }
    plAgeLinkStruct * GetPrevAgeLink() { return &fPrevAgeLink; }

    // helpers
    static ST::string GetProperAgeName( const ST::string & ageName );    // attempt to fix wrong case age name.

private:
    bool                fLinkingEnabled;
    bool                fLinkedIn;

    // The age we are either joining or are joined with.
    plAgeLinkStruct     fAgeLink;

    // The age we just left.
    plAgeLinkStruct     fPrevAgeLink;
};


#endif // plNetLinkingMgr_h_inc
