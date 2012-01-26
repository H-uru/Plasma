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
#ifndef plNetTransport_h
#define plNetTransport_h

#include "HeadSpin.h"
#include "hsStlUtils.h"

//
// The transport class handles the details of sending net msgs to
// the server or to groups of clients (members).
// It hides whether we are playing in P2P mode or C/S mode.
// It is currently used only clientside but is general enough to 
// be used by a server as well.
//
class plKey;
class plNetTransportMember;
typedef std::vector<plNetTransportMember*> plMembersList;
class plNetMessage;
class plNetTransport
{
private:
    plMembersList fMembers;                     // master list of all members in the game, server is member[0]
    std::vector<plMembersList> fChannelGroups;  // members grouped by channel

    void IUnSubscribeToAllChannelGrps(plNetTransportMember* mbr);
    void IRemoveMember(plNetTransportMember* mbr);
public:
    plNetTransport() {}
    ~plNetTransport();

    void DumpState();
    
    // master list ops
    void GetMemberListDistSorted(plNetTransportMember**& listPtr) const;    // allocates and sorts array
    int FindMember(const plKey avKey) const;                    // return array index or -1
    int FindMember(uint32_t playerID) const;                      // return array index or -1
    int FindMember(const plNetTransportMember* mbr);            // return array index or -1
    int AddMember(plNetTransportMember* mbr);                   // to master list, if not there
    hsBool RemoveMember(plNetTransportMember* mbr);             // from master list and all channels
    hsBool RemoveMember(int idx);                               // from master list and all channels
    int GetNumMembers() const { return fMembers.size(); }
    plNetTransportMember* GetMember(int i) const { return i>=0 && i<fMembers.size() ? fMembers[i] : nil; }
    void ClearMembers();

    // channel group ops
    void SubscribeToChannelGrp(plNetTransportMember* mbr, int channel);
    hsBool UnSubscribeToChannelGrp(plNetTransportMember* mbr, int channel);
    void GetSubscriptions(plNetTransportMember* mbr, std::vector<int>* channels) const;
    void ClearChannelGrp(int channel);
    void SetNumChannels(int n);

    int SendMsg(int channel, plNetMessage* msg) const;
};

#endif  // plNetTransport_h
