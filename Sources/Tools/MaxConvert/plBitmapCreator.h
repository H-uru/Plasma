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
#include "../pnKeyedObject/plKey.h"

class BitmapInfo;
class Bitmap;
class plBitmap;
class plMipmap;
class hsMaxLayerBase;
class plLocation;
class plErrorMsg;


class plBitmapData
{
public:
	enum
	{
		kClampU	= 0x01,
		kClampV	= 0x02
	};

	const char *fileName;
	UInt32 texFlags;
	UInt32 createFlags;
	hsScalar detailDropoffStart;
	hsScalar detailDropoffStop;
	hsScalar detailMax;
	hsScalar detailMin;
	float sig;
	hsBool	isStaticCubicEnvMap;
	hsBool	invertAlpha;
	const char	*faceNames[ 6 ];
	UInt32	maxDimension;
	UInt8	clampFlags;
	bool	useJPEG;

	plBitmapData()
	{
		fileName = nil;
		texFlags = 0;
		createFlags = 0;
		detailDropoffStart = detailDropoffStop = 0.f;
		detailMax = detailMin = 0.f;
		sig = 0;
		isStaticCubicEnvMap = false;
		invertAlpha = false;
		faceNames[ 0 ] = faceNames[ 1 ] = faceNames[ 2 ] = faceNames[ 3 ] = faceNames[ 4 ] = faceNames[ 5 ] = nil;
		maxDimension = 0;
		clampFlags = 0;
		useJPEG = false;
	}
};

class plRegistryKeyIterator;
class plBitmapCreator
{
	public:

		static plBitmapCreator	&Instance();

		plBitmap	*CreateTexture( plBitmapData *bd, const plLocation &loc, int clipID = -1 );
		plMipmap	*CreateBlankMipmap( UInt32 width, UInt32 height, unsigned config, UInt8 numLevels, const char *keyName, const plLocation &keyLocation );

		void	Init( hsBool save, plErrorMsg *msg );
		void	DeInit( void );
		void	CleanUpMaps( void );

		~plBitmapCreator();

		// This will also set the key you pass in to nil, so be careful
		void	DeleteExportedBitmap( const plKey &key );

	protected:

		plErrorMsg				*fErrorMsg;

		plBitmapCreator();

		plBitmap	*ICreateTexture( plBitmapData *bd, const plLocation &loc, int clipID = -1 );
		plMipmap	*ICreateBitmap( plBitmapData *bd );

		void	ICheckOutBitmap( BitmapInfo *bInfo, Bitmap *bm, const char *fileName );
		int		IResampBitmap( Bitmap *bm, plMipmap &hBitmap );
		int		ICopyBitmap( Bitmap *bm, plMipmap &hBitmap );
		int		IInvertAlpha( plMipmap &hBitmap );

		void	IAddBitmap( plBitmap *bitmap, hsBool dontRef = false );
};
