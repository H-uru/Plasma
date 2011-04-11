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
//	plDXTextureRef.h - Hardware Texture DeviceRef Definition				 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	4.25.2001 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plDXTextureRef_h
#define _plDXTextureRef_h

#include "hsMatrix44.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "plDXDeviceRef.h"


//// Definition ///////////////////////////////////////////////////////////////

class plBitmap;

class plDXTextureRef : public plDXDeviceRef
{
	public:
		enum Flags
		{
			kExternData			= 0x00000002,	// fData points to user data, don't delete
			kRenderTarget		= 0x00000004,	// Created via a render target
			kCubicMap			= 0x00000008,	// Texture is really a cubic map texture
			kPerspProjection	= 0x00000010,	// Perspective projection
			kOrthoProjection	= 0x00000020,	// Orthogonal projection
			kProjection			= kPerspProjection | kOrthoProjection,
			kOffscreenRT		= 0x00000040,	// Offscreen renderTarget. Never used as an actual texture,
												// but handy to still have it as a textureRef
			kUVWNormal			= 0x00000080	// Use the normal as the UVW src
		};

		IDirect3DBaseTexture9	*fD3DTexture;
		D3DFORMAT				fFormatType;	// Format of the D3D texture object

		UInt32		fMMLvs;			// Number of mipmap levels
		UInt32		fMaxWidth;		// Width of the highest mipmap level
		UInt32		fMaxHeight;		// Height of the highest mipmap level (no pun intended)
		UInt32		fNumPix;		// total num texels in all mip levels
		UInt32		fDataSize;		// size of fData[0..n] in bytes
		UInt32*		fLevelSizes;	// fLevelSize[i] == size in bytes of level i
		//UInt32		fCurrLOD;		// Current LOD setting for this texture

		plBitmap	*fOwner;

		void*		fData;			// for reloading

		UInt32		GetFlags( void ) { return fFlags; }
		void		SetFlags( UInt32 flag ) { fFlags = flag; }

		plDXTextureRef& Set( D3DFORMAT tp, UInt32 ml, UInt32 mw, UInt32 mh, UInt32 np, UInt32 sz, UInt32 manSize, UInt32* lSz, void* pd, hsBool ed=false, hsBool renderTarget = false );

		plDXTextureRef( D3DFORMAT tp, UInt32 ml, UInt32 mw, UInt32 mh, UInt32 np, UInt32 sz, UInt32 manSize, UInt32* lSz, void* pd, hsBool ed=false, hsBool renderTarget = false );
		virtual ~plDXTextureRef();

		void			Link( plDXTextureRef **back ) { plDXDeviceRef::Link( (plDXDeviceRef **)back ); }
		plDXTextureRef	*GetNext( void ) { return (plDXTextureRef *)fNext; }

		void	Release( void );
};

class plDXCubeTextureRef : public plDXTextureRef
{
	public:
		void	*fFaceData[ 5 ];			// First face is in the inherited fData

		plDXCubeTextureRef( D3DFORMAT tp, UInt32 ml, UInt32 mw, UInt32 mh, UInt32 np, UInt32 sz, UInt32 manSize, UInt32* lSz, void* pd, hsBool ed=false, hsBool renderTarget = false ) :
							plDXTextureRef( tp, ml, mw, mh, np, sz, manSize, lSz, pd, ed, renderTarget )
		{

		}
};

#endif // _plDXTextureRef_h
