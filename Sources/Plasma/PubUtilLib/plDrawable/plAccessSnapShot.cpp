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

#include "hsTypes.h"
#include "plAccessSnapShot.h"

#include "hsGeometry3.h"

#include <memory.h>

void plAccessSnapShot::Clear()
{
	ClearVerts();
	
	int i;
	for( i = 0; i < kNumValidChans; i++ )
		fChanSize[i] = 0;
}

void plAccessSnapShot::Destroy()
{
	Clear();

	delete [] fData;
	
	fRefCnt = 0;
}

void plAccessSnapShot::Release()
{
	hsAssert(fRefCnt, "Releasing a snapshot with no refs. Check matching TakeSnapShot/ReleaseSnapShot calls.");
	if( !--fRefCnt )
	{
		Destroy();
	}
}

UInt32 plAccessSnapShot::ICheckAlloc(const plAccessVtxSpan& src, UInt32 chanMask, UInt32 chan, UInt16 chanSize)
{
	if( ((1 << chan) & chanMask) && src.fStrides[chan] )
	{
		if( fChanSize[chan] )
		{
			// We already have this one
			chanMask &= ~(1 << chan);
		}
		else
		{
			// We'll get this one
			fChanSize[chan] = chanSize;
		}
	}
	else
	{
		// either we haven't been asked for this or src doesn't have it.
		// either way, we're never going to get it.
		chanMask &= ~(1 << chan);
	}
	return chanMask;
}

void plAccessSnapShot::IRecordSizes(UInt16 sizes[]) const
{
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
		sizes[chan] = fChanSize[chan];
}

UInt16 plAccessSnapShot::IComputeStride() const
{
	UInt16 stride = 0;
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
		stride += fChanSize[chan];

	return stride;
}

void plAccessSnapShot::ICopyOldData(UInt8* data, const UInt16* const oldSizes, UInt16 oldStride, UInt16 newStride)
{
	UInt32 oldOffset = 0;
	UInt32 newOffset = 0;
	UInt8*	srcChannels[kNumValidChans];
	UInt8*	dstChannels[kNumValidChans];
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
	{
		if( oldSizes[chan] )
		{
			hsAssert(fChanSize[chan], "Copying a channel we don't have");
			srcChannels[chan] = data + oldOffset;
			oldOffset += oldSizes[chan];
		}

		if( fChanSize[chan] )
		{
			dstChannels[chan] = fData + newOffset;
			newOffset += fChanSize[chan];
		}
	}
	int i;
	for( i = 0; i < fNumVerts; i++ )
	{
		for( chan = 0; chan < kNumValidChans; chan++ )
		{
			if( oldSizes[chan] )
			{
				memcpy(dstChannels[chan], srcChannels[chan], oldSizes[chan]);
				dstChannels[chan] += newStride;
				srcChannels[chan] += oldStride;
			}
		}
	}
}

void plAccessSnapShot::ISetupPointers(UInt16 newStride)
{
	fData = TRACKED_NEW UInt8[fNumVerts * newStride];

	int size = 0;
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
	{
		if( fChanSize[chan] )
		{
			fStrides[chan] = newStride;
			fChannels[chan] = fData + size;
			size += fChanSize[chan];
		}
	}
}

UInt32 plAccessSnapShot::CopyFrom(const plAccessVtxSpan& src, UInt32 chanMask)
{
	hsAssert(!fNumVerts || (fNumVerts == src.fNumVerts), "Copying from a different sized span");

	fNumVerts = src.fNumVerts;

	UInt16 oldSize[kNumValidChans];
	UInt8* oldData = fData;
	IRecordSizes(oldSize);

	UInt16 oldStride = IComputeStride();

	// First, allocate any storage we need. Kill any requested channels out of the
	// mask that we already have.
	chanMask = ICheckAlloc(src, chanMask, kPosition, sizeof(hsPoint3));

	chanMask = ICheckAlloc(src, chanMask, kWeight, sizeof(hsScalar) * src.fNumWeights);
	if( fChanSize[kWeight] )
		fNumWeights = src.fNumWeights;

	chanMask = ICheckAlloc(src, chanMask, kWgtIndex, sizeof(UInt32));

	chanMask = ICheckAlloc(src, chanMask, kNormal, sizeof(hsVector3));

	chanMask = ICheckAlloc(src, chanMask, kDiffuse, sizeof(UInt32));

	chanMask = ICheckAlloc(src, chanMask, kSpecular, sizeof(UInt32));

	chanMask = ICheckAlloc(src, chanMask, kUVW, sizeof(hsPoint3) * src.fNumUVWsPerVert);
	if( fChanSize[kUVW] )
		fNumUVWsPerVert = src.fNumUVWsPerVert;

	// If our chanMask has gone to zero, we've only been asked to record
	// channels we already have, so there's nothing to do.
	if( !chanMask )
		return 0;

	UInt16 newStride = IComputeStride();

	ISetupPointers(newStride);

	ICopyOldData(oldData, oldSize, oldStride, newStride);

	UInt8*	srcChannels[kNumValidChans];
	UInt8*	dstChannels[kNumValidChans];
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
	{
		srcChannels[chan] = src.fChannels[chan];
		dstChannels[chan] = fChannels[chan];
	}

	int i;
	for( i = 0; i < src.VertCount(); i++ )
	{
		for( chan = 0; chan < kNumValidChans; chan++ )
		{
			if( (1<< chan) & chanMask )
			{
				memcpy(dstChannels[chan], srcChannels[chan], fChanSize[chan]);
				dstChannels[chan] += fStrides[chan];
				srcChannels[chan] += src.fStrides[chan];
			}
		}
	}

	return chanMask;
}

UInt32 plAccessSnapShot::CopyTo(const plAccessVtxSpan& dst, UInt32 chanMask)
{
	hsAssert(fNumVerts == dst.fNumVerts, "Vertex count mismatch, is this our real source?");

	int chan; 
	for( chan = 0; chan < kNumValidChans; chan++ )
	{
		if( !(fChanSize[chan] && dst.fStrides[chan]) )
			chanMask &= ~(1 << chan);
	}
	// If chanMask has gone to zero, either we don't have any of the requested channels
	// recorded, or dst doesn't have them. Both being true is valid, but
	// us having a channel recorded that's not in dst is probably an error.
	if( !chanMask )
		return 0;

	int i;
	for( i = 0; i < fNumVerts; i++ )
	{
		for( chan = 0; chan < kNumValidChans; chan++ )
		{
			if( (1 << chan) & chanMask )
			{
				memcpy(
					dst.fChannels[chan] + dst.fStrides[chan] * i, 
					fChannels[chan] + fStrides[chan] * i, 
					fChanSize[chan]);
			}
		}
	}
	return chanMask;
}

void plAccessSnapShot::SetupChannels(plAccessVtxSpan& dst) const
{
	int chan;
	for( chan = 0; chan < kNumValidChans; chan++ )
	{
		if( fChanSize[chan] )
		{
			dst.fChannels[chan] = fChannels[chan];
			dst.fStrides[chan] = fStrides[chan];
			dst.fOffsets[chan] = 0;
		}
	}
}
