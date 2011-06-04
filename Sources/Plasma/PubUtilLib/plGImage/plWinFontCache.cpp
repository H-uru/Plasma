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
//	plWinFontCache Class Header												 //
//	I've stopped keeping track, there are far too many reasons already to	 //
//	hate Microsoft. Anyway, this class keeps track of various Win32 fonts we //
//	allocate because Win98/ME seems to have problems re-allocating the exact //
//	same freaking goddamn font over and over again. I mean, you'd think		 //
//  there'd be a rule somewhere about deterministic behavior when calling	 //
//	the exact same function with the exact same parameters over and over...	 //
//	Oh, wait...																 //
//																			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	11.25.2002 mcn - Created.												 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsWindows.h"
#include "plWinFontCache.h"

#include "../plStatusLog/plStatusLog.h"
#include "../plFile/hsFiles.h"
#include "../plGImage/plDynSurfaceWriter.h"

#if HS_BUILD_FOR_WIN32

#include <wingdi.h>


char	*plWinFontCache::kCustFontExtension = ".prf";


plWinFontCache::plWinFontCache()
{
	fInShutdown = false;
	fCustFontDir = nil;
}

plWinFontCache::~plWinFontCache()
{
	fInShutdown = true;
	Clear();
	delete [] fCustFontDir;
}

plWinFontCache	&plWinFontCache::GetInstance( void )
{
	static plWinFontCache	cache;
	return cache;
}

HFONT	plWinFontCache::IFindFont( const char *face, int height, int weight, hsBool italic, UInt32 quality )
{
	int		i;


	for( i = 0; i < fFontCache.GetCount(); i++ )
	{
		// Do other tests first, since they're cheaper
		if( fFontCache[ i ].fHeight == height &&
			fFontCache[ i ].fWeight == weight &&
			fFontCache[ i ].fItalic == italic &&
			fFontCache[ i ].fQuality == quality )
		{
			if( strcmp( fFontCache[ i ].fFace, face ) == 0 )
				return fFontCache[ i ].fFont;
		}
	}

	return nil;
}

HFONT	plWinFontCache::IMakeFont( const char *face, int height, int weight, hsBool italic, UInt32 quality )
{
	plFontRecord	myRec;
	int				i;


	// Find a cached name for us
	for( i = 0; i < fFontNameCache.GetCount(); i++ )
	{
		if( strcmp( face, fFontNameCache[ i ] ) == 0 )
			break;
	}

	if( i == fFontNameCache.GetCount() )
		fFontNameCache.Append( hsStrcpy( face ) );

	myRec.fFace = fFontNameCache[ i ];
	myRec.fHeight = height;
	myRec.fWeight = weight;
	myRec.fItalic = italic;
	myRec.fQuality = quality;

	myRec.fFont = CreateFont( height, 0, 0, 0, weight, italic ? TRUE : FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, VARIABLE_PITCH, face );

	if( myRec.fFont != nil )
	{
//#ifdef HS_DEBUGGING
#if 1
		LOGFONT	fontInfo;

		if( GetObject( myRec.fFont, sizeof( fontInfo ), &fontInfo ) )
		{
			const char *err = nil;

			if( fontInfo.lfQuality != quality )			
				err = "Quality of created font does not match";
			if( fontInfo.lfHeight != height )
				err = "Height of created font does not match";
			if( fontInfo.lfWeight != weight )
				err = "Weight of created font does not match";
			if( fontInfo.lfItalic != italic )
				err = "Italic-ness of created font does not match";
			if( stricmp( fontInfo.lfFaceName, face ) != 0 )
				err = "Face of created font does not match";

			if( err != nil )
			{
				static bool triedClearing = false;

				if( fontInfo.lfQuality != quality )			
				{
					plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() failed to return proper font (%s). Using what was given...", err );
				}
				else
				{
					plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() failed to return proper font (%s). %s", err, triedClearing ? "" : "Clearing cache and retrying..." );
					if( !triedClearing )
					{
						triedClearing = true;

						// Didn't work, so get rid of it
						DeleteObject( myRec.fFont );

						// Clear all fonts and try again
						Clear();
						
						// Make sure we reload our custom fonts tho
						ILoadCustomFonts();
						
						// Try again
						HFONT font = IMakeFont( face, height, weight, italic, quality );
						
						triedClearing = false;

						return font;
					}
				}
			}
		}
#endif

		fFontCache.Append( myRec );
	}
	else
	{
		plStatusLog::AddLineS( "pipeline.log", "ERROR: CreateFont() call FAILED (face: %s, size: %d %s %s)", face, -height, weight == FW_BOLD ? "bold" : "", italic ? "italic" : "" );
	}

	return myRec.fFont;
}

HFONT	plWinFontCache::GetMeAFont( const char *face, int height, int weight, hsBool italic, UInt32 quality )
{
	HFONT	font = IFindFont( face, height, weight, italic, quality );
	if( font == nil )
		font = IMakeFont( face, height, weight, italic, quality );

	return font;
}

void	plWinFontCache::Clear( void )
{
	int		i;


	if( !fInShutdown )
		plStatusLog::AddLineS( "pipeline.log", "** Clearing Win32 font cache **" );

	for( i = 0; i < fFontCache.GetCount(); i++ )
		DeleteObject( fFontCache[ i ].fFont );
	fFontCache.Reset();

	for( i = 0; i < fFontNameCache.GetCount(); i++ )
		delete [] fFontNameCache[ i ];
	fFontNameCache.Reset();

	for( i = 0; i < fCustFonts.GetCount(); i++ )
	{
#if (_WIN32_WINNT >= 0x0500)
		if( plDynSurfaceWriter::CanHandleLotsOfThem() )
			RemoveFontResourceEx( fCustFonts[ i ]->fFilename, FR_PRIVATE, 0 );
		else
#endif
			if( RemoveFontResource( fCustFonts[ i ]->fFilename ) == 0 )
			{
				int q= 0;
				DWORD e = GetLastError();
			}
		delete fCustFonts[ i ];
	}
	fCustFonts.Reset();
}

void	plWinFontCache::FreeFont( HFONT font )
{
	// Currently a no-op, but should do some sort of ref-counting
}

void	plWinFontCache::LoadCustomFonts( const char *dir )
{
	delete [] fCustFontDir;
	fCustFontDir = ( dir != nil ) ? hsStrcpy( dir ) : nil;

	ILoadCustomFonts();
}

void	plWinFontCache::ILoadCustomFonts( void )
{
	if( fCustFontDir == nil )
		return;

	// Iterate through all the custom fonts in our dir
	hsFolderIterator	iter( fCustFontDir );
	char				fileName[ kFolderIterator_MaxPath ];
	int					numAdded;


	while( iter.NextFileSuffix( kCustFontExtension ) )
	{
		iter.GetPathAndName( fileName );
	
		// Note that this call can be translated as "does my OS suck?"
#if (_WIN32_WINNT >= 0x0500)
		if( plDynSurfaceWriter::CanHandleLotsOfThem() )
			numAdded = AddFontResourceEx( fileName, FR_PRIVATE, 0 );
		else
#endif
			numAdded = AddFontResource( fileName );

		if( numAdded > 0 )
		{
			plStatusLog::AddLineS( "pipeline.log", "WinFontCache: Added custom font %s, %d fonts", fileName, numAdded );
			fCustFonts.Append( TRACKED_NEW plCustFont( fileName ) );
		}
		else
		{
			plStatusLog::AddLineS( "pipeline.log", "WinFontCache: Unable to load custom font %s", fileName );
		}
	}
}

#endif
