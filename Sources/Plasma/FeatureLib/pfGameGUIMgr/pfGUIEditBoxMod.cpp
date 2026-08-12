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
//  pfGUIEditBoxMod Definition                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef PLASMA_EXTERNAL_RELEASE
//#define LIMIT_VOICE_CHAT 1
#endif

#include "pfGUIEditBoxMod.h"

#include "HeadSpin.h"
#include "plgDispatch.h"
#include "hsResMgr.h"

#include <string_theory/string_stream>

#include "pfGameGUIMgr.h"

#include "pnInputCore/plKeyMap.h"
#include "pnMessage/plRefMsg.h"

#include "plClipboard/plClipboard.h"
#include "plGImage/plDynamicTextMap.h"
#include "plInputCore/plInputDevice.h"

#include <locale>


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIEditBoxMod::pfGUIEditBoxMod()
    : fEscapedFlag(), fFirstHalfExitKeyPushed(), fSpecialCaptureKeyEventMode(),
      fBuffer(), fSavedKey(), fSavedModifiers()
{
    SetFlag(kWantsInterest);
    SetFlag(kTakesSpecialKeys);
    SetBufferSize(128);
}

//// IEval ///////////////////////////////////////////////////////////////////

bool    pfGUIEditBoxMod::IEval( double secs, float del, uint32_t dirty )
{
    return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

bool    pfGUIEditBoxMod::MsgReceive( plMessage *msg )
{
    return pfGUIControlMod::MsgReceive( msg );
}

//// IPostSetUpDynTextMap ////////////////////////////////////////////////////

void    pfGUIEditBoxMod::IPostSetUpDynTextMap()
{
    hsWeakRef<pfGUIColorScheme> scheme = GetColorScheme();
    fDynTextMap->SetFont( scheme->fFontFace, scheme->fFontSize, scheme->fFontFlags, 
                            HasFlag( kXparentBgnd ) ? false : true );
}

//// IUpdate /////////////////////////////////////////////////////////////////

void    pfGUIEditBoxMod::IUpdate()
{
    hsColorRGBA c;


    if (fDynTextMap == nullptr || !fDynTextMap->IsValid())
        return;

    c.Set( 0.f, 0.f, 0.f, 1.f );
    if ( fFocused && fSpecialCaptureKeyEventMode )
        fDynTextMap->ClearToColor( GetColorScheme()->fSelBackColor );
    else
        fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );

    // First, calc the cursor position, so we can adjust the scrollPos as necessary
    int16_t cursorPos, oldCursorPos;
    if( fFocused && !fSpecialCaptureKeyEventMode )
    {
        // Really cheap hack here to figure out where to draw the cursor
        wchar_t backup = fBuffer[ fCursorPos ];
        fBuffer[ fCursorPos ] = 0;
        cursorPos = fDynTextMap->CalcStringWidth( fBuffer.c_str() );
        fBuffer[ fCursorPos ] = backup;

        oldCursorPos = cursorPos;
        cursorPos -= (int16_t)fScrollPos;

        // plDynamicTextMap::GetVisibleWidth returns an unsigned integer.
        // We have to explicitly convert it to signed,
        // otherwise the entire comparison will become unsigned and behave incorrectly for negative numbers.
        hsAssert(fDynTextMap->GetVisibleWidth() <= INT16_MAX, "Visible width too large for 16-bit int?!");
        int16_t visWidth = static_cast<int16_t>(fDynTextMap->GetVisibleWidth());
        if (4 + cursorPos > visWidth - 4 - 2)
        {
            fScrollPos += (4 + cursorPos) - (visWidth - 4 - 2);
        }
        else if( 4 + cursorPos < 4 )
        {
            fScrollPos -= 4 - ( 4 + cursorPos );
            if( fScrollPos < 0 )
                fScrollPos = 0;
        }

        cursorPos = (int16_t)(oldCursorPos - fScrollPos);
    }

    if ( fFocused && fSpecialCaptureKeyEventMode )
        // if special and has focus then use select
        fDynTextMap->SetTextColor( GetColorScheme()->fSelForeColor, GetColorScheme()->fTransparent &&
                                                                 GetColorScheme()->fSelBackColor.a == 0.f );
    else
        fDynTextMap->SetTextColor( GetColorScheme()->fForeColor, GetColorScheme()->fTransparent &&
                                                                 GetColorScheme()->fBackColor.a == 0.f );
    fDynTextMap->DrawClippedString( (int16_t)(4 - fScrollPos), 4, fBuffer.c_str(), 
                                    4, 4, fDynTextMap->GetVisibleWidth() - 8, fDynTextMap->GetVisibleHeight() - 8 );

    if( fFocused && !fSpecialCaptureKeyEventMode )
    {
        fDynTextMap->FrameRect( 4 + cursorPos, 4, 2, fDynTextMap->GetVisibleHeight() - 8, GetColorScheme()->fSelForeColor );
    }
    fDynTextMap->FlushToHost();
}

static inline bool IIsWordBreaker(const wchar_t c)
{
    if (c == L' ')
        return true;
    if (c == L'\0')
        return true;
    return false;
}

/**
 * Check the next/previous character and adjust our position output variable accordingly.
 */
pfGUIEditBoxMod::CharType pfGUIEditBoxMod::IAdvanceChar(bool next, uint32_t& pos) const
{
    hsAssert(pos >= 0, "out of range");
    hsAssert(pos <= wcslen(fBuffer.c_str()), "out of range");
    hsAssert(next || pos > 0, "advanced left at start of string");

    wchar_t c = next ? fBuffer[pos] : fBuffer[pos - 1];
    if (next)
        pos++;
    else
        pos--;
    if (IIsWordBreaker(c))
        return CharType::kWordBreaker;
    return CharType::kNormal;
}

/**
 * Advance the supplied position by one word left or right.
 */
void pfGUIEditBoxMod::IAdvanceWordFromPos(bool next, uint32_t& pos) const
{
    // If we're moving left, we want to move into the previous word first.
    while (!next && pos > 0) {
        if (IAdvanceChar(next, pos) == CharType::kNormal)
            break;
    }
    // Now, keep moving until we advanced past a word breaker
    while (next && pos < wcslen(fBuffer.c_str()) || !next && pos > 0) {
        if (IAdvanceChar(next, pos) == CharType::kWordBreaker) {
            // If we're moving left, we now want to stop before a word breaker, so go one back when we moved past one.
            if (!next)
                IAdvanceChar(true, pos);
            break;
        }
    }
}

void pfGUIEditBoxMod::IDeleteChar(bool next)
{
    if (!next && fCursorPos > 0 || next && fCursorPos < wcslen(fBuffer.c_str())) {
        if (!next)
            IAdvanceChar(false, fCursorPos);
        memmove(&fBuffer[fCursorPos], &fBuffer[fCursorPos + 1], (wcslen(&fBuffer[fCursorPos + 1]) + 1) * sizeof(wchar_t));
    }
}

void pfGUIEditBoxMod::IDeleteWord(bool next)
{
    uint32_t oldCursor, newCursor;
    oldCursor = newCursor = fCursorPos;
    IAdvanceWordFromPos(next, newCursor);

    if (oldCursor != newCursor) {
        if (!next)
            fCursorPos = newCursor;
        int32_t count = next ? newCursor - oldCursor : oldCursor - newCursor;
        memmove(&fBuffer[fCursorPos], &fBuffer[fCursorPos + count], (wcslen(&fBuffer[fCursorPos + count]) + 1) * sizeof(wchar_t));
    }
}

void pfGUIEditBoxMod::PurgeDynaTextMapImage()
{
    if (fDynTextMap != nullptr)
        fDynTextMap->PurgeImage();
}

//// Read/Write //////////////////////////////////////////////////////////////

void    pfGUIEditBoxMod::Read( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Read(s, mgr);
}

void    pfGUIEditBoxMod::Write( hsStream *s, hsResMgr *mgr )
{
    pfGUIControlMod::Write( s, mgr );
}

//// HandleMouseDown /////////////////////////////////////////////////////////
//  What we do: normal click deselects all and selects the item clicked on
//  (if any). Shift-click and ctrl-click avoids the deselect and toggles
//  the item clicked on.

void    pfGUIEditBoxMod::HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers )
{
    wchar_t backup;
    uint16_t  width;


    if (fDynTextMap != nullptr)
    {
        if( !fBounds.IsInside( &mousePt ) )
            return;

        IScreenToLocalPt( mousePt );

        mousePt.fX *= fDynTextMap->GetVisibleWidth();
        mousePt.fX += fScrollPos - 4;
        for( fCursorPos = 0; fCursorPos < wcslen( fBuffer.c_str() ); fCursorPos++ )
        {
            backup = fBuffer[ fCursorPos + 1 ];
            fBuffer[ fCursorPos + 1 ] = 0;
            width = fDynTextMap->CalcStringWidth( fBuffer.c_str() );
            fBuffer[ fCursorPos + 1 ] = backup;

            if( width > mousePt.fX )
                break;
        }

        IUpdate();
    }
}

//// HandleMouseUp ///////////////////////////////////////////////////////////

void    pfGUIEditBoxMod::HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers )
{
}

//// HandleMouseDrag /////////////////////////////////////////////////////////

void    pfGUIEditBoxMod::HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers )
{
}

bool    pfGUIEditBoxMod::HandleKeyPress( wchar_t key, uint8_t modifiers )
{
    int i = wcslen( fBuffer.c_str() );

    // Insert character at the current cursor position, then inc the cursor by one
    if( i < fBuffer.size() && key != 0 )
    {
        memmove( &fBuffer[fCursorPos + 1], &fBuffer[fCursorPos], (i - fCursorPos + 1) * sizeof(wchar_t) );
        fBuffer[ fCursorPos ] = key;
        fCursorPos++;

        HandleExtendedEvent( kValueChanging );
    }
    IUpdate();
    return true;
}

bool    pfGUIEditBoxMod::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers )
{
    if ( fSpecialCaptureKeyEventMode)
    {
        // handle doing special caputre mode
        if ( event == pfGameGUIMgr::kKeyDown )
        {
#ifdef LIMIT_VOICE_CHAT
            // don't allow them to map the TAB key to anything! 'cause we'll use it later
            if ( key == KEY_TAB)
            {
                fIgnoreNextKey = true;
                fFirstHalfExitKeyPushed = false;
                return true;
            }
#endif
            SetLastKeyCapture((uint32_t)key, modifiers);

            // done capturing... tell the handler
            DoSomething();
        }
        fFirstHalfExitKeyPushed = false;
        return true;
    }
    else
    {
        // HACK for now--pass through caps lock so the runlock stuff will work even while a GUI is up
        if( key == KEY_CAPSLOCK )
            return false;

        if( event == pfGameGUIMgr::kKeyDown || event == pfGameGUIMgr::kKeyRepeat )
        {
            fFirstHalfExitKeyPushed = false;
            // Use arrow keys to do our dirty work
            if( key == KEY_HOME )
            {
                SetCursorToHome();
            }
            else if( key == KEY_END )
            {
                SetCursorToEnd();
            }
            else if (key == KEY_LEFT && modifiers & pfGameGUIMgr::kCtrlDown) {
                IAdvanceWordFromPos(false, fCursorPos);
            }
            else if( key == KEY_LEFT )
            {
                if( fCursorPos > 0 )
                    IAdvanceChar(false, fCursorPos);
            }
            else if (key == KEY_RIGHT && modifiers & pfGameGUIMgr::kCtrlDown) {
                IAdvanceWordFromPos(true, fCursorPos);
            }
            else if (key == KEY_RIGHT)
            {
                if (fCursorPos < wcslen(fBuffer.c_str()))
                    IAdvanceChar(true, fCursorPos);
            }
            else if (key == KEY_BACKSPACE && modifiers & pfGameGUIMgr::kCtrlDown) {
                IDeleteWord(false);
            }
            else if (key == KEY_BACKSPACE)
            {
                IDeleteChar(false);
            }
            else if (key == KEY_DELETE && modifiers & pfGameGUIMgr::kCtrlDown) {
                IDeleteWord(true);
            }
            else if (key == KEY_DELETE)
            {
                IDeleteChar(true);
            }
            else if( key == KEY_ENTER )
            {
                // do nothing here... wait for the keyup event
                fFirstHalfExitKeyPushed = true;
            }
            else if( key == KEY_ESCAPE )
            {
//              // do nothing here... wait for the keyup event
//              fFirstHalfExitKeyPushed = true;
                fEscapedFlag = true;
                DoSomething();      // Query WasEscaped() to see if it was escape vs enter
                return true;
            }
            else if (key == KEY_TAB) 
            {
                // Send notify for python scripts
                HandleExtendedEvent(kWantAutocomplete);
            }
            else if (key == KEY_UP)
            {
                // Send notify for python scripts
                HandleExtendedEvent(kWantMessageHistoryUp);
            }
            else if (key == KEY_DOWN)
            {
                // Send notify for python scripts
                HandleExtendedEvent(kWantMessageHistoryDown);
            }
            else if (modifiers & pfGameGUIMgr::kCtrlDown) 
            {
                if (key == KEY_C) 
                {
                    plClipboard::GetInstance().SetClipboardText(ST::string::from_wchar(fBuffer.c_str(), ST_AUTO_SIZE));
                }
                else if (key == KEY_V)
                {
                    ST::string contents = plClipboard::GetInstance().GetClipboardText();
                    ST::wchar_buffer tmp = contents.to_wchar();
                    size_t len = tmp.size();
                    if (len > 0) {
                        wchar_t* insertTarget = &fBuffer[fCursorPos];
                        size_t bufferTailLen = wcslen(insertTarget);
                        if (fCursorPos + len + bufferTailLen <= fBuffer.size()) {
                            memmove(insertTarget + len, insertTarget, bufferTailLen * sizeof(wchar_t));
                            memcpy(insertTarget, tmp.data(), len * sizeof(wchar_t));
                            fCursorPos += len;
                            HandleExtendedEvent( kValueChanging );
                        }
                    }
                }
            }

            IUpdate();
            return true;
        }
        // wait until the Key up for enter and escape to make sure we capture the whole key
        // ...before we give on focus control
        else if( event == pfGameGUIMgr::kKeyUp )
        {
            if( key == KEY_ENTER )
            {
                if (fFirstHalfExitKeyPushed)
                {
                    // Do jack, just here to filter out it being added to the buffer
                    // Well, ok, actually do *something*. *cough*.
                    DoSomething();
                    fFirstHalfExitKeyPushed = false;
                    return true;
                }
            }
            else if( key == KEY_ESCAPE )
            {
                if (fFirstHalfExitKeyPushed)
                {
//                  fEscapedFlag = true;
//                  DoSomething();      // Query WasEscaped() to see if it was escape vs enter
                    fFirstHalfExitKeyPushed = false;
                    return true;
                }
            }
            fFirstHalfExitKeyPushed = false;
            return true;
        }
        else
        {
            // We don't process them, but we don't want anybody else processing them either
            return true;
        }
    }
}

void    pfGUIEditBoxMod::ClearBuffer()
{
    memset(fBuffer.data(), 0, fBuffer.size());
    fCursorPos = 0;
    fScrollPos = 0;
    IUpdate();
}

void    pfGUIEditBoxMod::SetText( const ST::string& str )
{
    ST::wchar_buffer buf = str.to_wchar();
    wcsncpy( fBuffer.data(), buf.c_str(), fBuffer.size() );
    fCursorPos = 0;
    fScrollPos = 0;
    IUpdate();
}

void    pfGUIEditBoxMod::SetBufferSize( uint32_t size )
{
    fBuffer.allocate(size, 0);
    fCursorPos = 0;
    fScrollPos = 0;
}


void    pfGUIEditBoxMod::SetCursorToHome()
{
    fCursorPos = 0;
}

void    pfGUIEditBoxMod::SetCursorToEnd()
{
    fCursorPos = wcslen( fBuffer.c_str() );
}

void pfGUIEditBoxMod::SetLastKeyCapture(uint32_t key, uint8_t modifiers)
{
    // capture the key
    fSavedKey = (plKeyDef)key;
    fSavedModifiers = modifiers;

    // turn key event into string
    ST::string keyStr = plKeyMap::ConvertVKeyToChar(key);

    if(keyStr.empty())
    {
        if (key < 0x80 && isalnum(key))
        {
            char keyChar = (char)key;
            keyStr = ST::string::from_latin_1(&keyChar, 1);
        }
        else
            keyStr = plKeyMap::GetStringUnmapped();
    }
    else
    {
        // check to see the buffer has ForewardSlash and change it to ForwardSlash
        if (keyStr == "ForewardSlash")
        {
            keyStr = ST_LITERAL("ForwardSlash");
        }
    }

    // Using ST::string_stream here avoids extra allocations
    // because its SSO buffer is much larger than ST::string's.
    ST::string_stream newKey;
    if( modifiers & kShift )
        newKey << plKeyMap::GetStringShift();
    if( modifiers & kCtrl )
        newKey << plKeyMap::GetStringCtrl();
    newKey << keyStr;

    // set something in the buffer to be displayed
    ST::wchar_buffer temp = newKey.to_string().to_wchar();
    wcsncpy( fBuffer.data(), temp.c_str(), fBuffer.size() );

    fCursorPos = 0;
    SetCursorToEnd();
    IUpdate();
}

void pfGUIEditBoxMod::SetChatMode(bool state)
{
    plKeyboardDevice::IgnoreCapsLock(state);
}
