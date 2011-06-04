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
//	plMipmap Class Functions												 //
//	Derived bitmap class representing a single mipmap.						 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plMipmap.h"
#include "hsStream.h"
#include "hsExceptions.h"
#include "hsUtils.h"
#include "hsColorRGBA.h"
#include "../plPipeline/hsGDeviceRef.h"
#include "plProfile.h"
#include "../plJPEG/plJPEG.h"

plProfile_CreateMemCounter("Mipmaps", "Memory", MemMipmaps);

//// Constructor & Destructor /////////////////////////////////////////////////

plMipmap::plMipmap() : fImage( nil ), fLevelSizes( nil ), fCurrLevelPtr( nil ), fCurrLevel( 0 ), fTotalSize( 0 )
{
	SetConfig( kARGB32Config );
	fCompressionType = kUncompressed;
	fUncompressedInfo.fType = UncompressedInfo::kRGB8888;

#ifdef MEMORY_LEAK_TRACER
	fNumMipmaps++;
#endif
}

plMipmap::~plMipmap()
{
	Reset();

#ifdef MEMORY_LEAK_TRACER
	fNumMipmaps--;
	if( fNumMipmaps == 0 )
		IReportLeaks();
#endif
}

plMipmap::plMipmap( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels, UInt8 compType, UInt8 format )
{
	Create( width, height, config, numLevels, compType, format );

#ifdef MEMORY_LEAK_TRACER
	fNumMipmaps++;
#endif
}

//// Create ///////////////////////////////////////////////////////////////////

void	plMipmap::Create( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels, UInt8 compType, UInt8 format )
{
	int		i;


	SetConfig( config );

	fWidth = width;
	fHeight = height;
	fRowBytes = fWidth * fPixelSize >> 3;
	if( numLevels > 0 )
		fNumLevels = numLevels;
	else
	{
		for( fNumLevels = 1; width > 1 || height > 1; fNumLevels++ )
		{
			if( width > 1 )
				width >>= 1;
			if( height > 1 )
				height >>= 1;
		}
	}
	
	fCompressionType = compType;
	if( compType == kUncompressed )
	{
		fUncompressedInfo.fType = format;
	}
	else if( compType == kJPEGCompression )
	{
		fUncompressedInfo.fType = format;
	}
	else
	{
		fDirectXInfo.fBlockSize = ( format == DirectXInfo::kDXT1 ) ? 8 : 16;
		fDirectXInfo.fCompressionType = format;

		if( format == DirectXInfo::kDXT1 )
		{
			// Has an alpha bit, but no channel
			fFlags |= kAlphaBitFlag;
			fFlags &= ~kAlphaChannelFlag;
		}
		else // All other formats have an actual alpha channel
		{
			fFlags &= ~kAlphaBitFlag;
			fFlags |= kAlphaChannelFlag;
		}
	}

	fLevelSizes = nil;
	IBuildLevelSizes();

	fTotalSize = 0;
	for( i = 0; i < fNumLevels; i++ )
		fTotalSize += fLevelSizes[ i ];

	fImage = (void *)TRACKED_NEW UInt8[ fTotalSize ];
	memset(fImage, 0, fTotalSize);

	SetCurrLevel( 0 );
	plProfile_NewMem(MemMipmaps, fTotalSize);

#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaCreate );
#endif

}

//// Reset ////////////////////////////////////////////////////////////////////

void	plMipmap::Reset()
{
	delete [] fLevelSizes;
	fLevelSizes = nil;
	if( !( fFlags & kUserOwnsBitmap ) )
	{
#ifdef MEMORY_LEAK_TRACER
		if( fImage != nil )
			IRemoveFromMemRecord( (UInt8 *)fImage );
#endif

		delete [] fImage;
		plProfile_DelMem(MemMipmaps, fTotalSize);
	}
	fImage = nil;
}


///////////////////////////////////////////////////////////////////////////////
//// Virtual Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// GetTotalSize /////////////////////////////////////////////////////////////
//	Get the total size in bytes

UInt32	plMipmap::GetTotalSize() const
{
	return fTotalSize;
}

//// Read /////////////////////////////////////////////////////////////////////

UInt32  plMipmap::Read( hsStream *s )
{
	UInt32 totalRead = plBitmap::Read( s );

	// Decide to clamp if we were told to
	int	clampBy = fGlobalNumLevelsToChop;
	const kMaxSkipLevels = 1;
	if( clampBy > kMaxSkipLevels )
		clampBy = kMaxSkipLevels;
	if( fFlags & kNoMaxSize )
		clampBy = 0;
	else if( ( fFlags & kHalfSize ) && clampBy > 1 )
		clampBy = 1;
	UInt32	amtToSkip = 0;

	fWidth = s->ReadSwap32();
	fHeight = s->ReadSwap32();
	fRowBytes = s->ReadSwap32();
	fTotalSize = s->ReadSwap32();
	fNumLevels = s->ReadByte();

	totalRead += 4 * 4 + 1;
	
	if( fTotalSize == 0 )
		fImage = nil; 
	else
	{
		IBuildLevelSizes();

		if (fCompressionType != kJPEGCompression) // JPEGs don't play nicely with quality settings the way they are written, so we ignore them
		{
			if( clampBy > 0 )
			{
				int	i;
				for( i = 0; i < clampBy && fNumLevels > 1 && fWidth > 4 && fHeight > 4; i++ )
				{
					amtToSkip += fLevelSizes[ i ];
					fWidth >>= 1;
					fHeight >>= 1;
					fRowBytes >>= 1;
					fNumLevels--;
				}
				fTotalSize -= amtToSkip;
				IBuildLevelSizes();
			}
		}

		fImage = (void *)TRACKED_NEW UInt8[ fTotalSize ];
#ifdef MEMORY_LEAK_TRACER
		IAddToMemRecord( this, plRecord::kViaRead );
#endif
		plProfile_NewMem(MemMipmaps, fTotalSize);
		
		switch( fCompressionType )
		{
			case kDirectXCompression:
				s->Skip( amtToSkip );
				s->Read( fTotalSize, fImage );
				break;
				
			case kUncompressed:
				s->Skip( amtToSkip );
				IReadRawImage( s );
				break;
				
			case kJPEGCompression:
				IReadJPEGImage( s );
				break;
				
			default:
				hsAssert( false, "Unknown compression type in plMipmap::Read()" );
				return totalRead;
		}
		totalRead += fTotalSize;
	}
	return totalRead;
}

//// Write ////////////////////////////////////////////////////////////////////

UInt32 	plMipmap::Write( hsStream *s )
{
	UInt32 totalWritten = plBitmap::Write( s );

	s->WriteSwap32( fWidth );
	s->WriteSwap32( fHeight );
	s->WriteSwap32( fRowBytes );
	s->WriteSwap32( fTotalSize );
	s->WriteByte( fNumLevels );

	totalWritten += 4 * 4 + 1;

	if( fTotalSize > 0 )
	{
		switch( fCompressionType )
		{
			case kDirectXCompression:
				s->Write( fTotalSize, fImage );
				break;

			case kUncompressed:
				IWriteRawImage( s );
				break;

			case kJPEGCompression:
				IWriteJPEGImage( s );
				break;

			default:
				hsAssert( false, "Unknown compression type in plMipmap::Read()" );
				return totalWritten;
		}
		totalWritten += fTotalSize;
	}

	return totalWritten;
}


///////////////////////////////////////////////////////////////////////////////
//// Utility Functions ////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IReadRawImage ////////////////////////////////////////////////////////////

void	plMipmap::IReadRawImage( hsStream *stream )
{
	UInt32		i;
	UInt8		*data = (UInt8 *)fImage;


	switch( fPixelSize )
	{
		case 32:
			for( i = 0; i < fNumLevels; i++ )
			{	
				stream->ReadSwap32( fLevelSizes[ i ] >> 2, (UInt32 *)data );
				data += fLevelSizes[ i ];
			}
			break;

		case 16:
			for( i = 0; i < fNumLevels; i++ )
			{
				stream->ReadSwap16( fLevelSizes[ i ] >> 1, (UInt16 *)data );
				data += fLevelSizes[ i ];
			}
			break;

		default:
			hsThrow( hsBadParamException() );
	}
}

//// IWriteRawImage ///////////////////////////////////////////////////////////

void	plMipmap::IWriteRawImage( hsStream *stream )
{
	UInt32		i;
	UInt8		*data = (UInt8 *)fImage;


	switch( fPixelSize )
	{
		case 32:
			for( i = 0; i < fNumLevels; i++ )
			{	
				stream->WriteSwap32( fLevelSizes[ i ] >> 2, (UInt32 *)data );
				data += fLevelSizes[ i ];
			}
			break;

		case 16:
			for( i = 0; i < fNumLevels; i++ )
			{
				stream->WriteSwap16( fLevelSizes[ i ] >> 1, (UInt16 *)data );
				data += fLevelSizes[ i ];
			}
			break;

		default:
			hsThrow( hsBadParamException() );
	}
}

plMipmap *plMipmap::ISplitAlpha()
{
	plMipmap *retVal = TRACKED_NEW plMipmap();
	retVal->CopyFrom(this);
	memset( retVal->fImage, 0, fTotalSize );

	UInt8 *curLoc = (UInt8 *)fImage;
	UInt8 *destLoc = (UInt8 *)retVal->fImage;
	UInt32 numBytes = fTotalSize;
	UInt32 curByte = 0;

	switch( fUncompressedInfo.fType )
	{
	case fUncompressedInfo.kRGB8888:
		// first byte is the alpha channel, we will drop this byte into the red channel for compression
		while (curByte < numBytes)
		{
			curLoc += 3;
			destLoc += 2; // make the destination pointer point at the red channel
			*destLoc = *curLoc; // copy the information
			destLoc += 2; // make the destination pointer point at the beginning of the next pixel
			curLoc ++; // same here for source pointer
			curByte += 4;
		}
		break;
	default:
		break; // not going to mess with other formats for now
	}
	return retVal;
}

// alphaChannel must be in the format generated from ISplitAlpha, or strange things will happen
void plMipmap::IRecombineAlpha( plMipmap *alphaChannel )
{
	UInt8 *curLoc = (UInt8 *)alphaChannel->fImage;
	UInt8 *destLoc = (UInt8 *)fImage;
	UInt32 numBytes = fTotalSize;
	UInt32 curByte = 0;

	switch( fUncompressedInfo.fType )
	{
	case fUncompressedInfo.kRGB8888:
		// first byte is the alpha channel, we will grab this byte from the red channel for reconstitution
		while (curByte < numBytes)
		{
			curLoc += 2; // pointer points at the red channel (where the alpha is stored)
			destLoc += 3;
			*destLoc = *curLoc; // copy the data
			destLoc++; // move the pointer to the next pixel
			curLoc += 2;
			curByte += 4;
		}
		break;
	default:
		break; // not going to mess with other formats for now
	}
	fFlags |= plBitmap::kAlphaChannelFlag;
}

plMipmap *plMipmap::IReadRLEImage( hsStream *stream )
{
	UInt32 count,color;
	bool done = false;

	plMipmap *retVal = TRACKED_NEW plMipmap(fWidth,fHeight,plMipmap::kARGB32Config,1);

	UInt32 *curPos = (UInt32*)retVal->fImage;
	UInt32 curLoc = 0;

	while (!done)
	{
		count = stream->ReadSwap32();
		color = stream->ReadSwap32();
		if (count == 0)
			done = true;
		else
		{
			for (UInt32 i=0; i<count; i++)
			{
				*curPos = color;
				curPos++;
				curLoc++;
			}
		}
	}
	// We really don't want to suddenly start calling this uncompressed now that it's read in.
	// Case in point, on export we load in all previously exported textures (like this JPEG one)
	// share those, add any textures that aren't already there, then write the whole thing back
	// out. Viola, we just converted our nice small compressed 1024x1024 JPEG (~128k) to a 
	// monster uncompressed 4Mb which it will remain for ever more.
//	retVal->fCompressionType = kUncompressed;
	return retVal;
}

void plMipmap::IWriteRLEImage( hsStream *stream, plMipmap *mipmap )
{
	UInt32 count=0,color=0,curColor=0;
	UInt32 curPos = 0;
	UInt32 totalSize = mipmap->fLevelSizes[0]/4; // we only save the first mipmap level

	UInt32 *src = (UInt32*)mipmap->fImage;
	curColor = *src;
	curColor &= 0x00FFFFFF; // strip the alpha (if there is any)
	while (curPos < totalSize)
	{
		color = *src;
		color &= 0x00FFFFFF; // strip the alpha (if there is any)
		if (color != curColor)
		{
			stream->WriteSwap32(count);
			stream->WriteSwap32(curColor);
			count = 0;
			curColor = color;
		}
		count++;
		src++;
		curPos++;
	}
	stream->WriteSwap32(count);
	stream->WriteSwap32(color);
	stream->WriteSwap32(0); // terminate with zero count
	stream->WriteSwap32(0);
}

void plMipmap::IReadJPEGImage( hsStream *stream )
{
	UInt8 flags = 0;
	flags = stream->ReadByte();

	plMipmap *temp = nil;
	plMipmap *alpha = nil;

	if (flags & kColorDataRLE)
		temp = IReadRLEImage(stream);
	else
		temp = plJPEG::Instance().ReadFromStream(stream);
	
	if (temp)
	{
		// copy the data
		CopyFrom(temp);
		
		if (flags & kAlphaDataRLE)
			alpha = IReadRLEImage(stream);
		else
			alpha = plJPEG::Instance().ReadFromStream(stream);

		if (alpha)
		{
			IRecombineAlpha(alpha);
			delete alpha;
		}
		delete temp;
	}
}

void plMipmap::IWriteJPEGImage( hsStream *stream )
{
	plMipmap *alpha = ISplitAlpha();
	UInt8 flags = 0;

	hsNullStream *nullStream = TRACKED_NEW hsNullStream();
	IWriteRLEImage(nullStream,this);
	if (nullStream->GetBytesWritten() < 5120) // we use RLE if it can get the image size under 5k, otherwise we use JPEG
		flags |= kColorDataRLE;
	delete nullStream;
	nullStream = TRACKED_NEW hsNullStream();
	IWriteRLEImage(nullStream,alpha);
	if (nullStream->GetBytesWritten() < 5120)
		flags |= kAlphaDataRLE;
	delete nullStream;
	stream->WriteByte(flags);

	if (flags & kColorDataRLE)
		IWriteRLEImage(stream,this);
	else
	{
		plJPEG::Instance().SetWriteQuality(70);
		plJPEG::Instance().WriteToStream(stream, this);
	}
	if (flags & kAlphaDataRLE)
		IWriteRLEImage(stream,alpha);
	else
	{
		plJPEG::Instance().SetWriteQuality(100);
		plJPEG::Instance().WriteToStream(stream, alpha);
	}
	delete alpha;
}

//// GetLevelSize /////////////////////////////////////////////////////////////
//	Get the size of a single mipmap level (0 is the largest)

UInt32	plMipmap::GetLevelSize( UInt8 level )
{
	if( fLevelSizes == nil )
		IBuildLevelSizes();

	return fLevelSizes[ level ];
}

//// IBuildLevelSizes /////////////////////////////////////////////////////////
//	Creates the table of level sizes, for quick reference

void	plMipmap::IBuildLevelSizes()
{
	UInt8		level;
	UInt32		width, height, rowBytes;


	delete [] fLevelSizes;
	fLevelSizes = TRACKED_NEW UInt32[ fNumLevels ];
	memset( fLevelSizes, 0, sizeof( UInt32 ) * fNumLevels );

	for( level = 0, width = fWidth, height = fHeight, rowBytes = fRowBytes; level < fNumLevels; level++ )
	{
		switch( fCompressionType )
		{
			case kDirectXCompression:
				if( ( width | height ) & 0x03 )
					fLevelSizes[ level ] = height * rowBytes;
				else
					fLevelSizes[ level ] = ( height * width * (UInt32)fDirectXInfo.fBlockSize ) >> 4;
				break;

			case kUncompressed:
			case kJPEGCompression:
				fLevelSizes[ level ] = height * rowBytes;
				break;

			default:
				hsAssert( false, "Bad compression type." );
				return;
		}

		// Scale down and go!
		if( width > 1 )
		{
			width >>= 1;
			rowBytes >>= 1;
		}
		if( height > 1 )
			height >>= 1;
	}
}

//// GetLevelPtr //////////////////////////////////////////////////////////////

UInt8	*plMipmap::GetLevelPtr( UInt8 level, UInt32 *width, UInt32 *height, UInt32 *rowBytes )
{
	UInt8	*data, i;
	UInt32	w, h, r;


	if( fLevelSizes == nil )
		IBuildLevelSizes();

	for( i = 0, data = (UInt8 *)fImage, w = fWidth, h = fHeight, r = fRowBytes; i < level; i++ )
	{
		data += fLevelSizes[ i ];
		if( w > 1 )
		{
			w >>= 1;
			r >>= 1;
		}
		if( h > 1 )
			h >>= 1;
	}

	if( width != nil )
		*width = w;
	if( height != nil )
		*height = h;
	if( rowBytes != nil )
		*rowBytes = r;

	return data;
}

//// SetCurrLevel /////////////////////////////////////////////////////////////
//	Sets the current level pointer for use with GetAddr*

void	plMipmap::SetCurrLevel( UInt8 level )
{
	fCurrLevel = level;
	fCurrLevelPtr = GetLevelPtr( level, &fCurrLevelWidth, &fCurrLevelHeight, &fCurrLevelRowBytes );
}

//// SetConfig ////////////////////////////////////////////////////////////////

void	plMipmap::SetConfig( unsigned config )
{
	switch( config )
	{
		case kRGB32Config:
			fPixelSize	= 32;
			fSpace	= kDirectSpace;
			fFlags	= kNoFlag;
			break;
		case kARGB32Config:
			fPixelSize	= 32;
			fSpace	= kDirectSpace;
			fFlags	= kAlphaChannelFlag;
			break;
		case kRGB16Config:
			fPixelSize	= 16;
			fSpace	= kDirectSpace;
			fFlags	= kAlphaBitFlag;
			break;
		case kColor8Config:
			fPixelSize	= 8;
			fSpace	= kIndexSpace;
			fFlags	= kNoFlag;
			break;
		case kGray8Config:
			fPixelSize	= 8;
			fSpace	= kDirectSpace;
			fFlags	= kNoFlag;
			break;
		case kGray44Config:
			fPixelSize	= 8;
			fSpace	= kGraySpace;
			fFlags	= kAlphaChannelFlag;
			break;
		case kGray4Config:
			fPixelSize	= 4;
			fSpace	= kGraySpace;
			fFlags	= kNoFlag;
			break;
		default:
			hsDebugMessage( "unknown config", config );
			break;
	}
}

//// ClipToMaxSize ////////////////////////////////////////////////////////////
//	Looks for mipmap levels above the given dimension and "clips" them out, 
//	i.e. deletes them. So if you have a 512x1024 mipmap and call this with
//	a size of 256, when this function returns the mipmap will be 256x128 in
//	size.

void	plMipmap::ClipToMaxSize( UInt32 maxDimension )
{
	UInt8		*srcData, *destData, i;
	UInt32		newSize;


	for( i = 0, newSize = fTotalSize, srcData = (UInt8 *)fImage; fWidth > maxDimension || fHeight > maxDimension; i++ )
	{
		srcData += fLevelSizes[ i ];
		newSize -= fLevelSizes[ i ];
		if( fWidth > 1 )
		{
			fWidth >>= 1;
			fRowBytes >>= 1;
		}
		if( fHeight > 1 )
			fHeight >>= 1;
	}

	if( i == 0 )
		// No change
		return;

	/// Create a new image pointer
	destData = TRACKED_NEW UInt8[ newSize ];
	hsAssert( destData != nil, "Out of memory in ClipToMaxSize()" );
	memcpy( destData, srcData, newSize );

#ifdef MEMORY_LEAK_TRACER
	IRemoveFromMemRecord( (UInt8 *)fImage );
#endif
	delete [] fImage;
	plProfile_DelMem(MemMipmaps, fTotalSize);

	fImage = destData;
	fTotalSize = newSize;
	fNumLevels -= i;
	IBuildLevelSizes();
	plProfile_NewMem(MemMipmaps, fTotalSize);

#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaClipToMaxSize );
#endif
}

//// RemoveMipping ////////////////////////////////////////////////////////////
//	Basically removes mipmap levels so that there is only one level remaining;
//	i.e. a plain bitmap instead of a mipmap.

void	plMipmap::RemoveMipping()
{
	UInt8		*destData;


	/// Create a new image pointer
	destData = TRACKED_NEW UInt8[ fLevelSizes[ 0 ] ];
	hsAssert( destData != nil, "Out of memory in ClipToMaxSize()" );
	memcpy( destData, fImage, fLevelSizes[ 0 ] );

#ifdef MEMORY_LEAK_TRACER
	IRemoveFromMemRecord( (UInt8 *)fImage );
#endif
	delete [] fImage;
	plProfile_DelMem(MemMipmaps, fTotalSize);

	fImage = destData;
	fTotalSize = fLevelSizes[ 0 ];
	fNumLevels = 1;
	fFlags |= kForceOneMipLevel;
	IBuildLevelSizes();
	plProfile_NewMem(MemMipmaps, fTotalSize);

#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaClipToMaxSize );
#endif
}


///////////////////////////////////////////////////////////////////////////////



#include "hsCodecManager.h"


namespace {
	const UInt32 kVersion			= 1;
	const hsScalar kDefaultSigma	= 1.f;
	const UInt32 kDefaultDetailBias = 5;

	// Color masks (out of 0-2)
	const UInt8 fColorMasks[ 10 ][ 3 ] = { { 2, 0, 0 }, { 0, 2, 2 }, { 2, 0, 2 }, { 0, 2, 0 },			
				{ 0, 0, 2 }, { 2, 2, 0 }, { 2, 2, 2 }, { 2, 0, 1 }, { 0, 2, 1 }, { 1, 0, 2 } };
											
}

///////////////////////////////////////////////////////////////////////////////
//// plFilterMask Helper Class ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// This filter mask class actually introduces a half pixel per level shift
// artifact. It's done that since I wrote it a couple of years ago, and nobody's
// noticed but me, so it's probably okay. But fixing it would be easy. The
// mask needs to extend -n down and n+1 up. To see why, notice that the current
// upper level pixel (i,j) maps to four lower pixels (i<<1,j<<1) .. (i<<1+1,j<<1+1).
// So the mask value on pixel (i<<1,j<<1) should give equal weight to those other
// 3 pixels.
// The mask here is symmetric about the src pixel, which is right if you are
// just filtering the src bitmap. But we're filtering and resampling, so the
// mask needs to be symmetric about the dst pixel, which is off by a half
// dst pixel, or on whole src pixel.
class plFilterMask
{
	protected:
		int				fExt;
		hsScalar		**fMask;

	public:

		plFilterMask( hsScalar sig );
		virtual ~plFilterMask();

		int		Begin() const { return -fExt; }
		int		End() const { return fExt; }

		hsScalar	Mask( int i, int j ) const { return fMask[ i ][ j ]; }
};

plFilterMask::plFilterMask( hsScalar sig )
{
	fExt = (int)( sig * 2.f );
	if( fExt < 1 )
		fExt = 1;

	hsScalar **m = TRACKED_NEW hsScalar *[ ( fExt << 1 ) + 1 ];
	m += fExt;
	int i, j;
	hsScalar ooSigSq = 1.f / ( sig * sig );

	for( i = -fExt; i <= fExt; i++ )
	{
		m[ i ] = ( TRACKED_NEW hsScalar[ ( fExt << 1 ) + 1] ) + fExt;
		for( j = -fExt; j <= fExt; j++ )
		{
			m[ i ][ j ] = expf( -( i*i + j*j ) * ooSigSq );
		}
	}
	fMask = m;
}

plFilterMask::~plFilterMask()
{
	int i;
	for( i = -fExt; i <= fExt; i++ )
		delete [] ( fMask[ i ] - fExt );
	delete [] ( fMask - fExt );
}


///////////////////////////////////////////////////////////////////////////////
//// Some More Functions //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

plMipmap::plMipmap( plMipmap *bm, hsScalar sig, UInt32 createFlags, 
        hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
        hsScalar detailMax, hsScalar detailMin)
{
	int		i;


	hsAssert(bm->GetHeight() && bm->GetWidth(), "Degenerate Bitmap into Mipmap");

	if( sig <= 0 )
		sig = kDefaultSigma;

	fHeight = bm->GetHeight();
	fWidth = bm->GetWidth();
	fRowBytes = bm->GetRowBytes();
	fPixelSize = bm->GetPixelSize();
	fImage = nil;
	fFlags = bm->GetFlags();

	UInt32 minDim = fHeight < fWidth ? fHeight : fWidth;
	for( fNumLevels = 0; (minDim >> fNumLevels); fNumLevels++ ) /* empty */;

	fLevelSizes = nil;
	fCompressionType = kUncompressed;
	fUncompressedInfo.fType = bm->fUncompressedInfo.fType;
	IBuildLevelSizes();

	fTotalSize = 0;
	for( i = 0; i < fNumLevels; i++ )
		fTotalSize += fLevelSizes[ i ];
	fCurrLevel = 0;

	fImage = (void *)TRACKED_NEW UInt8[ fTotalSize ];
	memset(fImage, 0, fTotalSize);
	memcpy( fImage, bm->fImage, bm->GetLevelSize( 0 ) );
#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaDetailMapConstructor );
#endif
	plProfile_NewMem(MemMipmaps, fTotalSize);

	/// Filter levels!
	plFilterMask mask(sig);

	/// First, fill in all the mipmap levels sans detail levels
	for( i = 1; i < fNumLevels; i++ )
		ICreateLevelNoDetail(i, mask);

    if (createFlags & kCreateDetailMask) 
	{
		// Fill in the detail levels afterwards, so we can just grab the current level's texture
		// alpha (old way did it at the same time, which accumulated the detail alpha each level down)
        detailDropoffStart *= fNumLevels;
        detailDropoffStop *= fNumLevels;

		switch( createFlags & kCreateDetailMask )
		{
			case kCreateDetailAlpha:
	            fFlags |= kAlphaChannelFlag;
				for( i = 0; i < fNumLevels; i++ )
					IBlendLevelDetailAlpha(i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
				break;

			case kCreateDetailAdd:
				for( i = 0; i < fNumLevels; i++ )
					IBlendLevelDetailAdd( i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
				break;

			case kCreateDetailMult:
				for( i = 0; i < fNumLevels; i++ )
					IBlendLevelDetailMult(i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
				break;
		}
    }
	if( createFlags & kCreateCarryAlpha )
	{
		for( i = 1; i < fNumLevels; i++ )
			ICarryZeroAlpha(i);
	}
	if( createFlags & (kCreateCarryWhite | kCreateCarryBlack) )
	{
		UInt32 col = createFlags & kCreateCarryWhite ? 0x00ffffff : 0x00000000;
		for( i = 1; i < fNumLevels; i++ )
			ICarryColor(i, col);
	}


#ifdef MEMORY_LEAK_TRACER
	fNumMipmaps++;
#endif
}

//// IGetDetailLevelAlpha /////////////////////////////////////////////////////
//	Given the detail range and the current level, returns the detail's alpha
//	value at that level.

hsScalar plMipmap::IGetDetailLevelAlpha( UInt8 level, hsScalar dropStart, hsScalar dropStop, hsScalar min, hsScalar max )
{
	hsScalar detailAlpha;


	detailAlpha = ( level - dropStart ) * ( min - max ) / ( dropStop - dropStart ) + max;

	if( min < max )
		detailAlpha = hsMinimum( max, hsMaximum( min, detailAlpha ) );
	else
		detailAlpha = hsMinimum( min, hsMaximum( max, detailAlpha ) );

	return detailAlpha;
}

void plMipmap::SetBitmapAsLevel(UInt8 iDst, plMipmap *bm, hsScalar sig, UInt32 createFlags, 
                                      hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
                                      hsScalar detailMax, hsScalar detailMin)
{
    SetCurrLevel( iDst );

    hsAssert((bm->fHeight == fCurrLevelHeight) && (bm->fWidth == fCurrLevelWidth), "Wrong size bitmap for Mipmap level.");

    memcpy( fCurrLevelPtr, bm->fImage, bm->GetLevelSize( 0 ) );
	plFilterMask mask(sig);
	int i;

	// Fill in levels w/o detail first
	for( i = iDst+1; i < fNumLevels; i++ )
		ICreateLevelNoDetail(i, mask);

	// Now fill in the detail alphas, if we have any
	switch( createFlags & kCreateDetailMask )
	{
		case 0:
			break;

		case kCreateDetailAlpha:
			for( i = iDst; i < fNumLevels; i++ )
				IBlendLevelDetailAlpha(i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
			break;

		case kCreateDetailAdd:
			for( i = iDst; i < fNumLevels; i++ )
				IBlendLevelDetailAdd(i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
			break;

		case kCreateDetailMult:
			for( i = iDst; i < fNumLevels; i++ )
				IBlendLevelDetailMult(i, mask, detailDropoffStart, detailDropoffStop, detailMax, detailMin);
			break;
	}
	if( createFlags & kCreateCarryAlpha )
	{
		for( i = iDst+1; i < fNumLevels; i++ )
			ICarryZeroAlpha(i);
	}
	if( createFlags & (kCreateCarryWhite | kCreateCarryBlack) )
	{
		UInt32 col = createFlags & kCreateCarryWhite ? 0x00ffffff : 0x00000000;
		for( i = iDst+1; i < fNumLevels; i++ )
			ICarryColor(i, col);
	}

	SetCurrLevel(0);
}

//// ICreateLevelNoDetail /////////////////////////////////////////////////////
//	Creates one level of a mipmap based on a filter on the previous layer.
//	This version assumes no detail map fading.

void	plMipmap::ICreateLevelNoDetail( UInt8 iDst, const plFilterMask& mask )
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int i, j, ii, jj;

	if( 32 == fPixelSize )
	{
		SetCurrLevel(iDst);

		UInt8 *src = (UInt8 *)GetLevelPtr( iDst-1 );
		UInt8 *dst = (UInt8 *)GetLevelPtr(iDst);

		UInt32 srcRowBytes = fCurrLevelRowBytes << 1;
		UInt32 srcHeight = fCurrLevelHeight << 1;
		UInt32 srcWidth = fCurrLevelWidth << 1;

		for( i = 0; i < fCurrLevelHeight; i++ )
		{
			for( j = 0; j < fCurrLevelWidth; j++ )
			{
				UInt8 *center = src + (i << 1) * srcRowBytes + (j << 3);

				UInt32 chan;
				for( chan = 0; chan < 4; chan++ )
				{
					hsScalar w = 0;
					hsScalar a = 0;

					for( ii = mask.Begin(); ii <= mask.End(); ii++ )
					{
						for( jj = mask.Begin(); jj <= mask.End(); jj++ )
						{
							if( (ii + (i << 1) >= 0)&&(ii + (i << 1) < srcHeight)
							  &&(jj + (j << 1) >= 0)&&(jj + (j << 1) < srcWidth) )
							{
								w += mask.Mask(ii, jj);
								a += (hsScalar(center[ii*srcRowBytes + (jj<<2) + chan]) + 0.5f) * mask.Mask(ii, jj);
							}
						}
					}
					a /= w;

					dst[i * fCurrLevelRowBytes + (j << 2) + chan] = (UInt8)a;
				}
			}
		}
	}
}

void plMipmap::ICarryZeroAlpha(UInt8 iDst)
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int i, j;

	if( 32 == fPixelSize )
	{
		SetCurrLevel(iDst);

		UInt8 *src = (UInt8 *)GetLevelPtr( iDst-1 );
		UInt8 *dst = (UInt8 *)GetLevelPtr(iDst);

		UInt32 srcRowBytes = fCurrLevelRowBytes << 1;
		UInt32 srcHeight = fCurrLevelHeight << 1;
		UInt32 srcWidth = fCurrLevelWidth << 1;

		const UInt8 alphaOff = 3;
		for( i = 0; i < fCurrLevelHeight; i++ )
		{
			for( j = 0; j < fCurrLevelWidth; j++ )
			{
				UInt8 *center = src + (i << 1) * srcRowBytes + (j << 3) + alphaOff;

				if( !center[0]
					|| !center[4]
					|| !center[srcRowBytes + 0]
					|| !center[srcRowBytes + 4] )
				{
					dst[i * fCurrLevelRowBytes + (j << 2) + alphaOff] = 0;
				}
			}
		}
	}
}

void plMipmap::ICarryColor(UInt8 iDst, UInt32 col)
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int i, j;

	if( 32 == fPixelSize )
	{
		SetCurrLevel(iDst);

		UInt32* src = (UInt32*)GetLevelPtr( iDst-1 );
		UInt32* dst = (UInt32*)GetLevelPtr(iDst);

		UInt32 srcHeight = fCurrLevelHeight << 1;
		UInt32 srcWidth = fCurrLevelWidth << 1;

		const UInt8 alphaOff = 3;
		for( i = 0; i < fCurrLevelHeight; i++ )
		{
			for( j = 0; j < fCurrLevelWidth; j++ )
			{
				UInt32* center = src + (i << 1) * srcWidth + (j << 1);

				if( ((center[0] & 0x00ffffff) == col)
					||((center[1] & 0x00ffffff) == col)
					||((center[srcHeight] & 0x00ffffff) == col)
					||((center[srcHeight+1] & 0x00ffffff) == col) )
				{
					dst[i * fCurrLevelWidth + j] &= 0xff000000;
					dst[i * fCurrLevelWidth + j] |= col;
				}
			}
		}
	}
}

//// IBlendLevelDetailAlpha ///////////////////////////////////////////////////
//	Blends in the detail alpha for a given level. This version assumes 
//	standard detail map blending.

void	plMipmap::IBlendLevelDetailAlpha( UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin )
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int		i, j;
	UInt32	offset;


	SetCurrLevel(iDst);

	UInt8 *dst = (UInt8 *)GetLevelPtr(iDst);

	hsScalar detailAlpha = IGetDetailLevelAlpha( iDst, detailDropoffStart, detailDropoffStop, detailMin, detailMax );

	for( i = 0; i < fCurrLevelHeight; i++ )
	{
		for( j = 0; j < fCurrLevelWidth; j++ )
		{
			UInt32 chan = 3;	// Alpha channel only
			offset = i * fCurrLevelRowBytes + (j << 2) + chan;

			float a = (float)dst[ offset ] * detailAlpha;
			dst[ offset ] = (UInt8)a;
		}
	}
}

//// IBlendLevelDetailAdd /////////////////////////////////////////////////////
//	Blends in the detail alpha for a given level. This version assumes additive 
//	detail map blending. (Shesh, gotta hate C sometimes....)

void	plMipmap::IBlendLevelDetailAdd( UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin )
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int		i, j;
	UInt32	offset;


	SetCurrLevel(iDst);

	UInt8 *dst = (UInt8 *)GetLevelPtr(iDst);

	hsScalar detailAlpha = IGetDetailLevelAlpha( iDst, detailDropoffStart, detailDropoffStop, detailMin, detailMax );

	for( i = 0; i < fCurrLevelHeight; i++ )
	{
		for( j = 0; j < fCurrLevelWidth; j++ )
		{
			UInt32 chan;

			/// Blend all but the alpha channel, since we're doing additive blending
			for( chan = 0; chan < 3; chan++ )
			{
				offset = i * fCurrLevelRowBytes + (j << 2) + chan;

				float a = (float)dst[ offset ] * detailAlpha;
				dst[ offset ] = (UInt8)a;
			}
		}
	}
}

//// IBlendLevelDetailMult ////////////////////////////////////////////////////
//	Blends in the detail alpha for a given level. This version assumes 
//	multiplicitive detail map blending. (Shesh, gotta hate C sometimes....)

void	plMipmap::IBlendLevelDetailMult( UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin )
{
	hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int		i, j;
	UInt32	offset;


	SetCurrLevel(iDst);

	UInt8 *dst = (UInt8 *)GetLevelPtr(iDst);

	hsScalar	detailAlpha = IGetDetailLevelAlpha( iDst, detailDropoffStart, detailDropoffStop, detailMin, detailMax );
	hsScalar	invDetailAlpha = ( 1.f - detailAlpha ) * 255.f;

	for( i = 0; i < fCurrLevelHeight; i++ )
	{
		for( j = 0; j < fCurrLevelWidth; j++ )
		{
			UInt32 chan;

			for( chan = 0; chan < 4; chan++ )
			{
				offset = i * fCurrLevelRowBytes + (j << 2) + chan;

				float a = (float)dst[ offset ];

				// Mult should fade to white, not black like with additive blending
				a = invDetailAlpha + a * detailAlpha;

				dst[ offset ] = (UInt8)a;
			}
		}
	}
}

//// EnsureKonstantBorder /////////////////////////////////////////////////////
//	Checks a mipmap's levels and makes sure that if the top level has constant
//	border color/alpha, the rest of the levels get that border, too, regardless
//	of filtering. (This is us guessing that that border was apparently what
//	was intented, thus removing ugly stretching problems on clamped textures).

void	plMipmap::EnsureKonstantBorder( hsBool clampU, hsBool clampV )
{
	if( fPixelSize != 32 )
	{
		hsAssert( false, "Only 32 bit color supported in EnsureKonstantBorder()" );
		return;
	}

	if( !clampU && !clampV )
		return;		// Um, exactly what are we supposed to do, again?

	UInt32	uColor, vColor;
	int		i;


	if( clampU && !IGrabBorderColor( false, &uColor ) )
		return;
	if( clampV && !IGrabBorderColor( true, &vColor ) )
		return;

	if( clampU && clampV && ( uColor != vColor ) )
		return;

	for( i = 1; i < fNumLevels; i++ )
	{
		SetCurrLevel( i );
		if( clampU )
			ISetCurrLevelUBorder( uColor );
		if( clampV )
			ISetCurrLevelVBorder( vColor );
	}

	SetCurrLevel( 0 );
}

//// IGrabBorderColor /////////////////////////////////////////////////////////
//	Grabs the top level's border color, or returns false if not all pixels
//	on the border are the same color/alpha.

hsBool	plMipmap::IGrabBorderColor( hsBool grabVNotU, UInt32 *color )
{
	int			i;
	UInt32		*src1 = (UInt32 *)fImage, *src2, testColor;


	if( !grabVNotU )
	{
		src2 = (UInt32 *)( (UInt8 *)fImage + fRowBytes * ( fHeight - 1 ) );

		testColor = *src1;
		for( i = 0; i < fWidth; i++ )
		{
			if( src1[ i ] != testColor || src2[ i ] != testColor )
				return false;
		}

		*color = testColor;
		return true;
	}
	else
	{
		src2 = src1 + ( fWidth - 1 );

		testColor = *src1;
		for( i = 0; i < fHeight; i++ )
		{
			if( *src1 != testColor || *src2 != testColor )
				return false;
			src1 += fWidth;
			src2 += fWidth;
		}

		*color = testColor;
		return true;
	}
}

//// ISetCurrLevelUBorder /////////////////////////////////////////////////////

void	plMipmap::ISetCurrLevelUBorder( UInt32 color )
{
	int		i;
	UInt32	*src1 = (UInt32 *)fCurrLevelPtr, *src2;


	src2 = (UInt32 *)( (UInt8 *)fCurrLevelPtr + fCurrLevelRowBytes * ( fCurrLevelHeight - 1 ) );

	for( i = 0; i < fCurrLevelWidth; i++ )
	{
		src1[ i ] = color;
		src2[ i ] = color;
	}
}

//// ISetCurrLevelVBorder /////////////////////////////////////////////////////

void	plMipmap::ISetCurrLevelVBorder( UInt32 color )
{
	int		i;
	UInt32	*src1 = (UInt32 *)fCurrLevelPtr, *src2;


	src2 = src1 + ( fCurrLevelWidth - 1 );

	for( i = 0; i < fCurrLevelHeight; i++ )
	{
		*src1 = color;
		*src2 = color;
		src1 += fCurrLevelWidth;
		src2 += fCurrLevelWidth;
	}
}

void plMipmap::Filter(hsScalar sig)
{
    hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int i, j, ii, jj;

	if( 32 == fPixelSize )
	{
		UInt8 *dst = (UInt8 *)(fImage);

		UInt8* src = (UInt8*)HSMemory::New(fRowBytes * fHeight);
		HSMemory::BlockMove(dst, src, fRowBytes * fHeight);

		if( sig <= 0 )
			sig = kDefaultSigma;

		plFilterMask mask(sig);

		UInt32 srcRowBytes = fRowBytes;
		UInt32 srcHeight = fHeight;
		UInt32 srcWidth = fWidth;

		for( i = 0; i < fHeight; i++ )
		{
			for( j = 0; j < fWidth; j++ )
			{
				UInt8 *center = src + i * srcRowBytes + (j << 2);

				UInt32 chan;
				for( chan = 0; chan < 4; chan++ )
				{
					hsScalar w = 0;
					hsScalar a = 0;

					for( ii = mask.Begin(); ii <= mask.End(); ii++ )
					{
						for( jj = mask.Begin(); jj <= mask.End(); jj++ )
						{
							if( (ii + i >= 0)&&(ii + i < srcHeight)
							  &&(jj + j >= 0)&&(jj + j < srcWidth) )
							{
								w += mask.Mask(ii, jj);
								a += (hsScalar(center[ii*srcRowBytes + (jj<<2) + chan]) + 0.5f) * mask.Mask(ii, jj);
							}
						}
					}
					a /= w;

					dst[i * fRowBytes + (j << 2) + chan] = (UInt8)(a);
				}
			}
		}

		HSMemory::Delete(src);
	}
}

static void CopyPixels(UInt32 srcWidth, UInt32 srcHeight,void *srcPixels,
					UInt32 skipX, UInt32 skipY, UInt32 dstFormat,
					void * &destPixels, UInt32 copyOptions)
{
	int y;
	int xInc = skipX + 1;
	int yInc = skipY + 1;
	int i = 0; 
	int firstX = 0;

	int rowSkip = yInc * srcWidth; // Number of pixels to skip for each line

	const hsRGBAColor32 *p =(const hsRGBAColor32 *)srcPixels;
	UInt16 *pixels16 = (UInt16*)destPixels;
	UInt8 *pixels8 = (UInt8*)destPixels;

	for(y = 0; y < srcHeight; y += yInc, firstX += rowSkip)
	{
		const hsRGBAColor32 *srcPix = &(p[firstX]);
		int x;
		switch (dstFormat)
		{
			case plMipmap::kPixelAI88:
					for(x =0; x < srcWidth; x += xInc)
						pixels16[i++]= ((srcPix[x].a & 0xff) << 8) | (srcPix[x].r & 0xff);
			break;
			case plMipmap::kPixelI8:
				for(x =0; x < srcWidth; x += xInc)
					pixels8[i++]= srcPix[x].r;
			break;
			case plMipmap::kPixelARGB4444:
				for(x = 0; x < srcWidth; x += xInc)
					pixels16[i++]= (((srcPix[x].r>>4) & 0xf) << 8) 
						| (((srcPix[x].g >> 4) & 0xf) << 4) 
						| (((srcPix[x].b >> 4) & 0xf) )
						| (((srcPix[x].a >> 4) & 0xf) << 12);
			break;
			case plMipmap::kPixelARGB1555:
				for(x = 0; x < srcWidth; x += xInc)
					pixels16[i++]= (((srcPix[x].r>>3) & 0x1f) << 10) | 
						(((srcPix[x].g >> 3) & 0x1f) << 5) |
						((srcPix[x].b >> 3) & 0x1f) | ((srcPix[x].a == 0) ? 0 : 0x8000);
			break;
		}
	}
	destPixels = (char *)destPixels + (i * ((dstFormat == plMipmap::kPixelI8 ) ? 1 : 2));
}


UInt32 plMipmap::CopyOutPixels(UInt32 destXSize, UInt32 destYSize, 
					UInt32 dstFormat, void *destPixels, UInt32 copyOptions)
{

	hsAssert(fPixelSize == 32, "Only 32 bit implemented");
	ASSERT_UNCOMPRESSED();

	int i;

	int skipX = fWidth/destXSize - 1;
	int skipY = fHeight/destYSize - 1;

	hsAssert(!fCurrLevel,"Mip Map not at level 0");

	for(i = 0 ; i < (fNumLevels - fCurrLevel); i++)
	{
		CopyPixels(fWidth >> i , fHeight >> i, GetLevelPtr( i ), skipX, skipY,
						dstFormat, destPixels, copyOptions);
	}

	return 0;
}

//// CopyFrom /////////////////////////////////////////////////////////////////

void	plMipmap::CopyFrom( const plMipmap *source )
{
	hsAssert( source != nil, "nil source in plMipmap::CopyFrom()" );

	plProfile_DelMem(MemMipmaps, fTotalSize);
#ifdef MEMORY_LEAK_TRACER
	IRemoveFromMemRecord( (UInt8 *)fImage );
#endif
	delete [] fImage;

	fWidth = source->fWidth;
	fHeight = source->fHeight;
	fRowBytes = source->fRowBytes;
	fPixelSize = source->fPixelSize;
	fFlags = source->fFlags;
	fSpace = source->fSpace;
	fCompressionType = source->fCompressionType;
	fTotalSize = source->fTotalSize;

	fImage = (void *)TRACKED_NEW UInt8[ fTotalSize ];
	memcpy( fImage, source->fImage, fTotalSize );
#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaCopyFrom );
#endif
	plProfile_NewMem(MemMipmaps, fTotalSize);
	
	fNumLevels = source->fNumLevels;

	switch( fCompressionType )
	{
		case kDirectXCompression:
			{
				fDirectXInfo.fBlockSize = source->fDirectXInfo.fBlockSize;
				fDirectXInfo.fCompressionType = source->fDirectXInfo.fCompressionType;
			}
			break;
		case kUncompressed:
		case kJPEGCompression:
			fUncompressedInfo.fType = source->fUncompressedInfo.fType;
			break;
		default:
			hsAssert(false, "Reading unknown compression format.");
			break;
	}

	// Gotta do this AFTER we set our block size, etc.
	IBuildLevelSizes();
	
	// We just changed our texture, so if we have a texture ref, we better dirty it
	if( GetDeviceRef() != nil )
		GetDeviceRef()->SetDirty( true );
}

//// Clone ////////////////////////////////////////////////////////////////////

plMipmap	*plMipmap::Clone() const
{
	plMipmap *newMap = TRACKED_NEW plMipmap;

	newMap->CopyFrom( this );

	return newMap;
}

//// Composite ////////////////////////////////////////////////////////////////
//	Compositing function. Take a (smaller) mipmap and composite it onto this one 
//	at the given location

void	plMipmap::Composite( plMipmap *source, UInt16 x, UInt16 y, plMipmap::CompositeOptions *options )
{
	UInt8	level, numLevels, srcNumLevels, srcLevelOffset, levelsToSkip;
	UInt16	pX, pY;
	UInt32	*srcLevelPtr, *dstLevelPtr, *srcPtr, *dstPtr;
	UInt32	srcRowBytes, dstRowBytes, srcRowBytesToCopy, r, g, b, dR, dG, dB, srcWidth, srcHeight;
	UInt32	srcAlpha, oneMinusAlpha, destAlpha;
	UInt16	srcClipX, srcClipY;


	// Currently we only support 32 bit uncompressed mipmaps
	if( fPixelSize != 32 || fCompressionType == kDirectXCompression )
	{
		hsAssert( false, "Destination mipmap on composite has unsupported format" );
		return;
	}
	if( source->fPixelSize != 32 || source->fCompressionType == kDirectXCompression )
	{
		hsAssert( false, "Source mipmap on composite has unsupported format" );
		return;
	}

	// Grab the correct options pointer
	if( options == nil )
	{
		static CompositeOptions		defaultOptions;
		options = &defaultOptions;
	}

	// Src level skipping
	srcWidth = source->fWidth;
	srcHeight = source->fHeight;
	srcLevelPtr = (UInt32 *)source->fImage;
	srcRowBytes = source->fRowBytes;
	srcNumLevels = source->fNumLevels;
	for( srcLevelOffset = 0, levelsToSkip = options->fSrcLevelsToSkip; levelsToSkip > 0; levelsToSkip--, srcLevelOffset++ )
	{
		srcWidth >>= 1;
		srcHeight >>= 1;
		srcLevelPtr += source->fLevelSizes[ srcLevelOffset ] >> 2;
		srcRowBytes >>= 1;
		srcNumLevels--;
	}

	// Src clipping setup
	srcClipX = srcClipY = 0;
	srcRowBytesToCopy = srcRowBytes;
	if( options->fSrcClipY > 0 )
	{
		srcHeight -= options->fSrcClipY;
		srcClipY = options->fSrcClipY;
	}
	if( options->fSrcClipHeight > 0 )
	{
		srcHeight = options->fSrcClipHeight;
	}
	if( options->fSrcClipX > 0 )
	{
		srcWidth -= options->fSrcClipX;
		srcClipX = options->fSrcClipX;
		srcRowBytesToCopy -= options->fSrcClipX * ( source->fPixelSize >> 3 );
	}
	if( options->fSrcClipWidth > 0 )
	{
		srcWidth = options->fSrcClipWidth;
		srcRowBytesToCopy = srcWidth * ( source->fPixelSize >> 3 );
	}

	// Position checks
	if( x + srcWidth > fWidth || y + srcHeight > fHeight )
	{
		hsAssert( false, "Illegal position on mipmap composite" );
		return;
	}

	// Do the composite on each level
	numLevels = fNumLevels;
	if( numLevels > srcNumLevels )
		numLevels = srcNumLevels;

	dstLevelPtr = (UInt32 *)fImage;
	dstRowBytes = fRowBytes;

	if( options->fFlags & kForceOpaque )
	{
		for( level = 0; level < numLevels; level++, y >>= 1, x >>= 1 )
		{
			srcPtr = srcLevelPtr;
			dstPtr = dstLevelPtr + ( y * dstRowBytes >> 2 ) + x;

			// Clipping
			srcPtr += srcClipY * ( srcRowBytes >> 2 ) + srcClipX;

			for( pY = (UInt16)srcHeight; pY > 0; pY-- )
			{
				memcpy( dstPtr, srcPtr, srcRowBytesToCopy );
				for( pX = 0; pX < srcWidth; pX++ )
				{
					// Force the alpha opaque
					dstPtr[ pX ] |= 0xff000000;
				}
				dstPtr += dstRowBytes >> 2;
				srcPtr += srcRowBytes >> 2;
			}
		
			srcLevelPtr += source->fLevelSizes[ level + srcLevelOffset ] >> 2;
			dstLevelPtr += fLevelSizes[ level ] >> 2;
			srcRowBytes >>= 1;
			dstRowBytes >>= 1;
			srcRowBytesToCopy >>= 1;
			if( srcHeight > 1 )
				srcHeight >>= 1;
			srcClipX >>= 1;
			srcClipY >>= 1;
		}	
	}
	else if( options->fFlags & kCopySrcAlpha )
	{
		for( level = 0; level < numLevels; level++, y >>= 1, x >>= 1 )
		{
			srcPtr = srcLevelPtr;
			dstPtr = dstLevelPtr + ( y * dstRowBytes >> 2 ) + x;

			// Clipping
			srcPtr += srcClipY * ( srcRowBytes >> 2 ) + srcClipX;

			for( pY = (UInt16)srcHeight; pY > 0; pY-- )
			{
				memcpy( dstPtr, srcPtr, srcRowBytesToCopy );
				dstPtr += dstRowBytes >> 2;
				srcPtr += srcRowBytes >> 2;
			}
		
			srcLevelPtr += source->fLevelSizes[ level + srcLevelOffset ] >> 2;
			dstLevelPtr += fLevelSizes[ level ] >> 2;
			srcRowBytes >>= 1;
			dstRowBytes >>= 1;
			srcRowBytesToCopy >>= 1;
			if( srcHeight > 1 )
				srcHeight >>= 1;
			srcClipX >>= 1;
			srcClipY >>= 1;
		}	
	}
	else if( options->fFlags & kMaskSrcAlpha )
	{
		for( level = 0; level < numLevels; level++, y >>= 1, x >>= 1 )
		{
			srcPtr = srcLevelPtr;
			dstPtr = dstLevelPtr + ( y * dstRowBytes >> 2 ) + x;

			// Clipping
			srcPtr += srcClipY * ( srcRowBytes >> 2 ) + srcClipX;

			for( pY = (UInt16)srcHeight; pY > 0; pY-- )
			{
				for( pX = 0; pX < srcWidth; pX++ )
				{
					srcAlpha = options->fOpacity * ( ( srcPtr[ pX ] >> 16 ) & 0x0000ff00 ) / 255 / 256;
					if( srcAlpha != 0 )
						dstPtr[ pX ] = ( srcPtr[ pX ] & 0x00ffffff ) | ( srcAlpha << 24 );
				}
				dstPtr += dstRowBytes >> 2;
				srcPtr += srcRowBytes >> 2;
			}
		
			srcLevelPtr += source->fLevelSizes[ level + srcLevelOffset ] >> 2;
			dstLevelPtr += fLevelSizes[ level ] >> 2;
			srcRowBytes >>= 1;
			dstRowBytes >>= 1;
			srcRowBytesToCopy >>= 1;
			if( srcHeight > 1 )
				srcHeight >>= 1;
			srcClipX >>= 1;
			srcClipY >>= 1;
		}	
	}
	else
	{
		for( level = 0; level < numLevels; level++, y >>= 1, x >>= 1 )
		{
			srcPtr = srcLevelPtr;
			dstPtr = dstLevelPtr + ( y * dstRowBytes >> 2 ) + x;

			// Clipping
			srcPtr += srcClipY * ( srcRowBytes >> 2 ) + srcClipX;

			for( pY = (UInt16)srcHeight; pY > 0; pY-- )			
			{
				// Reverse the loop so we can count downwards--slightly faster
				pX = (UInt16)srcWidth;
				do
				{
					pX--;

					// Wacko trick here. Alphas are 0-255, which means scaling by alpha would
					// be a v' = v * alpha / 255 operation sequence. However, since we hate
					// dividing by 255 all the time, we actually scale the alpha just ever so
					// slightly so it's 0-256, which makes the divide a simple shift. Note
					// that this will result in some tiny bit of aliasing, but it shouldn't be
					// enough to notice
					
					if (!(srcPtr[pX] >> 24)) // Zero alpha. Skip this pixel
						continue;

					srcAlpha = options->fOpacity * ( ( srcPtr[ pX ] >> 16 ) & 0x0000ff00 ) / 255 / 256;
					oneMinusAlpha = 256 - srcAlpha;
					destAlpha = dstPtr[ pX ] & 0xff000000;

					r = (UInt32)((( srcPtr[ pX ] >> 16 ) & 0x000000ff) * options->fRedTint);
					g = (UInt32)((( srcPtr[ pX ] >> 8  ) & 0x000000ff) * options->fGreenTint);
					b = (UInt32)((( srcPtr[ pX ]       ) & 0x000000ff) * options->fBlueTint);
					dR = ( dstPtr[ pX ] >> 16 ) & 0x000000ff;
					dG = ( dstPtr[ pX ] >> 8  ) & 0x000000ff;
					dB = ( dstPtr[ pX ]       ) & 0x000000ff;
					r = ( r * srcAlpha ) >> 8;
					g = ( g * srcAlpha ) >> 8;
					b = ( b * srcAlpha ) >> 8;
					dR = ( dR * oneMinusAlpha ) >> 8;
					dG = ( dG * oneMinusAlpha ) >> 8;
					dB = ( dB * oneMinusAlpha ) >> 8;

					// Dest alpha for now is just our original dest alpha
					dstPtr[ pX ] = ( ( r + dR ) << 16 ) | ( ( g + dG ) << 8 ) | ( b + dB ) | destAlpha;

					if( !( options->fFlags & kBlendWriteAlpha ) )
						continue;

					// Unless our blend option is set of course
					dstPtr[ pX ] = ( dstPtr[ pX ] & 0x00ffffff ) | ( srcAlpha << 24 );

				} while( pX > 0 );

				dstPtr += dstRowBytes >> 2;
				srcPtr += srcRowBytes >> 2;
			}
		
			srcLevelPtr += source->fLevelSizes[ level + srcLevelOffset ] >> 2;
			dstLevelPtr += fLevelSizes[ level ] >> 2;
			srcRowBytes >>= 1;
			dstRowBytes >>= 1;
			if( srcWidth > 1 )
				srcWidth >>= 1;
			if( srcHeight > 1 )
				srcHeight >>= 1;
			srcClipX >>= 1;
			srcClipY >>= 1;
		}	
	}

	// All done!
	if( GetDeviceRef() != nil )
		GetDeviceRef()->SetDirty( true );
}

//// Colorize /////////////////////////////////////////////////////////////////
//	Colorizes a mipmap so that each level is color-coded. Assume max 10 levels
//	(coloring wraps after 10).

void	plMipmap::Colorize()
{
	UInt32		currColor, width, height;
	UInt8		currLevel;


	if( fPixelSize != 32 && fCompressionType != kDirectXCompression )
	{
		/// Most likely this is a luminance or luminance/alpha map,
		/// so we ignore it (it's up to the device to make sure we
		/// only get 32-bit or compressed mipmaps)
		return;
	}

	if( fFlags & kForceOneMipLevel )
	{
		// Don't colorize if it's not mipmapped...
		return;
	}

	/// First handle compressed levels, if any
	currLevel = 0;
	currColor = 0;
	width = fWidth;
	height = fHeight;
	if( fCompressionType == kDirectXCompression )
	{
		for( ; currLevel < fNumLevels; currLevel++ )
		{
			/// Are we over the threshold?
			SetCurrLevel( currLevel );
			if( ( fCurrLevelWidth | fCurrLevelHeight ) & 0x03 )
				break;

			/// Since this level is compressed, we have to use the codec...
			hsCodecManager::Instance().ColorizeCompMipmap( this, fColorMasks[ currColor ] );

			/// Increment the color
			currColor = ( currColor >=9 ) ? 0 : currColor + 1;
		}
	}

	/// Now loop through the uncompressed levels
	for( ; currLevel < fNumLevels; currLevel++ )
	{
		/// Do this one
		IColorLevel( currLevel, fColorMasks[ currColor ] );

		/// Increment the color
		currColor = ( currColor >=9 ) ? 0 : currColor + 1;
	}

	SetCurrLevel( 0 );
}

//// IColorLevel //////////////////////////////////////////////////////////////
//	Colorizes the current level of a mipmap according to the color mask given
//	(percentages of r, g & b in the range of 0-2).

void	plMipmap::IColorLevel( UInt8 level, const UInt8 *colorMask )
{	
	UInt32		index, max, color, gray, grayDiv2, *data, width, height;
	UInt8		compMasks[ 3 ][ 2 ] = { { 0, 0 }, { 0, 0xff }, { 0xff, 0 } };
	

	data = (UInt32 *)GetLevelPtr( level, &width, &height );
	max = fLevelSizes[ level ] >> 2;

	for( index = 0; index < max; index++ )
	{
		/// Get color and calculate gray (average of r, g & b)
		color = data[ index ];
		gray = ( ( color >> 16 ) & 0xff ) + ( ( color >> 8 ) & 0xff ) + ( color & 0xff );
		gray /= 3;
		gray = 0xff - ( ( 0xff - gray ) >> 1 );		// Lighten it 50%
		grayDiv2 = gray >> 1;

		/// Preserve alpha
		color &= 0xff000000;

		/// Now rewrite the components based on the color mask
		color |= ( ( gray & compMasks[ colorMask[ 0 ] ][ 0 ] ) |
			   ( grayDiv2 & compMasks[ colorMask[ 0 ] ][ 1 ] ) ) << 16;
		color |= ( ( gray & compMasks[ colorMask[ 1 ] ][ 0 ] ) |
			   ( grayDiv2 & compMasks[ colorMask[ 1 ] ][ 1 ] ) ) << 8;
		color |= ( ( gray & compMasks[ colorMask[ 2 ] ][ 0 ] ) |
			   ( grayDiv2 & compMasks[ colorMask[ 2 ] ][ 1 ] ) );

		data[ index ] = color;
	}
}

///////////////////////////////////////////////////////////////////////////////
//// Scaling //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// ScaleNicely //////////////////////////////////////////////////////////////
//	Does a nice (smoothed) scaling of a 1-level mipmap onto another 1-level 
//	mipmap. Works only for 32-bit mipmaps.

void	plMipmap::ScaleNicely( UInt32 *destPtr, UInt16 destWidth, UInt16 destHeight,
								UInt16 destStride, plMipmap::ScaleFilter filter ) const
{
	UInt16		destX, destY, srcX, srcY;
	Int16		srcStartX, srcEndX, srcStartY, srcEndY;
	float		srcPosX, srcPosY, destToSrcXScale, destToSrcYScale, filterWidth, filterHeight, weight;
	float		totalWeight;
	hsColorRGBA	color, accumColor;
	float		whyWaits[ 16 ], whyWait, xWeights[ 16 ];
	UInt32		*srcPtr;


	// Init
	destToSrcXScale = (float)fWidth / (float)destWidth;
	destToSrcYScale = (float)fHeight / (float)destHeight;

	// Filter size is the radius of the area (or rather, half the box size) around the source position 
	// that we sample from. We calculate it so that a 1:1 scale would result in a filter size of 1 (thus 
	// making a box filter at 1:1 result in a straight copy of the original)
	filterWidth = 1.f * destToSrcXScale;
	filterHeight = 1.f * destToSrcYScale;

	// If we are upsampling, we still want a filter at least a pixel half-width/height, which will just do
	// a bilerp up. That doesn't make this function correctly resample, or excuse the incredibly complicated
	// code to do something incredibly simple, but at least it doesn't fail so obviously.
	if( filterWidth < 1.f )
		filterWidth = 1.f;
	if( filterHeight < 1.f )
		filterHeight = 1.f;

	// Process
	for( destY = 0; destY < destHeight; destY++ )
	{
		// Calculate the span across this row
		srcPosY = destY * destToSrcYScale;

		srcStartY = (Int16)( srcPosY - filterHeight );
		if( srcStartY < 0 ) 
			srcStartY = 0;

		srcEndY = (Int16)( srcPosY + filterHeight );
		if( srcEndY >= fHeight ) 
			srcEndY = (Int16)(fHeight - 1);

		// Precalc the y weights
		for( srcY = srcStartY; srcY <= srcEndY && ( srcY - srcStartY ) < 16; srcY++ )
			whyWaits[ srcY - srcStartY ] = 1.f - ( fabs( (float)srcY - srcPosY ) / filterHeight );

		for( destX = 0; destX < destWidth; destX++ )
		{
			// For this pixel in the destination, figure out where in the source image we virtually are
			srcPosX = destX * destToSrcXScale;

			// Range of pixels that the filter covers
			srcStartX = (Int16)( srcPosX - filterWidth );
			if( srcStartX < 0 ) 
				srcStartX = 0;
			
			srcEndX = (Int16)( srcPosX + filterWidth );
			if( srcEndX >= fWidth ) 
				srcEndX = (Int16)(fWidth - 1);

			// Precalc the x weights
			for( srcX = srcStartX; srcX <= srcEndX && ( srcX - srcStartX ) < 16; srcX++ )
				xWeights[ srcX - srcStartX ] = 1.f - ( fabs( (float)srcX - srcPosX ) / filterWidth );

			// Sum up all the weighted colors in the filter area
			accumColor.Set( 0.f, 0.f, 0.f, 0.f );
			totalWeight = 0.f;
			for( srcY = srcStartY; srcY <= srcEndY; srcY++ )
			{
				if( srcY - srcStartY < 16 )
					whyWait = whyWaits[ srcY - srcStartY ];
				else
					whyWait = 1.f - ( fabs( (float)srcY - srcPosY ) / filterHeight );

				if( whyWait <= 0.f )
					continue;

				srcPtr = GetAddr32( srcStartX, srcY );
				for( srcX = srcStartX; srcX <= srcEndX; srcX++, srcPtr++ )
				{
					// Our weight...
					weight = ( srcX - srcStartX < 16 ) ? xWeights[ srcX - srcStartX ] : 
								( 1.f - ( fabs( (float)srcX - srcPosX ) / filterWidth ) );
					weight *= whyWait;

					if( weight > 0.f )
					{
						// Grab pixel values from us...
						color.FromARGB32( *srcPtr );
						color *= weight;
						accumColor += color;
						totalWeight += weight;
					}
				}
			}
			accumColor *= 1.f / totalWeight;

			// Set the final value
			*destPtr = accumColor.ToARGB32();
			destPtr++;
		}
		destPtr += destStride - destWidth;
	}
}

//// ResizeNicely /////////////////////////////////////////////////////////////
//	Resizes us using the ScaleNicely function. Only works for 1-level, 32bpp
//	uncompressed mipmaps.

hsBool	plMipmap::ResizeNicely( UInt16 newWidth, UInt16 newHeight, plMipmap::ScaleFilter filter )
{
	// Make a temp buffer
	UInt32	*newData = TRACKED_NEW UInt32[ newWidth * newHeight ];
	if( newData == nil )
		return false;

	// Scale to it
	ScaleNicely( newData, newWidth, newHeight, newWidth, filter );

	// Reset us to that
	Reset();
	fWidth = newWidth;
	fHeight = newHeight;
	fRowBytes = fWidth * fPixelSize >> 3;
	fTotalSize = fRowBytes * fWidth;
	fNumLevels = 1;
	IBuildLevelSizes();
	fImage = newData;
	SetCurrLevel( 0 );
	plProfile_NewMem(MemMipmaps, fTotalSize);

#ifdef MEMORY_LEAK_TRACER
	IAddToMemRecord( this, plRecord::kViaResize );
#endif

	// All done!
	return true;
}

#ifdef MEMORY_LEAK_TRACER
//// Debug Mipmap Memory Leak Tracker /////////////////////////////////////////

plMipmap::plRecord	*plMipmap::fRecords = nil;
UInt32				plMipmap::fNumMipmaps = 0;

void	plMipmap::IAddToMemRecord( plMipmap *mip, plRecord::Method method )
{
	plRecord	*newRecord = TRACKED_NEW plRecord;


	newRecord->fCompressionType = mip->fCompressionType;
	newRecord->fCreationMethod = method;
	newRecord->fHeight = mip->fHeight;
	newRecord->fWidth = mip->fWidth;
	newRecord->fImage = mip->fImage;
	newRecord->fNumLevels = mip->fNumLevels;
	newRecord->fRowBytes = mip->fRowBytes;
	if( mip->GetKey() )
		strcpy( newRecord->fKeyName, mip->GetKeyName() );
	else
		strcpy( newRecord->fKeyName, "<noKey>" );
	if( mip->fCompressionType != kDirectXCompression )
		newRecord->fUncompressedInfo.fType = mip->fUncompressedInfo.fType;
	else
	{
		newRecord->fDirectXInfo.fBlockSize = mip->fDirectXInfo.fBlockSize;
		newRecord->fDirectXInfo.fCompressionType = mip->fDirectXInfo.fCompressionType;
	}

	newRecord->Link( &fRecords );
}

void	plMipmap::IRemoveFromMemRecord( UInt8 *image )
{
	plRecord	*record;


	for( record = fRecords; record != nil; record = record->fNext )
	{
		if( record->fImage == image )
		{
			record->Unlink();
			delete record;
			return;
		}
	}
}

void	plMipmap::IReportLeaks()
{
	plRecord	*record, *next;
	static char	msg[ 512 ], m2[ 128 ];
	UInt32		size;


	hsStatusMessage( "--- plMipmap Leaks ---\n" );
	for( record = fRecords; record != nil;  )
	{
		size = record->fHeight * record->fRowBytes;
		if( size >= 1024 )
			sprintf( msg, "%s, %4.1f kB: \t%dx%d, %d levels, %d bpr", record->fKeyName, size / 1024.f, record->fWidth, record->fHeight, record->fNumLevels, record->fRowBytes );
		else
			sprintf( msg, "%s, %u bytes: \t%dx%d, %d levels, %d bpr", record->fKeyName, size, record->fWidth, record->fHeight, record->fNumLevels, record->fRowBytes );

		if( record->fCompressionType != kDirectXCompression )
			sprintf( m2, " UType: %d", record->fUncompressedInfo.fType );
		else
			sprintf( m2, " DXT%d BSz: %d", record->fDirectXInfo.fCompressionType, record->fDirectXInfo.fBlockSize );
		strcat( msg, m2 );

		switch( record->fCreationMethod )
		{
			case plRecord::kViaCreate: strcat( msg, " via Create\n" ); break;
			case plRecord::kViaRead: strcat( msg, " via Read\n" ); break;
			case plRecord::kViaClipToMaxSize: strcat( msg, " via ClipToMaxSize\n" ); break;
			case plRecord::kViaDetailMapConstructor: strcat( msg, " via DetailMapConstructor\n" ); break;
			case plRecord::kViaCopyFrom: strcat( msg, " via CopyFrom\n" ); break;
			case plRecord::kViaResize: strcat( msg, " via Resize\n" ); break;
		}

		hsStatusMessage( msg );

		next = record->fNext;
		record->Unlink();
		delete record;
		record = next;
	}
	hsStatusMessage( "--- End of plMipmap Leaks ---\n" );
}

#endif
