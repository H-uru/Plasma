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
#include <memory.h>
#include "HeadSpin.h"
#include "hsColorRGBA.h"
#include "hsDXTSoftwareCodec.h"
#include "plMipmap.h"
#include "hsCodecManager.h"

#define SWAPVARS( x, y, t ) { t = x; x = y; y = t; }

// This is the color depth that we decompress to by default if we're not told otherwise
#define kDefaultDepth   32


bool hsDXTSoftwareCodec::fRegistered = false;

hsDXTSoftwareCodec& hsDXTSoftwareCodec::Instance()
{
    static hsDXTSoftwareCodec the_instance;
    
    return the_instance;
}

hsDXTSoftwareCodec::hsDXTSoftwareCodec()
{
}

hsDXTSoftwareCodec::~hsDXTSoftwareCodec()
{
}

//// CreateCompressedMipmap ///////////////////////////////////////////////////
//  Updated 8.15.2000 mcn to generate uncompressed mipmaps down to 1x1 (the
//  decompressor better know how to deal with this!) Also cleaned up so I can
//  read it :)

plMipmap *hsDXTSoftwareCodec::CreateCompressedMipmap( plMipmap *uncompressed )
{
    uint8_t           format;
    plMipmap        *compressed = nullptr;
    uint8_t           i;


    /// Sanity checks
    hsAssert( fRegistered, "Calling member of unregistered codec." );
    hsAssert( !uncompressed->IsCompressed(), "Trying to compress already compressed bitmap.");

    /// Check width and height
    if( ( uncompressed->GetWidth() | uncompressed->GetHeight() ) & 0x03 )
        return nullptr;     /// Width and height must be multiple of 4

    format = ICalcCompressedFormat( uncompressed );
    if( format == plMipmap::DirectXInfo::kError )
        return nullptr;


    /// Set up structure
    compressed = new plMipmap( uncompressed->GetWidth(), uncompressed->GetHeight(), plMipmap::kARGB32Config,
                                uncompressed->GetNumLevels(), plMipmap::kDirectXCompression, format );

    {
        /// Now loop through and compress each one (that is a valid size!)
        for( i = 0; i < compressed->GetNumLevels(); i++ )
        {
            uncompressed->SetCurrLevel( i );
            compressed->SetCurrLevel( i );
            if( ( compressed->GetCurrWidth() | compressed->GetCurrHeight() ) & 0x03 )
                break;

            CompressMipmapLevel( uncompressed, compressed );
        }

        /// Now copy the rest straight over
        for( ; i < compressed->GetNumLevels(); i++ )
            memcpy( compressed->GetLevelPtr( i ), uncompressed->GetLevelPtr( i ), uncompressed->GetLevelSize( i ) );

        /// Reset
        uncompressed->SetCurrLevel( 0 );
        compressed->SetCurrLevel( 0 );
    }

    return compressed;
}

//// CreateUncompressedBitmap /////////////////////////////////////////////////
//
//  Takes a compressed bitmap and uncompresses it. Duh!
//
//  8.14.2000 mcn - Updated to decompress to 16-bit modes and set flags
//                  accordingly.
//  8.15.2000 mcn - Updated to handle compressed mipmaps with invalid levels
//                  (i.e. they're not compressed). See CreateCompressedMipmap().
//  8.18.2000 mcn - Updated to handle new flags instead of just a bit depth

plMipmap *hsDXTSoftwareCodec::CreateUncompressedMipmap( plMipmap *compressed, uint8_t flags )
{
    plMipmap    *uncompressed = nullptr;
    int32_t       totalSize;
    int32_t       width, height;
    uint8_t       i;


    /// Sanity checks
    hsAssert( fRegistered, "Calling member of unregistered codec." );
    hsAssert( compressed->fCompressionType == plMipmap::kDirectXCompression, "Uncompressing wrong format." );
    hsAssert( ( compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1 ) ||
              ( compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5 ),
              "Unsupported directX compression format." );


    /// Set up the info
    uncompressed = ICreateUncompressedMipmap( compressed, flags );

    /// Handle mipmaps?
    {
        totalSize = 0;
        width = compressed->GetWidth();
        height = compressed->GetHeight();

        for( i = 0; i < compressed->GetNumLevels(); i++ )
        {
            /// Note: THIS is valid no matter whether the compressed level is
            /// really compressed or not
            totalSize += width * height * uncompressed->GetPixelSize() >> 3;
            width >>= 1;
            height >>= 1;
        }

        /// Loop through and decompress!
        width = compressed->GetWidth();
        height = compressed->GetHeight();
        for( i = 0; i < uncompressed->GetNumLevels(); i++ )
        {
            uncompressed->SetCurrLevel( i );
            compressed->SetCurrLevel( i );

            if( ( width | height ) & 0x03 )
                break;

            UncompressMipmap( uncompressed, compressed, flags );

            if( width > 1 )
                width >>= 1;
            if( height > 1 )
                height >>= 1;
        }

        /// Now loop through the *rest* and just copy (they won't be compressed)
        for( ; i < uncompressed->GetNumLevels(); i++ )
        {
            uncompressed->SetCurrLevel( i );
            compressed->SetCurrLevel( i );

            IXlateColorBlock( (plMipmap *)uncompressed, (uint32_t *)compressed->GetLevelPtr( i ), flags );
        }

        /// Reset
        uncompressed->SetCurrLevel( 0 );
        compressed->SetCurrLevel( 0 );
    }

    return uncompressed;
}

//// IXlateColorBlock /////////////////////////////////////////////////////////
//  Converts a block from ARGB 8888 to the given format of the bitmap.

void    hsDXTSoftwareCodec::IXlateColorBlock( plMipmap *destBMap, uint32_t *srcBlock, uint8_t flags )
{
    uint8_t       *bytePtr;
    uint16_t      *wordPtr, tempVal;
    uint32_t      i;


    if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB8888 )
        memcpy( destBMap->GetCurrLevelPtr(), srcBlock, destBMap->GetCurrLevelSize() );
    else
    {
        if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kAInten88 )
        {
            /// Upper 8 bits from alpha, lower 8 bits from blue
            wordPtr = (uint16_t *)destBMap->GetCurrLevelPtr();
            for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
            {
                wordPtr[ i ] = (uint16_t)( ( ( srcBlock[ i ] >> 16 ) & 0xff00 ) | ( srcBlock[ i ] & 0x00ff ) );
            }
        }
        else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kInten8 )
        {
            /// 8 bits from blue
            bytePtr = (uint8_t *)destBMap->GetCurrLevelPtr();
            for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
            {
                bytePtr[ i ] = (uint8_t)( srcBlock[ i ] & 0x00ff );
            }
        }
        else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB1555 )
        {
            wordPtr = (uint16_t *)destBMap->GetCurrLevelPtr();

            if( ( flags & hsCodecManager::kCompOrderMask ) == hsCodecManager::kWeirdCompOrder )
            {
                /// Really do 5551
                for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
                {
                    tempVal = (uint16_t)( ( ( srcBlock[ i ] & 0x000000f8 ) >> 2 ) |
                              ( ( ( srcBlock[ i ] & 0x0000f800 ) >> 5 ) ) |
                              ( ( ( srcBlock[ i ] & 0x00f80000 ) >> 8 ) ) |
                              ( ( srcBlock[ i ] & 0x80000000 ) >> 31 ) );

                    wordPtr[ i ] = tempVal;
                }
            }
            else
            {
                /// Normal 1555
                for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
                {
                    tempVal = (uint16_t)( ( ( srcBlock[ i ] & 0x000000f8 ) >> 3 ) |
                              ( ( ( srcBlock[ i ] & 0x0000f800 ) >> 6 ) ) |
                              ( ( ( srcBlock[ i ] & 0x00f80000 ) >> 9 ) ) |
                              ( ( srcBlock[ i ] & 0x80000000 ) >> 16 ) );

                    wordPtr[ i ] = tempVal;
                }
            }
        }
        else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB4444 )
        {
            wordPtr = (uint16_t *)destBMap->GetCurrLevelPtr();

            if( ( flags & hsCodecManager::kCompOrderMask ) == hsCodecManager::kWeirdCompOrder )
            {
                /// Really do 4444 reversed (switch red and blue)
                for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
                {
                    tempVal = (uint16_t)( ( ( srcBlock[ i ] & 0x000000f0 ) << 4 ) |
                              ( ( ( srcBlock[ i ] & 0x0000f000 ) >> 8 ) ) |
                              ( ( ( srcBlock[ i ] & 0x00f00000 ) >> 20 ) ) |
                              ( ( ( srcBlock[ i ] & 0xf0000000 ) >> 16 ) ) );

                    wordPtr[ i ] = tempVal;
                }
            }
            else
            {
                /// Normal 4444
                for( i = 0; i < destBMap->GetCurrWidth() * destBMap->GetCurrHeight(); i++ )
                {
                    tempVal = (uint16_t)( ( ( srcBlock[ i ] & 0x000000f0 ) >> 4 ) |
                              ( ( ( srcBlock[ i ] & 0x0000f000 ) >> 8 ) ) |
                              ( ( ( srcBlock[ i ] & 0x00f00000 ) >> 12 ) ) |
                              ( ( ( srcBlock[ i ] & 0xf0000000 ) >> 16 ) ) );

                    wordPtr[ i ] = tempVal;
                }
            }
        }
        else
            hsAssert( false, "Unsupported pixel format during color block translation" );
    }
}

//// ICreateUncompressedMipmap ////////////////////////////////////////////////
//  Sets the fields of an uncompressed bitmap according to the source
//  (compressed) bitmap and the given bit depth.

plMipmap    *hsDXTSoftwareCodec::ICreateUncompressedMipmap( plMipmap *compressed, uint8_t flags )
{
    uint8_t       bitDepth, type;
    plMipmap    *newMap;


    if( compressed->GetFlags() & plMipmap::kIntensityMap )
    {
        /// Intensity map--alpha too?
        if( compressed->GetFlags() & plMipmap::kAlphaChannelFlag )
        {
            type = plMipmap::UncompressedInfo::kAInten88;
            bitDepth = 16;
        }
        else
        {
            type = plMipmap::UncompressedInfo::kInten8;
            bitDepth = 8;
        }
    }
    else
    {
        /// Default to 32 bit depth
        if( ( flags & hsCodecManager::kBitDepthMask ) == hsCodecManager::kDontCareDepth )
            bitDepth = kDefaultDepth;
        else if( ( flags & hsCodecManager::kBitDepthMask ) == hsCodecManager::k32BitDepth )
            bitDepth = 32;
        else
            bitDepth = 16;

        if( bitDepth == 32 )
            type = plMipmap::UncompressedInfo::kRGB8888;
        else if( compressed->GetFlags() & plMipmap::kAlphaChannelFlag )
            type = plMipmap::UncompressedInfo::kRGB4444;
        else
            type = plMipmap::UncompressedInfo::kRGB1555;
    }

    newMap = new plMipmap( compressed->GetWidth(), compressed->GetHeight(), bitDepth,
                            compressed->GetNumLevels(), plMipmap::kUncompressed, type );
    newMap->SetFlags( compressed->GetFlags() );                         

    return newMap;
}



/*
-----------------------------------------------------------
  OLD DECOMPRESSION FUNCTION--KEEPING HERE FOR LEGACY
-----------------------------------------------------------

void hsDXTSoftwareCodec::UncompressMipmap(plMipmap *uncompressed, plMipmap *compressed)
{
    uint32_t *compressedImage = (uint32_t *)compressed->GetCurrLevelPtr();
    uint32_t *uncompressedImage = (uint32_t *)uncompressed->GetCurrLevelPtr();
    uint32_t blockSize = compressed->fDirectXInfo.fBlockSize;
    int32_t x, y;
    int32_t xMax = compressed->GetCurrWidth() >> 2;
    int32_t yMax = compressed->GetCurrHeight() >> 2;
    for (x = 0; x < xMax; ++x)
    {
        for (y = 0; y < yMax; ++y)
        {
            uint8_t alpha[8];
            uint16_t color[4];
            uint32_t *block = &compressedImage[(x + xMax * y) * (blockSize >> 2)];
            uint8_t *charBlock = (uint8_t *)block;
            uint8_t *alphaBlock = nullptr;
            uint8_t *colorBlock = nullptr;

            if (compressed->fDirectXInfo.fCompressionType == hsGBitmap::DirectXInfo::kDXT5)
            {
                alphaBlock = charBlock;
                colorBlock = (charBlock + 8);

                alpha[0] = charBlock[0];
                alpha[1] = charBlock[1];

                // 8-alpha or 6-alpha block?    
                if (alpha[0] > alpha[1]) {    
                    // 8-alpha block:  derive the other 6 alphas.    
                    // 000 = alpha[0], 001 = alpha[1], others are interpolated
                    alpha[2] = (6 * alpha[0] + alpha[1]) / 7;      // bit code 010
                    alpha[3] = (5 * alpha[0] + 2 * alpha[1]) / 7;  // Bit code 011    
                    alpha[4] = (4 * alpha[0] + 3 * alpha[1]) / 7;  // Bit code 100    
                    alpha[5] = (3 * alpha[0] + 4 * alpha[1]) / 7;  // Bit code 101
                    alpha[6] = (2 * alpha[0] + 5 * alpha[1]) / 7;  // Bit code 110    
                    alpha[7] = (alpha[0] + 6 * alpha[1]) / 7;      // Bit code 111
                }    
                else {  // 6-alpha block:  derive the other alphas.    
                    // 000 = alpha[0], 001 = alpha[1], others are interpolated
                    alpha[2] = (4 * alpha[0] + alpha[1]) / 5;      // Bit code 010
                    alpha[3] = (3 * alpha[0] + 2 * alpha[1]) / 5;  // Bit code 011    
                    alpha[4] = (2 * alpha[0] + 3 * alpha[1]) / 5;  // Bit code 100    
                    alpha[5] = (alpha[0] + 4 * alpha[1]) / 5;      // Bit code 101
                    alpha[6] = 0;                                // Bit code 110
                    alpha[7] = 255;                              // Bit code 111
                }
            }
            else if (compressed->fDirectXInfo.fCompressionType == hsGBitmap::DirectXInfo::kDXT1)
            {
                alphaBlock = nullptr;
                colorBlock = charBlock;
            }
            else
            {
                hsAssert(false, "Unrecognized compression scheme.");
            }

            uint32_t encoding;
            color[0] = (colorBlock[1] << 8) | colorBlock[0];
            color[1] = (colorBlock[3] << 8) | colorBlock[2];

            if (color[0] > color[1]) 
            {
                // Four-color block: derive the other two colors.    
                // 00 = color[0], 01 = color[1], 10 = color[2, 11 = color[3
                // These two bit codes correspond to the 2-bit fields 
                // stored in the 64-bit block.
                color[2] = BlendColors16(2, color[0], 1, color[1]);
                color[3] = BlendColors16(1, color[0], 2, color[1]);

                encoding = kFourColorEncoding;
            }    
            else
            { 
                // Three-color block: derive the other color.
                // 00 = color[0],  01 = color[1],  10 = color[2,  
                // 11 = transparent.
                // These two bit codes correspond to the 2-bit fields 
                // stored in the 64-bit block. 
                color[2] = BlendColors16(1, color[0], 1, color[1]);
                color[3] = 0;    

                encoding = kThreeColorEncoding;
            }

            uint8_t r, g, b, a;
            int32_t xx, yy;
            for (xx = 0; xx < 4; ++xx)
            {
                for (yy = 0; yy < 4; ++yy)
                {
                    if (alphaBlock)
                    {
                        uint32_t alphaMask = 0x7;
                        uint32_t firstThreeBytes = (alphaBlock[4] << 16) + (alphaBlock[3] << 8) + alphaBlock[2];
                        uint32_t secondThreeBytes = (alphaBlock[7] << 16) + (alphaBlock[6] << 8) + alphaBlock[5];
                        uint32_t alphaIndex;
                        uint32_t alphaShift;
                        if (yy < 2)
                        {
                            alphaShift = 3 * (4 * yy + xx);
                            alphaIndex = (firstThreeBytes >> alphaShift) & alphaMask;
                        }
                        else
                        {
                            alphaShift = 3 * (4 * (yy - 2) + xx);
                            alphaIndex = (secondThreeBytes >> alphaShift) & alphaMask;
                        }

                        a = alpha[alphaIndex];
                    }
                    else
                    {
                        a = 255;
                    }

                    uint32_t colorMask = 0x3;
                    uint32_t colorDWord = (colorBlock[7] << 24) | (colorBlock[6] << 16) | 
                        (colorBlock[5] << 8) | colorBlock[4];
                    uint32_t colorShift = 2 * (4 * yy + xx);
                    uint32_t colorIndex = (colorDWord >> colorShift) & colorMask;

                    if ((encoding == kThreeColorEncoding) && (colorIndex == 3))
                    {
                        r = g = b = a = 0;
                    }
                    else
                    {
                        r = (uint8_t)((color[colorIndex] >> 8) & 0xf8);
                        g = (uint8_t)((color[colorIndex] >> 3) & 0xfc);
                        b = (uint8_t)((color[colorIndex] << 3) & 0xf8);
                    }

                    hsRGBAColor32* pixel = (hsRGBAColor32*)uncompressed->GetAddr32(4 * x + xx, 4 * y + yy);
                    pixel->a = a;
                    pixel->r = r;
                    pixel->g = g;
                    pixel->b = b;
                }
            }   
        }
    }
}
*/

//// UncompressBitmap /////////////////////////////////////////////////////////
//
//  Workhorse function distribution. Takes a compressed GBitmapClass as input 
//  and writes the uncompressed version to the destination bitmap class.
//  (Doesn't actually do anything but call the appropriate function per format.
//  Change this function to add support for more compression formats.)
//
//  Note: Supports DXT1 and DXT5 compression formats.
//
//  7.31.2000 mcn - Created, based on old code (uncredited)
//  8.18.2000 mcn - Updated to handle 'weird' formats and other flags.

void    hsDXTSoftwareCodec::UncompressMipmap( plMipmap *destBMap, plMipmap *srcBMap, uint8_t flags )
{
    if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB8888 )
    {
        /// 32-bit ARGB - Can be either DXT5 or DXT1
        if( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5 )
            IUncompressMipmapDXT5To32( destBMap, srcBMap );
        else if( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1 )
            IUncompressMipmapDXT1To32( destBMap, srcBMap );
    }
    else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB1555 )
    {
        /// 16-bit ARGB 1555--can ONLY be DXT1
        hsAssert( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1,
                    "Only DXT1 bitmaps can decompress to ARGB1555 format!" );
    
        if( ( flags & hsCodecManager::kCompOrderMask ) == hsCodecManager::kWeirdCompOrder )
            IUncompressMipmapDXT1To16Weird( destBMap, srcBMap );
        else
            IUncompressMipmapDXT1To16( destBMap, srcBMap );
    }
    else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kRGB4444 )
    {
        /// 16-bit ARGB 4444--can ONLY be DXT5
        hsAssert( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5,
                    "Only DXT5 bitmaps can decompress to ARGB4444 format!" );

        if( ( flags & hsCodecManager::kCompOrderMask ) == hsCodecManager::kWeirdCompOrder )
            IUncompressMipmapDXT5To16Weird( destBMap, srcBMap );
        else
            IUncompressMipmapDXT5To16( destBMap, srcBMap );
    }
    else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kInten8 )
    {
        /// 8-bit intensity--can ONLY be DXT1
        hsAssert( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1,
                    "Only DXT1 bitmaps can decompress to 8-bit Intensity format!" );
        IUncompressMipmapDXT1ToInten( destBMap, srcBMap );
    }
    else if( destBMap->fUncompressedInfo.fType == plMipmap::UncompressedInfo::kAInten88 )
    {
        /// 16-bit alpha-intensity--can ONLY be DXT5
        hsAssert( srcBMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5,
                    "Only DXT5 bitmaps can decompress to 8-8 Alpha-Intensity format!" );
        IUncompressMipmapDXT5ToAInten( destBMap, srcBMap );
    }
    else
        hsAssert( false, "Unsupported target decompression format" );
}


//// IUncompressMipmapDXT5To16 ////////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT5 compression. DXT5 is 3-bit linear
//  interpolated alpha channel compression. Output is a 16-bit RGB 4444 bitmap.

void    hsDXTSoftwareCodec::IUncompressMipmapDXT5To16( plMipmap *destBMap, plMipmap *srcBMap )
{
    uint16_t      *srcData;
    uint16_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      x, y, bMapStride;
    uint16_t      colors[ 4 ];
    int32_t       numBlocks, i, j;
    uint8_t       *bytePtr;
    uint16_t      alphas[ 8 ], aTemp, a0, a1;

    uint32_t      aBitSrc1, aBitSrc2;
    uint16_t      cBitSrc1, cBitSrc2;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr16's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr16( 0, 1 ) - destBMap->GetAddr16( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Per block--determine alpha compression type first
        bytePtr = (uint8_t *)srcData;
        alphas[ 0 ] = bytePtr[ 0 ];
        alphas[ 1 ] = bytePtr[ 1 ];

        /// Note that we use the preshifted alphas really as fixed point.
        /// The result: more accuracy, and no need to shift the alphas afterwards
        if( alphas[ 0 ] > alphas[ 1 ] )
        {
            /// 8-alpha block: interpolate 6 others
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            /// Note that, unlike below, we can't combine a0 and a1 into
            /// one value, because that would give us a negative value,
            /// and we're using unsigned values here. (i.e. we need all the bits)
            aTemp = alphas[ 0 ];
            a0 = ( aTemp / 7 );
            a1 = ( alphas[ 1 ] / 7 );           
            for( j = 2; j < 8; j++ )
            {
                aTemp += a1 - a0;
                alphas[ j ] = aTemp & 0xf000;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }
        }
        else
        {
            /// 6-alpha block: interpolate 4 others, then assume last 2 are 0 and 255
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            aTemp = alphas[ 0 ];
            a0 = ( alphas[ 1 ] - aTemp ) / 5;
            for( j = 2; j < 6; j++ )
            {
                aTemp += a0;
                alphas[ j ] = aTemp & 0xf000;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }

            alphas[ 6 ] = 0;
            alphas[ 7 ] = 0xf000;
        }

        /// Mask off the original two now
        alphas[ 0 ] &= 0xf000;
        alphas[ 1 ] &= 0xf000;

        /// Now do the 16 pixels in 2 blocks, decompressing 3-bit lookups
        aBitSrc1 = ( (uint32_t)bytePtr[ 4 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 3 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 2 ] );
        aBitSrc2 = ( (uint32_t)bytePtr[ 7 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 6 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 5 ] );

        /// Now decompress color data
        srcData += 4;       // Alpha was 4 16-bit words worth
        //hsAssert( srcData[ 0 ] > srcData[ 1 ], "Invalid block compression for DX5 method" );  /// If not, then it's one-bit
                                                /// alpha, but this is DX5!
        colors[ 0 ] = IRGB565To4444( srcData[ 0 ] );
        colors[ 1 ] = IRGB565To4444( srcData[ 1 ] );
        colors[ 2 ] = IMixTwoThirdsRGB4444( colors[ 0 ], colors[ 1 ] );
        colors[ 3 ] = IMixTwoThirdsRGB4444( colors[ 1 ], colors[ 0 ] );
        
        cBitSrc1 = hsToLE16( srcData[ 2 ] );
        cBitSrc2 = hsToLE16( srcData[ 3 ] );
        
        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = alphas[ aBitSrc1 & 0x07 ] | colors[ cBitSrc1 & 0x03 ];
            aBitSrc1 >>= 3;
            cBitSrc1 >>= 2;
            destBlock[ j + 8 ] = alphas[ aBitSrc2 & 0x07 ] | colors[ cBitSrc2 & 0x03 ];
            aBitSrc2 >>= 3;
            cBitSrc2 >>= 2;

            destBlock[ j ] = hsToLE16( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE16( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr16( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize - 4;       /// JUUUST in case our block size is diff
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT5To16Weird ///////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT5 compression. DXT5 is 3-bit linear
//  interpolated alpha channel compression. Output is a 16-bit RGB 4444 
//  reversed bitmap (Red and blue are swapped ala OpenGL).
//  (Note: Annoyingly enough, this is exactly the same as the above function
//  EXCEPT for two stupid lines. We can't use function pointers as the two
//  function calls are inline. Wouldn't it be nice if we could somehow write
//  the inline opcodes beforehand?)

void    hsDXTSoftwareCodec::IUncompressMipmapDXT5To16Weird( plMipmap *destBMap, plMipmap *srcBMap )
{
    uint16_t      *srcData;
    uint16_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      x, y, bMapStride;
    uint16_t      colors[ 4 ];
    int32_t       numBlocks, i, j;
    uint8_t       *bytePtr;
    uint16_t      alphas[ 8 ], aTemp, a0, a1;

    uint32_t      aBitSrc1, aBitSrc2;
    uint16_t      cBitSrc1, cBitSrc2;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr16's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr16( 0, 1 ) - destBMap->GetAddr16( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Per block--determine alpha compression type first
        bytePtr = (uint8_t *)srcData;
        alphas[ 0 ] = bytePtr[ 0 ];
        alphas[ 1 ] = bytePtr[ 1 ];

        /// Note that we use the preshifted alphas really as fixed point.
        /// The result: more accuracy, and no need to shift the alphas afterwards
        if( alphas[ 0 ] > alphas[ 1 ] )
        {
            /// 8-alpha block: interpolate 6 others
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            /// Note that, unlike below, we can't combine a0 and a1 into
            /// one value, because that would give us a negative value,
            /// and we're using unsigned values here. (i.e. we need all the bits)
            aTemp = alphas[ 0 ];
            a0 = ( aTemp / 7 );
            a1 = ( alphas[ 1 ] / 7 );           
            for( j = 2; j < 8; j++ )
            {
                aTemp += a1 - a0;
                alphas[ j ] = aTemp & 0xf000;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }
        }
        else
        {
            /// 6-alpha block: interpolate 4 others, then assume last 2 are 0 and 255
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            aTemp = alphas[ 0 ];
            a0 = ( alphas[ 1 ] - aTemp ) / 5;
            for( j = 2; j < 6; j++ )
            {
                aTemp += a0;
                alphas[ j ] = aTemp & 0xf000;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }

            alphas[ 6 ] = 0;
            alphas[ 7 ] = 0xf000;
        }

        /// Mask off the original two now
        alphas[ 0 ] &= 0xf000;
        alphas[ 1 ] &= 0xf000;

        /// Now do the 16 pixels in 2 blocks, decompressing 3-bit lookups
        aBitSrc1 = ( (uint32_t)bytePtr[ 4 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 3 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 2 ] );
        aBitSrc2 = ( (uint32_t)bytePtr[ 7 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 6 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 5 ] );

        /// Now decompress color data
        srcData += 4;       // Alpha was 4 16-bit words worth
        colors[ 0 ] = IRGB565To4444Rev( srcData[ 0 ] );
        colors[ 1 ] = IRGB565To4444Rev( srcData[ 1 ] );
        colors[ 2 ] = IMixTwoThirdsRGB4444( colors[ 0 ], colors[ 1 ] );
        colors[ 3 ] = IMixTwoThirdsRGB4444( colors[ 1 ], colors[ 0 ] );
        
        cBitSrc1 = hsToLE16( srcData[ 2 ] );
        cBitSrc2 = hsToLE16( srcData[ 3 ] );
        
        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = alphas[ aBitSrc1 & 0x07 ] | colors[ cBitSrc1 & 0x03 ];
            aBitSrc1 >>= 3;
            cBitSrc1 >>= 2;
            destBlock[ j + 8 ] = alphas[ aBitSrc2 & 0x07 ] | colors[ cBitSrc2 & 0x03 ];
            aBitSrc2 >>= 3;
            cBitSrc2 >>= 2;

            destBlock[ j ] = hsToLE16( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE16( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr16( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize - 4;       /// JUUUST in case our block size is diff
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT5To32 ////////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT5 compression. DXT5 is 3-bit linear
//  interpolated alpha channel compression. Output is a 32-bit ARGB 8888 bitmap.
//
//  7.31.2000 - M.Burrack - Created, based on old code (uncredited)
//
//  8.14.2000 - M.Burrack - Optimized on the alpha blending. Now we precalc
//                          the divided values and run a for loop. This gets
//                          us only about 10% :(

void    hsDXTSoftwareCodec::IUncompressMipmapDXT5To32( plMipmap *destBMap, plMipmap *srcBMap )
{
    uint16_t      *srcData;
    uint32_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      x, y, bMapStride;
    uint32_t      colors[ 4 ];
    int32_t       numBlocks, i, j;
    uint8_t       *bytePtr;
    uint32_t      alphas[ 8 ], aTemp, a0, a1;

    uint32_t      aBitSrc1, aBitSrc2;
    uint16_t      cBitSrc1, cBitSrc2;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr32's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr32( 0, 1 ) - destBMap->GetAddr32( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Per block--determine alpha compression type first
        bytePtr = (uint8_t *)srcData;
        alphas[ 0 ] = bytePtr[ 0 ];
        alphas[ 1 ] = bytePtr[ 1 ];

        /// Note that we use the preshifted alphas really as fixed point.
        /// The result: more accuracy, and no need to shift the alphas afterwards
        if( alphas[ 0 ] > alphas[ 1 ] )
        {
            /// 8-alpha block: interpolate 6 others
/*          //// Here's the old code, for reference ////
            alphas[ 2 ] = ( 6 * alphas[ 0 ] +     alphas[ 1 ] ) / 7;
            alphas[ 3 ] = ( 5 * alphas[ 0 ] + 2 * alphas[ 1 ] ) / 7;
            alphas[ 4 ] = ( 4 * alphas[ 0 ] + 3 * alphas[ 1 ] ) / 7;
            alphas[ 5 ] = ( 3 * alphas[ 0 ] + 4 * alphas[ 1 ] ) / 7;
            alphas[ 6 ] = ( 2 * alphas[ 0 ] + 5 * alphas[ 1 ] ) / 7;
            alphas[ 7 ] = (     alphas[ 0 ] + 6 * alphas[ 1 ] ) / 7;
*/
            alphas[ 0 ] <<= 24;
            alphas[ 1 ] <<= 24;

            /// Note that, unlike below, we can't combine a0 and a1 into
            /// one value, because that would give us a negative value,
            /// and we're using unsigned values here. (i.e. we need all the bits)
            aTemp = alphas[ 0 ];
            a0 = ( aTemp / 7 ) & 0xff000000;
            a1 = ( alphas[ 1 ] / 7 ) & 0xff000000;          
            for( j = 2; j < 8; j++ )
            {
                aTemp += a1 - a0;
                alphas[ j ] = aTemp;
            }
        }
        else
        {
            /// 6-alpha block: interpolate 4 others, then assume last 2 are 0 and 255
/*          //// Here's the old code, for reference ////
            alphas[ 2 ] = ( 4 * alphas[ 0 ] +     alphas[ 1 ] ) / 5;
            alphas[ 3 ] = ( 3 * alphas[ 0 ] + 2 * alphas[ 1 ] ) / 5;
            alphas[ 4 ] = ( 2 * alphas[ 0 ] + 3 * alphas[ 1 ] ) / 5;
            alphas[ 5 ] = (     alphas[ 0 ] + 4 * alphas[ 1 ] ) / 5;
*/
            alphas[ 0 ] <<= 24;
            alphas[ 1 ] <<= 24;

            aTemp = alphas[ 0 ];
            a0 = ( alphas[ 1 ] - aTemp ) / 5;
            for( j = 2; j < 6; j++ )
            {
                aTemp += a0;
                alphas[ j ] = aTemp & 0xff000000;
            }

            alphas[ 6 ] = 0;
            alphas[ 7 ] = 255 << 24;
        }

        /// Now do the 16 pixels in 2 blocks, decompressing 3-bit lookups
        aBitSrc1 = ( (uint32_t)bytePtr[ 4 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 3 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 2 ] );
        aBitSrc2 = ( (uint32_t)bytePtr[ 7 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 6 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 5 ] );

        /// Now decompress color data
        srcData += 4;       // Alpha was 4 16-bit words worth
        //hsAssert( srcData[ 0 ] > srcData[ 1 ], "Invalid block compression for DX5 method" );  /// If not, then it's one-bit
                                                /// alpha, but this is DX5!
        colors[ 0 ] = IRGB16To32Bit( srcData[ 0 ] );
        colors[ 1 ] = IRGB16To32Bit( srcData[ 1 ] );
        colors[ 2 ] = IMixTwoThirdsRGB32( colors[ 0 ], colors[ 1 ] );
        colors[ 3 ] = IMixTwoThirdsRGB32( colors[ 1 ], colors[ 0 ] );
        
        cBitSrc1 = hsToLE16( srcData[ 2 ] );
        cBitSrc2 = hsToLE16( srcData[ 3 ] );
        
        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = alphas[ aBitSrc1 & 0x07 ] | colors[ cBitSrc1 & 0x03 ];
            aBitSrc1 >>= 3;
            cBitSrc1 >>= 2;
            destBlock[ j + 8 ] = alphas[ aBitSrc2 & 0x07 ] | colors[ cBitSrc2 & 0x03 ];
            aBitSrc2 >>= 3;
            cBitSrc2 >>= 2;

            destBlock[ j ] = hsToLE32( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE32( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr32( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize - 4;       /// JUUUST in case our block size is diff
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT5ToAInten ////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT5 compression. DXT5 is 3-bit linear
//  interpolated alpha channel compression. Output is a 16-bit Alpha-intensity
//  map.

void    hsDXTSoftwareCodec::IUncompressMipmapDXT5ToAInten( plMipmap *destBMap, plMipmap *srcBMap )
{
    uint16_t      *srcData;
    uint16_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      x, y, bMapStride;
    uint8_t       colors[ 4 ];
    int32_t       numBlocks, i, j;
    uint8_t       *bytePtr;
    uint16_t      alphas[ 8 ], aTemp, a0, a1;

    uint32_t      aBitSrc1, aBitSrc2;
    uint16_t      cBitSrc1, cBitSrc2;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr32's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr16( 0, 1 ) - destBMap->GetAddr16( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Per block--determine alpha compression type first
        bytePtr = (uint8_t *)srcData;
        alphas[ 0 ] = bytePtr[ 0 ];
        alphas[ 1 ] = bytePtr[ 1 ];

        /// Note that we use the preshifted alphas really as fixed point.
        /// The result: more accuracy, and no need to shift the alphas afterwards
        if( alphas[ 0 ] > alphas[ 1 ] )
        {
            /// 8-alpha block: interpolate 6 others
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            /// Note that, unlike below, we can't combine a0 and a1 into
            /// one value, because that would give us a negative value,
            /// and we're using unsigned values here. (i.e. we need all the bits)
            aTemp = alphas[ 0 ];
            a0 = ( aTemp / 7 );
            a1 = ( alphas[ 1 ] / 7 );           
            for( j = 2; j < 8; j++ )
            {
                aTemp += a1 - a0;
                alphas[ j ] = aTemp & 0xff00;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }
        }
        else
        {
            /// 6-alpha block: interpolate 4 others, then assume last 2 are 0 and 255
            alphas[ 0 ] <<= 8;
            alphas[ 1 ] <<= 8;

            aTemp = alphas[ 0 ];
            a0 = ( alphas[ 1 ] - aTemp ) / 5;
            for( j = 2; j < 6; j++ )
            {
                aTemp += a0;
                alphas[ j ] = aTemp & 0xff00;   /// Mask done here to retain as
                                                /// much accuracy as possible
            }

            alphas[ 6 ] = 0;
            alphas[ 7 ] = 0xff00;
        }

        /// Now do the 16 pixels in 2 blocks, decompressing 3-bit lookups
        aBitSrc1 = ( (uint32_t)bytePtr[ 4 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 3 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 2 ] );
        aBitSrc2 = ( (uint32_t)bytePtr[ 7 ] << 16 ) + 
                    ( (uint32_t)bytePtr[ 6 ] << 8 ) + 
                    ( (uint32_t)bytePtr[ 5 ] );

        /// Now decompress color data
        srcData += 4;       // Alpha was 4 16-bit words worth
        colors[ 0 ] = (uint8_t)IRGB16To32Bit( srcData[ 0 ] );
        colors[ 1 ] = (uint8_t)IRGB16To32Bit( srcData[ 1 ] );
        colors[ 2 ] = IMixTwoThirdsInten( colors[ 0 ], colors[ 1 ] );
        colors[ 3 ] = IMixTwoThirdsInten( colors[ 1 ], colors[ 0 ] );
        
        cBitSrc1 = hsToLE16( srcData[ 2 ] );
        cBitSrc2 = hsToLE16( srcData[ 3 ] );
        
        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = alphas[ aBitSrc1 & 0x07 ] | (uint16_t)colors[ cBitSrc1 & 0x03 ];
            aBitSrc1 >>= 3;
            cBitSrc1 >>= 2;
            destBlock[ j + 8 ] = alphas[ aBitSrc2 & 0x07 ] | (uint16_t)colors[ cBitSrc2 & 0x03 ];
            aBitSrc2 >>= 3;
            cBitSrc2 >>= 2;

            destBlock[ j ] = hsToLE16( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE16( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr16( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize - 4;       /// JUUUST in case our block size is diff
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT1To16 ////////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT1 compression. DXT1 is a simple on/off
//  or all-on alpha 'compression'.
//
//  Note: this version decompresses to a 1-5-5-5 ARGB format.

void    hsDXTSoftwareCodec::IUncompressMipmapDXT1To16( plMipmap *destBMap, plMipmap *srcBMap )
{
    uint16_t      *srcData, tempW1, tempW2;
    uint16_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      bitSource, bitSource2, x, y, bMapStride;
    uint16_t      colors[ 4 ];
    int32_t       numBlocks, i, j;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr32's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr16( 0, 1 ) - destBMap->GetAddr16( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Decompress color data block
        colors[ 0 ] = IRGB565To1555( srcData[ 0 ] ) | 0x8000;       // Make sure alpha is set
        colors[ 1 ] = IRGB565To1555( srcData[ 1 ] ) | 0x8000;       // Make sure alpha is set

        tempW1 = hsToLE16( srcData[ 0 ] );
        tempW2 = hsToLE16( srcData[ 1 ] );

        if( tempW1 > tempW2 )
        {
            /// Four-color block--mix the other two
            colors[ 2 ] = IMixTwoThirdsRGB1555( colors[ 0 ], colors[ 1 ] ) | 0x8000;//IMixTwoThirdsRGB32( colors[ 0 ], colors[ 1 ] ) | 0xff000000;
            colors[ 3 ] = IMixTwoThirdsRGB1555( colors[ 1 ], colors[ 0 ] ) | 0x8000;//IMixTwoThirdsRGB32( colors[ 1 ], colors[ 0 ] ) | 0xff000000;
        }
        else
        {
            /// Three-color block and transparent
            colors[ 2 ] = IMixEqualRGB1555( colors[ 0 ], colors[ 1 ] ) | 0x8000;//IMixEqualRGB32( colors[ 0 ], colors[ 1 ] ) | 0xff000000;
            colors[ 3 ] = 0;
        }

        bitSource = hsToLE16( srcData[ 2 ] );
        bitSource2 = hsToLE16( srcData[ 3 ] );

        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = colors[ bitSource & 0x03 ];
            bitSource >>= 2;
            destBlock[ j + 8 ] = colors[ bitSource2 & 0x03 ];
            bitSource2 >>= 2;

            destBlock[ j ] = hsToLE16( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE16( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr16( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize;
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT1To16Weird ///////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT1 compression. DXT1 is a simple on/off
//  or all-on alpha 'compression'.
//
//  Note: this version decompresses to a 5-5-5-1 RGBA format.

void    hsDXTSoftwareCodec::IUncompressMipmapDXT1To16Weird( plMipmap *destBMap, 
                                                   plMipmap *srcBMap )
{
    uint16_t      *srcData, tempW1, tempW2;
    uint16_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      bitSource, bitSource2, x, y, bMapStride;
    uint16_t      colors[ 4 ];
    int32_t       numBlocks, i, j;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr32's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr16( 0, 1 ) - destBMap->GetAddr16( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Decompress color data block
        colors[ 0 ] = IRGB565To5551( srcData[ 0 ] ) | 0x0001;       // Make sure alpha is set
        colors[ 1 ] = IRGB565To5551( srcData[ 1 ] ) | 0x0001;       // Make sure alpha is set

        tempW1 = hsToLE16( srcData[ 0 ] );
        tempW2 = hsToLE16( srcData[ 1 ] );

        if( tempW1 > tempW2 )
        {
            /// Four-color block--mix the other two
            colors[ 2 ] = IMixTwoThirdsRGB5551( colors[ 0 ], colors[ 1 ] ) | 0x0001;
            colors[ 3 ] = IMixTwoThirdsRGB5551( colors[ 1 ], colors[ 0 ] ) | 0x0001;
        }
        else
        {
            /// Three-color block and transparent
            colors[ 2 ] = IMixEqualRGB5551( colors[ 0 ], colors[ 1 ] ) | 0x0001;
            colors[ 3 ] = 0;
        }

        bitSource = hsToLE16( srcData[ 2 ] );
        bitSource2 = hsToLE16( srcData[ 3 ] );

        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = colors[ bitSource & 0x03 ];
            bitSource >>= 2;
            destBlock[ j + 8 ] = colors[ bitSource2 & 0x03 ];
            bitSource2 >>= 2;

            destBlock[ j ] = hsToLE16( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE16( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr16( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize;
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT1To32 ////////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT1 compression. DXT1 is a simple on/off
//  or all-on alpha 'compression'. Output is a 32-bit ARGB 8888 bitmap.
//
//  7.31.2000 - M.Burrack - Created, based on old code (uncredited)

void    hsDXTSoftwareCodec::IUncompressMipmapDXT1To32( plMipmap *destBMap, 
                                                   plMipmap *srcBMap )
{
    uint16_t      *srcData, tempW1, tempW2;
    uint32_t      *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      bitSource, bitSource2, x, y, bMapStride;
    uint32_t      colors[ 4 ];
    int32_t       numBlocks, i, j;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr32's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr32( 0, 1 ) - destBMap->GetAddr32( 0, 0 ) );
    x = y = 0;


    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Decompress color data block
        colors[ 0 ] = IRGB16To32Bit( srcData[ 0 ] ) | 0xff000000;
        colors[ 1 ] = IRGB16To32Bit( srcData[ 1 ] ) | 0xff000000;

        tempW1 = hsToLE16( srcData[ 0 ] );
        tempW2 = hsToLE16( srcData[ 1 ] );

        if( tempW1 > tempW2 )
        {
            /// Four-color block--mix the other two
            colors[ 2 ] = IMixTwoThirdsRGB32( colors[ 0 ], colors[ 1 ] ) | 0xff000000;
            colors[ 3 ] = IMixTwoThirdsRGB32( colors[ 1 ], colors[ 0 ] ) | 0xff000000;
        }
        else
        {
            /// Three-color block and transparent
            colors[ 2 ] = IMixEqualRGB32( colors[ 0 ], colors[ 1 ] ) | 0xff000000;
            colors[ 3 ] = 0;
        }

        bitSource = hsToLE16( srcData[ 2 ] );
        bitSource2 = hsToLE16( srcData[ 3 ] );

        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = colors[ bitSource & 0x03 ];
            bitSource >>= 2;
            destBlock[ j + 8 ] = colors[ bitSource2 & 0x03 ];
            bitSource2 >>= 2;

            destBlock[ j ] = hsToLE32( destBlock[ j ] );
            destBlock[ j + 8 ] = hsToLE32( destBlock[ j + 8 ] );
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr32( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize;
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IUncompressMipmapDXT1ToInten /////////////////////////////////////////////
//
//  UncompressBitmap internal call for DXT1 compression. DXT1 is a simple on/off
//  or all-on alpha 'compression'. Output is an 8-bit intensity bitmap, 
//  constructed from the blue-color channel of the DXT1 output.

void    hsDXTSoftwareCodec::IUncompressMipmapDXT1ToInten( plMipmap *destBMap, 
                                                   plMipmap *srcBMap )
{
    uint16_t      *srcData, tempW1, tempW2;
    uint8_t       *destData, destBlock[ 16 ];
    uint32_t      blockSize;
    uint32_t      bitSource, bitSource2, x, y, bMapStride;
    uint8_t       colors[ 4 ];
    int32_t       numBlocks, i, j;


    /// Setup some nifty stuff
    hsAssert( ( srcBMap->GetCurrWidth() & 3 ) == 0, "Bitmap width must be multiple of 4" );
    hsAssert( ( srcBMap->GetCurrHeight() & 3 ) == 0, "Bitmap height must be multiple of 4" );
    numBlocks = ( srcBMap->GetCurrWidth() * srcBMap->GetCurrHeight() ) >> 4;

    blockSize = srcBMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)srcBMap->GetCurrLevelPtr();
    // Note our trick here to make sure nothing breaks if GetAddr8's 
    // formula changes
    bMapStride = (uint32_t)( destBMap->GetAddr8( 0, 1 ) - destBMap->GetAddr8( 0, 0 ) );
    x = y = 0;

    /// Loop through the # of blocks (width*height / 16-pixel-blocks)
    for( i = 0; i < numBlocks; i++ )
    {
        /// Decompress color data block (cast will automatically truncate to blue)
        colors[ 0 ] = (uint8_t)IRGB16To32Bit( srcData[ 0 ] );
        colors[ 1 ] = (uint8_t)IRGB16To32Bit( srcData[ 1 ] );

        tempW1 = hsToLE16( srcData[ 0 ] );
        tempW2 = hsToLE16( srcData[ 1 ] );

        if( tempW1 > tempW2 )
        {
            /// Four-color block--mix the other two
            colors[ 2 ] = IMixTwoThirdsInten( colors[ 0 ], colors[ 1 ] );
            colors[ 3 ] = IMixTwoThirdsInten( colors[ 1 ], colors[ 0 ] );
        }
        else
        {
            /// Three-color block and transparent
            colors[ 2 ] = IMixEqualInten( colors[ 0 ], colors[ 1 ] );
            colors[ 3 ] = 0;
        }

        bitSource = hsToLE16( srcData[ 2 ] );
        bitSource2 = hsToLE16( srcData[ 3 ] );

        for( j = 0; j < 8; j++ )
        {
            destBlock[ j ] = colors[ bitSource & 0x03 ];
            bitSource >>= 2;
            destBlock[ j + 8 ] = colors[ bitSource2 & 0x03 ];
            bitSource2 >>= 2;
        }
        
        /// Now copy the block to the destination bitmap
        /// (Trust me, this is actually *faster* than memcpy for some reason
        destData = destBMap->GetAddr8( x, y );
        destData[ 0 ] = destBlock[ 0 ];
        destData[ 1 ] = destBlock[ 1 ];
        destData[ 2 ] = destBlock[ 2 ];
        destData[ 3 ] = destBlock[ 3 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 4 ];
        destData[ 1 ] = destBlock[ 5 ];
        destData[ 2 ] = destBlock[ 6 ];
        destData[ 3 ] = destBlock[ 7 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 8 ];
        destData[ 1 ] = destBlock[ 9 ];
        destData[ 2 ] = destBlock[ 10 ];
        destData[ 3 ] = destBlock[ 11 ];
        destData += bMapStride;
        destData[ 0 ] = destBlock[ 12 ];
        destData[ 1 ] = destBlock[ 13 ];
        destData[ 2 ] = destBlock[ 14 ];
        destData[ 3 ] = destBlock[ 15 ];

        /// Increment and loop!
        srcData += blockSize;
        x += 4;
        if( x == srcBMap->GetCurrWidth() )
        {
            x = 0;
            y += 4;
        }
    }
}

//// IRGB16To32Bit ////////////////////////////////////////////////////////////
//
//  Converts a RGB565 16-bit color into a RGB888 32-bit color. Alpha (upper 8
//  bits) is 0. Will be optimized LATER.
//

uint32_t  hsDXTSoftwareCodec::IRGB16To32Bit( uint16_t color )
{
    uint32_t      r, g, b;

    color = hsToLE16(color);
    
    b = ( color & 31 ) << 3;
    color >>= 5;

    g = ( color & 63 ) << ( 2 + 8 );
    color >>= 6;
    
    r = ( color & 31 ) << ( 3 + 16 );

    return( r + g + b );
}

//// IRGB565To4444 ////////////////////////////////////////////////////////////
//
//  Converts a RGB565 16-bit color into a RGB4444 16-bit color. Alpha (upper 8
//  bits) is 0.
//

uint16_t  hsDXTSoftwareCodec::IRGB565To4444( uint16_t color )
{
    uint16_t      r, g, b;

    color = hsToLE16( color );
    
    b = ( color & 31 ) >> 1;
    color >>= 5;

    g = ( color & 63 ) >> 2;
    color >>= 6;
    g <<= 4;
    
    r = ( color & 31 ) >> 1;
    r <<= 8;

    return( r + g + b );
}

//// IRGB565To4444Rev /////////////////////////////////////////////////////////
//
//  Converts a RGB565 16-bit color into a RGB4444 16-bit color. Alpha (upper 8
//  bits) is 0. This is the OpenGL-friendly version (B and R are swapped)
//

uint16_t  hsDXTSoftwareCodec::IRGB565To4444Rev( uint16_t color )
{
    uint16_t      r, g, b;

    color = hsToLE16( color );
    
    r = ( color & 31 ) >> 1;
    color >>= 5;
    r <<= 8;

    g = ( color & 63 ) >> 2;
    color >>= 6;
    g <<= 4;
    
    b = ( color & 31 ) >> 1;

    return( r + g + b );
}

//// IRGB565To1555 ////////////////////////////////////////////////////////////
//
//  Converts a RGB565 16-bit color into a RGB1555 16-bit color. Alpha (upper 1
//  bit) is 0.
//

uint16_t  hsDXTSoftwareCodec::IRGB565To1555( uint16_t color )
{
    uint16_t      r, g, b;

    color = hsToLE16( color );
    
    b = ( color & 31 );
    color >>= 5;

    g = ( color & 63 ) >> 1;
    g <<= 5;
    color >>= 6;
    
    r = ( color & 31 );
    r <<= 10;

    return( r + g + b );
}

//// IRGB565To5551 ////////////////////////////////////////////////////////////
//
//  Converts a RGB565 16-bit color into a RGB5551 16-bit color. Alpha (lowest 1
//  bit) is 0.
//

uint16_t  hsDXTSoftwareCodec::IRGB565To5551( uint16_t color )
{
    uint16_t      rg, b;

    color = hsToLE16( color );
    
    rg = color & 0xffc0;        /// Masks off red and green
    b = ( color & 31 );
    b <<= 1;

    return( rg | b );
}

//// IMixTwoThirdsRGB32 ///////////////////////////////////////////////////////
//
//  Returns a 32-bit RGB888 value resulting from the mixing of 2/3rds of the
//  first parameter and 1/3rd of the second. Will be optimized LATER.
//

uint32_t  hsDXTSoftwareCodec::IMixTwoThirdsRGB32( uint32_t twoThirds, uint32_t oneThird )
{
    uint32_t  r, g, b;


    r = ( ( twoThirds & 0x00ff0000 ) + ( twoThirds & 0x00ff0000 )
        + ( oneThird & 0x00ff0000 ) ) / 3;
    r &= 0x00ff0000;

    g = ( ( twoThirds & 0x0000ff00 ) + ( twoThirds & 0x0000ff00 )
        + ( oneThird & 0x0000ff00 ) ) / 3;
    g &= 0x0000ff00;

    b = ( ( twoThirds & 0x000000ff ) + ( twoThirds & 0x000000ff )
        + ( oneThird & 0x000000ff ) ) / 3;
    b &= 0x000000ff;

    return( r + g + b );
}

//// IMixTwoThirdsRGB1555 /////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB1555 value resulting from the mixing of 2/3rds of the
//  first parameter and 1/3rd of the second. Alpha is ignored.
//

uint16_t  hsDXTSoftwareCodec::IMixTwoThirdsRGB1555( uint16_t twoThirds, uint16_t oneThird )
{
    uint16_t  r, g, b;


    r = ( ( twoThirds & 0x7c00 ) + ( twoThirds & 0x7c00 )
        + ( oneThird & 0x7c00 ) ) / 3;
    r &= 0x7c00;

    g = ( ( twoThirds & 0x03e0 ) + ( twoThirds & 0x03e0 )
        + ( oneThird & 0x03e0 ) ) / 3;
    g &= 0x03e0;

    b = ( ( twoThirds & 0x001f ) + ( twoThirds & 0x001f )
        + ( oneThird & 0x001f ) ) / 3;
    b &= 0x001f;

    return( r + g + b );
}

//// IMixTwoThirdsRGB5551 /////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB5551 value resulting from the mixing of 2/3rds of the
//  first parameter and 1/3rd of the second. Alpha is ignored.
//

uint16_t  hsDXTSoftwareCodec::IMixTwoThirdsRGB5551( uint16_t twoThirds, uint16_t oneThird )
{
    uint16_t  r, g, b;


    r = ( ( twoThirds & 0xf800 ) + ( twoThirds & 0xf800 )
        + ( oneThird & 0xf800 ) ) / 3;
    r &= 0xf800;

    g = ( ( twoThirds & 0x07c0 ) + ( twoThirds & 0x07c0 )
        + ( oneThird & 0x07c0 ) ) / 3;
    g &= 0x07c0;

    b = ( ( twoThirds & 0x003e ) + ( twoThirds & 0x003e )
        + ( oneThird & 0x003e ) ) / 3;
    b &= 0x003e;

    return( r + g + b );
}

//// IMixTwoThirdsRGB4444 /////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB4444 value resulting from the mixing of 2/3rds of the
//  first parameter and 1/3rd of the second. Alpha is ignored.
//

uint16_t  hsDXTSoftwareCodec::IMixTwoThirdsRGB4444( uint16_t twoThirds, uint16_t oneThird )
{
    uint16_t  r, g, b;


    r = ( ( twoThirds & 0x0f00 ) + ( twoThirds & 0x0f00 )
        + ( oneThird & 0x0f00 ) ) / 3;
    r &= 0x0f00;

    g = ( ( twoThirds & 0x00f0 ) + ( twoThirds & 0x00f0 )
        + ( oneThird & 0x00f0 ) ) / 3;
    g &= 0x00f0;

    b = ( ( twoThirds & 0x000f ) + ( twoThirds & 0x000f )
        + ( oneThird & 0x000f ) ) / 3;
    b &= 0x000f;

    return( r + g + b );
}

//// IMixTwoThirdsInten ///////////////////////////////////////////////////////
//
//  Returns an 8-bit intensity value resulting from the mixing of 2/3rds of the
//  first parameter and 1/3rd of the second.
//

uint8_t   hsDXTSoftwareCodec::IMixTwoThirdsInten( uint8_t twoThirds, uint8_t oneThird )
{
    return( ( twoThirds + twoThirds + oneThird ) / 3 );
}

//// IMixEqualRGB32 ///////////////////////////////////////////////////////////
//
//  Returns a 32-bit RGB888 value resulting from the mixing of equal parts of
//  the two colors given.
//

uint32_t  hsDXTSoftwareCodec::IMixEqualRGB32( uint32_t color1, uint32_t color2 )
{
    uint32_t  r, g, b;


    r = ( ( color1 & 0x00ff0000 ) + ( color2 & 0x00ff0000 ) ) >> 1;
    r &= 0x00ff0000;

    g = ( ( color1 & 0x0000ff00 ) + ( color2 & 0x0000ff00 ) ) >> 1;
    g &= 0x0000ff00;

    b = ( ( color1 & 0x000000ff ) + ( color2 & 0x000000ff ) ) >> 1;
    b &= 0x000000ff;

    return( r + g + b );
}

//// IMixEqualRGB1555 /////////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB1555 value resulting from the mixing of equal parts of
//  the two colors given.
//

uint16_t  hsDXTSoftwareCodec::IMixEqualRGB1555( uint16_t color1, uint16_t color2 )
{
    uint16_t  r, g, b;


    r = ( ( color1 & 0x7c00 ) + ( color2 & 0x7c00 ) ) >> 1;
    r &= 0x7c00;

    g = ( ( color1 & 0x03e0 ) + ( color2 & 0x03e0 ) ) >> 1;
    g &= 0x03e0;

    b = ( ( color1 & 0x001f ) + ( color2 & 0x001f ) ) >> 1;
    b &= 0x001f;

    return( r + g + b );
}

//// IMixEqualRGB5551 /////////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB5551 value resulting from the mixing of equal parts of
//  the two colors given.
//

uint16_t  hsDXTSoftwareCodec::IMixEqualRGB5551( uint16_t color1, uint16_t color2 )
{
    uint16_t  r, g, b;


    r = ( ( color1 & 0xf800 ) + ( color2 & 0xf800 ) ) >> 1;
    r &= 0xf800;

    g = ( ( color1 & 0x07c0 ) + ( color2 & 0x07c0 ) ) >> 1;
    g &= 0x07c0;

    b = ( ( color1 & 0x003e ) + ( color2 & 0x003e ) ) >> 1;
    b &= 0x003e;

    return( r + g + b );
}

//// IMixEqualRGB4444 /////////////////////////////////////////////////////////
//
//  Returns a 16-bit RGB4444 value resulting from the mixing of equal parts of
//  the two colors given.
//

uint16_t  hsDXTSoftwareCodec::IMixEqualRGB4444( uint16_t color1, uint16_t color2 )
{
    uint16_t  r, g, b;


    r = ( ( color1 & 0x0f00 ) + ( color2 & 0x0f00 ) ) >> 1;
    r &= 0x0f00;

    g = ( ( color1 & 0x00f0 ) + ( color2 & 0x00f0 ) ) >> 1;
    g &= 0x00f0;

    b = ( ( color1 & 0x000f ) + ( color2 & 0x000f ) ) >> 1;
    b &= 0x000f;

    return( r + g + b );
}

//// IMixEqualInten ///////////////////////////////////////////////////////////
//
//  Returns an 8-bit intensity value resulting from the mixing of equal parts of
//  the two colors given.
//

uint8_t   hsDXTSoftwareCodec::IMixEqualInten( uint8_t color1, uint8_t color2 )
{
    return( ( color1 + color2 ) >> 1 );
}



void hsDXTSoftwareCodec::CompressMipmapLevel( plMipmap *uncompressed, plMipmap *compressed )
{
    uint32_t *compressedImage = (uint32_t *)compressed->GetCurrLevelPtr();
    int32_t x, y;
    int32_t xMax = uncompressed->GetCurrWidth() >> 2;
    int32_t yMax = uncompressed->GetCurrHeight() >> 2;
    for (x = 0; x < xMax; ++x)
    {
        for (y = 0; y < yMax; ++y)
        {
            uint8_t maxAlpha = 0;
            uint8_t minAlpha = 255;
            uint8_t oldMaxAlpha = 0;
            uint8_t oldMinAlpha = 255;
            uint8_t alpha[8];
            int32_t maxDistance = 0;
            hsRGBAColor32 color[4];
            bool hasTransparency = false;

            int32_t xx, yy;
            for (xx = 0; xx < 4; ++xx)
            {
                for (yy = 0; yy < 4; ++yy)
                {
                    hsRGBAColor32* pixel = (hsRGBAColor32*)uncompressed->GetAddr32(4 * x + xx, 4 * y + yy);
                    uint8_t pixelAlpha = pixel->a;
                    if (pixelAlpha != 255)
                    {
                        hasTransparency = true;
                    }

                    if (compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5)
                    {
                        if (pixelAlpha > maxAlpha)
                        {
                            maxAlpha = pixelAlpha;
                        }
                        
                        if ((pixelAlpha > oldMaxAlpha) && (pixelAlpha < 255))
                        {
                            oldMaxAlpha = pixelAlpha;
                        }

                        if (pixelAlpha < minAlpha)
                        {
                            minAlpha = pixelAlpha;
                        }

                        if ((pixelAlpha < oldMinAlpha) && (pixelAlpha > 0))
                        {
                            oldMinAlpha = minAlpha;
                        }
                    }
                    
                    int32_t xx2, yy2;
                    for (xx2 = 0; xx2 < 4; ++xx2)
                    {
                        for (yy2 = 0; yy2 < 4; ++yy2)
                        {
                            hsRGBAColor32* pixel1 = (hsRGBAColor32*)uncompressed->GetAddr32(4 * x + xx, 4 * y + yy);
                            hsRGBAColor32* pixel2 = (hsRGBAColor32*)uncompressed->GetAddr32(4 * x + xx2, 4 * y + yy2);
                            
                            int32_t distance = ColorDistanceARGBSquared(*pixel1, *pixel2);
                            if (distance >= maxDistance)
                            {
                                maxDistance = distance;
                                color[0] = *pixel1;
                                color[1] = *pixel2;
                            }
                        } // for yy2
                    } // for xx2
                } // for yy
            } // for xx
            
            if (oldMinAlpha == 255)
            {
                hsAssert(oldMaxAlpha == 0, "Weirdness in oldMaxAlpha hsDXTSoftwareCodec::CompressBitmap.");
                oldMinAlpha = 0;
                oldMaxAlpha = 255;
            }

            if (compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5)
            {
                if ((maxAlpha == 255) && (minAlpha == 0))
                {
                    hsAssert(oldMinAlpha <= oldMaxAlpha, "Min > Max in hsDXTSoftwareCodec::CompressBitmap 1.");
                    alpha[0] = oldMinAlpha;
                    alpha[1] = oldMaxAlpha;
                    alpha[2] = (4 * alpha[0] + alpha[1]) / 5;      // Bit code 010
                    alpha[3] = (3 * alpha[0] + 2 * alpha[1]) / 5;  // Bit code 011    
                    alpha[4] = (2 * alpha[0] + 3 * alpha[1]) / 5;  // Bit code 100    
                    alpha[5] = (alpha[0] + 4 * alpha[1]) / 5;      // Bit code 101
                    alpha[6] = 0;                                // Bit code 110
                    alpha[7] = 255;                              // Bit code 111
                }
                else if (maxAlpha == minAlpha)
                {
                    alpha[0] = minAlpha;
                    alpha[1] = maxAlpha;
                    alpha[2] = (4 * alpha[0] + alpha[1]) / 5;      // Bit code 010
                    alpha[3] = (3 * alpha[0] + 2 * alpha[1]) / 5;  // Bit code 011    
                    alpha[4] = (2 * alpha[0] + 3 * alpha[1]) / 5;  // Bit code 100    
                    alpha[5] = (alpha[0] + 4 * alpha[1]) / 5;      // Bit code 101
                    alpha[6] = 0;                                // Bit code 110
                    alpha[7] = 255;                              // Bit code 111
                }
                else
                {
                    hsAssert(minAlpha < maxAlpha, "Min => Max in hsDXTSoftwareCodec::CompressBitmap 3.");
                    alpha[0] = maxAlpha;
                    alpha[1] = minAlpha;
                    alpha[2] = (6 * alpha[0] + alpha[1]) / 7;      // bit code 010
                    alpha[3] = (5 * alpha[0] + 2 * alpha[1]) / 7;  // Bit code 011    
                    alpha[4] = (4 * alpha[0] + 3 * alpha[1]) / 7;  // Bit code 100    
                    alpha[5] = (3 * alpha[0] + 4 * alpha[1]) / 7;  // Bit code 101
                    alpha[6] = (2 * alpha[0] + 5 * alpha[1]) / 7;  // Bit code 110    
                    alpha[7] = (alpha[0] + 6 * alpha[1]) / 7;      // Bit code 111
                }
            }
            
            uint32_t encoding;
            uint16_t shortColor[2];
            shortColor[0] = Color32To16(color[0]);
            shortColor[1] = Color32To16(color[1]);
            if ((shortColor[0] == shortColor[1]) ||
                ((compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1) &&
                hasTransparency))
            {
                encoding = kThreeColorEncoding;

                if (shortColor[0] > shortColor[1])
                {
                    uint16_t temp = shortColor[1];
                    shortColor[1] = shortColor[0];
                    shortColor[0] = temp;
                    
                    hsRGBAColor32 temp32 = color[1];
                    color[1] = color[0];
                    color[0] = temp32;
                }

                color[2] = BlendColors32(1, color[0], 1, color[1]);

                hsRGBAColor32 black;
                black.Set(0, 0, 0, 0);

                color[3] = black;
            }
            else
            {
                encoding = kFourColorEncoding;

                if (shortColor[0] < shortColor[1])
                {
                    uint16_t temp = shortColor[1];
                    shortColor[1] = shortColor[0];
                    shortColor[0] = temp;
                    
                    hsRGBAColor32 temp32 = color[1];
                    color[1] = color[0];
                    color[0] = temp32;
                }

                color[2] = BlendColors32(2, color[0], 1, color[1]);
                color[3] = BlendColors32(1, color[0], 2, color[1]);
            }
            
            // Process each pixel in block
            uint32_t blockSize = compressed->fDirectXInfo.fBlockSize;
            uint32_t *block = &compressedImage[(x + xMax * y) * (blockSize >> 2)];
            uint8_t *byteBlock = (uint8_t *)block;
            uint8_t *alphaBlock = nullptr;
            uint16_t *colorBlock = nullptr;
            if (compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5)
            {
                alphaBlock = byteBlock;
                colorBlock = (uint16_t *)(byteBlock + 8);
                alphaBlock[0] = 0;
                alphaBlock[1] = 0;
                alphaBlock[2] = 0;
                alphaBlock[3] = 0;
                alphaBlock[4] = 0;
                alphaBlock[5] = 0;
                alphaBlock[6] = 0;
                alphaBlock[7] = 0;
            }
            else if (compressed->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT1)
            {
                alphaBlock = nullptr;
                colorBlock = (uint16_t *)(byteBlock);
            }
            else
            {
                hsAssert(false, "Unrecognized compression scheme.");
            }
            
            colorBlock[0] = 0;
            colorBlock[1] = 0;
            colorBlock[2] = 0;
            colorBlock[3] = 0;
            for (xx = 0; xx < 4; ++xx)
            {
                for (yy = 0; yy < 4; ++yy)
                {
                    hsRGBAColor32* pixel = (hsRGBAColor32*)uncompressed->GetAddr32(4 * x + xx, 4 * y + yy);
                    uint8_t pixelAlpha = pixel->a;
                    if (alphaBlock)
                    {
                        uint32_t alphaIndex = 0;
                        uint32_t alphaDistance = abs(pixelAlpha - alpha[0]);
                        
                        int32_t i;
                        for (i = 1; i < 8; i++)
                        {
                            uint32_t distance = abs(pixelAlpha - alpha[i]);
                            if (distance < alphaDistance)
                            {
                                alphaIndex = i;
                                alphaDistance = distance;
                            }
                        }
                        
                        if (yy < 2)
                        {
                            uint32_t alphaShift = 3 * (4 * yy + xx);
                            uint32_t threeAlphaBytes = alphaIndex << alphaShift;
                            alphaBlock[2] |= (threeAlphaBytes & 0xff);
                            alphaBlock[3] |= ((threeAlphaBytes >> 8) & 0xff);
                            alphaBlock[4] |= ((threeAlphaBytes >> 16) & 0xff);
                        }
                        else
                        {
                            uint32_t alphaShift = 3 * (4 * (yy - 2) + xx);
                            uint32_t threeAlphaBytes = alphaIndex << alphaShift;
                            alphaBlock[5] |= (threeAlphaBytes & 0xff);
                            alphaBlock[6] |= ((threeAlphaBytes >> 8) & 0xff);
                            alphaBlock[7] |= ((threeAlphaBytes >> 16) & 0xff);
                        }
                    }
                    
                    uint32_t colorIndex = 0;
                    uint32_t colorDistance = ColorDistanceARGBSquared(*pixel, color[0]);
                    
                    if ((encoding == kThreeColorEncoding) &&
                        (pixelAlpha == 0))
                    {
                        colorIndex = 3;
                    }
                    else
                    {
                        int32_t i;
                        int32_t colorMax = (encoding == kThreeColorEncoding) ? 3 : 4;
                        for (i = 1; i < colorMax; i++)
                        {
                            uint32_t distance = ColorDistanceARGBSquared(*pixel, color[i]);
                            if (distance < colorDistance)
                            {
                                colorIndex = i;
                                colorDistance = distance;
                            }
                        }
                    }

                    if (yy < 2)
                    {
                        uint32_t colorShift = 2 * (4 * yy + xx);
                        uint16_t colorWord = (uint16_t)(colorIndex << colorShift);
                        colorBlock[2] |= colorWord;
                    }
                    else
                    {
                        uint32_t colorShift = 2 * (4 * (yy - 2) + xx);
                        uint16_t colorWord = (uint16_t)(colorIndex << colorShift);
                        colorBlock[3] |= colorWord;
                    }
                } // for yy
            } // for xx
            
            if (alphaBlock)
            {
                alphaBlock[0] = alpha[0];
                alphaBlock[1] = alpha[1];
            }
            
            colorBlock[0] = shortColor[0];
            colorBlock[1] = shortColor[1];
        } // for y
    } // for x
}

uint16_t hsDXTSoftwareCodec::BlendColors16(uint16_t weight1, uint16_t color1, uint16_t weight2, uint16_t color2)
{
    uint16_t r1, r2, g1, g2, b1, b2;
    uint16_t r, g, b;

    r1 = (color1 >> 8) & 0xf8;
    g1 = (color1 >> 3) & 0xfc;
    b1 = (color1 << 3) & 0xf8;
    
    r2 = (color2 >> 8) & 0xf8;
    g2 = (color2 >> 3) & 0xfc;
    b2 = (color2 << 3) & 0xf8;
    
    r = ((uint16_t)r1 * weight1 + (uint16_t)r2 * weight2)/(weight1 + weight2);
    g = ((uint16_t)g1 * weight1 + (uint16_t)g2 * weight2)/(weight1 + weight2);
    b = ((uint16_t)b1 * weight1 + (uint16_t)b2 * weight2)/(weight1 + weight2);

    return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
}


hsRGBAColor32 hsDXTSoftwareCodec::BlendColors32(uint32_t weight1, hsRGBAColor32 color1, 
                                         uint32_t weight2, hsRGBAColor32 color2)
{
    hsRGBAColor32 result;

    result.r = static_cast<uint8_t>((color1.r * weight1 + color2.r * weight2)/(weight1 + weight2));
    result.g = static_cast<uint8_t>((color1.g * weight1 + color2.g * weight2)/(weight1 + weight2));
    result.b = static_cast<uint8_t>((color1.b * weight1 + color2.b * weight2)/(weight1 + weight2));

    return result;
}


int32_t hsDXTSoftwareCodec::ColorDistanceARGBSquared(hsRGBAColor32 color1, hsRGBAColor32 color2)
{
    int32_t r1, g1, b1;
    int32_t r2, g2, b2;

    r1 = color1.r;
    r2 = color2.r;
    g1 = color1.g;
    g2 = color2.g;
    b1 = color1.b;
    b2 = color2.b;

    return (r1 - r2) * (r1 - r2) + (g1 - g2) * (g1 - g2) + (b1 - b2) * (b1 - b2);
}

uint16_t hsDXTSoftwareCodec::Color32To16(hsRGBAColor32 color)
{
    uint8_t r = (uint8_t)(color.r & 0xf8);
    uint8_t g = (uint8_t)(color.g & 0xfc);
    uint8_t b = (uint8_t)(color.b & 0xf8);

    return (r << 8) | (g << 3) | (b >> 3);
}

bool hsDXTSoftwareCodec::Register()
{
    return hsCodecManager::Instance().Register(&(Instance()), plMipmap::kDirectXCompression, 100);
}

//// ICalcCompressedFormat ////////////////////////////////////////////////////
//  Determine the DXT compression format based on a bitmap.

uint8_t   hsDXTSoftwareCodec::ICalcCompressedFormat( plMipmap *bMap )
{
    if( bMap->GetFlags() & plMipmap::kAlphaChannelFlag )
        return plMipmap::DirectXInfo::kDXT5;

    return plMipmap::DirectXInfo::kDXT1;
}


//// ColorizeCompBitmap ///////////////////////////////////////////////////////
//  Colorizes a compressed bitmap according to the color mask given.

bool hsDXTSoftwareCodec::ColorizeCompMipmap( plMipmap *bMap, const uint8_t *colorMask )
{
    uint32_t  numBlocks, blockSize;
    uint16_t  *srcData, color1, color2, gray, grayDiv2, i;
    uint8_t   compMasks[ 3 ][ 2 ] = { { 0, 0 }, { 0, 0xff }, { 0xff, 0 } };


    /// Sanity checks
    hsAssert(bMap != nullptr, "Nil bitmap passed to ColorizeCompMipmap()");
    hsAssert(colorMask != nullptr, "Nil color mask passed to ColorizeCompMipmap()");
    hsAssert( bMap->IsCompressed(), "Trying to colorize uncompressed bitmap" );


    /// Calc some stuff
    numBlocks = ( bMap->GetCurrWidth() * bMap->GetCurrHeight() ) >> 4;

    blockSize = bMap->fDirectXInfo.fBlockSize >> 1; // In 16-bit words
    srcData = (uint16_t *)bMap->GetCurrLevelPtr();
    
    // If we're DXT5, we'll artificially advance srcData so it points to the start
    // of the first *color* block, not the first compressed block
    if( bMap->fDirectXInfo.fCompressionType == plMipmap::DirectXInfo::kDXT5 )
        srcData += 4;       // Alpha was 4 16-bit words worth


    /// Loop!
    for( ; numBlocks > 0; numBlocks-- )
    {
        /// Get the two colors to colorize (our decompression scheme will do the rest...
        /// handy, eh? :)
        color1 = hsToLE16( srcData[ 0 ] );
        color2 = hsToLE16( srcData[ 1 ] );

        /// Now colorize using our age-old formula (see hsGMipmap::ColorLevel for details)
        gray = ( ( color1 >> 11 ) & 0x1f ) + ( ( color1 >> 6 ) & 0x1f ) + ( color1 & 0x1f );
        gray /= 3;
        gray = 0x1f - ( ( 0x1f - gray ) >> 1 );     // Lighten it 50%
        grayDiv2 = gray >> 1;

        color1 = ( ( gray & compMasks[ colorMask[ 0 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 0 ] ][ 1 ] ) ) << 11;
        color1 |= ( ( gray & compMasks[ colorMask[ 1 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 1 ] ][ 1 ] ) ) << 6;
        color1 |= ( ( gray & compMasks[ colorMask[ 2 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 2 ] ][ 1 ] ) );

        gray = ( ( color2 >> 11 ) & 0x1f ) + ( ( color2 >> 6 ) & 0x1f ) + ( color2 & 0x1f );
        gray /= 3;
        gray = 0x1f - ( ( 0x1f - gray ) >> 1 );     // Lighten it 50%
        grayDiv2 = gray >> 1;

        color2 = ( ( gray & compMasks[ colorMask[ 0 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 0 ] ][ 1 ] ) ) << 11;
        color2 |= ( ( gray & compMasks[ colorMask[ 1 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 1 ] ][ 1 ] ) ) << 6;
        color2 |= ( ( gray & compMasks[ colorMask[ 2 ] ][ 0 ] ) |
                ( grayDiv2 & compMasks[ colorMask[ 2 ] ][ 1 ] ) );

        /// IMPORTANT: Check to make sure we're preserving the block type
        if( srcData[ 0 ] > srcData[ 1 ] && color1 <= color2 )
        {
            /// NOPE! We better flip the colors and flip all the color refs
            /// in this block, to preserve block type
            if( color1 == color2 )
                color1++;       // This is a hack--our rounding may cause this,
                                // in which case we need to make SURE c1 > c2
            else
                SWAPVARS( color1, color2, i );

            srcData[ 2 ] ^= 0x5555;
            srcData[ 3 ] ^= 0x5555;
        }
        else if( srcData[ 0 ] <= srcData[ 1 ] && color1 > color2 )
        {
            SWAPVARS( color1, color2, i );

            /// 1/2 block w/ alpha -- switch only first two
            /// (watch carefully :)
            srcData[ 2 ] ^= ( ( ~( srcData[ i ] >> 1 ) ) & 0x5555 );
            srcData[ 3 ] ^= ( ( ~( srcData[ i ] >> 1 ) ) & 0x5555 );

            /// Spoiler for above: we shift the uint16_t right one bit, then
            /// not the bits, then mask off the lower bits of the pairs
            /// (which of course used to be the upper bits). Now any upper
            /// bits that were 0 are now lower bits of 1, and everything 
            /// else 0's. This we then xor with the original value to 
            /// flip the lower bits of those pairs whose upper bits are 0.
            /// Nifty, eh?
        }

        /// Write back and go!
        srcData[ 0 ] = hsToLE16( color1 );
        srcData[ 1 ] = hsToLE16( color2 );

        srcData += blockSize;
    }

    return true;        /// We handled this bitmap
}

