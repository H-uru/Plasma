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

#ifndef plRenderLevel_inc
#define plRenderLevel_inc

class plRenderLevel
{
public:
	// A renderlevel is [Major bits 32..8]|[Minor bits 7..0]
	// The major render level is further broken into 3 ranges.
	// Range [0x00..0xff] - Blend onto the frame buffer (before even opaque objects)
	// Range [0x0100..0xffff] - Opaque and nearly opaque objects
	// Range [0x010000..0xffffff] - For blending objects (typically sorted amongst each other)
	// The minor bits denote a slight difference in draw order. For example, a decal wants
	// to be drawn after the opaque object it is applied to, but hopefully not very long after.
	// The avatar gets a render priority of kDefRendMajorLevel,kAvatarRendMinorLevel. This puts
	// it in the group of normal opaque objects with no render dependencies, but with the maximum
	// permitted minor level. So it will be drawn after the opaque background, and the opaque background's
	// decals, but before the first thing with a render dependency on the background (e.g. plants).

	// Removed kAvatarBlendRendMinorLevel, not being used anywhere. mf
	
	enum {
		kOpaqueMajorLevel		= 0x0,
		kFBMajorLevel			= 0x1,
		kDefRendMajorLevel		= 0x2,
		kBlendRendMajorLevel	= 0x4,
		kLateRendMajorLevel		= 0x8

	};
	enum {
		kMajorShift				= 28
	};
	enum {
		kDefRendMinorLevel					= 0x00,
		kOpaqueMinorLevel					= 0x0,
		kMinorLevelMask						= ((1 << kMajorShift) - 1),
		kAvatarRendMinorLevel				= kMinorLevelMask-1
	};
public:
	plRenderLevel() { Set(kDefRendMajorLevel, kDefRendMinorLevel); }
	plRenderLevel(UInt32 l) : fLevel(l) {}
	plRenderLevel(UInt32 major, UInt32 minor) { Set(major, minor); }

	int operator==(const plRenderLevel& l) const { return fLevel == l.fLevel; }
	int operator!=(const plRenderLevel& l) const { return fLevel != l.fLevel; }
	int operator>(const plRenderLevel& l) const { return fLevel > l.fLevel; }
	int operator<(const plRenderLevel& l) const { return fLevel < l.fLevel; }
	int operator>=(const plRenderLevel& l) const { return fLevel >= l.fLevel; }
	int operator<=(const plRenderLevel& l) const { return fLevel <= l.fLevel; }

	UInt32	Level() const { return fLevel; }

	UInt32  Minor() const { return UInt32(fLevel & kMinorLevelMask); }
	UInt32	Major() const { return UInt32(fLevel >> kMajorShift); }

	plRenderLevel& Set(UInt32 l) { fLevel = l; return *this; }
	plRenderLevel& Set(UInt32 major, UInt32 minor) { fLevel = (UInt32(major) << kMajorShift) | UInt32(minor); return *this; }

	UInt32	fLevel;

	static plRenderLevel OpaqueRenderLevel() { return plRenderLevel(kOpaqueMajorLevel, kOpaqueMinorLevel); }
};

#endif // plRenderLevel_inc
