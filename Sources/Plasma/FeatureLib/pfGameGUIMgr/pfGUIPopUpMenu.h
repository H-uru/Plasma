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
//	pfGUIPopUpMenu Header													//
//																			//
//	Pop-up menus are really just dialogs that know how to create themselves	//
//	and create buttons on themselves to simulate a menu (after all, that's	//
//	all a menu really is anyway).											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIPopUpMenu_h
#define _pfGUIPopUpMenu_h


#include "pfGUIDialogMod.h"
#include "hsBounds.h"

class plMessage;
class pfGUIButtonMod;
class pfPopUpKeyGenerator;
class pfGUICtrlProcObject;
class hsGMaterial;
class plSceneNode;
class pfGUIMenuItemProc;
class pfGUISkin;

class pfGUIPopUpMenu : public pfGUIDialogMod
{
	public:

		enum Alignment
		{
			kAlignUpLeft,
			kAlignUpRight,
			kAlignDownLeft,
			kAlignDownRight		// Default
		};

	protected:

		friend class pfGUIMenuItemProc;

		pfGUIDialogMod		*fParent;	// Pop-up menus also have a sense of who owns them
		plSceneNode			*fParentNode;

		pfPopUpKeyGenerator	*fKeyGen;	// Generates keys for our dynamic objects

		class pfMenuItem
		{
			// Simple wrapper class that tells us how to build our menu
			public:
				std::wstring		fName;
				pfGUICtrlProcObject	*fHandler;
				pfGUIPopUpMenu		*fSubMenu;
				float				fYOffsetToNext;		// Filled in by IBuildMenu()

				pfMenuItem& operator=(const int zero) { fName = L""; fHandler = nil; fSubMenu = nil; fYOffsetToNext = 0; return *this; }
		};

		// Array of info to rebuild our menu from. Note that this is ONLY used when rebuilding
		hsBool					fNeedsRebuilding, fWaitingForSkin;
		hsScalar				fOriginX, fOriginY;
		UInt16					fMargin;
		hsTArray<pfMenuItem>	fMenuItems;
		Int32					fSubMenuOpen;

		pfGUISkin				*fSkin;

		plSceneObject			*fOriginAnchor;
		pfGUIDialogMod			*fOriginContext;

		Alignment				fAlignment;


		hsBool		IBuildMenu( void );
		void		ITearDownMenu( void );

		hsGMaterial	*ICreateDynMaterial( void );

		void		IHandleMenuSomething( UInt32 idx, pfGUIControlMod *ctrl, Int32 extended = -1 );

		void		ISeekToOrigin( void );

	public:

		pfGUIPopUpMenu();
		virtual ~pfGUIPopUpMenu();

		CLASSNAME_REGISTER( pfGUIPopUpMenu );
		GETINTERFACE_ANY( pfGUIPopUpMenu, pfGUIDialogMod );

		enum MenuFlags
		{
			kStayOpenAfterClick = kDerivedFlagsStart,
			kModalOutsideMenus,
			kOpenSubMenusOnHover,
			kScaleWithResolution
		};

		enum Refs
		{
			kRefSkin = kRefDerviedStart,
			kRefSubMenu,
			kRefOriginAnchor,
			kRefOriginContext,
			kRefParentNode
		};

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	SetEnabled( hsBool e );
		virtual hsBool	HandleMouseEvent( pfGameGUIMgr::EventType event, hsScalar mouseX, hsScalar mouseY, UInt8 modifiers );

		void			Show( hsScalar x, hsScalar y );

		void	SetOriginAnchor( plSceneObject *anchor, pfGUIDialogMod *context );
		void	SetAlignment( Alignment a ) { fAlignment = a; }
		void	ClearItems( void );
		void	AddItem( const char *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu = nil );
		void	AddItem( const wchar_t *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu = nil );
		void	SetSkin( pfGUISkin *skin );

		static pfGUIPopUpMenu	*Build( const char *name, pfGUIDialogMod *parent, hsScalar x, hsScalar y, const plLocation &destLoc = plLocation::kGlobalFixedLoc );

};

// Skin definition. Here for now 'cause only the menus use it, but might move it later
class plMipmap;
class pfGUISkin : public hsKeyedObject
{
	public:
		enum Elements
		{
			kUpLeftCorner = 0,
			kTopSpan,
			kUpRightCorner,
			kRightSpan,
			kLowerRightCorner,
			kBottomSpan,
			kLowerLeftCorner,
			kLeftSpan,
			kMiddleFill,
			kSelectedFill,
			kSubMenuArrow,
			kSelectedSubMenuArrow,
			kTreeButtonClosed,
			kTreeButtonOpen,
			kNumElements
		};

		class pfSRect
		{
			public:
				UInt16	fX, fY, fWidth, fHeight;

				void	Empty( void ) { fX = fY = fWidth = fHeight = 0; }
				void	Read( hsStream *s );
				void	Write( hsStream *s );
		};

	protected:

		plMipmap	*fTexture;
		pfSRect		fElements[ kNumElements ];
		UInt16		fItemMargin, fBorderMargin;

	public:

		pfGUISkin();
		pfGUISkin( plMipmap *texture );
		virtual ~pfGUISkin();

		CLASSNAME_REGISTER( pfGUISkin );
		GETINTERFACE_ANY( pfGUISkin, hsKeyedObject );

		enum Refs
		{
			kRefMipmap
		};

		virtual void	Read( hsStream *s, hsResMgr *mgr );
		virtual void	Write( hsStream *s, hsResMgr *mgr );
		virtual hsBool	MsgReceive( plMessage *msg );

		plMipmap		*GetTexture( void ) const { return fTexture; }
		void			SetTexture( plMipmap *tex );

		const pfSRect	&GetElement( UInt32 idx ) const { return fElements[ idx ]; }
		hsBool			IsElementSet( UInt32 idx ) const { return ( fElements[ idx ].fWidth > 0 && fElements[ idx ].fHeight > 0 ); }
		void			SetElement( UInt32 idx, UInt16 x, UInt16 y, UInt16 w, UInt16 h );

		void			SetMargins( UInt16 item, UInt16 border ) { fItemMargin = item; fBorderMargin = border; }
		UInt16			GetItemMargin( void ) const { return fItemMargin; }
		UInt16			GetBorderMargin( void ) const { return fBorderMargin; }
};

#endif // _pfGUIPopUpMenu_h
