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
//	pfGUIControlMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIControlMod_h
#define _pfGUIControlMod_h


#include "../pnModifier/plSingleModifier.h"
#include "hsBounds.h"
#include "../plMessage/plInputEventMsg.h"
#include "pfGameGUIMgr.h"
#include "hsColorRGBA.h"
#include "hsRefCnt.h"

class plMessage;
class plPostEffectMod;
class pfGUIDialogMod;
class pfGUICtrlProcObject;
class pfGUIDropTargetProc;
class plDynamicTextMap;
class plLayerInterface;

//// pfGUIColorScheme ////////////////////////////////////////////////////////
//	Tiny helper wrapper for a set of colors used to draw various controls

class pfGUIColorScheme : public hsRefCnt
{
	public:
		hsColorRGBA	fForeColor, fBackColor;
		hsColorRGBA	fSelForeColor, fSelBackColor;
		hsBool		fTransparent;

		char		*fFontFace;
		UInt8		fFontSize;
		UInt8		fFontFlags;

		enum FontFlags
		{
			kFontBold		= 0x01,
			kFontItalic		= 0x02,
			kFontShadowed	= 0x04
		};

		pfGUIColorScheme();
		~pfGUIColorScheme();
		pfGUIColorScheme( hsColorRGBA &foreColor, hsColorRGBA &backColor );
		pfGUIColorScheme( const char *face, UInt8 size, UInt8 fontFlags );

		void	SetFontFace( const char *face );

		void	Read( hsStream *s );
		void	Write( hsStream *s );

		hsBool	IsBold( void ) { return ( fFontFlags & kFontBold ) ? true : false; }
		hsBool	IsItalic( void ) { return ( fFontFlags & kFontItalic ) ? true : false; }
		hsBool	IsShadowed( void ) { return ( fFontFlags & kFontShadowed ) ? true : false; }

	protected:

		void	IReset( void );
};

//// Class Def ///////////////////////////////////////////////////////////////

class pfGUISkin;
class pfGUIControlMod : public plSingleModifier
{
	friend class pfGUIDialogMod;

	protected:

		UInt32				fTagID;
		hsBool				fEnabled, fFocused, fVisible, fInteresting;
		hsBool				fNotifyOnInteresting;
		pfGUIDialogMod		*fDialog;

		hsBounds3			fBounds, fInitialBounds;		// Z component is 0-1
		hsScalar			fScreenMinZ;	// Closest Z coordinate in screen space
		hsPoint3			fScreenCenter;
		hsBool				fBoundsValid, fCenterValid;
		hsMatrix44			fXformMatrix;	// Only used for doing drag work, etc.

		pfGUICtrlProcObject	*fHandler;
		pfGUIDropTargetProc *fDropTargetHdlr;

		plDynamicTextMap	*fDynTextMap;	// Some controls use this; for others, it'll be nil
		plLayerInterface	*fDynTextLayer;	// Juse so we can reset the transform. Sheesh!

		pfGUIColorScheme	*fColorScheme;
		plSceneObject		*fProxy;

		hsTArray<hsPoint3>	fBoundsPoints;		// For more accurate bounds tests

		hsTArray<int>	fSoundIndices;	// Indices of sounds to trigger on the target SO's audible interface

		pfGUISkin		*fSkin;


		hsBool			ISetUpDynTextMap( plPipeline *pipe );
		virtual void	IPostSetUpDynTextMap( void ) {}
		virtual void	IGrowDTMDimsToDesiredSize( UInt16 &width, UInt16 &height ) { }

		virtual hsBool	IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		void			ISetDialog( pfGUIDialogMod *mod ) { fDialog = mod; }
		void			IScreenToLocalPt( hsPoint3 &pt );

		virtual void	IUpdate( void ) {;}
		void			ISetHandler( pfGUICtrlProcObject *h, hsBool clearInheritFlag = false );

		void			IPlaySound( UInt8 guiCtrlEvent, hsBool loop = false );
		void			IStopSound( UInt8 guiCtrlEvent );

		virtual UInt32		IGetDesiredCursor( void ) const { return 0; }	// As specified in plInputInterface.h

	public:

		pfGUIControlMod();
		virtual ~pfGUIControlMod();

		CLASSNAME_REGISTER( pfGUIControlMod );
		GETINTERFACE_ANY( pfGUIControlMod, plSingleModifier );


		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		UInt32		GetTagID( void ) { return fTagID; }

		virtual void	SetEnabled( hsBool e );
		virtual hsBool	IsEnabled( void ) { return fEnabled; }
		virtual void	SetFocused( hsBool e );
		virtual hsBool	IsFocused( void ) { return fFocused; }
		virtual void	SetVisible( hsBool vis );
		virtual hsBool	IsVisible( void ) { return fVisible; }

		virtual void	SetInteresting( hsBool i );
		hsBool			IsInteresting( void ) { return fInteresting; }

		virtual void	SetNotifyOnInteresting( hsBool state ) { fNotifyOnInteresting = state; }

		pfGUIDialogMod	*GetOwnerDlg( void ) { return fDialog; }
		
		virtual void	Refresh( void );

		virtual void	UpdateBounds( hsMatrix44 *invXformMatrix = nil, hsBool force = false );
		void			SetObjectCenter( hsScalar x, hsScalar y );
		virtual hsPoint3 GetObjectCenter() { return fScreenCenter; }
		hsScalar		GetScreenMinZ( void ) { return fScreenMinZ; }
		void			CalcInitialBounds( void );

		const hsBounds3	&GetBounds( void );
		hsBool			PointInBounds( const hsPoint3 &point );

		virtual void	SetTarget( plSceneObject *object );

		// Return false if you actually DON'T want the mouse clicked at this point (should only be used for non-rectangular region rejection)
		virtual hsBool	FilterMousePosition( hsPoint3 &mousePt ) { return true; }

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers ) {;}
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers ) {;}
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers ) {;}
		virtual void	HandleMouseHover( hsPoint3 &mousePt, UInt8 modifiers ) {;}
		virtual void	HandleMouseDblClick( hsPoint3 &mousePt, UInt8 modifiers ) {;}

		virtual hsBool	HandleKeyPress( char key, UInt8 modifiers );
		virtual hsBool	HandleKeyEvent( pfGameGUIMgr::EventType event, plKeyDef key, UInt8 modifiers );

		void			SetHandler( pfGUICtrlProcObject *h ) { ISetHandler( h, true ); }
		void			DoSomething( void );						// Will call the handler
		void			HandleExtendedEvent( UInt32 event );		// Will call the handler

		pfGUICtrlProcObject	*GetHandler( void ) const { return fHandler; }

		void					SetDropTargetHdlr( pfGUIDropTargetProc *drop );
		pfGUIDropTargetProc		*GetDropTargetHdlr( void ) { return fDropTargetHdlr; }

		enum
		{
			kRefDynTextMap,
			kRefDynTextLayer,
			kRefProxy,
			kRefSkin,
			kRefDerivedStart	= 32
		};

		enum Flags	// plSingleModifier already has SetFlag()/HasFlag()
		{
			kWantsInterest,
			kInheritProcFromDlg,
			kIntangible,		// I.E. it doesn't exists on the screen/can't be clicked on.
								// Used for group objects like the up/down pair
			kXparentBgnd,
			kScaleTextWithResolution,		// I.E. take up the same space on screen no matter the resolution
			kTakesSpecialKeys,				// I.E. disable bindings for keys like backspace because we want them
			kHasProxy,
			kBetterHitTesting,
			kDerivedFlagsStart = 32
		};

		virtual void		SetColorScheme( pfGUIColorScheme *newScheme );
		pfGUIColorScheme	*GetColorScheme( void ) const;

		// should be override by specific GUIcontrol
		virtual void		PurgeDynaTextMapImage() {;}

		// Override from plModifier so we can update our bounds
		virtual void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l);

		// Forces an immediate play of the given GUI control event sound
		void	PlaySound( UInt8 guiCtrlEvent, hsBool loop = false ) { IPlaySound( guiCtrlEvent, loop ); }
		void	StopSound( UInt8 guiCtrlEvent ) { IStopSound( guiCtrlEvent ); }

		// Export only
		void		SetTagID( UInt32 id ) { fTagID = id; }
		void		SetDynTextMap( plLayerInterface *layer, plDynamicTextMap *dynText );
		void		SetSoundIndex( UInt8 guiCtrlEvent, int soundIndex );
};

#endif // _pfGUIControlMod_h
