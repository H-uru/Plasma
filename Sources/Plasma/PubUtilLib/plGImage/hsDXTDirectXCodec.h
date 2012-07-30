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
//  hsDXTDirectXCodec Class Functions                                        //
//  DirectX-based codec functions                                            //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  6.8.2001 mcn - Got a much-needed Plasma 2.0/DX8 update.                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsDXTDirectXCodec_inc
#define hsDXTDirectXCodec_inc

#include "hsCodec.h"

class plMipmap;
typedef struct hsColor32 hsRGBAColor32;
#if HS_BUILD_FOR_WIN32
struct IDirect3DDevice8;
struct IDirectDrawSurface7;
struct IDirectDraw7;
#endif

class hsDXTDirectXCodec : public hsCodec
{
private:
    hsDXTDirectXCodec();
public:
    ~hsDXTDirectXCodec();
    static hsDXTDirectXCodec& Instance();

    static void Init()  { fRegistered = Register(); }

    plMipmap    *CreateCompressedMipmap(plMipmap *uncompressed);
    plMipmap    *CreateUncompressedMipmap(plMipmap *compressed, uint8_t bitDepth = 0);

    // Colorize a compressed mipmap
    bool    ColorizeCompMipmap( plMipmap *bMap, const uint8_t *colorMask );

#if HS_BUILD_FOR_WIN32
    void        Initialize( IDirect3DDevice8 *directDraw );
#endif
    bool        Initialized()           { return (fFlags & kInitialized) != 0; }

#if HS_BUILD_FOR_WIN32
private:
    uint32_t ICompressedFormat(const plMipmap *uncompressed);
    IDirectDrawSurface7     *IMakeDirect3DSurface( uint32_t formatType, uint32_t mipMapLevels, uint32_t width, uint32_t height );
    void                    IFillSurface( hsRGBAColor32* src, uint32_t mmlvs, IDirectDrawSurface7 *pddsDest );
    void                    IFillFromSurface( hsRGBAColor32* dest, uint32_t mmlvs, IDirectDrawSurface7 *pddsSrc );
    void                    ICopySurface( IDirectDrawSurface7 *dest, IDirectDrawSurface7 *src, int32_t mipMapLevels );
    void                    CheckErrorCode(HRESULT res);
    bool                    IInitialize();
#endif

#if HS_BUILD_FOR_WIN32
    IDirectDraw7    *fDirectDraw;
    HINSTANCE       fDDLibraryInstance;
#else
    void*           fDirectDraw;
    void*           fDDLibraryInstance;
#endif
    uint32_t          fFlags;
    
    enum
    {
        kInitialized            = 0x1,
        kExternalInit           = 0x2
    };

    static bool Register();
    static bool fRegistered;
};

#endif // hsDXTDirectXCodec_inc
