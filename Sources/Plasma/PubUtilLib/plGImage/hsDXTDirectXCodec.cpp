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

#include "hsConfig.h"
#include "hsWindows.h"

#include <ddraw.h>
#include <d3d.h>

#include "hsTypes.h"
#include "hsDXTDirectXCodec.h"
#include "plMipmap.h"
#include "hsCodecManager.h"
#include "../plPipeline/hsGDDrawDllLoad.h"

namespace {
	typedef HRESULT(WINAPI * DIRECTDRAWCREATEEX)( GUID*, VOID**, REFIID, IUnknown* );
}

enum
{
	D3DTEXTURE_FMT_ARGB32_8888					= 0x00000002,		// No. 2: 8888 ARGB (32-bit) format.
	D3DTEXTURE_FMT_FOURCC_DXT1					= 0x00002000,		// No.14: DXTn FourCC (DXT1), format.
	D3DTEXTURE_FMT_FOURCC_DXT5					= 0x00020000,		// No.18: DXTn FourCC (DXT5), format.
};

hsBool hsDXTDirectXCodec::fRegistered = false;

//// Small Init Functions /////////////////////////////////////////////////////

hsDXTDirectXCodec& hsDXTDirectXCodec::Instance()
{
	static hsDXTDirectXCodec the_instance;

	return the_instance;
}

hsDXTDirectXCodec::hsDXTDirectXCodec() : fDirectDraw( nil ), fDDLibraryInstance( nil ), fFlags( 0 )
{
}

hsDXTDirectXCodec::~hsDXTDirectXCodec()
{
	if ((fFlags & kExternalInit) == 0)
	{
		if (fDirectDraw)
		{
			fDirectDraw->Release();
		}
		
		if (fDDLibraryInstance)
		{
			FreeLibrary(fDDLibraryInstance);
		}
	}

	fDirectDraw = nil;
	fDDLibraryInstance = nil;
}

hsBool hsDXTDirectXCodec::Register()
{
	return hsCodecManager::Instance().Register( &(Instance()), plMipmap::kDirectXCompression, 500 );
}

//// Initialize ///////////////////////////////////////////////////////////////

void hsDXTDirectXCodec::Initialize( IDirect3DDevice8 *directDraw )
{
/*	if (directDraw)
	{
		fFlags |= (kInitialized | kExternalInit);
	}
	else
	{
		fFlags &= ~(kInitialized | kExternalInit);
	}
	fDirectDraw = directDraw;
*/
	// Don't do anything--force IInitialize to run
}

//// IInitialize //////////////////////////////////////////////////////////////
//	Gotta initialize D3D ourself and create a device and everything. This is
//	a LOT messier than it was in DX7, since this time we got D3D to deal with

hsBool	hsDXTDirectXCodec::IInitialize()
{
	fFlags |= kInitialized;

//	if( hsGDDrawDllLoad::GetDDrawDll() == nil )
//		return false;	

    DIRECTDRAWCREATEEX DirectDrawCreateEx = 0;

	// Initialize DirectDraw
	HRESULT hr;
	DirectDrawCreateEx = (DIRECTDRAWCREATEEX)GetProcAddress( hsGDDrawDllLoad::GetD3DDll(), "DirectDrawCreateEx" );
	if( DirectDrawCreateEx == nil )
        return false;

	/// Using EMULATIONONLY here usually fails--using NULL forces the
	/// use of the standard display driver, which DOES work.
	if (FAILED(hr = DirectDrawCreateEx((GUID FAR *)NULL/*DDCREATE_EMULATIONONLY*/, (VOID**)&fDirectDraw, IID_IDirectDraw7, NULL)))
		return false;

	if (FAILED(hr = fDirectDraw->SetCooperativeLevel(NULL, DDSCL_NORMAL)))
		return false;

	return true;
}

//// CreateCompressedMipmap ///////////////////////////////////////////////////
//	Updated 8.15.2000 mcn to generate uncompressed mipmaps down to 1x1 (the
//	decompressor better know how to deal with this!) Also cleaned up so I can
//	read it :)

plMipmap *hsDXTDirectXCodec::CreateCompressedMipmap( plMipmap *uncompressed )
{
	const plMipmap	*b = uncompressed;
	plMipmap		*compressed = nil;

	UInt32	numLevels = 1, numCompLevels;
	UInt32	compFormat, totalSize, compSize;
	Int32	width, height, blockSize, i;


	/// Sanity checks, initialization, etc.
	if( !Initialized() )
	{
		if( !IInitialize() )
			return nil;
	}

	hsAssert( fRegistered, "Calling member of unregistered codec." );
	hsAssert( !uncompressed->IsCompressed(), "Trying to re-compress compressed bitmap." );

	if( !fDirectDraw )
		return nil;

	/// Check width and height
	if( ( uncompressed->GetWidth() | uncompressed->GetHeight() ) & 0x03 )
		return nil;		/// Width and height must be multiple of 4

	/// This used to be checked later--but WHY? We can check it now and
	/// potentially avoid a lot of headache
	compFormat = ICompressedFormat( b );
	if( !compFormat )
		return nil;

	/// Precalc this
	blockSize = ( compFormat == D3DTEXTURE_FMT_FOURCC_DXT1 ) ? 8 : 16;


	{
		compSize = 0;
		numCompLevels = 0;

		width = uncompressed->GetWidth();
		height = uncompressed->GetHeight();

		/// Count the levels we're going to compress
		for( i = 0; i < uncompressed->GetNumLevels(); i++ )
		{
			if( ( width | height ) & 0x03 )
				break;

			numCompLevels++;
			compSize += blockSize * width * height >> 4;
			width >>= 1;
			height >>= 1;
		}

		/// NEW 8.15.2000 mcn - Now count up remaining levels down to 1x1
		totalSize = compSize;
		numLevels = numCompLevels;
		for( ; i < uncompressed->GetNumLevels(); i++ )
		{
			totalSize += uncompressed->GetLevelSize( (UInt8)i );

			width >>= 1;
			height >>= 1;
			numLevels++;
		}
	}


	/// Create source DirectDraw surface
	IDirectDrawSurface7 *srcSurface = IMakeDirect3DSurface( D3DTEXTURE_FMT_ARGB32_8888, 
															numCompLevels, uncompressed->GetWidth(), 
															uncompressed->GetHeight() );

	IFillSurface( (hsRGBAColor32 *)uncompressed->GetImage(), numCompLevels, srcSurface );

	/// Create destination DirectDraw surface
	IDirectDrawSurface7 *destSurface = IMakeDirect3DSurface( compFormat, numCompLevels, 
															uncompressed->GetWidth(), 
															uncompressed->GetHeight() );
	ICopySurface( destSurface, srcSurface, numCompLevels );


	/// Now set up the data structures
	compressed = TRACKED_NEW plMipmap( uncompressed->GetWidth(), uncompressed->GetHeight(), plMipmap::kARGB32Config,
								uncompressed->GetNumLevels(), plMipmap::kDirectXCompression,
								( compFormat == D3DTEXTURE_FMT_FOURCC_DXT1 ) ? 
										plMipmap::DirectXInfo::kDXT1 : plMipmap::DirectXInfo::kDXT5 );
	
	/// Copy compressed data back from the surface
	IFillFromSurface( (hsRGBAColor32 *)compressed->GetImage(), numCompLevels, destSurface );

	/// Finally, copy back any remaining data
	if( numCompLevels < numLevels )
	{
		/// Now copy the rest straight over
		for( i = numCompLevels; i < numLevels; i++ )
		{
			memcpy( compressed->GetLevelPtr( (UInt8)i ), uncompressed->GetLevelPtr( (UInt8)i ), 
					uncompressed->GetLevelSize( (UInt8)i ) );
		}
	}

	/// All done!
	return compressed;
}

//// CreateUncompressedMipmap /////////////////////////////////////////////////
//	Updated 8.15.2000 mcn to support mipmaps down to 1x1. See
//	CreateCompressedMipmap(). Also cleaned up the code a bit (too tired to 
//	clean it ALL up)

plMipmap *hsDXTDirectXCodec::CreateUncompressedMipmap( plMipmap *compressed, 
															UInt8 bitDepth )
{
	/// Use software decompression ALWAYS
	return nil;
/*
	plMipmap	*uncompressed = nil;

	UInt32	mmlvs, formatType, numCompLevels;
	Int32	totalSize, width, height, i;


	/// Check bit depth--if it's 16 bit, we don't support it (for now)
	if( ( bitDepth & hsCodecManager::kBitDepthMask ) != hsCodecManager::k16BitDepth )
		return nil;

	if( !Initialized() )
	{
		if( !IInitialize() )
			return nil;
	}

	/// Sanity checks
	hsAssert( fRegistered, "Calling member of unregistered codec." );
	hsAssert( compressed->fFlags & hsGMipmap::kCompressed, "Trying to uncompress already uncompressed bitmap." );
	hsAssert( compressed->fCompressionFormat == hsGMipmap::kDirectXCompression, "Uncompressing wrong format." );
	hsAssert( ( compressed->fDirectXInfo.fCompressionType == hsGMipmap::DirectXInfo::kDXT1 ) ||
		( compressed->fDirectXInfo.fCompressionType == hsGMipmap::DirectXInfo::kDXT5 ),
		"Unsupported directX compression format." );

	if( !fDirectDraw )
		return nil;

	if( compressed->fDirectXInfo.fCompressionType == hsGMipmap::DirectXInfo::kDXT5 )
	{
		// Fall out since directX software decompressor doesn't work...
		return nil;
	}

	if( compressed->fFlags & hsGMipmap::kMipMap )
	{
		mmCompressed->SetLevel( 0 );
		mmlvs = mmCompressed->GetNumLevels();

		for( i = 0, numCompLevels = 0; i < mmlvs; i++ )
		{
			mmCompressed->SetLevel( i );
			if( ( mmCompressed->fWidth | mmCompressed->fHeight ) & 0x03 )
				break;
			numCompLevels++;
		}
		mmCompressed->SetLevel( 0 );
	}
	else
	{
		mmlvs = numCompLevels = 1;
	}

	/// Get format type
	formatType = ( compressed->fDirectXInfo.fCompressionType == hsGMipmap::DirectXInfo::kDXT1 ) ? 
							D3DTEXTURE_FMT_FOURCC_DXT1 : D3DTEXTURE_FMT_FOURCC_DXT5;

	/// Make the surfaces (decompress in the process)
	IDirectDrawSurface7 *srcSurface = IMakeDirect3DSurface( formatType, numCompLevels, 
															compressed->fWidth, compressed->fHeight );
	IFillSurface( (hsRGBAColor32 *)compressed->fImage, numCompLevels, srcSurface );


	IDirectDrawSurface7 *destSurface = IMakeDirect3DSurface( D3DTEXTURE_FMT_ARGB32_8888, numCompLevels, 
														compressed->fWidth, compressed->fHeight );
	ICopySurface( destSurface, srcSurface, numCompLevels );


	/// Set up the uncompressed data structure
	if( compressed->fFlags & hsGMipmap::kMipMap )
	{
		mmUncompressed = TRACKED_NEW hsGMipmapClass;
		uncompressed = mmUncompressed;
	}
	else
	{
		uncompressed = TRACKED_NEW plMipmap;
		mmUncompressed = nil;
	}

	uncompressed->fWidth = compressed->fWidth;
	uncompressed->fHeight = compressed->fHeight;
	uncompressed->fPixelSize = 32;
	uncompressed->fRowBytes = uncompressed->fWidth * uncompressed->fPixelSize >> 3;
	uncompressed->fFlags = compressed->fFlags & ~hsGMipmap::kCompressed;
	uncompressed->fCompressionFormat = 0;
	uncompressed->fDirectXInfo.fBlockSize = 0;
	uncompressed->fDirectXInfo.fCompressionType = hsGMipmap::DirectXInfo::kError;

	/// Handle mipmaps or single image?
	if( mmUncompressed )
	{
		totalSize = 0;
		width = compressed->fWidth;
		height = compressed->fHeight;
		for( i = 0; i < mmlvs; i++ )
		{
			totalSize += width * height * uncompressed->fPixelSize >> 3;
			width >>= 1;
			height >>= 1;
		}
		mmUncompressed->fImage = HSMemory::New( totalSize );
		mmUncompressed->SetData( mmUncompressed->fImage );
		mmUncompressed->SetNumLevels( mmlvs );
	}
	else
	{
		uncompressed->fImage = HSMemory::New( uncompressed->fWidth * uncompressed->fHeight * 
												uncompressed->fPixelSize >> 3 );
	}

	/// Copy over compressed levels
	IFillFromSurface( (hsRGBAColor32 *)uncompressed->fImage, numCompLevels, destSurface );

	/// Now take care of the remainder levels
	if( mmUncompressed )
	{
		for( i = numCompLevels; i < mmlvs; i++ )
		{
			mmUncompressed->SetLevel( i );
			mmCompressed->SetLevel( i );

			memcpy( mmUncompressed->fImage, mmCompressed->fImage, mmCompressed->ImageSize() );
		}
		mmUncompressed->SetLevel( 0 );
		mmCompressed->SetLevel( 0 );
	}

	/// All done!
	return uncompressed;
*/
}

UInt32 hsDXTDirectXCodec::ICompressedFormat(const plMipmap *uncompressed)
{
	if( uncompressed->GetFlags() & plMipmap::kAlphaChannelFlag )
		return D3DTEXTURE_FMT_FOURCC_DXT5;

	return D3DTEXTURE_FMT_FOURCC_DXT1;
}

//// IFindTextureFormat ///////////////////////////////////////////////////////
//	Changed to a local function 6.8.2001 to avoid certain annoying problems
//	with headers and the compiler.

DDPIXELFORMAT	IFindTextureFormat(UInt32 formatType)
{
	DDPIXELFORMAT ddPixelFormat;
	memset( &ddPixelFormat, 0x00, sizeof(DDPIXELFORMAT) );
	ddPixelFormat.dwSize = sizeof(DDPIXELFORMAT);

	switch (formatType)
	{
	case D3DTEXTURE_FMT_ARGB32_8888:
		ddPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
		ddPixelFormat.dwFourCC = 0;
		ddPixelFormat.dwRGBBitCount = 32;
		ddPixelFormat.dwRBitMask = 0x00ff0000;
		ddPixelFormat.dwGBitMask = 0x0000ff00;
		ddPixelFormat.dwBBitMask = 0x000000ff;
		ddPixelFormat.dwRGBAlphaBitMask = 0xff000000;
		return ddPixelFormat;
	case D3DTEXTURE_FMT_FOURCC_DXT1:
		ddPixelFormat.dwFlags = DDPF_FOURCC;
		ddPixelFormat.dwFourCC = FOURCC_DXT1;
		return ddPixelFormat;
	case D3DTEXTURE_FMT_FOURCC_DXT5:
		ddPixelFormat.dwFlags = DDPF_FOURCC;
		ddPixelFormat.dwFourCC = FOURCC_DXT5;
		return ddPixelFormat;
	default:
		hsAssert(false, "Unknown texture format selected");
		return ddPixelFormat;
	}
}

IDirectDrawSurface7 *hsDXTDirectXCodec::IMakeDirect3DSurface(UInt32 formatType, UInt32 mipMapLevels, 
															  UInt32 width, UInt32 height)
{
	DDSURFACEDESC2 ddsd2;
	memset( &ddsd2, 0x00, sizeof(DDSURFACEDESC2) );
	ddsd2.dwSize          = sizeof(DDSURFACEDESC2);
	ddsd2.dwFlags         = DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT|DDSD_PIXELFORMAT;
	ddsd2.ddsCaps.dwCaps  = DDSCAPS_TEXTURE;
	if(mipMapLevels > 1 )
	{
		ddsd2.dwFlags |= DDSD_MIPMAPCOUNT;
		ddsd2.ddsCaps.dwCaps |= DDSCAPS_MIPMAP|DDSCAPS_COMPLEX;
	}
	ddsd2.dwMipMapCount   = mipMapLevels;
	ddsd2.dwWidth         = width;
	ddsd2.dwHeight        = height;
	ddsd2.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

	ddsd2.ddpfPixelFormat = IFindTextureFormat(formatType);

	IDirectDrawSurface7 *lpDDsTex;
	HRESULT res = fDirectDraw->CreateSurface( &ddsd2, &lpDDsTex, NULL );
	if( S_OK != res )
		CheckErrorCode(res);
	return lpDDsTex;
}

void hsDXTDirectXCodec::IFillSurface(hsRGBAColor32* src, UInt32 mmlvs, IDirectDrawSurface7 *pddsDest)
{
	UInt8					*pTexDat = (UInt8*)src;
	UInt32					cap      = 0;

	HRESULT		hr;
	DDSCAPS2	ddsCaps2;

	for( WORD wNum=0; wNum < mmlvs; wNum++ )
	{
		DDSURFACEDESC2 ddsd2;
		memset( &ddsd2, 0x00, sizeof(DDSURFACEDESC2) );
		ddsd2.dwSize = sizeof( DDSURFACEDESC2 );

		hr  = pddsDest->Lock( NULL, &ddsd2, DDLOCK_WAIT | DDLOCK_WRITEONLY | DDLOCK_DISCARDCONTENTS, NULL );
		if (ddsd2.ddpfPixelFormat.dwFlags == DDPF_FOURCC)
		{
			Int32 blockSize = (ddsd2.ddpfPixelFormat.dwFourCC == FOURCC_DXT1) ? 8 : 16;
			cap = ddsd2.dwHeight * ddsd2.dwWidth * blockSize >> 4;
			memcpy( (char*)ddsd2.lpSurface, pTexDat, cap );
			pTexDat += cap;
		}
		else
		{
			hsAssert(ddsd2.ddpfPixelFormat.dwRGBBitCount == 32, "Format not supported.");

			Int32* dest = (Int32*)ddsd2.lpSurface;
			Int32 pixelCount = ddsd2.dwHeight * ddsd2.dwWidth;
			Int32 j;
			for (j = 0; j < pixelCount; ++j)
			{
				dest[j] =  ( ( src->a << 24 ) | ( src->r << 16 ) | ( src->g << 8 ) | src->b );
				src++;
			}
		}
		hr = pddsDest->Unlock( NULL );

		memset( &ddsCaps2, 0x00, sizeof(DDSCAPS2) );
		ddsCaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
		if( SUCCEEDED( pddsDest->GetAttachedSurface( &ddsCaps2, &pddsDest ) ) )
			pddsDest->Release();
	}
}

void hsDXTDirectXCodec::IFillFromSurface(hsRGBAColor32* dest, UInt32 mmlvs, IDirectDrawSurface7 *pddsSrc)
{
	UInt8			*pTexDat = (UInt8 *)dest;
	UInt32			cap      = 0;

	HRESULT			hr;
	DDSCAPS2		ddsCaps2;
	DDSURFACEDESC2	ddsd2;


	for( WORD wNum = 0; wNum < mmlvs; wNum++ )
	{
		memset( &ddsd2, 0, sizeof( DDSURFACEDESC2 ) );
		ddsd2.dwSize = sizeof( DDSURFACEDESC2 );

		hr = pddsSrc->Lock( NULL, &ddsd2, DDLOCK_WAIT | DDLOCK_READONLY | 
											DDLOCK_DISCARDCONTENTS, NULL );

		if( ddsd2.ddpfPixelFormat.dwFlags == DDPF_FOURCC )
		{
			Int32 blockSize = ( ddsd2.ddpfPixelFormat.dwFourCC == FOURCC_DXT1 ) ? 8 : 16;

			cap = ddsd2.dwHeight * ddsd2.dwWidth * blockSize >> 4;
			memcpy( pTexDat, (char*)ddsd2.lpSurface, cap );
			pTexDat += cap;
		}
		else
		{
			hsAssert( ddsd2.ddpfPixelFormat.dwRGBBitCount == 32, "Format not supported." );

			Int32* src = (Int32*)ddsd2.lpSurface;
			Int32 pixelCount = ddsd2.dwHeight * ddsd2.dwWidth;
			Int32 j;
			for (j = 0; j < pixelCount; ++j)
			{
				dest->a = (UInt8)((src[j] >> 24) & 0xff);
				dest->r = (UInt8)((src[j] >> 16) & 0xff);
				dest->g = (UInt8)((src[j] >> 8) & 0xff);
				dest->b = (UInt8)((src[j]) & 0xff);
				dest++;
			}
		}
		hr = pddsSrc->Unlock( NULL );

		memset( &ddsCaps2, 0, sizeof( DDSCAPS2 ) );
		ddsCaps2.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;

		if( SUCCEEDED( pddsSrc->GetAttachedSurface( &ddsCaps2, &pddsSrc ) ) )
			pddsSrc->Release();
	}
}

void hsDXTDirectXCodec::ICopySurface(IDirectDrawSurface7 *dest, IDirectDrawSurface7 *src, Int32 mipMapLevels)
{
	DDSURFACEDESC2 ddsd2;
	memset( &ddsd2, 0x00, sizeof(DDSURFACEDESC2) );
	ddsd2.dwSize = sizeof( DDSURFACEDESC2 );

	DDSCAPS2 ddsCaps;
	
	ZeroMemory(&ddsCaps, sizeof(ddsCaps));
	ddsCaps.dwCaps = DDSCAPS_TEXTURE;
	if (mipMapLevels > 1)
		ddsCaps.dwCaps2 = DDSCAPS2_MIPMAPSUBLEVEL;

	IDirectDrawSurface7 *lpSrc = src;
	IDirectDrawSurface7 *lpDst = dest;

	int mmlvs = mipMapLevels;

	HRESULT hr;
	int i;
	for( i = 0; i < mmlvs; i++ )
	{
		if( FAILED(hr = lpDst->Blt(NULL, lpSrc, NULL, DDBLT_WAIT, NULL)) )
		{
			hsAssert(false, "Uh oh!");
		}
		
		if (FAILED(hr = lpSrc->GetAttachedSurface(&ddsCaps, &lpSrc))
		  ||FAILED(hr = lpDst->GetAttachedSurface(&ddsCaps, &lpDst)))
		{
			break;
		}
		lpSrc->Release();
		lpDst->Release();
	}
}

void hsDXTDirectXCodec::CheckErrorCode(HRESULT res)
{
	switch( res )
	{
		// This object is already initialized
		case DDERR_ALREADYINITIALIZED:
			hsAssert(false, "DDERR_ALREADYINITIALIZED.");
			break;

		// This surface can not be attached to the requested surface.
		case DDERR_CANNOTATTACHSURFACE:
			hsAssert(false, "DDERR_CANNOTATTACHSURFACE.");
			break;

		// This surface can not be detached from the requested surface.
		case DDERR_CANNOTDETACHSURFACE:
			hsAssert(false, "DDERR_CANNOTDETACHSURFACE.");
			break;

		// Support is currently not available.
		case DDERR_CURRENTLYNOTAVAIL:
			hsAssert(false, "DDERR_CURRENTLYNOTAVAIL.");
			break;

		// An exception was encountered while performing the requested operation
		case DDERR_EXCEPTION:
			hsAssert(false, "DDERR_EXCEPTION.");
			break;

		// Generic failure.
		case DDERR_GENERIC:
			hsAssert(false, "DDERR_GENERIC.");
			break;

		// Height of rectangle provided is not a multiple of reqd alignment
		case DDERR_HEIGHTALIGN:
			hsAssert(false, "DDERR_HEIGHTALIGN.");
			break;

		// Unable to match primary surface creation request with existing
		// primary surface.
		case DDERR_INCOMPATIBLEPRIMARY:
			hsAssert(false, "DDERR_INCOMPATIBLEPRIMARY.");
			break;

		// One or more of the caps bits passed to the callback are incorrect.
		case DDERR_INVALIDCAPS:
			hsAssert(false, "DDERR_INVALIDCAPS.");
			break;

		// DirectDraw does not support provided Cliplist.
		case DDERR_INVALIDCLIPLIST:
			hsAssert(false, "DDERR_INVALIDCLIPLIST.");
			break;

		// DirectDraw does not support the requested mode
		case DDERR_INVALIDMODE:
			hsAssert(false, "DDERR_INVALIDMODE.");
			break;

		// DirectDraw received a pointer that was an invalid DIRECTDRAW object.
		case DDERR_INVALIDOBJECT:
			hsAssert(false, "DDERR_INVALIDOBJECT.");
			break;

		// One or more of the parameters passed to the callback function are
		// incorrect.
		case DDERR_INVALIDPARAMS:
			hsAssert(false, "DDERR_INVALIDPARAMS.");
			break;

		// pixel format was invalid as specified
		case DDERR_INVALIDPIXELFORMAT:
			hsAssert(false, "DDERR_INVALIDPIXELFORMAT.");
			break;

		// Rectangle provided was invalid.
		case DDERR_INVALIDRECT:
			hsAssert(false, "DDERR_INVALIDRECT.");
			break;

		// Operation could not be carried out because one or more surfaces are locked
		case DDERR_LOCKEDSURFACES:
			hsAssert(false, "DDERR_LOCKEDSURFACES.");
			break;

		// There is no 3D present.
		case DDERR_NO3D:
			hsAssert(false, "DDERR_NO3D.");
			break;

		// Operation could not be carried out because there is no alpha accleration
		// hardware present or available.
		case DDERR_NOALPHAHW:
			hsAssert(false, "DDERR_NOALPHAHW.");
			break;

		// no clip list available
		case DDERR_NOCLIPLIST:
			hsAssert(false, "DDERR_NOCLIPLIST.");
			break;

		// Operation could not be carried out because there is no color conversion
		// hardware present or available.
		case DDERR_NOCOLORCONVHW:
			hsAssert(false, "DDERR_NOCOLORCONVHW.");
			break;

		// Create function called without DirectDraw object method SetCooperativeLevel
		// being called.
		case DDERR_NOCOOPERATIVELEVELSET:
			hsAssert(false, "DDERR_NOCOOPERATIVELEVELSET.");
			break;

		// Surface doesn't currently have a color key
		case DDERR_NOCOLORKEY:
			hsAssert(false, "DDERR_NOCOLORKEY.");
			break;

		// Operation could not be carried out because there is no hardware support
		// of the dest color key.
		case DDERR_NOCOLORKEYHW:
			hsAssert(false, "DDERR_NOCOLORKEYHW.");
			break;

		// No DirectDraw support possible with current display driver
		case DDERR_NODIRECTDRAWSUPPORT:
			hsAssert(false, "DDERR_NODIRECTDRAWSUPPORT.");
			break;

		// Operation requires the application to have exclusive mode but the
		// application does not have exclusive mode.
		case DDERR_NOEXCLUSIVEMODE:
			hsAssert(false, "DDERR_NOEXCLUSIVEMODE.");
			break;

		// Flipping visible surfaces is not supported.
		case DDERR_NOFLIPHW:
			hsAssert(false, "DDERR_NOFLIPHW.");
			break;

		// There is no GDI present.
		case DDERR_NOGDI:
			hsAssert(false, "DDERR_NOGDI.");
			break;

		// Operation could not be carried out because there is no hardware present
		// or available.
		case DDERR_NOMIRRORHW:
			hsAssert(false, "DDERR_NOMIRRORHW.");
			break;

		// Requested item was not found
		case DDERR_NOTFOUND:
			hsAssert(false, "DDERR_NOTFOUND.");
			break;

		// Operation could not be carried out because there is no overlay hardware
		// present or available.
		case DDERR_NOOVERLAYHW:
			hsAssert(false, "DDERR_NOOVERLAYHW.");
			break;

		// Operation could not be carried out because the source and destination
		// rectangles are on the same surface and overlap each other.
		case DDERR_OVERLAPPINGRECTS:
			hsAssert(false, "DDERR_OVERLAPPINGRECTS.");
			break;

		// Operation could not be carried out because there is no appropriate raster
		// op hardware present or available.
		case DDERR_NORASTEROPHW:
			hsAssert(false, "DDERR_NORASTEROPHW.");
			break;

		// Operation could not be carried out because there is no rotation hardware
		// present or available.
		case DDERR_NOROTATIONHW:
			hsAssert(false, "DDERR_NOROTATIONHW.");
			break;

		// Operation could not be carried out because there is no hardware support
		// for stretching
		case DDERR_NOSTRETCHHW:
			hsAssert(false, "DDERR_NOSTRETCHHW.");
			break;

		// DirectDrawSurface is not in 4 bit color palette and the requested operation
		// requires 4 bit color palette.
		case DDERR_NOT4BITCOLOR:
			hsAssert(false, "DDERR_NOT4BITCOLOR.");
			break;

		// DirectDrawSurface is not in 4 bit color index palette and the requested
		// operation requires 4 bit color index palette.
		case DDERR_NOT4BITCOLORINDEX:
			hsAssert(false, "DDERR_NOT4BITCOLORINDEX.");
			break;

		// DirectDraw Surface is not in 8 bit color mode and the requested operation
		// requires 8 bit color.
		case DDERR_NOT8BITCOLOR:
			hsAssert(false, "DDERR_NOT8BITCOLOR.");
			break;

		// Operation could not be carried out because there is no texture mapping
		// hardware present or available.
		case DDERR_NOTEXTUREHW:
			hsAssert(false, "DDERR_NOTEXTUREHW.");
			break;

		// Operation could not be carried out because there is no hardware support
		// for vertical blank synchronized operations.
		case DDERR_NOVSYNCHW:
			hsAssert(false, "DDERR_NOVSYNCHW.");
			break;

		// Operation could not be carried out because there is no hardware support
		// for zbuffer blting.
		case DDERR_NOZBUFFERHW:
			hsAssert(false, "DDERR_NOZBUFFERHW.");
			break;

		// Overlay surfaces could not be z layered based on their BltOrder because
		// the hardware does not support z layering of overlays.
		case DDERR_NOZOVERLAYHW:
			hsAssert(false, "DDERR_NOZOVERLAYHW.");
			break;

		// The hardware needed for the requested operation has already been
		// allocated.
		case DDERR_OUTOFCAPS:
			hsAssert(false, "DDERR_OUTOFCAPS.");
			break;

		// DirectDraw does not have enough memory to perform the operation.
		case DDERR_OUTOFMEMORY:
			hsAssert(false, "DDERR_OUTOFMEMORY.");
			break;

		// DirectDraw does not have enough memory to perform the operation.
		case DDERR_OUTOFVIDEOMEMORY:
			hsAssert(false, "DDERR_OUTOFVIDEOMEMORY.");
			break;

		// hardware does not support clipped overlays
		case DDERR_OVERLAYCANTCLIP:
			hsAssert(false, "DDERR_OVERLAYCANTCLIP.");
			break;

		// Can only have ony color key active at one time for overlays
		case DDERR_OVERLAYCOLORKEYONLYONEACTIVE:
			hsAssert(false, "DDERR_OVERLAYCOLORKEYONLYONEACTIVE.");
			break;

		// Access to this palette is being refused because the palette is already
		// locked by another thread.
		case DDERR_PALETTEBUSY:
			hsAssert(false, "DDERR_PALETTEBUSY.");
			break;

		// No src color key specified for this operation.
		case DDERR_COLORKEYNOTSET:
			hsAssert(false, "DDERR_COLORKEYNOTSET.");
			break;

		// This surface is already attached to the surface it is being attached to.
		case DDERR_SURFACEALREADYATTACHED:
			hsAssert(false, "DDERR_SURFACEALREADYATTACHED.");
			break;

		// This surface is already a dependency of the surface it is being made a
		// dependency of.
		case DDERR_SURFACEALREADYDEPENDENT:
			hsAssert(false, "DDERR_SURFACEALREADYDEPENDENT.");
			break;

		// Access to this surface is being refused because the surface is already
		// locked by another thread.
		case DDERR_SURFACEBUSY:
			hsAssert(false, "DDERR_SURFACEBUSY.");
			break;

		// Access to this surface is being refused because no driver exists
		// which can supply a pointer to the surface.
		// This is most likely to happen when attempting to lock the primary
		// surface when no DCI provider is present.
		// Will also happen on attempts to lock an optimized surface.
		case DDERR_CANTLOCKSURFACE:
			hsAssert(false, "DDERR_CANTLOCKSURFACE.");
			break;

		// Access to Surface refused because Surface is obscured.
		case DDERR_SURFACEISOBSCURED:
			hsAssert(false, "DDERR_SURFACEISOBSCURED.");
			break;

		// Access to this surface is being refused because the surface is gone.
		// The DIRECTDRAWSURFACE object representing this surface should
		// have Restore called on it.
		case DDERR_SURFACELOST:
			hsAssert(false, "DDERR_SURFACELOST.");
			break;

		// The requested surface is not attached.
		case DDERR_SURFACENOTATTACHED:
			hsAssert(false, "DDERR_SURFACENOTATTACHED.");
			break;

		// Height requested by DirectDraw is too large.
		case DDERR_TOOBIGHEIGHT:
			hsAssert(false, "DDERR_TOOBIGHEIGHT.");
			break;

		// Size requested by DirectDraw is too large --	 The individual height and
		// width are OK.
		case DDERR_TOOBIGSIZE:
			hsAssert(false, "DDERR_TOOBIGSIZE.");
			break;

		// Width requested by DirectDraw is too large.
		case DDERR_TOOBIGWIDTH:
			hsAssert(false, "DDERR_TOOBIGWIDTH.");
			break;

		// Action not supported.
		case DDERR_UNSUPPORTED:
			hsAssert(false, "DDERR_UNSUPPORTED.");
			break;

		// FOURCC format requested is unsupported by DirectDraw
		case DDERR_UNSUPPORTEDFORMAT:
			hsAssert(false, "DDERR_UNSUPPORTEDFORMAT.");
			break;

		// Bitmask in the pixel format requested is unsupported by DirectDraw
		case DDERR_UNSUPPORTEDMASK:
			hsAssert(false, "DDERR_UNSUPPORTEDMASK.");
			break;

		// The specified stream contains invalid data
		case DDERR_INVALIDSTREAM:
			hsAssert(false, "DDERR_INVALIDSTREAM.");
			break;

		// vertical blank is in progress
		case DDERR_VERTICALBLANKINPROGRESS:
			hsAssert(false, "DDERR_VERTICALBLANKINPROGRESS.");
			break;

		// Informs DirectDraw that the previous Blt which is transfering information
		// to or from this Surface is incomplete.
		case DDERR_WASSTILLDRAWING:
			hsAssert(false, "DDERR_WASSTILLDRAWING.");
			break;

		// Rectangle provided was not horizontally aligned on reqd. boundary
		case DDERR_XALIGN:
			hsAssert(false, "DDERR_XALIGN.");
			break;

		// The GUID passed to DirectDrawCreate is not a valid DirectDraw driver
		// identifier.
		case DDERR_INVALIDDIRECTDRAWGUID:
			hsAssert(false, "DDERR_INVALIDDIRECTDRAWGUID.");
			break;

		// A DirectDraw object representing this driver has already been created
		// for this process.
		case DDERR_DIRECTDRAWALREADYCREATED:
			hsAssert(false, "DDERR_DIRECTDRAWALREADYCREATED.");
			break;

		// A hardware only DirectDraw object creation was attempted but the driver
		// did not support any hardware.
		case DDERR_NODIRECTDRAWHW:
			hsAssert(false, "DDERR_NODIRECTDRAWHW.");
			break;

		// this process already has created a primary surface
		case DDERR_PRIMARYSURFACEALREADYEXISTS:
			hsAssert(false, "DDERR_PRIMARYSURFACEALREADYEXISTS.");
			break;

		// software emulation not available.
		case DDERR_NOEMULATION:
			hsAssert(false, "DDERR_NOEMULATION.");
			break;

		// region passed to Clipper::GetClipList is too small.
		case DDERR_REGIONTOOSMALL:
			hsAssert(false, "DDERR_REGIONTOOSMALL.");
			break;

		// an attempt was made to set a clip list for a clipper objec that
		// is already monitoring an hwnd.
		case DDERR_CLIPPERISUSINGHWND:
			hsAssert(false, "DDERR_CLIPPERISUSINGHWND.");
			break;

		// No clipper object attached to surface object
		case DDERR_NOCLIPPERATTACHED:
			hsAssert(false, "DDERR_NOCLIPPERATTACHED.");
			break;

		// Clipper notification requires an HWND or
		// no HWND has previously been set as the CooperativeLevel HWND.
		case DDERR_NOHWND:
			hsAssert(false, "DDERR_NOHWND.");
			break;

		// HWND used by DirectDraw CooperativeLevel has been subclassed,
		// this prevents DirectDraw from restoring state.
		case DDERR_HWNDSUBCLASSED:
			hsAssert(false, "DDERR_HWNDSUBCLASSED.");
			break;

		// The CooperativeLevel HWND has already been set.
		// It can not be reset while the process has surfaces or palettes created.
		case DDERR_HWNDALREADYSET:
			hsAssert(false, "DDERR_HWNDALREADYSET.");
			break;

		// No palette object attached to this surface.
		case DDERR_NOPALETTEATTACHED:
			hsAssert(false, "DDERR_NOPALETTEATTACHED.");
			break;

		// No hardware support for 16 or 256 color palettes.
		case DDERR_NOPALETTEHW:
			hsAssert(false, "DDERR_NOPALETTEHW.");
			break;

		// If a clipper object is attached to the source surface passed into a
		// BltFast call.
		case DDERR_BLTFASTCANTCLIP:
			hsAssert(false, "DDERR_BLTFASTCANTCLIP.");
			break;

		// No blter.
		case DDERR_NOBLTHW:
			hsAssert(false, "DDERR_NOBLTHW.");
			break;

		// No DirectDraw ROP hardware.
		case DDERR_NODDROPSHW:
			hsAssert(false, "DDERR_NODDROPSHW.");
			break;

		// returned when GetOverlayPosition is called on a hidden overlay
		case DDERR_OVERLAYNOTVISIBLE:
			hsAssert(false, "DDERR_OVERLAYNOTVISIBLE.");
			break;

		// returned when GetOverlayPosition is called on a overlay that UpdateOverlay
		// has never been called on to establish a destionation.
		case DDERR_NOOVERLAYDEST:
			hsAssert(false, "DDERR_NOOVERLAYDEST.");
			break;

		// returned when the position of the overlay on the destionation is no longer
		// legal for that destionation.
		case DDERR_INVALIDPOSITION:
			hsAssert(false, "DDERR_INVALIDPOSITION.");
			break;

		// returned when an overlay member is called for a non-overlay surface
		case DDERR_NOTAOVERLAYSURFACE:
			hsAssert(false, "DDERR_NOTAOVERLAYSURFACE.");
			break;

		// An attempt was made to set the cooperative level when it was already
		// set to exclusive.
		case DDERR_EXCLUSIVEMODEALREADYSET:
			hsAssert(false, "DDERR_EXCLUSIVEMODEALREADYSET.");
			break;

		// An attempt has been made to flip a surface that is not flippable.
		case DDERR_NOTFLIPPABLE:
			hsAssert(false, "DDERR_NOTFLIPPABLE.");
			break;

		// Can't duplicate primary & 3D surfaces, or surfaces that are implicitly
		// created.
		case DDERR_CANTDUPLICATE:
			hsAssert(false, "DDERR_CANTDUPLICATE.");
			break;

		// Surface was not locked.  An attempt to unlock a surface that was not
		// locked at all, or by this process, has been attempted.
		case DDERR_NOTLOCKED:
			hsAssert(false, "DDERR_NOTLOCKED.");
			break;

		// Windows can not create any more DCs, or a DC was requested for a paltte-indexed
		// surface when the surface had no palette AND the display mode was not palette-indexed
		// (in this case DirectDraw cannot select a proper palette into the DC)
		case DDERR_CANTCREATEDC:
			hsAssert(false, "DDERR_CANTCREATEDC.");
			break;

		// No DC was ever created for this surface.
		case DDERR_NODC:
			hsAssert(false, "DDERR_NODC.");
			break;

		// This surface can not be restored because it was created in a different
		// mode.
		case DDERR_WRONGMODE:
			hsAssert(false, "DDERR_WRONGMODE.");
			break;

		// This surface can not be restored because it is an implicitly created
		// surface.
		case DDERR_IMPLICITLYCREATED:
			hsAssert(false, "DDERR_IMPLICITLYCREATED.");
			break;

		// The surface being used is not a palette-based surface
		case DDERR_NOTPALETTIZED:
			hsAssert(false, "DDERR_NOTPALETTIZED.");
			break;

		// The display is currently in an unsupported mode
		case DDERR_UNSUPPORTEDMODE:
			hsAssert(false, "DDERR_UNSUPPORTEDMODE.");
			break;

		// Operation could not be carried out because there is no mip-map
		// texture mapping hardware present or available.
		case DDERR_NOMIPMAPHW:
			hsAssert(false, "DDERR_NOMIPMAPHW.");
			break;

		// The requested action could not be performed because the surface was of
		// the wrong type.
		case DDERR_INVALIDSURFACETYPE:
			hsAssert(false, "DDERR_INVALIDSURFACETYPE.");
			break;

		// Device does not support optimized surfaces, therefore no video memory optimized surfaces
		case DDERR_NOOPTIMIZEHW:
			hsAssert(false, "DDERR_NOOPTIMIZEHW.");
			break;

		// Surface is an optimized surface, but has not yet been allocated any memory
		case DDERR_NOTLOADED:
			hsAssert(false, "DDERR_NOTLOADED.");
			break;

		// Attempt was made to create or set a device window without first setting
		// the focus window
		case DDERR_NOFOCUSWINDOW:
			hsAssert(false, "DDERR_NOFOCUSWINDOW.");
			break;

		// A DC has already been returned for this surface. Only one DC can be
		// retrieved per surface.
		case DDERR_DCALREADYCREATED:
			hsAssert(false, "DDERR_DCALREADYCREATED.");
			break;

		// An attempt was made to allocate non-local video memory from a device
		// that does not support non-local video memory.
		case DDERR_NONONLOCALVIDMEM:
			hsAssert(false, "DDERR_NONONLOCALVIDMEM.");
			break;

		// The attempt to page lock a surface failed.
		case DDERR_CANTPAGELOCK:
			hsAssert(false, "DDERR_CANTPAGELOCK.");
			break;

		// The attempt to page unlock a surface failed.
		case DDERR_CANTPAGEUNLOCK:
			hsAssert(false, "DDERR_CANTPAGEUNLOCK.");
			break;

		// An attempt was made to page unlock a surface with no outstanding page locks.
		case DDERR_NOTPAGELOCKED:
			hsAssert(false, "DDERR_NOTPAGELOCKED.");
			break;

		// There is more data available than the specified buffer size could hold
		case DDERR_MOREDATA:
			hsAssert(false, "DDERR_MOREDATA.");
			break;

		// The data has expired and is therefore no longer valid.
		case DDERR_EXPIRED:
			hsAssert(false, "DDERR_EXPIRED.");
			break;

		// The video port is not active
		case DDERR_VIDEONOTACTIVE:
			hsAssert(false, "DDERR_VIDEONOTACTIVE.");
			break;

		// Surfaces created by one direct draw device cannot be used directly by
		// another direct draw device.
		case DDERR_DEVICEDOESNTOWNSURFACE:
			hsAssert(false, "DDERR_DEVICEDOESNTOWNSURFACE.");
			break;

		// An attempt was made to invoke an interface member of a DirectDraw object
		// created by CoCreateInstance() before it was initialized.
		case DDERR_NOTINITIALIZED:
			hsAssert(false, "DDERR_NOTINITIALIZED.");
			break;

		default:
			hsAssert(false, "Unknown error.");
			break;
	}
}

hsBool hsDXTDirectXCodec::ColorizeCompMipmap( plMipmap *bMap, const UInt8 *colorMask )
{
	return false;
}

