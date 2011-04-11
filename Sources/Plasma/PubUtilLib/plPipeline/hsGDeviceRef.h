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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	hsGDeviceRef.h - Header for the generic deviceRef class      			 //
//	Cyan, Inc.																 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsGDeviceRef_inc
#define hsGDeviceRef_inc

#include "hsRefCnt.h"


class hsGDeviceRef : public hsRefCnt
{
protected:
	UInt32		fFlags;

public:
	// Note, derived classes define more flags. Take care if adding flags here.
	// Currently have flags 0x0 - 0x8 reserved.
	enum {
		kNone			= 0x0,
		kDirty			= 0x1
	};

	UInt32					fUseTime;		// time stamp when last used - stat gather only

	hsBool IsDirty() const { return (fFlags & kDirty); }
	void SetDirty(hsBool on) { if(on)fFlags |= kDirty; else fFlags &= ~kDirty; }

	hsGDeviceRef() : fFlags(0), fUseTime(0) {}
	virtual ~hsGDeviceRef() {}
};

#endif // hsGDeviceRef_inc
