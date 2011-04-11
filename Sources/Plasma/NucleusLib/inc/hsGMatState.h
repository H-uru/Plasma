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
#ifndef hsGMatState_inc
#define hsGMatState_inc

#include "hsColorRGBA.h"

class hsStream;

class hsGMatState {
public:
enum hsGMatBlendFlags {
	kBlendTest	= 0x1,							// dev
	// Rest of blends are mutually exclusive
	kBlendAlpha						= 0x2,		// dev
	kBlendMult						= 0x4,		// dev
	kBlendAdd						= 0x8,		// dev
	kBlendAddColorTimesAlpha		= 0x10,		// dev
	kBlendAntiAlias					= 0x20,
	kBlendDetail					= 0x40,
	kBlendNoColor					= 0x80,		// dev
	kBlendMADD						= 0x100,
	kBlendDot3						= 0x200,
	kBlendAddSigned					= 0x400,
	kBlendAddSigned2X				= 0x800,
	kBlendMask						= kBlendAlpha
									| kBlendMult
									| kBlendAdd
									| kBlendAddColorTimesAlpha
									| kBlendDetail
									| kBlendMADD
									| kBlendDot3
									| kBlendAddSigned
									| kBlendAddSigned2X,
	kBlendInvertAlpha				= 0x1000,	// dev
	kBlendInvertColor				= 0x2000,	// dev
	kBlendAlphaMult					= 0x4000,
	kBlendAlphaAdd					= 0x8000,
	kBlendNoVtxAlpha				= 0x10000,
	kBlendNoTexColor				= 0x20000,
	kBlendNoTexAlpha				= 0x40000,
	kBlendInvertVtxAlpha			= 0x80000,	// Invert ONLY the vertex alpha source
	kBlendAlphaAlways				= 0x100000,	// Alpha test always passes (even for alpha=0).
	kBlendInvertFinalColor			= 0x200000,
	kBlendInvertFinalAlpha			= 0x400000,
	kBlendEnvBumpNext				= 0x800000,
	kBlendSubtract					= 0x1000000,
	kBlendRevSubtract				= 0x2000000,
	kBlendAlphaTestHigh				= 0x4000000
};
enum hsGMatClampFlags {

	kClampTextureU	= 0x1,			// dev
	kClampTextureV	= 0x2,			// dev
	kClampTexture	= 0x3				// dev
};

enum hsGMatShadeFlags {

	kShadeSoftShadow		= 0x1,			// view, dev
	kShadeNoProjectors		= 0x2,			// projector
	kShadeEnvironMap		= 0x4,			// dev, load
	kShadeVertexShade		= 0x20,			// dev
	kShadeNoShade			= 0x40,			// view,dev
	kShadeBlack				= kShadeNoShade,
	kShadeSpecular			= 0x80,			// view, dev
	//kShadeNoFog				= 0x100,		// dev
	kShadeWhite				= 0x200,
	kShadeSpecularAlpha		= 0x400,
	kShadeSpecularColor		= 0x800,
	kShadeSpecularHighlight	= 0x1000,
	kShadeVertColShade		= 0x2000,
    kShadeInherit           = 0x4000,
	kShadeIgnoreVtxIllum	= 0x8000,
	kShadeEmissive			= 0x10000,		// Moved here 8.27 mcn. Only really sane to use with kMiscEndPassHere
	kShadeReallyNoFog		= 0x20000
};

enum hsGMatZFlags {
	kZIncLayer			= 0x1, // dev
	kZClearZ			= 0x4, // dev
	kZNoZRead			= 0x8, // dev
	kZNoZWrite			= 0x10,
	kZMask				= kZNoZWrite | kZClearZ | kZNoZRead,
	kZLODBias			= 0x20
};

enum hsGMatMiscFlags {
	kMiscWireFrame			= 0x1,			// dev (running out of bits)
	kMiscDrawMeshOutlines	= 0x2,			// dev, currently unimplemented
	kMiscTwoSided			= 0x4,			// view,dev
	kMiscDrawAsSplats		= 0x8,			// dev? bwt
	kMiscAdjustPlane		= 0x10,
	kMiscAdjustCylinder		= 0x20,
	kMiscAdjustSphere		= 0x40,
	kMiscAdjust				= kMiscAdjustPlane | kMiscAdjustCylinder| kMiscAdjustSphere,
    kMiscTroubledLoner      = 0x80,
	kMiscBindSkip			= 0x100,
	kMiscBindMask			= 0x200,
	kMiscBindNext			= 0x400,
	kMiscLightMap			= 0x800,
	kMiscUseReflectionXform	= 0x1000,		// Use the calculated reflection environment 
											// texture transform instead of layer->GetTransform()
	kMiscPerspProjection	= 0x2000,
	kMiscOrthoProjection	= 0x4000,
	kMiscProjection			= kMiscPerspProjection | kMiscOrthoProjection,

	kMiscRestartPassHere	= 0x8000,		// Tells pipeline to start a new pass beginning with this layer
											// Kinda like troubledLoner, but only cuts off lower layers, not 
											// higher ones (kMiscBindNext sometimes does this by implication)

	kMiscBumpLayer			= 0x10000,
	kMiscBumpDu				= 0x20000,
	kMiscBumpDv				= 0x40000,
	kMiscBumpDw				= 0x80000,
	kMiscBumpChans			= kMiscBumpDu | kMiscBumpDv | kMiscBumpDw,

	kMiscNoShadowAlpha		= 0x100000,
	kMiscUseRefractionXform	= 0x200000, // Use a refraction-like hack.
	kMiscCam2Screen         = 0x400000, // Expects tex coords to be XYZ in camera space. Does a cam to screen (not NDC) projection
										// and swaps Z with W, so that the texture projection can produce projected 2D screen coordinates.

	kAllMiscFlags			= 0xffffffff
};
enum StateIdx {
	kBlend,
	kClamp,
	kShade,
	kZ,
	kMisc 
};
	UInt32			fBlendFlags;
	UInt32			fClampFlags;
	UInt32			fShadeFlags;
	UInt32			fZFlags;
	UInt32			fMiscFlags;

	static hsBool Differs(UInt32 mine, UInt32 hers, UInt32 mask)
	{
		return (mine & mask) ^ (hers & mask);
	}

	static hsBool Differs(UInt32 mine, UInt32 hers)
	{
		return mine ^ hers;
	}

	hsBool operator!=(const hsGMatState& other)
	{
		return ((fBlendFlags ^ other.fBlendFlags)
			| (fClampFlags ^ other.fClampFlags)
			| (fShadeFlags ^ other.fShadeFlags)
			| (fZFlags ^ other.fZFlags)
			| (fMiscFlags ^ other.fMiscFlags));
	}
	UInt32 Value(int i) const
	{
		switch(i)
		{
		case kBlend:
			return fBlendFlags;
		case kClamp:
			return fClampFlags;
		case kShade:
			return fShadeFlags;
		case kZ:
			return fZFlags;
		case kMisc:
			return fMiscFlags;
		}
		hsAssert(false, "Bad param");
		return fBlendFlags;
	}
	UInt32& operator[](const int i)
	{
		switch(i)
		{
		case kBlend:
			return fBlendFlags;
		case kClamp:
			return fClampFlags;
		case kShade:
			return fShadeFlags;
		case kZ:
			return fZFlags;
		case kMisc:
			return fMiscFlags;
		}
		hsAssert(false, "Bad param");
		return fBlendFlags;
	}
	hsGMatState& operator|=(const hsGMatState& other)
	{
		fBlendFlags |= other.fBlendFlags;
		fClampFlags |= other.fClampFlags;
		fShadeFlags |= other.fShadeFlags;
		fZFlags |= other.fZFlags;
		fMiscFlags |= other.fMiscFlags;
		return *this;
	}
	hsGMatState& operator+=(const hsGMatState& other)
	{
		return operator|=(other);
	}
	hsGMatState& operator-=(const hsGMatState& other)
	{
		fBlendFlags &= ~other.fBlendFlags;
		fClampFlags &= ~other.fClampFlags;
		fShadeFlags &= ~other.fShadeFlags;
		fZFlags &= ~other.fZFlags;
		fMiscFlags &= ~other.fMiscFlags;
		return *this;
	}

	inline void Read(hsStream* s);
	inline void Write(hsStream* s);

	hsGMatState(UInt32 blend=0, UInt32 clamp=0, UInt32 shade=0, UInt32 z=0, UInt32 misc=0) 
		:	fBlendFlags(blend), 
			fClampFlags(clamp),
			fShadeFlags(shade),
			fZFlags(z),
			fMiscFlags(misc) {}
	void Reset() { fBlendFlags = fClampFlags = fShadeFlags = fZFlags = fMiscFlags = 0; }
	inline void Clear(const hsGMatState& state);
	inline void Composite(const hsGMatState& want, const hsGMatState& on, const hsGMatState& off);
};


#endif // hsGMatState_inc
