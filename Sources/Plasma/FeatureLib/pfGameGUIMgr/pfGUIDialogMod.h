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
//	pfGUIDialogMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDialogMod_h
#define _pfGUIDialogMod_h


#include "../pnModifier/plSingleModifier.h"
#include "pfGameGUIMgr.h"
#include "hsMatrix44.h"

class plMessage;
class plPostEffectMod;
class pfGUIControlMod;
class pfGUIDialogProc;
class pfGUIListElement;
class pfGUIColorScheme;

class pfGUIDialogMod : public plSingleModifier
{
	private:
		pfGUIDialogMod	*fNext, **fPrevPtr;

	protected:

		UInt32		fTagID;		// 0 if none

		UInt32		fVersion;	// Nice for syncing to C++ code

		plPostEffectMod				*fRenderMod;
		hsBool						fEnabled;
		char						fName[ 128 ];
		hsTArray<pfGUIControlMod *>	fControls;
		pfGUIControlMod				*fControlOfInterest;
		pfGUIControlMod				*fFocusCtrl;
		pfGUIControlMod				*fMousedCtrl;	// Which one is the mouse over?
		pfGUIColorScheme			*fColorScheme;

		pfGUIDialogProc				*fHandler;
		plKey						fProcReceiver;		// Non-nil means we handle everything by creating notify messages and sending them to this key

		hsTArray<pfGUIListElement *>	fDragElements;
		hsBool							fDragMode, fDragReceptive;
		pfGUIControlMod					*fDragTarget;
		pfGUIControlMod					*fDragSource;

		plKey			fSceneNodeKey;


		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		void	IHandleDrag( hsPoint3 &mousePt, pfGameGUIMgr::EventType event, UInt8 modifiers );

	public:

		enum
		{
			kRenderModRef = 0,
			kControlRef,
			kRefDerviedStart
		};

		enum Flags
		{
			kModal,
			kDerivedFlagsStart
		};

		pfGUIDialogMod();
		virtual ~pfGUIDialogMod();

		CLASSNAME_REGISTER( pfGUIDialogMod );
		GETINTERFACE_ANY( pfGUIDialogMod, plSingleModifier );


		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		void		SetSceneNodeKey( plKey &key ) { fSceneNodeKey = key; }
		plKey		GetSceneNodeKey( void );

		virtual void	SetEnabled( hsBool e );
		hsBool			IsEnabled( void ) { return fEnabled; }

		const char	*GetName( void ) { return fName; }

		void		ScreenToWorldPoint( hsScalar x, hsScalar y, hsScalar z, hsPoint3 &outPt );
		hsPoint3	WorldToScreenPoint( const hsPoint3 &inPt );

		virtual hsBool	HandleMouseEvent( pfGameGUIMgr::EventType event, hsScalar mouseX, hsScalar mouseY, UInt8 modifiers );
		hsBool			HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers );
		hsBool			HandleKeyPress( char key, UInt8 modifiers );
		void			UpdateInterestingThings( hsScalar mouseX, hsScalar mouseY, UInt8 modifiers, hsBool modalPreset );

		void			SetControlOfInterest( pfGUIControlMod *c );
		pfGUIControlMod	*GetControlOfInterest( void ) const { return fControlOfInterest; }
		UInt32			GetDesiredCursor( void ) const;

		void		UpdateAspectRatio( void );
		void		UpdateAllBounds( void );
		void		RefreshAllControls( void );

		void			AddControl( pfGUIControlMod *ctrl );
		UInt32			GetNumControls( void ) { return fControls.GetCount(); }
		pfGUIControlMod	*GetControl( UInt32 idx ) { return fControls[ idx ]; }

		pfGUIColorScheme	*GetColorScheme( void ) const { return fColorScheme; }

		void	LinkToList( pfGUIDialogMod **prevPtr )
		{
			fNext = *prevPtr;
			if( *prevPtr )
				(*prevPtr)->fPrevPtr = &fNext;
			fPrevPtr = prevPtr;
			*prevPtr = this;
		}

		void	Unlink( void )
		{
			if( fNext )
				fNext->fPrevPtr = fPrevPtr;
			*fPrevPtr = fNext;

			fPrevPtr = nil;
			fNext = nil;
		}

		void			SetFocus( pfGUIControlMod *ctrl );
		void			Show( void );
		void			ShowNoReset( void );
		void			Hide( void );
		hsBool			IsVisible( void ) { return IsEnabled(); }

		pfGUIControlMod	*GetFocus( void ) { return fFocusCtrl; }

		pfGUIDialogMod	*GetNext( void ) { return fNext; }
		UInt32			GetTagID( void ) { return fTagID; }
		pfGUIControlMod	*GetControlFromTag( UInt32 tagID );

		void			SetHandler( pfGUIDialogProc *hdlr );
		pfGUIDialogProc	*GetHandler( void ) const { return fHandler; }

		plPostEffectMod *GetRenderMod( void ) const { return fRenderMod; }

		// This sets the handler for the dialog and ALL of its controls
		void			SetHandlerForAll( pfGUIDialogProc *hdlr );

		// Just a little macro-type thing here
		void			SetControlHandler( UInt32 tagID, pfGUIDialogProc *hdlr );

		/// Methods for doing drag & drop of listElements

		void	ClearDragList( void );
		void	AddToDragList( pfGUIListElement *e );
		void	EnterDragMode( pfGUIControlMod *source );

		UInt32	GetVersion( void ) const { return fVersion; }

		// Export only
		void		SetRenderMod( plPostEffectMod *mod ) { fRenderMod = mod; }
		void		SetName( const char *name ) { hsStrncpy( fName, name, sizeof( fName ) - 1 ); }
		void		AddControlOnExport( pfGUIControlMod *ctrl );
		void		SetTagID( UInt32 id ) { fTagID = id; }
		void		SetProcReceiver( plKey key ) { fProcReceiver = key; }
		void		SetVersion( UInt32 version ) { fVersion = version; }
};

#endif // _pfGUIDialogMod_h
