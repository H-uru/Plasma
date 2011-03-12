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
#include "hsTypes.h"
#include "max.h"
#include <commdlg.h>
#include "bmmlib.h"

#include "../../Plasma/PubUtilLib/plGImage/plMipmap.h"
#include "hsExceptionStack.h"
#include "../../Plasma/PubUtilLib/plGImage/hsCodecManager.h"

#include "plBitmapCreator.h"

#include "../MaxMain/plPluginResManager.h"
#include "../MaxExport/plErrorMsg.h"
#include "../MaxPlasmaMtls/Layers/plStaticEnvLayer.h"

#include "../plGImage/plMipmap.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plGImage/plCubicEnvironmap.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plLocalization.h"
#include "../plAgeDescription/plAgeDescription.h"

//// plCommonBitmapLib ///////////////////////////////////////////////////////
//	Derived class for our textures, since they all go in a common page
//	(namely, "Textures")

#include "../MaxMain/plCommonObjLib.h"

class plCommonBitmapLib : public plCommonObjLib
{
	public:
		virtual hsBool	IsInteresting( const plKey &objectKey )
		{
			if( objectKey->GetUoid().GetClassType() == plCubicEnvironmap::Index() ||
				objectKey->GetUoid().GetClassType() == plMipmap::Index() )
			{
				return true;
			}
			return false;
		}
};

static plCommonBitmapLib		sCommonBitmapLib;


plBitmapCreator::plBitmapCreator()
{
	fErrorMsg = nil;
}
plBitmapCreator::~plBitmapCreator()
{
}

plBitmapCreator	&plBitmapCreator::Instance()
{
	static plBitmapCreator	fInstance;
	return fInstance;
}

void	plBitmapCreator::Init( hsBool save, plErrorMsg *msg )
{
	fErrorMsg = msg;
}

void	plBitmapCreator::DeInit( void )
{
	CleanUpMaps();
}

void	plBitmapCreator::CleanUpMaps( void )
{
	sCommonBitmapLib.ClearObjectList();
}

void	plBitmapCreator::DeleteExportedBitmap( const plKey &constKey )
{
	plKey key = constKey;
	sCommonBitmapLib.RemoveObjectAndKey( key );
}

//
// Create Bitmap
//
plMipmap *plBitmapCreator::ICreateBitmap(plBitmapData *bd)
{
	hsGuardBegin("hsConverterUtils::CreateBitmap");

    // Load the bitmap
    BitmapInfo bi;
    bi.SetName(bd->fileName);

#if 0 // This isn't really an issue since the textures are packed -Colin
	const int kMaxFileNameLength = 30;
	if (strlen(bi.Filename()) > kMaxFileNameLength)
	{
		// Allow to continue, But make it painful
		char errStr[256];
		sprintf(errStr, "File name longer than %d, won't burn to CD (%s)", kMaxFileNameLength, bi.Filename());//bitmapTex->GetName());
		MessageBox(GetActiveWindow(), errStr, bd->fileName, MB_OK|MB_ICONEXCLAMATION);	
	}
#endif

	hsBool notMipped = (bd->texFlags & plMipmap::kForceOneMipLevel) != 0;
	float sigma = bd->sig;
    
    // Load the bitmap
	Bitmap *bm = TheManager->Load(&bi);
	if (!bm)
	{
		// FIXME
		/*
		if (fErrorMsg->Set(!(fWarned & kWarnedNoMoreBitmapLoadErr), 
			"Error loading bitmap", pathName).CheckAskOrCancel())
		{
			fWarned |= kWarnedNoMoreBitmapLoadErr;
		}
		*/
		return nil;
	}
	BitmapStorage *storage = bm->Storage();
	BitmapInfo *bInfo = &storage->bi;

	ICheckOutBitmap(bInfo, bm, bd->fileName);

    //
    // Create a plMipmap 
    //
    plMipmap *hBitmap = TRACKED_NEW plMipmap;
    if( (bm->Width() ^ (bm->Width() & -bm->Width()))
        ||(bm->Height() ^ (bm->Height() & -bm->Height())) )
    {
        IResampBitmap(bm, *hBitmap);
    }
    else if( ((bm->Width() >> 3) > bm->Height())||((bm->Height() >> 3) > bm->Width()) )
    {
        IResampBitmap(bm, *hBitmap);
    }
    else
    {
        ICopyBitmap(bm, *hBitmap);
    }
	bm->DeleteThis();

	if( bd->invertAlpha )
		IInvertAlpha(*hBitmap);

	// Do it
	plMipmap *hMipmap = nil;
	if (sigma > 0.f)
	{
		hMipmap = TRACKED_NEW plMipmap(hBitmap, sigma, bd->createFlags, bd->detailDropoffStart, 
								bd->detailDropoffStop, bd->detailMax, bd->detailMin);
	}
	else
	{
		hMipmap = TRACKED_NEW plMipmap(hBitmap, -1.f, bd->createFlags, bd->detailDropoffStart, 
								bd->detailDropoffStop, bd->detailMax, bd->detailMin);
	}
	delete hBitmap;

	/// Clamp the border if we're using clamping
	if( bd->clampFlags != 0 )
	{
		hMipmap->EnsureKonstantBorder( ( bd->clampFlags & plBitmapData::kClampU ) ? true : false,
									   ( bd->clampFlags & plBitmapData::kClampV ) ? true : false );
	}

	/// Cut this down to whatever size we were told to :)
	if( bd->maxDimension != 0 )
		hMipmap->ClipToMaxSize( bd->maxDimension );

	if( notMipped )
	{
		// Done AFTER ClipToMaxSize() so we still get the export size specified 
		hMipmap->RemoveMipping();
	}

	hBitmap = hMipmap;
        
	UInt32	flagsToSet = 0;

	if( bd->texFlags & plMipmap::kNoMaxSize )
		flagsToSet |= plMipmap::kNoMaxSize;
	if( bd->texFlags & plMipmap::kHalfSize )
		flagsToSet |= plMipmap::kHalfSize;
	if( bd->texFlags & plMipmap::kDontThrowAwayImage )
		flagsToSet |= plMipmap::kDontThrowAwayImage;

	hBitmap->SetFlags( hBitmap->GetFlags() | flagsToSet );
	if (bd->useJPEG)
		hBitmap->fCompressionType = plMipmap::kJPEGCompression;

	// FIXME
	if (/*fSave &&*/ !(bd->texFlags & plMipmap::kForceNonCompressed) && !bd->useJPEG)
	{
		// Are we on?  Check Plasma Util panel
//		SwitchUtil *pu = (SwitchUtil *)CreateInstance(UTILITY_CLASS_ID, PlasmaUtilClassID);
//		if (!pu || pu->TextureCompressionEnabled()) 
		{
			plMipmap *compressed = hsCodecManager::Instance().CreateCompressedMipmap(plMipmap::kDirectXCompression, hBitmap);
//				hsDXTSoftwareCodec::Instance().CreateCompressedBitmap(hBitmap);
//				hsDXTDirectXCodec::Instance().CreateCompressedBitmap(hBitmap);

			if (compressed)
			{
				delete hBitmap;
				hBitmap = compressed;
				hBitmap->SetFlags( hBitmap->GetFlags() | flagsToSet );
			}
		}
	}

	return hBitmap;
	hsGuardEnd; 
}




//
// Verify that bitmap is the correct type/size
//
void plBitmapCreator::ICheckOutBitmap(BitmapInfo* bInfo, Bitmap* bm, const char *fileName)
{
	hsGuardBegin("hsConverterUtils::ICheckOutBitmap");

	// Check out bitmap
	if (bm->Flags() & MAP_FLIPPED)
		MessageBox(GetActiveWindow(), "Bitmap is flipped horizontally", fileName, MB_OK);	
	if (bm->Flags() & MAP_INVERTED)
		MessageBox(GetActiveWindow(), "Bitmap is inverted vertically", fileName, MB_OK);	

	if (bInfo->Flags() & MAP_FLIPPED)
		MessageBox(GetActiveWindow(), "BI:Bitmap is flipped horizontally", fileName, MB_OK);	
	if (bInfo->Flags() & MAP_INVERTED)
		MessageBox(GetActiveWindow(), "BI:Bitmap is inverted vertically", fileName, MB_OK);	

	hsGuardEnd;
}

int plBitmapCreator::IResampBitmap(Bitmap *bm, plMipmap &hBitmap)
{
	hsGuardBegin("hsConverterUtils::IResampBitmap");

	BitmapStorage *storage = bm->Storage();
	BitmapInfo *bInfo = &storage->bi;

	int dbgW = bm->Width(), dbgH = bm->Height();
	int it;
	for( it = 1; it <= bm->Width(); it <<= 1 );
	it >>= 1;
	hBitmap.fWidth = it;
	for( it = 1; it <= bm->Height(); it <<= 1 );
	it >>= 1;
	hBitmap.fHeight = it;
	if( (hBitmap.fHeight >> 3) > hBitmap.fWidth )
		hBitmap.fHeight = hBitmap.fWidth << 3;
	else
	if( (hBitmap.fWidth >> 3) > hBitmap.fHeight )
		hBitmap.fWidth = hBitmap.fHeight << 3;

	hBitmap.fPixelSize	= 32;
	hBitmap.fRowBytes	= hBitmap.fWidth * hBitmap.fPixelSize >> 3;
	hBitmap.fNumLevels = 1;

	hBitmap.fImage		= HSMemory::New(hBitmap.fRowBytes * hBitmap.fHeight);

#ifdef COLOR_BLACK_WHITE
	hBitmap.fFlags |= plMipmap::kColorWhite | plMipmap::kColorBlack;
#endif // COLOR_BLACK_WHITE
	hBitmap.fFlags &= ~( plMipmap::kAlphaBitFlag | plMipmap::kAlphaChannelFlag );

	int y,x;
	float scaleY, scaleX;
	hsRGBAColor32  *dstColor;
	dstColor = (hsRGBAColor32*)hBitmap.fImage;

	scaleX = ((float)bm->Width())/(float)hBitmap.fWidth;
	scaleY = ((float)bm->Height())/(float)hBitmap.fHeight;
	for (y = 0; y < hBitmap.fHeight; y++)
	{
		for (x = 0; x < hBitmap.fWidth; x++)
		{
			BMM_Color_64 c64_00, c64_01, c64_10, c64_11;
			int ix, iy;
			float fracX, fracY;
			float t;
			t = x * scaleX;
			ix = (int)t;
			fracX = t-ix;
			t = y * scaleY;
			iy = (int)t;
			fracY = t-iy;

			int ret = storage->GetPixels(ix,iy,1,&c64_00);
// FIXME
//			fErrorMsg->Set(ret == 0, "ResampBitmap", "Failure getting pixels %dX%d", x, y).Check();
			ret = storage->GetPixels(ix+1,iy,1,&c64_10);
			ret = storage->GetPixels(ix,iy+1,1,&c64_01);
			ret = storage->GetPixels(ix+1,iy+1,1,&c64_11);

			dstColor->r = (unsigned char)(255.0 / 65535.0
				* ( c64_00.r * (1.f - fracX) * (1.f - fracY)
				  + c64_10.r * (      fracX) * (1.f - fracY)
				  + c64_01.r * (1.f - fracX) * (      fracY)
				  + c64_11.r * (      fracX) * (      fracY) ));
			dstColor->g = (unsigned char)(255.0 / 65535.0
				* ( c64_00.g * (1.f - fracX) * (1.f - fracY)
				  + c64_10.g * (      fracX) * (1.f - fracY)
				  + c64_01.g * (1.f - fracX) * (      fracY)
				  + c64_11.g * (      fracX) * (      fracY) ));
			dstColor->b =  (unsigned char)(255.0 / 65535.0
				* ( c64_00.b * (1.f - fracX) * (1.f - fracY)
				  + c64_10.b * (      fracX) * (1.f - fracY)
				  + c64_01.b * (1.f - fracX) * (      fracY)
				  + c64_11.b * (      fracX) * (      fracY) ));
			dstColor->a =  (unsigned char)(255.0 / 65535.0
				* ( c64_00.a * (1.f - fracX) * (1.f - fracY)
				  + c64_10.a * (      fracX) * (1.f - fracY)
				  + c64_01.a * (1.f - fracX) * (      fracY)
				  + c64_11.a * (      fracX) * (      fracY) ));


#ifdef COLOR_BLACK_WHITE
			if( dstColor->r | dstColor->g | dstColor->b )
				hBitmap.fFlags &= ~plMipmap::kColorBlack;
			if( ~(dstColor->r & dstColor->g & dstColor->b) )
				hBitmap.fFlags &= ~plMipmap::kColorWhite;
#endif // COLOR_BLACK_WHITE

			if( dstColor->a < 255 )
			{
				hBitmap.fFlags |= plMipmap::kAlphaBitFlag;
				if( dstColor->a > 0 )
					hBitmap.fFlags |= plMipmap::kAlphaChannelFlag;
			}
			dstColor++;
		}
	}
	if( hBitmap.fFlags & plMipmap::kAlphaChannelFlag )
		hBitmap.fFlags &= ~plMipmap::kAlphaBitFlag;

	return 0;
	hsGuardEnd; 
}

int plBitmapCreator::ICopyBitmap(Bitmap *bm, plMipmap &hBitmap)
{
	hsGuardBegin("hsConverterUtils::ICopyBitmap");

	BitmapStorage *storage = bm->Storage();
	BitmapInfo *bInfo = &storage->bi;

	hBitmap.fWidth		= bm->Width();
	hBitmap.fHeight		= bm->Height();
	hBitmap.fPixelSize	= 32;
	hBitmap.fRowBytes	= bm->Width()*hBitmap.fPixelSize/8;
	hBitmap.fNumLevels = 1;

	hBitmap.fImage		= HSMemory::New(hBitmap.fRowBytes * hBitmap.fHeight);

#ifdef COLOR_BLACK_WHITE
	hBitmap.fFlags |= plMipmap::kColorWhite | plMipmap::kColorBlack;
#endif // COLOR_BLACK_WHITE
	hBitmap.fFlags &= ~( plMipmap::kAlphaBitFlag | plMipmap::kAlphaChannelFlag );

	int y,x;
	hsRGBAColor32  *dstColor;
	dstColor = (hsRGBAColor32*)hBitmap.fImage;

	for (y = 0; y < hBitmap.fHeight; y++)
	{
		for (x = 0; x < hBitmap.fWidth; x++)
		{
			BMM_Color_64 c64;
			int ret = storage->GetPixels(x,y,1,&c64);
// FIXME
//			fErrorMsg->Set(ret == 0, "CopyBitmap", "Failure getting pixels %dX%d", x, y).Check();

			// Convert from 16 bits to 8 bits
			dstColor->r = (char)(255.0*c64.r/65535.0);
			dstColor->g = (char)(255.0*c64.g/65535.0);
			dstColor->b = (char)(255.0*c64.b/65535.0);
			dstColor->a = (char)(255.0*c64.a/65535.0);

#ifdef COLOR_BLACK_WHITE
			if( dstColor->r | dstColor->g | dstColor->b )
				hBitmap.fFlags &= ~plMipmap::kColorBlack;
			if( ~(dstColor->r & dstColor->g & dstColor->b) )
				hBitmap.fFlags &= ~plMipmap::kColorWhite;
#endif // COLOR_BLACK_WHITE

			if( dstColor->a < 255 )
			{
				hBitmap.fFlags |= plMipmap::kAlphaBitFlag;
				if( dstColor->a > 0 )
					hBitmap.fFlags |= plMipmap::kAlphaChannelFlag;
			}
			dstColor++;
		}
	}
	if( hBitmap.fFlags & plMipmap::kAlphaChannelFlag )
		hBitmap.fFlags &= ~plMipmap::kAlphaBitFlag;

	return 0;
	hsGuardEnd; 
}

int plBitmapCreator::IInvertAlpha(plMipmap& hBitmap)
{
	hsGuardBegin("hsConverterUtils::ICopyBitmap");

	hsAssert(hBitmap.fPixelSize == 32, "Only RGBA32 implemented");
	if( hBitmap.fPixelSize != 32 )
		return -1;

	hsRGBAColor32* dstColor = (hsRGBAColor32*)hBitmap.fImage;
	int n = hBitmap.GetWidth() * hBitmap.GetHeight();
	int i;
	for( i = 0; i < n; i++ )
	{
		dstColor->a = 255 - dstColor->a;
		dstColor++;
	}

	return 0;
	hsGuardEnd; 
}

plBitmap *plBitmapCreator::CreateTexture(plBitmapData *bd, const plLocation &loc, int clipID)
{
	plBitmap* bm = ICreateTexture(bd, loc, clipID);

	for (int i = 0; i < plLocalization::GetNumLocales(); i++)
	{
		char localName[MAX_PATH];
		if (plLocalization::ExportGetLocalized(bd->fileName, i, localName))
		{
			const char* oldName = bd->fileName;
			bd->fileName = localName;
			ICreateTexture(bd, loc, clipID);
			bd->fileName = oldName;
		}
	}

	return bm;
}

//// ICreateTexture ////////////////////////////////////////////////////////////
//	Plasma texture creator.  Pass it a completed bitmapdata structure and it 
//	returns a registered texture pointer, or nil if something goes wrong.
//
//	9.14.2001 mcn - clipID added to uniquely identify mipmaps that have been 
//	rescaled differently (approximately represents how many powers of 2 it was 
//	scaled down from the original source).
//
//	3.29.2002 mcn - Moved to plBitmapCreator, where it really belongs, and
//	added code to handle tracking/cleaning up all materials that are exported.

plBitmap *plBitmapCreator::ICreateTexture( plBitmapData *bd, const plLocation &loc, int clipID )
{
	hsGuardBegin( "plBitmapCreator::CreateTexture" );

	const plLocation &textureLoc = plPluginResManager::ResMgr()->GetCommonPage( loc, plAgeDescription::kTextures );

	if( !bd )
	{
		fErrorMsg->Set( true, "Bitmap Error", "No bitmap data" ).Show();
		fErrorMsg->Set();
		return nil;
	}

	if( bd->fileName == nil || bd->fileName[ 0 ] == 0 )
	{
		fErrorMsg->Set( true, "Bitmap Error", "Material texture has null bitmap name." ).Show();
		fErrorMsg->Set();	
		return nil;
	}

	// Get and mangle key name
	char name[ 256 ], temp[ 256 ];
	_splitpath(bd->fileName, NULL, NULL, temp, NULL);

	// Somehow, sometimes, we get the same file in with different cases. So we need to force the
	// case identical all the time, else the patching process for dat files will think they're
	// "different" when they're really not
	strlwr( temp );

	/// Mangle name for detail textures, so we don't end up overwriting settings elsewhere
	if( bd->createFlags & plMipmap::kCreateDetailMask )
	{	
		// Mangle of the form: name@dropStart&dropStop&max&min
		if( clipID != -1 )
			sprintf( name, "%s*%x#%d@%s&%3.2f&%3.2f&%3.2f&%3.2f", temp, bd->texFlags, clipID, 
					bd->createFlags & plMipmap::kCreateDetailAlpha ? "al" : ( bd->createFlags & plMipmap::kCreateDetailAdd ? "ad" : "mu" ),
					bd->detailDropoffStart, bd->detailDropoffStop, bd->detailMax, bd->detailMin );
		else
			sprintf( name, "%s*%x@%s&%3.2f&%3.2f&%3.2f&%3.2f", temp, bd->texFlags, 
					bd->createFlags & plMipmap::kCreateDetailAlpha ? "al" : ( bd->createFlags == plMipmap::kCreateDetailAdd ? "ad" : "mu" ),
					bd->detailDropoffStart, bd->detailDropoffStop, bd->detailMax, bd->detailMin );
	}
	else if( clipID != -1 )
		sprintf( name, "%s*%x#%d", temp, bd->texFlags, clipID );
	else
		sprintf( name, "%s*%x", temp, bd->texFlags );
	if( bd->invertAlpha )
		strcat( name, "_inva" );
	strcat( name, ".hsm" );


	// Has this texture been used before?
	plKey key;

	plBitmap *texture = plBitmap::ConvertNoRef( sCommonBitmapLib.FindObject( name, ( bd->isStaticCubicEnvMap ) ? plCubicEnvironmap::Index() : plMipmap::Index() ) );
	//hsAssert( texture == nil || texture->GetKey()->GetUoid().GetLocation() == textureLoc, "Somehow our texture objectLib has a texture not in the right page? Should be harmless tho..." );

	// Texture reuse optimization
	if( texture )
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttrib;
		GetFileAttributesEx(bd->fileName, GetFileExInfoStandard, &fileAttrib);
		FILETIME &fileTime = fileAttrib.ftLastWriteTime;

		// If this texture has been modified since the last export, delete the old version but reuse the key
		if (!texture->IsSameModifiedTime(fileTime.dwLowDateTime, fileTime.dwHighDateTime))
		{
			DeleteExportedBitmap( texture->GetKey() );
			texture = nil;
			key = nil;
		}
	}

	if( texture )
	{
		// If it's in the registry, great, use it.
		if( bd->texFlags & plMipmap::kNoMaxSize )
			texture->SetFlags( texture->GetFlags() | plMipmap::kNoMaxSize );

		if( bd->texFlags & plMipmap::kHalfSize )
			texture->SetFlags( texture->GetFlags() | plMipmap::kHalfSize );
	}
	else
	{
		// If it hasn't been used before, make a new texture
		if( bd->isStaticCubicEnvMap )
		{
			plCubicEnvironmap *cubic = TRACKED_NEW plCubicEnvironmap;
			
			plMipmap	*face;

			/// Build and set the faces
			bd->fileName = bd->faceNames[ plStaticEnvLayer::kTopFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kTopFace );

			bd->fileName = bd->faceNames[ plStaticEnvLayer::kBottomFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kBottomFace );

			bd->fileName = bd->faceNames[ plStaticEnvLayer::kLeftFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kLeftFace );

			bd->fileName = bd->faceNames[ plStaticEnvLayer::kRightFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kRightFace );

			/// NOTE: For whatever reason, MAX decided that the front and back faces should be'
			/// switched, literally. It's as if the cube for the cube map starts at the back face
			/// and then wraps around, instead of starting at the front face. Since we do things
			/// the RIGHT way (or rather, the front way :) on client-side, we need to flip the 
			/// two here. If you convert this to the real MAX UI, make sure the faces are still
			/// flipped!!!!!!!!

			bd->fileName = bd->faceNames[ plStaticEnvLayer::kBackFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kFrontFace );

			bd->fileName = bd->faceNames[ plStaticEnvLayer::kFrontFace ];
			face = ICreateBitmap( bd );
			if( face == nil ) return nil;
			cubic->CopyToFace( face, plCubicEnvironmap::kBackFace );


			key = hsgResMgr::ResMgr()->NewKey( name, cubic, textureLoc );

			texture = (plBitmap *)cubic;
		}
		else
		{
			plMipmap *mipmap = ICreateBitmap(bd);
			if (!mipmap)
				return nil;

			key = hsgResMgr::ResMgr()->NewKey( name, mipmap, textureLoc );

			texture = (plBitmap *)mipmap;
		}

		// Texture reuse optimization
		WIN32_FILE_ATTRIBUTE_DATA fileAttrib;
		GetFileAttributesEx(bd->fileName, GetFileExInfoStandard, &fileAttrib);
		FILETIME &fileTime = fileAttrib.ftLastWriteTime;
		texture->SetModifiedTime(fileTime.dwLowDateTime, fileTime.dwHighDateTime);

		// Add to our list of created textures and ref, since we have a hold of them
		IAddBitmap( texture );
	}

	return texture;

	hsGuardEnd;
}

//// IAddBitmap ///////////////////////////////////////////////////////////////

void	plBitmapCreator::IAddBitmap( plBitmap *bitmap, hsBool dontRef )
{
	sCommonBitmapLib.AddObject( bitmap );
}

//// CreateBlankMipmap ////////////////////////////////////////////////////////
//	Simple mipmap creator, but importantly, it also adds the mipmap to the list
//	of "converted" maps to clean up at the end of export.

plMipmap	*plBitmapCreator::CreateBlankMipmap( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels, 
												 const char *keyName, const plLocation &keyLocation )
{
	hsGuardBegin( "plBitmapCreator::CreateBlankMipmap" );

	// Get our real location
	const plLocation &textureLoc = plPluginResManager::ResMgr()->GetCommonPage( keyLocation, plAgeDescription::kTextures );

	// Is it already created?
	plKey key = hsgResMgr::ResMgr()->FindKey( plUoid( textureLoc, plMipmap::Index(), keyName ) );
	if( key != nil )
		return plMipmap::ConvertNoRef( key->GetObjectPtr() );

	// Create
	plMipmap	*mip = TRACKED_NEW plMipmap( width, height, config, numLevels );

	// Assign key
	hsgResMgr::ResMgr()->NewKey( keyName, mip, textureLoc );

	// Add to our list
	IAddBitmap( mip );

	return mip;

	hsGuardEnd;
}

