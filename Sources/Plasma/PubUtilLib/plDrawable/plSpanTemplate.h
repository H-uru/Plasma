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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
    static uint16_t MakeFormat(bool hasColor, int numUVWs,
                            bool hasWgtIdx = false,
                            int numWgts = 0,
                            bool hasNorm = true,
                            bool hasPos = true,
                            bool hasColor2 = true)
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
        kPosMask        = 0x1,

        kNormMask       = 0x2,

        kColorMask      = 0x4,

        kWgtIdxMask     = 0x8,

        kUVWMask        = 0xf0,

        kWeightMask     = 0x300,

        kColor2Mask     = 0x400
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

    uint16_t  fNumVerts;
    uint16_t  fFormat;
    uint8_t   fStride;
    uint16_t  fNumTris;

    // Data stored interleaved. Any channels may be omitted, but
    // existing channels are in exactly the following order:
    //  Position
    //  Weights
    //  Indices
    //  Normal
    //  Color
    //  Color2
    //  UVWs
    uint8_t*          fData;

    uint16_t*         fIndices;

    friend class plClusterUtil;
public:
    plSpanTemplate()
        : fNumVerts(), fNumTris(), fFormat(), fStride(),
          fData(), fIndices()
    { }
    virtual ~plSpanTemplate() { DeAlloc(); }

    const uint8_t*    VertData() const { return fData; }

    const uint16_t*   IndexData() const { return fIndices; }

    uint32_t  NumVerts() const { return fNumVerts; }
    uint32_t  Stride() const { return uint32_t(fStride); }
    uint32_t  CalcStride();
    uint32_t  VertSize() const { return NumVerts() * Stride(); }

    uint32_t  NumTris() const { return fNumTris; }
    uint32_t  NumIndices() const { return NumTris() * 3; }
    uint32_t  IndexSize() const { return NumIndices() * sizeof(uint16_t); }
    
    uint8_t   PositionOffset() const { return uint8_t(0); }
    uint8_t   WgtIdxOffset() const { return uint8_t(PositionOffset() + NumPos() * sizeof(hsPoint3)); }
    uint8_t   WeightOffset() const { return uint8_t(WgtIdxOffset() + NumWgtIdx() * sizeof(uint32_t)); }
    uint8_t   NormalOffset() const { return uint8_t(WeightOffset() + NumWeights() * sizeof(float)); }
    uint8_t   ColorOffset() const { return uint8_t(NormalOffset() + NumNorm() * sizeof(hsVector3)); }
    uint8_t   Color2Offset() const { return uint8_t(ColorOffset() + NumColor() * sizeof(uint32_t)); }
    uint8_t   UVWOffset() const { return uint8_t(Color2Offset() + NumColor2() * sizeof(uint32_t)); }

    uint32_t  NumUVWs() const { return (fFormat & kUVWMask) >> 4; }
    uint32_t  NumWeights() const { return (fFormat & kWeightMask) >> 8; }

    uint32_t  NumPos() const { return (fFormat & kPosMask) >> 0; }
    uint32_t  NumNorm() const { return (fFormat & kNormMask) >> 1; }
    uint32_t  NumColor() const { return (fFormat & kColorMask) >> 2; }
    uint32_t  NumColor2() const { return (fFormat & kColor2Mask) >> 10; }
    uint32_t  NumWgtIdx() const { return (fFormat & kWgtIdxMask) >> 3; }

    hsPoint3*           Position(int i) const { return (hsPoint3*)GetData(kPosition, i); }
    hsVector3*          Normal(int i) const { return (hsVector3*)GetData(kNormal, i); }
    uint32_t*             Color(int i) const { return (uint32_t*)GetData(kColor, i); }
    uint32_t*             Color2(int i) const { return (uint32_t*)GetData(kColor2, i); }
    uint32_t*             WgtIdx(int i) const { return (uint32_t*)GetData(kWgtIdx, i); }
    hsPoint3*           UVWs(int iv, int iuv) const { return (hsPoint3*)GetData(kUVW, iv, iuv); }
    float*           Weight(int iv, int iw) const { return (float*)GetData(kWeight, iv, iw); }

    uint8_t*              GetData(Channel chan, int i, int j=0) const
    {
        ValidateInput(chan, i, j);
        uint8_t* base = fData + i * fStride;
        switch(chan)
        {
        case kPosition:
            return base;
        case kWeight:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + j * sizeof(float);
        case kWgtIdx:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + NumWeights() * sizeof(float);
        case kNormal:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + NumWeights() * sizeof(float)
                + NumWgtIdx() * sizeof(uint32_t);
        case kColor:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + NumWeights() * sizeof(float)
                + NumWgtIdx() * sizeof(uint32_t)
                + NumNorm() * sizeof(hsVector3);
        case kColor2:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + NumWeights() * sizeof(float)
                + NumWgtIdx() * sizeof(uint32_t)
                + NumNorm() * sizeof(hsVector3)
                + NumColor() * sizeof(uint32_t);
        case kUVW:
            return base 
                + NumPos() * sizeof(hsPoint3)
                + NumWeights() * sizeof(float)
                + NumWgtIdx() * sizeof(uint32_t)
                + NumNorm() * sizeof(hsVector3)
                + NumColor() * sizeof(uint32_t)
                + NumColor2() * sizeof(uint32_t)
                + j * sizeof(hsPoint3);
        }
        hsAssert(false, "Unrecognized vertex channel");
        return nullptr;
    }

    bool ValidateInput(Channel chan, int i, int j) const
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
    
    void Alloc(uint16_t format, uint32_t numVerts, uint32_t numTris);
    void DeAlloc();

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

// An export only version
class plSpanTemplateB : public plSpanTemplate
{
    INode*          fSrc;
    hsBounds3Ext    fLocalBounds;

    hsColorRGBA*    fMultColors;
    hsColorRGBA*    fAddColors;

public:
    plSpanTemplateB(INode* src) : plSpanTemplate(), fSrc(src), fMaterial() { }
    virtual ~plSpanTemplateB() { DeAllocColors(); }

    void ComputeBounds();

    const hsBounds3Ext& GetLocalBounds() const { return fLocalBounds; }

    INode*          GetSrcNode() const { return fSrc; }

    hsGMaterial*    fMaterial;

    plRenderLevel   fRenderLevel;

    hsColorRGBA*    MultColor(int i) const { return &fMultColors[i]; }
    hsColorRGBA*    AddColor(int i) const { return &fAddColors[i]; }

    void AllocColors();
    void DeAllocColors();
};

#endif // plSpanTemplate_inc
