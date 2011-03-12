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
#include "plSpanTemplate.h"
#include "plSpanInstance.h"

#include "hsStream.h"
#include "hsGeometry3.h"



plSpanTemplate::plSpanTemplate()
:	fNumVerts(0),
	fFormat(0),
	fData(nil),
	fNumTris(0),
	fIndices(nil)
{
}

UInt32 plSpanTemplate::CalcStride() 
{
	fStride = 0;
	if( NumPos() )
		fStride += sizeof(hsPoint3);
	if( NumNorm() )
		fStride += sizeof(hsVector3);
	if( NumColor() )
		fStride += sizeof(UInt32);
	if( NumColor2() )
		fStride += sizeof(UInt32);
	if( NumWgtIdx() )
		fStride += sizeof(UInt32);
	if( NumUVWs() )
		fStride += (UInt8)(sizeof(hsPoint3) * NumUVWs());
	if( NumWeights() )
		fStride += (UInt8)(sizeof(hsScalar) * NumWeights());

	return UInt32(fStride);
}

void plSpanTemplate::Alloc(UInt16 format, UInt32 numVerts, UInt32 numTris)
{
	DeAlloc();
	fNumVerts = (UInt16)numVerts;
	fFormat = format;
	CalcStride();

	fNumTris = (UInt16)numTris;

	fData = TRACKED_NEW UInt8[VertSize()];

	fIndices = TRACKED_NEW UInt16[NumIndices()];
}

void plSpanTemplate::DeAlloc()
{
	delete [] fData;
	delete [] fIndices;

	fNumTris = 0;
	fNumVerts = 0;
	fFormat = 0;

	fData = nil;
	fIndices = nil;
}

void plSpanTemplate::Read(hsStream* stream)
{
	fNumVerts = stream->ReadSwap16();
	fFormat = stream->ReadSwap16();
	fNumTris = stream->ReadSwap16();

	Alloc(fFormat, fNumVerts, fNumTris);

	stream->Read(VertSize(), fData);
	stream->Read(IndexSize(), fIndices);
}

void plSpanTemplate::Write(hsStream* stream) const
{
	stream->WriteSwap16(fNumVerts);
	stream->WriteSwap16(fFormat);
	stream->WriteSwap16(fNumTris);

	stream->Write(VertSize(), fData);
	stream->Write(IndexSize(), fIndices);
}


void plSpanTemplateB::ComputeBounds()
{
	fLocalBounds.MakeEmpty();
	int i;
	for( i = 0; i < NumVerts(); i++ )
		fLocalBounds.Union(Position(i));
}

void plSpanTemplateB::AllocColors()
{
	fMultColors = TRACKED_NEW hsColorRGBA[NumVerts()];
	fAddColors = TRACKED_NEW hsColorRGBA[NumVerts()];
}

void plSpanTemplateB::DeAllocColors()
{
	delete [] fMultColors;
	delete [] fAddColors;

	fMultColors = nil;
	fAddColors = nil;
}
