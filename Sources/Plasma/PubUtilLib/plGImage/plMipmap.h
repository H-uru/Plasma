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
//	plMipmap Class Header													 //
//	Derived bitmap class representing a single mipmap.						 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plMipmap_h
#define _plMipmap_h

#include "plBitmap.h"

#ifdef HS_DEBUGGING
	#define ASSERT_PIXELSIZE(bitmap, pixelsize)		hsAssert((bitmap)->fPixelSize == (pixelsize), "pixelSize mismatch")
	#define ASSERT_XY(bitmap, x, y)					hsAssert(x < (bitmap)->fWidth && y < (bitmap)->fHeight, "bad XY")
	#define ASSERT_UNCOMPRESSED()					hsAssert(fCompressionType!=kDirectXCompression, "Can't operate on compressed map.")

	// Define the following konstant to enable mipmap leak checking. This is because our normal
	// memory manager sucks when trying to track down these problems
	#define MEMORY_LEAK_TRACER
#else
	#define ASSERT_PIXELSIZE(bitmap, pixelsize)
	#define ASSERT_XY(bitmap, x, y)
	#define ASSERT_UNCOMPRESSED()				
#endif

//// Class Definition /////////////////////////////////////////////////////////

class plBitmapCreator;
class plTextGenerator;

class plMipmap : public plBitmap
{
	friend class plBitmapCreator;
	friend class plTextGenerator;

	public:
		//// Public Flags ////


		//// Public Data /////
		
		
		//// Public Members ////


		plMipmap();
		plMipmap( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels = 0, UInt8 compType = kUncompressed, UInt8 format = UncompressedInfo::kRGB8888 );
		plMipmap( plMipmap *bm, hsScalar sig, UInt32 createFlags, 
							hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
							hsScalar detailMax, hsScalar detailMin );
		virtual ~plMipmap();

		CLASSNAME_REGISTER( plMipmap );
		GETINTERFACE_ANY( plMipmap, plBitmap );


		void			Create( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels, UInt8 compType = kUncompressed, UInt8 format = UncompressedInfo::kRGB8888 );

		virtual void	Reset();

		// Get the total size in bytes
		virtual UInt32	GetTotalSize() const;

		virtual void	Read( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Read( s, mgr ); this->Read( s ); }
		virtual void	Write( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Write( s, mgr ); this->Write( s ); }

		virtual UInt8	GetNumLevels() const { return fNumLevels; }
		virtual UInt32	GetLevelSize( UInt8 level );		// 0 is the largest

		virtual void		Colorize();
		virtual plMipmap	*Clone() const;
		virtual void		CopyFrom( const plMipmap *source );

		inline UInt32	GetWidth() const { return fWidth; }
		inline UInt32	GetHeight() const { return fHeight; }
		inline UInt32	GetRowBytes() const { return fRowBytes; }

		void			*GetImage() const { return fImage; }
		void			SetImagePtr( void *ptr ) { fImage = ptr; }
		UInt8			*GetLevelPtr( UInt8 level, UInt32 *width = nil, UInt32 *height = nil, UInt32 *rowBytes = nil );

		// Sets the current level pointer for use with GetAddr*
		virtual void	SetCurrLevel(UInt8 level);
		void			*GetCurrLevelPtr() const { return fCurrLevelPtr; }
		UInt32			GetCurrWidth() const { return fCurrLevelWidth; }
		UInt32			GetCurrHeight() const { return fCurrLevelHeight; }
		UInt32			GetCurrLevelSize() const { return fLevelSizes[ fCurrLevel ]; }
		UInt32			GetCurrLevel() const { return fCurrLevel; }

		//	These methods return the address of the pixel specified by x and y
		//	They are meant to be fast, therefore they are inlined and do not check
		//	the fPixelSize field at runtime (except when debugging)

		UInt8*	GetAddr8(unsigned x, unsigned y) const
				{
					ASSERT_PIXELSIZE(this, 8);
					ASSERT_XY(this, x, y);
					ASSERT_UNCOMPRESSED();
					return (UInt8*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + x);
				}
		UInt16*	GetAddr16(unsigned x, unsigned y) const
				{
					ASSERT_PIXELSIZE(this, 16);
					ASSERT_XY(this, x, y);
					ASSERT_UNCOMPRESSED();
					return (UInt16*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 1));
				}
		UInt32*	GetAddr32(unsigned x, unsigned y) const
				{
					ASSERT_PIXELSIZE(this, 32);
					ASSERT_XY(this, x, y);
					ASSERT_UNCOMPRESSED();
					return (UInt32*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 2));
				}
		void*	GetAddr64(unsigned x, unsigned y) const
				{
					ASSERT_PIXELSIZE(this, 64);
					ASSERT_XY(this, x, y);
					ASSERT_UNCOMPRESSED();
					return (void*)((char*)fCurrLevelPtr + y * fCurrLevelRowBytes + (x << 3));
				}

		//	This sets fPixelSize, fSpace, fFlags, for you
		//	All you need to set is
		//		fWidth, fHeight, fRowBytes, fImage and fColorTable
		enum {
			kColor8Config = 0,
			kGray44Config = 1,
			kGray4Config = 2,
			kGray8Config = 8,		// So we can use bit depths instead
			kRGB16Config = 16,
			kRGB32Config = 24,
			kARGB32Config = 32,
		};

		void	SetConfig( unsigned config );


		//// Really complex creation stuff ////

		enum {
			kCreateDetailAlpha		= 0x1,
			kCreateDetailAdd		= 0x2,
			kCreateDetailMult		= 0x4,
			kCreateDetailMask		= kCreateDetailAlpha | kCreateDetailAdd | kCreateDetailMult,
			kCreateCarryAlpha		= 0x8,
			kCreateCarryWhite		= 0x10,
			kCreateCarryBlack		= 0x20,
			kCreateCarryMask		= kCreateCarryAlpha | kCreateCarryWhite | kCreateCarryBlack
		};
		enum hsGPixelType {
			kPixelARGB4444,
			kPixelARGB1555,
			kPixelAI88,
			kPixelI8
		};
		enum hsGCopyOptions {
			kCopyLODMask,
		};

		enum {
			kColorDataRLE	= 0x1,
			kAlphaDataRLE	= 0x2
		};

		void	SetBitmapAsLevel(UInt8 iDst, plMipmap *bm, hsScalar sig, UInt32 createFlags, 
											  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
											  hsScalar detailMax, hsScalar detailMin);
		void	ICreateLevelNoDetail(UInt8 iDst, const plFilterMask& mask);
		void	IBlendLevelDetailAlpha(UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin);
		void	IBlendLevelDetailAdd(UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin);
		void	IBlendLevelDetailMult(UInt8 iDst, const plFilterMask& mask, 
										  hsScalar detailDropoffStart, hsScalar detailDropoffStop, 
										  hsScalar detailMax, hsScalar detailMin);
		void	Filter(hsScalar sig);
		UInt32	CopyOutPixels(UInt32 destXSize, UInt32 destYSize, UInt32 dstFormat, void *destPixels, UInt32 copyOptions);

		void	ClipToMaxSize( UInt32 maxDimension );
		void	RemoveMipping();

		void	EnsureKonstantBorder( hsBool clampU, hsBool clampV );

		enum CompositeFlags
		{
			kForceOpaque	= 0x0001,		// Copy src pixels raw, force dest alphas to opaque
			kCopySrcAlpha	= 0x0002,		// Copy the src pixels raw, including alphas, overwrite dest
			kBlendSrcAlpha	= 0x0004,		// Blend src pixels onto dest using src alpha, dest alpha = src alpha
			kMaskSrcAlpha	= 0x0008,		// Same as copySrcAlpha, but dest is untouched when src alpha = 0
			kBlendWriteAlpha= 0x0010		// Like default (0), but writes dest alpha values
		};

		class CompositeOptions
		{
			// Helper class for specifying options to Composite()
			public:
				UInt16		fFlags;
				UInt8		fSrcLevelsToSkip;
				UInt8		fOpacity;
				hsScalar	fRedTint, fGreenTint, fBlueTint;
				UInt16		fSrcClipX, fSrcClipY;			// Clipping is applied AFTER levelSkip
				UInt16		fSrcClipWidth, fSrcClipHeight;	// 0 means max width/height

				CompositeOptions() { fFlags = 0; fSrcLevelsToSkip = 0; fRedTint = fGreenTint = fBlueTint = 1.f;
									fSrcClipX = fSrcClipY = fSrcClipWidth = fSrcClipHeight = 0; fOpacity = 255;}

				CompositeOptions( UInt16 flags, UInt8 srcLevelsToSkip = 0, hsScalar red = 1.f, hsScalar green = 1.f,
									hsScalar blue = 1.f, UInt16 srcClipX = 0, UInt16 srcClipY = 0, 
									UInt16 srcClipWidth = 0, UInt16 srcClipHeight = 0, UInt8 opacity = 255 ) 
				{
					fFlags = flags;
					fSrcLevelsToSkip = srcLevelsToSkip;
					fRedTint = red;
					fGreenTint = green;
					fBlueTint = blue;
					fSrcClipX = srcClipX;
					fSrcClipY = srcClipY;
					fSrcClipWidth = srcClipWidth;
					fSrcClipHeight = srcClipHeight;
					fOpacity = opacity;
				}
		};

		// Compositing function. Take a (smaller) mipmap and composite it onto this one at the given location. Nil options means use default
		virtual void	Composite( plMipmap *source, UInt16 x, UInt16 y, CompositeOptions *options = nil );

		// Scaling function
		enum ScaleFilter
		{
			kBoxFilter,
			kDefaultFilter = kBoxFilter
		};

		virtual void	ScaleNicely( UInt32 *destPtr, UInt16 destWidth, UInt16 destHeight,
								UInt16 destStride, plMipmap::ScaleFilter filter ) const;

		virtual hsBool	ResizeNicely( UInt16 newWidth, UInt16 newHeight, plMipmap::ScaleFilter filter );

	protected:

		//// Protected Members ////

		void		*fImage;
		UInt32		fWidth, fHeight, fRowBytes, fTotalSize;
		UInt8		fNumLevels;
		UInt32		*fLevelSizes;

		void		*fCurrLevelPtr;
		UInt8		fCurrLevel;
		UInt32		fCurrLevelWidth, fCurrLevelHeight, fCurrLevelRowBytes;

		void	IReadRawImage( hsStream *stream );
		void	IWriteRawImage( hsStream *stream );
		plMipmap *ISplitAlpha();
		void	IRecombineAlpha( plMipmap *alphaChannel );
		plMipmap *IReadRLEImage( hsStream *stream );
		void	IWriteRLEImage( hsStream *stream, plMipmap *mipmap );
		void	IReadJPEGImage( hsStream *stream );
		void	IWriteJPEGImage( hsStream *stream );
		void	IBuildLevelSizes();

		void	IColorLevel( UInt8 level, const UInt8 *colorMask );

		hsScalar	IGetDetailLevelAlpha( UInt8 level, hsScalar dropStart, hsScalar dropStop, hsScalar min, hsScalar max );

		void		ICarryZeroAlpha(UInt8 iDst);
		void		ICarryColor(UInt8 iDst, UInt32 col);

		hsBool		IGrabBorderColor( hsBool grabVNotU, UInt32 *color );
		void		ISetCurrLevelUBorder( UInt32 color );
		void		ISetCurrLevelVBorder( UInt32 color );

		virtual UInt32 	Read( hsStream *s );
		virtual UInt32 	Write( hsStream *s );

		friend class plCubicEnvironmap;

#ifdef MEMORY_LEAK_TRACER

	protected:
	
		class plRecord
		{
			public:
				plRecord	*fNext;
				plRecord	**fBackPtr;

				char		fKeyName[ 256 ];
				void		*fImage;
				UInt32		fWidth, fHeight, fRowBytes;
				UInt8		fNumLevels;	
				UInt8		fCompressionType;
				union 
				{
					DirectXInfo			fDirectXInfo;
					UncompressedInfo	fUncompressedInfo;
				};
				enum Method
				{
					kViaCreate,
					kViaRead,
					kViaClipToMaxSize,
					kViaDetailMapConstructor,
					kViaCopyFrom,
					kViaResize
				} fCreationMethod;

				void	Link( plRecord **backPtr )
				{
					fBackPtr = backPtr;
					fNext = *backPtr;
					if( fNext != nil )
						fNext->fBackPtr = &fNext;
					*backPtr = this;
				}

				void	Unlink()
				{
					*fBackPtr = fNext;
					if( fNext != nil )
						fNext->fBackPtr = fBackPtr;
				}
		};

		static plRecord	*fRecords;
		static UInt32	fNumMipmaps;

		static void	IAddToMemRecord( plMipmap *mip, plRecord::Method method );
		static void	IRemoveFromMemRecord( UInt8 *image );
		static void	IReportLeaks();

#endif
};


#endif // _plMipmap_h
