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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfGUIMultiLineEditCtrl Definition                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfGUIMultiLineEditCtrl.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include <memory>

#include "pfGameGUIMgr.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIUpDownPairMod.h"

#include "pnMessage/plRefMsg.h"

#include "plAnimation/plAGModifier.h"
#include "plClipboard/plClipboard.h"
#include "plInputCore/plInputInterface.h"
#include "plMessage/plAnimCmdMsg.h"
#include "plGImage/plDynamicTextMap.h"

#include "pfMessage/pfGameGUIMsg.h"

//// Tiny Helper Class ///////////////////////////////////////////////////////

class plStringSlicer
{
    wchar_t         *fString;
    wchar_t         fTempChar;
    size_t          fStart, fEnd;

    typedef wchar_t *CharPtr;

public:
    plStringSlicer(std::vector<wchar_t> &string, size_t start, size_t end)
    {
        fString = string.data();
        fTempChar = 0L;
        fStart = start;
        if (end < string.size())
            fEnd = end;
        else
            fEnd = fStart;

        if (fEnd > fStart)
        {
            fTempChar = fString[end];
            fString[end] = 0L;
        }
    }

    ~plStringSlicer()
    {
        if (fEnd > fStart)
            fString[fEnd] = fTempChar;
    }

    operator CharPtr() const
    {
        return &fString[fStart];
    }
};

//// Wee Little Control Proc for scrolling ///////////////////////////////////

class pfMLScrollProc : public pfGUICtrlProcObject
{
    protected:

        pfGUIMultiLineEditCtrl  *fParent;

    public:

        pfMLScrollProc( pfGUIMultiLineEditCtrl *parent ) : fParent( parent ) {}

        void    DoSomething(pfGUIControlMod *ctrl) override
        {
            // Do a check here to make sure we actually changed scroll
            // positions--if not, we don't want to update, since that'll be 
            // slow as hell
            int newScrollPos = (int)fParent->fScrollControl->GetMax() - (int)fParent->fScrollControl->GetCurrValue();
            fParent->SetScrollPosition( newScrollPos );
        }
};


//// Constants ///////////////////////////////////////////////////////////////

constexpr wchar_t kColorCodeChar = (wchar_t)1;
constexpr wchar_t kStyleCodeChar = (wchar_t)2;
constexpr wchar_t kLinkCodeChar = (wchar_t)3;
constexpr size_t kColorCodeSize = 5;
constexpr size_t kStyleCodeSize = 3;
constexpr size_t kLinkCodeSize = 3;

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIMultiLineEditCtrl::pfGUIMultiLineEditCtrl()
    : fBuffer({L'\0'}), fCursorPos(), fLastCursorLine(),
      fClickedLinkId(-1), fCurrLinkId(-1),
      fBufferLimit(-1), fScrollControl(), fScrollProc(), fScrollPos(),
      fReadyToRender(), fLastKeyModifiers(), fLastKeyPressed(), fLockCount(),
      fNextCtrl(), fPrevCtrl(), fEventProc(),
      fTopMargin(), fLeftMargin(), fBottomMargin(), fRightMargin(),
      fFontSize(), fFontStyle(), fFontFlagsSet(), fCanUpdate(true),
      fLineHeight(), fCurrCursorX(), fCurrCursorY(), fCalcedFontSize()
{
    SetFlag(kWantsInterest);
    SetFlag(kTakesSpecialKeys);
    fFontColor.FromARGB32(0xFF000000);
}

pfGUIMultiLineEditCtrl::~pfGUIMultiLineEditCtrl()
{
    ClearNext(); // make sure that no one is referencing us
    ClearPrev();

    if( fScrollProc && fScrollProc->DecRef() )
        delete fScrollProc;
    delete fEventProc;
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIMultiLineEditCtrl::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIMultiLineEditCtrl::MsgReceive( plMessage *msg )
{
    plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
    if (refMsg != nullptr)
    {
        if( refMsg->fType == kRefScrollCtrl )
        {
            if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
            {
                fScrollControl = pfGUIValueCtrl::ConvertNoRef( refMsg->GetRef() );
                fScrollControl->SetHandler( fScrollProc );
                fScrollControl->SetStep( 1.f );
                IUpdateScrollRange();
            }
            else
                fScrollControl = nullptr;
            return true;
        }
    }

    return pfGUIControlMod::MsgReceive( msg );
}

//// SetScrollPosition ///////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::SetScrollPosition( int32_t topLine )
{
    topLine = std::clamp(topLine, 0, (int32_t)fLineStarts.size() - ICalcNumVisibleLines() + 1);

    if( fScrollPos == topLine )
        return;

    if (topLine >= 0)
        fScrollPos = topLine;
    else
        fScrollPos = 0;

    IUpdate();

    if (fScrollControl != nullptr)
        // Scroll control values are reversed
        fScrollControl->SetCurrValue( fScrollControl->GetMax() - (float)fScrollPos );

    HandleExtendedEvent( pfGUIMultiLineEditCtrl::kScrollPosChanged );

    // notify thru the dialog something has changed
    if (fDialog && fDialog->GetHandler())
        fDialog->GetHandler()->DoSomething(this);
}

//// GetScrollPosition ///////////////////////////////////////////////////////

int32_t   pfGUIMultiLineEditCtrl::GetScrollPosition()
{

    return fScrollPos;
}

//// MoveCursor - by direction command////////////////////////////////////////////////
void    pfGUIMultiLineEditCtrl::MoveCursor( Direction dir )
{
    IMoveCursor(dir);
}


//// IUpdateScrollRange //////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::IUpdateScrollRange()
{
    if (fScrollControl == nullptr)
        return;

    if ((int32_t)fLineStarts.size() > ICalcNumVisibleLines() - 1)
    {
        // +1 here because the last visible line is only a partial, but we want to be able to view
        // full lines all the way to the end.
        int32_t newMax = (int32_t)fLineStarts.size() - ICalcNumVisibleLines() + 1;

        if ((float)newMax != fScrollControl->GetMax())
        {
            fScrollControl->SetRange(0, (float)newMax);
            fScrollControl->SetEnabled(true);
            if (fScrollPos > newMax)
            {
                fScrollPos = newMax;
                fScrollControl->SetCurrValue( fScrollControl->GetMax() - (float)fScrollPos );
            }

            // All bets are off on scrolling, so refresh the whole area
            IUpdate();
        }
    }
    else
    {
        if( fScrollControl->GetMax() > 0 )
        {
            fScrollControl->SetRange( 0, 0 );
            fScrollControl->SetEnabled( false );
            fScrollPos = 0;
            IUpdate();
        }
    }
}

void pfGUIMultiLineEditCtrl::SetScrollEnable( bool state )
{
    if (fScrollControl == nullptr)
        return;

    if ( state )
    {
        IUpdateScrollRange();
    }
    else
    {
        fScrollControl->SetRange( 0, 0 );
        fScrollControl->SetEnabled( false );
        fScrollPos = 0;
        IUpdate();
    }
}

//// IPostSetUpDynTextMap ////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::IPostSetUpDynTextMap()
{
    pfGUIColorScheme *scheme = GetColorScheme();

    // fill in any blanks
    if (!(fFontFlagsSet & kFontFaceSet))
    {
        fFontFace = scheme->fFontFace;
        fFontFlagsSet |= kFontFaceSet;
    }
    if (!(fFontFlagsSet & kFontColorSet))
    {
        fFontColor = scheme->fForeColor;
        fFontFlagsSet |= kFontColorSet;
    }
    if (!(fFontFlagsSet & kFontSizeSet))
    {
        fFontSize = scheme->fFontSize;
        fFontFlagsSet |= kFontSizeSet;
    }
    if (!(fFontFlagsSet & kFontStyleSet))
    {
        fFontStyle = scheme->fFontFlags;
        fFontFlagsSet |= kFontStyleSet;
    }

    fDynTextMap->SetFont( fFontFace, fFontSize, fFontStyle,
                            HasFlag( kXparentBgnd ) ? false : true );

    // Calculate a height for each line
    fDynTextMap->CalcStringWidth( L"The quick brown fox jumped over the lazy dog.", &fLineHeight );
    fCalcedFontSize = fFontSize;

    fReadyToRender = true;
    IRecalcLineStarts( 0, true );
}

//// ICalcNumVisibleLines ////////////////////////////////////////////////////

int32_t   pfGUIMultiLineEditCtrl::ICalcNumVisibleLines() const
{
    if (fDynTextMap == nullptr || fLineHeight == 0)
        return 0;
    return (fDynTextMap->GetVisibleHeight() + fLineHeight - (fTopMargin + fBottomMargin + 1)) / fLineHeight;
}

//// IUpdate /////////////////////////////////////////////////////////////////
//  Ranged version

void    pfGUIMultiLineEditCtrl::IUpdate( int32_t startLine, int32_t endLine )
{
    if (!fReadyToRender || !fCanUpdate)
        return;

    // Detect whether we need to recalc all of our dimensions entirely
    if( fFontFlagsSet & (kFontFaceSet & kFontColorSet & kFontSizeSet & kFontStyleSet) )
        IPostSetUpDynTextMap();

    if (fLineStarts.empty())
    {
        // Just clear and go away
        fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );
        if( IsFocused() )
            fDynTextMap->FrameRect( fLeftMargin, fTopMargin, 2, fLineHeight, GetColorScheme()->fSelForeColor );
        fDynTextMap->FlushToHost();
        return;
    }

    // Recalculate the visible range due to our visible area and the scroll range.
    // adjust the scroll position if we are linked
    if (fPrevCtrl) // the first control (which has a nil fPrevCtrl) shouldn't adjust it's scroll pos because the app may want it to start there
    {
        IUpdateBuffer(); // make sure we are rendering the correct text
        fScrollPos = GetFirstVisibleLine();
    }
    int32_t numVisibleLines = ICalcNumVisibleLines();
    if (fNextCtrl || fPrevCtrl)
        numVisibleLines--; // we don't want "partially visible" lines
    int32_t lastVisibleLine = std::min(fScrollPos + numVisibleLines - 1,
                                       (int32_t)fLineStarts.size() - 1);

    if( startLine < fScrollPos )
        startLine = fScrollPos;
    if( endLine > lastVisibleLine )
        endLine = lastVisibleLine;

    bool clearEachLine = true;
    if( startLine == fScrollPos && endLine == lastVisibleLine )
    {
        fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );
        clearEachLine = false;
    }

    // Start at our line
    uint16_t y = (uint16_t)((startLine - fScrollPos) * fLineHeight + fTopMargin);
    // And loop!

    int32_t line;
    for (line = startLine; line <= endLine; line++)
    {
        // Clear this line
        if( clearEachLine )
        {
            fDynTextMap->FillRect(0, y, fDynTextMap->GetVisibleWidth(), fLineHeight,
                                  GetColorScheme()->fBackColor);
        }

        int32_t start = fLineStarts[line];
        int32_t end = (line == (int32_t)fLineStarts.size() - 1)
                      ? (int32_t)fBuffer.size() : fLineStarts[line + 1];

        // Render the actual text
        IRenderLine(fLeftMargin, y, start, end);

        // Render the cursor
        if( fCursorPos >= start && fCursorPos < end && IsFocused() && !IsLocked() )
        {
            uint16_t x = (fCursorPos > start)
                         ? (uint16_t)IRenderLine(fLeftMargin, y, start, fCursorPos, true)
                         : (uint16_t)fLeftMargin;

            fDynTextMap->FrameRect(x, y, 2, fLineHeight, GetColorScheme()->fSelForeColor);

            // Store the cursor X,Y pair. Go figure, the ONLY time we actually need this is
            // to move up or down one line, and even then it's only because we want to keep
            // the same approximate horizontal position (versus same character offset)
            fCurrCursorX = x;
            fCurrCursorY = y;
        }
        y += fLineHeight;
    }
    if (clearEachLine && line >= (int32_t)fLineStarts.size() && y < fDynTextMap->GetVisibleHeight() - fBottomMargin)
    {
        // No lines left, so clear the rest of the visible area
        fDynTextMap->FillRect(0, y, fDynTextMap->GetVisibleWidth(), fDynTextMap->GetVisibleHeight() - y,
                              GetColorScheme()->fBackColor);
    }
    fDynTextMap->FlushToHost();
}

//// IReadColorCode //////////////////////////////////////////////////////////
//  Reads a color code from the given position and advances the given position
//  appropriately

void    pfGUIMultiLineEditCtrl::IReadColorCode( int32_t &pos, hsColorRGBA &color ) const
{
    const wchar_t *buffer = fBuffer.data() + pos;
    uint8_t r, g, b;

    hsAssert(buffer[0] == kColorCodeChar, "Invalid position in IReadColorCode()");
    buffer++;
    r = (uint8_t)buffer[ 0 ];
    g = (uint8_t)buffer[ 1 ];
    b = (uint8_t)buffer[ 2 ];
    pos += kColorCodeSize;      // We have a duplicate code at the end of this block, for searching backwards
    color.Set( r / 255.f, g / 255.f, b / 255.f, fFontColor.a );
}

//// IReadStyleCode //////////////////////////////////////////////////////////
//  Reads a style code from the given position and advances the given position
//  appropriately

void    pfGUIMultiLineEditCtrl::IReadStyleCode( int32_t &pos, uint8_t &fontFlags ) const
{
    const wchar_t *buffer = fBuffer.data() + pos;

    hsAssert(buffer[0] == kStyleCodeChar, "Invalid position in IReadStyleCode()");
    fontFlags = (uint8_t)buffer[ 1 ];
    pos += kStyleCodeSize;      // We have a duplicate code at the end of this block, for searching backwards
}

inline bool pfGUIMultiLineEditCtrl::IIsRenderable( const wchar_t c )
{
    return ( !IIsCodeChar( c ) && c != L'\n' && c != L'\t' );
}

inline bool pfGUIMultiLineEditCtrl::IIsCodeChar( const wchar_t c )
{
    return (c == kColorCodeChar || c == kStyleCodeChar || c == kLinkCodeChar);
}

//// IFindLastCode Functions /////////////////////////////////////////////////
//  Given a position, these functions start at that position and work
//  backward until they find their respective code type, then return that code
//  type. If none is found, they set the given parameter to the default value
//  and return false.

bool    pfGUIMultiLineEditCtrl::IFindLastColorCode( int32_t pos, hsColorRGBA &color, bool ignoreFirstCharacter ) const
{
    for( ; pos >= 0; pos -= IOffsetToNextCharFromPos( pos - 1 ) )
    {
        if( fBuffer[ pos ] == kColorCodeChar && !ignoreFirstCharacter )
        {
            IReadColorCode( pos, color );
            return true;
        }
        ignoreFirstCharacter = false;
    }
    color = fFontColor; // use our default color
    return false;
}

bool    pfGUIMultiLineEditCtrl::IFindLastStyleCode( int32_t pos, uint8_t &style, bool ignoreFirstCharacter ) const
{
    for( ; pos >= 0; pos -= IOffsetToNextCharFromPos( pos - 1 ) )
    {
        if (fBuffer[pos] == kStyleCodeChar && !ignoreFirstCharacter)
        {
            IReadStyleCode( pos, style );
            return true;
        }
        ignoreFirstCharacter = false;
    }

    style = fFontStyle; // use our default style
    return false;
}

//// IRenderLine /////////////////////////////////////////////////////////////
//  Renders a null-terminated string to the dynamic text map at the location
//  given. Takes into account style codes and special characters (like returns
//  and tabs). Returns the final X value after rendering.

uint32_t  pfGUIMultiLineEditCtrl::IRenderLine( uint16_t x, uint16_t y, int32_t start, int32_t end, bool dontRender )
{
    int32_t       pos;
    hsColorRGBA currColor = fFontColor;
    uint8_t       currStyle;
    const wchar_t *buffer = fBuffer.data();

    // First, gotta go back from our starting position and find a color and style code to use
    IFindLastColorCode( start, currColor );
    IFindLastStyleCode( start, currStyle );

    fDynTextMap->SetTextColor( currColor, HasFlag( kXparentBgnd ) ? true : false );
    fDynTextMap->SetFont( fFontFace, fFontSize, GetColorScheme()->fFontFlags | currStyle,
                            HasFlag( kXparentBgnd ) ? false : true );
    
    // Now, start from our start and go to the end and keep eating up as many chunks
    // that aren't made up of control codes or carriage returns or tabs (since we render
    // those separately)
    for( pos = start; pos < end; )
    {
        start = pos;
        if( IIsRenderable( buffer[ pos ] ) )
        {
            // State 1: Render a renderable chunk
            while( pos < end && IIsRenderable( buffer[ pos ] ) )
                pos++;

            {
                plStringSlicer slicer( fBuffer, start, pos );
                if( !dontRender )
                    fDynTextMap->DrawString( x, y, slicer );
                x += fDynTextMap->CalcStringWidth( slicer );
            }
        }
        else
        {
            // State 2: Process non-renderable characters
            if (buffer[pos] == kColorCodeChar)
            {
                // Read color and switch to that one
                IReadColorCode( pos, currColor );
                if( !dontRender )
                    fDynTextMap->SetTextColor( currColor, HasFlag( kXparentBgnd ) ? true : false );
            }
            else if (buffer[pos] == kStyleCodeChar)
            {
                // Read style and switch to that one
                IReadStyleCode( pos, currStyle );
                if( !dontRender )
                    fDynTextMap->SetFont( fFontFace, fFontSize  , GetColorScheme()->fFontFlags | currStyle,
                                            HasFlag( kXparentBgnd ) ? false : true );
            }
            else if (buffer[pos] == kLinkCodeChar)
            {
                // Advance beyond the link code
                pos += kLinkCodeSize;
            }
            else if( buffer[ pos ] == L'\n' )
            {
                // We stop at carriage returns
                break;
            }
            else if( buffer[ pos ] == L'\t' )
            {
                // Increment by tab amount
                x += 16;
                pos++;
            }
            else
            {
                hsAssert( false, "Unknown unrenderable character, ignoring" );
                pos++;
            }
        }
    }

    return x;
}

void pfGUIMultiLineEditCtrl::PurgeDynaTextMapImage()
{
    if (fDynTextMap != nullptr)
        fDynTextMap->PurgeImage();
}

//// IUpdate /////////////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::IUpdate()
{
    // Just call the ranged one with a full range
    IUpdate(0, (int32_t)fLineStarts.size() - 1);
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);

    fScrollControl = nullptr;
    if( s->ReadBool() )
    {
        fScrollProc = new pfMLScrollProc( this );
        fScrollProc->IncRef();
        mgr->ReadKeyNotifyMe( s, new plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefScrollCtrl ), plRefFlags::kActiveRef );
    }
}

void    pfGUIMultiLineEditCtrl::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );

    if (fScrollControl != nullptr)
    {
        s->WriteBool( true );
        mgr->WriteKey( s, fScrollControl->GetKey() );
    }
    else
        s->WriteBool( false );
}

//// IPointToPosition ////////////////////////////////////////////////////////
//  Translates a 2D point on the visible texture surface to a cursor position.

int32_t   pfGUIMultiLineEditCtrl::IPointToPosition( int16_t ptX, int16_t ptY, bool searchOutsideBounds )
{
    // Find our line
    if (fPrevCtrl)
        fScrollPos = GetFirstVisibleLine(); // update the scroll position if we are linked

    int32_t lastVisibleLine = (int32_t)fLineStarts.size();
    if (!searchOutsideBounds)
        lastVisibleLine = std::min(fScrollPos + ICalcNumVisibleLines(), lastVisibleLine);

    int32_t line = searchOutsideBounds ? 0 : fScrollPos;
    int16_t y = (int16_t)(-(fScrollPos - line) * fLineHeight) + fTopMargin;
    for( ; line < lastVisibleLine; line++, y += fLineHeight )
    {
        if( ptY >= y && ptY < y + fLineHeight )
        {
            if (line < 0)
                break; // abort, and yes, this IS possible with this crappy code

            // Found the line, figure out what character
            int32_t start = fLineStarts[line];
            int32_t end = (line == (int32_t)fLineStarts.size() - 1)
                          ? (int32_t)fBuffer.size() - 1 : fLineStarts[line + 1];

            int32_t pos;
            for (pos = start; pos < end; pos++)
            {
                int16_t x = (int16_t)IRenderLine(fLeftMargin, 0, start, pos, true);
                if( x > ptX )
                    break;
            }

            // Go figure, our test puts it 1 past no matter what :)
            return pos - 1;
        }
    }

    // Just put us at the end of the last line
    return (int32_t)fBuffer.size() - 1;
}

//// IIsWordBreaker //////////////////////////////////////////////////////////
//  Returns whether the given character is one that can break a line

inline bool IIsWordBreaker( const wchar_t c )
{
    return (wcschr(L" \t,.;\n", c) != nullptr);
}

//// IOffsetToNextChar ///////////////////////////////////////////////////////

inline  int32_t   pfGUIMultiLineEditCtrl::IOffsetToNextChar( wchar_t stringChar )
{
    if (stringChar == kColorCodeChar)
        return kColorCodeSize;
    else if (stringChar == kStyleCodeChar)
        return kStyleCodeSize;
    else if (stringChar == kLinkCodeChar)
        return kLinkCodeSize;
    else
        return 1;
}

inline  int32_t   pfGUIMultiLineEditCtrl::IOffsetToNextCharFromPos( int32_t position ) const
{
    if( position >= 0 )
        return IOffsetToNextChar( fBuffer[ position ] );
    else
        return 1;
}

//// IRecalcLineStarts ///////////////////////////////////////////////////////
//  Recalculates all the uint16_t wrapping/line start values, starting at the
//  given line. If not forced, recalc will stop once a calculated line start
//  matches one already stored (this implying that everything after will be
//  the same as well, assuming contents are the same). If this assumption can't
//  be made, force recalc of all the lines.
//  Returns the last line recalced.
//  The starting line is basically the first line whose start might have
//  changed, so we assume as a hint that every line before the starting line
//  is still valid. If startingLine = 0, we recalc 'em all.

int32_t   pfGUIMultiLineEditCtrl::IRecalcLineStarts( int32_t startingLine, bool force, bool dontUpdate )
{
    uint16_t      wrapWidth, widthCounter;
    int32_t       charPos = 0, nextPos, startPos;
    int32_t       currLine, realStartingLine;

    if( fPrevCtrl )
        IUpdateBuffer(); // make sure our buffer is correct if we are linked
    
    if (fDynTextMap == nullptr)
    {
        // Can't calculate anything. Just return invalid
        fLineStarts.clear();
        IUpdateScrollRange();
        return -1;
    }

    // Figure out our starting character
    if( startingLine > 0 )
    {
        if (startingLine >= (int32_t)fLineStarts.size())
        {
            // Must be a problem, force full recalc
            hsStatusMessage( "Invalid starting line in IRecalcLineStarts(), forcing full recalc" );
            currLine = 0;
            force = true;
        }
        else
        {
            // Start 1 line before, since we assume that line start is still valid
            currLine = startingLine - 1;
            charPos = fLineStarts[ currLine ];
        }
    }
    else
        currLine = 0;

    realStartingLine = currLine;    // For the IUpdate call later

    // Precalculate some helper values
    wrapWidth = fDynTextMap->GetVisibleWidth() - fRightMargin;
    wchar_t* buffer = fBuffer.data();

    for (; charPos < (int32_t)fBuffer.size(); currLine++)
    {
        //// Store this line start
        startPos = charPos;
        if( IStoreLineStart( currLine, startPos ) )
        {
            if( currLine > startingLine )
            {
                // We just stored a line that didn't change its starting position (first line doesn't count)
                if( !force )
                    break;
            }
            else if( currLine > realStartingLine )
            {
                // Line start didn't change, but we're at the start. See, we always go one line back in case
                // something changed that would cause the line before to update. But if we're here, both the
                // line before and this line have the same starting position, which means that the line before
                // didn't change, so we can increment realStartingLine and skip re-drawing that line
                realStartingLine++;
            }
        }

        //// We do a walk where we find the start of the next uint16_t (i.e. the end of this uint16_t plus
        //// any "white space"), and then see if we can fit everything up to that point. If we can,
        //// keep walking, if not, stop at whatever we had as a starting point.
        nextPos = charPos;
        for( widthCounter = 0; widthCounter < wrapWidth; )
        {
            charPos = nextPos;

            // Are we on a line break?
            if (nextPos >= (int32_t)fBuffer.size() || buffer[nextPos] == L'\n' || buffer[nextPos] == 0L)
            {
                charPos++;
                break;  // Yup, so do so
            }

            // Find the end of this word
            while (nextPos < (int32_t)fBuffer.size() && !IIsWordBreaker(buffer[nextPos]))
                nextPos += IOffsetToNextChar( buffer[ nextPos ] );

            // Now we're at some white space, keep going until we hit the next word
            while (nextPos < (int32_t)fBuffer.size() && IIsWordBreaker(buffer[nextPos]) && buffer[nextPos] != L'\n')
                nextPos += IOffsetToNextChar( buffer[ nextPos ] );

            // Now see how much width this is
            widthCounter = (uint16_t)IRenderLine( fLeftMargin, 0, startPos, nextPos, true );
            
            // Now we loop. If wrapWidth is too much, we'll break the loop with charPos pointing to the
            // end of our line. If not, charPos will advance to start the search again
        }

        // Check to see if the line was just too long to wrap at all
        if( startPos == charPos )
        {
            // OK, so just try again and break as soon as necessary. nextPos should be 
            // already advanced too far, so start from there and go back
            while( widthCounter >= wrapWidth && nextPos > startPos )
            {
                nextPos -= IOffsetToNextChar( buffer[ nextPos - 1 ] );
                widthCounter = (uint16_t)IRenderLine( fLeftMargin, 0, startPos, nextPos, true );
            }

            charPos = nextPos;
        }

        // Continue on!     
    }

    if (charPos >= (int32_t)fBuffer.size())
    {
        // Make sure there are no lines stored after this one
        fLineStarts.resize(currLine);
    }

    IUpdateScrollRange();

    if( !dontUpdate )
        // We just changed some of the line starts, so time to redraw
        IUpdate( realStartingLine, currLine - 1 );

    // we just updated the line starts, so set all following controls to the same set
    if (fNextCtrl)
        fNextCtrl->ISetLineStarts(fLineStarts);

    return currLine;
}

//// IStoreLineStart /////////////////////////////////////////////////////////
//  Stores a single line start, expanding the array if necessary.

bool    pfGUIMultiLineEditCtrl::IStoreLineStart(int32_t line, int32_t start)
{
    if ((int32_t)fLineStarts.size() <= line)
    {
        hsAssert(line == (int32_t)fLineStarts.size(), "Trying to store a line way past the end of line starts!");
        fLineStarts.resize(line + 1);
        fLineStarts[ line ] = -1;
    }

    bool same = ( fLineStarts[ line ] == start ) ? true : false;
    fLineStarts[ line ] = start;
    return same;
}

//// IFindCursorLine /////////////////////////////////////////////////////////
//  Calculates the line the cursor is sitting on

int32_t   pfGUIMultiLineEditCtrl::IFindCursorLine( int32_t cursorPos ) const
{
    int32_t   line;


    if( cursorPos == -1 )
        cursorPos = fCursorPos;

    for (line = 0; line < (int32_t)fLineStarts.size() - 1; line++)
    {
        if( fLineStarts[ line + 1 ] > cursorPos )
            break;
    }
    return line;
}

//// IRecalcFromCursor ///////////////////////////////////////////////////////
//  Recalcs starting with the line that the cursor is sitting on

void    pfGUIMultiLineEditCtrl::IRecalcFromCursor( bool force )
{
    IRecalcLineStarts( IFindCursorLine(), force );
}

//// IOffsetLineStarts ///////////////////////////////////////////////////////
//  Given the position and offset, offsets all line starts >= that position
//  by the given amount. Used to insert a character and you know that all the
//  line offsets after that character should just bump up one, for example.
//  Also offsets the end of the current selection if desired and in range.
//  Why only the end of the selection? Because in the only cases where we're
//  doing this, we are inserting into (and offseting inside) a selection,
//  so we don't want the start moving around.

void    pfGUIMultiLineEditCtrl::IOffsetLineStarts( uint32_t position, int32_t offset, bool offsetSelectionEnd )
{
    // Check our first line and make sure offsetting it won't make it invalid.
    // If it will, we need to recalc the line starts entirely (which is fine,
    // since this function is just called to try to optimize out doing so, but
    // when you gotta, you gotta...)
    int32_t line = IFindCursorLine( position );
    if (line < (int32_t)fLineStarts.size() - 1)
    {
        if( fLineStarts[ line + 1 ] + offset <= fLineStarts[ line ] )
        {
            // We want to force the line starts to recalc. Otherwise, IRecalc() will attempt
            // to stop once it detects a line start that hasn't changed. Which doesn't work
            // because our line starts just got offsetted, hence why this function was called.
            // Even then, it isn't necessarily a bad thing, since that line will be OK, but
            // the assumption IRecalc() makes is that all the lines *after* that must be OK too
            // b/c it just found the same line start, which in this case it didn't.
            // Just trust me.
            IRecalcLineStarts( line, true );
            return;
        }
    }

    // Offset all lines past our given position
    for (line = 0; line < (int32_t)fLineStarts.size(); line++)
    {
        if( fLineStarts[ line ] > position )
            fLineStarts[ line ] += offset;
    }

    // now update all following controls
    if (fNextCtrl)
        fNextCtrl->ISetLineStarts(fLineStarts);
}

//// IFindLink ///////////////////////////////////////////////////////////////

int16_t pfGUIMultiLineEditCtrl::IFindLink(int32_t cursorPos, uint8_t modifiers) const
{
    // If we can edit, then require a Ctrl-Click
    if (fFocused && !hsCheckBits(modifiers, pfGameGUIMgr::kCtrlDown))
        return -1;

    if (cursorPos >= 0) {
        for (hsSsize_t i = cursorPos; i > 0; --i) {
            if (fBuffer[i] == kLinkCodeChar)
                return static_cast<int16_t>(fBuffer[i - 1]);
        }
    }
    return -1;
}

//// HandleMouse /////////////////////////////////////////////////////////////

bool pfGUIMultiLineEditCtrl::IHandleMouse(hsPoint3& mousePt)
{
    if (fDynTextMap == nullptr || !fBounds.IsInside(&mousePt))
        return false;

    IScreenToLocalPt(mousePt);
    mousePt.fX *= fDynTextMap->GetVisibleWidth();
    mousePt.fY *= fDynTextMap->GetVisibleHeight();
    return true;
}

//// FocusOnMouseDown ////////////////////////////////////////////////////////

bool pfGUIMultiLineEditCtrl::FocusOnMouseDown(const hsPoint3& mousePt, uint8_t modifiers) const
{
    // Don't focus the control if we've clicked on a link inside of it.
    return fClickedLinkId == -1;
}

//// HandleMouseDown /////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    if (IHandleMouse(mousePt)) {
        int32_t cursorPos = IPointToPosition((int16_t)(mousePt.fX), (int16_t)(mousePt.fY));
        fClickedLinkId = fCurrLinkId = IFindLink(cursorPos, modifiers);
        if (fCurrLinkId == -1)
            IMoveCursorTo(cursorPos);
    } else {
        fClickedLinkId = fCurrLinkId = -1;
    }
}

//// HandleMouseUp ///////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
    if (IHandleMouse(mousePt)) {
        int32_t cursorPos = IPointToPosition((int16_t)(mousePt.fX), (int16_t)(mousePt.fY));

        fCurrLinkId = IFindLink(cursorPos, modifiers);
        if (fClickedLinkId == fCurrLinkId && fCurrLinkId != -1) {
            HandleExtendedEvent(kLinkClicked);
            fClickedLinkId = -1;
        } else {
            IMoveCursorTo(cursorPos);
        }
    } else {
        fClickedLinkId = fCurrLinkId = -1;
    }
}

//// HandleMouseDrag /////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
    if (IHandleMouse(mousePt)) {
        int32_t cursorPos = IPointToPosition((int16_t)(mousePt.fX), (int16_t)(mousePt.fY));
        fCurrLinkId = IFindLink(cursorPos, modifiers);
        if (fCurrLinkId == -1)
            IMoveCursorTo(cursorPos);
    } else {
        fCurrLinkId = -1;
    }
}

//// HandleMouseHover /////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::HandleMouseHover(hsPoint3& mousePt, uint8_t modifiers)
{
    if (IHandleMouse(mousePt)) {
        int32_t cursorPos = IPointToPosition((int16_t)(mousePt.fX), (int16_t)(mousePt.fY));
        fCurrLinkId = IFindLink(cursorPos, modifiers);
    } else {
        fCurrLinkId = -1;
    }
}

//// IGetDesiredCursor /////////////////////////////////////////////////////////

uint32_t    pfGUIMultiLineEditCtrl::IGetDesiredCursor() const
{
    // Return a poised cursor if we're over any link, unless we're editing
    // the control (FIXME this last part...)
    if (fCurrLinkId == -1)
        return plInputInterface::kNullCursor;
    else if (fCurrLinkId == fClickedLinkId)
        return plInputInterface::kCursorClicked;
    else
        return plInputInterface::kCursorPoised;
}

bool    pfGUIMultiLineEditCtrl::HandleKeyPress( wchar_t key, uint8_t modifiers )
{
    if ((fPrevCtrl || fNextCtrl) && ((int32_t)fLineStarts.size() <= GetFirstVisibleLine()))
        return true; // we're ignoring if we can't actually edit our visible frame (and we're linked)

    // Store info for the event we're about to send out
    fLastKeyModifiers = modifiers;
    fLastKeyPressed = key;
    
    // Send out a key pressed event. 
    HandleExtendedEvent( kKeyPressedEvent );

    // We discard keys when locked only after we give our handler the key
    if( IsLocked() )
        return true;

    // Insert character at the current cursor position, then inc the cursor by one
    // Note: we always want selection mode off when we're typing
    InsertChar( key );
    return true;
}

bool    pfGUIMultiLineEditCtrl::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers )
{
    if( key == KEY_CAPSLOCK )
        return false;

    if ((fPrevCtrl || fNextCtrl) && ((int32_t)fLineStarts.size() <= GetFirstVisibleLine()))
        return true; // we're ignoring if we can't actually edit our visible frame (and we're linked)

    if( event == pfGameGUIMgr::kKeyDown || event == pfGameGUIMgr::kKeyRepeat )
    {
        // Use arrow keys to do our dirty work
        if( key == KEY_UP )
            IMoveCursor( kOneLineUp );
        else if( key == KEY_DOWN )
            IMoveCursor( kOneLineDown );
        else if( key == KEY_HOME )
            IMoveCursor( ( modifiers & pfGameGUIMgr::kCtrlDown ) ? kBufferStart : kLineStart );
        else if( key == KEY_END )
            IMoveCursor( ( modifiers & pfGameGUIMgr::kCtrlDown ) ? kBufferEnd : kLineEnd );
        else if( key == KEY_PAGEUP )
            IMoveCursor( kPageUp );
        else if( key == KEY_PAGEDOWN )
            IMoveCursor( kPageDown );
        
        else if( key == KEY_LEFT )
            IMoveCursor( ( modifiers & pfGameGUIMgr::kCtrlDown ) ? kOneWordBack : kOneBack );
        else if( key == KEY_RIGHT )
            IMoveCursor( ( modifiers & pfGameGUIMgr::kCtrlDown ) ? kOneWordForward : kOneForward );

        else if( key == KEY_BACKSPACE )
        {
            if( IsLocked() )
                return true;

            if( fCursorPos > 0 )
            {
                IMoveCursor(kOneBack);
                DeleteChar();
            }
        }
        else if( key == KEY_DELETE )
        {
            if( IsLocked() )
                return true;

            DeleteChar();
        }
        else if( key == KEY_ENTER )
        {
            if( IsLocked() )
                return true;

            InsertChar(L'\n');
        }
        else if (modifiers & pfGameGUIMgr::kCtrlDown) 
        {
            // Not exactly safe way to do it, since there are control codes inside buffer.
            // But what's the worst thing that can happen? Horribly colorful ki-mails?
            // Too lazy to worry about that...
            if (key == KEY_C) 
            {
                plClipboard::GetInstance().SetClipboardText(ST::string::from_wchar(fBuffer.data(), fBuffer.size() - 1));
            }
            else if (key == KEY_V)
            {
                ST::string contents = plClipboard::GetInstance().GetClipboardText();
                InsertString(contents);
            }
        } 
        else if( key == KEY_ESCAPE )
        {
//          fEscapedFlag = true;
            DoSomething();      // Query WasEscaped() to see if it was escape vs enter
        }

        return true;
    }
    else
        // We don't process them, but we don't want anybody else processing them either
        return true;
}

//// ISetCursor //////////////////////////////////////////////////////////////
//  Only moves the cursor and redraws, doesn't bother with selections. You
//  should probably call IMoveCursorTo() unless you really know what you're
//  doing and don't want the current selection updated.

void    pfGUIMultiLineEditCtrl::ISetCursor( int32_t newPosition )
{
    fCursorPos = newPosition;

    int32_t newLine = IFindCursorLine();

    // Rescroll if necessary
    if( fLastCursorLine != newLine )
    {
        if( newLine < fScrollPos )
        {
            if (fPrevCtrl) // we are linked
            {
                fPrevCtrl->ISetCursor(newPosition); // so tell the prev control to set its cursor
                fPrevCtrl->GetOwnerDlg()->SetFocus(fPrevCtrl); // give control to the prev control (since we just moved our cursor there)
            }
            else if (!fPrevCtrl && fNextCtrl) // we are linked, but there isn't anyone before us
                IHitBeginningOfControlList(newPosition); // send an event to the person that wanted it
            else
                SetScrollPosition( newLine );
        }
        else
        {
            // -2 here for a reason: 1 because we want the last fully visible line, not partially visible,
            // and 1 because we want the actual last visible line index, which is of course start + len - 1
            int32_t delta = newLine - ( fScrollPos + ICalcNumVisibleLines() - 2 );
            if( delta > 0 )
            {
                if (fNextCtrl) // we are linked
                {
                    fNextCtrl->ISetCursor(newPosition); // so tell the next control to set its cursor
                    fNextCtrl->GetOwnerDlg()->SetFocus(fNextCtrl); // give control to the next control (since we just moved our cursor there)
                }
                else if (!fNextCtrl && fPrevCtrl) // we are linked, but there isn't anyone after us
                    IHitEndOfControlList(newPosition); // send an event to the person that wanted it
                else
                    SetScrollPosition( fScrollPos + delta );
            }
        }
    }

    if( fLastCursorLine < newLine )
        IUpdate( fLastCursorLine, newLine );
    else
        IUpdate( newLine, fLastCursorLine );

    fLastCursorLine = newLine;
}

//// IMoveCursor /////////////////////////////////////////////////////////////
//  Moves the cursor in a relative direction.

void    pfGUIMultiLineEditCtrl::IMoveCursor( pfGUIMultiLineEditCtrl::Direction dir )
{
    int32_t   cursor = fCursorPos, line, offset, end;


    switch( dir )
    {
        case kLineStart:
            while( cursor > 0 && fBuffer[ cursor - 1 ] != L'\n' )
                cursor--;
            break;

        case kLineEnd:
            while (cursor < (int32_t)fBuffer.size() - 1 && fBuffer[cursor] != L'\n')
                cursor++;
            break;

        case kBufferStart:
            cursor = 0;
            break;

        case kBufferEnd:
            cursor = (int32_t)fBuffer.size() - 1;
            break;

        case kOneBack:
            if( cursor > 0 )
            {
                cursor--;
                while (cursor > 0 && IIsCodeChar(fBuffer[cursor]))
                    cursor -= IOffsetToNextChar( fBuffer[ cursor ] );
            }
            break;

        case kOneForward:
            if (cursor < (int32_t)fBuffer.size() - 1)
            {
                cursor++;
                while (cursor < (int32_t)fBuffer.size() - 1 && IIsCodeChar(fBuffer[cursor]))
                    cursor += IOffsetToNextChar( fBuffer[ cursor ] );
            }
            break;

        case kOneWordBack:
            if( cursor > 0 )
            {
                for( ; cursor > 0 && IIsWordBreaker( fBuffer[ cursor - 1 ] ); cursor-- );
                for( ; cursor > 0 && !IIsWordBreaker( fBuffer[ cursor - 1 ] ); cursor-- );
            }
            break;

        case kOneWordForward:
            if (cursor < (int32_t)fBuffer.size() - 1)
            {
                for (; cursor < (int32_t)fBuffer.size() - 1 && !IIsWordBreaker(fBuffer[cursor]); cursor++);
                for (; cursor < (int32_t)fBuffer.size() - 1 && IIsWordBreaker(fBuffer[cursor]); cursor++);
            }
            break;

        case kOneLineUp:
            // The wonderful thing is, since we keep going on the position of the cursor (which magically
            // is the left side of the character we're on), we end up drifting left as we keep going up
            // or down. So to compensate, we put a little fudge factor in that lets us attempt to stay 
            // "on course", as it were.
            if( IFindCursorLine( cursor ) > 0 )
                cursor = IPointToPosition( fCurrCursorX + ( fLineHeight >> 2 ), fCurrCursorY - fLineHeight, true );
            break;

        case kOneLineDown:
            if (IFindCursorLine(cursor) < (int32_t)fLineStarts.size() - 1)
                cursor = IPointToPosition( fCurrCursorX + ( fLineHeight >> 2 ), fCurrCursorY + fLineHeight, true );
            break;

        case kPageUp:
            line = IFindCursorLine( cursor );
            offset = cursor - fLineStarts[ line ];

            line -= ( ICalcNumVisibleLines() - 1 );
            if( line < 0 )
                line = 0;

            end = (line < (int32_t)fLineStarts.size() - 1)
                  ? fLineStarts[line + 1] : (int32_t)fBuffer.size() - 1;
            if( fLineStarts[ line ] + offset > end )
                offset = end - fLineStarts[ line ] - 1;

            cursor = fLineStarts[ line ] + offset;
            break;

        case kPageDown:
            line = IFindCursorLine( cursor );
            offset = cursor - fLineStarts[ line ];

            line += ( ICalcNumVisibleLines() - 1 );
            if (line > (int32_t)fLineStarts.size() - 1)
                line = (int32_t)fLineStarts.size() - 1;

            end = (line < (int32_t)fLineStarts.size() - 1)
                  ? fLineStarts[line + 1] : (int32_t)fBuffer.size() - 1;
            if( fLineStarts[ line ] + offset > end )
                offset = end - fLineStarts[ line ] - 1;

            cursor = fLineStarts[ line ] + offset;
            break;
    }

    IMoveCursorTo( cursor );
}

//// IMoveCursorTo ///////////////////////////////////////////////////////////
//  Moves the cursor to the given absolute position and updates the selection
//  area accordingly and if necessary.

void    pfGUIMultiLineEditCtrl::IMoveCursorTo( int32_t position )
{
    ISetCursor( position );
}

//////////////////////////////////////////////////////////////////////////////
//// Buffer Modification /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// InsertChar //////////////////////////////////////////////////////////////
//  Inserts a character at the cursor, or if there is a selection, replaces
//  the selection with the given character. Either way, at the end the cursor
//  is after the inserted character.

void    pfGUIMultiLineEditCtrl::InsertChar( wchar_t c )
{
    // Error checking--make sure our actual character isn't in the range of our
    // code characters, or chaos could erupt
    //if( c < 3 )
    //return;
    if (c == 0)
        return;

    if (fBufferLimit == -1 || (int32_t)fBuffer.size() + 1 < fBufferLimit - 1)
    {
        fBuffer.insert(fBuffer.begin() + fCursorPos, c);

        ISetGlobalBuffer(); // update the global buffer

        IOffsetLineStarts( fCursorPos, 1 );
        IMoveCursor( kOneForward );
        IRecalcFromCursor();
    }
}

//// InsertString ////////////////////////////////////////////////////////////
//  Same as InsertChar, only with a string of characters instead of just one.

void    pfGUIMultiLineEditCtrl::InsertString( const ST::string& string )
{
    ST::wchar_buffer buf = string.to_wchar();

    if (fBufferLimit == -1 || (int32_t)(fBuffer.size() + buf.size()) < fBufferLimit - 1)
    {
        fBuffer.insert(fBuffer.begin() + fCursorPos, buf.begin(), buf.end());

        ISetGlobalBuffer(); // update the global buffer

        IOffsetLineStarts( fCursorPos, buf.size() );
        IMoveCursorTo( fCursorPos + buf.size() );
        IRecalcFromCursor();
    }
}

//// InsertColor /////////////////////////////////////////////////////////////
//  Inserts a color control code into the buffer, or if there is a selection,
//  places TWO color codes in: one before the selection with the given color
//  and the other after the given selection with the previous color.
//  There is one catch to all of this, though: if we have a selection AND
//  we're inserting a code that conflicts with any code contained in the
//  selection, we remove the conflicting codes.

void    pfGUIMultiLineEditCtrl::InsertColor( hsColorRGBA &color )
{
    IActuallyInsertColor( fCursorPos, color );
    IOffsetLineStarts(fCursorPos, kColorCodeSize);
    fCursorPos += kColorCodeSize;
    IRecalcFromCursor( true );  // Force update of all following lines, since 
                                // insertion of this code changes appearance of following characters
}

void    pfGUIMultiLineEditCtrl::IActuallyInsertColor( int32_t pos, hsColorRGBA &color )
{
    if (fBufferLimit == -1 || (int32_t)fBuffer.size() + 4 < fBufferLimit - 1)
    {
        fBuffer.insert(fBuffer.begin() + pos, {
            kColorCodeChar,
            (uint8_t)(color.r * 255.f),
            (uint8_t)(color.g * 255.f),
            (uint8_t)(color.b * 255.f),
            kColorCodeChar
        });
    }
}

//// InsertStyle /////////////////////////////////////////////////////////////
//  Same thing as InsertColor(), only with a style code (or two).

void    pfGUIMultiLineEditCtrl::InsertStyle( uint8_t fontStyle )
{
    IActuallyInsertStyle( fCursorPos, fontStyle );

    IOffsetLineStarts(fCursorPos, kStyleCodeSize);
    fCursorPos += kStyleCodeSize;
    IRecalcFromCursor( true );  // Force update of all following lines, since 
                                // insertion of this code changes appearance of following characters
}

void    pfGUIMultiLineEditCtrl::IActuallyInsertStyle( int32_t pos, uint8_t style )
{
    if (fBufferLimit == -1 || (int32_t)fBuffer.size() + 3 < fBufferLimit - 1)
    {
        fBuffer.insert(fBuffer.begin() + pos, {
            kStyleCodeChar,
            (wchar_t)style,
            kStyleCodeChar
        });
    }
}

void    pfGUIMultiLineEditCtrl::InsertLink(int16_t linkId)
{
    IActuallyInsertLink(fCursorPos, linkId);

    IOffsetLineStarts(fCursorPos, kLinkCodeSize);
    fCursorPos += kLinkCodeSize;
    IRecalcFromCursor();
}

void     pfGUIMultiLineEditCtrl::IActuallyInsertLink(int32_t pos, int16_t linkId)
{
    if (fBufferLimit == -1 || (int32_t)fBuffer.size() + kLinkCodeSize < fBufferLimit - 1) {
        fBuffer.insert(fBuffer.begin() + pos, {
            kLinkCodeChar,
            (wchar_t)linkId,
            kLinkCodeChar
        });
    }
}

//// DeleteChar //////////////////////////////////////////////////////////////
//  If there is no selection, deletes the single character that the cursor
//  is in front of. Otherwise, deletes the current selection and places the
//  cursor where the selection used to be.

void    pfGUIMultiLineEditCtrl::DeleteChar()
{
    if (fCursorPos < (int32_t)fBuffer.size() - 1)
    {
        int32_t offset = IOffsetToNextChar( fBuffer[ fCursorPos ] );
        bool forceUpdate = IIsCodeChar( fBuffer[ fCursorPos ] );
        const auto cursorIter = fBuffer.cbegin() + fCursorPos;
        fBuffer.erase(cursorIter, cursorIter + offset);

        ISetGlobalBuffer(); // update the global buffer

        IOffsetLineStarts( fCursorPos, -offset );
        IRecalcFromCursor( forceUpdate );
    }
}

//// ICopyRange //////////////////////////////////////////////////////////////
//  Generic coded-to-non-coded conversion. Returns a copy of the string.

ST::wchar_buffer pfGUIMultiLineEditCtrl::ICopyRange( int32_t start, int32_t end ) const
{
    int32_t   stringSize, pos;

    // First loop, just count how much space we need
    for( stringSize = 0, pos = start; pos < end; pos = pos + IOffsetToNextChar( fBuffer[ pos ] ) )
    {
        if( !IIsCodeChar( fBuffer[ pos ] ) )
            stringSize++;
    }

    // Our string...
    ST::wchar_buffer string;
    string.allocate(stringSize);

    // Now actually copy the characters
    for( stringSize = 0, pos = start; pos < end; pos = pos + IOffsetToNextChar( fBuffer[ pos ] ) )
    {
        if( !IIsCodeChar( fBuffer[ pos ] ) )
            string[ stringSize++ ] = fBuffer[ pos ];
    }

    // All done!
    return string;
}

//// ClearBuffer /////////////////////////////////////////////////////////////
//  Clears everything, including the undo list.

void    pfGUIMultiLineEditCtrl::ClearBuffer()
{
    fBuffer = {L'\0'};
    fCursorPos = 0;
    fLastCursorLine = 0;
    fScrollPos = 0;
    IRecalcLineStarts( 0, true );
}

//// SetBuffer ///////////////////////////////////////////////////////////////
//  Replaces the entire contents of the buffer with the given text. Also
//  clears the undo list. Can handle buffers with style codes in them.

void    pfGUIMultiLineEditCtrl::SetBuffer(const ST::string& text)
{
    ST::wchar_buffer buffer = text.to_wchar();
    SetBuffer(buffer.c_str(), buffer.size());
}

void    pfGUIMultiLineEditCtrl::SetBuffer(const wchar_t *codedText, size_t length)
{
    // recursively call back to the first control and set it
    if (fPrevCtrl)
    {
        fPrevCtrl->SetBuffer(codedText,length);
        IUpdateBuffer();
    }
    else // we are the first control, so set our buffer
    {
        fBuffer.assign(codedText, codedText + length);
        fBuffer.emplace_back(L'\0');
        IRecalcLineStarts( 0, true ); // only the first control will recalc, this function recurses down to handle all following controls
    }

    fScrollPos = GetFirstVisibleLine();
    fCursorPos = 0;
    fLastCursorLine = 0;
}

//// GetNonCodedBuffer ///////////////////////////////////////////////////////
//  Copies the entire buffer into an plain text string (i.e. strips formatting)
//  and returns it.

ST::wchar_buffer pfGUIMultiLineEditCtrl::GetNonCodedBuffer() const
{
    // recursively search back to the first control in the linked list and grab its buffer
    if (fPrevCtrl)
        return fPrevCtrl->GetNonCodedBuffer();
    else
    {
        // -1 for the null terminator
        return ICopyRange(0, (int32_t)fBuffer.size() - 1);
    }
}

//// GetCodedBuffer //////////////////////////////////////////////////////////
//  Basically does a blanket copy of the entire buffer and returns it.

ST::wchar_buffer pfGUIMultiLineEditCtrl::GetCodedBuffer() const
{
    // recursively search back to the first control in the linked list and grab its buffer
    if (fPrevCtrl)
        return fPrevCtrl->GetCodedBuffer();
    else
    {
        return ST::wchar_buffer(fBuffer.data(), fBuffer.size() - 1);
    }
}

//// ICharPosToBufferPos /////////////////////////////////////////////////////
//  Given a character position (i.e. a buffer position if we didn't have
//  control codes), returns the actual buffer pos.

int32_t   pfGUIMultiLineEditCtrl::ICharPosToBufferPos( int32_t charPos ) const
{
    int32_t   pos;

    for (pos = 0; charPos > 0 && pos < (int32_t)fBuffer.size() - 1; pos += IOffsetToNextCharFromPos(pos), charPos--);

    return pos;
}

//// Locking /////////////////////////////////////////////////////////////////

void    pfGUIMultiLineEditCtrl::Lock()
{
    fLockCount++;
    IUpdate();  
}

void    pfGUIMultiLineEditCtrl::Unlock()
{
    fLockCount--;
    //hsAssert( fLockCount >= 0, "Too many unlocks for pfGUIMultiLineEditCtrl" );
    if (fLockCount < 0)
        fLockCount = 0; // instead of asserting, hande it nicely
    IUpdate();  
}

// linking functions, for linking multiple multi-line edit controls together

void    pfGUIMultiLineEditCtrl::SetNext( pfGUIMultiLineEditCtrl *newNext )
{
    ClearNext(); // clear the existing link (if there is one)
    if (newNext)
    {
        newNext->ClearPrev(); // clear any existing prev link
        fNextCtrl = newNext;
        fNextCtrl->fPrevCtrl = this;
        fNextCtrl->IUpdateBuffer();
        fNextCtrl->fScrollPos = fNextCtrl->GetFirstVisibleLine();
    }
}

void    pfGUIMultiLineEditCtrl::ClearNext()
{
    if (fNextCtrl)
        fNextCtrl->fPrevCtrl = nullptr; // unlink ourselves from the next control
    fNextCtrl = nullptr;
}

void    pfGUIMultiLineEditCtrl::SetPrev( pfGUIMultiLineEditCtrl *newPrev )
{
    ClearPrev(); // clear existing link
    if (newPrev)
    {
        newPrev->ClearNext(); // clear any existing next link
        fPrevCtrl = newPrev;
        fPrevCtrl->fNextCtrl = this;
        IUpdateBuffer();
        fScrollPos = GetFirstVisibleLine();
    }
}

void    pfGUIMultiLineEditCtrl::ClearPrev()
{
    if (fPrevCtrl)
        fPrevCtrl->fNextCtrl = nullptr; // unlink ourselves from the prev control
    fPrevCtrl = nullptr;
}

void    pfGUIMultiLineEditCtrl::SetEventProc(pfGUIMultiLineEditProc *eventProc)
{
    if (fPrevCtrl)
        fPrevCtrl->SetEventProc(eventProc);
    else
    {
        if (fEventProc)
            delete fEventProc;
        fEventProc = eventProc;
    }
}

void    pfGUIMultiLineEditCtrl::ClearEventProc()
{
    if (fPrevCtrl)
        fPrevCtrl->ClearEventProc();
    else
    {
        if (fEventProc)
            delete fEventProc;
        fEventProc = nullptr;
    }
}

int32_t   pfGUIMultiLineEditCtrl::GetFirstVisibleLine()
{
    // recursively search back to the first control and work our way forwards to where we are supposed to be
    if (fPrevCtrl)
        return fPrevCtrl->GetLastVisibleLine();
    return fScrollPos; // we're the first control, so we show the first part of the buffer
}

int32_t   pfGUIMultiLineEditCtrl::GetLastVisibleLine()
{
    // simply add our number of lines to our first visible line
    return GetFirstVisibleLine()+ICalcNumVisibleLines()-1;
}

void    pfGUIMultiLineEditCtrl::SetGlobalStartLine(int32_t line)
{
    // recursively call back to the first control and set it
    if (fPrevCtrl)
        fPrevCtrl->SetGlobalStartLine(line);
    else
    {
        fScrollPos = line;
        IRecalcLineStarts(fScrollPos,true);
    }
}

void    pfGUIMultiLineEditCtrl::IUpdateLineStarts()
{
    // make sure our line starts array matches the gloabl one
    if (fPrevCtrl)
    {
        fPrevCtrl->IUpdateLineStarts();
        fLineStarts = fPrevCtrl->fLineStarts;
    }
}

void    pfGUIMultiLineEditCtrl::IUpdateBuffer()
{
    // make sure our local buffer matches the global one
    if (fPrevCtrl)
    {
        // copy the buffer from our global one
        size_t length;
        ST::wchar_buffer codedText = GetCodedBuffer();
        fBuffer.assign(codedText.begin(), codedText.end());
        fBuffer.emplace_back(L'\0');
    }
}

void pfGUIMultiLineEditCtrl::ISetGlobalBuffer()
{
    if (fPrevCtrl)
    {
        fPrevCtrl->fBuffer = fBuffer;
        fPrevCtrl->ISetGlobalBuffer(); // pass the update backwards
    }
}

void pfGUIMultiLineEditCtrl::ISetLineStarts(const std::vector<int32_t> &lineStarts)
{
    if (fNextCtrl)
        fNextCtrl->ISetLineStarts(lineStarts); // pass it on down

    IUpdateBuffer(); // make sure the buffer is correct
    fLineStarts = lineStarts;

    IUpdate(); // make sure everything looks right
}

void pfGUIMultiLineEditCtrl::SetMargins(int top, int left, int bottom, int right)
{
    fTopMargin = top;
    fLeftMargin = left;
    fBottomMargin = bottom;
    fRightMargin = right;
    IRecalcLineStarts(0,false);
}

void pfGUIMultiLineEditCtrl::IHitEndOfControlList(int32_t cursorPos)
{
    if (fPrevCtrl)
        fPrevCtrl->IHitEndOfControlList(cursorPos);
    else
    {
        if (fEventProc)
            fEventProc->OnEndOfControlList(cursorPos);
    }
}

void pfGUIMultiLineEditCtrl::IHitBeginningOfControlList(int32_t cursorPos)
{
    if (fPrevCtrl)
        fPrevCtrl->IHitBeginningOfControlList(cursorPos);
    else
    {
        if (fEventProc)
            fEventProc->OnBeginningOfControlList(cursorPos);
    }
}

void pfGUIMultiLineEditCtrl::SetFontFace(const ST::string &fontFace)
{
    fFontFace = fontFace;
    fFontFlagsSet |= kFontFaceSet;
    fDynTextMap->SetFont( fFontFace, fFontSize, fFontStyle,
                            HasFlag( kXparentBgnd ) ? false : true );
    fDynTextMap->CalcStringWidth( "The quick brown fox jumped over the lazy dog.", &fLineHeight );
    // The line length might have changed due to the new font face, so recalc the line wrapping.
    IRecalcLineStarts(0, true);
}

void pfGUIMultiLineEditCtrl::SetFontSize(uint8_t fontSize)
{
    fFontSize = fontSize;
    fFontFlagsSet |= kFontSizeSet;
    fCalcedFontSize = fontSize;
    fDynTextMap->SetFont( fFontFace, fFontSize, fFontStyle,
                            HasFlag( kXparentBgnd ) ? false : true );
    fDynTextMap->CalcStringWidth( "The quick brown fox jumped over the lazy dog.", &fLineHeight );
    // The line length might have changed due to the new font size, so recalc the line wrapping.
    IRecalcLineStarts(0, true);
}

// are we showing the beginning of the buffer? (controls before us are too far up)
bool pfGUIMultiLineEditCtrl::ShowingBeginningOfBuffer()
{
    if (fScrollPos == 0)
        return true;
    return false;
}

// are we showing the end of the buffer? (controls after us are too far down)
bool pfGUIMultiLineEditCtrl::ShowingEndOfBuffer()
{
    //IRecalcLineStarts(0,true); // This function gets called a lot from the journal book, so IRecalcLineStarts() REALLY slows things
    // down if we're looking at a large amount of text, hopefully we can mess with the existing line starts for now without issue
    if (GetLastVisibleLine() >= (int32_t)fLineStarts.size())
        return true;
    return false;
}

void pfGUIMultiLineEditCtrl::DeleteLinesFromTop(int numLines)
{
    if (fPrevCtrl || fNextCtrl)
        return; // don't do anything

    ST::wchar_buffer buffer = GetCodedBuffer();
    size_t bufferLen = buffer.size();

    for (int curLine = 0; curLine < numLines; ++curLine)
    {
        bool hitEnd = true; // did we hit the end of the buffer before we hit a newline?

        // search for the first newline and nuke it and everything before it
        bool skippingColor = false, skippingStyle = false, skippingLink = false;
        int curColorPos = 0, curStylePos = 0, curLinkPos = 0;
        for (uint32_t curChar = 0; curChar < bufferLen - 1; ++curChar)
        {
            // we need to skip the crappy color and style "tags" so non-character values inside them
            // don't trigger our newline check
            if (!skippingColor && !skippingStyle)
            {
                if (buffer[curChar] == kColorCodeChar)
                {
                    curColorPos = 0;
                    skippingColor = true;
                    continue;
                }
                else if (buffer[curChar] == kStyleCodeChar)
                {
                    curStylePos = 0;
                    skippingStyle = true;
                    continue;
                }
                else if (buffer[curChar] == kLinkCodeChar)
                {
                    curLinkPos = 0;
                    skippingLink = true;
                    continue;
                }
            }

            if (skippingColor)
            {
                ++curColorPos;
                if (curColorPos == kColorCodeSize)
                    skippingColor = false;
                else
                    continue;
            }

            if (skippingStyle)
            {
                ++curStylePos;
                if (curStylePos == kStyleCodeSize)
                    skippingStyle = false;
                else
                    continue;
            }

            if (skippingLink)
            {
                ++curLinkPos;
                if (curLinkPos == kLinkCodeSize)
                    skippingLink = false;
                else
                    continue;
            }

            // if it's a newline, erase it and everything before it
            if ((buffer[curChar] == L'\n') || (buffer[curChar] == L'\r'))
            {
                hitEnd = false;
                uint32_t newBufferStart = curChar + 1; // +1 so we eat the newline as well
                uint32_t newBufferLen = bufferLen - newBufferStart;
                memcpy(buffer.data(), &buffer[newBufferStart], newBufferLen * sizeof(wchar_t)); // copy all bytes after the newline to the beginning
                memset(&buffer[newBufferLen], 0, (bufferLen - newBufferLen) * sizeof(wchar_t)); // fill out the rest of the buffer with null chars
                bufferLen = newBufferLen;
                break;
            }
        }

        if (hitEnd)
        {
            SetBuffer(ST::string()); // we are removing too many (or all) lines, just clear it
            return;
        }

        // still got lines to go, start looking for the next line
    }

    // we got here, so buffer is now our new buffer
    SetBuffer(buffer.c_str(), bufferLen);
    return;
}
