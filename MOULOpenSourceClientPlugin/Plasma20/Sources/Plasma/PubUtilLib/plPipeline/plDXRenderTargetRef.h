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
//	plDXRenderTargetRef.h - RenderTarget DeviceRef Definitions				 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	7.19.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXRenderTargetRef_h
#define _plDXRenderTargetRef_h

#include "hsMatrix44.h"
#include "hsTemplates.h"
#include "plDXTextureRef.h"


//// Definition ///////////////////////////////////////////////////////////////

class plRenderTarget;
class plDXRenderTargetRef: public plDXTextureRef
{
	public:

		IDirect3DSurface9	*fD3DColorSurface;
		IDirect3DSurface9	*fD3DDepthSurface;

		hsBool				fReleaseDepth;

		void					Link( plDXRenderTargetRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
		plDXRenderTargetRef	*GetNext( void ) { return (plDXRenderTargetRef *)fNext; }


		plDXRenderTargetRef( D3DFORMAT tp, UInt32 ml, plRenderTarget *owner, hsBool releaseDepthOnDelete = true );
		plDXRenderTargetRef& Set( D3DFORMAT tp, UInt32 ml, plRenderTarget *owner );

		virtual void	SetOwner( plRenderTarget *targ ) { fOwner = (plBitmap *)targ; }

		void	SetTexture( IDirect3DSurface9 *surface, IDirect3DSurface9 *depth );
		void	SetTexture( IDirect3DTexture9 *surface, IDirect3DSurface9 *depth );
		void	SetTexture( IDirect3DCubeTexture9 *surface, IDirect3DSurface9 *depth );

		IDirect3DSurface9	*GetColorSurface( void ) const
		{
			if( fD3DTexture != nil )
			{
				IDirect3DSurface9* psurf;
				((IDirect3DTexture9*)fD3DTexture)->GetSurfaceLevel(0, &psurf);
				if( psurf )
					psurf->Release();
				return psurf;
			}

			return fD3DColorSurface;
		}

		virtual ~plDXRenderTargetRef();
		void	Release( void );
};


#endif // _plDXRenderTargetRef_h
