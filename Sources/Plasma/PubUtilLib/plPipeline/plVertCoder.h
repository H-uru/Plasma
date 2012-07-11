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

#ifndef plVertCoder_inc
#define plVertCoder_inc

class hsStream;

class plVertCoder
{
public:
    enum {
        kPosition,
        kWeight,
        kUVW,
        kNumFloatFields = kUVW + 8
    };

protected:

    class FloatCode
    {
    public:
        float    fOffset;
        bool        fAllSame;
        uint16_t      fCount;
    };

    FloatCode       fFloats[kNumFloatFields][3];

    class byteCode
    {
    public:
        uint16_t      fCount;
        uint8_t       fVal;
        uint8_t       fSame;
    };

    byteCode        fColors[4];

    static uint32_t   fCodedVerts;
    static uint32_t   fCodedBytes;
    static uint32_t   fRawBytes;
    static uint32_t   fSkippedBytes;

    inline void ICountFloats(const uint8_t* src, uint16_t maxCnt, const float quant, const uint32_t stride, float& lo, bool& allSame, uint16_t& count);
    inline void IEncodeFloat(hsStream* s, const uint32_t vertsLeft, const int field, const int chan, const uint8_t*& src, const uint32_t stride);
    inline void IDecodeFloat(hsStream* s, const int field, const int chan, uint8_t*& dst, const uint32_t stride);

    inline void IEncodeNormal(hsStream* s, const uint8_t*& src, const uint32_t stride);
    inline void IDecodeNormal(hsStream* s, uint8_t*& dst, const uint32_t stride);

    inline void ICountBytes(const uint32_t vertsLeft, const uint8_t* src, const uint32_t stride, uint16_t& len, uint8_t& same);
    inline void IEncodeByte(hsStream* s, const int chan, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride);
    inline void IDecodeByte(hsStream* s, const int chan, uint8_t*& dst, const uint32_t stride);
    inline void IEncodeColor(hsStream* s, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride);
    inline void IDecodeColor(hsStream* s, uint8_t*& dst, const uint32_t stride);

    inline void IEncode(hsStream* s, const uint32_t vertsLeft, const uint8_t*& src, const uint32_t stride, const uint8_t format);
    inline void IDecode(hsStream* s, uint8_t*& dst, const uint32_t stride, const uint8_t format);

public:
    plVertCoder();
    ~plVertCoder();

    void Clear();

    void Read(hsStream* s, uint8_t* dst, const uint8_t format, const uint32_t stride, const uint16_t numVerts);
    void Write(hsStream* s, const uint8_t* src, const uint8_t format, const uint32_t stride, const uint16_t numVerts);


    static void ClearAverage() { fCodedVerts = 0; fCodedBytes = 0; fRawBytes = 0; fSkippedBytes = 0; }
    static uint32_t CodedBytes() { return fCodedBytes; }
    static uint32_t RawBytes() { return fRawBytes; }
    static uint32_t CodedVerts() { return fCodedVerts; }
    static float AverageCodedVertSize() { return fCodedVerts ? float(fCodedBytes) / float(fCodedVerts) : 0; }
    static float AverageRawVertSize() { return fCodedVerts ? float(fRawBytes) / float(fCodedVerts) : 0; }

    static uint32_t SkippedBytes() { return fSkippedBytes; }
    static void AddSkippedBytes(uint32_t f) { fSkippedBytes += f; }
};

#endif // plVertCoder_inc
