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
//	plFontFreeType Functions												 //
//	Basically our cheat to allow importing fonts via FreeType2 into our		 //
//	plFont system without having to make plFont.cpp reliant on FreeType2.	 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "plFontFreeType.h"

#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H

#define kMaxGlyphs	65536



hsBool	plFontFreeType::ImportFreeType( const char *fontPath, Options *options, plBDFConvertCallback *callback )
{
	FT_Library  ftLibrary;

	
	IClear();

	// Init FreeType2
	FT_Error error = FT_Init_FreeType( &ftLibrary );
	if( error )
	{
		return false;
	}

	try
	{

		// Load our font that we're converting
		FT_Face ftFace;
		error = FT_New_Face( ftLibrary, fontPath, 0, &ftFace );
		if( error == FT_Err_Unknown_File_Format )
		{
			// Unsupported inport format
			throw false;
		}
		else if( error )
		{
			// Some other error
			throw false;
		}

		// Set our point size to convert to
		error = FT_Set_Char_Size( ftFace, 0, options->fSize * 64, options->fScreenRes, options->fScreenRes );	// Size is in 1/64ths of a point
		if( error )
		{
			throw false;
		}

		// Run through our glyphs, loading them into a temp array and calcing bounding boxes for them
		FT_GlyphSlot	ftSlot = ftFace->glyph;
		FT_ULong		ftChar;
		FT_UInt			ftIndex;
		UInt32			numGlyphs = 0, totalHeight = 0, maxChar = 0, i;
		FT_Glyph		ftGlyphs[ kMaxGlyphs ];
		UInt16			glyphChars[ kMaxGlyphs ];
		FT_Vector		ftAdvances[ kMaxGlyphs ];
		FT_BBox			ftGlyphBox, ftFontBox;
		FT_UInt			previous = 0;
		hsBool			useKerning = false;


		if( options->fUseKerning )
			useKerning = FT_HAS_KERNING( ftFace );

		ftFontBox.xMin = ftFontBox.yMin = 32000;
		ftFontBox.xMax = ftFontBox.yMax = -32000;

		// Hack for now: if we don't have a charmap active already, just choose the first one
		if( ftFace->charmap == nil )
		{
			if( ftFace->num_charmaps == 0 )
				throw false;

			FT_Set_Charmap( ftFace, ftFace->charmaps[ 0 ] );
		}

		ftChar = FT_Get_First_Char( ftFace, &ftIndex );
		while( ftIndex != 0 && numGlyphs < kMaxGlyphs )
		{
			error = FT_Load_Glyph( ftFace, ftIndex, FT_LOAD_DEFAULT );
			if( !error && ftChar <= options->fMaxCharLimit )
			{

				// Got this glyph, store it and update our bounding box
				error = FT_Get_Glyph( ftFace->glyph, &ftGlyphs[ numGlyphs ] );
				if( error )
					throw false;
	
				// Don't know why this isn't valid per actual glyph...
				ftAdvances[ numGlyphs ] = ftSlot->advance;

				FT_Glyph_Get_CBox( ftGlyphs[ numGlyphs ], ft_glyph_bbox_pixels, &ftGlyphBox );
				if( ftGlyphBox.xMin < ftFontBox.xMin )
					ftFontBox.xMin = ftGlyphBox.xMin;
				if( ftGlyphBox.yMin < ftFontBox.yMin )
					ftFontBox.yMin = ftGlyphBox.yMin;
				if( ftGlyphBox.xMax > ftFontBox.xMax )
					ftFontBox.xMax = ftGlyphBox.xMax;
				if( ftGlyphBox.yMax > ftFontBox.yMax )
					ftFontBox.yMax = ftGlyphBox.yMax;

				totalHeight += ftGlyphBox.yMax - ftGlyphBox.yMin + 1;

				if( maxChar < ftChar )
					maxChar = ftChar;
				glyphChars[ numGlyphs ] = (UInt16)ftChar;

				numGlyphs++;
			}

			ftChar = FT_Get_Next_Char( ftFace, ftChar, &ftIndex );
		}

		// Init some of our font properties
		fBPP = options->fBitDepth;
		fWidth = ( ftFontBox.xMax - ftFontBox.xMin + 1 );
		if( fBPP == 1 )
			fWidth = ( ( fWidth + 7 ) >> 3 ) << 3;
		fHeight = totalHeight;
		fBMapData = new UInt8[ ( fWidth * fHeight * fBPP ) >> 3 ];
		memset( fBMapData, 0, ( fWidth * fHeight * fBPP ) >> 3 );

		// Set the name and size of our font
		fSize = options->fSize;
		char str[ 512 ];
		
		if( ftFace->style_flags & FT_STYLE_FLAG_ITALIC )
			SetFlag( kFlagItalic, true );
		if( ftFace->style_flags & FT_STYLE_FLAG_BOLD )
			SetFlag( kFlagBold, true );
		
		if( IsFlagSet( kFlagItalic | kFlagBold ) )
			sprintf( str, "%s %s", ftFace->family_name, ftFace->style_name );
		else
			strcpy( str, ftFace->family_name );
		SetFace( str );

		// # of bytes per row 
		UInt32 stride = ( fBPP == 1 ) ? ( fWidth >> 3 ) : fWidth;

		// Pre-expand our char list
		fCharacters.ExpandAndZero( maxChar + 1 );
		fCharacters.SetCount( 0 );
		if( callback != nil )
			callback->NumChars( (UInt16)(maxChar + 1) );

		// Now re-run through our stored list of glyphs, converting them to bitmaps
		for( i = 0; i < numGlyphs; i++ )
		{
			if( ftGlyphs[ i ]->format != ft_glyph_format_bitmap )
			{
				FT_Vector origin;
				origin.x = 32;		// Half a pixel over
				origin.y = 0;

				error = FT_Glyph_To_Bitmap( &ftGlyphs[ i ], ( fBPP == 1 ) ? ft_render_mode_mono : ft_render_mode_normal, &origin, 0 );
				if( error )
					throw false;
			}

			if( fCharacters.GetCount() < glyphChars[ i ] + 1 )
				fCharacters.SetCount( glyphChars[ i ] + 1 );

			FT_BitmapGlyph ftBitmap = (FT_BitmapGlyph)ftGlyphs[ i ];
			plCharacter *ch = &fCharacters[ glyphChars[ i ] ];

			UInt8 *ourBitmap = IGetFreeCharData( ch->fBitmapOff );
			if( ourBitmap == nil )
				throw false;

			if( ch->fBitmapOff > ( ( fWidth * ( fHeight - ftBitmap->bitmap.rows ) * fBPP ) >> 3 ) )
			{
				hsAssert( false, "Invalid position found in IGetFreeCharData()" );
				return nil;
			}

			// Set these now, since setting them before would've changed the IGetFreeCharData results
			ch->fLeftKern = (hsScalar)ftBitmap->left;
			ch->fRightKern = (hsScalar)ftAdvances[ i ].x / 64.f - (hsScalar)fWidth - ch->fLeftKern;//ftBitmap->bitmap.width;
			ch->fBaseline = ftBitmap->top;
			ch->fHeight = ftBitmap->bitmap.rows;

			// Copy data straight through to our bitmap
			int y;
			UInt8 *srcData = (UInt8 *)ftBitmap->bitmap.buffer;

			UInt32 bytesWide = ( fBPP == 1 ) ? ( ( ftBitmap->bitmap.width + 7 ) >> 3 ) : ftBitmap->bitmap.width;

			for( y = 0; y < ch->fHeight; y++ )
			{
				memcpy( ourBitmap, srcData, bytesWide );
				srcData += ftBitmap->bitmap.pitch;
				ourBitmap += stride;
			}

			FT_Done_Glyph( ftGlyphs[ i ] );

			if( callback != nil )
				callback->CharDone();
		}

	}
	catch( ... )
	{
		// Shut down FreeType2
		FT_Done_FreeType( ftLibrary );
		return false;
	}

	ICalcFontAscent();

	// Shut down FreeType2
	FT_Done_FreeType( ftLibrary );
	return true;
}
