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
#ifndef __HSMAXLAYERBASE_H
#define __HSMAXLAYERBASE_H

#include "stdmat.h"

#define HSMAX_LAYER_CLASS_ID 0x41990fe7

const Class_ID hsMaxLayerClassID(HSMAX_LAYER_CLASS_ID, 0x72404998);
const Class_ID hsMaxMtlClassID(0x2f335902, 0x111d2ea7);
const Class_ID hsEnvironMapMtlClassID(0x98777b3, 0x5eb270dd);

class hsMaxLayerBase : public BitmapTex {
public:
   enum hsMatBlendFlags {
        kBlendTest	= 0x1,							// dev
            // Rest of blends are mutually exclusive
            kBlendAlpha						= 0x2,		// dev
            kBlendMult						= 0x4,		// dev
            kBlendAdd						= 0x8,		// dev
            kBlendAddColorTimesAlpha		= 0x10,		// dev
            kBlendAntiAlias					= 0x20,
            kBlendDetail					= 0x40,
            kBlendDetailAdd                 = 0x80,
            kBlendMask						= kBlendAlpha
            | kBlendMult
            | kBlendAdd
            | kBlendAddColorTimesAlpha
            | kBlendAntiAlias
            | kBlendDetail
            | kBlendDetailAdd,
            kBlendInvertAlpha				= 0x1000,	// dev
            kBlendInvertColor				= 0x2000,	// dev
			kBlendAlphaMult					= 0x4000,
			kBlendAlphaAdd					= 0x8000,
			kBlendNoColor					= 0x10000,
			kBlendNoVtxAlpha				= 0x20000
    };
    
   enum hsMatZFlags {
       kZIncLayer			= 0x1, // dev
           kZOnlyZ				= 0x2,		// dev
           kZClearZ			= 0x4, // dev
           kZNoZRead			= 0x8, // dev
           kZNoZWrite			= 0x10,
           kZMask				= kZNoZWrite | kZClearZ | kZNoZRead,
           kZLODBias			= 0x20
   };

    enum hsMatShadeFlags {
        kShadeSoftShadow		= 0x1,			// view, dev
            kShadeNoProjectors		= 0x2,			// projector
            kShadeVertexShade		= 0x20,			// dev
            kShadeNoShade			= 0x40,			// view,dev
            kShadeBlack				= kShadeNoShade,
            kShadeSpecular			= 0x80,			// view, dev
            kShadeNoFog				= 0x100,		// dev
            kShadeWhite				= 0x200,
            kShadeSpecularAlpha		= 0x400,
            kShadeSpecularColor		= 0x800,
            kShadeSpecularHighlight	= 0x1000,
	        kShadeVertColShade		= 0x2000,
            kShadeInherit           = 0x4000
    };

    enum hsMatMiscFlags {
        kMiscWireFrame			= 0x1,			// dev (running out of bits)
            kMiscDrawMeshOutlines	= 0x2,			// dev, currently unimplemented
            kMiscTwoSided			= 0x4,			// view,dev
            kMiscDrawAsSplats		= 0x8,			// dev? bwt
            kMiscMipMap             = 0x10,
            kMiscUseBitmap          = 0x20,
            kMiscIntensityOnly      = 0x40,
            kMiscAutoStart          = 0x80,
            kMiscDetailBias         = 0x100,    // obsolete...
            kMiscDetailMax          = 0x200,    // obsolete...
            kMiscExplicitMipmap     = 0x400,
            kMiscAdjustPlane		= 0x800,
            kMiscAdjustCylinder		= 0x1000,
            kMiscAdjustSphere		= 0x2000,
            kMiscTroubledLoner		= 0x4000,
			kMiscBindSkip			= 0x8000,
			kMiscBindMask			= 0x10000,
			kMiscForceNonCompressed	= 0x20000,
			kMiscNoMaxSize			= 0x40000,
			kMiscHalfSize			= 0x80000,
			kMiscBindNext			= 0x100000,
			kMiscBindPrev			= 0x200000,
			kMiscReserved			= 0x400000
    };

	enum ProcType {
		kProcTypeDefault,
		kProcTypeWater
	};

	enum hsMatUsage {
		kUseNone		= 0x0,
		kUseBase		= 0x1,
		kUseDetail		= 0x2,
		kUseGrime		= 0x4,
		kUseTransition	= 0x8,
		kUseHighlight	= 0x10,
		kUseMask		= 0x20,
		kUseShadowLight	= 0x40,
		kUseHelper		= 0x80,
		
		kUseGuess		= 0x10000000
	};

public:
    // For hsMaxMtl...  Special case for higher layers.  Sigh.
    virtual void SetDirty(BOOL state) = 0;
    virtual void SetBlendFlag(int i, BOOL state) = 0;
    virtual void SetZFlag(int flag, BOOL state) = 0;
    virtual void SetShadeFlag(int flag, BOOL state) = 0;
    virtual void SetMiscFlag(int flag, BOOL state) = 0;
	virtual void SetProcType(ProcType type) = 0;
	virtual void SetUsage(hsMatUsage use) = 0;
	virtual void GuessUsage() = 0;

    // For interactive renderer
    virtual Color GetAmbient(int mtlNum=0, BOOL backFace=FALSE) = 0;
    virtual Color GetColor(int mtlNum=0, BOOL backFace=FALSE) = 0;

    virtual float GetShininess(int mtlNum=0, BOOL backFace=FALSE) = 0;
    virtual float GetShinStr(int mtlNum=0, BOOL backFace=FALSE) = 0;
    virtual float GetOpacity(int mtlNum=0, BOOL backFace=FALSE) = 0;

    // For exporter
    virtual Color GetAmbient(TimeValue t) const = 0;
    virtual Color GetColor(TimeValue t) const = 0;        
    
    virtual float GetShininess(TimeValue t) const = 0;
    virtual float GetShinStr(TimeValue t) const = 0;
    virtual float GetMapPercent(TimeValue t) const = 0;
    virtual float GetOpacity(TimeValue t) const = 0;
    virtual float GetMipMapBlur(TimeValue t) const = 0;
    virtual float GetLODBias(TimeValue t) const = 0;
    virtual float GetDetailDropoffStart(TimeValue t) const = 0;
    virtual float GetDetailDropoffStop(TimeValue t) const = 0;
    virtual float GetDetailMax(TimeValue t) const = 0;
    virtual float GetDetailMin(TimeValue t) const = 0;
    virtual int GetEnvironMapSize(TimeValue t) const = 0;

    virtual BOOL GetDirty() const = 0;
    virtual ULONG GetBlendFlags() const = 0;
    virtual ULONG GetZFlags() const = 0;
    virtual ULONG GetShadeFlags() const = 0;
    virtual ULONG GetMiscFlags() const = 0;
	virtual ProcType GetProcType() const = 0;
	virtual hsMatUsage GetUsage() const = 0;

    virtual int GetNumExplicitMipmaps() const = 0;
    virtual TCHAR *GetExplicitMipmapName(int i) const = 0;
    virtual BOOL ExplicitMipmapEnabled(int i) const = 0;
    virtual int GetExplicitMipmapLevel(int i) const = 0;

	// KLUDGE - Had to do this to compile under MAX4 beta
	virtual void fnReload() {};
	virtual void fnViewImage() {};
};

#endif
