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
#include "plVertCoder.h"

#include "hsStream.h"

#include "plGBufferGroup.h"

const hsScalar kPosQuantum = 1.f / hsScalar(1 << 10);
const hsScalar kWeightQuantum = 1.f / hsScalar(1 << 15);
const hsScalar kUVWQuantum = 1.f / hsScalar(1 << 16);

UInt32	plVertCoder::fCodedVerts = 0;
UInt32	plVertCoder::fCodedBytes = 0;
UInt32	plVertCoder::fRawBytes = 0;
UInt32	plVertCoder::fSkippedBytes = 0;

static const hsScalar kQuanta[plVertCoder::kNumFloatFields] =
{
	kPosQuantum,
	kWeightQuantum,
	
	kUVWQuantum,
	kUVWQuantum,
	kUVWQuantum,
	kUVWQuantum,

	kUVWQuantum,
	kUVWQuantum,
	kUVWQuantum,
	kUVWQuantum,
};


inline void plVertCoder::ICountFloats(const UInt8* src, UInt16 maxCnt, const hsScalar quant, const UInt32 stride, 
									  hsScalar& lo, hsBool &allSame, UInt16& count)
{
	lo = *(hsScalar*)src;
	lo = floor(lo / quant + 0.5f) * quant;
	allSame = false;
	hsScalar hi = lo;
	
	count = 1;

	const hsScalar maxRange = hsScalar(UInt16(0xffff)) * quant;

	src += stride;
	maxCnt--;

	while( maxCnt-- )
	{
		hsScalar val = *(hsScalar*)src;
		val = floor(val / quant + 0.5f) * quant;
		if( val < lo )
		{
			if( hi - val > maxRange )
				return;
			lo = val;
		}
		else if( val > hi )
		{
			if( val - lo > maxRange )
				return;
			hi = val;
		}
		count++;
		src += stride;
	}
	allSame = (lo == hi);
}

static inline void IWriteFloat(hsStream* s, const UInt8*& src, const hsScalar offset, const hsScalar quantum)
{
	float fval = *(float*)src;
	fval -= offset;
	fval /= quantum;
//	hsAssert(fval < hsScalar(UInt16(0xffff)), "Bad offset?");

	const UInt16 ival = UInt16(floor(fval + 0.5f));
	s->WriteSwap16(ival);

	src += 4;
}

static inline void IReadFloat(hsStream* s, UInt8*& dst, const hsScalar offset, const hsScalar quantum)
{
	const UInt16 ival = s->ReadSwap16();
	float fval = float(ival) * quantum;
	fval += offset;

	hsScalar* val = (hsScalar*)dst;
	*val = fval;

	dst += 4;
}

inline void plVertCoder::IEncodeFloat(hsStream* s, const UInt32 vertsLeft, const int field, const int chan, const UInt8*& src, const UInt32 stride)
{
	if( !fFloats[field][chan].fCount )
	{
		ICountFloats(src, (UInt16)vertsLeft, kQuanta[field], stride, fFloats[field][chan].fOffset, fFloats[field][chan].fAllSame, fFloats[field][chan].fCount);

		s->WriteSwapScalar(fFloats[field][chan].fOffset);
		s->WriteBool(fFloats[field][chan].fAllSame);
		s->WriteSwap16(fFloats[field][chan].fCount);

	}
	if (!fFloats[field][chan].fAllSame)
		IWriteFloat(s, src, fFloats[field][chan].fOffset, kQuanta[field]);
	else
		src += 4;

	fFloats[field][chan].fCount--;
}

inline void plVertCoder::IDecodeFloat(hsStream* s, const int field, const int chan, UInt8*& dst, const UInt32 stride)
{
	if( !fFloats[field][chan].fCount )
	{
		fFloats[field][chan].fOffset = s->ReadSwapScalar();
		fFloats[field][chan].fAllSame = s->ReadBool();
		fFloats[field][chan].fCount = s->ReadSwap16();
	}

	if (!fFloats[field][chan].fAllSame)
		IReadFloat(s, dst, fFloats[field][chan].fOffset, kQuanta[field]);
	else
	{
		*((hsScalar*)dst) = fFloats[field][chan].fOffset;
		dst += 4;
	}

	fFloats[field][chan].fCount--;
}

static inline int INumWeights(const UInt8 format)
{
	return (format & plGBufferGroup::kSkinWeightMask) >> 4;
}

static const hsScalar kNormalScale(Int16(0x7fff));
static const hsScalar kInvNormalScale(1.f / kNormalScale);

inline void plVertCoder::IEncodeNormal(hsStream* s, const UInt8*& src, const UInt32 stride)
{

	hsScalar x = *(hsScalar*)src;
	s->WriteByte((UInt8)((x / 2.f + .5f) * 255.9f));
	src += 4;

	x = *(hsScalar*)src;
	s->WriteByte((UInt8)((x / 2.f + .5f) * 255.9f));
	src += 4;

	x = *(hsScalar*)src;
	s->WriteByte((UInt8)((x / 2.f + .5f) * 255.9f));
	src += 4;
}

inline void plVertCoder::IDecodeNormal(hsStream* s, UInt8*& dst, const UInt32 stride)
{

	UInt8 ix = s->ReadByte();
	hsScalar* x = (hsScalar*)dst;
	*x = (ix / 255.9f - .5f) * 2.f;
	dst += 4;

	ix = s->ReadByte();
	x = (hsScalar*)dst;
	*x = (ix / 255.9f - .5f) * 2.f;
	dst += 4;

	ix = s->ReadByte();
	x = (hsScalar*)dst;
	*x = (ix / 255.9f - .5f) * 2.f;
	dst += 4;
}

inline void plVertCoder::ICountBytes(const UInt32 vertsLeft, const UInt8* src, const UInt32 stride, UInt16& len, UInt8& same)
{
	// We want to run length encode this. So we're looking here for either
	// the number of consecutive bytes of the same value,
	// or the number of consective bytes of different values.
	// The latter is so we don't wind up getting larger when there aren't any 
	// runs of the same value (count=1 and val=c1, count=1 and val=c2, etc.).
	// The break-even point is a run of 3, so we'll look for a minimum run of 4.

	if( vertsLeft < 4 )
	{
		len = (UInt16)vertsLeft;
		same = false;
		
		return;
	}

	// First, count how many values are the same as the first one
	int i;
	for( i = 0; i < vertsLeft; i++ )
	{
		if( src[i * stride] != src[0] )
			break;
	}

	if( i >= 4 )
	{
		// Found a good run.
		len = i;
		same = true;

		return;
	}

	// Okay, we're in a section of varying values. How far to the next 
	// section of sameness?
	same = false;
	for( ; i < vertsLeft-4; i++ )
	{
		if( (src[i*stride] == src[(i+1)*stride])
			&&(src[i*stride] == src[(i+2)*stride])
			&&(src[i*stride] == src[(i+3)*stride]) )
			break;
	}

	if( i < vertsLeft-4 )
	{
		len = i;
		return;
	}

	len = (UInt16)vertsLeft;
	return;
}

static const UInt16 kSameMask(0x8000);

inline void plVertCoder::IEncodeByte(hsStream* s, const int chan, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride)
{
	if( !fColors[chan].fCount )
	{
		ICountBytes(vertsLeft, src, stride, fColors[chan].fCount, fColors[chan].fSame);

		UInt16 cnt = fColors[chan].fCount;
		if( fColors[chan].fSame )
			cnt |= kSameMask;
		s->WriteSwap16(cnt);

		if( fColors[chan].fSame )
			s->WriteByte(*src);
	}

	if( !fColors[chan].fSame )
		s->WriteByte(*src);
	
	src++;
	fColors[chan].fCount--;
}

inline void plVertCoder::IDecodeByte(hsStream* s, const int chan, UInt8*& dst, const UInt32 stride)
{
	if( !fColors[chan].fCount )
	{
		UInt16 cnt = s->ReadSwap16();
		if( cnt & kSameMask )
		{
			fColors[chan].fSame = true;
			fColors[chan].fVal = s->ReadByte();

			cnt &= ~kSameMask;
		}
		else
		{
			fColors[chan].fSame = false;
		}
		fColors[chan].fCount = cnt;
	}
	if( !fColors[chan].fSame )
		*dst = s->ReadByte();
	else
		*dst = fColors[chan].fVal;

	dst++;
	fColors[chan].fCount--;
}

inline void plVertCoder::IEncodeColor(hsStream* s, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride)
{
	IEncodeByte(s, 0, vertsLeft, src, stride);
	IEncodeByte(s, 1, vertsLeft, src, stride);
	IEncodeByte(s, 2, vertsLeft, src, stride);
	IEncodeByte(s, 3, vertsLeft, src, stride);
}

inline void plVertCoder::IDecodeColor(hsStream* s, UInt8*& dst, const UInt32 stride)
{
	IDecodeByte(s, 0, dst, stride);
	IDecodeByte(s, 1, dst, stride);
	IDecodeByte(s, 2, dst, stride);
	IDecodeByte(s, 3, dst, stride);
}

inline void plVertCoder::IEncode(hsStream* s, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride, const UInt8 format)
{
	IEncodeFloat(s, vertsLeft, kPosition, 0, src, stride);
	IEncodeFloat(s, vertsLeft, kPosition, 1, src, stride);
	IEncodeFloat(s, vertsLeft, kPosition, 2, src, stride);

	// Weights and indices?
	const int numWeights = INumWeights(format);
	if( numWeights )
	{
		int j;
		for( j = 0; j < numWeights; j++ )
			IEncodeFloat(s, vertsLeft, kWeight, j, src, stride);

		if( format & plGBufferGroup::kSkinIndices )
		{
			const UInt32 idx = *(UInt32*)src;
			s->WriteSwap32(idx);
			src += 4;
		}
	}

	IEncodeNormal(s, src, stride);

	IEncodeColor(s, vertsLeft, src, stride);

	// COLOR2
	src += 4;

	const int numUVWs = format & plGBufferGroup::kUVCountMask;
	int i;
	for( i = 0; i < numUVWs; i++ )
	{
		IEncodeFloat(s, vertsLeft, kUVW + i, 0, src, stride);
		IEncodeFloat(s, vertsLeft, kUVW + i, 1, src, stride);
		IEncodeFloat(s, vertsLeft, kUVW + i, 2, src, stride);
	}
}

inline void plVertCoder::IDecode(hsStream* s, UInt8*& dst, const UInt32 stride, const UInt8 format)
{
	IDecodeFloat(s, kPosition, 0, dst, stride);
	IDecodeFloat(s, kPosition, 1, dst, stride);
	IDecodeFloat(s, kPosition, 2, dst, stride);

	// Weights and indices?
	const int numWeights = INumWeights(format);
	if( numWeights )
	{
		int j;
		for( j = 0; j < numWeights; j++ )
			IDecodeFloat(s, kWeight, j, dst, stride);

		if( format & plGBufferGroup::kSkinIndices )
		{
			UInt32* idx = (UInt32*)dst;
			*idx = s->ReadSwap32();
			dst += 4;
		}
	}

	IDecodeNormal(s, dst, stride);

	IDecodeColor(s, dst, stride);

	// COLOR2
	UInt32* trash = (UInt32*)dst;
	*trash = 0;
	dst += 4;

	const int numUVWs = format & plGBufferGroup::kUVCountMask;
	int i;
	for( i = 0; i < numUVWs; i++ )
	{
		IDecodeFloat(s, kUVW + i, 0, dst, stride);
		IDecodeFloat(s, kUVW + i, 1, dst, stride);
		IDecodeFloat(s, kUVW + i, 2, dst, stride);
	}
}

void plVertCoder::Read(hsStream* s, UInt8* dst, const UInt8 format, const UInt32 stride, const UInt16 numVerts)
{
	Clear();

	int i = numVerts;
	for( i = 0; i < numVerts; i++ )
		IDecode(s, dst, stride, format);
}


void plVertCoder::Write(hsStream* s, const UInt8* src, const UInt8 format, const UInt32 stride, const UInt16 numVerts)
{
	Clear();

	UInt32 streamStart = s->GetPosition();

	int numLeft = numVerts;
	while( numLeft )
	{
		IEncode(s, numLeft, src, stride, format);
		numLeft--;
	}

	fCodedVerts += numVerts;
	fCodedBytes += (s->GetPosition() - streamStart);
	fRawBytes += numVerts * stride;
}

plVertCoder::plVertCoder()
{
	Clear();
}

plVertCoder::~plVertCoder()
{
}

void plVertCoder::Clear()
{
	memset(this, 0, sizeof(*this));
}

