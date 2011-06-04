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
#ifndef plNetClientGroup_h
#define plNetClientGroup_h

#include "hsStlUtils.h"
#include "../../NucleusLib/pnNetCommon/plNetGroup.h"

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
		OwnedGroup(plNetGroupId g, bool o) : fGroup(g),fOwnIt(o) { fGroup.SetDesc(g.GetDesc()); }
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
			(*it).fOwnIt=ownIt;
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
				return 1;	// yes
			return 0;		// no
		}
		return -1;			// don't know about it
	}
};



#endif	//  plNetClientGroup_h
