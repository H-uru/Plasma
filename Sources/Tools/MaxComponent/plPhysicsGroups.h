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
#ifndef plPhysicsGroups_inc
#define plPhysicsGroups_inc

// These are dead now, and are only around for old Max files.  New physicals use
// plSimDefs::Group.
//
// Note: These bits are actually saved in the Max file.  If you swap them around
// the plEventGroupProc code will have to be changed to map between the Max version
// and Plasma version.  To make things easier, just add new ones and don't reuse old
// ones that are removed.
namespace plPhysicsGroups_DEAD
{
	enum
	{
		kUserGroup1		= 1<<1,
		kUserGroup2		= 1<<2,
		kUserGroup3		= 1<<3,
		kUserGroup4		= 1<<4,
		kUserGroup5		= 1<<5,
		kUserGroup6		= 1<<6,
		kUserGroup7		= 1<<7,
		kUserGroup8		= 1<<8,
		kUserGroup9		= 1<<9,
		kUserGroup10	= 1<<10,
		kUserGroup11	= 1<<11,
		kUserGroup12	= 1<<12,
		kUserGroup13	= 1<<13,
		kUserGroup14	= 1<<14,
		kUserGroup15	= 1<<15,
		kUserGroup16	= 1<<16,

		kAvatars			= 1<<17,
		kCreatures			= 1<<18,
		kAnimated			= 1<<23,
		kDynamicSimulated	= 1<<24,
		kStaticSimulated	= 1<<25,
		kDetectors			= 1<<26,
		// Local avatars are included in kAvatars, you only need to use this if
		// you want to only do something with local avatars.
		kLocalAvatars		= 1<<27,
	};
};

#endif // plPhysicsGroups_inc
