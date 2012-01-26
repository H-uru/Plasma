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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  hsCodecManager Class Header                                              //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.7.2001 mcn - Updated for new bitmap classes.                           //
//                                                                           //
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
    ~hsCodecManager()       { }
    static hsCodecManager& Instance();

    plMipmap    *CreateCompressedMipmap( uint32_t compressionFormat, plMipmap *uncompressed );
    plMipmap    *CreateUncompressedMipmap( plMipmap *compressed, uint8_t bitDepth = 0 );
    hsBool      ColorizeCompMipmap( plMipmap *bMap, const uint8_t *colorMask );

    hsBool      Register(hsCodec *codec, uint32_t compressionFormat, float priority);

    /// Decompression flags
    enum {
        kBitDepthMask = 0x0003,
        kCompOrderMask = 0x0004
    };
    enum {  /// Bit depths
        kDontCareDepth = 0x0000,
        k16BitDepth = 0x0001,
        k32BitDepth = 0x0002
    };
    enum {  /// uint8_t orders
        kNormalCompOrder = 0x0000,      // DirectX, Glide
        kWeirdCompOrder = 0x0004        // OpenGL
    };

private:
    struct hsCodecEntry
    {
        hsCodecEntry() : fPriority(0), fCodec(nil) { }
        hsCodecEntry(float p, hsCodec *c) : fPriority(p), fCodec(c) { }

        float                fPriority;
        hsCodec                 *fCodec;
    };

    struct hsCodecList
    {
        hsCodecList() : fCompressionFormat(0) { }
        hsCodecList(uint32_t f) : fCompressionFormat(f) { }

        uint32_t                  fCompressionFormat;
        hsTArray<hsCodecEntry>  fCodecList;
    };

    hsTArray<hsCodecList>       fCodecTable;
};

#endif // hsCodecManager_inc
