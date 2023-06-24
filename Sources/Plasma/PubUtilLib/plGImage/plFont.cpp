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
#include <string>

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
    s->ReadLE32(&fBitmapOff);
    s->ReadLE32(&fHeight);
    s->ReadLE32(&fBaseline);
    s->ReadLEFloat(&fLeftKern);
    s->ReadLEFloat(&fRightKern);
}

void plFont::plCharacter::Write(hsStream *s) const
{
    s->WriteLE32(fBitmapOff);
    s->WriteLE32(fHeight);
    s->WriteLE32(fBaseline);
    s->WriteLEFloat(fLeftKern);
    s->WriteLEFloat(fRightKern);
}

//// Constructor/Read/Write/Destructor/etc ////////////////////////////////////

plFont::plFont()
    : fFontAscent(), fFontDescent()
{
    IClear(true);
}

plFont::~plFont()
{
    IClear();
}

void    plFont::IClear( bool onConstruct )
{
    if( !onConstruct )
        delete [] fBMapData;

    fFace = ST::string();
    fSize = 0;
    fFlags = 0;

    fWidth = fHeight = 0;
    fBPP = 0;
    fBMapData = nullptr;
    fFirstChar = 0;
    fMaxCharHeight = 0;
    fCharacters.clear();

    fRenderInfo.fFlags = 0;
    fRenderInfo.fX = fRenderInfo.fY = fRenderInfo.fNumCols = 0;
    fRenderInfo.fMaxWidth = fRenderInfo.fMaxHeight = 0;
    fRenderInfo.fDestPtr = nullptr;
    fRenderInfo.fDestStride = 0;
    fRenderInfo.fColor = 0;
    fRenderInfo.fMipmap = nullptr;
    fRenderInfo.fRenderFunc = nullptr;
    fRenderInfo.fVolatileStringPtr = nullptr;
    fRenderInfo.fFirstLineIndent = 0;
    fRenderInfo.fLineSpacing = 0;
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

void    plFont::ICalcFontAscent()
{
    // Hack for now, only calc the ascent for characters in the 127 character ASCII range
    fFontAscent = 0;
    fFontDescent = 0;
    fMaxCharHeight = 0;
    for (size_t i = 0; i < fCharacters.size(); i++)
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
    return (strchr(" \t,.;\n", c) != nullptr);
}

//// IIsDrawableWordBreak /////////////////////////////////////////////////////
// Returns whether the given character is a line breaker that has to be drawn
// (non whitespace)

static inline bool  IIsDrawableWordBreak( const char c )
{
    return (strchr(",.;", c) != nullptr);
}

//// RenderString /////////////////////////////////////////////////////////////
//  The base render function. Additional options are specified externally,
//  so that their effects can be cached for optimization

void    plFont::RenderString( plMipmap *mip, uint16_t x, uint16_t y, const ST::string &string, uint16_t *lastX, uint16_t *lastY )
{
    // TEMP
    RenderString(mip, x, y, string.to_wchar().data(), lastX, lastY);
}


void    plFont::RenderString( plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, uint16_t *lastX, uint16_t *lastY )
{
    if( mip->IsCompressed() )
    {
        hsAssert( false, "Unable to render string to compressed mipmap" );
        return;
    }

    IRenderString( mip, x, y, string, false );
    if (lastX != nullptr)
        *lastX = fRenderInfo.fLastX;
    if (lastY != nullptr)
        *lastY = fRenderInfo.fLastY;
}

void    plFont::IRenderString( plMipmap *mip, uint16_t x, uint16_t y, const wchar_t *string, bool justCalc )
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
    fRenderInfo.fRenderFunc = nullptr;
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
                if( fRenderInfo.fFlags & kRenderAlphaPremultiplied )
                {
                    if (fRenderInfo.fFlags & kRenderShadow)
                        fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32AlphaPremShadow;
                    else
                        fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32AlphaPremultiplied;
                }
                else if( ( fRenderInfo.fColor & 0xff000000 ) != 0xff000000 )
                    fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32Alpha;
                else
                    fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32FullAlpha;
            }
            else
                fRenderInfo.fRenderFunc = &plFont::IRenderChar8To32;
        }
    }

    if (fRenderInfo.fRenderFunc == nullptr)
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
                plCharacter* charToDraw = nullptr;
                if (fCharacters.size() <= ((uint16_t)string[i] - fFirstChar))
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
            int16_t prevX;
            do
            {
                prevX = fRenderInfo.fX;
                IRenderLoop( string, 1 );
            }
            while( fRenderInfo.fX <= fRenderInfo.fClipRect.fX && *++string != 0 );
            fRenderInfo.fMaxWidth += fRenderInfo.fX - prevX;
            fRenderInfo.fDestPtr -= (fRenderInfo.fX - prevX) * fRenderInfo.fDestBPP;
            fRenderInfo.fX = prevX;
            fRenderInfo.fRenderFunc = oldFunc;
        }

        // There used to be an adjustment of the X coordinate by -ch.fLeftKern for the case
        // of kRenderJustXForceLeft here, but it was buggy in that it neglected to adjust
        // fRenderInfo.fDestPtr and therefore had no visible effect (or almost none - only
        // at the end of the line). Fixing the bug however (making kRenderJustXForceLeft
        // work as intended) causes the text shadow to be cut off in some places. To ensure
        // enough space for the shadow, and considering that all content was developed and
        // visually optimized with the bug in place, it seems better to preserve the buggy
        // behavior and make kRenderJustXForceLeft work exactly like kRenderJustXLeft.

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
            if (c >= fCharacters.size())
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
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
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
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - (c.fBaseline >> 1) * int32_t(fRenderInfo.fDestStride));
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
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
    int16_t   x, y, thisHeight, xstart, thisWidth;
    uint32_t  srcAlpha, oneMinusAlpha, r, g, b, dR, dG, dB, destAlpha;
    uint8_t   srcR, srcG, srcB;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;
    if( thisWidth >= fRenderInfo.fMaxWidth )
        thisWidth = fRenderInfo.fMaxWidth;

    xstart = fRenderInfo.fClipRect.fX - fRenderInfo.fX;
    if( xstart < 0 )
        xstart = 0;

    srcR = (uint8_t)(( fRenderInfo.fColor >> 16 ) & 0x000000ff);
    srcG = (uint8_t)(( fRenderInfo.fColor >> 8  ) & 0x000000ff);
    srcB = (uint8_t)(( fRenderInfo.fColor       ) & 0x000000ff);

    y = fRenderInfo.fClipRect.fY - fRenderInfo.fY + (int16_t)c.fBaseline;
    if( y < 0 )
        y = 0;
    else
    {
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + y*fRenderInfo.fDestStride );
        src += y*fRenderInfo.fNumCols;
    }

    thisHeight = fRenderInfo.fMaxHeight + (int16_t)c.fBaseline;
    if( thisHeight > (int16_t)c.fHeight )
        thisHeight = (int16_t)c.fHeight;

    for( ; y < thisHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = xstart; x < thisWidth; x++ )
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
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
    int16_t   x, y, thisHeight, xstart, thisWidth;
    uint32_t  destColorOnly;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;
    if( thisWidth >= fRenderInfo.fMaxWidth )
        thisWidth = fRenderInfo.fMaxWidth;

    xstart = fRenderInfo.fClipRect.fX - fRenderInfo.fX;
    if( xstart < 0 )
        xstart = 0;

    destColorOnly = fRenderInfo.fColor & 0x00ffffff;

    y = fRenderInfo.fClipRect.fY - fRenderInfo.fY + (int16_t)c.fBaseline;
    if( y < 0 )
        y = 0;
    else
    {
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + y*fRenderInfo.fDestStride );
        src += y*fRenderInfo.fNumCols;
    }

    thisHeight = fRenderInfo.fMaxHeight + (int16_t)c.fBaseline;
    if( thisHeight > (int16_t)c.fHeight )
        thisHeight = (int16_t)c.fHeight;

    for( ; y < thisHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = xstart; x < thisWidth; x++ )
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
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
    int16_t   x, y, thisHeight, xstart, thisWidth;
    uint32_t  destColorOnly, alphaMult, fullAlpha;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;
    if( thisWidth >= fRenderInfo.fMaxWidth )
        thisWidth = fRenderInfo.fMaxWidth;

    xstart = fRenderInfo.fClipRect.fX - fRenderInfo.fX;
    if( xstart < 0 )
        xstart = 0;

    destColorOnly = fRenderInfo.fColor & 0x00ffffff;
    // alphaMult should come out to be a value to satisfy (fontAlpha * alphaMult >> 8) as the right alpha,
    // but then we want it so (fontAlpha * alphaMult) will be in the upper 8 bits
    fullAlpha = fRenderInfo.fColor & 0xff000000;
    alphaMult = fullAlpha / 255;

    y = fRenderInfo.fClipRect.fY - fRenderInfo.fY + (int16_t)c.fBaseline;
    if( y < 0 )
        y = 0;
    else
    {
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + y*fRenderInfo.fDestStride );
        src += y*fRenderInfo.fNumCols;
    }

    thisHeight = fRenderInfo.fMaxHeight + (int16_t)c.fBaseline;
    if( thisHeight > (int16_t)c.fHeight )
        thisHeight = (int16_t)c.fHeight;

    for( ; y < thisHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = xstart; x < thisWidth; x++ )
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

void    plFont::IRenderChar8To32AlphaPremultiplied( const plFont::plCharacter &c )
{
    uint8_t   *src = fBMapData + c.fBitmapOff;
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
    int16_t   x, y, thisHeight, xstart, thisWidth;
    uint8_t   srcA, srcR, srcG, srcB;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth;// + (int32_t)c.fRightKern;
    if( thisWidth >= fRenderInfo.fMaxWidth )
        thisWidth = fRenderInfo.fMaxWidth;

    xstart = fRenderInfo.fClipRect.fX - fRenderInfo.fX;
    if( xstart < 0 )
        xstart = 0;

    srcA = (uint8_t)(( fRenderInfo.fColor >> 24 ) & 0x000000ff);
    srcR = (uint8_t)(( fRenderInfo.fColor >> 16 ) & 0x000000ff);
    srcG = (uint8_t)(( fRenderInfo.fColor >> 8  ) & 0x000000ff);
    srcB = (uint8_t)(( fRenderInfo.fColor       ) & 0x000000ff);

    y = fRenderInfo.fClipRect.fY - fRenderInfo.fY + (int16_t)c.fBaseline;
    if( y < 0 )
        y = 0;
    else
    {
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + y*fRenderInfo.fDestStride );
        src += y*fRenderInfo.fNumCols;
    }

    thisHeight = fRenderInfo.fMaxHeight + (int16_t)c.fBaseline;
    if( thisHeight > (int16_t)c.fHeight )
        thisHeight = (int16_t)c.fHeight;

    for( ; y < thisHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = xstart; x < thisWidth; x++ )
        {
            uint32_t a = src[ x ];
            if (a != 0)
            {
                if (srcA != 0xff)
                    a = (srcA*a + 127)/255;
                destPtr[ x ] = ( a << 24 ) | (((srcR*a + 127)/255) << 16) | (((srcG*a + 127)/255) << 8) | ((srcB*a + 127)/255);
            }
        }
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
        src += fWidth;
    }
}

void    plFont::IRenderChar8To32AlphaPremShadow( const plFont::plCharacter &c )
{
    uint32_t  *destPtr, *destBasePtr = (uint32_t *)(fRenderInfo.fDestPtr - c.fBaseline * int32_t(fRenderInfo.fDestStride));
    int16_t   x, y, thisHeight, xstart, thisWidth;
    uint8_t   srcA, srcR, srcG, srcB;


    // Unfortunately for some fonts, their right kern value actually is
    // farther left than the right edge of the bitmap (think of overlapping
    // script fonts). Ideally, we should store the actual width of each char's
    // bitmap and use that here. However, it really shouldn't make too big of a
    // difference, especially since the dest pixels that we end up overlapping
    // should already be in the cache. If it does, time to upgrade the font
    // format (again)
    thisWidth = fWidth + 2;// + (int32_t)c.fRightKern;
    if( thisWidth >= fRenderInfo.fMaxWidth )
        thisWidth = fRenderInfo.fMaxWidth;

    xstart = fRenderInfo.fClipRect.fX - fRenderInfo.fX;
    if( xstart < -2 )
        xstart = -2;

    srcA = (uint8_t)(( fRenderInfo.fColor >> 24 ) & 0x000000ff);
    if (srcA == 0)
        return;
    srcR = (uint8_t)(( fRenderInfo.fColor >> 16 ) & 0x000000ff);
    srcG = (uint8_t)(( fRenderInfo.fColor >> 8  ) & 0x000000ff);
    srcB = (uint8_t)(( fRenderInfo.fColor       ) & 0x000000ff);

    static const uint32_t kernel[5][5] = {
        {1,  2,  2,  2, 1},
        {1, 13, 13, 13, 1},
        {1, 10, 10, 10, 1},
        {1,  7,  7,  7, 1},
        {1,  1,  1,  1, 1}
    };

    uint32_t clamp = 220 - ((2 * srcR + 4 * srcG + srcB) >> 4);

    y = fRenderInfo.fClipRect.fY - fRenderInfo.fY + (int16_t)c.fBaseline;
    if( y < -2 )
        y = -2;
    destBasePtr = (uint32_t *)((uint8_t *)destBasePtr + y * int32_t(fRenderInfo.fDestStride));

    thisHeight = fRenderInfo.fMaxHeight + (int16_t)c.fBaseline;
    if( thisHeight > (int16_t)c.fHeight + 2 )
        thisHeight = (int16_t)c.fHeight + 2;

    for( ; y < thisHeight; y++ )
    {
        destPtr = destBasePtr;
        for( x = xstart; x < thisWidth; x++ )
        {
            uint32_t sa = 0;
            for (int32_t i = -2; i <= 2; i++) {
                for (int32_t j = -2; j <= 2; j++) {
                    uint32_t m = kernel[j+2][i+2];
                    if (m != 0)
                        sa += m * IGetCharPixel(c, x+i, y+j);
                }
            }
            sa = (sa * clamp) >> 13;
            if (sa > clamp)
                sa = clamp;
            uint32_t a = IGetCharPixel(c, x, y);
            if (srcA != 0xff)
            {
                a = (srcA * a + 127) / 255;
                sa = (srcA * sa + 127) / 255;
            }
            uint32_t ta = a + sa - ((a*sa + 127) / 255);
            if (ta > (destPtr[ x ] >> 24))
                destPtr[ x ] = ( ta << 24 ) | (((srcR * a + 127) / 255) << 16) |
                                              (((srcG * a + 127) / 255) <<  8) |
                                              (((srcB * a + 127) / 255) <<  0);
        }
        destBasePtr = (uint32_t *)( (uint8_t *)destBasePtr + fRenderInfo.fDestStride );
    }
}


void    plFont::IRenderCharNull( const plCharacter &c )
{
}

//// CalcString Variations ////////////////////////////////////////////////////

uint16_t  plFont::CalcStringWidth( const ST::string &string )
{
    uint16_t w, h, a, lX, lY;
    CalcStringExtents( string, w, h, a, lX, lY );
    return w;
}

uint16_t  plFont::CalcStringWidth( const wchar_t *string )
{
    uint16_t w, h, a, lX, lY;
    uint32_t s;
    CalcStringExtents( string, w, h, a, s, lX, lY );
    return w;
}

void    plFont::CalcStringExtents( const ST::string &string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint16_t &lastX, uint16_t &lastY )
{
    // convert the char string to a wchar_t string
    // We don't expose firstClippedChar as an out parameter in the ST::string overload,
    // because converting it to a correct UTF-8-based count is a bit complicated
    // and currently nothing uses this information.
    uint32_t firstClippedChar;
    CalcStringExtents(string.to_wchar().data(), width, height, ascent, firstClippedChar, lastX, lastY);
}

void    plFont::CalcStringExtents( const wchar_t *string, uint16_t &width, uint16_t &height, uint16_t &ascent, uint32_t &firstClippedChar, uint16_t &lastX, uint16_t &lastY )
{
    IRenderString(nullptr, 0, 0, string, true);
    width = fRenderInfo.fFarthestX;
    height = (uint16_t)(fRenderInfo.fY + fFontDescent);//fRenderInfo.fMaxDescent;
    ascent = fRenderInfo.fMaxAscent;
    lastX = fRenderInfo.fLastX;
    lastY = fRenderInfo.fLastY;

    // firstClippedChar is an index into the given string that points to the start of the part of the string
    // that got clipped (i.e. not rendered).
    firstClippedChar = fRenderInfo.fVolatileStringPtr - string;
}

//// IGetFreeCharData /////////////////////////////////////////////////////////
//  Used for constructing fonts one character at a time; finds a pointer to
//  the first free space in our allocated font bitmap, based on the heights
//  and offsets of all the current characters

uint8_t   *plFont::IGetFreeCharData( uint32_t &newOffset )
{
    newOffset = 0;
    for (hsSsize_t i = fCharacters.size() - 1; i >= 0; i--)
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
        return nullptr;
    }

    // Return pointer to the new area
    return fBMapData + newOffset;
}

//// LoadFromP2FFile //////////////////////////////////////////////////////////
//  Handy quick wrapper 

bool    plFont::LoadFromP2FFile( const plFileName &path )
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

bool    plFont::LoadFromFNT( const plFileName &path )
{
    hsUNIXStream    stream;     // Ahh, irony
    if( !stream.Open( path, "rb" ) )
        return false;

    return LoadFromFNTStream( &stream );
}

bool    plFont::LoadFromFNTStream( hsStream *stream )
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
            
                s->ReadLE16(&type);
                s->ReadLE16(&points);
                s->ReadLE16(&vertRes);
                s->ReadLE16(&horzRes);
                s->ReadLE16(&ascent);
                s->ReadLE16(&internalLeading);
                s->ReadLE16(&externalLeading);
                s->ReadByte(&italic);
                s->ReadByte(&underline);
                s->ReadByte(&strikeout);
                s->ReadLE16(&weight);
                s->ReadByte(&charSet);
                s->ReadLE16(&pixWidth);
                s->ReadLE16(&pixHeight);
                s->ReadByte(&pitchFamily);
                s->ReadLE16(&avgWidth);
                s->ReadLE16(&maxWidth);
                s->ReadByte(&firstChar);
                s->ReadByte(&lastChar);
                s->ReadByte(&defaultChar);
                s->ReadByte(&breakChar);
                s->ReadLE16(&widthBytes);
                s->ReadLE32(&device);
                s->ReadLE32(&face);
                s->ReadLE32(&bitsPointer);
                s->ReadLE32(&bitsOffset);
                s->ReadByte(&reserved);
                if( version == 0x0300 )
                {
                    s->ReadLE32(&flags);
                    s->ReadLE16(&aSpace);
                    s->ReadLE16(&bSpace);
                    s->ReadLE16(&cSpace);
                    s->ReadLE32(&colorPointer);
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

        char faceName[ 257 ], deviceName[ 256 ];
        if( fntInfo.face != 0 )
        {
            stream->SetPosition( fntInfo.face );
            for( i = 0; i < 256; i++ )
            {
                faceName[ i ] = stream->ReadByte();
                if( faceName[ i ] == 0 )
                    break;
            }
            faceName[256] = 0;
            fFace = faceName;
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
            fCharacters.emplace_back(outChar);

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
    catch (std::exception &e)
    {
        printf("Exception caught in plFont::LoadFromFNTStream: %s\n", e.what());
        IClear();
        return false;
    }
    catch (...)
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

/*bool      plFont::LoadFromBDF( const char *path, plBDFConvertCallback *callback )
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
                return nullptr;

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

        const char  *GetKeyword()
        {
            return IGetNextToken();
        }

        const char  *GetString()
        {
            IAdvanceToNextToken();
            return IGetNextToken( sQuoteTester );
        }

        const char  *GetKeywordNoDashes()
        {
            return IGetNextToken( sDashTester );
        }

        int32_t       GetInt()
        {
            return atoi( IGetNextToken() );
        }

        float    GetFloat()
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

        virtual ~plBDFSectParser() { }

        virtual plBDFSectParser *ParseKeyword( const char *keyword, plLineParser &line )
        {
            return nullptr;
        }
};


class plBDFLookForEndCharParser : public plBDFSectParser
{
    public:
        plBDFLookForEndCharParser( plFont &myFont, plBDFConvertCallback *callback ) : plBDFSectParser( myFont, callback ) {}

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override;
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
        bool                fDoingData;
        uint32_t              fBytesWide, fBMapStride;

        inline void IReset()
        {
            fBitmap = nullptr;
            fCharacter = nullptr;
            fWhichChar = 0;
            fDoingData = false;
        }

    public:
        static uint32_t       fResolution;

        plBDFCharsParser(plFont& myFont, plBDFConvertCallback* callback)
            : plBDFSectParser(myFont, callback), fDoingData(), fWhichChar(),
              fRowsLeft(), fCharacter(), fBitmap(), fBytesWide(), fBMapStride()
        { }

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override
        {
            if( strcmp( keyword, "ENDFONT" ) == 0 )
            {
                // All done!
                return nullptr;
            }
            else if( strcmp( keyword, "ENDCHAR" ) == 0 )
            {
                // End of the character, reset
                IReset();
                if (fCallback != nullptr)
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
                    if (fFont.fCharacters.size() < fWhichChar + 1)
                        fFont.fCharacters.resize(fWhichChar + 1);

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
                if (fBitmap == nullptr)
                    throw false;

                if( fCharacter->fBitmapOff > ( ( fFont.fWidth * ( fFont.fHeight - pixH ) * fFont.fBPP ) >> 3 ) )
                {
                    hsAssert( false, "Invalid position found in IGetFreeCharData()" );
                    return nullptr;
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

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override
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

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override
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

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override
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

                fFont.SetFace(face != nullptr ? face : vendor);

                const char *weight = line.GetKeywordNoDashes();

                if (weight != nullptr && stricmp(weight, "Bold") == 0)
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
                if (fFont.fBMapData == nullptr)
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
        bool    fSkipNext;

    public:

        uint32_t  fMaxWidth, fMaxHeight, fNumChars, fTotalHeight;
        uint16_t  fMaxChar;

        plBDFCheckDimsParser(plFont &myFont) : plBDFSectParser(myFont, nullptr) { fMaxWidth = fMaxHeight = fNumChars = fTotalHeight = 0; fSkipNext = false; fMaxChar = 0; }

        plBDFSectParser *ParseKeyword(const char *keyword, plLineParser &line) override
        {
            if( strcmp( keyword, "ENDFONT" ) == 0 )
            {
                // All done!
                return nullptr;
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

bool    plFont::LoadFromBDFStream( hsStream *stream, plBDFConvertCallback *callback )
{
    return false;
}

bool    plFont::LoadFromBDF( const plFileName &path, plBDFConvertCallback *callback )
{
    FILE *fp = fopen( path.AsString().c_str(), "rt" );
    if (fp == nullptr)
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
            if (keyword != nullptr)
            {
                if (checkDims.ParseKeyword(keyword, parser) == nullptr)
                    break;
            }
        }

        // Set up initial values
        fWidth = checkDims.fMaxWidth;
        fHeight = checkDims.fTotalHeight;
        fBPP = 1;

        // Pre-expand our char list
        fCharacters.clear();
        fCharacters.reserve(checkDims.fMaxChar + 1);

        if (callback != nullptr)
            callback->NumChars( (uint16_t)(checkDims.fNumChars) );

        // Rewind and continue normally
        fseek( fp, 0, SEEK_SET );

        // Start with the header parser
        plBDFSectParser *currParser = new plBDFHeaderParser( *this, callback );

        // Read from the stream one line at a time (don't want comments, and char #1 should be invalid for a BDF)
        while (fgets(line, sizeof(line), fp) && currParser != nullptr)
        {
            // Parse this one
            plLineParser    parser( line );

            // Get the keyword for this line
            const char *keyword = parser.GetKeyword();
            if (keyword != nullptr)
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
    catch (std::exception &e)
    {
        printf("Exception caught in plFont::LoadFromBDF: %s\n", e.what());
        IClear();
        fclose(fp);
        return false;
    }
    catch (...)
    {
        IClear();
        fclose( fp );
        return false;
    }

    fclose( fp );

    ICalcFontAscent();
    return true;
}

bool    plFont::ReadRaw( hsStream *s )
{
    char face_buf[257];
    s->Read(256, face_buf);
    face_buf[256] = 0;
    fFace = face_buf;

    fSize = s->ReadByte();
    s->ReadLE32(&fFlags);

    s->ReadLE32(&fWidth);
    s->ReadLE32(&fHeight);
    s->ReadLE32(&fMaxCharHeight);

    fBPP = s->ReadByte();

    uint32_t size = ( fWidth * fHeight * fBPP ) >> 3;
    if( size > 0 )
    {
        fBMapData = new uint8_t[ size ];
        s->Read( size, fBMapData );
    }
    else
        fBMapData = nullptr;

    s->ReadLE16(&fFirstChar);

    fCharacters.resize(s->ReadLE32());
    for (plCharacter& ch : fCharacters)
        ch.Read(s);

    ICalcFontAscent();

    return true;
}

bool    plFont::WriteRaw( hsStream *s )
{
    char face_buf[256] = { 0 };
    memcpy(face_buf, fFace.c_str(), fFace.size() * sizeof(char));
    s->Write(sizeof(face_buf), face_buf);

    s->WriteByte( fSize );
    s->WriteLE32(fFlags);

    s->WriteLE32(fWidth);
    s->WriteLE32(fHeight);
    s->WriteLE32(fMaxCharHeight);

    s->WriteByte( fBPP );

    uint32_t size = ( fWidth * fHeight * fBPP ) >> 3;
    if( size > 0 )
        s->Write( size, fBMapData );

    s->WriteLE16(fFirstChar);

    s->WriteLE32((uint32_t)fCharacters.size());
    for (const plCharacter& ch : fCharacters)
        ch.Write(s);

    return true;
}
