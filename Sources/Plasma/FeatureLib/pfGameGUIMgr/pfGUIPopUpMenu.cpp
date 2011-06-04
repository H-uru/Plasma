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

#include "hsTypes.h"
#include "pfGameGUIMgr.h"
#include "pfGUIPopUpMenu.h"
#include "pfGUIMenuItem.h"
#include "pfGUIButtonMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogNotifyProc.h"
#include "pfGUIControlHandlers.h"
#include "pfGUICtrlGenerator.h"

#include "plgDispatch.h"
#include "hsResMgr.h"

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayer.h"
#include "../plGImage/plDynamicTextMap.h"
#include "../plMessage/plLayRefMsg.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plDrawInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnMessage/plIntRefMsg.h"
#include "../pnMessage/plObjRefMsg.h"
#include "../pnMessage/plNodeRefMsg.h"

#include "../plScene/plPostEffectMod.h"
#include "../plScene/plSceneNode.h"
#include "../pnMessage/plClientMsg.h"

#include "plViewTransform.h"
#include "../plPipeline/plDebugText.h"


class pfPopUpKeyGenerator
{
	public:
		char		fPrefix[ 128 ];
		UInt32		fKeyCount;
		plLocation	fLoc;

		pfPopUpKeyGenerator( const char *p, const plLocation &loc )
		{
			strcpy( fPrefix, p );
			fLoc = loc;
		}

		plKey	CreateKey( hsKeyedObject *ko )
		{
			char name[ 256 ];
			sprintf( name, "%s-%d", fPrefix, fKeyCount++ );

			return hsgResMgr::ResMgr()->NewKey( name, ko, fLoc );
		}
};

//// Router Proc So The Parent Can Handle Click Events ///////////////////////

class pfGUIMenuItemProc : public pfGUICtrlProcObject
{
	protected:

		pfGUIPopUpMenu		*fParent;
		UInt32				fIndex;

	public:

		pfGUIMenuItemProc( pfGUIPopUpMenu *parent, UInt32 idx )
		{
			fParent = parent;
			fIndex = idx;
		}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			fParent->IHandleMenuSomething( fIndex, ctrl );
		}

		virtual void	HandleExtendedEvent( pfGUIControlMod *ctrl, UInt32 event )
		{
			fParent->IHandleMenuSomething( fIndex, ctrl, (Int32)event );
		}
};

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIPopUpMenu::pfGUIPopUpMenu()
{
	fNeedsRebuilding = false;
	fParent = nil;
	fKeyGen = nil;
	fSubMenuOpen = -1;
	SetFlag( kModalOutsideMenus );
	fMargin = 4;
	fSkin = nil;
	fWaitingForSkin = false;

	fParentNode = nil;
	fOriginX = fOriginY = 0.f;
	fOriginAnchor = nil;
	fOriginContext = nil;

	fAlignment = kAlignDownRight;
}

pfGUIPopUpMenu::~pfGUIPopUpMenu()
{
	SetSkin( nil );

//	if( fParentNode != nil )
//		fParentNode->GetKey()->UnRefObject();

	ITearDownMenu();
	ClearItems();

	delete fKeyGen;
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIPopUpMenu::MsgReceive( plMessage *msg )
{
	plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
	if( ref != nil )
	{
		if( ref->fType == kRefSkin )
		{
			if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fSkin = pfGUISkin::ConvertNoRef( ref->GetRef() );
				fWaitingForSkin = false;
			}
			else
				fSkin = nil;

			fNeedsRebuilding = true;
			if( IsVisible() )
			{
				// Rebuild NOW
				ITearDownMenu();
				IBuildMenu();
			}
			return true;
		}
		else if( ref->fType == kRefSubMenu )
		{
			if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fMenuItems[ ref->fWhich ].fSubMenu = pfGUIPopUpMenu::ConvertNoRef( ref->GetRef() );
			else
				fMenuItems[ ref->fWhich ].fSubMenu = nil;
			return true;
		}
		else if( ref->fType == kRefOriginAnchor )
		{
			if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fOriginAnchor = plSceneObject::ConvertNoRef( ref->GetRef() );
			else
				fOriginAnchor = nil;
			return true;
		}
		else if( ref->fType == kRefOriginContext )
		{
			if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fOriginContext = pfGUIDialogMod::ConvertNoRef( ref->GetRef() );
			else
				fOriginContext = nil;
			return true;
		}
		else if( ref->fType == kRefParentNode )
		{
			if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
				fParentNode = plSceneNode::ConvertNoRef( ref->GetRef() );
			else
				fParentNode = nil;
			return true;
		}
	}

	return pfGUIDialogMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIPopUpMenu::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIDialogMod::Read( s, mgr );

	// In case we need it...
	fKeyGen = TRACKED_NEW pfPopUpKeyGenerator( GetName(), GetKey()->GetUoid().GetLocation() );

	fOriginX = fOriginY = -1.f;

	fMargin = s->ReadSwap16();

	UInt32 i, count = s->ReadSwap32();
	fMenuItems.SetCountAndZero( count );
	for( i = 0; i < count; i++ )
	{
		char readTemp[ 256 ];
		s->Read( sizeof( readTemp ), readTemp );
		wchar_t *wReadTemp = hsStringToWString( readTemp );
		fMenuItems[ i ].fName = wReadTemp;
		delete [] wReadTemp;
		
		fMenuItems[ i ].fHandler = pfGUICtrlProcWriteableObject::Read( s );

		mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, i, kRefSubMenu ), plRefFlags::kActiveRef );
	}

	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginAnchor ), plRefFlags::kPassiveRef );
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginContext ), plRefFlags::kPassiveRef );

	fAlignment = (Alignment)s->ReadByte();

	fNeedsRebuilding = true;
}

void	pfGUIPopUpMenu::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIDialogMod::Write( s, mgr );


	s->WriteSwap16( fMargin );

	s->WriteSwap32( fMenuItems.GetCount() );
	UInt32 i;
	for( i = 0; i < fMenuItems.GetCount(); i++ )
	{
		char writeTemp[ 256 ];
		char *sName = hsWStringToString( fMenuItems[ i ].fName.c_str() );
		strncpy( writeTemp, sName, sizeof( writeTemp ) );
		delete [] sName;
		s->Write( sizeof( writeTemp ), writeTemp );

		// Write the handler out (if it's not a writeable, damn you)
		pfGUICtrlProcWriteableObject::Write( (pfGUICtrlProcWriteableObject *)fMenuItems[ i ].fHandler, s );

		mgr->WriteKey( s, fMenuItems[ i ].fSubMenu );
	}

	// Note: we force parentNode to nil here because we only use it when we dynamically
	// create nodes at runtime and need to unref and destroy them later. Since we're
	// reading from disk, we'll already have a sceneNode somewhere, so we don't need
	// this.
	fParentNode = nil;

	mgr->WriteKey( s, fSkin );
	mgr->WriteKey( s, fOriginAnchor );
	mgr->WriteKey( s, fOriginContext );

	s->WriteByte( (UInt8)fAlignment );
}

void	pfGUIPopUpMenu::SetOriginAnchor( plSceneObject *anchor, pfGUIDialogMod *context )
{
	fOriginAnchor = anchor; 
	fOriginContext = context; 
	hsgResMgr::ResMgr()->AddViaNotify( fOriginAnchor->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginAnchor ), plRefFlags::kPassiveRef );
	hsgResMgr::ResMgr()->AddViaNotify( fOriginContext->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefOriginContext ), plRefFlags::kPassiveRef );
}

//// SetEnabled //////////////////////////////////////////////////////////////

void	pfGUIPopUpMenu::SetEnabled( hsBool e )
{
	if( e && fNeedsRebuilding )
	{
		// Make sure our menu is rebuilt before enabling
		ITearDownMenu();
		IBuildMenu();
	}
	else if( !e )
	{
		if( fParent != nil )
			pfGUIPopUpMenu::ConvertNoRef( fParent )->fSubMenuOpen = -1;

		// Hide our submenus if we have any open
		if( fSubMenuOpen != -1 )
		{
			fMenuItems[ fSubMenuOpen ].fSubMenu->Hide();
			fSubMenuOpen = -1;
		}
	}

	pfGUIDialogMod::SetEnabled( e );
}

void	pfGUIPopUpMenu::Show( hsScalar x, hsScalar y )
{
	fOriginX = x;
	fOriginY = y;
	pfGUIDialogMod::Show();	// C++ is kinda stupid if it can't find this naturally
	ISeekToOrigin();
}

void	pfGUIPopUpMenu::ISeekToOrigin( void )
{
#if 0
	UInt32 i;
	float	x = 0.5f/*fOriginX*/, y = fOriginY;

	for( i = 0; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ] != nil )
		{
			fControls[ i ]->SetObjectCenter( x, y );

//			const hsBounds3 &bnds = fControls[ i ]->GetBounds();
			y += fMenuItems[ i ].fYOffsetToNext;//bnds.GetMaxs().fY - bnds.GetMins().fY;

/*			hsMatrix44 p2l, l2p = GetTarget()->GetLocalToWorld();

			hsPoint3 center, origin;
			ScreenToWorldPoint( fOriginX, fOriginY, 100.f, center );
			ScreenToWorldPoint( 0.f, 0.f, 100.f, origin );

			center = origin - center;

			center.fZ = 0.f;
			l2p.SetTranslate( &center );
			l2p.GetInverse( &p2l );

			GetTarget()->SetTransform( l2p, p2l );
*/		}
	}
#endif
}

//// IHandleMenuSomething ////////////////////////////////////////////////////
//	Handles a normal event from one of the item controls.

void	pfGUIPopUpMenu::IHandleMenuSomething( UInt32 idx, pfGUIControlMod *ctrl, Int32 extended )
{
	if( extended != -1 )
	{
		if( fSubMenuOpen != -1 && fSubMenuOpen != idx )
		{
			// Better close the submenu(s)
			fMenuItems[ fSubMenuOpen ].fSubMenu->Hide();
			fSubMenuOpen = -1;
		}

		if( extended == pfGUIMenuItem::kMouseHover && fMenuItems[ idx ].fSubMenu != nil )
		{
			// Open new submenu
			const hsBounds3 &bnds = ctrl->GetBounds();
			fMenuItems[ idx ].fSubMenu->Show( bnds.GetMaxs().fX, bnds.GetMins().fY );
			fSubMenuOpen = idx;
		}
	}
	else
	{
		if( fMenuItems[ idx ].fHandler != nil )
			fMenuItems[ idx ].fHandler->DoSomething( ctrl );
		
		// If item isn't a sub-menu, close this menu. Else add to list of menus to close
		// once the smallest submenu goes away
		if( fMenuItems[ idx ].fSubMenu == nil )
		{
			// Basically, we want to hide ourselves and as many up in the chain of command as
			// can be hidden
			pfGUIPopUpMenu *menu = this;
			while( menu != nil && !menu->HasFlag( kStayOpenAfterClick ) )
			{
				menu->Hide();
				menu = pfGUIPopUpMenu::ConvertNoRef( menu->fParent );
			}
		}
		else
		{
			// Show relative to the corner of our current item
			const hsBounds3 &bnds = ctrl->GetBounds();
			fMenuItems[ idx ].fSubMenu->Show( bnds.GetMaxs().fX, bnds.GetMins().fY );
			fSubMenuOpen = idx;
		}
	}
}

//// IBuildMenu //////////////////////////////////////////////////////////////
//	Given the list of menu items, builds our set of dynamic buttons

hsBool	pfGUIPopUpMenu::IBuildMenu( void )
{
	int		i;


	if( fWaitingForSkin && fSkin == nil )
		return false;		// Still waiting to get our skin before building

	pfGUIColorScheme *scheme = TRACKED_NEW pfGUIColorScheme();
	scheme->fForeColor.Set( 0, 0, 0, 1 );
	scheme->fBackColor.Set( 1, 1, 1, 1 );

	// If we don't have origin points, get them from translating our anchor
	if( fOriginX == -1 || fOriginY == -1 && fOriginAnchor != nil )
	{
		hsPoint3 scrnPt;
		const plDrawInterface *di = fOriginAnchor->GetDrawInterface();
		if( di != nil )
		{
			scrnPt = di->GetLocalBounds().GetCenter();
			scrnPt = fOriginAnchor->GetLocalToWorld() * scrnPt;
		}
		else
			scrnPt = fOriginAnchor->GetLocalToWorld().GetTranslate();
		if( fOriginContext != nil )
			scrnPt = fOriginContext->WorldToScreenPoint( scrnPt );
		else
			scrnPt = WorldToScreenPoint( scrnPt );

		if( fOriginX == -1 )
			fOriginX = scrnPt.fX;
		if( fOriginY == -1 )
			fOriginY = scrnPt.fY;
	}

	float x = fOriginX, y = fOriginY;
	float width = 0.f, height = 0.f;
	float topMargin = ( fSkin != nil ) ? fSkin->GetBorderMargin() : 0.f;
	
	// First step: loop through and calculate the size of our menu
	// The PROBLEM is that we can't do that unless we have a friggin surface on
	// which to calculate the text extents! So sadly, we're going to have to create
	// a whole new DTMap and use it to calculate some stuff
	plDynamicTextMap *scratch = TRACKED_NEW plDynamicTextMap( 8, 8, false );
	scratch->SetFont( scheme->fFontFace, scheme->fFontSize, scheme->fFontFlags, true );
	for( i = 0; i < fMenuItems.GetCount(); i++ )
	{
		UInt16	thisW, thisH;
		thisW = scratch->CalcStringWidth( fMenuItems[ i ].fName.c_str(), &thisH );
		if( fMenuItems[ i ].fSubMenu != nil )
		{
			if( fSkin != nil )
				thisW += 4 + ( fSkin->GetElement( pfGUISkin::kSubMenuArrow ).fWidth << 1 );
			else
				thisW += scratch->CalcStringWidth( " >>", nil );
		}
		thisH += 2;	// Give us at least one pixel on each side

		int margin = fMargin;
		if( fSkin != nil )
			margin = fSkin->GetBorderMargin() << 1;

		if( width < thisW + margin )
			width = (float)(thisW + margin);

		if( fSkin != nil )
			margin = fSkin->GetItemMargin() << 1;

		if( height < thisH + margin )
			height = (float)(thisH + margin);
	}
	delete scratch;

	width += 4; // give us a little space, just in case

	UInt32 scrnWidth, scrnHeight;
	// A cheat here, I know, but I'm lazy
	plDebugText::Instance().GetScreenSize( &scrnWidth, &scrnHeight );

	// Use the same base res calc that dtMaps use
	if( !HasFlag( kScaleWithResolution ) )
	{
		// Just use what we were passed in
	}
	else
	{
		// Scale with the resolution so that we take up the same % of screen space no matter what resolution
		// Assume a base "resolution" of 1024xX, where X is such that the ratio "1024/X = scrnWidth/scrnHt" holds
		const int kBaseScaleRes = 1024;
		scrnHeight = ( scrnHeight * kBaseScaleRes ) / scrnWidth;
		scrnWidth = kBaseScaleRes;
	}

	width /= (float)scrnWidth;
	height /= (float)scrnHeight;
	topMargin /= (float)scrnHeight;

	switch( fAlignment )
	{
		case kAlignUpLeft: x -= width; y -= height * fMenuItems.GetCount(); break;
		case kAlignUpRight: y -= height * fMenuItems.GetCount(); break;
		case kAlignDownLeft: x -= width; break;
		case kAlignDownRight: break;
	}

	if( y + height * fMenuItems.GetCount() > 1.f )
	{
		// Make sure we don't go off the bottom
		y = 1.f - height * fMenuItems.GetCount();
	}
	// And the top (takes precedence)
	if( y < 0.f )
		y = 0.f;

	// Control positions are in the lower left corner, so increment Y by 1 control height first
	y += height;// + topMargin;

	hsTArray<pfGUIPopUpMenu *>	buildList;

	for( i = 0; i < fMenuItems.GetCount(); i++ )
	{
		hsGMaterial *mat = ICreateDynMaterial();

		float thisMargin = ( i == 0 || i == fMenuItems.GetCount() - 1 ) ? topMargin : 0.f;
		float thisOffset = ( i == fMenuItems.GetCount() - 1 ) ? topMargin : 0.f;

		pfGUIMenuItem *button = pfGUIMenuItem::ConvertNoRef( pfGUICtrlGenerator::Instance().CreateRectButton( this, fMenuItems[ i ].fName.c_str(), x, y + thisOffset, width, height + thisMargin, mat, true ) );
		if( button != nil )
		{
			button->SetColorScheme( scheme );
			button->SetName( fMenuItems[ i ].fName.c_str() );
			button->SetHandler( TRACKED_NEW pfGUIMenuItemProc( this, i ) );
			// make the tag ID the position in the menu list
			button->SetTagID(i);
			button->SetDynTextMap( mat->GetLayer( 0 ), plDynamicTextMap::ConvertNoRef( mat->GetLayer( 0 )->GetTexture() ) );
			button->SetFlag( pfGUIMenuItem::kReportHovers );
			button->SetSkin( fSkin, ( i == 0 ) ? pfGUIMenuItem::kTop : ( i == fMenuItems.GetCount() - 1 ) ? pfGUIMenuItem::kBottom : pfGUIMenuItem::kMiddle );
			if( fMenuItems[ i ].fSubMenu != nil )
			{
				button->SetFlag( pfGUIMenuItem::kDrawSubMenuArrow );
				buildList.Append( pfGUIPopUpMenu::ConvertNoRef( fMenuItems[ i ].fSubMenu ) );
			}
		}

		// Tiny bit of overlap to prevent gaps
		fMenuItems[ i ].fYOffsetToNext = height + thisOffset;
		y += height + thisOffset;// - ( 1.f / kBaseScaleResY );
	}

	fNeedsRebuilding = false;

#if 0
	// Finally, go down our list of submenus and rebuild them, since they'll need to be rebuilt soon anyway,
	// and at least this way it's all in one pass

	// Also, we need to bump the tag ID used, such as adding parent menuItem TagID * 100.. or something

	// Disabled because right now we can't move menus, which is required for this to work
	for( i = 0; i < buildList.GetCount(); i++ )
		buildList[ i ]->IBuildMenu();
#endif

	return true;
}

//// ITearDownMenu ///////////////////////////////////////////////////////////
//	Destroys all of our dynamic controls representing the menu

void	pfGUIPopUpMenu::ITearDownMenu( void )
{
	int		i;


	for( i = 0; i < fControls.GetCount(); i++ )
	{
		if( fControls[ i ] != nil )
		{
			// It's not enough to release the key, we have to have the sceneNode release the key, too.
			// Easy enough to do by just setting it's sn to nil
			if( fControls[ i ]->GetTarget() != nil )
				fControls[ i ]->GetTarget()->SetSceneNode( nil );

			// Now release it from us
			GetKey()->Release( fControls[ i ]->GetKey() );
		}
	}

	fNeedsRebuilding = true;
}

//// HandleMouseEvent ////////////////////////////////////////////////////////

hsBool		pfGUIPopUpMenu::HandleMouseEvent( pfGameGUIMgr::EventType event, hsScalar mouseX, hsScalar mouseY,
												UInt8 modifiers )
{
	hsBool r = pfGUIDialogMod::HandleMouseEvent( event, mouseX, mouseY, modifiers );
	if( r == false && event == pfGameGUIMgr::kMouseUp )
	{
		// We don't want to be active anymore!
		if( !HasFlag( kStayOpenAfterClick ) )
		{
			Hide();

			// Now we pass the click to our parent. Why? Because it's possible that someone above us
			// will either a) also want to hide (cancel the entire menu selection) or b) select
			// another option
			if( fParent != nil )
				return fParent->HandleMouseEvent( event, mouseX, mouseY, modifiers );
		}
	}

	return ( fParent != nil ) ? r : ( HasFlag( kModalOutsideMenus ) || ( fSubMenuOpen != -1 ) );
}

//// ClearItems //////////////////////////////////////////////////////////////
//	Clears the list of template items

void	pfGUIPopUpMenu::ClearItems( void )
{
	int		i;


	for( i = 0; i < fMenuItems.GetCount(); i++ )
	{
		if( fMenuItems[ i ].fHandler != nil )
		{
			if( fMenuItems[ i ].fHandler->DecRef() )
				delete fMenuItems[ i ].fHandler;
		}
	}

	fMenuItems.Reset();

	fNeedsRebuilding = true;
}

//// AddItem /////////////////////////////////////////////////////////////////
//	Append a new item to the list of things to build the menu from 

void	pfGUIPopUpMenu::AddItem( const char *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu )
{
	wchar_t *wName = hsStringToWString(name);
	AddItem(wName,handler,subMenu);
	delete [] wName;
}

void	pfGUIPopUpMenu::AddItem( const wchar_t *name, pfGUICtrlProcObject *handler, pfGUIPopUpMenu *subMenu )
{
	pfMenuItem	newItem;


	newItem.fName = name;
	newItem.fHandler = handler;
	if( newItem.fHandler != nil )
		newItem.fHandler->IncRef();
	newItem.fSubMenu = subMenu;

	if( subMenu != nil )
		subMenu->fParent = this;

	fMenuItems.Append( newItem );

	fNeedsRebuilding = true;
}

//// ICreateDynMaterial //////////////////////////////////////////////////////
//	Creates the hsGMaterial tree for a single layer with a plDynamicTextMap.

hsGMaterial	*pfGUIPopUpMenu::ICreateDynMaterial( void )
{
	hsColorRGBA		black, white;

	
	// Create the new dynTextMap
	plDynamicTextMap	*textMap = TRACKED_NEW plDynamicTextMap();
	fKeyGen->CreateKey( textMap );

	// Create the material
	hsGMaterial *material = TRACKED_NEW hsGMaterial;
	fKeyGen->CreateKey( material );

	// Create the layer and attach
	plLayer *lay = material->MakeBaseLayer();
	white.Set( 1.f,1.f,1.f,1.f );
	black.Set( 0.f,0.f,0.f,1.f );

	lay->SetRuntimeColor( black );
	lay->SetPreshadeColor( black );
	lay->SetAmbientColor( white );
	lay->SetClampFlags( hsGMatState::kClampTexture );

	// Do sendRef here, since we're going to need it set pretty darned quick
	hsgResMgr::ResMgr()->SendRef( textMap->GetKey(), TRACKED_NEW plLayRefMsg( lay->GetKey(), plRefMsg::kOnCreate, 0, plLayRefMsg::kTexture ), plRefFlags::kActiveRef );

	return material;

}

//// Build ///////////////////////////////////////////////////////////////////
//	Constructs a shiny new pop-up menu at runtime, complete with trimmings

#include "../plJPEG/plJPEG.h"

pfGUIPopUpMenu	*pfGUIPopUpMenu::Build( const char *name, pfGUIDialogMod *parent, hsScalar x, hsScalar y, const plLocation &destLoc )
{
	float			fovX, fovY;
	

	// Create the menu and give it a key gen
	pfGUIPopUpMenu	*menu = TRACKED_NEW pfGUIPopUpMenu();
	menu->fKeyGen = TRACKED_NEW pfPopUpKeyGenerator( name, destLoc );
	menu->fKeyGen->CreateKey( menu );

	menu->fOriginX = x;
	menu->fOriginY = y;

	// By default, share the same skin as the parent
	if( parent != nil && ( (pfGUIPopUpMenu *)parent )->fSkin != nil )
	{
		menu->fWaitingForSkin = true;
		hsgResMgr::ResMgr()->SendRef( ( (pfGUIPopUpMenu *)parent )->fSkin->GetKey(), TRACKED_NEW plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, -1, pfGUIPopUpMenu::kRefSkin ), plRefFlags::kActiveRef );
	}

	// HACK for now: create us a temp skin to use
/*	static pfGUISkin *skin = nil;
	if( skin == nil )
	{
		plLocation loc;
		loc.Set( 0x1425 );
		plKey skinKey = hsgResMgr::ResMgr()->FindKey( plUoid( loc, pfGUISkin::Index(), "GUISkin01_GUISkin" ) );
		menu->fWaitingForSkin = true;
		hsgResMgr::ResMgr()->AddViaNotify( skinKey, TRACKED_NEW plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, -1, pfGUIPopUpMenu::kRefSkin ), plRefFlags::kActiveRef );
	}
*/

	// Create the rendermod
	plPostEffectMod	*renderMod = TRACKED_NEW plPostEffectMod;
	menu->fKeyGen->CreateKey( renderMod );

	renderMod->SetHither( 0.5f );
	renderMod->SetYon( 200.f );

	float scrnWidth = 20.f;

	// fovX should be such that scrnWidth is the projected width at z=100
	fovX = atan( scrnWidth / ( 2.f * 100.f ) ) * 2.f;
	fovY = fovX;// * 3.f / 4.f;

	renderMod->SetFovX( fovX * 180.f / hsScalarPI );
	renderMod->SetFovY( fovY * 180.f / hsScalarPI );

	// Create the sceneNode to go with it
	menu->fParentNode= TRACKED_NEW plSceneNode;
	menu->fKeyGen->CreateKey( menu->fParentNode );
//	menu->fParentNode->GetKey()->RefObject();
	hsgResMgr::ResMgr()->SendRef( menu->fParentNode->GetKey(), TRACKED_NEW plGenRefMsg( menu->GetKey(), plRefMsg::kOnCreate, 0, kRefParentNode ), plRefFlags::kActiveRef );		

	hsgResMgr::ResMgr()->AddViaNotify( menu->fParentNode->GetKey(), TRACKED_NEW plGenRefMsg( renderMod->GetKey(), plRefMsg::kOnCreate, 0, plPostEffectMod::kNodeRef ), plRefFlags::kPassiveRef );		

	menu->SetRenderMod( renderMod );
	menu->SetName( name );

	// Create the dummy scene object to hold the menu
	plSceneObject	*newObj = TRACKED_NEW plSceneObject;
	menu->fKeyGen->CreateKey( newObj );

	// *#&$(*@&#$ need a coordIface...
	plCoordinateInterface *newCI = TRACKED_NEW plCoordinateInterface;
	menu->fKeyGen->CreateKey( newCI );

	hsMatrix44 l2w, w2l;
	l2w.Reset();
	l2w.GetInverse( &w2l );

	// Using SendRef here because AddViaNotify will queue the messages up, which doesn't do us any good
	// if we need these refs right away
	hsgResMgr::ResMgr()->SendRef( newCI->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface ), plRefFlags::kActiveRef );		
	hsgResMgr::ResMgr()->SendRef( renderMod->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		
	newObj->SetSceneNode( menu->fParentNode->GetKey() );
	newObj->SetTransform( l2w, w2l );

	hsgResMgr::ResMgr()->SendRef( menu->GetKey(), TRACKED_NEW plObjRefMsg( newObj->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kModifier ), plRefFlags::kActiveRef );		

	// Add the menu to the GUI mgr
	plGenRefMsg *refMsg = TRACKED_NEW plGenRefMsg( pfGameGUIMgr::GetInstance()->GetKey(), 
											plRefMsg::kOnCreate, 0, pfGameGUIMgr::kDlgModRef );
	hsgResMgr::ResMgr()->AddViaNotify( menu->GetKey(), refMsg, plRefFlags::kActiveRef );		

	menu->ISeekToOrigin();

	return menu;
}

//// SetSkin /////////////////////////////////////////////////////////////////

void	pfGUIPopUpMenu::SetSkin( pfGUISkin *skin )
{
	// Just a function wrapper for SendRef
	if( fSkin != nil )
		GetKey()->Release( fSkin->GetKey() );

	if( skin != nil )
	{
		hsgResMgr::ResMgr()->SendRef( skin->GetKey(), TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefSkin ), plRefFlags::kActiveRef );
		fWaitingForSkin = true;
	}
	else
		fWaitingForSkin = false;
}


//////////////////////////////////////////////////////////////////////////////
//// pfGUISkin Implementation ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfGUISkin::pfGUISkin()
{
	fTexture = nil;
	memset( fElements, 0, sizeof( pfSRect ) * kNumElements );
}

pfGUISkin::pfGUISkin( plMipmap *texture )
{
	fTexture = texture;
	if( fTexture != nil )
	{
		hsAssert( fTexture->GetKey() != nil, "Creating a GUI skin via a mipmap with no key!" );
		fTexture->GetKey()->RefObject();
	}
	memset( fElements, 0, sizeof( pfSRect ) * kNumElements );
}

pfGUISkin::~pfGUISkin()
{
	SetTexture( nil );
}

void	pfGUISkin::SetTexture( plMipmap *tex )
{
	if( fTexture != nil && fTexture->GetKey() != nil )
		fTexture->GetKey()->UnRefObject();

	fTexture = tex;
	if( fTexture != nil )
	{
		hsAssert( fTexture->GetKey() != nil, "Creating a GUI skin via a mipmap with no key!" );
		fTexture->GetKey()->RefObject();
	}
}

void	pfGUISkin::SetElement( UInt32 idx, UInt16 x, UInt16 y, UInt16 w, UInt16 h )
{
	fElements[ idx ].fX = x;
	fElements[ idx ].fY = y;
	fElements[ idx ].fWidth = w;
	fElements[ idx ].fHeight = h;
}

void	pfGUISkin::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );

	s->ReadSwap( &fItemMargin );
	s->ReadSwap( &fBorderMargin );

	UInt32 i, count;
	s->ReadSwap( &count );

	for( i = 0; i < count; i++ )
		fElements[ i ].Read( s );

	for( ; i < kNumElements; i++ )
		fElements[ i ].Empty();

	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefMipmap ), plRefFlags::kActiveRef );
}

void	pfGUISkin::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write( s, mgr );

	s->WriteSwap( fItemMargin );
	s->WriteSwap( fBorderMargin );

	UInt32 i = kNumElements;
	s->WriteSwap( i );

	for( i = 0; i < kNumElements; i++ )
		fElements[ i ].Write( s );

	mgr->WriteKey( s, fTexture );
}

hsBool	pfGUISkin::MsgReceive( plMessage *msg )
{
	plGenRefMsg *ref = plGenRefMsg::ConvertNoRef( msg );
	if( ref != nil )
	{
		if( ref->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			fTexture = plMipmap::ConvertNoRef( ref->GetRef() );
		else
			fTexture = nil;

		return true;
	}

	return hsKeyedObject::MsgReceive( msg );
}

void	pfGUISkin::pfSRect::Read( hsStream *s )
{
	s->ReadSwap( &fX );
	s->ReadSwap( &fY );
	s->ReadSwap( &fWidth );
	s->ReadSwap( &fHeight );
}

void	pfGUISkin::pfSRect::Write( hsStream *s )
{
	s->WriteSwap( fX );
	s->WriteSwap( fY );
	s->WriteSwap( fWidth );
	s->WriteSwap( fHeight );
}
