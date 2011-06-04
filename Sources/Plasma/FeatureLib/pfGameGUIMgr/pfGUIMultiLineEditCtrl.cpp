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
//	pfGUIMultiLineEditCtrl Definition										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIMultiLineEditCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIUpDownPairMod.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogMod.h"
#include "pfGUIDialogHandlers.h"

#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "../plGImage/plDynamicTextMap.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Tiny Helper Class ///////////////////////////////////////////////////////

class plStringSlicer
{
	wchar_t			*fString;
	wchar_t			fTempChar;
	UInt32			fStart, fEnd;

	typedef wchar_t	*CharPtr;

	public:
		plStringSlicer( wchar_t *string, UInt32 start, UInt32 end )
		{
			fString = string;
			fTempChar = string[ end ];
			string[ end ] = 0L;
			fStart = start;
			fEnd = end;
		}

		plStringSlicer( hsTArray<wchar_t> &string, UInt32 start, UInt32 end )
		{
			fString = string.AcquireArray();
			fStart = start;
			if( end < string.GetCount() )
				fEnd = end;
			else
				fEnd = fStart;

			if( fEnd > fStart )
			{
				fTempChar = fString[ end ];
				fString[ end ] = 0L;
			}
		}

		~plStringSlicer()
		{
			if( fEnd > fStart )
				fString[ fEnd ] = fTempChar;
		}

		operator const CharPtr() const
		{
			return &fString[ fStart ];
		}
};

//// Wee Little Control Proc for scrolling ///////////////////////////////////

class pfMLScrollProc : public pfGUICtrlProcObject
{
	protected:

		pfGUIMultiLineEditCtrl	*fParent;

	public:

		pfMLScrollProc( pfGUIMultiLineEditCtrl *parent ) : fParent( parent ) {}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			// Do a check here to make sure we actually changed scroll
			// positions--if not, we don't want to update, since that'll be 
			// slow as hell
			int newScrollPos = (int)fParent->fScrollControl->GetMax() - (int)fParent->fScrollControl->GetCurrValue();
			fParent->SetScrollPosition( newScrollPos );
		}
};


//// Statics /////////////////////////////////////////////////////////////////

wchar_t	pfGUIMultiLineEditCtrl::fColorCodeChar = (wchar_t)1;
wchar_t	pfGUIMultiLineEditCtrl::fStyleCodeChar = (wchar_t)2;
UInt32	pfGUIMultiLineEditCtrl::fColorCodeSize = (wchar_t)5;
UInt32	pfGUIMultiLineEditCtrl::fStyleCodeSize = (wchar_t)3;

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIMultiLineEditCtrl::pfGUIMultiLineEditCtrl()
{
	SetFlag( kWantsInterest );
	SetFlag( kTakesSpecialKeys );
	fCursorPos = 0;
	fLastCursorLine = 0;
	fBuffer.Append( 0L );
	fBufferLimit = -1;
	fIgnoreNextKey = false;
	fScrollControl = nil;
	fScrollProc = nil;
	fScrollPos = 0;
	fReadyToRender = false;


	fLastKeyModifiers = 0;
	fLastKeyPressed = 0;
	fLockCount = 0;

	fLastDeadKey = 0;
	SetupDeadKeyConverter();

	fNextCtrl = nil;
	fPrevCtrl = nil;

	fEventProc = nil;

	fTopMargin = fLeftMargin = 0;
	fBottomMargin = fRightMargin = 0;

	fFontFace = "";
	fFontColor.FromARGB32(0xFF000000);
	fFontSize = 0;
	fFontStyle = 0;
	fFontFlagsSet = 0;
}

pfGUIMultiLineEditCtrl::~pfGUIMultiLineEditCtrl()
{
	ClearNext(); // make sure that no one is referencing us
	ClearPrev();

	if( fScrollProc && fScrollProc->DecRef() )
		delete fScrollProc;
	if (fEventProc)
		delete fEventProc;
}

void pfGUIMultiLineEditCtrl::SetupDeadKeyConverter()
{
	int i,j;
	for (i=0; i<255; i++)
		for (j=0; j<255; j++)
			fDeadKeyConverter[i][j] = 0L;

	// we are adding 100 to the indexes because some of these chars have a negative index for some reason
	fDeadKeyConverter['^'+100]['a'] = L'â';
	fDeadKeyConverter['^'+100]['e'] = L'ê';
	fDeadKeyConverter['^'+100]['i'] = L'î';
	fDeadKeyConverter['^'+100]['o'] = L'ô';
	fDeadKeyConverter['^'+100]['u'] = L'û';
	fDeadKeyConverter['^'+100]['A'] = L'Â';
	fDeadKeyConverter['^'+100]['E'] = L'Ê';
	fDeadKeyConverter['^'+100]['I'] = L'Î';
	fDeadKeyConverter['^'+100]['O'] = L'Ô';
	fDeadKeyConverter['^'+100]['U'] = L'Û';
	
	fDeadKeyConverter['¨'+100]['a'] = L'ä';
	fDeadKeyConverter['¨'+100]['e'] = L'ë';
	fDeadKeyConverter['¨'+100]['i'] = L'ï';
	fDeadKeyConverter['¨'+100]['o'] = L'ö';
	fDeadKeyConverter['¨'+100]['u'] = L'ü';
	fDeadKeyConverter['¨'+100]['A'] = L'Ä';
	fDeadKeyConverter['¨'+100]['E'] = L'Ë';
	fDeadKeyConverter['¨'+100]['I'] = L'Ï';
	fDeadKeyConverter['¨'+100]['O'] = L'Ö';
	fDeadKeyConverter['¨'+100]['U'] = L'Ü';

	fDeadKeyConverter['´'+100]['a'] = L'á';
	fDeadKeyConverter['´'+100]['e'] = L'é';
	fDeadKeyConverter['´'+100]['i'] = L'í';
	fDeadKeyConverter['´'+100]['o'] = L'ó';
	fDeadKeyConverter['´'+100]['u'] = L'ú';
	fDeadKeyConverter['´'+100]['y'] = L'ý';
	fDeadKeyConverter['´'+100]['A'] = L'Á';
	fDeadKeyConverter['´'+100]['E'] = L'É';
	fDeadKeyConverter['´'+100]['I'] = L'Í';
	fDeadKeyConverter['´'+100]['O'] = L'Ó';
	fDeadKeyConverter['´'+100]['U'] = L'Ú';
	fDeadKeyConverter['´'+100]['Y'] = L'Ý';

	fDeadKeyConverter['`'+100]['a'] = L'à';
	fDeadKeyConverter['`'+100]['e'] = L'è';
	fDeadKeyConverter['`'+100]['i'] = L'ì';
	fDeadKeyConverter['`'+100]['o'] = L'ò';
	fDeadKeyConverter['`'+100]['u'] = L'ù';
	fDeadKeyConverter['`'+100]['A'] = L'À';
	fDeadKeyConverter['`'+100]['E'] = L'È';
	fDeadKeyConverter['`'+100]['I'] = L'Ì';
	fDeadKeyConverter['`'+100]['O'] = L'Ò';
	fDeadKeyConverter['`'+100]['U'] = L'Ù';
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIMultiLineEditCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIMultiLineEditCtrl::MsgReceive( plMessage *msg )
{
	plGenRefMsg	*refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
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
				fScrollControl = nil;
			return true;
		}
	}

	return pfGUIControlMod::MsgReceive( msg );
}

//// SetScrollPosition ///////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::SetScrollPosition( Int32 topLine )
{
	if( topLine < 0 )
		topLine = 0;
	else if( topLine > fLineStarts.GetCount() - ICalcNumVisibleLines() + 1 )
		topLine = fLineStarts.GetCount() - ICalcNumVisibleLines() + 1;

	if( fScrollPos == topLine )
		return;

	if (topLine >= 0)
		fScrollPos = topLine;
	else
		fScrollPos = 0;

	IUpdate();

	if( fScrollControl != nil ) 
		// Scroll control values are reversed
		fScrollControl->SetCurrValue( fScrollControl->GetMax() - (hsScalar)fScrollPos );

	HandleExtendedEvent( pfGUIMultiLineEditCtrl::kScrollPosChanged );

	// notify thru the dialog something has changed
	if (fDialog && fDialog->GetHandler())
		fDialog->GetHandler()->DoSomething(this);
}

//// MoveCursor - by direction command////////////////////////////////////////////////
void	pfGUIMultiLineEditCtrl::MoveCursor( Direction dir )
{
	IMoveCursor(dir);
}


//// IUpdateScrollRange //////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::IUpdateScrollRange( void )
{
	if( fScrollControl == nil )
		return;

	if( fLineStarts.GetCount() > ICalcNumVisibleLines() - 1 )
	{
		// +1 here because the last visible line is only a partial, but we want to be able to view
		// full lines all the way to the end.
		hsScalar newMax = (hsScalar)( fLineStarts.GetCount() - ICalcNumVisibleLines() + 1 );

		if( newMax != fScrollControl->GetMax() )
		{
			fScrollControl->SetRange( 0, (hsScalar)(fLineStarts.GetCount() - ICalcNumVisibleLines() + 1) );
			fScrollControl->SetEnabled( true );
			if( fScrollPos > fLineStarts.GetCount() - ICalcNumVisibleLines() + 1 )
			{
				fScrollPos = fLineStarts.GetCount() - ICalcNumVisibleLines() + 1;
				fScrollControl->SetCurrValue( fScrollControl->GetMax() - (hsScalar)fScrollPos );
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

void pfGUIMultiLineEditCtrl::SetScrollEnable( hsBool state )
{
	if (fScrollControl == nil )
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

void	pfGUIMultiLineEditCtrl::IPostSetUpDynTextMap( void )
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

	fDynTextMap->SetFont( fFontFace.c_str(), fFontSize, fFontStyle, 
							HasFlag( kXparentBgnd ) ? false : true );

	// Calculate a height for each line
	fDynTextMap->CalcStringWidth( L"The quick brown fox jumped over the lazy dog.", &fLineHeight );
	fCalcedFontSize = fFontSize;

	fReadyToRender = true;
	IRecalcLineStarts( 0, true );
}

//// ICalcNumVisibleLines ////////////////////////////////////////////////////

Int32	pfGUIMultiLineEditCtrl::ICalcNumVisibleLines( void ) const
{
	if (fDynTextMap == nil || fLineHeight == 0)
		return 0;
	Int32 numLines = 0;
	numLines = (fDynTextMap->GetVisibleHeight() + fLineHeight - (fTopMargin+fBottomMargin+1))/fLineHeight;
	return numLines;
}

//// IUpdate /////////////////////////////////////////////////////////////////
//	Ranged version

void	pfGUIMultiLineEditCtrl::IUpdate( Int32 startLine, Int32 endLine )
{
	hsColorRGBA c;
	static int	testingFlip = 0;
	bool		clearEachLine = true;
	UInt32		line, x, y = 0;
	Int32		numVisibleLines, lastVisibleLine;


	if( !fReadyToRender )
		return;

	// Detect whether we need to recalc all of our dimensions entirely
	if( fFontFlagsSet & (kFontFaceSet & kFontColorSet & kFontSizeSet & kFontStyleSet) )
		IPostSetUpDynTextMap();

	if( fLineStarts.GetCount() == 0 )
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
	numVisibleLines = ICalcNumVisibleLines();
	if (fNextCtrl || fPrevCtrl)
		numVisibleLines--; // we don't want "partially visible" lines
	lastVisibleLine = fScrollPos + numVisibleLines - 1;
	if( lastVisibleLine > fLineStarts.GetCount() - 1 )
		lastVisibleLine = fLineStarts.GetCount() - 1;

	if( startLine < fScrollPos )
		startLine = fScrollPos;
	if( endLine > lastVisibleLine )
		endLine = lastVisibleLine;

	if( startLine == fScrollPos && endLine == lastVisibleLine )
	{
		c.Set( 0.f, 0.f, 0.f, 1.f );
		fDynTextMap->ClearToColor( GetColorScheme()->fBackColor );
		clearEachLine = false;
	}

	// Start at our line
	y = ( startLine - fScrollPos ) * fLineHeight + fTopMargin;
	// And loop!

	for( line = startLine; line <= endLine; line++ )
	{
		// Clear this line
		if( clearEachLine )
		{
			fDynTextMap->FillRect( 0, (UInt16)y, fDynTextMap->GetVisibleWidth(), fLineHeight, 
									GetColorScheme()->fBackColor );
		}

		UInt32 start = fLineStarts[ line ], end;
		if( line == fLineStarts.GetCount() - 1 )
			end = fBuffer.GetCount();
		else
			end = fLineStarts[ line + 1 ];

		// Render the actual text
		IRenderLine( fLeftMargin, (UInt16)y, start, end );

		// Render the cursor
		if( fCursorPos >= start && fCursorPos < end && IsFocused() )
		{
			if( fCursorPos > start )
				x = IRenderLine( fLeftMargin, (UInt16)y, start, fCursorPos, true );
			else
				x = fLeftMargin;
			
			fDynTextMap->FrameRect( (UInt16)x, (UInt16)y, 2, fLineHeight, GetColorScheme()->fSelForeColor );

			// Store the cursor X,Y pair. Go figure, the ONLY time we actually need this is
			// to move up or down one line, and even then it's only because we want to keep
			// the same approximate horizontal position (versus same character offset)
			fCurrCursorX = (UInt16)x;	
			fCurrCursorY = (UInt16)y;
		}
		y += fLineHeight;
	}
	if( clearEachLine && line >= fLineStarts.GetCount() && y < fDynTextMap->GetVisibleHeight()-fBottomMargin )
	{
		// No lines left, so clear the rest of the visible area
		fDynTextMap->FillRect( 0, (UInt16)y, fDynTextMap->GetVisibleWidth(), (UInt16)(fDynTextMap->GetVisibleHeight() - y), 
								GetColorScheme()->fBackColor );
	}
	fDynTextMap->FlushToHost();
}

//// IReadColorCode //////////////////////////////////////////////////////////
//	Reads a color code from the given position and advances the given position
//	appropriately

void	pfGUIMultiLineEditCtrl::IReadColorCode( Int32 &pos, hsColorRGBA &color ) const
{
	UInt16	*buffer = (UInt16 *)fBuffer.AcquireArray() + pos;
	UInt8	r, g, b;


	hsAssert( buffer[ 0 ] == fColorCodeChar, "Invalid position in IReadColorCode()" );
	buffer++;
	r = (UInt8)buffer[ 0 ];
	g = (UInt8)buffer[ 1 ];
	b = (UInt8)buffer[ 2 ];
	pos += fColorCodeSize;		// We have a duplicate code at the end of this block, for searching backwards
	color.Set( r / 255.f, g / 255.f, b / 255.f, fFontColor.a );
}

//// IReadStyleCode //////////////////////////////////////////////////////////
//	Reads a style code from the given position and advances the given position
//	appropriately

void	pfGUIMultiLineEditCtrl::IReadStyleCode( Int32 &pos, UInt8 &fontFlags ) const
{
	UInt16	*buffer = (UInt16 *)fBuffer.AcquireArray() + pos;


	hsAssert( buffer[ 0 ] == fStyleCodeChar, "Invalid position in IReadStyleCode()" );
	fontFlags = (UInt8)buffer[ 1 ];
	pos += fStyleCodeSize;		// We have a duplicate code at the end of this block, for searching backwards
}

inline bool	pfGUIMultiLineEditCtrl::IIsRenderable( const wchar_t c )
{
	return ( !IIsCodeChar( c ) && c != L'\n' && c != L'\t' );
}

inline bool pfGUIMultiLineEditCtrl::IIsCodeChar( const wchar_t c )
{
	return ( c == fColorCodeChar || c == fStyleCodeChar );
}

//// IFindLastCode Functions /////////////////////////////////////////////////
//	Given a position, these functions start at that position and work
//	backward until they find their respective code type, then return that code
//	type. If none is found, they set the given parameter to the default value
//	and return false.

hsBool	pfGUIMultiLineEditCtrl::IFindLastColorCode( Int32 pos, hsColorRGBA &color, hsBool ignoreFirstCharacter ) const
{
	for( ; pos >= 0; pos -= IOffsetToNextCharFromPos( pos - 1 ) )
	{
		if( fBuffer[ pos ] == fColorCodeChar && !ignoreFirstCharacter )
		{
			IReadColorCode( pos, color );
			return true;
		}
		ignoreFirstCharacter = false;
	}
	color = fFontColor; // use our default color
	return false;
}

hsBool	pfGUIMultiLineEditCtrl::IFindLastStyleCode( Int32 pos, UInt8 &style, hsBool ignoreFirstCharacter ) const
{
	for( ; pos >= 0; pos -= IOffsetToNextCharFromPos( pos - 1 ) )
	{
		if( fBuffer[ pos ] == fStyleCodeChar && !ignoreFirstCharacter )
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
//	Renders a null-terminated string to the dynamic text map at the location
//	given. Takes into account style codes and special characters (like returns
//	and tabs). Returns the final X value after rendering.

UInt32	pfGUIMultiLineEditCtrl::IRenderLine( UInt16 x, UInt16 y, Int32 start, Int32 end, hsBool dontRender )
{
	Int32		pos;
	hsColorRGBA	currColor = fFontColor;
	UInt8		currStyle;
	const wchar_t	*buffer = fBuffer.AcquireArray();

	// First, gotta go back from our starting position and find a color and style code to use
	IFindLastColorCode( start, currColor );
	IFindLastStyleCode( start, currStyle );

	fDynTextMap->SetTextColor( currColor, HasFlag( kXparentBgnd ) ? true : false );
	fDynTextMap->SetFont( fFontFace.c_str(), fFontSize, GetColorScheme()->fFontFlags | currStyle, 
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
			if( buffer[ pos ] == fColorCodeChar )
			{
				// Read color and switch to that one
				IReadColorCode( pos, currColor );
				if( !dontRender )
					fDynTextMap->SetTextColor( currColor, HasFlag( kXparentBgnd ) ? true : false );
			}
			else if( buffer[ pos ] == fStyleCodeChar )
			{
				// Read style and switch to that one
				IReadStyleCode( pos, currStyle );
				if( !dontRender )
					fDynTextMap->SetFont( fFontFace.c_str(), fFontSize	, GetColorScheme()->fFontFlags | currStyle, 
											HasFlag( kXparentBgnd ) ? false : true );
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
	if ( fDynTextMap != nil )
		fDynTextMap->PurgeImage();
}

//// IUpdate /////////////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::IUpdate( void )
{
	// Just call the ranged one with a full range
	IUpdate( 0, fLineStarts.GetCount() - 1 );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);

	fScrollControl = nil;
	if( s->ReadBool() )
	{
		fScrollProc = TRACKED_NEW pfMLScrollProc( this );
		fScrollProc->IncRef();
		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefScrollCtrl ), plRefFlags::kActiveRef );
	}
}

void	pfGUIMultiLineEditCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );

	if( fScrollControl != nil )
	{
		s->WriteBool( true );
		mgr->WriteKey( s, fScrollControl->GetKey() );
	}
	else
		s->WriteBool( false );
}

//// IPointToPosition ////////////////////////////////////////////////////////
//	Translates a 2D point on the visible texture surface to a cursor position.

Int32	pfGUIMultiLineEditCtrl::IPointToPosition( Int16 ptX, Int16 ptY, hsBool searchOutsideBounds )
{
	// Find our line
	Int32	line, start, pos, end, lastVisibleLine;
	Int16	x, y;
	
	if (fPrevCtrl)
		fScrollPos = GetFirstVisibleLine(); // update the scroll position if we are linked

	if( searchOutsideBounds )
		lastVisibleLine = fLineStarts.GetCount() - 1;
	else
	{
		lastVisibleLine = fScrollPos + ICalcNumVisibleLines() - 1;
		if( lastVisibleLine > fLineStarts.GetCount() - 1 )
			lastVisibleLine	= fLineStarts.GetCount() - 1;
	}

	line = searchOutsideBounds ? 0 : fScrollPos;
	y = (Int16)(-( fScrollPos - line ) * fLineHeight);
	y += fTopMargin;
	for( ; line < lastVisibleLine; line++, y += fLineHeight )
	{
		if( ptY >= y && ptY < y + fLineHeight )
		{
			if (line < 0)
				break; // abort, and yes, this IS possible with this crappy code

			// Found the line, figure out what character
			start = fLineStarts[ line ];
			end = ( line == fLineStarts.GetCount() - 1 ) ? fBuffer.GetCount() - 1 : fLineStarts[ line + 1 ];

			for( pos = start; pos < end; pos++ )
			{
				x = (Int16)IRenderLine( fLeftMargin, 0, start, pos, true );
				if( x > ptX )
					break;
			}

			// Go figure, our test puts it 1 past no matter what :)
			return pos - 1;
		}
	}

	// Just put us at the end of the last line
	return fBuffer.GetCount() - 1;
}

//// IIsWordBreaker //////////////////////////////////////////////////////////
//	Returns whether the given character is one that can break a line

inline bool	IIsWordBreaker( const wchar_t c )
{
	return ( wcschr( L" \t,.;\n", c ) != nil ) ? true : false;
}

//// IOffsetToNextChar ///////////////////////////////////////////////////////

inline	Int32	pfGUIMultiLineEditCtrl::IOffsetToNextChar( wchar_t stringChar )
{
	if( stringChar == fColorCodeChar )
		return fColorCodeSize;
	else if( stringChar == fStyleCodeChar )
		return fStyleCodeSize;
	else
		return 1;
}

inline	Int32	pfGUIMultiLineEditCtrl::IOffsetToNextCharFromPos( Int32 position ) const
{
	if( position >= 0 )
		return IOffsetToNextChar( fBuffer[ position ] );
	else
		return 1;
}

//// IRecalcLineStarts ///////////////////////////////////////////////////////
//	Recalculates all the word wrapping/line start values, starting at the
//	given line. If not forced, recalc will stop once a calculated line start
//	matches one already stored (this implying that everything after will be
//	the same as well, assuming contents are the same). If this assumption can't
//	be made, force recalc of all the lines.
//	Returns the last line recalced.
//	The starting line is basically the first line whose start might have
//	changed, so we assume as a hint that every line before the starting line
//	is still valid. If startingLine = 0, we recalc 'em all.

Int32	pfGUIMultiLineEditCtrl::IRecalcLineStarts( Int32 startingLine, hsBool force, hsBool dontUpdate )
{
	UInt16		wrapWidth, widthCounter;
	UInt32		charPos = 0, nextPos, startPos, lastStartPos;
	Int32		currLine, realStartingLine;
	bool		firstLine;
	wchar_t		*buffer;
	const wchar_t	wordBreaks[] = L" \t,.";
	const wchar_t	wordSeparators[] = L" \t,.\n";

	if( fPrevCtrl )
		IUpdateBuffer(); // make sure our buffer is correct if we are linked
	
	if( fDynTextMap == nil )
	{
		// Can't calculate anything. Just return invalid
		fLineStarts.Reset();
		IUpdateScrollRange();
		return -1;
	}

	// Figure out our starting character
	if( startingLine > 0 )
	{
		if( startingLine >= fLineStarts.GetCount() )
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

	realStartingLine = currLine;	// For the IUpdate call later

	// Precalculate some helper values
	wrapWidth = fDynTextMap->GetVisibleWidth() - fRightMargin;
	buffer = fBuffer.AcquireArray();
	firstLine = true;
	lastStartPos = (UInt32)-1;

	for( ; charPos < fBuffer.GetCount(); currLine++ )
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

		firstLine = false;

		//// We do a walk where we find the start of the next word (i.e. the end of this word plus
		//// any "white space"), and then see if we can fit everything up to that point. If we can,
		//// keep walking, if not, stop at whatever we had as a starting point.
		nextPos = charPos;
		for( widthCounter = 0; widthCounter < wrapWidth; )
		{
			charPos = nextPos;

			// Are we on a line break?
			if( nextPos >= fBuffer.GetCount() || buffer[ nextPos ] == L'\n' || buffer[ nextPos ] == 0L )
			{
				charPos++;
				break;	// Yup, so do so
			}

			// Find the end of this word
			while( nextPos < fBuffer.GetCount() && !IIsWordBreaker( buffer[ nextPos ] ) )
				nextPos += IOffsetToNextChar( buffer[ nextPos ] );

			// Now we're at some white space, keep going until we hit the next word
			while( nextPos < fBuffer.GetCount() && IIsWordBreaker( buffer[ nextPos ] ) && buffer[ nextPos ] != L'\n' )
				nextPos += IOffsetToNextChar( buffer[ nextPos ] );

			// Now see how much width this is
			widthCounter = (UInt16)IRenderLine( fLeftMargin, 0, startPos, nextPos, true );
			
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
				widthCounter = (UInt16)IRenderLine( fLeftMargin, 0, startPos, nextPos, true );
			}

			charPos = nextPos;
		}

		// Continue on!		
	}

	if( charPos >= fBuffer.GetCount() )
	{
		// Make sure there are no lines stored after this one
		fLineStarts.SetCount( currLine );
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
//	Stores a single line start, expanding the array if necessary.

hsBool	pfGUIMultiLineEditCtrl::IStoreLineStart( UInt32 line, Int32 start )
{
	if( fLineStarts.GetCount() <= line )
	{
		hsAssert( line == fLineStarts.GetCount(), "Trying to store a line way past the end of line starts!" );
		fLineStarts.Expand( line + 1 );
		fLineStarts.SetCount( line + 1 );
		fLineStarts[ line ] = -1;
	}

	hsBool same = ( fLineStarts[ line ] == start ) ? true : false;
	fLineStarts[ line ] = start;
	return same;
}

//// IFindCursorLine /////////////////////////////////////////////////////////
//	Calculates the line the cursor is sitting on

Int32	pfGUIMultiLineEditCtrl::IFindCursorLine( Int32 cursorPos ) const
{
	Int32	line;


	if( cursorPos == -1 )
		cursorPos = fCursorPos;

	for( line = 0; line < fLineStarts.GetCount() - 1; line++ )
	{
		if( fLineStarts[ line + 1 ] > cursorPos )
			break;
	}
	return line;
}

//// IRecalcFromCursor ///////////////////////////////////////////////////////
//	Recalcs starting with the line that the cursor is sitting on

void	pfGUIMultiLineEditCtrl::IRecalcFromCursor( hsBool force )
{
	IRecalcLineStarts( IFindCursorLine(), force );
}

//// IOffsetLineStarts ///////////////////////////////////////////////////////
//	Given the position and offset, offsets all line starts >= that position
//	by the given amount. Used to insert a character and you know that all the
//	line offsets after that character should just bump up one, for example.
//	Also offsets the end of the current selection if desired and in range.
//	Why only the end of the selection? Because in the only cases where we're
//	doing this, we are inserting into (and offseting inside) a selection,
//	so we don't want the start moving around.

void	pfGUIMultiLineEditCtrl::IOffsetLineStarts( UInt32 position, Int32 offset, hsBool offsetSelectionEnd )
{
	Int32	line;

	// Check our first line and make sure offsetting it won't make it invalid.
	// If it will, we need to recalc the line starts entirely (which is fine,
	// since this function is just called to try to optimize out doing so, but
	// when you gotta, you gotta...)
	line = IFindCursorLine( position );
	if( line < fLineStarts.GetCount() - 1 )
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
	for( line = 0; line < fLineStarts.GetCount(); line++ )
	{
		if( fLineStarts[ line ] > position )
			fLineStarts[ line ] += offset;
	}

	// now update all following controls
	if (fNextCtrl)
		fNextCtrl->ISetLineStarts(fLineStarts);
}

//// HandleMouseDown /////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fDynTextMap == nil || !fBounds.IsInside( &mousePt ) )
		return;

	IScreenToLocalPt( mousePt );
	mousePt.fX *= fDynTextMap->GetVisibleWidth();
	mousePt.fY *= fDynTextMap->GetVisibleHeight();

	IMoveCursorTo( IPointToPosition( (Int16)(mousePt.fX), (Int16)(mousePt.fY) ) );
}

//// HandleMouseUp ///////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fDynTextMap == nil || !fBounds.IsInside( &mousePt ) )
		return;

	IScreenToLocalPt( mousePt );
	mousePt.fX *= fDynTextMap->GetVisibleWidth();
	mousePt.fY *= fDynTextMap->GetVisibleHeight();

	IMoveCursorTo( IPointToPosition( (Int16)(mousePt.fX), (Int16)(mousePt.fY) ) );
}

//// HandleMouseDrag /////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fDynTextMap == nil || !fBounds.IsInside( &mousePt ) )
		return;

	IScreenToLocalPt( mousePt );
	mousePt.fX *= fDynTextMap->GetVisibleWidth();
	mousePt.fY *= fDynTextMap->GetVisibleHeight();

	IMoveCursorTo( IPointToPosition( (Int16)(mousePt.fX), (Int16)(mousePt.fY) ) );
}

hsBool	pfGUIMultiLineEditCtrl::HandleKeyPress( char keyIn, UInt8 modifiers )
{
	wchar_t key = (wchar_t)keyIn;

	if ((fPrevCtrl || fNextCtrl) && (fLineStarts.GetCount() <= GetFirstVisibleLine()))
		return true; // we're ignoring if we can't actually edit our visible frame (and we're linked)

	if (modifiers & pfGameGUIMgr::kCtrlDown)
		return true; // we're ignoring ctrl key events

	if( fIgnoreNextKey )
	{
		// So we don't process keys that already got handled by HandleKeyEvent()
		fIgnoreNextKey = false;
		return true;
	}

	// Store info for the event we're about to send out
	fLastKeyModifiers = modifiers;
	fLastKeyPressed = key;
	
	// Send out a key pressed event. 
	HandleExtendedEvent( kKeyPressedEvent );

	// We discard keys when locked only after we give our handler the key
	if( IsLocked() )
		return true;

	if (plKeyboardDevice::KeyIsDeadKey())
	{
		if (fLastDeadKey != 0)
		{
			wchar_t temp = key; // we have two dead keys in a row, print out the old one and store the new one
			key = fLastDeadKey;
			fLastDeadKey = temp;
		}
		else
		{
			fLastDeadKey = key; // store the dead key and don't print it until we get the next char
			return true;
		}
	}

	if (fLastDeadKey != 0) // we have a dead key that needs to be added in
	{
		wchar_t translatedKey = fDeadKeyConverter[(char)fLastDeadKey+100][(char)key];
		if (translatedKey == 0) // no translation possible?
		{
			// so we need to print the dead key, followed by the typed key
			// unless key is a space, then we just type the dead key
			if (key == ' ')
			{
				// Insert character at the current cursor position, then inc the cursor by one
				// Note: we always want selection mode off when we're typing
				InsertChar( fLastDeadKey );
				fLastDeadKey = 0;
				return true;
			}
			// Insert characters at the current cursor position, then inc the cursor by one
			// Note: we always want selection mode off when we're typing
			InsertChar( fLastDeadKey );
			InsertChar( key );
			fLastDeadKey = 0;
			return true;
		}
		// ok, so we have a translated key now, so assign it to our key and print it normally
		key = translatedKey;
		fLastDeadKey = 0;
	}

	// Insert character at the current cursor position, then inc the cursor by one
	// Note: we always want selection mode off when we're typing
	InsertChar( key );
	return true;
}

hsBool	pfGUIMultiLineEditCtrl::HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers )
{
	if( key == KEY_CAPSLOCK )
		return false;

	if ((fPrevCtrl || fNextCtrl) && (fLineStarts.GetCount() <= GetFirstVisibleLine()))
		return true; // we're ignoring if we can't actually edit our visible frame (and we're linked)

	if (modifiers & pfGameGUIMgr::kCtrlDown)
		return true; // we're ignoring ctrl key events

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
		else if( key == KEY_ESCAPE )
		{
//			fEscapedFlag = true;
			DoSomething();		// Query WasEscaped() to see if it was escape vs enter
		}
		else
		{
			fIgnoreNextKey = false;
			return true;
		}

		fIgnoreNextKey = true;
		return true;
	}
	else
		// We don't process them, but we don't want anybody else processing them either
		return true;
}

//// ISetCursor //////////////////////////////////////////////////////////////
//	Only moves the cursor and redraws, doesn't bother with selections. You
//	should probably call IMoveCursorTo() unless you really know what you're
//	doing and don't want the current selection updated.

void	pfGUIMultiLineEditCtrl::ISetCursor( Int32 newPosition )
{
	fCursorPos = newPosition;

	Int32 newLine = IFindCursorLine();

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
			Int32 delta = newLine - ( fScrollPos + ICalcNumVisibleLines() - 2 );
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
//	Moves the cursor in a relative direction.

void	pfGUIMultiLineEditCtrl::IMoveCursor( pfGUIMultiLineEditCtrl::Direction dir )
{
	Int32	cursor = fCursorPos, line, offset, end;


	switch( dir )
	{
		case kLineStart:
			while( cursor > 0 && fBuffer[ cursor - 1 ] != L'\n' )
				cursor--;
			break;

		case kLineEnd:
			while( cursor < fBuffer.GetCount() - 1 && fBuffer[ cursor ] != L'\n' )
				cursor++;
			break;

		case kBufferStart:
			cursor = 0;
			break;

		case kBufferEnd:
			cursor = fBuffer.GetCount() - 1;
			break;

		case kOneBack:
			if( cursor > 0 )
			{
				cursor--;
				while( cursor > 0 && ( fBuffer[ cursor ] == fColorCodeChar || fBuffer[ cursor ] == fStyleCodeChar ) )
					cursor -= IOffsetToNextChar( fBuffer[ cursor ] );
			}
			break;

		case kOneForward:
			if( cursor < fBuffer.GetCount() - 1 )
			{
				cursor++;
				while( cursor < fBuffer.GetCount() - 1 && ( fBuffer[ cursor ] == fColorCodeChar || fBuffer[ cursor ] == fStyleCodeChar ) )
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
			if( cursor < fBuffer.GetCount() - 1 )
			{
				for( ; cursor < fBuffer.GetCount() - 1 && !IIsWordBreaker( fBuffer[ cursor ] ); cursor++ );
				for( ; cursor < fBuffer.GetCount() - 1 && IIsWordBreaker( fBuffer[ cursor ] ); cursor++ );
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
			if( IFindCursorLine( cursor ) < fLineStarts.GetCount() - 1 )
				cursor = IPointToPosition( fCurrCursorX + ( fLineHeight >> 2 ), fCurrCursorY + fLineHeight, true );
			break;

		case kPageUp:
			line = IFindCursorLine( cursor );
			offset = cursor - fLineStarts[ line ];

			line -= ( ICalcNumVisibleLines() - 1 );
			if( line < 0 )
				line = 0;

			end = ( line < fLineStarts.GetCount() - 1 ) ? fLineStarts[ line + 1 ] : fBuffer.GetCount() - 1;
			if( fLineStarts[ line ] + offset > end )
				offset = end - fLineStarts[ line ] - 1;

			cursor = fLineStarts[ line ] + offset;
			break;

		case kPageDown:
			line = IFindCursorLine( cursor );
			offset = cursor - fLineStarts[ line ];

			line += ( ICalcNumVisibleLines() - 1 );
			if( line > fLineStarts.GetCount() - 1 )
				line = fLineStarts.GetCount() - 1;

			end = ( line < fLineStarts.GetCount() - 1 ) ? fLineStarts[ line + 1 ] : fBuffer.GetCount() - 1;
			if( fLineStarts[ line ] + offset > end )
				offset = end - fLineStarts[ line ] - 1;

			cursor = fLineStarts[ line ] + offset;
			break;
	}

	IMoveCursorTo( cursor );
}

//// IMoveCursorTo ///////////////////////////////////////////////////////////
//	Moves the cursor to the given absolute position and updates the selection
//	area accordingly and if necessary.

void	pfGUIMultiLineEditCtrl::IMoveCursorTo( Int32 position )
{
	ISetCursor( position );
}

//////////////////////////////////////////////////////////////////////////////
//// Buffer Modification /////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// InsertChar //////////////////////////////////////////////////////////////
//	Inserts a character at the cursor, or if there is a selection, replaces
//	the selection with the given character. Either way, at the end the cursor
//	is after the inserted character.

void	pfGUIMultiLineEditCtrl::InsertChar( char c )
{
	InsertChar((wchar_t)c); // a simple cast should be fine
}

void	pfGUIMultiLineEditCtrl::InsertChar( wchar_t c )
{
	// Error checking--make sure our actual character isn't in the range of our
	// code characters, or chaos could erupt
	//if( c < 3 )
	//return;
	if (c == 0)
		return;

	if ( fBufferLimit == -1 || fBuffer.GetCount()+1 < fBufferLimit-1)
	{
		fBuffer.Insert( fCursorPos, c );

		ISetGlobalBuffer(); // update the global buffer

		IOffsetLineStarts( fCursorPos, 1 );
		IMoveCursor( kOneForward );
		IRecalcFromCursor();
	}
}

//// InsertString ////////////////////////////////////////////////////////////
//	Same as InsertChar, only with a null-terminated string of characters
//	instead of just one.

void	pfGUIMultiLineEditCtrl::InsertString( const char *string )
{
	wchar_t* temp = hsStringToWString(string);
	InsertString(temp);
	delete [] temp;
}

void	pfGUIMultiLineEditCtrl::InsertString( const wchar_t *string )
{
	int numChars = wcslen( string );

	if ( fBufferLimit == -1 || fBuffer.GetCount() + numChars < fBufferLimit-1)
	{
		// Don't freak out, the cast is OK here. Insert() wants an array of chars.
		// It should really take a const array, since it's just copying them,
		// but since it doesn't and we know it's just copying them, we cast and
		// be happy for now.
		fBuffer.Insert( fCursorPos, numChars, (wchar_t *)string );

		ISetGlobalBuffer(); // update the global buffer

		IOffsetLineStarts( fCursorPos, numChars );
		IMoveCursorTo( fCursorPos + numChars );
		IRecalcFromCursor();
	}
}

//// InsertColor /////////////////////////////////////////////////////////////
//	Inserts a color control code into the buffer, or if there is a selection,
//	places TWO color codes in: one before the selection with the given color
//	and the other after the given selection with the previous color.
//	There is one catch to all of this, though: if we have a selection AND
//	we're inserting a code that conflicts with any code contained in the
//	selection, we remove the conflicting codes.

void	pfGUIMultiLineEditCtrl::InsertColor( hsColorRGBA &color )
{
	IActuallyInsertColor( fCursorPos, color );
	IOffsetLineStarts( fCursorPos, fColorCodeSize );
	fCursorPos += fColorCodeSize;
	IRecalcFromCursor( true );	// Force update of all following lines, since 
								// insertion of this code changes appearance of following characters
}

void	pfGUIMultiLineEditCtrl::IActuallyInsertColor( Int32 pos, hsColorRGBA &color )
{
	if ( fBufferLimit == -1 || fBuffer.GetCount()+4 < fBufferLimit-1 )
	{
		fBuffer.Insert( pos, fColorCodeChar );
		fBuffer.Insert( pos + 1, (UInt8)( color.r * 255.f ) );
		fBuffer.Insert( pos + 2, (UInt8)( color.g * 255.f ) );
		fBuffer.Insert( pos + 3, (UInt8)( color.b * 255.f ) );
		fBuffer.Insert( pos + 4, fColorCodeChar );
	}
}

//// InsertStyle /////////////////////////////////////////////////////////////
//	Same thing as InsertColor(), only with a style code (or two).

void	pfGUIMultiLineEditCtrl::InsertStyle( UInt8 fontStyle )
{
	IActuallyInsertStyle( fCursorPos, fontStyle );

	IOffsetLineStarts( fCursorPos, fStyleCodeSize );
	fCursorPos += fStyleCodeSize;
	IRecalcFromCursor( true );	// Force update of all following lines, since 
								// insertion of this code changes appearance of following characters
}

void	pfGUIMultiLineEditCtrl::IActuallyInsertStyle( Int32 pos, UInt8 style )
{
	if ( fBufferLimit == -1 || fBuffer.GetCount() + 3 < fBufferLimit-1 )
	{
		fBuffer.Insert( pos, fStyleCodeChar );
		fBuffer.Insert( pos + 1, (wchar_t)style );
		fBuffer.Insert( pos + 2, fStyleCodeChar );
	}
}

//// DeleteChar //////////////////////////////////////////////////////////////
//	If there is no selection, deletes the single character that the cursor
//	is in front of. Otherwise, deletes the current selection and places the
//	cursor where the selection used to be.

void	pfGUIMultiLineEditCtrl::DeleteChar( void )
{
	if( fCursorPos < fBuffer.GetCount() - 1 )
	{
		Int32 offset = IOffsetToNextChar( fBuffer[ fCursorPos ] );
		bool forceUpdate = IIsCodeChar( fBuffer[ fCursorPos ] );
		fBuffer.Remove( fCursorPos, offset );

		ISetGlobalBuffer(); // update the global buffer

		IOffsetLineStarts( fCursorPos, -offset );
		IRecalcFromCursor( forceUpdate );
	}
}

//// ICopyRange //////////////////////////////////////////////////////////////
//	Generic coded-to-non-coded conversion. Returns a copy of the string that
//	the caller must free.

wchar_t	*pfGUIMultiLineEditCtrl::ICopyRange( Int32 start, Int32 end ) const
{
	Int32	stringSize, pos;
	wchar_t	*string;


	// First loop, just count how much space we need
	for( stringSize = 0, pos = start; pos < end; pos = pos + IOffsetToNextChar( fBuffer[ pos ] ) )
	{
		if( !IIsCodeChar( fBuffer[ pos ] ) )
			stringSize++;
	}

	// Our string...
	string = TRACKED_NEW wchar_t[ stringSize + 1 ];

	// Now actually copy the characters
	for( stringSize = 0, pos = start; pos < end; pos = pos + IOffsetToNextChar( fBuffer[ pos ] ) )
	{
		if( !IIsCodeChar( fBuffer[ pos ] ) )
			string[ stringSize++ ] = fBuffer[ pos ];
	}
	string[ stringSize++ ] = 0;

	// All done!
	return string;
}

//// ClearBuffer /////////////////////////////////////////////////////////////
//	Clears everything, including the undo list.

void	pfGUIMultiLineEditCtrl::ClearBuffer( void )
{
	fBuffer.Reset();
	fBuffer.Append( 0 );
	fCursorPos = 0;
	fLastCursorLine = 0;
	fScrollPos = 0;
	IRecalcLineStarts( 0, true );
}

//// SetBuffer ///////////////////////////////////////////////////////////////
//	Replaces the entire contents of the buffer with the given text. Also
//	clears the undo list.

void	pfGUIMultiLineEditCtrl::SetBuffer( const char *asciiText )
{
	SetBuffer( (const UInt8 *)asciiText, (UInt32)strlen( asciiText ) );
}

void	pfGUIMultiLineEditCtrl::SetBuffer( const wchar_t *asciiText )
{
	SetBuffer( (const UInt16 *)asciiText, (UInt32)wcslen( asciiText ) );
}

//// SetBuffer ///////////////////////////////////////////////////////////////
//	The non-0-terminated-string version that can handle buffers with style
//	codes in them.

void	pfGUIMultiLineEditCtrl::SetBuffer( const UInt8 *codedText, UInt32 length )
{
	// convert to UInt16 and set
	UInt16 *convertedText = TRACKED_NEW UInt16[ length ];
	for( Int32 curChar = 0; curChar < length; curChar++ )
		convertedText[ curChar ] = (UInt16)codedText[ curChar ];
	SetBuffer(convertedText,length);
	delete [] convertedText;
}

void	pfGUIMultiLineEditCtrl::SetBuffer( const UInt16 *codedText, UInt32 length )
{
	// recursively call back to the first control and set it
	if (fPrevCtrl)
	{
		fPrevCtrl->SetBuffer(codedText,length);
		IUpdateBuffer();
	}
	else // we are the first control, so set our buffer
	{
		fBuffer.Reset();
		// Why o why doesn't Insert() take a const array....
		fBuffer.Insert( 0, length, (wchar_t *)codedText );
		fBuffer.Append( 0 );
		IRecalcLineStarts( 0, true ); // only the first control will recalc, this function recurses down to handle all following controls
	}

	fScrollPos = GetFirstVisibleLine();
	fCursorPos = 0;
	fLastCursorLine = 0;
}

//// GetNonCodedBuffer ///////////////////////////////////////////////////////
//	Copies the entire buffer into an ASCII string (i.e. strips formatting)
//	and returns it. The caller is responsible for freeing it.
//	To avoid code duplication, we'll just cheat and use CopySelection()...

char	*pfGUIMultiLineEditCtrl::GetNonCodedBuffer( void ) const
{
	// recursively search back to the first control in the linked list and grab its buffer
	if (fPrevCtrl)
		return fPrevCtrl->GetNonCodedBuffer();
	else
	{
		// -1 for the null terminator
		wchar_t *buffer = ICopyRange( 0, fBuffer.GetCount() - 1 );
		char *retVal = hsWStringToString(buffer);
		delete [] buffer;

		return retVal;
	}
}

wchar_t	*pfGUIMultiLineEditCtrl::GetNonCodedBufferW( void ) const
{
	// recursively search back to the first control in the linked list and grab its buffer
	if (fPrevCtrl)
		return fPrevCtrl->GetNonCodedBufferW();
	else
	{
		// -1 for the null terminator
		return ICopyRange( 0, fBuffer.GetCount() - 1 );
	}
}

//// GetCodedBuffer //////////////////////////////////////////////////////////
//	Basically does a blanket copy of the entire buffer and returns it and
//	the length. The caller is responsible for freeing the buffer.

UInt8	*pfGUIMultiLineEditCtrl::GetCodedBuffer( UInt32 &length ) const
{
	// recursively search back to the first control in the linked list and grab its buffer
	if (fPrevCtrl)
		return fPrevCtrl->GetCodedBuffer(length);
	else
	{
		length = fBuffer.GetCount() - 1;

		// convert to UInt8 and return
		UInt8 *buffer = TRACKED_NEW UInt8[ length ];

		for (Int32 curChar = 0; curChar < length; curChar++)
		{
			if (fBuffer[ curChar ] > (wchar_t)0xFF)
			{
				// char doesn't fit, fake it with a space
				buffer[ curChar ] = (UInt8)(L' ');
			}
			else
				buffer[ curChar ] = (UInt8)fBuffer[ curChar ];
		}
		return buffer;
	}
}

UInt16	*pfGUIMultiLineEditCtrl::GetCodedBufferW( UInt32 &length ) const
{
	// recursively search back to the first control in the linked list and grab its buffer
	if (fPrevCtrl)
		return fPrevCtrl->GetCodedBufferW(length);
	else
	{
		length = fBuffer.GetCount() - 1;

		UInt16 *buffer = TRACKED_NEW UInt16[ length ];

		// AcquireArray() isn't const...
		memcpy( buffer, &fBuffer[ 0 ], length * sizeof(UInt16) );

		return buffer;
	}
}

UInt32	pfGUIMultiLineEditCtrl::GetBufferSize()
{
	return fBuffer.GetCount() - 1;
}

//// ICharPosToBufferPos /////////////////////////////////////////////////////
//	Given a character position (i.e. a buffer position if we didn't have
//	control codes), returns the actual buffer pos.

Int32	pfGUIMultiLineEditCtrl::ICharPosToBufferPos( Int32 charPos ) const
{
	Int32	pos;


	for( pos = 0; charPos > 0 && pos < fBuffer.GetCount() - 1; pos += IOffsetToNextCharFromPos( pos ), charPos-- );

	return pos;
}

//// Locking /////////////////////////////////////////////////////////////////

void	pfGUIMultiLineEditCtrl::Lock( void )
{
	fLockCount++;
	IUpdate();	
}

void	pfGUIMultiLineEditCtrl::Unlock( void )
{
	fLockCount--;
	//hsAssert( fLockCount >= 0, "Too many unlocks for pfGUIMultiLineEditCtrl" );
	if (fLockCount < 0)
		fLockCount = 0; // instead of asserting, hande it nicely
	IUpdate();	
}

// linking functions, for linking multiple multi-line edit controls together

void	pfGUIMultiLineEditCtrl::SetNext( pfGUIMultiLineEditCtrl *newNext )
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

void	pfGUIMultiLineEditCtrl::ClearNext()
{
	if (fNextCtrl)
		fNextCtrl->fPrevCtrl = nil; // unlink ourselves from the next control
	fNextCtrl = nil;
}

void	pfGUIMultiLineEditCtrl::SetPrev( pfGUIMultiLineEditCtrl *newPrev )
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

void	pfGUIMultiLineEditCtrl::ClearPrev()
{
	if (fPrevCtrl)
		fPrevCtrl->fNextCtrl = nil; // unlink ourselves from the prev control
	fPrevCtrl = nil;
}

void	pfGUIMultiLineEditCtrl::SetEventProc(pfGUIMultiLineEditProc *eventProc)
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

void	pfGUIMultiLineEditCtrl::ClearEventProc()
{
	if (fPrevCtrl)
		fPrevCtrl->ClearEventProc();
	else
	{
		if (fEventProc)
			delete fEventProc;
		fEventProc = nil;
	}
}

Int32	pfGUIMultiLineEditCtrl::GetFirstVisibleLine()
{
	// recursively search back to the first control and work our way forwards to where we are supposed to be
	if (fPrevCtrl)
		return fPrevCtrl->GetLastVisibleLine();
	return fScrollPos; // we're the first control, so we show the first part of the buffer
}

Int32	pfGUIMultiLineEditCtrl::GetLastVisibleLine()
{
	// simply add our number of lines to our first visible line
	return GetFirstVisibleLine()+ICalcNumVisibleLines()-1;
}

void	pfGUIMultiLineEditCtrl::SetGlobalStartLine(Int32 line)
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

void	pfGUIMultiLineEditCtrl::IUpdateLineStarts()
{
	// make sure our line starts array matches the gloabl one
	if (fPrevCtrl)
	{
		fPrevCtrl->IUpdateLineStarts();
		fLineStarts = fPrevCtrl->fLineStarts;
	}
}

void	pfGUIMultiLineEditCtrl::IUpdateBuffer()
{
	// make sure our local buffer matches the global one
	if (fPrevCtrl)
	{
		// copy the buffer from our global one
		UInt32 length;
		UInt16 *codedText = GetCodedBufferW(length);
		fBuffer.Reset();
		fBuffer.Insert( 0, length, (wchar_t *)codedText );
		fBuffer.Append( 0 );
		delete [] codedText;
	}
}

void pfGUIMultiLineEditCtrl::ISetGlobalBuffer()
{
	if (fPrevCtrl)
	{
		fPrevCtrl->fBuffer.Reset();
		int i;
		for (i=0; i<fBuffer.GetCount(); i++)
			fPrevCtrl->fBuffer.Append(fBuffer[i]);
		fPrevCtrl->ISetGlobalBuffer(); // pass the update backwards
	}
}

void pfGUIMultiLineEditCtrl::ISetLineStarts(hsTArray<Int32> lineStarts)
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

void pfGUIMultiLineEditCtrl::IHitEndOfControlList(Int32 cursorPos)
{
	if (fPrevCtrl)
		fPrevCtrl->IHitEndOfControlList(cursorPos);
	else
	{
		if (fEventProc)
			fEventProc->OnEndOfControlList(cursorPos);
	}
}

void pfGUIMultiLineEditCtrl::IHitBeginningOfControlList(Int32 cursorPos)
{
	if (fPrevCtrl)
		fPrevCtrl->IHitBeginningOfControlList(cursorPos);
	else
	{
		if (fEventProc)
			fEventProc->OnBeginningOfControlList(cursorPos);
	}
}

void pfGUIMultiLineEditCtrl::SetFontFace(std::string fontFace)
{
	fFontFace = fontFace;
	fFontFlagsSet |= kFontFaceSet;
	fDynTextMap->SetFont( fFontFace.c_str(), fFontSize, fFontStyle, 
							HasFlag( kXparentBgnd ) ? false : true );
	fDynTextMap->CalcStringWidth( "The quick brown fox jumped over the lazy dog.", &fLineHeight );
}

void pfGUIMultiLineEditCtrl::SetFontSize(UInt8 fontSize)
{
	fFontSize = fontSize;
	fFontFlagsSet |= kFontSizeSet;
	fCalcedFontSize = fontSize;
	fDynTextMap->SetFont( fFontFace.c_str(), fFontSize, fFontStyle, 
							HasFlag( kXparentBgnd ) ? false : true );
	fDynTextMap->CalcStringWidth( "The quick brown fox jumped over the lazy dog.", &fLineHeight );
}

// are we showing the beginning of the buffer? (controls before us are too far up)
hsBool pfGUIMultiLineEditCtrl::ShowingBeginningOfBuffer()
{
	if (fScrollPos == 0)
		return true;
	return false;
}

// are we showing the end of the buffer? (controls after us are too far down)
hsBool pfGUIMultiLineEditCtrl::ShowingEndOfBuffer()
{
	//IRecalcLineStarts(0,true); // This function gets called a lot from the journal book, so IRecalcLineStarts() REALLY slows things
	// down if we're looking at a large amount of text, hopefully we can mess with the existing line starts for now without issue
	if (GetLastVisibleLine() >= fLineStarts.GetCount())
		return true;
	return false;
}

void pfGUIMultiLineEditCtrl::DeleteLinesFromTop(int numLines)
{
	if (fPrevCtrl || fNextCtrl)
		return; // don't do anything

	UInt32 bufferLen = 0;
	UInt16* buffer = GetCodedBufferW(bufferLen);

	if (bufferLen == 0)
	{
		delete [] buffer;
		return;
	}

	for (int curLine = 0; curLine < numLines; ++curLine)
	{
		bool hitEnd = true; // did we hit the end of the buffer before we hit a newline?

		// search for the first newline and nuke it and everything before it
		bool skippingColor = false, skippingStyle = false;
		int curColorPos = 0, curStylePos = 0;
		for (UInt32 curChar = 0; curChar < bufferLen - 1; ++curChar)
		{
			// we need to skip the crappy color and style "tags" so non-character values inside them
			// don't trigger our newline check
			if (!skippingColor && !skippingStyle)
			{
				if (buffer[curChar] == fColorCodeChar)
				{
					curColorPos = 0;
					skippingColor = true;
					continue;
				}
				else if (buffer[curChar] == fStyleCodeChar)
				{
					curStylePos = 0;
					skippingStyle = true;
					continue;
				}
			}

			if (skippingColor)
			{
				++curColorPos;
				if (curColorPos == fColorCodeSize)
					skippingColor = false;
				else
					continue;
			}

			if (skippingStyle)
			{
				++curStylePos;
				if (curStylePos == fStyleCodeSize)
					skippingStyle = false;
				else
					continue;
			}

			// if it's a newline, erase it and everything before it
			if ((buffer[curChar] == L'\n') || (buffer[curChar] == L'\r'))
			{
				hitEnd = false;
				UInt32 newBufferStart = curChar + 1; // +1 so we eat the newline as well
				UInt32 newBufferLen = bufferLen - newBufferStart;
				MemCopy(buffer, buffer + newBufferStart, newBufferLen * sizeof(UInt16)); // copy all bytes after the newline to the beginning
				MemSet(buffer + newBufferLen, 0, (bufferLen - newBufferLen) * sizeof(UInt16)); // fill out the rest of the buffer with null chars
				bufferLen = newBufferLen;
				break;
			}
		}

		if (hitEnd)
		{
			delete [] buffer;
			SetBuffer(L""); // we are removing too many (or all) lines, just clear it
			return;
		}

		// still got lines to go, start looking for the next line
	}

	// we got here, so buffer is now our new buffer
	SetBuffer(buffer, bufferLen);
	delete [] buffer;
	return;
}