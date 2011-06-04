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

#ifndef plSpanTemplate_inc
#define plSpanTemplate_inc

#include "hsGeometry3.h"
#include "hsBounds.h"
#include "hsColorRGBA.h"
#include "plRenderLevel.h"

class hsStream;

class INode;
class hsGMaterial;

class plSpanTemplate
{
public:
	// 99% of the time, the defaults are fine. Just tell me
	// how many UVWs, and whether you've got color.
	static UInt16 MakeFormat(hsBool hasColor, int numUVWs,
							hsBool hasWgtIdx = false,
							int numWgts = 0,
							hsBool hasNorm = true,
							hsBool hasPos = true,
							hsBool hasColor2 = true)
	{
		return (hasPos ? kPosMask : 0)
			| (hasNorm ? kNormMask : 0)
			| (hasColor ? kColorMask : 0)
			| (hasWgtIdx ? kWgtIdxMask : 0)
			| ((numUVWs << 4) & kUVWMask)
			| ((numWgts << 8) & kWeightMask)
			| (hasColor2 ? kColor2Mask : 0); // Till we can get this out of here.
	};
	enum
	{
		kPosMask		= 0x1,

		kNormMask		= 0x2,

		kColorMask		= 0x4,

		kWgtIdxMask		= 0x8,

		kUVWMask		= 0xf0,

		kWeightMask		= 0x300,

		kColor2Mask		= 0x400
	};
	enum Channel
	{
		kPosition,
		kWeight,
		kWgtIdx,
		kNormal,
		kColor,
		kColor2,
		kUVW
	};
protected:

	UInt16	fNumVerts;
	UInt16	fFormat;
	UInt8	fStride;
	UInt16	fNumTris;

	// Data stored interleaved. Any channels may be omitted, but
	// existing channels are in exactly the following order:
	//	Position
	//	Weights
	//	Indices
	//	Normal
	//	Color
	//	Color2
	//	UVWs
	UInt8*			fData;

	UInt16*			fIndices;

	friend class plClusterUtil;
public:
	plSpanTemplate();
	virtual ~plSpanTemplate() { DeAlloc(); }

	const UInt8*	VertData() const { return fData; }

	const UInt16*	IndexData() const { return fIndices; }

	UInt32	NumVerts() const { return fNumVerts; }
	UInt32	Stride() const { return UInt32(fStride); }
	UInt32	CalcStride();
	UInt32	VertSize() const { return NumVerts() * Stride(); }

	UInt32	NumTris() const { return fNumTris; }
	UInt32	NumIndices() const { return NumTris() * 3; }
	UInt32	IndexSize() const { return NumIndices() * sizeof(UInt16); }
	
	UInt8	PositionOffset() const { return UInt8(0); }
	UInt8	WgtIdxOffset() const { return UInt8(PositionOffset() + NumPos() * sizeof(hsPoint3)); }
	UInt8	WeightOffset() const { return UInt8(WgtIdxOffset() + NumWgtIdx() * sizeof(UInt32)); }
	UInt8	NormalOffset() const { return UInt8(WeightOffset() + NumWeights() * sizeof(hsScalar)); }
	UInt8	ColorOffset() const { return UInt8(NormalOffset() + NumNorm() * sizeof(hsVector3)); }
	UInt8	Color2Offset() const { return UInt8(ColorOffset() + NumColor() * sizeof(UInt32)); }
	UInt8	UVWOffset() const { return UInt8(Color2Offset() + NumColor2() * sizeof(UInt32)); }

	UInt32	NumUVWs() const { return (fFormat & kUVWMask) >> 4; }
	UInt32	NumWeights() const { return (fFormat & kWeightMask) >> 8; }

	UInt32	NumPos() const { return (fFormat & kPosMask) >> 0; }
	UInt32	NumNorm() const { return (fFormat & kNormMask) >> 1; }
	UInt32	NumColor() const { return (fFormat & kColorMask) >> 2; }
	UInt32	NumColor2() const { return (fFormat & kColor2Mask) >> 10; }
	UInt32	NumWgtIdx() const { return (fFormat & kWgtIdxMask) >> 3; }

	hsPoint3*			Position(int i) const { return (hsPoint3*)GetData(kPosition, i); }
	hsVector3*			Normal(int i) const { return (hsVector3*)GetData(kNormal, i); }
	UInt32*				Color(int i) const { return (UInt32*)GetData(kColor, i); }
	UInt32*				Color2(int i) const { return (UInt32*)GetData(kColor2, i); }
	UInt32*				WgtIdx(int i) const { return (UInt32*)GetData(kWgtIdx, i); }
	hsPoint3*			UVWs(int iv, int iuv) const { return (hsPoint3*)GetData(kUVW, iv, iuv); }
	hsScalar*			Weight(int iv, int iw) const { return (hsScalar*)GetData(kWeight, iv, iw); }

	UInt8*				GetData(Channel chan, int i, int j=0) const
	{
		ValidateInput(chan, i, j);
		UInt8* base = fData + i * fStride;
		switch(chan)
		{
		case kPosition:
			return base;
		case kWeight:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ j * sizeof(hsScalar);
		case kWgtIdx:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ NumWeights() * sizeof(hsScalar);
		case kNormal:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ NumWeights() * sizeof(hsScalar)
				+ NumWgtIdx() * sizeof(UInt32);
		case kColor:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ NumWeights() * sizeof(hsScalar)
				+ NumWgtIdx() * sizeof(UInt32)
				+ NumNorm() * sizeof(hsVector3);
		case kColor2:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ NumWeights() * sizeof(hsScalar)
				+ NumWgtIdx() * sizeof(UInt32)
				+ NumNorm() * sizeof(hsVector3)
				+ NumColor() * sizeof(UInt32);
		case kUVW:
			return base 
				+ NumPos() * sizeof(hsPoint3)
				+ NumWeights() * sizeof(hsScalar)
				+ NumWgtIdx() * sizeof(UInt32)
				+ NumNorm() * sizeof(hsVector3)
				+ NumColor() * sizeof(UInt32)
				+ NumColor2() * sizeof(UInt32)
				+ j * sizeof(hsPoint3);
		}
		hsAssert(false, "Unrecognized vertex channel");
		return nil;
	}

	hsBool ValidateInput(Channel chan, int i, int j) const
	{
		switch(chan)
		{
		case kPosition:
			hsAssert(NumPos(), "Invalid data request");
			return NumPos() > 0;
		case kWeight:
			hsAssert(NumWeights(), "Invalid data request");
			return NumWeights() > 0;
		case kWgtIdx:
			hsAssert(NumWgtIdx() > j, "Invalid data request");
			return NumWgtIdx() > j;
		case kNormal:
			hsAssert(NumNorm(), "Invalid data request");
			return NumNorm() > 0;
		case kColor:
			hsAssert(NumColor(), "Invalid data request");
			return NumColor() > 0;
		case kColor2:
			hsAssert(NumColor2(), "Invalid data request");
			return NumColor2() > 0;
		case kUVW:
			hsAssert(NumUVWs() > j, "Invalid data request");
			return NumUVWs() > j;
		}
		hsAssert(false, "Unrecognized vertex channel");
		return false;
	}
	
	void Alloc(UInt16 format, UInt32 numVerts, UInt32 numTris);
	void DeAlloc();

	void Read(hsStream* s);
	void Write(hsStream* s) const;
};

// An export only version
class plSpanTemplateB : public plSpanTemplate
{
	INode*			fSrc;
	hsBounds3Ext	fLocalBounds;

	hsColorRGBA*	fMultColors;
	hsColorRGBA*	fAddColors;

public:
	plSpanTemplateB(INode* src) : plSpanTemplate(), fSrc(src), fMaterial(nil) {}
	virtual ~plSpanTemplateB() { DeAllocColors(); }

	void ComputeBounds();

	const hsBounds3Ext& GetLocalBounds() const { return fLocalBounds; }

	INode*			GetSrcNode() const { return fSrc; }

	hsGMaterial*	fMaterial;

	plRenderLevel	fRenderLevel;

	hsColorRGBA*	MultColor(int i) const { return &fMultColors[i]; }
	hsColorRGBA*	AddColor(int i) const { return &fAddColors[i]; }

	void AllocColors();
	void DeAllocColors();
};

#endif // plSpanTemplate_inc
