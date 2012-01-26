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

#include "pfGUIControlMod.h"
#include "hsTemplates.h"

#include "plInputCore/plInputDevice.h"

class plMessage;
class hsGMaterial;
class plTextGenerator;
class pfMLScrollProc;
class pfGUIValueCtrl;

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

        mutable hsTArray<wchar_t>   fBuffer;        // Because AcquireArray() isn't const

        hsTArray<int32_t> fLineStarts;
        uint16_t          fLineHeight, fCurrCursorX, fCurrCursorY;
        int32_t           fCursorPos, fLastCursorLine;
        hsBool          fIgnoreNextKey, fReadyToRender;
        hsBounds3Ext    fLastP2PArea;
        int8_t            fLockCount;
        uint8_t           fCalcedFontSize;    // The font size that we calced our line height at

        uint8_t           fLastKeyModifiers;
        wchar_t         fLastKeyPressed;

        static wchar_t  fColorCodeChar, fStyleCodeChar;
        static uint32_t   fColorCodeSize, fStyleCodeSize;

        virtual hsBool  IEval( double secs, float del, uint32_t dirty ); // called only by owner object's Eval()

        virtual void    IPostSetUpDynTextMap( void );
        virtual void    IUpdate( void );
        void            IUpdate( int32_t startLine, int32_t endLine );

        friend class pfMLScrollProc;

        pfGUIValueCtrl  *fScrollControl;
        pfMLScrollProc  *fScrollProc;
        int32_t           fScrollPos;
        int32_t           fBufferLimit;

        pfGUIMultiLineEditCtrl *fNextCtrl; // used for linking multiple controls together to share a buffer
        pfGUIMultiLineEditCtrl *fPrevCtrl;

        pfGUIMultiLineEditProc *fEventProc; // where we send events to

        std::string fFontFace;
        hsColorRGBA fFontColor;
        uint8_t       fFontSize;
        uint8_t       fFontStyle;
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

        int32_t   IRecalcLineStarts( int32_t startingLine, hsBool force, hsBool dontUpdate = false );
        void    IRecalcFromCursor( hsBool forceUpdate = false );
        int32_t   IFindCursorLine( int32_t cursorPos = -1 ) const;
        hsBool  IStoreLineStart( uint32_t line, int32_t start );
        void    IOffsetLineStarts( uint32_t position, int32_t offset, hsBool offsetSelectionEnd = false );
        int32_t   IPointToPosition( int16_t x, int16_t y, hsBool searchOutsideBounds = false );
        int32_t   ICalcNumVisibleLines( void ) const;

        void    IReadColorCode( int32_t &pos, hsColorRGBA &color ) const;
        void    IReadStyleCode( int32_t &pos, uint8_t &fontStyle ) const;
        uint32_t  IRenderLine( uint16_t x, uint16_t y, int32_t start, int32_t end, hsBool dontRender = false );
        hsBool  IFindLastColorCode( int32_t pos, hsColorRGBA &color, hsBool ignoreFirstCharacter = false ) const;
        hsBool  IFindLastStyleCode( int32_t pos, uint8_t &style, hsBool ignoreFirstCharacter = false ) const;

        inline static bool  IIsCodeChar( const wchar_t c );
        inline static bool  IIsRenderable( const wchar_t c );
        inline static int32_t IOffsetToNextChar( wchar_t stringChar );
        inline int32_t        IOffsetToNextCharFromPos( int32_t pos ) const;

        void    IActuallyInsertColor( int32_t pos, hsColorRGBA &color );
        void    IActuallyInsertStyle( int32_t pos, uint8_t style );

        void    IUpdateScrollRange( void );

        wchar_t *ICopyRange( int32_t start, int32_t end ) const;

        int32_t   ICharPosToBufferPos( int32_t charPos ) const;

        void    IUpdateBuffer();
        void    IUpdateLineStarts();
        void    ISetGlobalBuffer();
        void    ISetLineStarts(hsTArray<int32_t> lineStarts);

        void    IHitEndOfControlList(int32_t cursorPos);
        void    IHitBeginningOfControlList(int32_t cursorPos);

    public:

        enum
        {
            kRefScrollCtrl = kRefDerivedStart
        };

        pfGUIMultiLineEditCtrl();
        virtual ~pfGUIMultiLineEditCtrl();

        CLASSNAME_REGISTER( pfGUIMultiLineEditCtrl );
        GETINTERFACE_ANY( pfGUIMultiLineEditCtrl, pfGUIControlMod );

        virtual hsBool  MsgReceive( plMessage* pMsg );
        
        virtual void Read( hsStream* s, hsResMgr* mgr );
        virtual void Write( hsStream* s, hsResMgr* mgr );

        virtual void    HandleMouseDown( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseUp( hsPoint3 &mousePt, uint8_t modifiers );
        virtual void    HandleMouseDrag( hsPoint3 &mousePt, uint8_t modifiers );

        virtual hsBool  HandleKeyPress( wchar_t key, uint8_t modifiers );
        virtual hsBool  HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, uint8_t modifiers );

        virtual void    PurgeDynaTextMapImage();

        // Extended event types
        enum ExtendedEvents
        {
            kValueChanging,
            kScrollPosChanged,
            kKeyPressedEvent
        };

        void    SetScrollPosition( int32_t topLine );
        void    MoveCursor( Direction dir );

        void    InsertChar( char c );
        void    InsertChar( wchar_t c);
        void    InsertString( const char *string );
        void    InsertString( const wchar_t *string );
        void    InsertColor( hsColorRGBA &color );
        void    InsertStyle( uint8_t fontStyle );
        void    DeleteChar( void );
        void    ClearBuffer( void );
        void    SetBuffer( const char *asciiText );
        void    SetBuffer( const wchar_t *asciiText );
        void    SetBuffer( const uint8_t *codedText, uint32_t length );
        void    SetBuffer( const uint16_t *codedText, uint32_t length );
        char    *GetNonCodedBuffer( void ) const;
        wchar_t *GetNonCodedBufferW( void ) const;
        uint8_t   *GetCodedBuffer( uint32_t &length ) const;
        uint16_t  *GetCodedBufferW( uint32_t &length ) const;
        uint32_t  GetBufferSize();

        void    SetBufferLimit(int32_t limit) { fBufferLimit = limit; }
        int32_t   GetBufferLimit() { return fBufferLimit; }

        void    GetThisKeyPressed( char &key, uint8_t &modifiers ) const { key = (char)fLastKeyPressed; modifiers = fLastKeyModifiers; }

        void    Lock( void );
        void    Unlock( void );
        hsBool  IsLocked( void ) const { return ( fLockCount > 0 ) ? true : false; }
        
        void    SetScrollEnable( hsBool state );

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

        void    SetFontFace(std::string fontFace);
        void    SetFontColor(hsColorRGBA fontColor) {fFontColor = fontColor; fFontFlagsSet |= kFontColorSet;}
        void    SetFontSize(uint8_t fontSize);
        void    SetFontStyle(uint8_t fontStyle) {fFontStyle = fontStyle; fFontFlagsSet |= kFontStyleSet;}

        hsBool  ShowingBeginningOfBuffer();
        hsBool  ShowingEndOfBuffer();

        void    DeleteLinesFromTop(int numLines); // cursor and scroll position might be off after this call, not valid on connected controls
};

#endif // _pfGUIMultiLineEditCtrl_h
