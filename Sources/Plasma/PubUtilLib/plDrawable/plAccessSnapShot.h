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

#ifndef plAccessSnapShot_inc
#define plAccessSnapShot_inc

#include "plAccessVtxSpan.h"

// All functions and fields here (including constructor) are private,
// because the only valid user of this is the friend class plAccessGeometry.
// Use this only via plAccessGeometry.
class plAccessSnapShot : public plAccessVtxSpan
{
public:
	void			Destroy(); // Free up and reset to zero.

protected:

	UInt16		fRefCnt;

	UInt8*		fData;

	UInt16		fChanSize[kNumValidChans];

	void			ICopyOldData(UInt8* data, const UInt16* const oldSizes, UInt16 oldStride, UInt16 newStride);
	UInt16			IComputeStride() const;
	void			IRecordSizes(UInt16 sizes[]) const;
	UInt32			ICheckAlloc(const plAccessVtxSpan& src, UInt32 chanMask, UInt32 chan, UInt16 chanSize);

	void			ISetupPointers(UInt16 newStride);

	void			SetupChannels(plAccessVtxSpan& dst) const;

	UInt32			CopyTo(const plAccessVtxSpan& dst, UInt32 chanMask);
	UInt32			CopyFrom(const plAccessVtxSpan& src, UInt32 chanMask);
	void			Release(); // Decrements refcnt, calls Destroy on zero
	void			Clear(); // Initialize to zeros

	void			IncRef() { fRefCnt++; }
	void			DecRef() { Release(); }

	plAccessSnapShot() : fRefCnt(0), fData(nil) { Clear(); }

	friend class plAccessGeometry;
};


#endif // plAccessSnapShot_inc
