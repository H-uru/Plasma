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
#ifndef __HSDXTSOFTWARECODEC_H
#define __HSDXTSOFTWARECODEC_H

#include "HeadSpin.h"
#include "hsCodec.h"

class plMipmap;
typedef struct hsColor32 hsRGBAColor32;

class hsDXTSoftwareCodec : public hsCodec
{
private:
    hsDXTSoftwareCodec();
public:
    ~hsDXTSoftwareCodec();
    static hsDXTSoftwareCodec& Instance();

    static void Init()  { fRegistered = Register(); }

    plMipmap *CreateCompressedMipmap(plMipmap *uncompressed) override;

    // Uncompresses the given source into a new destination mipmap
    plMipmap *CreateUncompressedMipmap(plMipmap *compressed, uint8_t flags = 0) override;

    // Colorize a compressed mipmap
    bool    ColorizeCompMipmap(plMipmap *bMap, const uint8_t *colorMask) override;

private:
    enum {
        kFourColorEncoding,
        kThreeColorEncoding
    };

    void    CompressMipmapLevel( plMipmap *uncompressed, plMipmap *compressed );

    uint16_t BlendColors16(uint16_t weight1, uint16_t color1, uint16_t weight2, uint16_t color2);
    hsRGBAColor32 BlendColors32(uint32_t weight1, hsRGBAColor32 color1, uint32_t weight2, hsRGBAColor32 color2);
    int32_t ColorDistanceARGBSquared(hsRGBAColor32 color1, hsRGBAColor32 color2);
    uint16_t Color32To16(hsRGBAColor32 color);

    // Calculates the DXT format based on a mipmap
    uint8_t   ICalcCompressedFormat( plMipmap *bMap );

    // Copy over a block from a mipmap level to a destination mipmap, converting if necessary
    void    IXlateColorBlock( plMipmap *destBMap, uint32_t *srcBlock, uint8_t flags = 0 );

    // Creates an uncompressed mipmap with the settings to match
    plMipmap    *ICreateUncompressedMipmap( plMipmap *compressed, uint8_t flags );

    // Dispatcher for all the decompression functions
    void inline UncompressMipmap( plMipmap *uncompressed, plMipmap *compressed, 
                                  uint8_t flags = 0 );

    // Decompresses a DXT5 compressed mipmap into a RGB4444 mipmap
    void    IUncompressMipmapDXT5To16( plMipmap *destBMap, plMipmap *srcBMap );
    // Decompresses a DXT5 compressed mipmap into a RGB4444 reversed mipmap
    void    IUncompressMipmapDXT5To16Weird( plMipmap *destBMap, plMipmap *srcBMap );
    // Decompresses a DXT5 compressed mipmap into a RGB8888 mipmap
    void    IUncompressMipmapDXT5To32( plMipmap *destBMap, plMipmap *srcBMap );

    // Decompresses a DXT1 compressed mipmap into a RGB1555 mipmap
    void    IUncompressMipmapDXT1To16( plMipmap *destBMap, plMipmap *srcBMap );
    // Decompresses a DXT1 compressed mipmap into a RGB5551 mipmap
    void    IUncompressMipmapDXT1To16Weird( plMipmap *destBMap, plMipmap *srcBMap );
    // Decompresses a DXT1 compressed mipmap into a RGB8888 mipmap
    void    IUncompressMipmapDXT1To32( plMipmap *destBMap, plMipmap *srcBMap );

    // Decompresses a DXT1 compressed mipmap into an intensity map
    void    IUncompressMipmapDXT1ToInten( plMipmap *destBMap, plMipmap *srcBMap );
    // Decompresses a DXT5 compressed mipmap into an alpha-intensity map
    void    IUncompressMipmapDXT5ToAInten( plMipmap *destBMap, plMipmap *srcBMap );

    // Mixes two RGB8888 colors equally
    uint32_t inline IMixEqualRGB32( uint32_t color1, uint32_t color2 );
    // Mixes two-thirds of the first RGB8888 color and one-third of the second
    uint32_t inline IMixTwoThirdsRGB32( uint32_t twoThirds, uint32_t oneThird );

    // Mixes two RGB1555 colors equally
    uint16_t inline IMixEqualRGB1555( uint16_t color1, uint16_t color2 );
    // Mixes two-thirds of the first RGB1555 color and one-third of the second
    uint16_t inline IMixTwoThirdsRGB1555( uint16_t twoThirds, uint16_t oneThird );

    // Mixes two RGB5551 colors equally
    uint16_t inline IMixEqualRGB5551( uint16_t color1, uint16_t color2 );
    // Mixes two-thirds of the first RGB5551 color and one-third of the second
    uint16_t inline IMixTwoThirdsRGB5551( uint16_t twoThirds, uint16_t oneThird );

    // Mixes two RGB4444 colors equally
    uint16_t inline IMixEqualRGB4444( uint16_t color1, uint16_t color2 );
    // Mixes two-thirds of the first RGB4444 color and one-third of the second
    uint16_t inline IMixTwoThirdsRGB4444( uint16_t twoThirds, uint16_t oneThird );
    
    // Mixes two intensity values equally
    uint8_t  inline IMixEqualInten( uint8_t color1, uint8_t color2 );
    // Mixes two-thirds of the first intensity and one-third of the second
    uint8_t  inline IMixTwoThirdsInten( uint8_t twoThirds, uint8_t oneThird );

    // Converts a color from RGB565 to RGB8888 format, with alpha=0
    uint32_t inline IRGB16To32Bit( uint16_t color );
    // Converts a color from RGB565 to RGB4444 format, with alpha=0
    uint16_t inline IRGB565To4444( uint16_t color );
    // Converts a color from RGB565 to RGB1555 format, with alpha=0
    uint16_t inline IRGB565To1555( uint16_t color );
    // Converts a color from RGB565 to RGB5551 format, with alpha=0
    uint16_t inline IRGB565To5551( uint16_t color );
    // Converts a color from RGB565 to RGB4444 reversed format, with alpha=0
    uint16_t inline IRGB565To4444Rev( uint16_t color );

    static bool Register();
    static bool fRegistered;
};

#endif // __HSDXTSOFTWARECODEC_H
