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
#ifndef PLNETMEMBER_inc
#define PLNETMEMBER_inc

#include "HeadSpin.h"

#include "pnFactory/plCreatable.h"

class plNetApp;
class plNetGenericServer;
namespace ST { class string; }

////////////////////////////////
// A participant (peer) who we can send and recv messages from/to
////////////////////////////////
class plNetMember : public plCreatable
{
public:
    enum Flags
    {
        kWaitingForLinkQuery            = 1<<0,     // only used server side
        kIndirectMember                 = 1<<1,     // this guy is behind a firewall of some sort
        kRequestP2P                     = 1<<2,     // wants to play peer to peer
        kWaitingForChallengeResponse    = 1<<3,     // waiting for client response
        kIsServer                       = 1<<4,     // used by transport member
        kAllowTimeOut                   = 1<<5,     // used by gameserver
    };

protected:
    friend class plNetGenericServer;
    friend class plNetClientMgr;
    friend class plNetClientMsgHandler;
    friend class plNetClientGamePlayMsgHandler;

    int32_t   fPeerID;            // low-level netPlayer object for msg send/recv
    uint32_t  fFlags;
    plNetApp* fNetApp;

    // these calls should be made by the client/server app only, 
    // so they can keep the netCorePeer userData point to the right member
    virtual ~plNetMember() {}

public:
    CLASSNAME_REGISTER( plNetMember );
    GETINTERFACE_ANY( plNetMember, plCreatable );

    plNetMember();
    plNetMember(plNetApp* na);
    
    virtual void Reset();   // doesn't remove from session, just resets to initial state
    virtual bool IsEqualTo(const plNetMember * other) const = 0;

    // getters
    int32_t GetPeerID() const { return fPeerID; }
    uint32_t GetFlags() const { return fFlags; }
    plNetApp* GetNetApp() { return fNetApp; }
    virtual ST::string AsString() const = 0;

    // setters
    void SetFlags(uint32_t f) { fFlags=f; }
    void SetNetApp(plNetApp* n) { fNetApp=n; }
    void SetPeerID(int32_t p) { fPeerID=p; }
};

#endif  // PLNETMEMBER_inc
