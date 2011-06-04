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
//	hsDXTDirectXCodec Class Functions										 //
//	DirectX-based codec functions											 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.8.2001 mcn - Got a much-needed Plasma 2.0/DX8 update.					 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef hsDXTDirectXCodec_inc
#define hsDXTDirectXCodec_inc

#include "hsWindows.h"
#include "hsCodec.h"

class plMipmap;
struct IDirect3DDevice8;
struct IDirectDrawSurface7;
struct IDirectDraw7;

class hsDXTDirectXCodec : public hsCodec
{
private:
	hsDXTDirectXCodec();
public:
	~hsDXTDirectXCodec();
	static hsDXTDirectXCodec& Instance();

	static void Init()	{ fRegistered = Register(); }

	plMipmap	*CreateCompressedMipmap(plMipmap *uncompressed);
	plMipmap	*CreateUncompressedMipmap(plMipmap *compressed, UInt8 bitDepth = 0);

	// Colorize a compressed mipmap
	hsBool	ColorizeCompMipmap( plMipmap *bMap, const UInt8 *colorMask );

	void		Initialize( IDirect3DDevice8 *directDraw );
	hsBool		Initialized()			{ return (fFlags & kInitialized) != 0; }

private:
	UInt32 ICompressedFormat(const plMipmap *uncompressed);
	IDirectDrawSurface7		*IMakeDirect3DSurface( UInt32 formatType, UInt32 mipMapLevels, UInt32 width, UInt32 height );
	void					IFillSurface( hsRGBAColor32* src, UInt32 mmlvs, IDirectDrawSurface7 *pddsDest );
	void					IFillFromSurface( hsRGBAColor32* dest, UInt32 mmlvs, IDirectDrawSurface7 *pddsSrc );
	void					ICopySurface( IDirectDrawSurface7 *dest, IDirectDrawSurface7 *src, Int32 mipMapLevels );
	void					CheckErrorCode(HRESULT res);
	hsBool					IInitialize();

	IDirectDraw7	*fDirectDraw;
    HINSTANCE       fDDLibraryInstance;
	UInt32			fFlags;
	
	enum
	{
		kInitialized			= 0x1,
		kExternalInit			= 0x2
	};

	static hsBool Register();
	static hsBool fRegistered;
};

#endif // hsDXTDirectXCodec_inc
