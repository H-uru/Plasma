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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUIMultiLineEditCtrl Header											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIMultiLineEditCtrl_h
#define _pfGUIMultiLineEditCtrl_h

#include "pfGUIControlMod.h"
#include "hsTemplates.h"

#include "../plInputCore/plInputDevice.h"

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
	virtual OnEndOfControlList(Int32 cursorPos) {}

	// we've hit the beginning of the control ist (by moving the cursor)
	virtual OnBeginningOfControlList(Int32 cursorPos) {}
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

		mutable hsTArray<wchar_t>	fBuffer;		// Because AcquireArray() isn't const

		hsTArray<Int32>	fLineStarts;
		UInt16			fLineHeight, fCurrCursorX, fCurrCursorY;
		Int32			fCursorPos, fLastCursorLine;
		hsBool			fIgnoreNextKey, fReadyToRender;
		hsBounds3Ext	fLastP2PArea;
		Int8			fLockCount;
		UInt8			fCalcedFontSize;	// The font size that we calced our line height at

		UInt8			fLastKeyModifiers;
		wchar_t			fLastKeyPressed;

		static wchar_t	fColorCodeChar, fStyleCodeChar;
		static UInt32	fColorCodeSize, fStyleCodeSize;
		
		wchar_t			fLastDeadKey; // if the previous key was a dead key, its value goes here
		wchar_t			fDeadKeyConverter[256][256]; // first index is the dead key, second index is the char to combine it with

		void SetupDeadKeyConverter();

		virtual hsBool	IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		virtual void	IPostSetUpDynTextMap( void );
		virtual void	IUpdate( void );
		void			IUpdate( Int32 startLine, Int32 endLine );

		friend class pfMLScrollProc;

		pfGUIValueCtrl	*fScrollControl;
		pfMLScrollProc	*fScrollProc;
		Int32			fScrollPos;
		Int32			fBufferLimit;

		pfGUIMultiLineEditCtrl *fNextCtrl; // used for linking multiple controls together to share a buffer
		pfGUIMultiLineEditCtrl *fPrevCtrl;

		pfGUIMultiLineEditProc *fEventProc; // where we send events to

		std::string fFontFace;
		hsColorRGBA fFontColor;
		UInt8		fFontSize;
		UInt8		fFontStyle;
		enum flagsSet
		{
			kFontFaceSet = 1,
			kFontColorSet = 2,
			kFontSizeSet = 4,
			kFontStyleSet = 8
		};
		UInt8		fFontFlagsSet;

		int		fTopMargin,fLeftMargin,fBottomMargin,fRightMargin;

		void	IMoveCursor( Direction dir );
		void	IMoveCursorTo( Int32 position );	// Updates selection
		void	ISetCursor( Int32 newPosition );	// Doesn't update selection

		Int32	IRecalcLineStarts( Int32 startingLine, hsBool force, hsBool dontUpdate = false );
		void	IRecalcFromCursor( hsBool forceUpdate = false );
		Int32	IFindCursorLine( Int32 cursorPos = -1 ) const;
		hsBool	IStoreLineStart( UInt32 line, Int32 start );
		void	IOffsetLineStarts( UInt32 position, Int32 offset, hsBool offsetSelectionEnd = false );
		Int32	IPointToPosition( Int16 x, Int16 y, hsBool searchOutsideBounds = false );
		Int32	ICalcNumVisibleLines( void ) const;

		void	IReadColorCode( Int32 &pos, hsColorRGBA &color ) const;
		void	IReadStyleCode( Int32 &pos, UInt8 &fontStyle ) const;
		UInt32	IRenderLine( UInt16 x, UInt16 y, Int32 start, Int32 end, hsBool dontRender = false );
		hsBool	IFindLastColorCode( Int32 pos, hsColorRGBA &color, hsBool ignoreFirstCharacter = false ) const;
		hsBool	IFindLastStyleCode( Int32 pos, UInt8 &style, hsBool ignoreFirstCharacter = false ) const;

		inline static bool	IIsCodeChar( const wchar_t c );
		inline static bool	IIsRenderable( const wchar_t c );
		inline static Int32	IOffsetToNextChar( wchar_t stringChar );
		inline Int32		IOffsetToNextCharFromPos( Int32 pos ) const;

		void	IActuallyInsertColor( Int32 pos, hsColorRGBA &color );
		void	IActuallyInsertStyle( Int32 pos, UInt8 style );

		void	IUpdateScrollRange( void );

		wchar_t	*ICopyRange( Int32 start, Int32 end ) const;

		Int32	ICharPosToBufferPos( Int32 charPos ) const;

		void	IUpdateBuffer();
		void	IUpdateLineStarts();
		void	ISetGlobalBuffer();
		void	ISetLineStarts(hsTArray<Int32> lineStarts);

		void	IHitEndOfControlList(Int32 cursorPos);
		void	IHitBeginningOfControlList(Int32 cursorPos);

	public:

		enum
		{
			kRefScrollCtrl = kRefDerivedStart
		};

		pfGUIMultiLineEditCtrl();
		virtual ~pfGUIMultiLineEditCtrl();

		CLASSNAME_REGISTER( pfGUIMultiLineEditCtrl );
		GETINTERFACE_ANY( pfGUIMultiLineEditCtrl, pfGUIControlMod );

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );

		virtual hsBool	HandleKeyPress( char key, UInt8 modifiers );
		virtual hsBool	HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers );

		virtual void	PurgeDynaTextMapImage();

		// Extended event types
		enum ExtendedEvents
		{
			kValueChanging,
			kScrollPosChanged,
			kKeyPressedEvent
		};

		void	SetScrollPosition( Int32 topLine );
		void	MoveCursor( Direction dir );

		void	InsertChar( char c );
		void	InsertChar( wchar_t c);
		void	InsertString( const char *string );
		void	InsertString( const wchar_t *string );
		void	InsertColor( hsColorRGBA &color );
		void	InsertStyle( UInt8 fontStyle );
		void	DeleteChar( void );
		void	ClearBuffer( void );
		void	SetBuffer( const char *asciiText );
		void	SetBuffer( const wchar_t *asciiText );
		void	SetBuffer( const UInt8 *codedText, UInt32 length );
		void	SetBuffer( const UInt16 *codedText, UInt32 length );
		char	*GetNonCodedBuffer( void ) const;
		wchar_t	*GetNonCodedBufferW( void ) const;
		UInt8	*GetCodedBuffer( UInt32 &length ) const;
		UInt16	*GetCodedBufferW( UInt32 &length ) const;
		UInt32	GetBufferSize();

		void	SetBufferLimit(Int32 limit) { fBufferLimit = limit; }
		Int32	GetBufferLimit() { return fBufferLimit; }

		void	GetThisKeyPressed( char &key, UInt8 &modifiers ) const { key = (char)fLastKeyPressed; modifiers = fLastKeyModifiers; }

		void	Lock( void );
		void	Unlock( void );
		hsBool	IsLocked( void ) const { return ( fLockCount > 0 ) ? true : false; }
		
		void	SetScrollEnable( hsBool state );

		void	ForceUpdate() {/*IRecalcLineStarts(0,true);*/IUpdateLineStarts(); IUpdate();}

		void	SetNext( pfGUIMultiLineEditCtrl *newNext );
		void	ClearNext();
		void	SetPrev( pfGUIMultiLineEditCtrl *newPrev );
		void	ClearPrev();
		void	SetEventProc( pfGUIMultiLineEditProc *eventProc );
		void	ClearEventProc();
		Int32	GetFirstVisibleLine();
		Int32	GetLastVisibleLine();
		Int32	GetNumVisibleLines() {return ICalcNumVisibleLines();}
		void	SetGlobalStartLine(Int32 line);

		void	SetCursorToLoc(Int32 loc) {ISetCursor(loc);}

		void	SetMargins(int top, int left, int bottom, int right);

		UInt8	GetFontSize() {return fFontSize;} // because we're too cool to use the color scheme crap

		void	SetFontFace(std::string fontFace);
		void	SetFontColor(hsColorRGBA fontColor) {fFontColor = fontColor; fFontFlagsSet |= kFontColorSet;}
		void	SetFontSize(UInt8 fontSize);
		void	SetFontStyle(UInt8 fontStyle) {fFontStyle = fontStyle; fFontFlagsSet |= kFontStyleSet;}

		hsBool	ShowingBeginningOfBuffer();
		hsBool	ShowingEndOfBuffer();

		void	DeleteLinesFromTop(int numLines); // cursor and scroll position might be off after this call, not valid on connected controls
};

#endif // _pfGUIMultiLineEditCtrl_h
