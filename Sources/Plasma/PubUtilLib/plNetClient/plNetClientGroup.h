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
#ifndef plNetClientGroup_h
#define plNetClientGroup_h

#include <set>
#include "pnNetCommon/plNetGroup.h"

//
// represents a collection of net groups.
// abstracted so that we can switch to a different container structure if necessary
//
class plNetClientGroups
{   
    friend class plNetClientMgr;
private:
    struct OwnedGroup
    {
        plNetGroupId fGroup;
        bool fOwnIt;
        
        bool operator<(const OwnedGroup& other) const { return other.fGroup<fGroup; }
        bool operator==(const OwnedGroup& other) const { return (other.fGroup==fGroup && other.fOwnIt==fOwnIt); }
        OwnedGroup(plNetGroupId g, bool o) : fGroup(std::move(g)), fOwnIt(o) { }
        OwnedGroup() : fOwnIt(false) {}
    };
    std::set<OwnedGroup> fGroups;
    
    std::set<OwnedGroup>::iterator IFind(const plNetGroupId& grpId)
    {
        std::set<OwnedGroup>::iterator it=fGroups.begin();
        for( ; it != fGroups.end(); it++)
            if ((*it).fGroup==grpId)
                break;
        return it;
    }
    
    // const version
    std::set<OwnedGroup>::const_iterator IFind(const plNetGroupId& grpId) const
    {
        std::set<OwnedGroup>::const_iterator it=fGroups.begin();
        for( ; it != fGroups.end(); it++)
            if ((*it).fGroup==grpId)
                break;
        return it;
    }

    void ISetGroupDesc(plNetGroupId& grpId);
public:
    void Reset()
    {
        ClearGroups();
        SetGroup(plNetGroup::kNetGroupLocalPlayer, true /*ownit*/);
        SetGroup(plNetGroup::kNetGroupRemotePlayer, false /*ownit*/);
        SetGroup(plNetGroup::kNetGroupLocalPhysicals, true /*ownit*/);
        SetGroup(plNetGroup::kNetGroupRemotePhysicals, false /*ownit*/);
    }
    
    void SetGroup(plNetGroupId& grpId, bool ownIt) 
    { 
        std::set<OwnedGroup>::iterator it=IFind(grpId);
        if (it != fGroups.end())
        {
            OwnedGroup grp(grpId, ownIt);
            fGroups.erase(it);
            fGroups.insert(grp);
        }
        else
        {
            ISetGroupDesc(grpId);
            fGroups.insert(OwnedGroup(grpId, ownIt));
        }
    }
    
#if 0
    void RemoveGroup(const plNetGroupId& grpId) 
    { 
        std::set<OwnedGroup>::iterator it=IFind(grpId);
        if (it != fGroups.end())
            fGroups.erase(it);
    }
#else
    void ClearGroups()
    {
        fGroups.clear();
    }
#endif
    int IsGroupLocal(const plNetGroupId& grpId) const
    { 
        std::set<OwnedGroup>::const_iterator it=IFind(grpId);
        if (it != fGroups.end())
        {
            if ((*it).fOwnIt) 
                return 1;   // yes
            return 0;       // no
        }
        return -1;          // don't know about it
    }
};



#endif  //  plNetClientGroup_h
