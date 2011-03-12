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
#ifndef __HSDXTSOFTWARECODEC_H
#define __HSDXTSOFTWARECODEC_H

#include "HeadSpin.h"
#include "hsCodec.h"

class plMipmap;

class hsDXTSoftwareCodec : public hsCodec
{
private:
	hsDXTSoftwareCodec();
public:
	~hsDXTSoftwareCodec();
	static hsDXTSoftwareCodec& Instance();

	static void Init()	{ fRegistered = Register(); }

	plMipmap *CreateCompressedMipmap(plMipmap *uncompressed);

	// Uncompresses the given source into a new destination mipmap
	plMipmap *CreateUncompressedMipmap( plMipmap *compressed, UInt8 flags = 0 );

	// Colorize a compressed mipmap
	hsBool	ColorizeCompMipmap( plMipmap *bMap, const UInt8 *colorMask );

private:
	enum {
		kFourColorEncoding,
		kThreeColorEncoding
	};

	void	CompressMipmapLevel( plMipmap *uncompressed, plMipmap *compressed );

	UInt16 BlendColors16(UInt16 weight1, UInt16 color1, UInt16 weight2, UInt16 color2);
	hsRGBAColor32 BlendColors32(UInt32 weight1, hsRGBAColor32 color1, UInt32 weight2, hsRGBAColor32 color2);
	Int32 ColorDistanceARGBSquared(hsRGBAColor32 color1, hsRGBAColor32 color2);
	UInt16 Color32To16(hsRGBAColor32 color);

	// Calculates the DXT format based on a mipmap
	UInt8	ICalcCompressedFormat( plMipmap *bMap );

	// Copy over a block from a mipmap level to a destination mipmap, converting if necessary
	void	IXlateColorBlock( plMipmap *destBMap, UInt32 *srcBlock, UInt8 flags = 0 );

	// Creates an uncompressed mipmap with the settings to match
	plMipmap	*ICreateUncompressedMipmap( plMipmap *compressed, UInt8 flags );

	// Dispatcher for all the decompression functions
	void inline	UncompressMipmap( plMipmap *uncompressed, plMipmap *compressed, 
								  UInt8 flags = 0 );

	// Decompresses a DXT5 compressed mipmap into a RGB4444 mipmap
	void	IUncompressMipmapDXT5To16( plMipmap *destBMap, plMipmap *srcBMap );
	// Decompresses a DXT5 compressed mipmap into a RGB4444 reversed mipmap
	void	IUncompressMipmapDXT5To16Weird( plMipmap *destBMap, plMipmap *srcBMap );
	// Decompresses a DXT5 compressed mipmap into a RGB8888 mipmap
	void	IUncompressMipmapDXT5To32( plMipmap *destBMap, plMipmap *srcBMap );

	// Decompresses a DXT1 compressed mipmap into a RGB1555 mipmap
	void	IUncompressMipmapDXT1To16( plMipmap *destBMap, plMipmap *srcBMap );
	// Decompresses a DXT1 compressed mipmap into a RGB5551 mipmap
	void	IUncompressMipmapDXT1To16Weird( plMipmap *destBMap, plMipmap *srcBMap );
	// Decompresses a DXT1 compressed mipmap into a RGB8888 mipmap
	void	IUncompressMipmapDXT1To32( plMipmap *destBMap, plMipmap *srcBMap );

	// Decompresses a DXT1 compressed mipmap into an intensity map
	void	IUncompressMipmapDXT1ToInten( plMipmap *destBMap, plMipmap *srcBMap );
	// Decompresses a DXT5 compressed mipmap into an alpha-intensity map
	void	IUncompressMipmapDXT5ToAInten( plMipmap *destBMap, plMipmap *srcBMap );

	// Mixes two RGB8888 colors equally
	UInt32 inline IMixEqualRGB32( UInt32 color1, UInt32 color2 );
	// Mixes two-thirds of the first RGB8888 color and one-third of the second
	UInt32 inline IMixTwoThirdsRGB32( UInt32 twoThirds, UInt32 oneThird );

	// Mixes two RGB1555 colors equally
	UInt16 inline IMixEqualRGB1555( UInt16 color1, UInt16 color2 );
	// Mixes two-thirds of the first RGB1555 color and one-third of the second
	UInt16 inline IMixTwoThirdsRGB1555( UInt16 twoThirds, UInt16 oneThird );

	// Mixes two RGB5551 colors equally
	UInt16 inline IMixEqualRGB5551( UInt16 color1, UInt16 color2 );
	// Mixes two-thirds of the first RGB5551 color and one-third of the second
	UInt16 inline IMixTwoThirdsRGB5551( UInt16 twoThirds, UInt16 oneThird );

	// Mixes two RGB4444 colors equally
	UInt16 inline IMixEqualRGB4444( UInt16 color1, UInt16 color2 );
	// Mixes two-thirds of the first RGB4444 color and one-third of the second
	UInt16 inline IMixTwoThirdsRGB4444( UInt16 twoThirds, UInt16 oneThird );
	
	// Mixes two intensity values equally
	UInt8  inline IMixEqualInten( UInt8 color1, UInt8 color2 );
	// Mixes two-thirds of the first intensity and one-third of the second
	UInt8  inline IMixTwoThirdsInten( UInt8 twoThirds, UInt8 oneThird );

	// Converts a color from RGB565 to RGB8888 format, with alpha=0
	UInt32 inline IRGB16To32Bit( UInt16 color );
	// Converts a color from RGB565 to RGB4444 format, with alpha=0
	UInt16 inline IRGB565To4444( UInt16 color );
	// Converts a color from RGB565 to RGB1555 format, with alpha=0
	UInt16 inline IRGB565To1555( UInt16 color );
	// Converts a color from RGB565 to RGB5551 format, with alpha=0
	UInt16 inline IRGB565To5551( UInt16 color );
	// Converts a color from RGB565 to RGB4444 reversed format, with alpha=0
	UInt16 inline IRGB565To4444Rev( UInt16 color );

	// Swaps the bytes in a doubleword
	UInt32 inline ISwapDwordOrder( UInt32 color );
	// Swaps the bytes in a word
	UInt16 inline ISwapWordOrder( UInt16 color );

	static hsBool Register();
	static hsBool fRegistered;
};

#endif // __HSDXTSOFTWARECODEC_H
