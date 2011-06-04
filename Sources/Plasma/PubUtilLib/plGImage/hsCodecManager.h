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
///////////////////////////////////////////////////////////////////////////////
//																			 //
//	hsCodecManager Class Header												 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Updated for new bitmap classes.							 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsCodecManager_inc
#define hsCodecManager_inc

#include "HeadSpin.h"
#include "hsTemplates.h"

class hsCodec;
class plMipmap;

class hsCodecManager
{
private:
	hsCodecManager();
public:
	~hsCodecManager()		{ }
	static hsCodecManager& Instance();

	plMipmap	*CreateCompressedMipmap( UInt32 compressionFormat, plMipmap *uncompressed );
	plMipmap	*CreateUncompressedMipmap( plMipmap *compressed, UInt8 bitDepth = 0 );
	hsBool		ColorizeCompMipmap( plMipmap *bMap, const UInt8 *colorMask );

	hsBool		Register(hsCodec *codec, UInt32 compressionFormat, hsScalar priority);

	/// Decompression flags
	enum {
		kBitDepthMask = 0x0003,
		kCompOrderMask = 0x0004
	};
	enum {	/// Bit depths
		kDontCareDepth = 0x0000,
		k16BitDepth = 0x0001,
		k32BitDepth = 0x0002
	};
	enum {	/// Byte orders
		kNormalCompOrder = 0x0000,		// DirectX, Glide
		kWeirdCompOrder = 0x0004		// OpenGL
	};

private:
	struct hsCodecEntry
	{
		hsCodecEntry() : fPriority(0), fCodec(nil) { }
		hsCodecEntry(hsScalar p, hsCodec *c) : fPriority(p), fCodec(c) { }

		hsScalar				fPriority;
		hsCodec					*fCodec;
	};

	struct hsCodecList
	{
		hsCodecList() : fCompressionFormat(0) { }
		hsCodecList(UInt32 f) : fCompressionFormat(f) { }

		UInt32					fCompressionFormat;
		hsTArray<hsCodecEntry>	fCodecList;
	};

	hsTArray<hsCodecList>		fCodecTable;
};

#endif // hsCodecManager_inc
