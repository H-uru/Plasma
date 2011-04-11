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
		hsScalar	fOffset;
		hsBool		fAllSame;
		UInt16		fCount;
	};

	FloatCode		fFloats[kNumFloatFields][3];

	class ByteCode
	{
	public:
		UInt16		fCount;
		UInt8		fVal;
		UInt8		fSame;
	};

	ByteCode		fColors[4];

	static UInt32	fCodedVerts;
	static UInt32	fCodedBytes;
	static UInt32	fRawBytes;
	static UInt32	fSkippedBytes;

	inline void ICountFloats(const UInt8* src, UInt16 maxCnt, const hsScalar quant, const UInt32 stride, hsScalar& lo, hsBool& allSame, UInt16& count);
	inline void IEncodeFloat(hsStream* s, const UInt32 vertsLeft, const int field, const int chan, const UInt8*& src, const UInt32 stride);
	inline void IDecodeFloat(hsStream* s, const int field, const int chan, UInt8*& dst, const UInt32 stride);

	inline void IEncodeNormal(hsStream* s, const UInt8*& src, const UInt32 stride);
	inline void IDecodeNormal(hsStream* s, UInt8*& dst, const UInt32 stride);

	inline void ICountBytes(const UInt32 vertsLeft, const UInt8* src, const UInt32 stride, UInt16& len, UInt8& same);
	inline void IEncodeByte(hsStream* s, const int chan, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride);
	inline void IDecodeByte(hsStream* s, const int chan, UInt8*& dst, const UInt32 stride);
	inline void IEncodeColor(hsStream* s, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride);
	inline void IDecodeColor(hsStream* s, UInt8*& dst, const UInt32 stride);

	inline void IEncode(hsStream* s, const UInt32 vertsLeft, const UInt8*& src, const UInt32 stride, const UInt8 format);
	inline void IDecode(hsStream* s, UInt8*& dst, const UInt32 stride, const UInt8 format);

public:
	plVertCoder();
	~plVertCoder();

	void Clear();

	void Read(hsStream* s, UInt8* dst, const UInt8 format, const UInt32 stride, const UInt16 numVerts);
	void Write(hsStream* s, const UInt8* src, const UInt8 format, const UInt32 stride, const UInt16 numVerts);


	static void ClearAverage() { fCodedVerts = 0; fCodedBytes = 0; fRawBytes = 0; fSkippedBytes = 0; }
	static UInt32 CodedBytes() { return fCodedBytes; }
	static UInt32 RawBytes() { return fRawBytes; }
	static UInt32 CodedVerts() { return fCodedVerts; }
	static float AverageCodedVertSize() { return fCodedVerts ? float(fCodedBytes) / float(fCodedVerts) : 0; }
	static float AverageRawVertSize() { return fCodedVerts ? float(fRawBytes) / float(fCodedVerts) : 0; }

	static UInt32 SkippedBytes() { return fSkippedBytes; }
	static void AddSkippedBytes(UInt32 f) { fSkippedBytes += f; }
};

#endif // plVertCoder_inc
