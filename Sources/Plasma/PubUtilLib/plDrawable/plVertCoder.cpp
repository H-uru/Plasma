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

#include "HeadSpin.h"
#include "plVertCoder.h"

#include "hsStream.h"
#include <cmath>
#include "plGBufferGroup.h"

const float kPosQuantum = 1.f / float(1 << 10);
const float kWeightQuantum = 1.f / float(1 << 15);
const float kUVWQuantum = 1.f / float(1 << 16);

uint32_t  plVertCoder::fCodedVerts = 0;
uint32_t  plVertCoder::fCodedBytes = 0;
uint32_t  plVertCoder::fRawBytes = 0;
uint32_t  plVertCoder::fSkippedBytes = 0;

static const float kQuanta[plVertCoder::kNumFloatFields] =
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


inline void plVertCoder::ICountFloats(const uint8_t* src, uint16_t maxCnt, const float quant, const uint32_t stride, 
                                      float& lo, bool &allSame, uint16_t& count)
{
    lo = *(float*)src;
    lo = floor(lo / quant + 0.5f) * quant;
    allSame = false;
    float hi = lo;
    
    count = 1;

    const float maxRange = float(uint16_t(0xffff)) * quant;

    src += stride;
    maxCnt--;

    while( maxCnt-- )
    {
        float val = *(float*)src;
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

static inline void IWriteFloat(hsStream* s, const uint8_t*& src, const float offset, const float quantum)
{
    float fval = *(float*)src;
    fval -= offset;
    fval /= quantum;
//  hsAssert(fval < float(uint16_t(0xffff)), "Bad offset?");

    const uint16_t ival = uint16_t(floor(fval + 0.5f));
    s->WriteLE16(ival);

    src += 4;
}

static inline void IReadFloat(hsStream* s, uint8_t*& dst, const float offset, const float quantum)
{
    const uint16_t ival = s->ReadLE16();
    float fval = float(ival) * quantum;
    fval += offset;

    float* val = (float*)dst;
    *val = fval;

    dst += 4;
}

inline void plVertCoder::IEncodeFloat(hsStream* s, const uint32_t vertsLeft, const int field, const int chan, const uint8_t*& src, const uint32_t stride)
{
    if( !fFloats[field][chan].fCount )
    {
        ICountFloats(src, (uint16_t)vertsLeft, kQuanta[field], stride, fFloats[field][chan].fOffset, fFloats[field][chan].fAllSame, fFloats[field][chan].fCount);

        s->WriteLEFloat(fFloats[field][chan].fOffset);
        s->WriteBool(fFloats[field][chan].fAllSame);
        s->WriteLE16(fFloats[field][chan].fCount);

    }
    if (!fFloats[field][chan].fAllSame)
        IWriteFloat(s, src, fFloats[field][chan].fOffset, kQuanta[field]);
    else
        src += 4;

    fFloats[field][chan].fCount--;
}

inline void plVertCoder::IDecodeFloat(hsStream* s, const int field, const int chan, uint8_t*& dst, const uint32_t stride)
{
    if( !fFloats[field][chan].fCount )
    {
        fFloats[field][chan].fOffset = s->ReadLEFloat();
        fFloats[field][chan].fAllSame = s->ReadBool();
        fFloats[field][chan].fCount = s->ReadLE16();
    }

    if (!fFloats[field][chan].fAllSame)
        IReadFloat(s, dst, fFloats[field][chan].fOffset, kQuanta[field]);
    else
    {
        *((float*)dst) = fFloats[field][chan].fOffset;
        dst += 4;
    }

    fFloats[field][chan].fCount--;
}

static inline int INumWeights(const uint8_t format)
{
    return (format & plGBufferGroup::kSkinWeightMask) >> 4;
}

static const float kNormalScale(int16_t(0x7fff));
static const float kInvNormalScale(1.f / kNormalScale);

inline void plVertCoder::IEncodeNormal(hsStream* s, const uint8_t*& src, const uint32_t stride)
{

    float x = *(float*)src;
    s->WriteByte((uint8_t)((x / 2.f + .5f) * 255.9f));
    src += 4;

    x = *(float*)src;
    s->WriteByte((uint8_t)((x / 2.f + .5f) * 255.9f));
    src += 4;

    x = *(float*)src;
    s->WriteByte((uint8_t)((x / 2.f + .5f) * 255.9f));
    src += 4;
}

inline void plVertCoder::IDecodeNormal(hsStream* s, uint8_t*& dst, const uint32_t stride)
{

    uint8_t ix = s->ReadByte();
    float* x = (float*)dst;
    *x = (ix / 255.9f - .5f) * 2.f;
    dst += 4;

    ix = s->ReadByte();
    x = (float*)dst;
    *x = (ix / 255.9f - .5f) * 2.f;
    dst += 4;

    ix = s->ReadByte();
    x = (float*)dst;
    *x = (ix / 255.9f - .5f) * 2.f;
    dst += 4;
}

inline void plVertCoder::ICountBytes(const uint32_t vertsLeft, const uint8_t* src, const uint32_t stride, uint16_t& len, uint8_t& same)
{
    // We want to run length encode this. So we're looking here for either
    // the number of consecutive bytes of the same value,
    // or the number of consective bytes of different values.
    // The latter is so we don't wind up getting larger when there aren't any 
    // runs of the same value (count=1 and val=c1, count=1 and val=c2, etc.).
    // The break-even point is a run of 3, so we'll look for a minimum run of 4.

    if( vertsLeft < 4 )
    {
        len = (uint16_t)vertsLeft;
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

    len = (uint16_t)vertsLeft;
    return;
}

static const uint16_t kSameMask(0x8000);

inline void plVertCoder::IEncodeByte(hsStream* s, const int chan, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride)
{
    if( !fColors[chan].fCount )
    {
        ICountBytes(vertsLeft, src, stride, fColors[chan].fCount, fColors[chan].fSame);

        uint16_t cnt = fColors[chan].fCount;
        if( fColors[chan].fSame )
            cnt |= kSameMask;
        s->WriteLE16(cnt);

        if( fColors[chan].fSame )
            s->WriteByte(*src);
    }

    if( !fColors[chan].fSame )
        s->WriteByte(*src);
    
    src++;
    fColors[chan].fCount--;
}

inline void plVertCoder::IDecodeByte(hsStream* s, const int chan, uint8_t*& dst, const uint32_t stride)
{
    if( !fColors[chan].fCount )
    {
        uint16_t cnt = s->ReadLE16();
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

inline void plVertCoder::IEncodeColor(hsStream* s, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride)
{
    IEncodeByte(s, 0, vertsLeft, src, stride);
    IEncodeByte(s, 1, vertsLeft, src, stride);
    IEncodeByte(s, 2, vertsLeft, src, stride);
    IEncodeByte(s, 3, vertsLeft, src, stride);
}

inline void plVertCoder::IDecodeColor(hsStream* s, uint8_t*& dst, const uint32_t stride)
{
    IDecodeByte(s, 0, dst, stride);
    IDecodeByte(s, 1, dst, stride);
    IDecodeByte(s, 2, dst, stride);
    IDecodeByte(s, 3, dst, stride);
}

inline void plVertCoder::IEncode(hsStream* s, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride, const uint8_t format)
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
            const uint32_t idx = *(uint32_t*)src;
            s->WriteLE32(idx);
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

inline void plVertCoder::IDecode(hsStream* s, uint8_t*& dst, const uint32_t stride, const uint8_t format)
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
            uint32_t* idx = (uint32_t*)dst;
            *idx = s->ReadLE32();
            dst += 4;
        }
    }

    IDecodeNormal(s, dst, stride);

    IDecodeColor(s, dst, stride);

    // COLOR2
    uint32_t* trash = (uint32_t*)dst;
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

void plVertCoder::Read(hsStream* s, uint8_t* dst, const uint8_t format, const uint32_t stride, const uint16_t numVerts)
{
    Clear();

    int i = numVerts;
    for( i = 0; i < numVerts; i++ )
        IDecode(s, dst, stride, format);
}


void plVertCoder::Write(hsStream* s, const uint8_t* src, const uint8_t format, const uint32_t stride, const uint16_t numVerts)
{
    Clear();

    uint32_t streamStart = s->GetPosition();

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

