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
//	pfGUIListBoxMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIListBoxMod_h
#define _pfGUIListBoxMod_h

#include "pfGUIControlMod.h"

class plMessage;
class hsGMaterial;
class plTextGenerator;
class pfGUIListElement;
class pfScrollProc;
class pfGUIValueCtrl;

class pfGUIListBoxMod : public pfGUIControlMod
{
	friend class pfScrollProc;

	protected:

		struct plSmallRect
		{
			Int16	fLeft, fTop, fRight, fBottom;

			void	Set( Int16 l, Int16 t, Int16 r, Int16 b );
			hsBool	Contains( Int16 x, Int16 y );

			plSmallRect& operator=(const int zero) { fLeft = fTop = fRight = fBottom = 0; return *this; }
		};

		pfGUIValueCtrl	*fScrollControl;

		pfScrollProc	*fScrollProc;

		hsTArray<pfGUIListElement *>	fElements;
		Int32							fCurrClick, fScrollPos, fCurrHover;
		UInt8							fModsAtDragTime;
		Int32							fMinSel, fMaxSel;
		hsBool							fCheckScroll, fClicking;
		Int32							fSingleSelElement;
		hsBool							fScrollRangeUpdateDeferred;
		hsBool							fLocked, fReadyToRoll;
		hsTArray<plSmallRect>			fElementBounds;
		hsTArray<Int16>					fWrapStartIdxs;


		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		void	ICalcScrollRange( void );
		void	ICalcWrapStarts( void );

		virtual void	IUpdate( void );
		virtual void	IPostSetUpDynTextMap( void );
		virtual UInt32	IGetDesiredCursor( void ) const;

		Int32	IGetItemFromPoint( hsPoint3 &mousePt );
		void	IFindSelectionRange( Int32 *min, Int32 *max );
		void	ISelectRange( Int8 min, Int8 max, hsBool select );

	public:

		pfGUIListBoxMod();
		virtual ~pfGUIListBoxMod();

		CLASSNAME_REGISTER( pfGUIListBoxMod );
		GETINTERFACE_ANY( pfGUIListBoxMod, pfGUIControlMod );

		enum OurFlags
		{
			kSingleSelect = kDerivedFlagsStart,
			kDragAndDropCapable,
			kDisableSelection,
			kDisableKeyActions,
			kAllowMultipleElementsPerRow,
			kScrollLeftToRight,
			kAllowMousePassThrough,
			kGrowLeavesAndProcessOxygen,
			kHandsOffMultiSelect,		// Do multiselect w/o needing ctrl or shift
			kForbidNoSelection
		};

		// Extended event types
		enum ExtendedEvents
		{
			kScrollPosChanged,
			kItemAdded,
			kItemRemoved,
			kListCleared
		};

		enum
		{
			kRefScrollCtrl = kRefDerivedStart
		};

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseHover( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDblClick( hsPoint3 &mousePt, UInt8 modifiers );

		virtual hsBool	HandleKeyPress( char key, UInt8 modifiers );
		virtual hsBool	HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers );

		virtual hsBool	FilterMousePosition( hsPoint3 &mousePt );

		virtual void PurgeDynaTextMapImage();

		// Returns selected element. Only valid for kSingleSelect list boxes
		Int32	GetSelection( void ) { return fSingleSelElement; }
		void	SetSelection( Int32 item );
		void	RemoveSelection( Int32 item );
		void	AddSelection( Int32 item );
		
		virtual void	ScrollToBegin( void );
		virtual void	ScrollToEnd( void );
		virtual void	SetScrollPos( Int32 pos );
		virtual Int32	GetScrollPos( void );
		virtual Int32	GetScrollRange( void );


		void	Refresh( void ) { IUpdate(); }

		virtual void		SetColorScheme( pfGUIColorScheme *newScheme );

		// Element manipulation

		UInt16	AddElement( pfGUIListElement *el );
		void	RemoveElement( UInt16 index );
		Int16	FindElement( pfGUIListElement *toCompareTo );
		void	ClearAllElements( void );

		void	LockList( void );
		void	UnlockList( void );

		UInt16				GetNumElements( void );
		pfGUIListElement	*GetElement( UInt16 idx );

		UInt16	AddString( const char *string );
		UInt16	AddString( const wchar_t *string );
		Int16	FindString( const char *toCompareTo );
		Int16	FindString( const wchar_t *toCompareTo );

		// Export only
		void	SetScrollCtrl( pfGUIValueCtrl *ctrl ) { fScrollControl = ctrl; }
		void	SetSingleSelect( hsBool yes ) { if( yes ) SetFlag( kSingleSelect ); else ClearFlag( kSingleSelect ); }
};

#endif // _pfGUIListBoxMod_h
