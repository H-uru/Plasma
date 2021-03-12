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
#ifndef plNetTransportMember_h
#define plNetTransportMember_h

#include "HeadSpin.h"
#include "plNetCommon/plNetMember.h"
#include "pnKeyedObject/plKey.h"

#include <string_theory/string>
#include <vector>

//
// This represents a participant in the game, ie. another 
// remote user.
// It is a basic net member with a list of channels that it subscribes to.
//
class plKey;

class plNetTransportMember : public plNetMember
{
public:
    enum TransportFlags
    {
        kSendingVoice   = 1<<0,
        kSendingActions = 1<<1,
    };
protected:
    plKey           fAvatarKey;
    ST::string      fPlayerName;
    uint32_t        fPlayerID;
    std::vector<int> fSubscriptions;    // list of channelGrp subscriptions
    uint32_t        fTransportFlags;
    float           fDistSq;            // from local player, temp
    uint8_t         fCCRLevel;
public:
    CLASSNAME_REGISTER( plNetTransportMember);
    GETINTERFACE_ANY( plNetTransportMember, plNetMember);

    plNetTransportMember() :
        fTransportFlags(), fPlayerID(), fDistSq(-1), fCCRLevel() { }
    plNetTransportMember(plNetApp* na) : plNetMember(na),
        fTransportFlags(), fPlayerID(), fDistSq(-1), fCCRLevel() { }
    ~plNetTransportMember() {}

    void SetDistSq(float s) { fDistSq=s; }
    float GetDistSq() const { return fDistSq; }

    plKey GetAvatarKey() const { return fAvatarKey; }
    void SetAvatarKey(plKey k)
        {
            fAvatarKey=k;
        }
    void SetPlayerName(const ST::string & value) { fPlayerName=value;}
    ST::string GetPlayerName() const { return fPlayerName;}
    void SetPlayerID(uint32_t value) { fPlayerID=value;}
    uint32_t GetPlayerID() const { return fPlayerID;}
    void SetIsServer(bool value) { (value)?SetFlags(GetFlags()|kIsServer):SetFlags(GetFlags()&~kIsServer);}
    bool IsServer() const { return (GetFlags()&kIsServer)?true:false;}

    bool AddSubscription(int chan);
    bool RemoveSubscription(int chan);    // return true on success
    int FindSubscription(int chan);         // return index into subscription array or -1

    int GetNumSubscriptions() { return fSubscriptions.size(); }
    int GetSubscription(int i) { return fSubscriptions[i]; }

    void CopySubscriptions(std::vector<int>* channels) { *channels = fSubscriptions; }

    void SetTransportFlags(uint32_t f) { fTransportFlags=f; }
    uint32_t GetTransportFlags() const { return fTransportFlags; }

    bool IsPeerToPeer() const { return hsCheckBits(fFlags, kRequestP2P); }
    ST::string AsString() const override;
    bool IsEqualTo(const plNetMember * other) const override
    {
        const plNetTransportMember * o = plNetTransportMember::ConvertNoRef(other);
        if (!o) return false;
        return o->fPlayerID==fPlayerID;
    }

    bool IsCCR() const { return (fCCRLevel>0);  }
    uint8_t GetCCRLevel() const { return fCCRLevel;   }
    void SetCCRLevel(uint8_t cl) { fCCRLevel=cl;  }
};

#endif  // plNetTransportMember_h
