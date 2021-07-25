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
//  plDXTextureRef.h - Hardware Texture DeviceRef Definition                 //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  4.25.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXTextureRef_h
#define _plDXTextureRef_h

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "plDXDeviceRef.h"


//// Definition ///////////////////////////////////////////////////////////////

class plBitmap;

class plDXTextureRef : public plDXDeviceRef
{
    public:
        enum Flags
        {
            kExternData         = 0x00000002,   // fData points to user data, don't delete
            kRenderTarget       = 0x00000004,   // Created via a render target
            kCubicMap           = 0x00000008,   // Texture is really a cubic map texture
            kPerspProjection    = 0x00000010,   // Perspective projection
            kOrthoProjection    = 0x00000020,   // Orthogonal projection
            kProjection         = kPerspProjection | kOrthoProjection,
            kOffscreenRT        = 0x00000040,   // Offscreen renderTarget. Never used as an actual texture,
                                                // but handy to still have it as a textureRef
            kUVWNormal          = 0x00000080,   // Use the normal as the UVW src
            kAutoGenMipmap      = 0x00000100    // DirectX should generate mip levels for us
        };

        IDirect3DBaseTexture9   *fD3DTexture;
        D3DFORMAT               fFormatType;    // Format of the D3D texture object

        uint32_t      fMMLvs;         // Number of mipmap levels
        uint32_t      fMaxWidth;      // Width of the highest mipmap level
        uint32_t      fMaxHeight;     // Height of the highest mipmap level (no pun intended)
        uint32_t      fNumPix;        // total num texels in all mip levels
        uint32_t      fDataSize;      // size of fData[0..n] in bytes
        uint32_t*     fLevelSizes;    // fLevelSize[i] == size in bytes of level i
        //uint32_t        fCurrLOD;       // Current LOD setting for this texture

        plBitmap    *fOwner;

        void*       fData;          // for reloading

        uint32_t      GetFlags() const { return fFlags; }
        void        SetFlags( uint32_t flag ) { fFlags = flag; }

        plDXTextureRef& Set( D3DFORMAT tp, uint32_t ml, uint32_t mw, uint32_t mh, uint32_t np, uint32_t sz, uint32_t manSize, uint32_t* lSz, void* pd, bool ed=false, bool renderTarget = false );

        plDXTextureRef()
            : fD3DTexture(), fFormatType(), fMMLvs(), fMaxWidth(), fMaxHeight(), fNumPix(),
              fDataSize(), fLevelSizes(), fOwner(), fData()
        { }
        plDXTextureRef( D3DFORMAT tp, uint32_t ml, uint32_t mw, uint32_t mh, uint32_t np, uint32_t sz, uint32_t manSize, uint32_t* lSz, void* pd, bool ed=false, bool renderTarget = false )
            : fD3DTexture(), fFormatType(), fMMLvs(), fMaxWidth(), fMaxHeight(), fNumPix(),
              fDataSize(), fLevelSizes(), fOwner(), fData()
        {
            Set(tp, ml, mw, mh, np, sz, manSize, lSz, pd, ed, renderTarget);
        }

        virtual ~plDXTextureRef();

        void            Link( plDXTextureRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
        plDXTextureRef  *GetNext() { return (plDXTextureRef *)fNext; }

        void    Release() override;
};

class plDXCubeTextureRef : public plDXTextureRef
{
    public:
        void    *fFaceData[ 5 ];            // First face is in the inherited fData

        plDXCubeTextureRef( D3DFORMAT tp, uint32_t ml, uint32_t mw, uint32_t mh, uint32_t np, uint32_t sz, uint32_t manSize, uint32_t* lSz, void* pd, bool ed=false, bool renderTarget = false ) :
                            plDXTextureRef( tp, ml, mw, mh, np, sz, manSize, lSz, pd, ed, renderTarget )
        {

        }
};

#endif // _plDXTextureRef_h
