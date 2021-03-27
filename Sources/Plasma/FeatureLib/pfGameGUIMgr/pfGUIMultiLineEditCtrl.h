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
//  pfGUIMultiLineEditCtrl Header                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIMultiLineEditCtrl_h
#define _pfGUIMultiLineEditCtrl_h

#include "hsBounds.h"

#include <string_theory/string>
#include <vector>

#include "pfGUIControlMod.h"

#include "plInputCore/plInputDevice.h"

class hsGMaterial;
class pfGUIValueCtrl;
class plMessage;
class pfMLScrollProc;
class plTextGenerator;
struct plUndoAction;

class pfGUIMultiLineEditProc
{
public:
    pfGUIMultiLineEditProc() {}
    virtual ~pfGUIMultiLineEditProc() {}

    // we've hit the end of the control list (by moving the cursor)
    virtual void OnEndOfControlList(int32_t cursorPos) {}

    // we've hit the beginning of the control ist (by moving the cursor)
    virtual void OnBeginningOfControlList(int32_t cursorPos) {}
};

class pfGUIMultiLineEditCtrl : public pfGUIControlMod
{
    public:
        enum Direction
        {
            kLineStart = 1,
            kLineEnd,
            kBufferStart,
            kBufferEnd,
            kOneBack,
            kOneForward,
            kOneWordBack,
            kOneWordForward,
            kOneLineUp,
            kOneLineDown,
            kPageUp,
            kPageDown
        };

    protected:
        std::vector<wchar_t> fBuffer;
        std::vector<int32_t> fLineStarts;
        uint16_t        fLineHeight, fCurrCursorX, fCurrCursorY;
        int32_t         fCursorPos, fLastCursorLine;
        int16_t         fClickedLinkId, fCurrLinkId;
        bool            fReadyToRender;
        hsBounds3Ext    fLastP2PArea;
        int8_t          fLockCount;
        uint8_t         fCalcedFontSize;    // The font size that we calced our line height at

        uint8_t         fLastKeyModifiers;
        wchar_t         fLastKeyPressed;

        bool    IEval(double secs, float del, uint32_t dirty) override; // called only by owner object's Eval()

        void    IPostSetUpDynTextMap() override;
        void    IUpdate() override;
        void    IUpdate(int32_t startLine, int32_t endLine);

        friend class pfMLScrollProc;

        pfGUIValueCtrl  *fScrollControl;
        pfMLScrollProc  *fScrollProc;
        int32_t           fScrollPos;
        int32_t           fBufferLimit;
        bool              fCanUpdate;

        pfGUIMultiLineEditCtrl *fNextCtrl; // used for linking multiple controls together to share a buffer
        pfGUIMultiLineEditCtrl *fPrevCtrl;

        pfGUIMultiLineEditProc *fEventProc; // where we send events to

        ST::string  fFontFace;
        hsColorRGBA fFontColor;
        uint8_t     fFontSize;
        uint8_t     fFontStyle;
        enum flagsSet
        {
            kFontFaceSet = 1,
            kFontColorSet = 2,
            kFontSizeSet = 4,
            kFontStyleSet = 8
        };
        uint8_t       fFontFlagsSet;

        int     fTopMargin,fLeftMargin,fBottomMargin,fRightMargin;

        void    IMoveCursor( Direction dir );
        void    IMoveCursorTo( int32_t position );    // Updates selection
        void    ISetCursor( int32_t newPosition );    // Doesn't update selection

        int32_t   IRecalcLineStarts( int32_t startingLine, bool force, bool dontUpdate = false );
        void    IRecalcFromCursor( bool forceUpdate = false );
        int32_t   IFindCursorLine( int32_t cursorPos = -1 ) const;
        bool    IStoreLineStart(int32_t line, int32_t start);
        void    IOffsetLineStarts( uint32_t position, int32_t offset, bool offsetSelectionEnd = false );
        int32_t   IPointToPosition( int16_t x, int16_t y, bool searchOutsideBounds = false );
        int32_t   ICalcNumVisibleLines() const;

        void    IReadColorCode( int32_t &pos, hsColorRGBA &color ) const;
        void    IReadStyleCode( int32_t &pos, uint8_t &fontStyle ) const;
        uint32_t  IRenderLine( uint16_t x, uint16_t y, int32_t start, int32_t end, bool dontRender = false );
        bool    IFindLastColorCode( int32_t pos, hsColorRGBA &color, bool ignoreFirstCharacter = false ) const;
        bool    IFindLastStyleCode( int32_t pos, uint8_t &style, bool ignoreFirstCharacter = false ) const;

        inline static bool  IIsCodeChar( const wchar_t c );
        inline static bool  IIsRenderable( const wchar_t c );
        inline static int32_t IOffsetToNextChar( wchar_t stringChar );
        inline int32_t        IOffsetToNextCharFromPos( int32_t pos ) const;

        void    IActuallyInsertColor( int32_t pos, hsColorRGBA &color );
        void    IActuallyInsertStyle( int32_t pos, uint8_t style );
        void    IActuallyInsertLink(int32_t pos, int16_t linkId);

        void    IUpdateScrollRange();

        wchar_t *ICopyRange( int32_t start, int32_t end ) const;

        int32_t   ICharPosToBufferPos( int32_t charPos ) const;

        void    IUpdateBuffer();
        void    IUpdateLineStarts();
        void    ISetGlobalBuffer();
        void    ISetLineStarts(const std::vector<int32_t> &lineStarts);

        void    IHitEndOfControlList(int32_t cursorPos);
        void    IHitBeginningOfControlList(int32_t cursorPos);

        bool    IHandleMouse(hsPoint3& mousePt);
        int16_t IFindLink(int32_t cursorPos, uint8_t modifiers) const;
        uint32_t IGetDesiredCursor() const override;

    public:

        enum
        {
            kRefScrollCtrl = kRefDerivedStart
        };

        pfGUIMultiLineEditCtrl();
        virtual ~pfGUIMultiLineEditCtrl();

        CLASSNAME_REGISTER( pfGUIMultiLineEditCtrl );
        GETINTERFACE_ANY( pfGUIMultiLineEditCtrl, pfGUIControlMod );

        bool    MsgReceive(plMessage* pMsg) override;
        
        void Read(hsStream* s, hsResMgr* mgr) override;
        void Write(hsStream* s, hsResMgr* mgr) override;

        bool    FocusOnMouseDown(const hsPoint3& mousePt, uint8_t modifiers) const override;
        void    HandleMouseDown(hsPoint3 &mousePt, uint8_t modifiers) override;
        void    HandleMouseUp(hsPoint3 &mousePt, uint8_t modifiers) override;
        void    HandleMouseDrag(hsPoint3 &mousePt, uint8_t modifiers) override;
        void    HandleMouseHover(hsPoint3& mousePt, uint8_t modifiers) override;

        bool    HandleKeyPress(wchar_t key, uint8_t modifiers) override;
        bool    HandleKeyEvent(pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers) override;

        void    PurgeDynaTextMapImage() override;

        void    UpdateColorScheme() override { fFontFlagsSet = 0; pfGUIControlMod::UpdateColorScheme(); }

        // Extended event types
        enum ExtendedEvents
        {
            kValueChanging,
            kScrollPosChanged,
            kKeyPressedEvent,
            kLinkClicked,
        };

        void    SetScrollPosition( int32_t topLine );
        int32_t GetScrollPosition();
        void    MoveCursor( Direction dir );
        int32_t GetCursor() const { return fCursorPos; }

        void    InsertChar( char c );
        void    InsertChar( wchar_t c);
        void    InsertString( const char *string );
        void    InsertString( const wchar_t *string );

        void    InsertColor( hsColorRGBA &color );
        void    InsertStyle( uint8_t fontStyle );

        /** Inserts a clickable hyperlink at the current cursor position. */
        void    InsertLink(int16_t linkId);

        /** Clears any active hyperlink at the current cursor position. */
        void    ClearLink() { InsertLink(-1); }

        void    DeleteChar();
        void    ClearBuffer();
        void    SetBuffer( const char *asciiText );
        void    SetBuffer( const wchar_t *asciiText );
        void    SetBuffer(const char *codedText, size_t length);
        void    SetBuffer(const wchar_t *codedText, size_t length);
        char    *GetNonCodedBuffer() const;
        wchar_t *GetNonCodedBufferW() const;
        char    *GetCodedBuffer(size_t &length) const;
        wchar_t *GetCodedBufferW(size_t &length) const;
        size_t  GetBufferSize() const { return fBuffer.size() - 1; }

        void    SetBufferLimit(int32_t limit) { fBufferLimit = limit; }
        int32_t   GetBufferLimit() { return fBufferLimit; }

        /** Get the link the mouse is currently over. */
        int16_t GetCurrentLink() const { return fCurrLinkId; }

        void    GetThisKeyPressed( char &key, uint8_t &modifiers ) const { key = (char)fLastKeyPressed; modifiers = fLastKeyModifiers; }

        void    Lock();
        void    Unlock();
        bool    IsLocked() const { return ( fLockCount > 0 ) ? true : false; }
        
        void    SetScrollEnable( bool state );

        void    ForceUpdate() {/*IRecalcLineStarts(0,true);*/IUpdateLineStarts(); IUpdate();}

        void    SetNext( pfGUIMultiLineEditCtrl *newNext );
        void    ClearNext();
        void    SetPrev( pfGUIMultiLineEditCtrl *newPrev );
        void    ClearPrev();
        void    SetEventProc( pfGUIMultiLineEditProc *eventProc );
        void    ClearEventProc();
        int32_t   GetFirstVisibleLine();
        int32_t   GetLastVisibleLine();
        int32_t   GetNumVisibleLines() {return ICalcNumVisibleLines();}
        void    SetGlobalStartLine(int32_t line);

        void    SetCursorToLoc(int32_t loc) {ISetCursor(loc);}

        void    SetMargins(int top, int left, int bottom, int right);

        uint8_t   GetFontSize() {return fFontSize;} // because we're too cool to use the color scheme crap

        void    SetFontFace(const ST::string &fontFace);
        void    SetFontColor(hsColorRGBA fontColor) {fFontColor = fontColor; fFontFlagsSet |= kFontColorSet;}
        void    SetFontSize(uint8_t fontSize);
        void    SetFontStyle(uint8_t fontStyle) {fFontStyle = fontStyle; fFontFlagsSet |= kFontStyleSet;}

        bool    ShowingBeginningOfBuffer();
        bool    ShowingEndOfBuffer();

        void    DeleteLinesFromTop(int numLines); // cursor and scroll position might be off after this call, not valid on connected controls

        /** Signifies that the control will be updated heavily starting now, so suppress all redraws. */
        void BeginUpdate() { fCanUpdate = false; }

        /** Signifies that the massive updates are over. We can now redraw. */
        void EndUpdate(bool redraw=true)
        {
            fCanUpdate = true;
            if (redraw)
                IUpdate();
        }

        bool IsUpdating() const { return !fCanUpdate; }
};

#endif // _pfGUIMultiLineEditCtrl_h
