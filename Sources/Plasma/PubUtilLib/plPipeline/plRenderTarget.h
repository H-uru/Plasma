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
//	plRenderTarget.h - Header file for the plRenderTarget class   			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	5.21.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plRenderTarget_h
#define _plRenderTarget_h

#include "plPipeResReq.h"

#include "../plGImage/plBitmap.h"

#define ASSERT_ABSOLUTE		hsAssert( !fProportionalViewport, "Cannot perform this on a proportional RenderTarget" );
#define ASSERT_PROPORTIONAL	hsAssert( fProportionalViewport, "Cannot perform this on an absolute RenderTarget" );

//// Class Definition /////////////////////////////////////////////////////////

class hsGDeviceRef;
class plCubicRenderTarget;

class plRenderTarget : public plBitmap
{
	friend plCubicRenderTarget;

	protected:

		UInt16		fWidth, fHeight;

		union
		{
			struct 
			{
				UInt16		fLeft, fTop, fRight, fBottom;
			} fAbsolute;
			struct 
			{
				hsScalar	fLeft, fTop, fRight, fBottom;
			} fProportional;
		} fViewport;

		hsBool		fApplyTexQuality;
		hsBool		fProportionalViewport;
		UInt8		fZDepth, fStencilDepth;
	
		plCubicRenderTarget	*fParent;	

		virtual void SetKey(plKey k);

		virtual UInt32	Read( hsStream *s );
		virtual UInt32	Write( hsStream *s );
	public:

		CLASSNAME_REGISTER( plRenderTarget );
		GETINTERFACE_ANY( plRenderTarget, plBitmap );

		plRenderTarget()
		{
			fWidth = 0;
			fHeight = 0;
			fPixelSize = 0;
			fZDepth = 0;
			fStencilDepth = 0;
			fApplyTexQuality = false;
			fProportionalViewport = true;
			SetViewport( 0, 0, 1.f, 1.f );
			fFlags = 0;
			fParent = nil;

			plPipeResReq::Request();
		}

		plRenderTarget( UInt16 flags, UInt16 width, UInt16 height, UInt8 bitDepth, UInt8 zDepth = 0xff, UInt8 stencilDepth = 0xff )
		{
			fWidth = width;
			fHeight = height;
			fPixelSize = bitDepth;
			fZDepth = ( zDepth != 0xff ) ? zDepth : (bitDepth > 16 ? 24 : 16);
			fStencilDepth = ( stencilDepth != 0xff ) ? stencilDepth : 0;

			fFlags = flags;
			fParent = nil;

			hsAssert( fFlags & (kIsTexture|kIsOffscreen), "Cannot perform this on an on-screen RenderTarget" );
			fApplyTexQuality = false;
			fProportionalViewport = false;
			SetViewport( 0, 0, width, height );

			plPipeResReq::Request();
		}

		// Render-to-Screen constructor
		plRenderTarget( UInt16 flags, hsScalar left, hsScalar top, hsScalar right, hsScalar bottom, UInt8 bitDepth, UInt8 zDepth = 0xff, UInt8 stencilDepth = 0xff )
		{
			fWidth = 0;	// Can't really set these, at least not yet
			fHeight = 0;
			fPixelSize = bitDepth;
			fZDepth = ( zDepth != 0xff ) ? zDepth : 16;
			fStencilDepth = ( stencilDepth != 0xff ) ? stencilDepth : 0;

			fFlags = flags;
			fParent = nil;

			hsAssert( !( fFlags & (kIsTexture|kIsOffscreen) ), "Cannot perform this on an offscreen RenderTarget" );
			fApplyTexQuality = false;
			fProportionalViewport = true;
			SetViewport( left, top, right, bottom );

			plPipeResReq::Request();
		}

		virtual ~plRenderTarget() {}

		virtual void			SetViewport( UInt16 left, UInt16 top, UInt16 right, UInt16 bottom )
		{
			ASSERT_ABSOLUTE;
			fViewport.fAbsolute.fLeft = left; 
			fViewport.fAbsolute.fTop = top; 
			fViewport.fAbsolute.fRight = right; 
			fViewport.fAbsolute.fBottom = bottom;
		}

		virtual void			SetViewport( hsScalar left, hsScalar top, hsScalar right, hsScalar bottom )
		{
			ASSERT_PROPORTIONAL;
			fViewport.fProportional.fLeft = left; 
			fViewport.fProportional.fTop = top; 
			fViewport.fProportional.fRight = right; 
			fViewport.fProportional.fBottom = bottom;
		}

		UInt16	GetWidth( void ) { return fWidth; }
		UInt16	GetHeight( void ) { return fHeight; }
		UInt8	GetZDepth( void ) { return fZDepth; }
		UInt8	GetStencilDepth( void ) { return fStencilDepth; }

		UInt16		GetVPLeft( void )	{ ASSERT_ABSOLUTE; return fViewport.fAbsolute.fLeft; }
		UInt16		GetVPTop( void )	{ ASSERT_ABSOLUTE; return fViewport.fAbsolute.fTop; }
		UInt16		GetVPRight( void )	{ ASSERT_ABSOLUTE; return fViewport.fAbsolute.fRight; }
		UInt16		GetVPBottom( void ) { ASSERT_ABSOLUTE; return fViewport.fAbsolute.fBottom; }

		hsScalar	GetVPLeftProp( void )	{ ASSERT_PROPORTIONAL; return fViewport.fProportional.fLeft; }
		hsScalar	GetVPTopProp( void )	{ ASSERT_PROPORTIONAL; return fViewport.fProportional.fTop; }
		hsScalar	GetVPRightProp( void )	{ ASSERT_PROPORTIONAL; return fViewport.fProportional.fRight; }
		hsScalar	GetVPBottomProp( void )	{ ASSERT_PROPORTIONAL; return fViewport.fProportional.fBottom; }

		hsBool		ViewIsProportional( void ) const { return fProportionalViewport; }

		plCubicRenderTarget	*GetParent( void ) const { return fParent; }

		virtual UInt32	GetTotalSize( void ) const { return fWidth * fHeight * ( fPixelSize >> 3 ); }

		virtual hsBool MsgReceive(plMessage* msg);

		virtual void SetVisRegionName(char *name){} // override to set vis region names for anyone who cares
};


#endif // _plRenderTarget_h
