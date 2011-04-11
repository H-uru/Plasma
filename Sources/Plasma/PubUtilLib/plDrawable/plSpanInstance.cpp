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
#include "plSpanInstance.h"

#include "hsGeometry3.h"
#include "hsStream.h"

void plSpanEncoding::Read(hsStream* s)
{
	fCode = s->ReadByte();
	fPosScale = s->ReadSwapScalar();
}

void plSpanEncoding::Write(hsStream* s) const
{
	s->WriteByte((UInt8)fCode);
	s->WriteSwapScalar(fPosScale);
}


plSpanInstance::plSpanInstance()
:	fPosDelta(nil),
	fCol(nil)
{
	fL2W[0][1] = fL2W[0][2] = 0.f;
	fL2W[1][0] = fL2W[1][2] = 0.f;
	fL2W[2][0] = fL2W[2][1] = 0.f;
	fL2W[0][0] = fL2W[1][1] = fL2W[2][2] = 1.f;
}

plSpanInstance::~plSpanInstance()
{
	DeAlloc();
}

void plSpanInstance::DeAlloc()
{
	delete [] fPosDelta;
	fPosDelta = nil;
	delete [] fCol;
	fCol = nil;
}

void plSpanInstance::Alloc(const plSpanEncoding& encoding, UInt32 numVerts)
{
	DeAlloc();
	UInt32 posStride = PosStrideFromEncoding(encoding);
	if( posStride )
		fPosDelta = TRACKED_NEW UInt8[numVerts * posStride];

	UInt32 colStride = ColStrideFromEncoding(encoding);
	if( colStride )
		fCol = TRACKED_NEW UInt8[numVerts * colStride];
}

void plSpanInstance::Read(hsStream* stream, const plSpanEncoding& encoding, UInt32 numVerts)
{
	Alloc(encoding, numVerts);

	stream->Read(12 * sizeof(hsScalar), fL2W[0]);
	if( fPosDelta )
	{
		stream->Read(numVerts * PosStrideFromEncoding(encoding), fPosDelta);
	}

	if( fCol )
	{
		stream->Read(numVerts * ColStrideFromEncoding(encoding), fCol);
	}
}

void plSpanInstance::Write(hsStream* stream, const plSpanEncoding& encoding, UInt32 numVerts) const
{
	stream->Write(12 * sizeof(hsScalar), fL2W[0]);
	if( fPosDelta )
	{
		stream->Write(numVerts * PosStrideFromEncoding(encoding), fPosDelta);
	}
	if( fCol )
	{
		stream->Write(numVerts * ColStrideFromEncoding(encoding), fCol);
	}
}

hsMatrix44 plSpanInstance::LocalToWorld() const
{
	hsMatrix44 retVal;
	retVal.NotIdentity();
	int i;
	for( i = 0; i < 3; i++ )
	{
		int j;
		for( j = 0; j < 4; j++ )
		{
			retVal.fMap[i][j] = fL2W[i][j];
		}
	}
	retVal.fMap[3][0] =
		retVal.fMap[3][1] =
		retVal.fMap[3][2] = 0.f;
	retVal.fMap[3][3] = 1.f;

	return retVal;
}

hsMatrix44 plSpanInstance::WorldToLocal() const
{
	hsMatrix44 l2w = LocalToWorld();
	hsMatrix44 w2l;
	l2w.GetInverse(&w2l);
	return w2l;
}

void plSpanInstance::SetLocalToWorld(const hsMatrix44& l2w)
{
	int i;
	for( i = 0; i < 3; i++ )
	{
		int j;
		for( j = 0; j < 4; j++ )
		{
			fL2W[i][j] = l2w.fMap[i][j];
		}
	}
}

void plSpanInstance::Encode(const plSpanEncoding& encoding, UInt32 numVerts, const hsVector3* delPos, const UInt32* color)
{
	Alloc(encoding, numVerts);

	hsAssert(!(encoding.fCode & plSpanEncoding::kPosMask) == !delPos, "Position encoding mismatch");
	hsAssert(!(encoding.fCode & plSpanEncoding::kColMask) == !color, "Color encoding mismatch");

	// Check that there's anything to encode.
	if( !(fPosDelta || fCol) )
		return;

	Int8* pos888 = (Int8*)fPosDelta;
	Int16* pos161616 = (Int16*)fPosDelta;
	UInt32* pos101010 = (UInt32*)fPosDelta;

	UInt8* col8 = (UInt8*)fCol;
	UInt16* col16 = (UInt16*)fCol;
	UInt32*	col32 = (UInt32*)fCol;
	int i;
	for( i = 0; i < numVerts; i++ )
	{
		switch(encoding.fCode & plSpanEncoding::kPosMask)
		{
		case plSpanEncoding::kPos888:
			pos888[0] = Int8(delPos->fX / encoding.fPosScale);
			pos888[1] = Int8(delPos->fY / encoding.fPosScale);
			pos888[2] = Int8(delPos->fZ / encoding.fPosScale);
			pos888 += 3;
			delPos++;
			break;
		case plSpanEncoding::kPos161616:
			pos161616[0] = Int16(delPos->fX / encoding.fPosScale);
			pos161616[1] = Int16(delPos->fY / encoding.fPosScale);
			pos161616[2] = Int16(delPos->fZ / encoding.fPosScale);
			pos161616 += 3;
			delPos++;
			break;
		case plSpanEncoding::kPos101010:
			*pos101010 =
				((UInt32(delPos->fX / encoding.fPosScale) & 0x3f) << 0)
				| ((UInt32(delPos->fY / encoding.fPosScale) & 0x3f) << 10)
				| ((UInt32(delPos->fZ / encoding.fPosScale) & 0x3f) << 20);
			pos101010++;
			delPos++;
			break;
		case plSpanEncoding::kPos008:
			*pos888 = Int8(delPos->fZ / encoding.fPosScale);
			pos888++;
			delPos++;
			break;
		case plSpanEncoding::kPosNone:
		default:
			break;
		}
		switch(encoding.fCode & plSpanEncoding::kColMask)
		{
		case plSpanEncoding::kColA8:
			*col8 = (UInt8)((*color) >> 24);
			col8++;
			color++;
			break;
		case plSpanEncoding::kColI8:
			*col8 = (UInt8)((*color) & 0xff);
			col8++;
			color++;
			break;
		case plSpanEncoding::kColAI88:
			*col16 = (UInt16)(((*color) >> 16) & 0xffff);
			col16++;
			color++;
			break;
		case plSpanEncoding::kColRGB888:
			*col8++ = (UInt8)((*color >> 16) & 0xff);
			*col8++ = (UInt8)((*color >> 8) & 0xff);
			*col8++ = (UInt8)((*color >> 0) & 0xff);
			color++;
			break;
		case plSpanEncoding::kColARGB8888:
			*col32++ = *color;
			color++;
			break;
		case plSpanEncoding::kColNone:
		default:
			break;
		}
	}
}
