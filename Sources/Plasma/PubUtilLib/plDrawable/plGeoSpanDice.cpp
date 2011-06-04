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

#include "memory.h"

#include "hsTypes.h"
#include "plGeoSpanDice.h"
#include "plGeometrySpan.h"

plGeoSpanDice::plGeoSpanDice()
:	fMinFaces(0),
	fMaxFaces(0)
{
	fMaxSize.Set(0,0,0);
}

plGeoSpanDice::~plGeoSpanDice()
{
}

hsBool plGeoSpanDice::Dice(hsTArray<plGeometrySpan*>& spans) const
{
	int startingCount = spans.GetCount();

	hsTArray<plGeometrySpan*> out;
	hsTArray<plGeometrySpan*> next;

	while(spans.GetCount())
	{
		int i;
		for( i = 0; i < spans.GetCount(); i++ )
		{
			if( !IHalf(spans[i], next) )
			{
				out.Append(spans[i]);
			}
		}
		spans.Swap(next);
		next.SetCount(0);
	}
	spans.Swap(out);

	return spans.GetCount() != startingCount;
}

hsBool plGeoSpanDice::INeedSplitting(plGeometrySpan* src) const
{
	// Do we have enough faces to bother?
	if( fMinFaces )
	{
		if( src->fNumIndices < fMinFaces * 3 )
			return false;
	}
	// Do we have enough faces to bother no matter how small we are?
	if( fMaxFaces )
	{
		if( src->fNumIndices > fMaxFaces * 3 )
			return true;
	}
	// Are we big enough to bother?
	if( (fMaxSize.fX > 0) || (fMaxSize.fY > 0) || (fMaxSize.fZ > 0) )
	{
		hsPoint3 size = src->fLocalBounds.GetMaxs() - src->fLocalBounds.GetMins();
		if( size.fX > fMaxSize.fX )
			return true;
		if( size.fY > fMaxSize.fY )
			return true;
		if( size.fZ > fMaxSize.fZ )
			return true;
	}

	return false;
}

hsBool plGeoSpanDice::IHalf(plGeometrySpan* src, hsTArray<plGeometrySpan*>& out, int exclAxis) const
{
	if( !INeedSplitting(src) )
		return false;

	int iAxis = ISelectAxis(exclAxis, src);
	// Ran out of axes to try.
	if( iAxis < 0 )
		return false;

	hsScalar midPoint = src->fLocalBounds.GetCenter()[iAxis];

	hsTArray<UInt32>		loTris;
	hsTArray<UInt32>		hiTris;

	UInt16* indexData = src->fIndexData;

	int numTris = src->fNumIndices / 3;

	int stride = src->GetVertexSize(src->fFormat);

	int i;
	for( i = 0; i < numTris; i++ )
	{
		hsPoint3& pos0 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);
		hsPoint3& pos1 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);
		hsPoint3& pos2 = *(hsPoint3*)(src->fVertexData + *indexData++ * stride);

		if( (pos0[iAxis] >= midPoint)
			&&(pos1[iAxis] >= midPoint)
			&&(pos2[iAxis] >= midPoint) )
		{
			hiTris.Append(i);
		}
		else
		{
			loTris.Append(i);
		}
	}

	// This axis isn't working out, try another.
	if( !hiTris.GetCount() || !loTris.GetCount() )
		return IHalf(src, out, exclAxis | (1 << iAxis));


	plGeometrySpan* loDst = IExtractTris(src, loTris);
	plGeometrySpan* hiDst = IExtractTris(src, hiTris);

	delete src;

	out.Append(loDst);
	out.Append(hiDst);

	return true;
}

int plGeoSpanDice::ISelectAxis(int exclAxis, plGeometrySpan* src) const
{
	int iAxis = -1;
	hsScalar maxDim = 0;

	int i;
	for( i = 0; i < 3; i++ )
	{
		// Check to see if we've already tried this one.
		if( exclAxis & (1 << i) )
			continue;

		hsScalar dim = src->fLocalBounds.GetMaxs()[i] - src->fLocalBounds.GetMins()[i];
		if( dim > maxDim )
		{
			maxDim = dim;
			iAxis = i;
		}
	}
	return iAxis;
}

plGeometrySpan* plGeoSpanDice::IExtractTris(plGeometrySpan* src, hsTArray<UInt32>& tris) const
{
	// First off, find out how many and which vers we're talking here.
	// Easiest way is while we're building the LUTs we'll want later anyway.
	hsTArray<Int16>	fwdLUT;
	fwdLUT.SetCount(src->fNumVerts);
	memset(fwdLUT.AcquireArray(), -1, src->fNumVerts * sizeof(*fwdLUT.AcquireArray()));

	hsTArray<UInt16>	bckLUT;
	bckLUT.SetCount(0);

	int i;
	for( i = 0; i < tris.GetCount(); i++ )
	{
		UInt16* idx = src->fIndexData + tris[i] * 3;
		
		if( fwdLUT[*idx] < 0 )
		{
			fwdLUT[*idx] = bckLUT.GetCount();
			bckLUT.Append(*idx);
		}

		idx++;

		if( fwdLUT[*idx] < 0 )
		{
			fwdLUT[*idx] = bckLUT.GetCount();
			bckLUT.Append(*idx);
		}

		idx++;

		if( fwdLUT[*idx] < 0 )
		{
			fwdLUT[*idx] = bckLUT.GetCount();
			bckLUT.Append(*idx);
		}
	}

	int numVerts = bckLUT.GetCount();
	int numTris = tris.GetCount();

	plGeometrySpan* dst = IAllocSpace(src, numVerts, numTris);

	// Okay, set the index data.
	UInt16* idxTrav = dst->fIndexData;
	for( i = 0; i < tris.GetCount(); i++ )
	{
		*idxTrav++ = fwdLUT[src->fIndexData[ tris[i] * 3 + 0] ];
		*idxTrav++ = fwdLUT[src->fIndexData[ tris[i] * 3 + 1] ];
		*idxTrav++ = fwdLUT[src->fIndexData[ tris[i] * 3 + 2] ];
	}

	// Copy over the basic vertex data
	// We'll update the bounds as we go.
	int stride = src->GetVertexSize(src->fFormat);

	hsBounds3Ext localBnd;
	localBnd.MakeEmpty();
	UInt8* vtxTrav = dst->fVertexData;
	for( i = 0; i < numVerts; i++ )
	{
		memcpy(vtxTrav, src->fVertexData + bckLUT[i] * stride, stride);		

		hsPoint3* pos = (hsPoint3*)vtxTrav;
		localBnd.Union(pos);
		vtxTrav += stride;
	}
	if( src->fProps & plGeometrySpan::kWaterHeight )
	{
		src->AdjustBounds(localBnd);
	}
	dst->fLocalBounds = localBnd;

	// Now the rest of this optional garbage.
	if( src->fMultColor )
	{
		for( i = 0; i < numVerts; i++ )
		{
			dst->fMultColor[i] = src->fMultColor[bckLUT[i]];
		}
	}
	if( src->fAddColor )
	{
		for( i = 0; i < numVerts; i++ )
		{
			dst->fAddColor[i] = src->fAddColor[bckLUT[i]];
		}
	}
	if( src->fDiffuseRGBA )
	{
		for( i = 0; i < numVerts; i++ )
		{
			dst->fDiffuseRGBA[i] = src->fDiffuseRGBA[bckLUT[i]];
		}
	}
	if( src->fSpecularRGBA )
	{
		for( i = 0; i < numVerts; i++ )
		{
			dst->fSpecularRGBA[i] = src->fSpecularRGBA[bckLUT[i]];
		}
	}
	dst->fMaxOwner = src->fMaxOwner;

	return dst;
}

plGeometrySpan* plGeoSpanDice::IAllocSpace(plGeometrySpan* src, int numVerts, int numTris) const
{
	plGeometrySpan* dst = TRACKED_NEW plGeometrySpan;
	// Do a structure copy here. That's okay, because we're going to immediately 
	// fix up the pointers and counters that shouldn't have been copied. If
	// plGeometrySpan ever gets a copy constructor, this'll blow wide open.
	*dst = *src;

	int stride = src->GetVertexSize(src->fFormat);

	dst->fNumIndices = numTris * 3;
	dst->fIndexData = TRACKED_NEW UInt16[dst->fNumIndices];

	dst->fNumVerts = numVerts;
	dst->fVertexData = TRACKED_NEW UInt8[numVerts * stride];

	if( src->fMultColor )
	{
		dst->fMultColor = TRACKED_NEW hsColorRGBA[numVerts];
	}
	if( src->fAddColor )
	{
		dst->fAddColor = TRACKED_NEW hsColorRGBA[numVerts];
	}
	if( src->fDiffuseRGBA )
	{
		dst->fDiffuseRGBA = TRACKED_NEW UInt32[numVerts];
	}
	if( src->fSpecularRGBA )
	{
		dst->fSpecularRGBA = TRACKED_NEW UInt32[numVerts];
	}

	return dst;
}