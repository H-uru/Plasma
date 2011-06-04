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
//	plFont Class Header														 //
//	Seems like we've come full circle again. This is our generic Plasma		 //
//	bitmap font class/format. Quick list of things it supports, or needs to: //
//		- Antialiasing, either in the font def or at rendertime				 //
//		- Doublebyte character sets											 //
//		- Platform independence, of course									 //
//		- Render to reasonably arbitrary mipmap								 //
//		- Character-level kerning, both before and after, as well as		 //
//		  negative kerning (for ligatures)									 //
//																			 //
//	Cyan, Inc.																 //
//																			 //
//// Version History //////////////////////////////////////////////////////////
//																			 //
//	3.4.2003 mcn - Created.													 //
//	4.3.2003 mcn - Updated. Casting a char to a UInt16 sign-extends it if	 //
//				   the char is > 128, but casting it to an UInt8 first works.//
//				   Ugly as sin, but hey, so are you.						 //
//																			 //
///////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStlUtils.h"
#include "plFont.h"

#include "plMipmap.h"
#include "hsResMgr.h"


//// plCharacter Stuff ////////////////////////////////////////////////////////

plFont::plCharacter::plCharacter()
{
	fBitmapOff = 0;
	fHeight = 0;
	fBaseline = 0;
	fLeftKern = fRightKern = 0.f;
}

void	plFont::plCharacter::Read( hsStream *s )
{
	// Rocket science here...
	s->ReadSwap( &fBitmapOff );
	s->ReadSwap( &fHeight );
	s->ReadSwap( &fBaseline );
	s->ReadSwap( &fLeftKern );
	s->ReadSwap( &fRightKern );
}

void	plFont::plCharacter::Write( hsStream *s )
{
	s->WriteSwap( fBitmapOff );
	s->WriteSwap( fHeight );
	s->WriteSwap( fBaseline );
	s->WriteSwap( fLeftKern );
	s->WriteSwap( fRightKern );
}

//// Constructor/Read/Write/Destructor/etc ////////////////////////////////////

plFont::plFont()
{
	IClear( true );
}

plFont::~plFont()
{
	IClear();
}

void	plFont::IClear( bool onConstruct )
{
	if( !onConstruct )
		delete [] fBMapData;

	memset( fFace, 0, sizeof( fFace ) );
	fSize = 0;
	fFlags = 0;

	fWidth = fHeight = 0;
	fBPP = 0;
	fBMapData = nil;
	fFirstChar = 0;
	fMaxCharHeight = 0;
	fCharacters.Reset();

	fRenderInfo.fFlags = 0;
	fRenderInfo.fX = fRenderInfo.fY = fRenderInfo.fNumCols = 0;
	fRenderInfo.fMaxWidth = fRenderInfo.fMaxHeight = 0;
	fRenderInfo.fDestPtr = nil;
	fRenderInfo.fDestStride = 0;
	fRenderInfo.fColor = 0;
	fRenderInfo.fMipmap = nil;
	fRenderInfo.fRenderFunc = nil;
	fRenderInfo.fVolatileStringPtr = nil;
	fRenderInfo.fFirstLineIndent = 0;
	fRenderInfo.fLineSpacing = 0;
}

void	plFont::SetFace( const char *face )
{
	strncpy( fFace, face, sizeof( fFace ) );
}

void	plFont::SetSize( UInt8 size )
{
	fSize = size;
}

void	plFont::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );
	ReadRaw( s );
}

void	plFont::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write( s, mgr );
	WriteRaw( s );
}

///////////////////////////////////////////////////////////////////////////////
//// Rendering ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void	plFont::SetRenderColor( UInt32 color )
{
	fRenderInfo.fColor = color;
}

void	plFont::SetRenderClipRect( Int16 x, Int16 y, Int16 width, Int16 height )
{
	fRenderInfo.fClipRect.Set( x, y, width, height );
}

void	plFont::SetRenderClipping( Int16 x, Int16 y, Int16 width, Int16 height )
{
	SetRenderFlag( kRenderWrap, false );
	SetRenderFlag( kRenderClip, true );
	SetRenderClipRect( x, y, width, height );
}

void	plFont::SetRenderWrapping( Int16 x, Int16 y, Int16 width, Int16 height )
{
	SetRenderFlag( kRenderWrap, true );
	SetRenderFlag( kRenderClip, false );
	SetRenderClipRect( x, y, width, height );
}

void	plFont::ICalcFontAscent( void )
{
	UInt32	i;


	// Hack for now, only calc the ascent for characters in the 127 character ASCII range
	for( i = 0, fFontAscent = 0, fFontDescent = 0, fMaxCharHeight = 0; i < fCharacters.GetCount(); i++ )
	{
		if( i + fFirstChar < 128 && fFontAscent < fCharacters[ i ].fBaseline )
			fFontAscent = fCharacters[ i ].fBaseline;

		Int32 descent = fCharacters[ i ].fHeight - fCharacters[ i ].fBaseline;
		if( fFontDescent < descent )
			fFontDescent = descent;

		if( fMaxCharHeight < fCharacters[ i ].fHeight )
			fMaxCharHeight = fCharacters[ i ].fHeight;
	}
}

//// IIsWordBreaker //////////////////////////////////////////////////////////
//	Returns whether the given character is one that can break a line

static inline bool	IIsWordBreaker( const char c )
{
	return ( strchr( " \t,.;\n", c ) != nil ) ? true : false;
}

//// IIsDrawableWordBreak /////////////////////////////////////////////////////
// Returns whether the given character is a line breaker that has to be drawn
// (non whitespace)

static inline bool	IIsDrawableWordBreak( const char c )
{
	return ( strchr( ",.;", c ) != nil ) ? true : false;
}

//// RenderString /////////////////////////////////////////////////////////////
//	The base render function. Additional options are specified externally,
//	so that their effects can be cached for optimization

void	plFont::RenderString( plMipmap *mip, UInt16 x, UInt16 y, const char *string, UInt16 *lastX, UInt16 *lastY )
{
	// convert the char string to a wchar_t string
	wchar_t *wideString = hsStringToWString(string);
	RenderString(mip,x,y,wideString,lastX,lastY);
	delete [] wideString;
}


void	plFont::RenderString( plMipmap *mip, UInt16 x, UInt16 y, const wchar_t *string, UInt16 *lastX, UInt16 *lastY )
{
	if( mip->IsCompressed() )
	{
		hsAssert( false, "Unable to render string to compressed mipmap" );
		return;
	}

	IRenderString( mip, x, y, string, false );
	if( lastX != nil )
		*lastX = fRenderInfo.fLastX;
	if( lastY != nil )
		*lastY = fRenderInfo.fLastY;
}

void	plFont::IRenderString( plMipmap *mip, UInt16 x, UInt16 y, const wchar_t *string, hsBool justCalc )
{
	fRenderInfo.fMipmap = mip;
	fRenderInfo.fX = x;
	fRenderInfo.fY = y;
	fRenderInfo.fNumCols = (Int16)(( fBPP <= 8 ) ? ( ( fWidth * fBPP ) >> 3 ) : 0);
	fRenderInfo.fFloatWidth = (hsScalar)fWidth;
	fRenderInfo.fFarthestX = x;
	fRenderInfo.fMaxAscent = 0;
	fRenderInfo.fVolatileStringPtr = string;

	switch( fRenderInfo.fFlags & kRenderJustYMask )
	{
		case kRenderJustYTop: 
			fRenderInfo.fY += (Int16)fFontAscent; 
			break;
		case kRenderJustYBottom:
			if( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) )
				fRenderInfo.fY = (Int16)(fRenderInfo.fClipRect.GetBottom() - 1 - fMaxCharHeight + fFontAscent);
			else
				fRenderInfo.fY = (Int16)(mip->GetHeight() - 1 - fMaxCharHeight + fFontAscent);
			break;
		case kRenderJustYCenter:
			fRenderInfo.fY = (Int16)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetBottom() - 1 : mip->GetHeight() - 1);
			fRenderInfo.fY = (Int16)(( fRenderInfo.fY - fMaxCharHeight ) >> 1);
			fRenderInfo.fY += (Int16)fFontAscent;
			break;
		default:	// Just the baseline
			;
	}

	if( justCalc )
	{
		plCharacter &ch = fCharacters[ (UInt16)string[ 0 ] - fFirstChar ];
		fRenderInfo.fX = fRenderInfo.fFarthestX = x - (Int16)ch.fLeftKern;
		if( fRenderInfo.fX < 0 )
			fRenderInfo.fX = 0;
	}
	else
	{
		switch( fRenderInfo.fFlags & kRenderJustXMask )
		{
			case kRenderJustXLeft:
				// Default
				break;
			case kRenderJustXForceLeft:
				{
//					plCharacter &ch = fCharacters[ (UInt16)(UInt8)string[ 0 ] - fFirstChar ];
//					Int32 newX = x - (Int16)ch.fLeftKern;
//					if( newX < 0 )
//						newX = 0;
//					fRenderInfo.fX = fRenderInfo.fFarthestX = newX;
				}
				break;
			case kRenderJustXCenter:
				{
					UInt16 right = (UInt16)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetRight() : mip->GetWidth());
//					UInt16 width = CalcStringWidth( string );
					fRenderInfo.fX = fRenderInfo.fFarthestX = ( ( x + right ) >> 1 );// - ( width >> 1 );
				}
				break;
			case kRenderJustXRight:
				{
					UInt16 width = 0, right = (UInt16)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetRight() : mip->GetWidth());
/*					if( fRenderInfo.fFlags & kRenderClip )
					{
						fRenderInfo.fFlags &= ~kRenderClip;
						width = CalcStringWidth( string );
						fRenderInfo.fFlags |= kRenderClip;
					}
					else
*/						width = 0;//CalcStringWidth( string );
					fRenderInfo.fX = fRenderInfo.fFarthestX = right - width;
				}
				break;
		}
	}

	// Choose an optimal rendering function
	fRenderInfo.fRenderFunc = nil;
	if( justCalc )
		fRenderInfo.fRenderFunc = IRenderCharNull;
	else if( mip->GetPixelSize() == 32 )
	{
		if( fBPP == 1 )
			fRenderInfo.fRenderFunc = ( fRenderInfo.fFlags & kRenderScaleAA ) ? IRenderChar1To32AA : IRenderChar1To32;
		else if( fBPP == 8 )
		{
			if( fRenderInfo.fFlags & kRenderIntoAlpha )
			{
				if( ( fRenderInfo.fColor & 0xff000000 ) != 0xff000000 )
					fRenderInfo.fRenderFunc = IRenderChar8To32Alpha;
				else
					fRenderInfo.fRenderFunc = IRenderChar8To32FullAlpha;
			}
			else
				fRenderInfo.fRenderFunc = IRenderChar8To32;
		}
	}

	if( fRenderInfo.fRenderFunc == nil )
	{
		hsAssert( false, "Invalid combination of source and destination formats in RenderString()" );
		return;
	}

	// Init our other render values
	if( !justCalc )
	{
		fRenderInfo.fDestStride = mip->GetRowBytes();
		fRenderInfo.fDestBPP = mip->GetPixelSize() >> 3;
		fRenderInfo.fDestPtr = (UInt8 *)mip->GetImage();
		fRenderInfo.fDestPtr += fRenderInfo.fY * fRenderInfo.fDestStride;
		fRenderInfo.fDestPtr += fRenderInfo.fX * fRenderInfo.fDestBPP;
	}
	if( fRenderInfo.fFlags & ( kRenderWrap | kRenderClip ) )
	{
		fRenderInfo.fMaxHeight = (Int16)( fRenderInfo.fClipRect.GetBottom() - fRenderInfo.fY );
		fRenderInfo.fMaxWidth = (Int16)( fRenderInfo.fClipRect.GetRight() - x );
	}
	else if( justCalc )
	{
		// Just calculating, no wrapping, so the max is as far as we can go
		// Note: 32767 isn't quite right, since we'll be adding the left kern in before we
		// calc the first character, so adjust so we make sure we don't underflow
		plCharacter &ch = fCharacters[ (UInt16)(UInt8)string[ 0 ] - fFirstChar ];

		fRenderInfo.fMaxHeight = (Int16)fMaxCharHeight;
		fRenderInfo.fMaxWidth = (Int16)32767 + (Int16)ch.fLeftKern;
	}
	else
	{
		fRenderInfo.fMaxHeight = (Int16)( mip->GetHeight() - fRenderInfo.fY );
		fRenderInfo.fMaxWidth = (Int16)( mip->GetWidth() - x );
	}

	fRenderInfo.fMaxDescent = 0;

	if( fRenderInfo.fFlags & kRenderWrap )
	{
		// Hell, gotta word wrap the text
		// To avoid backtracking, we step forward in the string one word at a time until we hit a break,
		// then render what we have and continue
		Int32 lastWord = 0, i;
		bool isFirstLine = true;
		x = 0;
		Int16 firstMaxAscent = 0;
		UInt32 lineHt, lineDelta;
		if( fRenderInfo.fFlags & kRenderScaleAA )
			lineHt = fMaxCharHeight >> 1;
		else
			lineHt = fMaxCharHeight;
		
		// adjust the line height for spacing
		lineHt += fRenderInfo.fLineSpacing;

		lineDelta = lineHt * fRenderInfo.fDestStride;

		while( *string != 0 && fRenderInfo.fMaxHeight >= fFontDescent )
		{
			UInt8 *destStartPtr = fRenderInfo.fDestPtr;
			UInt32 destStartX = fRenderInfo.fX;
			Int16 destMaxWidth = fRenderInfo.fMaxWidth, thisIndent = 0;

			if( isFirstLine )
			{
				// First line, apply indent if applicable
				fRenderInfo.fX += fRenderInfo.fFirstLineIndent;
				fRenderInfo.fMaxWidth -= fRenderInfo.fFirstLineIndent;
				fRenderInfo.fDestPtr += fRenderInfo.fFirstLineIndent * fRenderInfo.fDestBPP;
				thisIndent = fRenderInfo.fFirstLineIndent;
				isFirstLine = false;
			}

			std::string ellipsisTracker = ""; // keeps track of ellipsis, since there are three word break chars that can't be split
			bool possibleEllipsis = false;
			int preEllipsisLastWord = 0; // where the word break was before we started tracking an ellipsis

			// Iterate through the string, looking for the next line break
			for( lastWord = 0, i = 0; string[ i ] != 0; i++ )
			{
				// If we're a carriage return, we go ahead and break anyway
				if( string[ i ] == L'\n' )
				{
					lastWord = i;
					break;
				}
				
				// handle invalid chars discretely
				plCharacter* charToDraw = NULL;
				if (fCharacters.Count() <= ((UInt16)string[i] - fFirstChar))
					charToDraw = &(fCharacters[(UInt16)L' ' - fFirstChar]);
				else
					charToDraw = &(fCharacters[(UInt16)string[i] - fFirstChar]);

				Int16 leftKern = (Int16)charToDraw->fLeftKern;
				if( fRenderInfo.fFlags & kRenderScaleAA )
					x += leftKern / 2;
				else
					x += leftKern;

				// Update our position and see if we're over
				// Note that our wrapping is slightly off, in that it doesn't take into account
				// the left kerning of characters. Hopefully that won't matter much...
				UInt16 charWidth = (UInt16)(fWidth + (Int16)charToDraw->fRightKern);
				if( fRenderInfo.fFlags & kRenderScaleAA )
					charWidth >>= 1;

				UInt16 nonAdjustedX = (UInt16)(x + fWidth); // just in case the actual bitmap is too big to fit on page and we think the character can (because of right kern)
				x += charWidth;

				if(( x >= fRenderInfo.fMaxWidth ) || (nonAdjustedX >= fRenderInfo.fMaxWidth))
				{
					// we're over, but lastWord may not be correct (especially if we're in the middle of an ellipsis)
					if (possibleEllipsis)
					{
						// ellipsisTracker will not be empty since possibleEllipsis is true (so there will be at least one period)
						if (ellipsisTracker == ".") // only one period so far
						{
							if ((string[i] == '.') && (string[i+1] == '.')) // we have an ellipsis, so reset the lastWord back before we found it
								lastWord = preEllipsisLastWord;
							// otherwise, we don't have an ellipsis, so lastWord is correct (but the grammer might not be ;-)
						}
						else if (ellipsisTracker == "..") // only two periods so far
						{
							if (string[i] == '.') // we have an ellipsis, so reset the lastWord back before we found it
								lastWord = preEllipsisLastWord;
							// otherwise, we don't have an ellipsis, so lastWord is correct (but the grammer might not be ;-)
						}
						// if neither of the above are true, then the full ellipsis was encountered and the lastWord is correct
						ellipsisTracker = "";
						possibleEllipsis = false;
					}
					// Over, so break
					break;
				}

				// Are we a word breaker?
				if( IIsWordBreaker( (char)(string[ i ]) ) )
				{
					if (string[i] == '.') // we might have an ellipsis here
					{
						if (ellipsisTracker == "...") // we already have a full ellipsis, so break between them
						{
							preEllipsisLastWord = i;
							ellipsisTracker = "";
						}
						else if (ellipsisTracker == "") // no ellipsis yet, so save the last word
							preEllipsisLastWord = lastWord;
						ellipsisTracker += '.';
						possibleEllipsis = true;
					}
					else
					{
						ellipsisTracker = ""; // no chance of an ellipsis, so kill it
						possibleEllipsis = false;
					}
					// Yes, and we didn't go over, so store as the last successfully fit word and move on
					lastWord = i;
				}			
			}

			if( string[ i ] == 0 )
				lastWord = i;		// Final catch for end-of-string
			else if( lastWord == 0 && string[ i ] != L'\n' && thisIndent == 0 )
				lastWord = i;		// Catch for a single word that didn't fit (just go up as many chars as we can)
									// (but NOT if we have a first line indent, mind you :)

			// Got to the end of a line (somewhere), so render up to lastWord, then advance from that point
			// to the first non-word-breaker and continue onward
			if( lastWord > 0 )
			{
				bool drawableWordBreak = IIsDrawableWordBreak((char)(string[lastWord]));
				if (drawableWordBreak)
					lastWord++; // make sure we draw it
				if( ( fRenderInfo.fFlags & kRenderJustXMask ) == kRenderJustXRight )
				{
					UInt16 baseX = fRenderInfo.fX, baseMaxW = fRenderInfo.fMaxWidth;
					UInt8 *baseDestPtr = fRenderInfo.fDestPtr;

					fRenderInfo.fX = 0;
					CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
					fRenderInfo.fRenderFunc = IRenderCharNull;

					IRenderLoop( string, lastWord );

					fRenderInfo.fRenderFunc = oldFunc;
					fRenderInfo.fX = baseX - fRenderInfo.fX;
					fRenderInfo.fMaxWidth = baseMaxW;
					fRenderInfo.fDestPtr = baseDestPtr + ( fRenderInfo.fX - baseX ) * fRenderInfo.fDestBPP;

					IRenderLoop( string, lastWord );

					fRenderInfo.fLastX = fRenderInfo.fX;
					fRenderInfo.fLastY = fRenderInfo.fY;
					fRenderInfo.fX = baseX;
				}
				else if( ( fRenderInfo.fFlags & kRenderJustXMask ) == kRenderJustXCenter )
				{
					UInt16 baseX = fRenderInfo.fX, baseMaxW = fRenderInfo.fMaxWidth;
					UInt8 *baseDestPtr = fRenderInfo.fDestPtr;

					fRenderInfo.fX = 0;
					CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
					fRenderInfo.fRenderFunc = IRenderCharNull;

					IRenderLoop( string, lastWord );

					fRenderInfo.fRenderFunc = oldFunc;
					fRenderInfo.fX = baseX - ( fRenderInfo.fX >> 1 );
					fRenderInfo.fMaxWidth = baseMaxW;
					fRenderInfo.fDestPtr = baseDestPtr + ( fRenderInfo.fX - baseX ) * fRenderInfo.fDestBPP;

					IRenderLoop( string, lastWord );

					fRenderInfo.fLastX = fRenderInfo.fX;
					fRenderInfo.fLastY = fRenderInfo.fY;
					fRenderInfo.fX = baseX;
				}
				else if( ( fRenderInfo.fFlags & kRenderJustXMask ) == kRenderJustXForceLeft )
				{
					Int16 baseX = fRenderInfo.fX;

					plCharacter &ch = fCharacters[ (UInt16)string[ 0 ] - fFirstChar ];

					fRenderInfo.fX -= (Int16)ch.fLeftKern;
					fRenderInfo.fDestPtr -= (Int16)ch.fLeftKern * fRenderInfo.fDestBPP;

					IRenderLoop( string, lastWord );
				
					fRenderInfo.fLastX = fRenderInfo.fX;
					fRenderInfo.fLastY = fRenderInfo.fY;
					fRenderInfo.fX = baseX;
				}
				else
				{
					IRenderLoop( string, lastWord );
					fRenderInfo.fLastX = fRenderInfo.fX;
					fRenderInfo.fLastY = fRenderInfo.fY;
				}
				if (drawableWordBreak)
					lastWord--; // reset it
			}


			// Store farthest X for later reference
			if( lastWord > 0 && fRenderInfo.fX > fRenderInfo.fFarthestX )
				fRenderInfo.fFarthestX = fRenderInfo.fX;

			if( firstMaxAscent == 0 )
				firstMaxAscent = fRenderInfo.fMaxAscent;

			x = 0;
			fRenderInfo.fX = (Int16)destStartX;
			fRenderInfo.fDestPtr = destStartPtr;
			fRenderInfo.fMaxWidth = destMaxWidth;
			fRenderInfo.fMaxAscent = 0;

			// Look for the next non-word-breaker. Note that if we have any carriage returns hidden in here, 
			// we'll want to be advancing down even further
			// Advance down
			if( string[ i ] != 0 )
			{
				fRenderInfo.fY += (Int16)lineHt;
				fRenderInfo.fMaxHeight -= (Int16)lineHt;
				fRenderInfo.fDestPtr += lineDelta;
				fRenderInfo.fLastX = fRenderInfo.fX;
				fRenderInfo.fLastY = fRenderInfo.fY;
			}
			for( i = lastWord; string[ i ] != 0 && IIsWordBreaker( (char)(string[ i ]) ) && string[ i ] != L'\n'; i++ )
			{
			}
			// Process any trailing carriage returns as a separate loop b/c we don't want to throw away white space
			// after returns
			for( ; string[ i ] == L'\n'; i++ )
			{
				// Don't process if i==lastWord, since we already did that one
				if( i > lastWord && string[ i ] == L'\n' )
				{
					// Advance down
					fRenderInfo.fY += (Int16)lineHt;
					fRenderInfo.fMaxHeight -= (Int16)lineHt;
					fRenderInfo.fDestPtr += lineDelta;
					fRenderInfo.fLastY = fRenderInfo.fY;
				}
			}

			// Keep going from here!
			string += i;
		}

		fRenderInfo.fMaxAscent = firstMaxAscent;	
		// Final y offset from the last line
		fRenderInfo.fY += (Int16)fFontDescent;

		fRenderInfo.fVolatileStringPtr = string; 
	}
	else
	{
		if( fRenderInfo.fFlags & kRenderClip )
		{
			// Advance left past any clipping area
			CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
			fRenderInfo.fRenderFunc = IRenderCharNull;
			while( fRenderInfo.fX < fRenderInfo.fClipRect.fX && *string != 0 )
			{
				IRenderLoop( string, 1 );
				string++;
			}
			fRenderInfo.fRenderFunc = oldFunc;
		}

		// Adjust for left kern
		if( ( fRenderInfo.fFlags & kRenderJustXMask ) == kRenderJustXForceLeft )
		{
			// See note at top of file
			plCharacter &ch = fCharacters[ (UInt16)string[ 0 ] - fFirstChar ];
			Int32 newX = x - (Int16)ch.fLeftKern;
			if( newX < 0 )
				newX = 0;
			fRenderInfo.fX = fRenderInfo.fFarthestX = (Int16)newX;
		}

		fRenderInfo.fVolatileStringPtr = string;	// Just so we can keep track of when we clip
		IRenderLoop( string, -1 );
		fRenderInfo.fFarthestX = fRenderInfo.fX;
	}
}

void	plFont::IRenderLoop( const wchar_t *string, Int32 maxCount )
{
	// Render the string straight across, one char at a time
	while( *string != 0 && maxCount != 0 )	// Note: if maxCount starts out -1, then it'll just keep 
	{										// decrementing...well ok, not for forever, but pretty close
		UInt16 c = (UInt16)*string;
		if( c < fFirstChar )
			;		// Invalid char
		else
		{
			c -= fFirstChar;
			if( c >= fCharacters.GetCount() )
				;	// Invalid char
			else
			{
				// First pass at supporting left kerning values, but only at the pixel level
				Int16 leftKern = (Int16)fCharacters[ c ].fLeftKern;
				if( leftKern != 0 )
				{
					if( fRenderInfo.fFlags & kRenderScaleAA )
						leftKern /= 2;

					fRenderInfo.fX += leftKern;
					fRenderInfo.fMaxWidth -= leftKern;
					fRenderInfo.fDestPtr += leftKern * fRenderInfo.fDestBPP;
				}

				UInt16 thisWidth = (UInt16)(fWidth + fCharacters[ c ].fRightKern);
				if( fRenderInfo.fFlags & kRenderScaleAA )
					thisWidth >>= 1;

				if( thisWidth >= fRenderInfo.fMaxWidth )
					break;

				(this->*(fRenderInfo.fRenderFunc))( fCharacters[ c ] );

				fRenderInfo.fX += thisWidth;
				fRenderInfo.fMaxWidth -= thisWidth;
				fRenderInfo.fDestPtr += thisWidth * fRenderInfo.fDestBPP;


				Int16 baseline = (Int16)(fCharacters[ c ].fBaseline);
				if( fRenderInfo.fFlags & kRenderScaleAA )
					baseline >>= 1;

				if( baseline > fRenderInfo.fMaxAscent )
					fRenderInfo.fMaxAscent = baseline;

				Int16 thisHt = (Int16)(fCharacters[ c ].fHeight - baseline);
				if( fRenderInfo.fMaxDescent < thisHt )
					fRenderInfo.fMaxDescent = thisHt;
			}
		}

		string++;
		fRenderInfo.fVolatileStringPtr++;
		maxCount--;
	}
}

//// The Rendering Functions //////////////////////////////////////////////////

void	plFont::IRenderChar1To32( const plFont::plCharacter &c )
{
	UInt8	bitMask, *src = fBMapData + c.fBitmapOff;
	UInt32	*destPtr, *destBasePtr = (UInt32 *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
	UInt16	x, y;

	
	if( (Int32)c.fHeight - (Int32)c.fBaseline >= fRenderInfo.fMaxHeight || c.fBaseline > fRenderInfo.fY )
		return;

	for( y = 0; y < c.fHeight; y++ )
	{
		destPtr = destBasePtr;
		if( fRenderInfo.fFlags & kRenderItalic )
		{
			// Faux italic
			destPtr += ( c.fHeight - y ) >> 1;
		}

		for( x = 0; x < fRenderInfo.fNumCols; x++ )
		{
			for( bitMask = 0x80; bitMask != 0; bitMask >>= 1 )
			{	
				if( *src & bitMask )
					*destPtr = fRenderInfo.fColor;
				destPtr++;
			}
			src++;
		}
		destBasePtr = (UInt32 *)( (UInt8 *)destBasePtr + fRenderInfo.fDestStride );
	}
}

void	plFont::IRenderChar1To32AA( const plFont::plCharacter &c )
{
	UInt8	bitMask, *src = fBMapData + c.fBitmapOff;
	UInt32	*destPtr, *destBasePtr = (UInt32 *)( fRenderInfo.fDestPtr - ( c.fBaseline >> 1 ) * fRenderInfo.fDestStride );
	UInt16	x, y;

	
	if( ( ( (Int32)c.fHeight - (Int32)c.fBaseline ) >> 1 ) >= fRenderInfo.fMaxHeight || ( c.fBaseline >> 1 ) > fRenderInfo.fY )
		return;

	for( y = 0; y < c.fHeight; y += 2 )
	{
		destPtr = destBasePtr;
		if( fRenderInfo.fFlags & kRenderItalic )
		{
			// Faux italic
			destPtr += ( c.fHeight - y ) >> 2;
		}

		for( x = 0; x < fRenderInfo.fNumCols; x++ )
		{
			for( bitMask = 0x80; bitMask != 0; bitMask >>= 2 )
			{	
				// Grab 4 bits and do 4-to-1 AA
				UInt8 value = ( *src & bitMask ) ? 1 : 0;
				value += ( *src & ( bitMask >> 1 ) ) ? 1 : 0;
				value += ( src[ fRenderInfo.fNumCols ] & bitMask ) ? 1 : 0;
				value += ( src[ fRenderInfo.fNumCols ] & ( bitMask >> 1 ) ) ? 1 : 0;

				switch( value )
				{
					case 1:
					{
						UInt32 src = ( fRenderInfo.fColor >> 2 ) & 0x3f3f3f3f;
						UInt32 dst = ( (*destPtr) >> 2 ) & 0x3f3f3f3f;
						*destPtr = src + dst + dst + dst;					
						break;
					}
					case 2:
					{
						UInt32 src = ( fRenderInfo.fColor >> 1 ) & 0x7f7f7f7f;
						UInt32 dst = ( (*destPtr) >> 1 ) & 0x7f7f7f7f;
						*destPtr = src + dst;
						break;
					}
					case 3:
					{
						UInt32 src = ( fRenderInfo.fColor >> 2 ) & 0x3f3f3f3f;
						UInt32 dst = ( (*destPtr) >> 2 ) & 0x3f3f3f3f;
						*destPtr = src + src + src + dst;
						break;
					}
					case 4:
						*destPtr = fRenderInfo.fColor;
				}

				destPtr++;
			}
			src++;
		}
		destBasePtr = (UInt32 *)( (UInt8 *)destBasePtr + fRenderInfo.fDestStride );
		src += fRenderInfo.fNumCols;
	}
}

void	plFont::IRenderChar8To32( const plFont::plCharacter &c )
{
	UInt8	*src = fBMapData + c.fBitmapOff;
	UInt32	*destPtr, *destBasePtr = (UInt32 *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
	UInt16	x, y;
	UInt32	srcAlpha, oneMinusAlpha, r, g, b, dR, dG, dB, destAlpha, thisWidth;
	UInt8	srcR, srcG, srcB;


	// Unfortunately for some fonts, their right kern value actually is
	// farther left than the right edge of the bitmap (think of overlapping
	// script fonts). Ideally, we should store the actual width of each char's
	// bitmap and use that here. However, it really shouldn't make too big of a
	// difference, especially since the dest pixels that we end up overlapping
	// should already be in the cache. If it does, time to upgrade the font
	// format (again)
	thisWidth = fWidth;// + (Int32)c.fRightKern;

	if( (Int32)c.fHeight - (Int32)c.fBaseline >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
		return;

	srcR = (UInt8)(( fRenderInfo.fColor >> 16 ) & 0x000000ff);
	srcG = (UInt8)(( fRenderInfo.fColor >> 8  ) & 0x000000ff);
	srcB = (UInt8)(( fRenderInfo.fColor       ) & 0x000000ff);

	for( y = 0; y < c.fHeight; y++ )
	{
		destPtr = destBasePtr;
		for( x = 0; x < thisWidth; x++ )
		{
			if( src[ x ] == 255 )
				destPtr[ x ] = fRenderInfo.fColor;
			else if( src[ x ] == 0 )
				;	// Empty
			else
			{
				srcAlpha = ( src[ x ] * ( fRenderInfo.fColor >> 24 ) ) / 255;
				oneMinusAlpha = 255 - srcAlpha;

				destAlpha = destPtr[ x ] & 0xff000000;

				dR = ( destPtr[ x ] >> 16 ) & 0x000000ff;
				dG = ( destPtr[ x ] >> 8  ) & 0x000000ff;
				dB = ( destPtr[ x ]       ) & 0x000000ff;
				r = ( srcR * srcAlpha ) >> 8;
				g = ( srcG * srcAlpha ) >> 8;
				b = ( srcB * srcAlpha ) >> 8;
				dR = ( dR * oneMinusAlpha ) >> 8;
				dG = ( dG * oneMinusAlpha ) >> 8;
				dB = ( dB * oneMinusAlpha ) >> 8;

				destPtr[ x ] = ( ( r + dR ) << 16 ) | ( ( g + dG ) << 8 ) | ( b + dB ) | destAlpha;
			}
		}
		destBasePtr = (UInt32 *)( (UInt8 *)destBasePtr + fRenderInfo.fDestStride );
		src += fWidth;
	}
}

void	plFont::IRenderChar8To32FullAlpha( const plFont::plCharacter &c )
{
	UInt8	*src = fBMapData + c.fBitmapOff;
	UInt32	*destPtr, *destBasePtr = (UInt32 *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
	UInt16	x, y;
	UInt32	destColorOnly, thisWidth;


	// Unfortunately for some fonts, their right kern value actually is
	// farther left than the right edge of the bitmap (think of overlapping
	// script fonts). Ideally, we should store the actual width of each char's
	// bitmap and use that here. However, it really shouldn't make too big of a
	// difference, especially since the dest pixels that we end up overlapping
	// should already be in the cache. If it does, time to upgrade the font
	// format (again)
	thisWidth = fWidth;// + (Int32)c.fRightKern;

	if( (Int32)c.fHeight - (Int32)c.fBaseline  >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
		return;

	destColorOnly = fRenderInfo.fColor & 0x00ffffff;

	for( y = 0; y < c.fHeight; y++ )
	{
		destPtr = destBasePtr;
		for( x = 0; x < thisWidth; x++ )
		{
			if( src[ x ] != 0 )
				destPtr[ x ] = ( src[ x ] << 24 ) | destColorOnly;
		}
		destBasePtr = (UInt32 *)( (UInt8 *)destBasePtr + fRenderInfo.fDestStride );
		src += fWidth;
	}
}

void	plFont::IRenderChar8To32Alpha( const plFont::plCharacter &c )
{
	UInt8	val, *src = fBMapData + c.fBitmapOff;
	UInt32	*destPtr, *destBasePtr = (UInt32 *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
	UInt16	x, y;
	UInt32	destColorOnly, alphaMult, fullAlpha, thisWidth;


	// Unfortunately for some fonts, their right kern value actually is
	// farther left than the right edge of the bitmap (think of overlapping
	// script fonts). Ideally, we should store the actual width of each char's
	// bitmap and use that here. However, it really shouldn't make too big of a
	// difference, especially since the dest pixels that we end up overlapping
	// should already be in the cache. If it does, time to upgrade the font
	// format (again)
	thisWidth = fWidth;// + (Int32)c.fRightKern;

	if( (Int32)c.fHeight - (Int32)c.fBaseline  >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
		return;

	destColorOnly = fRenderInfo.fColor & 0x00ffffff;
	// alphaMult should come out to be a value to satisfy (fontAlpha * alphaMult >> 8) as the right alpha,
	// but then we want it so (fontAlpha * alphaMult) will be in the upper 8 bits
	fullAlpha = fRenderInfo.fColor & 0xff000000;
	alphaMult = fullAlpha / 255;

	for( y = 0; y < c.fHeight; y++ )
	{
		destPtr = destBasePtr;
		for( x = 0; x < thisWidth; x++ )
		{
			val = src[ x ];
			if( val == 0xff )
				destPtr[ x ] = fullAlpha | destColorOnly;
			else if( val != 0 )
			{
				destPtr[ x ] = ( ( alphaMult * val ) & 0xff000000 ) | destColorOnly;
			}
		}
		destBasePtr = (UInt32 *)( (UInt8 *)destBasePtr + fRenderInfo.fDestStride );
		src += fWidth;
	}
}


void	plFont::IRenderCharNull( const plCharacter &c )
{
}

//// CalcString Variations ////////////////////////////////////////////////////

UInt16	plFont::CalcStringWidth( const char *string )
{
	UInt16 w, h, a, lX, lY;
	UInt32 s;
	CalcStringExtents( string, w, h, a, s, lX, lY );
	return w;
}

UInt16	plFont::CalcStringWidth( const wchar_t *string )
{
	UInt16 w, h, a, lX, lY;
	UInt32 s;
	CalcStringExtents( string, w, h, a, s, lX, lY );
	return w;
}

void	plFont::CalcStringExtents( const char *string, UInt16 &width, UInt16 &height, UInt16 &ascent, UInt32 &firstClippedChar, UInt16 &lastX, UInt16 &lastY )
{
	// convert the char string to a wchar_t string
	wchar_t *wideString = hsStringToWString(string);
	CalcStringExtents(wideString,width,height,ascent,firstClippedChar,lastX,lastY);
	delete [] wideString;
}

void	plFont::CalcStringExtents( const wchar_t *string, UInt16 &width, UInt16 &height, UInt16 &ascent, UInt32 &firstClippedChar, UInt16 &lastX, UInt16 &lastY )
{
	IRenderString( nil, 0, 0, string, true );
	width = fRenderInfo.fFarthestX;
	height = (UInt16)(fRenderInfo.fY + fFontDescent);//fRenderInfo.fMaxDescent;
	ascent = fRenderInfo.fMaxAscent;
	lastX = fRenderInfo.fLastX;
	lastY = fRenderInfo.fLastY;

	// firstClippedChar is an index into the given string that points to the start of the part of the string
	// that got clipped (i.e. not rendered).
	firstClippedChar = (UInt32)fRenderInfo.fVolatileStringPtr - (UInt32)string;
	firstClippedChar /= 2; // divide by 2 because a wchar_t is two bytes wide, instead of one (like a char)
}

//// IGetFreeCharData /////////////////////////////////////////////////////////
//	Used for constructing fonts one character at a time; finds a pointer to
//	the first free space in our allocated font bitmap, based on the heights
//	and offsets of all the current characters

UInt8	*plFont::IGetFreeCharData( UInt32 &newOffset )
{
	Int32 i;


	newOffset = 0;
	for( i = fCharacters.GetCount() - 1; i >= 0; i-- )
	{
		UInt32 thisOff = fCharacters[ i ].fBitmapOff + ( ( fCharacters[ i ].fHeight * fWidth * fBPP ) >> 3 );
		if( newOffset < thisOff )
		{
			newOffset = thisOff;
			break;
		}
	}

	if( newOffset >= ( ( fWidth * fHeight * fBPP ) >> 3 ) )
	{
		hsAssert( false, "Invalid position found in IGetFreeCharData()" );
		return nil;
	}

	// Return pointer to the new area
	return fBMapData + newOffset;
}

//// LoadFromP2FFile //////////////////////////////////////////////////////////
//	Handy quick wrapper 

hsBool	plFont::LoadFromP2FFile( const char *path )
{
	hsUNIXStream	stream;
	if( stream.Open( path, "rb" ) )
	{
		ReadRaw( &stream );
		return true;
	}
	return false;
}

//// LoadFromFNT //////////////////////////////////////////////////////////////
//	Load this font from the data found in the given Windows FNT file,
//	using the format specified in the Windows 3 Developers Notes.

hsBool	plFont::LoadFromFNT( const char *path )
{
	hsUNIXStream	stream;		// Ahh, irony
	if( !stream.Open( path, "rb" ) )
		return false;

	return LoadFromFNTStream( &stream );
}

hsBool	plFont::LoadFromFNTStream( hsStream *stream )
{
	IClear();

	try
	{

		// Note: hsUNIXStreams just happen to store in the same endian as Windows...
		struct FNTInfo
		{
			UInt16	version;
			UInt32	size;
			char	copyright[ 60 ];
			UInt16	type;
			UInt16	points;
			UInt16	vertRes;
			UInt16	horzRes;
			UInt16	ascent;
			UInt16	internalLeading;
			UInt16	externalLeading;
			UInt8	italic, underline, strikeout;
			UInt16	weight;
			UInt8	charSet;
			UInt16	pixWidth; // 0 means variable width chars
			UInt16	pixHeight;
			UInt8	pitchFamily;
			UInt16	avgWidth;
			UInt16	maxWidth;
			UInt8	firstChar, lastChar, defaultChar, breakChar;
			UInt16	widthBytes;
			UInt32	device, face;
			UInt32	bitsPointer, bitsOffset;
			UInt8	reserved;
			UInt32	flags;
			UInt16	aSpace, bSpace, cSpace;
			UInt32	colorPointer;
			UInt8	reserved1[ 16 ];

			void	Read( hsStream *s )
			{
				version = s->ReadSwap16();
				size = s->ReadSwap32();

				s->Read( sizeof( copyright ), copyright );
			
				s->ReadSwap( &type );
				s->ReadSwap( &points );
				s->ReadSwap( &vertRes );
				s->ReadSwap( &horzRes );
				s->ReadSwap( &ascent );
				s->ReadSwap( &internalLeading );
				s->ReadSwap( &externalLeading );
				s->ReadSwap( &italic );
				s->ReadSwap( &underline );
				s->ReadSwap( &strikeout );
				s->ReadSwap( &weight );
				s->ReadSwap( &charSet );
				s->ReadSwap( &pixWidth );
				s->ReadSwap( &pixHeight );
				s->ReadSwap( &pitchFamily );
				s->ReadSwap( &avgWidth );
				s->ReadSwap( &maxWidth );
				s->ReadSwap( &firstChar );
				s->ReadSwap( &lastChar );
				s->ReadSwap( &defaultChar );
				s->ReadSwap( &breakChar );
				s->ReadSwap( &widthBytes );
				s->ReadSwap( &device );
				s->ReadSwap( &face );
				s->ReadSwap( &bitsPointer );
				s->ReadSwap( &bitsOffset );
				s->ReadSwap( &reserved );
				if( version == 0x0300 )
				{
					s->ReadSwap( &flags );
					s->ReadSwap( &aSpace );
					s->ReadSwap( &bSpace );
					s->ReadSwap( &cSpace );
					s->ReadSwap( &colorPointer );
					s->Read( sizeof( reserved1 ), reserved1 );
				}
				else
				{
					flags = 0;
					aSpace = bSpace = cSpace = 0;
					colorPointer = 0;
				}
			}
		} fntInfo;

		fntInfo.Read( stream );

		struct charEntry
		{
			UInt16	width;
			UInt32	offset;
			charEntry() { width = 0; offset = 0; }
		} *charEntries;

		int i, count = fntInfo.lastChar - fntInfo.firstChar + 2;
		charEntries = TRACKED_NEW charEntry[ count ];
		for( i = 0; i < count; i++ )
		{
			charEntries[ i ].width = stream->ReadSwap16();
			if( fntInfo.version == 0x0200 )
				charEntries[ i ].offset = stream->ReadSwap16();
			else
				charEntries[ i ].offset = stream->ReadSwap32();
		}

		char faceName[ 256 ], deviceName[ 256 ];
		if( fntInfo.face != 0 )
		{
			stream->SetPosition( fntInfo.face );
			for( i = 0; i < 256; i++ )
			{
				faceName[ i ] = stream->ReadByte();
				if( faceName[ i ] == 0 )
					break;
			}
			strncpy( fFace, faceName, sizeof( fFace ) );
		}
		if( fntInfo.device != 0 )
		{
			stream->SetPosition( fntInfo.device );
			for( i = 0; i < 256; i++ )
			{
				deviceName[ i ] = stream->ReadByte();
				if( deviceName[ i ] == 0 )
					break;
			}
		}
		fSize = (UInt8)(fntInfo.points);

		// Figure out what we need to allocate our bitmap as
		fWidth = 0;
		fHeight = 0;
		for( i = 0; i < count; i++ )
		{
			if( fWidth < charEntries[ i ].width )
				fWidth = charEntries[ i ].width;
			if( charEntries[ i ].offset > 0 )
				fHeight += fntInfo.pixHeight;
		}
		fBPP = 1;

		// Since we're 1 bbp, make sure width is a multiple of 8
		fWidth = ( ( fWidth + 7 ) >> 3 ) << 3;

		UInt32 widthInBytes = ( fWidth * fBPP ) >> 3;

		// Allocate our bitmap now
		UInt32 size = widthInBytes * fHeight;
		fBMapData = TRACKED_NEW UInt8[ size ];
		memset( fBMapData, 0, size );

		fFirstChar = fntInfo.firstChar;
		fMaxCharHeight = 0;

		// Read the bitmap info
		UInt32 destOff = 0;
		for( i = 0; i < count; i++ )
		{
			int numCols = ( charEntries[ i ].width + 7 ) >> 3;
			int height = fntInfo.pixHeight;

			// Write a record for this char
			plCharacter outChar;
			outChar.fBitmapOff = destOff;
			outChar.fHeight = ( charEntries[ i ].offset == 0 ) ? 0 : height;
			outChar.fBaseline = fntInfo.ascent;
			outChar.fLeftKern = 0.f;
			outChar.fRightKern = (hsScalar)(charEntries[ i ].width - fWidth);
			fCharacters.Append( outChar );

			if( outChar.fHeight > fMaxCharHeight )
				fMaxCharHeight = outChar.fHeight;

			if( charEntries[ i ].offset == 0 )
				continue;

			// Seek to this char
			stream->SetPosition( charEntries[ i ].offset );

			// Write the actual bitmap data. Note: FNTs store the bits one column at a time,
			// one col after another, whereas we want it plain row by row
			int col, y;
			UInt8 *basePtr = fBMapData + destOff;
			for( col = 0; col < numCols; col++ )
			{
				UInt8 *yPtr = basePtr;
				for( y = 0; y < height; y++ )
				{
					*yPtr = stream->ReadByte();
					yPtr += widthInBytes;
				}
				basePtr++;
			}

			destOff += widthInBytes * outChar.fHeight;
		}

		delete [] charEntries;

		ICalcFontAscent();
		return true;
	}
	catch( ... )
	{
		// Somehow we crashed converting!
		IClear();
		return false;
	}
}

//// LoadFromBDF //////////////////////////////////////////////////////////////
//	Load this font from the data found in the given Adobe Systems BDF file,
//	using the format specified in the Glyph Bitmap Distribution Format (BDF)
//	Specification Version 2.2 from Adobe Systems.

/*hsBool	plFont::LoadFromBDF( const char *path, plBDFConvertCallback *callback )
{
	hsUNIXStream	stream;		// Ahh, irony
	if( !stream.Open( path, "rb" ) )
		return false;

	return LoadFromBDFStream( &stream, callback );
}
*/
// Some parsing helpers
typedef int	(*fnCharTester)( int );

static int sQuoteTester( int c )
{
	return ( c == '\"' );
}

static int sDashTester( int c )
{
	return isspace( c ) || ( c == '-' );
}

class plLineParser
{
	protected:
		static char	fLine[ 512 ];

		char	*fCursor, fRestore;

		void		IAdvanceToNextToken( fnCharTester tester = isspace )
		{
			// Last cursor
			*fCursor = fRestore;

			// Scan for the start of the next token
			while( *fCursor != 0 && (*tester)( *fCursor ) )
				fCursor++;

			if( *fCursor == 0 )
				return;

			// This is the start of our token
			const char *start = fCursor;

			// Put a stopper here
			fRestore = *fCursor;
			*fCursor = 0;

			// And return!
			return;
		}

		const char	*IGetNextToken( fnCharTester tester = isspace )
		{
			// Last cursor
			*fCursor = fRestore;

			// Scan for the start of the next token
			while( *fCursor != 0 && (*tester)( *fCursor ) )
				fCursor++;

			if( *fCursor == 0 )
				return nil;

			// This is the start of our token; find the end
			const char *start = fCursor;
			while( *fCursor != 0 && !(*tester)( *fCursor ) )
				fCursor++;

			// Put a stopper here
			fRestore = *fCursor;
			*fCursor = 0;

			// And return!
			return start;
		}

	public:

		plLineParser( const char *line )
		{
			strncpy( fLine, line, sizeof( fLine ) );
			fCursor = fLine;
			fRestore = *fCursor;
		}

		~plLineParser() { }

		const char	*GetKeyword( void )
		{
			return IGetNextToken();
		}

		const char	*GetString( void )
		{
			IAdvanceToNextToken();
			return IGetNextToken( sQuoteTester );
		}

		const char	*GetKeywordNoDashes( void )
		{
			return IGetNextToken( sDashTester );
		}

		Int32		GetInt( void )
		{
			return atoi( IGetNextToken() );
		}

		hsScalar	GetFloat( void )
		{
			return (hsScalar)atof( IGetNextToken() );
		}
};

char	plLineParser::fLine[ 512 ];


// Another helper--parser for various sections of the BDF format
class plBDFSectParser
{
	protected:
		plFont					&fFont;
		plBDFConvertCallback	*fCallback;

	public:
		plBDFSectParser( plFont &myFont, plBDFConvertCallback *callback ) : fFont( myFont ), fCallback( callback ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			return nil;
		}
};


class plBDFLookForEndCharParser : public plBDFSectParser
{
	public:
		plBDFLookForEndCharParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line );
};

inline UInt8	iHexCharToByte( char c )
{
	switch( c )
	{
		case '0': return 0;
		case '1': return 1;
		case '2': return 2;
		case '3': return 3;
		case '4': return 4;
		case '5': return 5;
		case '6': return 6;
		case '7': return 7;
		case '8': return 8;
		case '9': return 9;
		case 'a': case 'A': return 10;
		case 'b': case 'B': return 11;
		case 'c': case 'C': return 12;
		case 'd': case 'D': return 13;
		case 'e': case 'E': return 14;
		case 'f': case 'F': return 15;
		default: return 0;
	}
}
inline UInt8	iHexStringToByte( const char *str )
{
	return ( iHexCharToByte( str[ 0 ] ) << 4 ) | iHexCharToByte( str[ 1 ] );
}

class plBDFCharsParser : public plBDFSectParser
{
	protected:

		// Info about the current character we're translating
		UInt16				fWhichChar, fRowsLeft;
		plFont::plCharacter	*fCharacter;
		UInt8				*fBitmap;
		hsBool				fDoingData;
		UInt32				fBytesWide, fBMapStride;

		inline void	IReset( void )
		{
			fBitmap = nil;
			fCharacter = nil;
			fWhichChar = 0;
			fDoingData = false;
		}

	public:
		static UInt32		fResolution;

		plBDFCharsParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ), fDoingData( false ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			if( strcmp( keyword, "ENDFONT" ) == 0 )
			{
				// All done!
				return nil;
			}
			else if( strcmp( keyword, "ENDCHAR" ) == 0 )
			{
				// End of the character, reset
				IReset();
				if( fCallback != nil )
					fCallback->CharDone();
			}
			else if( fDoingData )
			{
				// If we're doing data, all lines are hex values until we hit "ENDCHAR"
				if( fRowsLeft == 0 )
					throw;

				int hDigit;
				for( hDigit = 0; *keyword != 0 && hDigit < fBytesWide; hDigit++, keyword += 2 )
				{
					*fBitmap = iHexStringToByte( keyword );
					fBitmap++;
				}

				fBitmap += fBMapStride - hDigit;
				fRowsLeft--;
			}
			else if( strcmp( keyword, "STARTCHAR" ) == 0 )
			{
				// Start of a new character--ignore the name, we'll just use the encoding
				IReset();
			}
			else if( strcmp( keyword, "ENCODING" ) == 0 )
			{
				int ch = line.GetInt();
				if( ch == -1 )
				{
					// Nonstandard encoding, skip the character entirely
					return TRACKED_NEW plBDFLookForEndCharParser( fFont, fCallback );
				}
				else
				{
					fWhichChar = (UInt16)(UInt8)ch;

					// Set up our pointer. Note that since BDFs don't tell us the starting character,
					// we just make it 0
					if( fFont.fCharacters.GetCount() < fWhichChar + 1 )
						fFont.fCharacters.SetCount( fWhichChar + 1 );

					fCharacter = &fFont.fCharacters[ fWhichChar ];
				}
			}
			// Horizontal mode offsets (writing mode 0)
			else if( strcmp( keyword, "SWIDTH" ) == 0 )
			{
				// In device units, unused
//				float fWidth = (float)line.GetInt() * ( (float)fFont.GetSize() / 1000.f ) * ( fResolution / 72.f );
//				fCharacter->fRightKern = fWidth - (float)fFont.fWidth;
//				fCharacter->fLeftKern = 0.f;
			}
			else if( strcmp( keyword, "DWIDTH" ) == 0 )
			{
				// In pixels, basically the offset to the next char. Note that we only
				// support the X direction (altho admittedly the idea of a font whose characters cause
				// the next chars to go up or down is way nifty)
				fCharacter->fRightKern = (float)line.GetInt() - (float)fFont.fWidth;
				fCharacter->fLeftKern = 0.f;
			}
			// Vertical mode offsets (unsupported)
			else if( strcmp( keyword, "SWIDTH1" ) == 0 )
			{

			}
			else if( strcmp( keyword, "DWIDTH1" ) == 0 )
			{

			}
			// Bitmap bounding box
			else if( strcmp( keyword, "BBX" ) == 0 )
			{
				int pixW = line.GetInt();
				int pixH = line.GetInt();
				int xOff = line.GetInt();
				int yOff = line.GetInt();

				// Got enough info now to allocate us a bitmap for this char
				fBitmap = fFont.IGetFreeCharData( fCharacter->fBitmapOff );
				if( fBitmap == nil )
					throw false;

				if( fCharacter->fBitmapOff > ( ( fFont.fWidth * ( fFont.fHeight - pixH ) * fFont.fBPP ) >> 3 ) )
				{
					hsAssert( false, "Invalid position found in IGetFreeCharData()" );
					return nil;
				}

				// Set these now, since setting them before would've changed the IGetFreeCharData results
				fCharacter->fLeftKern = (hsScalar)xOff;
				fCharacter->fBaseline = yOff + pixH;
				fCharacter->fHeight = fRowsLeft = pixH;

				fBytesWide = ( pixW + 7 ) >> 3;
				fBMapStride = fFont.fWidth >> 3;
			}
			// Bitmap data
			else if( strcmp( keyword, "BITMAP" ) == 0 )
			{
				fDoingData = true;
			}
			// This keyword is outputted from the "getbdf" utility on linux, for some reason...
			else if( strcmp( keyword, "ATTRIBUTES" ) == 0 )
			{

			}
			else
				throw false;		// Invalid keyword

			return this;
		}
};

UInt32 plBDFCharsParser::fResolution = 72;


plBDFSectParser *plBDFLookForEndCharParser::ParseKeyword( const char *keyword, plLineParser &line )
{
	if( strcmp( keyword, "ENDCHAR" ) == 0 )
	{
		// Horray!
		return TRACKED_NEW plBDFCharsParser( fFont, fCallback );
	}
	// Just gobble lines until we find the endchar section
	return this;
}

class plBDFLookForCharParser : public plBDFSectParser
{
	public:
		plBDFLookForCharParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			if( strcmp( keyword, "CHARS" ) == 0 )
			{
				// Horray!
				return TRACKED_NEW plBDFCharsParser( fFont, fCallback );
			}
			// Just gobble lines until we find the chars section
			return this;
		}
};

class plBDFPropertiesParser : public plBDFSectParser
{
	public:
		plBDFPropertiesParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			// Note: the properties section is entirely optional and arbitrary, but we
			// parse it in case we can get more accurate info about the font name and props
			if( strcmp( keyword, "FACE_NAME" ) == 0 )
			{
				fFont.SetFace( line.GetString() );
			}
			else if( strcmp( keyword, "WEIGHT_NAME" ) == 0 )
			{
				if( stricmp( line.GetString(), "Bold" ) == 0 )
					fFont.SetFlag( plFont::kFlagBold, true );
			}
			else if( strcmp( keyword, "ENDPROPERTIES" ) == 0 )
			{
				// Switch to waiting for the chars section
				return TRACKED_NEW plBDFLookForCharParser( fFont, fCallback );
			}
			// All tokens are technically valid, even if we don't recognize them
			return this;
		}
};

class plBDFHeaderParser : public plBDFSectParser
{
	public:
		plBDFHeaderParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ) {}

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			if( strcmp( keyword, "STARTFONT" ) == 0 )
			{
				// Start of the font; check version #
				float version = line.GetFloat();
				if( version < 2.1f )
					throw false;

				// Initial font values
				fFont.fFirstChar = 0;
				fFont.fMaxCharHeight = 0;
			}
			else if( strcmp( keyword, "FONT" ) == 0 )
			{
				// Postscript-style font name: figure out our face name and hopefully attributes too
				// Format is usually of the form: vendor-face-weight-otherStuff
				const char *vendor = line.GetKeywordNoDashes();
				const char *face = line.GetKeywordNoDashes();

				fFont.SetFace( face != nil ? face : vendor );

				const char *weight = line.GetKeywordNoDashes();

				if( weight != nil && stricmp( weight, "Bold" ) == 0 )
					fFont.SetFlag( plFont::kFlagBold, true );
			}
			else if( strcmp( keyword, "COMMENT" ) == 0 )
			{
				// Comment, ignore
			}
			else if( strcmp( keyword, "CONTENTVERSION" ) == 0 )
			{
				// Content version, app specific, not used by us
			}
			else if( strcmp( keyword, "SIZE" ) == 0 )
			{
				// Point size and target resolution of this font
				fFont.SetSize( (UInt8)(line.GetInt()) );
				plBDFCharsParser::fResolution = line.GetInt();
			}
			else if( strcmp( keyword, "FONTBOUNDINGBOX" ) == 0 )
			{
				// Max dimensions and base offsets
				// Note that we should've already grabbed the width from our
				// prescan earlier, to guard against malformed BDFs
				int thisWidth = line.GetInt();
				if( fFont.fWidth < thisWidth )
					fFont.fWidth = thisWidth;

				// Since we're 1 bbp, make sure width is a multiple of 8
				fFont.fWidth = ( ( fFont.fWidth + 7 ) >> 3 ) << 3;

				// Allocate our data now
				fFont.fBMapData = TRACKED_NEW UInt8[ ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 ];
				memset( fFont.fBMapData, 0, ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 );

				fFont.fMaxCharHeight = line.GetInt();
			}
			else if( strcmp( keyword, "METRICSSET" ) == 0 )
			{
				// Metrics set specification; unsupported
			}
			else if( strcmp( keyword, "STARTPROPERTIES" ) == 0 )
			{
				// Start of the properties section, switch to that
				return TRACKED_NEW plBDFPropertiesParser( fFont, fCallback );
			}
			else if( strcmp( keyword, "CHARS" ) == 0 )
			{
				// Allocate our bitmap if we haven't already

				// Since we're 1 bbp, make sure width is a multiple of 8
				if( fFont.fBMapData == nil )
				{
					fFont.fWidth = ( ( fFont.fWidth + 7 ) >> 3 ) << 3;

					// Allocate our data now
					fFont.fBMapData = TRACKED_NEW UInt8[ ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 ];
					memset( fFont.fBMapData, 0, ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 );
				}

				// Start of the char section
				return TRACKED_NEW plBDFCharsParser( fFont, fCallback );
			}
			else
			{
				// Unrecognized token, abort
				throw false;
			}

			// Default, keep going with our parser
			return this;
		}
};

class plBDFCheckDimsParser : public plBDFSectParser
{
		hsBool	fSkipNext;

	public:

		UInt32	fMaxWidth, fMaxHeight, fNumChars, fTotalHeight;
		UInt16	fMaxChar;

		plBDFCheckDimsParser( plFont &myFont ) : plBDFSectParser( myFont, nil ) { fMaxWidth = fMaxHeight = fNumChars = fTotalHeight = 0; fSkipNext = false; fMaxChar = 0; }

		virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
		{
			if( strcmp( keyword, "ENDFONT" ) == 0 )
			{
				// All done!
				return nil;
			}
			// Encoding
			else if( strcmp( keyword, "ENCODING" ) == 0 )
			{
				int ch = line.GetInt();
				if( ch == -1 )
					// We skip these
					fSkipNext = true;
				else
				{
					fSkipNext = false;
					if( fMaxChar < ch )
						fMaxChar = ch;
				}
			}
			// Bitmap bounding box
			else if( strcmp( keyword, "BBX" ) == 0 )
			{
				if( fSkipNext )
					return this;

				int pixW = line.GetInt();
				int pixH = line.GetInt();
				int xOff = line.GetInt();
				int yOff = line.GetInt();

				if( fMaxWidth < pixW )
					fMaxWidth = pixW;
				if( fMaxHeight < pixH )
					fMaxHeight = pixH;

				fNumChars++;

				fTotalHeight += pixH;
			}
			return this;
		}
};

hsBool	plFont::LoadFromBDFStream( hsStream *stream, plBDFConvertCallback *callback )
{
	return false;
}

hsBool	plFont::LoadFromBDF( const char *path, plBDFConvertCallback *callback )
{
	FILE *fp = fopen( path, "rt" );
	if( fp == nil )
		return false;
	try
	{
		IClear();

		char	line[ 512 ];

		// Run through the entire file first with a plBDFCheckDimsParser. This is because
		// some BDFs are naughty and don't report the correct fontboundingbox (i.e. too small)
		// (See the following loop below for details on the workings of this loop)
		plBDFCheckDimsParser	checkDims( *this );
		while( fgets( line, sizeof( line ), fp ) )
		{
			plLineParser	parser( line );
			const char *keyword = parser.GetKeyword();
			if( keyword != nil )
			{
				if( checkDims.ParseKeyword( keyword, parser ) == nil )
					break;
			}
		}

		// Set up initial values
		fWidth = checkDims.fMaxWidth;
		fHeight = checkDims.fTotalHeight;
		fBPP = 1;

		// Pre-expand our char list
		fCharacters.ExpandAndZero( checkDims.fMaxChar + 1 );
		fCharacters.SetCount( 0 );

		if( callback != nil )
			callback->NumChars( (UInt16)(checkDims.fNumChars) );

		// Rewind and continue normally
		fseek( fp, 0, SEEK_SET );

		// Start with the header parser
		plBDFSectParser *currParser = TRACKED_NEW plBDFHeaderParser( *this, callback );

		// Read from the stream one line at a time (don't want comments, and char #1 should be invalid for a BDF)
		while( fgets( line, sizeof( line ), fp ) && currParser != nil )
		{
			// Parse this one
			plLineParser	parser( line );

			// Get the keyword for this line
			const char *keyword = parser.GetKeyword();
			if( keyword != nil )
			{
				// Pass on to the current parser
				plBDFSectParser *newParser = currParser->ParseKeyword( keyword, parser );
				if( newParser != currParser )
				{
					delete currParser;
					currParser = newParser;
				}
			}
		}
	}
	catch( ... )
	{
		IClear();
		return false;
	}

	fclose( fp );

	ICalcFontAscent();
	return true;
}

hsBool	plFont::ReadRaw( hsStream *s )
{
	s->Read( sizeof( fFace ), fFace );
	fSize = s->ReadByte();
	s->ReadSwap( &fFlags );

	s->ReadSwap( &fWidth );
	s->ReadSwap( &fHeight );
	s->ReadSwap( &fMaxCharHeight );

	fBPP = s->ReadByte();

	UInt32 size = ( fWidth * fHeight * fBPP ) >> 3;
	if( size > 0 )
	{
		fBMapData = TRACKED_NEW UInt8[ size ];
		s->Read( size, fBMapData );
	}
	else
		fBMapData = nil;

	s->ReadSwap( &fFirstChar );

	UInt32 i;
	fCharacters.SetCountAndZero( s->ReadSwap32() );
	for( i = 0; i < fCharacters.GetCount(); i++ )
		fCharacters[ i ].Read( s );

	ICalcFontAscent();

	return true;
}

hsBool	plFont::WriteRaw( hsStream *s )
{
	s->Write( sizeof( fFace ), fFace );
	s->WriteByte( fSize );
	s->WriteSwap( fFlags );

	s->WriteSwap( fWidth );
	s->WriteSwap( fHeight );
	s->WriteSwap( fMaxCharHeight );

	s->WriteByte( fBPP );

	UInt32 size = ( fWidth * fHeight * fBPP ) >> 3;
	if( size > 0 )
		s->Write( size, fBMapData );

	s->WriteSwap( fFirstChar );

	UInt32 i;
	s->WriteSwap32( fCharacters.GetCount() );
	for( i = 0; i < fCharacters.GetCount(); i++ )
		fCharacters[ i ].Write( s );

	return true;
}
