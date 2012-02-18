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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plFont Class Header                                                      //
//  Seems like we've come full circle again. This is our generic Plasma      //
//  bitmap font class/format. Quick list of things it supports, or needs to: //
//      - Antialiasing, either in the font def or at rendertime              //
//      - Doublebyte character sets                                          //
//      - Platform independence, of course                                   //
//      - Render to reasonably arbitrary mipmap                              //
//      - Character-level kerning, both before and after, as well as         //
//        negative kerning (for ligatures)                                   //
//                                                                           //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  3.4.2003 mcn - Created.                                                  //
//  4.3.2003 mcn - Updated. Casting a char to a uint16_t sign-extends it if    //
//                 the char is > 128, but casting it to an uint8_t first works.//
//                 Ugly as sin, but hey, so are you.                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
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

void    plFont::plCharacter::Read( hsStream *s )
{
    // Rocket science here...
    s->ReadLE( &fBitmapOff );
    s->ReadLE( &fHeight );
    s->ReadLE( &fBaseline );
    s->ReadLE( &fLeftKern );
    s->ReadLE( &fRightKern );
}

void    plFont::plCharacter::Write( hsStream *s )
{
    s->WriteLE( fBitmapOff );
    s->WriteLE( fHeight );
    s->WriteLE( fBaseline );
    s->WriteLE( fLeftKern );
    s->WriteLE( fRightKern );
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

void    plFont::IClear( bool onConstruct )
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

void    plFont::SetFace( const char *face )
{
    strncpy( fFace, face, sizeof( fFace ) );
}

void    plFont::SetSize( uint8_t size )
{
    fSize = size;
}

void    plFont::Read( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Read( s, mgr );
    ReadRaw( s );
}

void    plFont::Write( hsStream *s, hsResMgr *mgr )
{
    hsKeyedObject::Write( s, mgr );
    WriteRaw( s );
}

///////////////////////////////////////////////////////////////////////////////
//// Rendering ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

void    plFont::SetRenderColor( uint32_t color )
{
    fRenderInfo.fColor = color;
}

void    plFont::SetRenderClipRect( int16_t x, int16_t y, int16_t width, int16_t height )
{
    fRenderInfo.fClipRect.Set( x, y, width, height );
}

void    plFont::SetRenderClipping( int16_t x, int16_t y, int16_t width, int16_t height )
{
    SetRenderFlag( kRenderWrap, false );
    SetRenderFlag( kRenderClip, true );
    SetRenderClipRect( x, y, width, height );
}

void    plFont::SetRenderWrapping( int16_t x, int16_t y, int16_t width, int16_t height )
{
    SetRenderFlag( kRenderWrap, true );
    SetRenderFlag( kRenderClip, false );
    SetRenderClipRect( x, y, width, height );
}

void    plFont::ICalcFontAscent( void )
{
    uint32_t  i;


    // Hack for now, only calc the ascent for characters in the 127 character ASCII range
    for( i = 0, fFontAscent = 0, fFontDescent = 0, fMaxCharHeight = 0; i < fCharacters.GetCount(); i++ )
    {
        if( i + fFirstChar < 128 && fFontAscent < fCharacters[ i ].fBaseline )
            fFontAscent = fCharacters[ i ].fBaseline;

        int32_t descent = fCharacters[ i ].fHeight - fCharacters[ i ].fBaseline;
        if( fFontDescent < descent )
            fFontDescent = descent;

        if( fMaxCharHeight < fCharacters[ i ].fHeight )
            fMaxCharHeight = fCharacters[ i ].fHeight;
    }
}

//// IIsWordBreaker //////////////////////////////////////////////////////////
//  Returns whether the given character is one that can break a line

static inline bool  IIsWordBreaker( const char c )
{
    return ( strchr( " \t,.;\n", c ) != nil ) ? true : false;
}

//// IIsDrawableWordBreak /////////////////////////////////////////////////////
// Returns whether the given character is a line breaker that has to be drawn
// (non whitespace)

static inline bool  IIsDrawableWordBreak( const char c )
{
    return ( strchr( ",.;", c ) != nil ) ? true : false;
}

//// RenderString /////////////////////////////////////////////////////////////
//  The base render function. Additional options are specified externally,
//  so that their effects can be cached for optimization

void    plFont::RenderString( plMipmap *mip, uint16_t x, uint16_t y, const char *string, uint16_t *lastX, uint16_t *lastY )
{
    // convert the char string to a wchar_t string
    wchar_t *wideString = hsStringToWString(string);
    RenderString(mip,x,y,wideString,lastX,lastY);
    delete [] wideString;
}


void    plFont::RenderString( plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, uint16_t *lastX, uint16_t *lastY )
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

void    plFont::IRenderString( plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, hsBool justCalc )
{
    fRenderInfo.fMipmap = mip;
    fRenderInfo.fX = x;
    fRenderInfo.fY = y;
    fRenderInfo.fNumCols = (int16_t)(( fBPP <= 8 ) ? ( ( fWidth * fBPP ) >> 3 ) : 0);
    fRenderInfo.fFloatWidth = (float)fWidth;
    fRenderInfo.fFarthestX = x;
    fRenderInfo.fMaxAscent = 0;
    fRenderInfo.fVolatileStringPtr = string;

    switch( fRenderInfo.fFlags & kRenderJustYMask )
    {
        case kRenderJustYTop: 
            fRenderInfo.fY += (int16_t)fFontAscent; 
            break;
        case kRenderJustYBottom:
            if( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) )
                fRenderInfo.fY = (int16_t)(fRenderInfo.fClipRect.GetBottom() - 1 - fMaxCharHeight + fFontAscent);
            else
                fRenderInfo.fY = (int16_t)(mip->GetHeight() - 1 - fMaxCharHeight + fFontAscent);
            break;
        case kRenderJustYCenter:
            fRenderInfo.fY = (int16_t)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetBottom() - 1 : mip->GetHeight() - 1);
            fRenderInfo.fY = (int16_t)(( fRenderInfo.fY - fMaxCharHeight ) >> 1);
            fRenderInfo.fY += (int16_t)fFontAscent;
            break;
        default:    // Just the baseline
            ;
    }

    if( justCalc )
    {
        plCharacter &ch = fCharacters[ (uint16_t)string[ 0 ] - fFirstChar ];
        fRenderInfo.fX = fRenderInfo.fFarthestX = x - (int16_t)ch.fLeftKern;
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
//                  plCharacter &ch = fCharacters[ (uint16_t)(uint8_t)string[ 0 ] - fFirstChar ];
//                  int32_t newX = x - (int16_t)ch.fLeftKern;
//                  if( newX < 0 )
//                      newX = 0;
//                  fRenderInfo.fX = fRenderInfo.fFarthestX = newX;
                }
                break;
            case kRenderJustXCenter:
                {
                    uint16_t right = (uint16_t)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetRight() : mip->GetWidth());
//                  uint16_t width = CalcStringWidth( string );
                    fRenderInfo.fX = fRenderInfo.fFarthestX = ( ( x + right ) >> 1 );// - ( width >> 1 );
                }
                break;
            case kRenderJustXRight:
                {
                    uint16_t width = 0, right = (uint16_t)(( fRenderInfo.fFlags & ( kRenderClip | kRenderWrap ) ) ? fRenderInfo.fClipRect.GetRight() : mip->GetWidth());
/*                  if( fRenderInfo.fFlags & kRenderClip )
                    {
                        fRenderInfo.fFlags &= ~kRenderClip;
                        width = CalcStringWidth( string );
                        fRenderInfo.fFlags |= kRenderClip;
                    }
                    else
*/                      width = 0;//CalcStringWidth( string );
                    fRenderInfo.fX = fRenderInfo.fFarthestX = right - width;
                }
                break;
        }
    }

    // Choose an optimal rendering function
    fRenderInfo.fRenderFunc = nil;
    if( justCalc )
        fRenderInfo.fRenderFunc = &plFont::IRenderCharNull;
    else if( mip->GetPixelSize() == 32 )
    {
        if( fBPP == 1 )
            fRenderInfo.fRenderFunc = ( fRenderInfo.fFlags & kRenderScaleAA ) ? &plFont::IRenderChar1To32AA : &plFont::IRenderChar1To32;
        else if( fBPP == 8 )
        {
            if( fRenderInfo.fFlags & kRenderIntoAlpha )
            {
                if( ( fRenderInfo.fColor & 0xff000000 ) != 0xff000000 )
                    fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32Alpha;
                else
                    fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32FullAlpha;
            }
            else
                fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32;
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
        fRenderInfo.fDestPtr = (uint8_t *)mip->GetImage();
        fRenderInfo.fDestPtr += fRenderInfo.fY * fRenderInfo.fDestStride;
        fRenderInfo.fDestPtr += fRenderInfo.fX * fRenderInfo.fDestBPP;
    }
    if( fRenderInfo.fFlags & ( kRenderWrap | kRenderClip ) )
    {
        fRenderInfo.fMaxHeight = (int16_t)( fRenderInfo.fClipRect.GetBottom() - fRenderInfo.fY );
        fRenderInfo.fMaxWidth = (int16_t)( fRenderInfo.fClipRect.GetRight() - x );
    }
    else if( justCalc )
    {
        // Just calculating, no wrapping, so the max is as far as we can go
        // Note: 32767 isn't quite right, since we'll be adding the left kern in before we
        // calc the first character, so adjust so we make sure we don't underflow
        plCharacter &ch = fCharacters[ (uint16_t)(uint8_t)string[ 0 ] - fFirstChar ];

        fRenderInfo.fMaxHeight = (int16_t)fMaxCharHeight;
        fRenderInfo.fMaxWidth = (int16_t)32767 + (int16_t)ch.fLeftKern;
    }
    else
    {
        fRenderInfo.fMaxHeight = (int16_t)( mip->GetHeight() - fRenderInfo.fY );
        fRenderInfo.fMaxWidth = (int16_t)( mip->GetWidth() - x );
    }

    fRenderInfo.fMaxDescent = 0;

    if( fRenderInfo.fFlags & kRenderWrap )
    {
        // Hell, gotta uint16_t wrap the text
        // To avoid backtracking, we step forward in the string one uint16_t at a time until we hit a break,
        // then render what we have and continue
        int32_t lastWord = 0, i;
        bool isFirstLine = true;
        x = 0;
        int16_t firstMaxAscent = 0;
        uint32_t lineHt, lineDelta;
        if( fRenderInfo.fFlags & kRenderScaleAA )
            lineHt = fMaxCharHeight >> 1;
        else
            lineHt = fMaxCharHeight;
        
        // adjust the line height for spacing
        lineHt += fRenderInfo.fLineSpacing;

        lineDelta = lineHt * fRenderInfo.fDestStride;

        while( *string != 0 && fRenderInfo.fMaxHeight >= fFontDescent )
        {
            uint8_t *destStartPtr = fRenderInfo.fDestPtr;
            uint32_t destStartX = fRenderInfo.fX;
            int16_t destMaxWidth = fRenderInfo.fMaxWidth, thisIndent = 0;

            if( isFirstLine )
            {
                // First line, apply indent if applicable
                fRenderInfo.fX += fRenderInfo.fFirstLineIndent;
                fRenderInfo.fMaxWidth -= fRenderInfo.fFirstLineIndent;
                fRenderInfo.fDestPtr += fRenderInfo.fFirstLineIndent * fRenderInfo.fDestBPP;
                thisIndent = fRenderInfo.fFirstLineIndent;
                isFirstLine = false;
            }

            std::string ellipsisTracker = ""; // keeps track of ellipsis, since there are three uint16_t break chars that can't be split
            bool possibleEllipsis = false;
            int preEllipsisLastWord = 0; // where the uint16_t break was before we started tracking an ellipsis

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
                if (fCharacters.Count() <= ((uint16_t)string[i] - fFirstChar))
                    charToDraw = &(fCharacters[(uint16_t)L' ' - fFirstChar]);
                else
                    charToDraw = &(fCharacters[(uint16_t)string[i] - fFirstChar]);

                int16_t leftKern = (int16_t)charToDraw->fLeftKern;
                if( fRenderInfo.fFlags & kRenderScaleAA )
                    x += leftKern / 2;
                else
                    x += leftKern;

                // Update our position and see if we're over
                // Note that our wrapping is slightly off, in that it doesn't take into account
                // the left kerning of characters. Hopefully that won't matter much...
                uint16_t charWidth = (uint16_t)(fWidth + (int16_t)charToDraw->fRightKern);
                if( fRenderInfo.fFlags & kRenderScaleAA )
                    charWidth >>= 1;

                uint16_t nonAdjustedX = (uint16_t)(x + fWidth); // just in case the actual bitmap is too big to fit on page and we think the character can (because of right kern)
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

                // Are we a uint16_t breaker?
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
                    // Yes, and we didn't go over, so store as the last successfully fit uint16_t and move on
                    lastWord = i;
                }           
            }

            if( string[ i ] == 0 )
                lastWord = i;       // Final catch for end-of-string
            else if( lastWord == 0 && string[ i ] != L'\n' && thisIndent == 0 )
                lastWord = i;       // Catch for a single uint16_t that didn't fit (just go up as many chars as we can)
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
                    uint16_t baseX = fRenderInfo.fX, baseMaxW = fRenderInfo.fMaxWidth;
                    uint8_t *baseDestPtr = fRenderInfo.fDestPtr;

                    fRenderInfo.fX = 0;
                    CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
                    fRenderInfo.fRenderFunc = &plFont::IRenderCharNull;

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
                    uint16_t baseX = fRenderInfo.fX, baseMaxW = fRenderInfo.fMaxWidth;
                    uint8_t *baseDestPtr = fRenderInfo.fDestPtr;

                    fRenderInfo.fX = 0;
                    CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
                    fRenderInfo.fRenderFunc = &plFont::IRenderCharNull;

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
                    int16_t baseX = fRenderInfo.fX;

                    plCharacter &ch = fCharacters[ (uint16_t)string[ 0 ] - fFirstChar ];

                    fRenderInfo.fX -= (int16_t)ch.fLeftKern;
                    fRenderInfo.fDestPtr -= (int16_t)ch.fLeftKern * fRenderInfo.fDestBPP;

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
            fRenderInfo.fX = (int16_t)destStartX;
            fRenderInfo.fDestPtr = destStartPtr;
            fRenderInfo.fMaxWidth = destMaxWidth;
            fRenderInfo.fMaxAscent = 0;

            // Look for the next non-word-breaker. Note that if we have any carriage returns hidden in here, 
            // we'll want to be advancing down even further
            // Advance down
            if( string[ i ] != 0 )
            {
                fRenderInfo.fY += (int16_t)lineHt;
                fRenderInfo.fMaxHeight -= (int16_t)lineHt;
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
                    fRenderInfo.fY += (int16_t)lineHt;
                    fRenderInfo.fMaxHeight -= (int16_t)lineHt;
                    fRenderInfo.fDestPtr += lineDelta;
                    fRenderInfo.fLastY = fRenderInfo.fY;
                }
            }

            // Keep going from here!
            string += i;
        }

        fRenderInfo.fMaxAscent = firstMaxAscent;    
        // Final y offset from the last line
        fRenderInfo.fY += (int16_t)fFontDescent;

        fRenderInfo.fVolatileStringPtr = string; 
    }
    else
    {
        if( fRenderInfo.fFlags & kRenderClip )
        {
            // Advance left past any clipping area
            CharRenderFunc oldFunc = fRenderInfo.fRenderFunc;
            fRenderInfo.fRenderFunc = &plFont::IRenderCharNull;
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
            plCharacter &ch = fCharacters[ (uint16_t)string[ 0 ] - fFirstChar ];
            int32_t newX = x - (int16_t)ch.fLeftKern;
            if( newX < 0 )
                newX = 0;
            fRenderInfo.fX = fRenderInfo.fFarthestX = (int16_t)newX;
        }

        fRenderInfo.fVolatileStringPtr = string;    // Just so we can keep track of when we clip
        IRenderLoop( string, -1 );
        fRenderInfo.fFarthestX = fRenderInfo.fX;
    }
}

void    plFont::IRenderLoop( const wchar_t *string, int32_t maxCount )
{
    // Render the string straight across, one char at a time
    while( *string != 0 && maxCount != 0 )  // Note: if maxCount starts out -1, then it'll just keep 
    {                                       // decrementing...well ok, not for forever, but pretty close
        uint16_t c = (uint16_t)*string;
        if( c < fFirstChar )
            ;       // Invalid char
        else
        {
            c -= fFirstChar;
            if( c >= fCharacters.GetCount() )
                ;   // Invalid char
            else
            {
                // First pass at supporting left kerning values, but only at the pixel level
                int16_t leftKern = (int16_t)fCharacters[ c ].fLeftKern;
                if( leftKern != 0 )
                {
                    if( fRenderInfo.fFlags & kRenderScaleAA )
                        leftKern /= 2;

                    fRenderInfo.fX += leftKern;
                    fRenderInfo.fMaxWidth -= leftKern;
                    fRenderInfo.fDestPtr += leftKern * fRenderInfo.fDestBPP;
                }

                uint16_t thisWidth = (uint16_t)(fWidth + fCharacters[ c ].fRightKern);
                if( fRenderInfo.fFlags & kRenderScaleAA )
                    thisWidth >>= 1;

                if( thisWidth >= fRenderInfo.fMaxWidth )
                    break;

                (this->*(fRenderInfo.fRenderFunc))( fCharacters[ c ] );

                fRenderInfo.fX += thisWidth;
                fRenderInfo.fMaxWidth -= thisWidth;
                fRenderInfo.fDestPtr += thisWidth * fRenderInfo.fDestBPP;


                int16_t baseline = (int16_t)(fCharacters[ c ].fBaseline);
                if( fRenderInfo.fFlags & kRenderScaleAA )
                    baseline >>= 1;

                if( baseline > fRenderInfo.fMaxAscent )
                    fRenderInfo.fMaxAscent = baseline;

                int16_t thisHt = (int16_t)(fCharacters[ c ].fHeight - baseline);
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

void    plFont::IRenderChar1To32( const plFont::plCharacter &c )
{
    uint8_t   bitMask, *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
    uint16_t  x, y;

    
    if( (int32_t)c.fHeight - (int32_t)c.fBaseline >= fRenderInfo.fMaxHeight || c.fBaseline > fRenderInfo.fY )
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
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
    }
}

void    plFont::IRenderChar1To32AA( const plFont::plCharacter &c )
{
    uint8_t   bitMask, *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)( fRenderInfo.fDestPtr - ( c.fBaseline >> 1 ) * fRenderInfo.fDestStride );
    uint16_t  x, y;

    
    if( ( ( (int32_t)c.fHeight - (int32_t)c.fBaseline ) >> 1 ) >= fRenderInfo.fMaxHeight || ( c.fBaseline >> 1 ) > fRenderInfo.fY )
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
                uint8_t value = ( *src & bitMask ) ? 1 : 0;
                value += ( *src & ( bitMask >> 1 ) ) ? 1 : 0;
                value += ( src[ fRenderInfo.fNumCols ] & bitMask ) ? 1 : 0;
                value += ( src[ fRenderInfo.fNumCols ] & ( bitMask >> 1 ) ) ? 1 : 0;

                switch( value )
                {
                    case 1:
                    {
                        uint32_t src = ( fRenderInfo.fColor >> 2 ) & 0x3f3f3f3f;
                        uint32_t dst = ( (*destPtr) >> 2 ) & 0x3f3f3f3f;
                        *destPtr = src + dst + dst + dst;                   
                        break;
                    }
                    case 2:
                    {
                        uint32_t src = ( fRenderInfo.fColor >> 1 ) & 0x7f7f7f7f;
                        uint32_t dst = ( (*destPtr) >> 1 ) & 0x7f7f7f7f;
                        *destPtr = src + dst;
                        break;
                    }
                    case 3:
                    {
                        uint32_t src = ( fRenderInfo.fColor >> 2 ) & 0x3f3f3f3f;
                        uint32_t dst = ( (*destPtr) >> 2 ) & 0x3f3f3f3f;
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
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
        src += fRenderInfo.fNumCols;
    }
}

void    plFont::IRenderChar8To32( const plFont::plCharacter &c )
{
    uint8_t   *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
    uint16_t  x, y;
    uint32_t  srcAlpha, oneMinusAlpha, r, g, b, dR, dG, dB, destAlpha, thisWidth;
    uint8_t   srcR, srcG, srcB;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;

    if( (int32_t)c.fHeight - (int32_t)c.fBaseline >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
        return;

    srcR = (uint8_t)(( fRenderInfo.fColor >> 16 ) & 0x000000ff);
    srcG = (uint8_t)(( fRenderInfo.fColor >> 8  ) & 0x000000ff);
    srcB = (uint8_t)(( fRenderInfo.fColor       ) & 0x000000ff);

    for( y = 0; y < c.fHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = 0; x < thisWidth; x++ )
        {
            if( src[ x ] == 255 )
                destPtr[ x ] = fRenderInfo.fColor;
            else if( src[ x ] == 0 )
                ;   // Empty
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
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
        src += fWidth;
    }
}

void    plFont::IRenderChar8To32FullAlpha( const plFont::plCharacter &c )
{
    uint8_t   *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
    uint16_t  x, y;
    uint32_t  destColorOnly, thisWidth;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;

    if( (int32_t)c.fHeight - (int32_t)c.fBaseline  >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
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
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
        src += fWidth;
    }
}

void    plFont::IRenderChar8To32Alpha( const plFont::plCharacter &c )
{
    uint8_t   val, *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)( fRenderInfo.fDestPtr - c.fBaseline * fRenderInfo.fDestStride );
    uint16_t  x, y;
    uint32_t  destColorOnly, alphaMult, fullAlpha, thisWidth;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;

    if( (int32_t)c.fHeight - (int32_t)c.fBaseline  >= fRenderInfo.fMaxHeight || thisWidth >= fRenderInfo.fMaxWidth || c.fBaseline > fRenderInfo.fY )
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
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
        src += fWidth;
    }
}


void    plFont::IRenderCharNull( const plCharacter &c )
{
}

//// CalcString Variations ////////////////////////////////////////////////////

uint16_t  plFont::CalcStringWidth( const char *string )
{
    uint16_t w, h, a, lX, lY;
    uint32_t s;
    CalcStringExtents( string, w, h, a, s, lX, lY );
    return w;
}

uint16_t  plFont::CalcStringWidth( const wchar_t *string )
{
    uint16_t w, h, a, lX, lY;
    uint32_t s;
    CalcStringExtents( string, w, h, a, s, lX, lY );
    return w;
}

void    plFont::CalcStringExtents( const char *string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint32_t &firstClippedChar, uint16_t &lastX, uint16_t &lastY )
{
    // convert the char string to a wchar_t string
    wchar_t *wideString = hsStringToWString(string);
    CalcStringExtents(wideString,width,height,ascent,firstClippedChar,lastX,lastY);
    delete [] wideString;
}

void    plFont::CalcStringExtents( const wchar_t *string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint32_t &firstClippedChar, uint16_t &lastX, uint16_t &lastY )
{
    IRenderString( nil, 0, 0, string, true );
    width = fRenderInfo.fFarthestX;
    height = (uint16_t)(fRenderInfo.fY + fFontDescent);//fRenderInfo.fMaxDescent;
    ascent = fRenderInfo.fMaxAscent;
    lastX = fRenderInfo.fLastX;
    lastY = fRenderInfo.fLastY;

    // firstClippedChar is an index into the given string that points to the start of the part of the string
    // that got clipped (i.e. not rendered).
    firstClippedChar = (uintptr_t)fRenderInfo.fVolatileStringPtr - (uintptr_t)string;
    firstClippedChar /= 2; // divide by 2 because a wchar_t is two bytes wide, instead of one (like a char)
}

//// IGetFreeCharData /////////////////////////////////////////////////////////
//  Used for constructing fonts one character at a time; finds a pointer to
//  the first free space in our allocated font bitmap, based on the heights
//  and offsets of all the current characters

uint8_t   *plFont::IGetFreeCharData( uint32_t &newOffset )
{
    int32_t i;


    newOffset = 0;
    for( i = fCharacters.GetCount() - 1; i >= 0; i-- )
    {
        uint32_t thisOff = fCharacters[ i ].fBitmapOff + ( ( fCharacters[ i ].fHeight * fWidth * fBPP ) >> 3 );
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
//  Handy quick wrapper 

hsBool  plFont::LoadFromP2FFile( const char *path )
{
    hsUNIXStream    stream;
    if( stream.Open( path, "rb" ) )
    {
        ReadRaw( &stream );
        return true;
    }
    return false;
}

//// LoadFromFNT //////////////////////////////////////////////////////////////
//  Load this font from the data found in the given Windows FNT file,
//  using the format specified in the Windows 3 Developers Notes.

hsBool  plFont::LoadFromFNT( const char *path )
{
    hsUNIXStream    stream;     // Ahh, irony
    if( !stream.Open( path, "rb" ) )
        return false;

    return LoadFromFNTStream( &stream );
}

hsBool  plFont::LoadFromFNTStream( hsStream *stream )
{
    IClear();

    try
    {

        // Note: hsUNIXStreams just happen to store in the same endian as Windows...
        struct FNTInfo
        {
            uint16_t  version;
            uint32_t  size;
            char    copyright[ 60 ];
            uint16_t  type;
            uint16_t  points;
            uint16_t  vertRes;
            uint16_t  horzRes;
            uint16_t  ascent;
            uint16_t  internalLeading;
            uint16_t  externalLeading;
            uint8_t   italic, underline, strikeout;
            uint16_t  weight;
            uint8_t   charSet;
            uint16_t  pixWidth; // 0 means variable width chars
            uint16_t  pixHeight;
            uint8_t   pitchFamily;
            uint16_t  avgWidth;
            uint16_t  maxWidth;
            uint8_t   firstChar, lastChar, defaultChar, breakChar;
            uint16_t  widthBytes;
            uint32_t  device, face;
            uint32_t  bitsPointer, bitsOffset;
            uint8_t   reserved;
            uint32_t  flags;
            uint16_t  aSpace, bSpace, cSpace;
            uint32_t  colorPointer;
            uint8_t   reserved1[ 16 ];

            void    Read( hsStream *s )
            {
                version = s->ReadLE16();
                size = s->ReadLE32();

                s->Read( sizeof( copyright ), copyright );
            
                s->ReadLE( &type );
                s->ReadLE( &points );
                s->ReadLE( &vertRes );
                s->ReadLE( &horzRes );
                s->ReadLE( &ascent );
                s->ReadLE( &internalLeading );
                s->ReadLE( &externalLeading );
                s->ReadLE( &italic );
                s->ReadLE( &underline );
                s->ReadLE( &strikeout );
                s->ReadLE( &weight );
                s->ReadLE( &charSet );
                s->ReadLE( &pixWidth );
                s->ReadLE( &pixHeight );
                s->ReadLE( &pitchFamily );
                s->ReadLE( &avgWidth );
                s->ReadLE( &maxWidth );
                s->ReadLE( &firstChar );
                s->ReadLE( &lastChar );
                s->ReadLE( &defaultChar );
                s->ReadLE( &breakChar );
                s->ReadLE( &widthBytes );
                s->ReadLE( &device );
                s->ReadLE( &face );
                s->ReadLE( &bitsPointer );
                s->ReadLE( &bitsOffset );
                s->ReadLE( &reserved );
                if( version == 0x0300 )
                {
                    s->ReadLE( &flags );
                    s->ReadLE( &aSpace );
                    s->ReadLE( &bSpace );
                    s->ReadLE( &cSpace );
                    s->ReadLE( &colorPointer );
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
            uint16_t  width;
            uint32_t  offset;
            charEntry() { width = 0; offset = 0; }
        } *charEntries;

        int i, count = fntInfo.lastChar - fntInfo.firstChar + 2;
        charEntries = new charEntry[ count ];
        for( i = 0; i < count; i++ )
        {
            charEntries[ i ].width = stream->ReadLE16();
            if( fntInfo.version == 0x0200 )
                charEntries[ i ].offset = stream->ReadLE16();
            else
                charEntries[ i ].offset = stream->ReadLE32();
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
        fSize = (uint8_t)(fntInfo.points);

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

        uint32_t widthInBytes = ( fWidth * fBPP ) >> 3;

        // Allocate our bitmap now
        uint32_t size = widthInBytes * fHeight;
        fBMapData = new uint8_t[ size ];
        memset( fBMapData, 0, size );

        fFirstChar = fntInfo.firstChar;
        fMaxCharHeight = 0;

        // Read the bitmap info
        uint32_t destOff = 0;
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
            outChar.fRightKern = (float)(charEntries[ i ].width - fWidth);
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
            uint8_t *basePtr = fBMapData + destOff;
            for( col = 0; col < numCols; col++ )
            {
                uint8_t *yPtr = basePtr;
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
//  Load this font from the data found in the given Adobe Systems BDF file,
//  using the format specified in the Glyph Bitmap Distribution Format (BDF)
//  Specification Version 2.2 from Adobe Systems.

/*hsBool    plFont::LoadFromBDF( const char *path, plBDFConvertCallback *callback )
{
    hsUNIXStream    stream;     // Ahh, irony
    if( !stream.Open( path, "rb" ) )
        return false;

    return LoadFromBDFStream( &stream, callback );
}
*/
// Some parsing helpers
typedef int (*fnCharTester)( int );

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
        static char fLine[ 512 ];

        char    *fCursor, fRestore;

        void        IAdvanceToNextToken( fnCharTester tester = isspace )
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

        const char  *IGetNextToken( fnCharTester tester = isspace )
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

        const char  *GetKeyword( void )
        {
            return IGetNextToken();
        }

        const char  *GetString( void )
        {
            IAdvanceToNextToken();
            return IGetNextToken( sQuoteTester );
        }

        const char  *GetKeywordNoDashes( void )
        {
            return IGetNextToken( sDashTester );
        }

        int32_t       GetInt( void )
        {
            return atoi( IGetNextToken() );
        }

        float    GetFloat( void )
        {
            return (float)atof( IGetNextToken() );
        }
};

char    plLineParser::fLine[ 512 ];


// Another helper--parser for various sections of the BDF format
class plBDFSectParser
{
    protected:
        plFont                  &fFont;
        plBDFConvertCallback    *fCallback;

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

inline uint8_t    iHexCharToByte( char c )
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
inline uint8_t    iHexStringToByte( const char *str )
{
    return ( iHexCharToByte( str[ 0 ] ) << 4 ) | iHexCharToByte( str[ 1 ] );
}

class plBDFCharsParser : public plBDFSectParser
{
    protected:

        // Info about the current character we're translating
        uint16_t              fWhichChar, fRowsLeft;
        plFont::plCharacter *fCharacter;
        uint8_t               *fBitmap;
        hsBool              fDoingData;
        uint32_t              fBytesWide, fBMapStride;

        inline void IReset( void )
        {
            fBitmap = nil;
            fCharacter = nil;
            fWhichChar = 0;
            fDoingData = false;
        }

    public:
        static uint32_t       fResolution;

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
                    return new plBDFLookForEndCharParser( fFont, fCallback );
                }
                else
                {
                    fWhichChar = (uint16_t)(uint8_t)ch;

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
//              float fWidth = (float)line.GetInt() * ( (float)fFont.GetSize() / 1000.f ) * ( fResolution / 72.f );
//              fCharacter->fRightKern = fWidth - (float)fFont.fWidth;
//              fCharacter->fLeftKern = 0.f;
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
                fCharacter->fLeftKern = (float)xOff;
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
                throw false;        // Invalid keyword

            return this;
        }
};

uint32_t plBDFCharsParser::fResolution = 72;


plBDFSectParser *plBDFLookForEndCharParser::ParseKeyword( const char *keyword, plLineParser &line )
{
    if( strcmp( keyword, "ENDCHAR" ) == 0 )
    {
        // Horray!
        return new plBDFCharsParser( fFont, fCallback );
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
                return new plBDFCharsParser( fFont, fCallback );
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
                return new plBDFLookForCharParser( fFont, fCallback );
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
                fFont.SetSize( (uint8_t)(line.GetInt()) );
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
                fFont.fBMapData = new uint8_t[ ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 ];
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
                return new plBDFPropertiesParser( fFont, fCallback );
            }
            else if( strcmp( keyword, "CHARS" ) == 0 )
            {
                // Allocate our bitmap if we haven't already

                // Since we're 1 bbp, make sure width is a multiple of 8
                if( fFont.fBMapData == nil )
                {
                    fFont.fWidth = ( ( fFont.fWidth + 7 ) >> 3 ) << 3;

                    // Allocate our data now
                    fFont.fBMapData = new uint8_t[ ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 ];
                    memset( fFont.fBMapData, 0, ( fFont.fWidth * fFont.fHeight * fFont.fBPP ) >> 3 );
                }

                // Start of the char section
                return new plBDFCharsParser( fFont, fCallback );
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
        hsBool  fSkipNext;

    public:

        uint32_t  fMaxWidth, fMaxHeight, fNumChars, fTotalHeight;
        uint16_t  fMaxChar;

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

hsBool  plFont::LoadFromBDFStream( hsStream *stream, plBDFConvertCallback *callback )
{
    return false;
}

hsBool  plFont::LoadFromBDF( const char *path, plBDFConvertCallback *callback )
{
    FILE *fp = fopen( path, "rt" );
    if( fp == nil )
        return false;
    try
    {
        IClear();

        char    line[ 512 ];

        // Run through the entire file first with a plBDFCheckDimsParser. This is because
        // some BDFs are naughty and don't report the correct fontboundingbox (i.e. too small)
        // (See the following loop below for details on the workings of this loop)
        plBDFCheckDimsParser    checkDims( *this );
        while( fgets( line, sizeof( line ), fp ) )
        {
            plLineParser    parser( line );
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
            callback->NumChars( (uint16_t)(checkDims.fNumChars) );

        // Rewind and continue normally
        fseek( fp, 0, SEEK_SET );

        // Start with the header parser
        plBDFSectParser *currParser = new plBDFHeaderParser( *this, callback );

        // Read from the stream one line at a time (don't want comments, and char #1 should be invalid for a BDF)
        while( fgets( line, sizeof( line ), fp ) && currParser != nil )
        {
            // Parse this one
            plLineParser    parser( line );

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
        fclose( fp );
        return false;
    }

    fclose( fp );

    ICalcFontAscent();
    return true;
}

hsBool  plFont::ReadRaw( hsStream *s )
{
    s->Read( sizeof( fFace ), fFace );
    fSize = s->ReadByte();
    s->ReadLE( &fFlags );

    s->ReadLE( &fWidth );
    s->ReadLE( &fHeight );
    s->ReadLE( &fMaxCharHeight );

    fBPP = s->ReadByte();

    uint32_t size = ( fWidth * fHeight * fBPP ) >> 3;
    if( size > 0 )
    {
        fBMapData = new uint8_t[ size ];
        s->Read( size, fBMapData );
    }
    else
        fBMapData = nil;

    s->ReadLE( &fFirstChar );

    uint32_t i;
    fCharacters.SetCountAndZero( s->ReadLE32() );
    for( i = 0; i < fCharacters.GetCount(); i++ )
        fCharacters[ i ].Read( s );

    ICalcFontAscent();

    return true;
}

hsBool  plFont::WriteRaw( hsStream *s )
{
    s->Write( sizeof( fFace ), fFace );
    s->WriteByte( fSize );
    s->WriteLE( fFlags );

    s->WriteLE( fWidth );
    s->WriteLE( fHeight );
    s->WriteLE( fMaxCharHeight );

    s->WriteByte( fBPP );

    uint32_t size = ( fWidth * fHeight * fBPP ) >> 3;
    if( size > 0 )
        s->Write( size, fBMapData );

    s->WriteLE( fFirstChar );

    uint32_t i;
    s->WriteLE32( fCharacters.GetCount() );
    for( i = 0; i < fCharacters.GetCount(); i++ )
        fCharacters[ i ].Write( s );

    return true;
}
