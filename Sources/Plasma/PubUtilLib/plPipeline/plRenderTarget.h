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
//  plRenderTarget.h - Header file for the plRenderTarget class              //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  5.21.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRenderTarget_h
#define _plRenderTarget_h

#include <string_theory/string>

#include "plPipeResReq.h"

#include "plGImage/plBitmap.h"

#define ASSERT_ABSOLUTE     hsAssert( !fProportionalViewport, "Cannot perform this on a proportional RenderTarget" );
#define ASSERT_PROPORTIONAL hsAssert( fProportionalViewport, "Cannot perform this on an absolute RenderTarget" );

//// Class Definition /////////////////////////////////////////////////////////

class hsGDeviceRef;
class plCubicRenderTarget;

class plRenderTarget : public plBitmap
{
    friend class plCubicRenderTarget;

    protected:

        uint16_t      fWidth, fHeight;

        union
        {
            struct 
            {
                uint16_t      fLeft, fTop, fRight, fBottom;
            } fAbsolute;
            struct 
            {
                float    fLeft, fTop, fRight, fBottom;
            } fProportional;
        } fViewport;

        bool        fApplyTexQuality;
        bool        fProportionalViewport;
        uint8_t     fZDepth, fStencilDepth;

        plCubicRenderTarget* fParent;

        void SetKey(plKey k) override;

        uint32_t  Read(hsStream *s) override;
        uint32_t  Write(hsStream *s) override;
    public:

        CLASSNAME_REGISTER( plRenderTarget );
        GETINTERFACE_ANY( plRenderTarget, plBitmap );

        plRenderTarget()
            : fWidth(0), fHeight(0), fZDepth(0), fStencilDepth(0), fApplyTexQuality(false),
              fProportionalViewport(true), fParent(nullptr)
        {
            fFlags = 0;
            fPixelSize = 0;
            SetViewport( 0, 0, 1.f, 1.f );
            plPipeResReq::Request();
        }

        plRenderTarget( uint16_t flags, uint16_t width, uint16_t height, uint8_t bitDepth, uint8_t zDepth = 0xff, uint8_t stencilDepth = 0xff )
        {
            fWidth = width;
            fHeight = height;
            fPixelSize = bitDepth;
            fZDepth = ( zDepth != 0xff ) ? zDepth : (bitDepth > 16 ? 24 : 16);
            fStencilDepth = ( stencilDepth != 0xff ) ? stencilDepth : 0;

            fFlags = flags;
            fParent = nullptr;

            hsAssert( fFlags & (kIsTexture|kIsOffscreen), "Cannot perform this on an on-screen RenderTarget" );
            fApplyTexQuality = false;
            fProportionalViewport = false;
            SetViewport( 0, 0, width, height );

            plPipeResReq::Request();
        }

        // Render-to-Screen constructor
        plRenderTarget( uint16_t flags, float left, float top, float right, float bottom, uint8_t bitDepth, uint8_t zDepth = 0xff, uint8_t stencilDepth = 0xff )
        {
            fWidth = 0; // Can't really set these, at least not yet
            fHeight = 0;
            fPixelSize = bitDepth;
            fZDepth = ( zDepth != 0xff ) ? zDepth : 16;
            fStencilDepth = ( stencilDepth != 0xff ) ? stencilDepth : 0;

            fFlags = flags;
            fParent = nullptr;

            hsAssert( !( fFlags & (kIsTexture|kIsOffscreen) ), "Cannot perform this on an offscreen RenderTarget" );
            fApplyTexQuality = false;
            fProportionalViewport = true;
            SetViewport( left, top, right, bottom );

            plPipeResReq::Request();
        }

        virtual ~plRenderTarget() {}

        virtual void            SetViewport( uint16_t left, uint16_t top, uint16_t right, uint16_t bottom )
        {
            ASSERT_ABSOLUTE;
            fViewport.fAbsolute.fLeft = left; 
            fViewport.fAbsolute.fTop = top; 
            fViewport.fAbsolute.fRight = right; 
            fViewport.fAbsolute.fBottom = bottom;
        }

        virtual void            SetViewport( float left, float top, float right, float bottom )
        {
            ASSERT_PROPORTIONAL;
            fViewport.fProportional.fLeft = left; 
            fViewport.fProportional.fTop = top; 
            fViewport.fProportional.fRight = right; 
            fViewport.fProportional.fBottom = bottom;
        }

        uint16_t  GetWidth() const { return fWidth; }
        uint16_t  GetHeight() const { return fHeight; }
        uint8_t   GetZDepth() { return fZDepth; }
        uint8_t   GetStencilDepth() { return fStencilDepth; }

        uint16_t      GetVPLeft()   { ASSERT_ABSOLUTE; return fViewport.fAbsolute.fLeft; }
        uint16_t      GetVPTop()    { ASSERT_ABSOLUTE; return fViewport.fAbsolute.fTop; }
        uint16_t      GetVPRight()  { ASSERT_ABSOLUTE; return fViewport.fAbsolute.fRight; }
        uint16_t      GetVPBottom() { ASSERT_ABSOLUTE; return fViewport.fAbsolute.fBottom; }

        float    GetVPLeftProp()   { ASSERT_PROPORTIONAL; return fViewport.fProportional.fLeft; }
        float    GetVPTopProp()    { ASSERT_PROPORTIONAL; return fViewport.fProportional.fTop; }
        float    GetVPRightProp()  { ASSERT_PROPORTIONAL; return fViewport.fProportional.fRight; }
        float    GetVPBottomProp() { ASSERT_PROPORTIONAL; return fViewport.fProportional.fBottom; }

        bool        ViewIsProportional() const { return fProportionalViewport; }

        plCubicRenderTarget *GetParent() const { return fParent; }

        uint32_t  GetTotalSize() const override { return fWidth * fHeight * ( fPixelSize >> 3 ); }

        bool MsgReceive(plMessage* msg) override;

        // override to set vis region names for anyone who cares
        virtual void SetVisRegionName(ST::string name) { } // NOLINT(performance-unnecessary-value-param)
};


#endif // _plRenderTarget_h
