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
//	plBitmap Class Header													 //
//	Base bitmap class for all the types of bitmaps (mipmaps, cubic envmaps,  //
//	etc.																	 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	6.7.2001 mcn - Created.													 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#ifndef _plBitmap_h
#define _plBitmap_h

#include "../pnKeyedObject/hsKeyedObject.h"

class hsResMgr;
class plFilterMask;
class hsGDeviceRef;

//// Class Definition /////////////////////////////////////////////////////////

class plBitmap : public hsKeyedObject
{
	public:

		//// Public Flags ////

		enum {
			kNoSpace,
			kDirectSpace,
			kGraySpace,
			kIndexSpace
		};

		enum Flags {
			kNoFlag				= 0x0000,
			kAlphaChannelFlag	= 0x0001,
			kAlphaBitFlag		= 0x0002,
			kBumpEnvMap			= 0x0004,
			kForce32Bit			= 0x0008,
			kDontThrowAwayImage	= 0x0010,	// We can delete the image, but the pipeline can't
			kForceOneMipLevel	= 0x0020,
			kNoMaxSize			= 0x0040,
			kIntensityMap		= 0x0080,
			kHalfSize			= 0x0100,
			kUserOwnsBitmap		= 0x0200,
			kForceRewrite       = 0x0400,
			kForceNonCompressed	= 0x0800,
			// For renderTargets:
			kIsTexture			= 0x1000,
			kIsOffscreen		= 0x2000,
			kMainScreen			= 0x0000,	// Exclusive, i.e. no renderTarget flags
			kIsProjected		= 0x4000,
			kIsOrtho			= 0x8000
		};

		//// Public Data /////

		enum		// Compression format
		{
			kUncompressed		= 0x0,
			kDirectXCompression	= 0x1,
			kJPEGCompression	= 0x2
		};

		struct DirectXInfo
		{
			enum	// Compression type
			{
				kError			= 0x0,
				kDXT1			= 0x1,
				//kDXT2			= 0x2,
				//kDXT3			= 0x3,
				//kDXT4			= 0x4,
				kDXT5			= 0x5
			};

			UInt8		fCompressionType;	
			UInt8		fBlockSize;				// In bytes
		};

		struct UncompressedInfo
		{
			enum
			{
				kRGB8888 = 0x00,	/// 32-bit 8888 ARGB format
				kRGB4444 = 0x01,	/// 16-bit 4444 ARGB format
				kRGB1555 = 0x02,	/// 16-bit 555 RGB format w/ alpha bit
				kInten8    = 0x03,	/// 8-bit intensity channel (monochrome)
				kAInten88  = 0x04	/// 8-bit intensity channel w/ 8-bit alpha
			};

			UInt8	fType;
		};

		//// Public Data /////

		UInt8		fCompressionType;
		union 
		{
			DirectXInfo			fDirectXInfo;
			UncompressedInfo	fUncompressedInfo;
		};


		//// Public Members ////

		plBitmap();
		virtual ~plBitmap();

		CLASSNAME_REGISTER( plBitmap );
		GETINTERFACE_ANY( plBitmap, hsKeyedObject );

		// Get the total size in bytes
		virtual UInt32	GetTotalSize( void ) const = 0;

		// Read and write
		virtual void	Read( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Read( s, mgr ); this->Read( s ); }
		virtual void	Write( hsStream *s, hsResMgr *mgr ) { hsKeyedObject::Write( s, mgr ); this->Write( s ); }

		UInt16			GetFlags( void ) const { return fFlags; }
		void			SetFlags( UInt16 flags ) { fFlags = flags; }

		UInt8			GetPixelSize( void ) const { return fPixelSize; }

		hsBool			IsCompressed( void ) const { return ( fCompressionType == kDirectXCompression ); }

		virtual void			MakeDirty();
		virtual hsGDeviceRef	*GetDeviceRef( void ) const { return fDeviceRef; }
		virtual void			SetDeviceRef( hsGDeviceRef *const devRef );

		static void		SetGlobalLevelChopCount( UInt8 count ) { fGlobalNumLevelsToChop = count; }
		static UInt8	GetGlobalLevelChopCount() { return fGlobalNumLevelsToChop; }

		// Compares and sets the modified time for the source texture
		bool IsSameModifiedTime(UInt32 lowTime, UInt32 highTime);
		void SetModifiedTime(UInt32 lowTime, UInt32 highTime);

	protected:

		//// Protected Members ////

		UInt8			fPixelSize;	// 1, 2, 4, 8, 16, (24), 32, 64
		UInt8			fSpace;		// no, direct, gray, index
		UInt16			fFlags;		// alphachannel | alphabit

		mutable hsGDeviceRef	*fDeviceRef;

		static UInt8			fGlobalNumLevelsToChop;

		// The modification time of the source texture.
		// Used to determine if we can reuse an already converted copy.
		UInt32 fLowModifiedTime;
		UInt32 fHighModifiedTime;

		virtual UInt32	Read( hsStream *s );
		virtual UInt32	Write( hsStream *s );
};

#endif // _plBitmap_h
