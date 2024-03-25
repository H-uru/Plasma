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

#ifndef plSpanInstance_inc
#define plSpanInstance_inc

#include "hsGeometry3.h"
#include "hsMatrix44.h"

// plClusterGroup
//      array of templates
//      array of materials (indexed by templates)
//      array of clusters
//      array of LOD blend 
//      array of vis sets

// plCluster
//      array of span instances that are combinable
//      LOD blend start and end index
//      vis set index
//      Combinable means:
//          same material
//          same format
//          same LOD blend
//          same vis set
//          same lights
//          same encoding?

// plSpanInstance
//      template idx
//      3x4 transform
//      encoding flags 
//          (what components does it include, delPos? color?)
//          (what's the encoding? 32bit RGBA? 16Bit/Chan? 10Bit/Chan? 8Bit/Chan?)
//          (need an encoding per channel)

// plSpanTemplate
//      numVerts
//      Format (assume pos & norm?)
//          kUVWMask
//          kColMask
//          kWeightMask
//      fPos
//      fNorm
//      fCol
//      fUVWs
//      fWeights


class plSpanEncoding
{
public:
    enum
    {
        kPosNone            = 0x0,
        kPos888             = 0x1,
        kPos161616          = 0x2,
        kPos101010          = 0x4,
        kPos008             = 0x8,
        kPosMask            = kPos888 | kPos161616 | kPos101010 | kPos008,

        kColNone            = 0x0,
        kColA8              = 0x10,
        kColI8              = 0x20,
        kColAI88            = 0x40,
        kColRGB888          = 0x80,
        kColARGB8888        = 0x100,
        kColMask            = kColNone
                                | kColA8
                                | kColI8
                                | kColAI88
                                | kColRGB888
                                | kColARGB8888,
    };

    uint32_t          fCode;
    float        fPosScale;

    plSpanEncoding() : fCode(kPosNone|kColNone), fPosScale(0) {}
    plSpanEncoding(uint32_t c, float s) : fCode(c), fPosScale(s) {}

    uint32_t      Code() const { return fCode; }
    float    Scale() const { return fPosScale; }

    void Read(hsStream* s);
    void Write(hsStream* s) const;
};

class plSpanInstance
{
protected:
    uint8_t*          fPosDelta;
    uint8_t*          fCol;

    float        fL2W[3][4];

    friend class plSpanInstanceIter;
public:
    plSpanInstance();
    ~plSpanInstance();

    void Read(hsStream* s, const plSpanEncoding& encoding, uint32_t numVerts);
    void Write(hsStream* s, const plSpanEncoding& encoding, uint32_t numVerts) const;

    void Encode(const plSpanEncoding& encoding, uint32_t numVerts, const hsVector3* delPos, const uint32_t* color);

    void Alloc(const plSpanEncoding& encoding, uint32_t numVerts);
    void DeAlloc();

    hsMatrix44 LocalToWorld() const;
    hsMatrix44 WorldToLocal() const;

    void SetLocalToWorld(const hsMatrix44& l2w);

    bool HasPosDelta() const { return fPosDelta != nullptr; }
    bool HasColor() const { return fCol != nullptr; }

    static uint16_t PosStrideFromEncoding(const plSpanEncoding& encoding)
    {
        switch(encoding.fCode & plSpanEncoding::kPosMask)
        {
        case plSpanEncoding::kPos888:
            return 3;
        case plSpanEncoding::kPos161616:
            return 6;
        case plSpanEncoding::kPos101010:
            return 4;
        case plSpanEncoding::kPos008:
            return 1;
        }
        return 0;
    }
    static uint16_t ColStrideFromEncoding(const plSpanEncoding& encoding)
    {
        switch(encoding.fCode & plSpanEncoding::kColMask)
        {
        case plSpanEncoding::kColNone:
            return 0;
        case plSpanEncoding::kColA8:
            return 1;
        case plSpanEncoding::kColI8:
            return 1;
        case plSpanEncoding::kColAI88:
            return 2;
        case plSpanEncoding::kColRGB888:
            return 3;
        case plSpanEncoding::kColARGB8888:
            return 4;
        }
        return 0;
    }
};

class plSpanInstanceIter
{
protected:
    plSpanInstance*     fInst;
    plSpanEncoding      fEncoding;
    uint32_t              fNumVerts;
    int32_t               fNumVertsLeft;
    uint16_t              fPosStride;
    uint16_t              fColStride;

    union {
        int8_t*       fPos888;
        int16_t*      fPos161616;
        uint32_t*     fPos101010;
    };
    union {
        uint8_t*      fA8;
        uint8_t*      fI8;
        uint16_t*     fAI88;
        uint8_t*      fRGB888;
        uint32_t*     fARGB8888;
    };

public:
    plSpanInstanceIter();
    plSpanInstanceIter(plSpanInstance* inst, const plSpanEncoding& encoding, uint32_t numVerts) { Init(inst, encoding, numVerts); }

    void Init(plSpanInstance* inst, const plSpanEncoding& encoding, uint32_t numVerts)
    {
        fInst = inst;
        fEncoding = encoding;
        fNumVerts = numVerts;
        fPosStride = inst->PosStrideFromEncoding(fEncoding);
        fColStride = inst->ColStrideFromEncoding(fEncoding);
        fNumVertsLeft = 0;
    }

    void        Begin()
    {
        fPos888 = (int8_t*)fInst->fPosDelta;
        fA8 = fInst->fCol;

        fNumVertsLeft = fNumVerts;
    }
    void        Advance()
    {
        fPos888 += fPosStride;
        fA8 += fColStride;

        fNumVertsLeft--;
    }
    bool        More() const { return fNumVertsLeft > 0; }

    hsVector3   DelPos() const
    {
        switch(fEncoding.fCode & plSpanEncoding::kPosMask)
        {
        case plSpanEncoding::kPos888:
            return hsVector3(fPos888[0] * fEncoding.fPosScale, 
                                fPos888[1] * fEncoding.fPosScale, 
                                fPos888[2] * fEncoding.fPosScale);
        case plSpanEncoding::kPos161616:
            return hsVector3(fPos161616[0] * fEncoding.fPosScale, 
                                fPos161616[1] * fEncoding.fPosScale, 
                                fPos161616[2] * fEncoding.fPosScale);
        case plSpanEncoding::kPos101010:
            return hsVector3(int(*fPos101010 & 0x3f) * fEncoding.fPosScale,
                int((*fPos101010 >> 10) & 0x3f) * fEncoding.fPosScale,
                int((*fPos101010 >> 20) & 0x3f) * fEncoding.fPosScale);
        case plSpanEncoding::kPos008:
            return hsVector3(0,0, *fPos888 * fEncoding.fPosScale);
        }
        return {};
    };
    hsPoint3 Position(const hsPoint3& p) const
    {
        return p + DelPos();
    };
    uint32_t      Color(uint32_t c) const 
    { 
        switch(fEncoding.fCode & plSpanEncoding::kColMask)
        {
        case plSpanEncoding::kColA8:
            return (c & 0x00ffffff) | *fA8;
        case plSpanEncoding::kColI8:
            return (c & 0xff000000) 
                | (*fI8 << 16)
                | (*fI8 << 8)
                | (*fI8 << 0);
        case plSpanEncoding::kColAI88:
            {
                const uint32_t col = *fAI88 & 0xff;
                return ((*fAI88 & 0xff00) << 24)
                    | (col << 16)
                    | (col << 8)
                    | (col << 0);
            }
        case plSpanEncoding::kColRGB888:
            return (c & 0xff000000)
                | (fRGB888[0] << 16)
                | (fRGB888[1] << 8)
                | (fRGB888[2] << 0);
        case plSpanEncoding::kColARGB8888:
            return *fARGB8888;
        }
        return c; 
    };
};

#endif // plSpanInstance_inc
