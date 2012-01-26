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
#ifndef plNetVoiceList_h
#define plNetVoiceList_h

#include "HeadSpin.h"
#include "hsStlUtils.h"

//
// a simple class used by the net client code to hold listenLists and talkLists
// for voice filtering.
//
class plNetTransportMember;
class plNetVoiceList
{
protected:
    typedef std::vector<plNetTransportMember*> VoiceListType;   
protected:
    VoiceListType fMembers; 

public: 
    plNetVoiceList() {}
    virtual ~plNetVoiceList() {}
    
    int GetNumMembers() const { return fMembers.size(); }
    plNetTransportMember* GetMember(int i) const { return fMembers[i];  }
    virtual void AddMember(plNetTransportMember* e) = 0;
    virtual void RemoveMember(plNetTransportMember* e) = 0;
    virtual void Clear() { fMembers.clear(); }
    int FindMember(plNetTransportMember* e);    // return index or -1
};

//
// Specialized version for listen list
// a list of other player I am listening to
//
class plNetListenList : public plNetVoiceList
{
private:
    double fLastUpdateTime;
    int fNumUpdates;
public:
    plNetListenList() : fNumUpdates(0) {}
    ~plNetListenList() {}

    static float kUpdateInterval;
    static int kMaxListenListSize;
    static float kMaxListenDistSq;
    
    void SetLastUpdateTime(double t) { fLastUpdateTime=t; fNumUpdates++;    }
    double GetLastUpdateTime() { return fLastUpdateTime;    }

    hsBool CheckForceSynch() { if (fNumUpdates>10) { fNumUpdates=0; return true;} return false; }

    virtual void AddMember(plNetTransportMember* e);
    virtual void RemoveMember(plNetTransportMember* e);

};

//
// Specialized version for talk list
// a list of other players I am talking to
//
class plNetClientMgr;
class plNetTalkList : public plNetVoiceList
{
private:
    enum
    {
        kDirty  = 0x1
    };
    uint32_t fFlags;
public:
    plNetTalkList() : fFlags(0) {}
    ~plNetTalkList() {}

    void UpdateTransportGroup(plNetClientMgr* nc);

    void AddMember(plNetTransportMember* e);
    void RemoveMember(plNetTransportMember* e);
    void Clear();
};

#endif  // plNetVoiceList_h
