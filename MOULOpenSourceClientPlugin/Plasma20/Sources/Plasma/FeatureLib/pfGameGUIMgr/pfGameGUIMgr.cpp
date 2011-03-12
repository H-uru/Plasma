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
//	pfGameGUIMgr															//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	11.13.2001 mcn	- Created												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include "hsTimer.h"
#include "hsTypes.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"
#include "pfGUIDialogHandlers.h"
#include "pfGUIDialogNotifyProc.h"
#include "pfGUIControlMod.h"
#include "pfGUIPopUpMenu.h"

#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../plMessage/plInputIfaceMgrMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "../plInputCore/plInputInterface.h"
#include "../plInputCore/plInputDevice.h"
#include "../plInputCore/plInputInterfaceMgr.h"
#include "../pnInputCore/plKeyMap.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnSceneObject/plSceneObject.h"	// So we can get the target sceneNode of a dialog
#include "../plMessage/plConsoleMsg.h"
#include "plgDispatch.h"

#include "../plResMgr/plKeyFinder.h"

#include "pfGUITagDefs.h"


//////////////////////////////////////////////////////////////////////////////
//// pfGameUIInputInterface Definition ///////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

class pfGameUIInputInterface : public plInputInterface
{
	protected:
		pfGameGUIMgr	* const fGUIManager;

		UInt8	fModifiers;
		UInt8	fButtonState;
		hsBool	fHaveInterestingCursor;
		UInt32	fCurrentCursor;

		virtual hsBool	IHandleCtrlCmd( plCtrlCmd *cmd );
		virtual hsBool	IControlCodeEnabled( ControlEventCode code );

	public:

		pfGameUIInputInterface( pfGameGUIMgr * const mgr );

		virtual UInt32	GetPriorityLevel( void ) const { return kGUISystemPriority; }
		virtual hsBool	InterpretInputEvent( plInputEventMsg *pMsg );
		virtual UInt32	GetCurrentCursorID( void ) const;
		virtual hsScalar GetCurrentCursorOpacity( void ) const;
		virtual hsBool	HasInterestingCursorID( void ) const { return fHaveInterestingCursor; }
		virtual hsBool	SwitchInterpretOrder( void ) const { return true; }

		virtual void	RestoreDefaultKeyMappings( void )
		{
			if( fControlMap != nil )
			{
				fControlMap->UnmapAllBindings();
				fControlMap->BindKey( KEY_BACKSPACE, B_CONTROL_EXIT_GUI_MODE, plKeyMap::kFirstAlways );
				fControlMap->BindKey( KEY_ESCAPE, B_CONTROL_EXIT_GUI_MODE, plKeyMap::kSecondAlways );
			}
		}
};

//////////////////////////////////////////////////////////////////////////////
//// pfGameGUIMgr Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfGameGUIMgr	*pfGameGUIMgr::fInstance = nil;


//// Constructor & Destructor ////////////////////////////////////////////////

pfGameGUIMgr::pfGameGUIMgr()
{
	fActivated = false;
	fInputCtlIndex = 0;
	fActiveDialogs = nil;

	fInputConfig = nil;

	fInstance = this;
	
	fDefaultCursor = plInputInterface::kCursorUp;
	fCursorOpacity = 1.f;
	fAspectRatio = 0;
}

pfGameGUIMgr::~pfGameGUIMgr()
{
	int		i;

	// the GUIMgr is dead!
	fInstance = nil;

	for( i = 0; i < fDialogs.GetCount(); i++ )
		UnloadDialog( fDialogs[ i ] );

	for( i = 0; i < fDialogToSetKeyOf.GetCount(); i++ )
		delete fDialogToSetKeyOf[i];

	if( fActivated )
		IActivateGUI( false );

	delete fInputConfig;
}


//// Init ////////////////////////////////////////////////////////////////////

hsBool	pfGameGUIMgr::Init( void )
{
	return true;
}

//// Draw ////////////////////////////////////////////////////////////////////

void	pfGameGUIMgr::Draw( plPipeline *p )
{
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGameGUIMgr::MsgReceive( plMessage* pMsg )
{
	pfGameGUIMsg	*guiMsg = pfGameGUIMsg::ConvertNoRef( pMsg );
	if( guiMsg != nil )
	{
		if( guiMsg->GetCommand() == pfGameGUIMsg::kLoadDialog )
			LoadDialog( guiMsg->GetString(), nil, guiMsg->GetAge() );
		else if( guiMsg->GetCommand() == pfGameGUIMsg::kShowDialog )
			IShowDialog( guiMsg->GetString() );
		else if( guiMsg->GetCommand() == pfGameGUIMsg::kHideDialog )
			IHideDialog( guiMsg->GetString() );

		return true;
	}

	plGenRefMsg	*refMsg = plGenRefMsg::ConvertNoRef( pMsg );
	if( refMsg != nil )
	{
		if( refMsg->fType == kDlgModRef )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest ) )
			{
				IAddDlgToList( refMsg->GetRef() );
			}
			else if( refMsg->GetContext() & plRefMsg::kOnReplace )
			{
				IRemoveDlgFromList( refMsg->GetOldRef() );
				IAddDlgToList( refMsg->GetRef() );
			}
			else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
			{
				IRemoveDlgFromList( refMsg->GetRef() );
			}
		}
		return true;
	}

	return hsKeyedObject::MsgReceive( pMsg );
}

//// IAddDlgToList ///////////////////////////////////////////////////////////

void	pfGameGUIMgr::IAddDlgToList( hsKeyedObject *obj )
{
	int		i;


	if( fDialogs.Find( (pfGUIDialogMod *)obj ) == fDialogs.kMissingIndex )
	{
		pfGUIDialogMod	*mod = pfGUIDialogMod::ConvertNoRef( obj );
		if( mod != nil )
		{
			mod->UpdateAspectRatio();	// adding a new dialog, make sure the correct aspect ratio is set
			fDialogs.Append( mod );


			// check to see if it is the dialog we are waiting for to be loaded
			for ( i=0 ; i<fDialogToSetKeyOf.Count() ; i++ )
			{
				if ( hsStrEQ(fDialogToSetKeyOf[i]->GetName(), mod->GetName()) )
				{
					SetDialogToNotify(mod,fDialogToSetKeyOf[i]->GetKey());
					// now remove this entry... we did it
					delete fDialogToSetKeyOf[i];
					fDialogToSetKeyOf.Remove(i);
					// that's all the damage we can do for now...
					break;
				}
			}
		}
	}

/*	// It's now officially "loaded"; take it off the pending list
	for( i = 0; i < fDlgsPendingLoad.GetCount(); i++ )
	{
		if( stricmp( fDlgsPendingLoad[ i ]->GetName(), ( (pfGUIDialogMod *)obj )->GetName() ) == 0 )
		{
			// Here it is
			delete fDlgsPendingLoad[ i ];
			fDlgsPendingLoad.Remove( i );
			break;
		}
	}
*/
}

//// IRemoveDlgFromList //////////////////////////////////////////////////////

void	pfGameGUIMgr::IRemoveDlgFromList( hsKeyedObject *obj )
{
	int idx = fDialogs.Find( (pfGUIDialogMod *)obj );
	if( idx != fDialogs.kMissingIndex )
	{
		pfGUIDialogMod	*mod = pfGUIDialogMod::ConvertNoRef( obj );
		hsAssert( mod != nil, "Non-dialog sent to gameGUIMgr::IRemoveDlg()" );

		if( mod != nil )
		{
			if( mod->IsEnabled() )
			{
				mod->SetEnabled( false );

				mod->Unlink();
				if( fActiveDialogs == nil )
					IActivateGUI( false );
			}

			// Needed?
//				GetKey()->Release( mod->GetKey() );
			fDialogs.Remove( idx );
		}
	}

	// It's now officially "unloaded"; take it off the pending list
/*	int i;
	for( i = 0; i < fDlgsPendingUnload.GetCount(); i++ )
	{
		if( stricmp( fDlgsPendingUnload[ i ]->GetName(), ( (pfGUIDialogMod *)obj )->GetName() ) == 0 )
		{
			// Here it is
			delete fDlgsPendingUnload[ i ];
			fDlgsPendingUnload.Remove( i );
			break;
		}
	}
*/
}

//// LoadDialog //////////////////////////////////////////////////////////////

void	pfGameGUIMgr::LoadDialog( const char *name, plKey recvrKey, const char *ageName )
{
	// see if they want to set the receiver key once the dialog is loaded
	if ( recvrKey != nil )
	{
		// first see if we are loading a dialog that is already being loaded
		bool alreadyLoaded = false;
		int i;
		for ( i=0 ; i<fDialogToSetKeyOf.Count() ; i++ )
		{
			if ( hsStrEQ(fDialogToSetKeyOf[i]->GetName(), name) )
			{
				alreadyLoaded = true;
				break;
			}
		}
		if (!alreadyLoaded)
		{
			pfDialogNameSetKey* pDNSK = TRACKED_NEW pfDialogNameSetKey(name,recvrKey);
			fDialogToSetKeyOf.Append(pDNSK);
		}
	}

	hsStatusMessageF("\nLoading Dialog %s %s ... %f\n", name, ( ageName != nil ) ? ageName : "GUI", hsTimer::GetSeconds() );

	plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

	plClientMsg *msg = TRACKED_NEW plClientMsg( plClientMsg::kLoadRoomHold );
	msg->AddReceiver( clientKey );
	msg->AddRoomLoc(plKeyFinder::Instance().FindLocation(ageName ? ageName : "GUI", name));
	msg->Send();

	// Now add this dialog to a list of pending loads (will remove it once it's fully loaded)
//	fDlgsPendingLoad.Append( TRACKED_NEW pfDialogNameSetKey( name, nil ) );
}

//// IShowDialog /////////////////////////////////////////////////////////////

void	pfGameGUIMgr::IShowDialog( const char *name )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
		{
			ShowDialog( fDialogs[ i ] );
			fDialogs[i]->RefreshAllControls();
			break;
		}
	}
}

//// IHideDialog /////////////////////////////////////////////////////////////

void	pfGameGUIMgr::IHideDialog( const char *name )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
		{
			HideDialog( fDialogs[ i ] );
			break;
		}
	}
}

//// ShowDialog //////////////////////////////////////////////////////////////

void	pfGameGUIMgr::ShowDialog( pfGUIDialogMod *dlg, bool resetClickables /* = true */ )
{
	if ( resetClickables )
		plInputInterfaceMgr::GetInstance()->ResetClickableState();
	if( !dlg->IsEnabled() )
	{
		dlg->SetEnabled( true );

		// Add to active list
		if( fActiveDialogs == nil )
			IActivateGUI( true );
		
		dlg->LinkToList( &fActiveDialogs );

		return;
	}
}

//// HideDialog //////////////////////////////////////////////////////////////

void	pfGameGUIMgr::HideDialog( pfGUIDialogMod *dlg )
{
	if( dlg->IsEnabled() )
	{
		dlg->SetEnabled( false );

		dlg->Unlink();
		if( fActiveDialogs == nil )
			IActivateGUI( false );
	}
}

//// UnloadDialog ////////////////////////////////////////////////////////////
//	Destroy the dialog and all the things associated with it. Fun fun.

void	pfGameGUIMgr::UnloadDialog( const char *name )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
		{
			UnloadDialog( fDialogs[ i ] );
			break;
		}
	}
}

void	pfGameGUIMgr::UnloadDialog( pfGUIDialogMod *dlg )
{
//	IRemoveDlgFromList( dlg );

	// Add the name to our list of dialogs pending unload
//	fDlgsPendingUnload.Append( TRACKED_NEW pfDialogNameSetKey( dlg->GetName(), nil ) );

	plKey		sceneNodeKey = dlg->GetSceneNodeKey();
	if( sceneNodeKey == nil )
	{
		hsStatusMessageF( "Warning: Unable to grab sceneNodeKey to unload dialog %s; searching for it...", dlg->GetName() );
		sceneNodeKey = plKeyFinder::Instance().FindSceneNodeKey( dlg->GetKey()->GetUoid().GetLocation() );
	}

//	if( dlg->GetTarget() )
	if( sceneNodeKey != nil )
	{
		plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );

		plClientMsg *msg = TRACKED_NEW plClientMsg( plClientMsg::kUnloadRoom );
		msg->AddReceiver( clientKey );
//		msg->SetProgressBarSuppression( true );
		msg->AddRoomLoc(sceneNodeKey->GetUoid().GetLocation());
		msg->Send();
	}
//	GetKey()->Release( dlg->GetKey() );
}

//// IsDialogLoaded ////// see if the dialog is in our list of loaded dialogs

hsBool	pfGameGUIMgr::IsDialogLoaded( const char *name )
{
	// search through all the dialogs we've loaded
	int		i;
	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
		{
			// found 'em
			return true;
		}
	}
	// nota
	return false;
}

pfGUIPopUpMenu	*pfGameGUIMgr::FindPopUpMenu( const char *name )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		pfGUIPopUpMenu *menu = pfGUIPopUpMenu::ConvertNoRef( fDialogs[ i ] );
		if( menu != nil && stricmp( menu->GetName(), name ) == 0 )
			return menu;
	}

	return nil;
}

std::vector<plPostEffectMod*> pfGameGUIMgr::GetDlgRenderMods( void ) const
{
	std::vector<plPostEffectMod*> retVal;
	pfGUIDialogMod* curDialog = fActiveDialogs;
	while (curDialog)
	{
		retVal.push_back(curDialog->GetRenderMod());
		curDialog = curDialog->GetNext();
	}
	return retVal;
}

///// SetDialogToNotify     - based on name
// This will Set the handler to a Notify Handler that will send a GUINotifyMsg to the receiver
//
void pfGameGUIMgr::SetDialogToNotify(const char *name, plKey recvrKey)
{
	int		i;
	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
		{
			SetDialogToNotify( fDialogs[ i ], recvrKey );
			break;
		}
	}
}

///// SetDialogToNotify     - pfGUIDialogMod*
// This will Set the handler to a Notify Handler that will send a GUINotifyMsg to the receiver
//
void pfGameGUIMgr::SetDialogToNotify(pfGUIDialogMod *dlg, plKey recvrKey)
{
	pfGUIDialogNotifyProc* handler = TRACKED_NEW pfGUIDialogNotifyProc(recvrKey);
	dlg->SetHandler(handler);
	handler->OnInit();
}


//// IActivateGUI ////////////////////////////////////////////////////////////
//	"Activates" the GUI manager. This means enabling the drawing of GUI
//	elements on the screen as well as rerouting input to us.

void	pfGameGUIMgr::IActivateGUI( hsBool activate )
{
	if( fActivated == activate )
		return;

	if( activate )
	{
		fInputConfig = TRACKED_NEW pfGameUIInputInterface( this );
		plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kAddInterface );
		msg->SetIFace( fInputConfig );
		plgDispatch::MsgSend( msg );
	}
	else
	{
		plInputIfaceMgrMsg *msg = TRACKED_NEW plInputIfaceMgrMsg( plInputIfaceMgrMsg::kRemoveInterface );
		msg->SetIFace( fInputConfig );
		plgDispatch::MsgSend( msg );

		hsRefCnt_SafeUnRef( fInputConfig );
		fInputConfig = nil;
	}

	fActivated = activate;
}

//// IHandleMouse ////////////////////////////////////////////////////////////
//	Distributes mouse events to the dialogs currently active

hsBool	pfGameGUIMgr::IHandleMouse( EventType event, hsScalar mouseX, hsScalar mouseY, UInt8 modifiers, UInt32 *desiredCursor ) 
{
	pfGUIDialogMod	*dlg;


	// Update interesting things first, no matter what, for ALL dialogs
	hsBool	modalPresent = false;
	for( dlg = fActiveDialogs; dlg != nil; dlg = dlg->GetNext() )
	{
		dlg->UpdateInterestingThings( mouseX, mouseY, modifiers, modalPresent );
		if (dlg->HasFlag( pfGUIDialogMod::kModal ))
			modalPresent = true;
	}

	for( dlg = fActiveDialogs; dlg != nil; dlg = dlg->GetNext() )
	{
		if( dlg->HandleMouseEvent( event, mouseX, mouseY, modifiers ) ||
			( dlg->HasFlag( pfGUIDialogMod::kModal ) && event != pfGameGUIMgr::kMouseUp ) )
		{
			// If this dialog handled it, also get the cursor it wants
			*desiredCursor = dlg->GetDesiredCursor();
			return true;
		}
	}

	return false;
}

//// IHandleKeyEvt ///////////////////////////////////////////////////////////
//	Distributes mouse events to the dialogs currently active

hsBool	pfGameGUIMgr::IHandleKeyEvt( EventType event, plKeyDef key, UInt8 modifiers ) 
{
	pfGUIDialogMod	*dlg;


	for( dlg = fActiveDialogs; dlg != nil; dlg = dlg->GetNext() )
	{
		if( dlg->HandleKeyEvent( event, key, modifiers ) )
			return true;
	}

	return false;
}

//// IHandleKeyPress /////////////////////////////////////////////////////////
//	Like IHandleKeyPress, but takes in a char for distributing actual 
//	characters typed.

hsBool	pfGameGUIMgr::IHandleKeyPress( char key, UInt8 modifiers ) 
{
	pfGUIDialogMod	*dlg;


	for( dlg = fActiveDialogs; dlg != nil; dlg = dlg->GetNext() )
	{
		if( dlg->HandleKeyPress( key, modifiers ) )
			return true;
	}

	return false;
}

//// IModalBlocking //////////////////////////////////////////////////////////
//	Looks at the chain of active dialogs and determines if there's any modals
//	blocking input. Returns true if so.

hsBool	pfGameGUIMgr::IModalBlocking( void )
{
	return ( IGetTopModal() != nil ) ? true : false;
}

//// IGetTopModal ////////////////////////////////////////////////////////////
//	Returns the topmost (visible) modal dialog, nil if none.

pfGUIDialogMod	*pfGameGUIMgr::IGetTopModal( void ) const
{
	pfGUIDialogMod	*dlg;


	for( dlg = fActiveDialogs; dlg != nil; dlg = dlg->GetNext() )
	{
		if( dlg->HasFlag( pfGUIDialogMod::kModal ) )
			return dlg;
	}

	return nil;
}


//////////////////////////////////////////////////////////////////////////////
//// Control Config Class ////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

pfGameUIInputInterface::pfGameUIInputInterface( pfGameGUIMgr * const mgr ) : plInputInterface(), fGUIManager( mgr )
{
	fModifiers = pfGameGUIMgr::kNoModifiers;
	fButtonState = 0;
	fHaveInterestingCursor = false;
	SetEnabled( true );			// Always enabled
	fCurrentCursor = kCursorUp;

	// Add our control codes to our control map. Do NOT add the key bindings yet.
	// Note: HERE is where you specify the actions for each command, i.e. net propagate and so forth.
	// This part basically declares us master of the bindings for these commands.
	
	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!

	fControlMap->AddCode( B_CONTROL_EXIT_GUI_MODE, kControlFlagNormal | kControlFlagNoRepeat );

	// IF YOU ARE LOOKING TO CHANGE THE DEFAULT KEY BINDINGS, DO NOT LOOK HERE. GO TO
	// RestoreDefaultKeyMappings()!!!!
}

hsBool	pfGameUIInputInterface::IControlCodeEnabled( ControlEventCode code )
{
	if( code == B_CONTROL_EXIT_GUI_MODE )
	{
		// Disable the exitGUIMode key binding if we don't have a modal dialog up or if 
		// the cursor is inside an edit or multiline edit control
		if( !fGUIManager->IModalBlocking() )
			return false;

		pfGUIDialogMod *dlg = fGUIManager->IGetTopModal();
		if( dlg != nil )
		{
			pfGUIControlMod *ctrl = dlg->GetFocus();
			if( ctrl != nil && ctrl->HasFlag( pfGUIControlMod::kTakesSpecialKeys ) )
				return false;
		}
	}
	return true;	// Enable all other codes
}

hsBool	pfGameUIInputInterface::IHandleCtrlCmd( plCtrlCmd *cmd )
{
	if( cmd->fControlCode == B_CONTROL_EXIT_GUI_MODE )
	{
		if( cmd->fControlActivated )
		{
			pfGUIDialogMod *dlg = fGUIManager->IGetTopModal();
			if( dlg != nil && dlg->GetHandler() != nil )
				dlg->GetHandler()->OnControlEvent( pfGUIDialogProc::kExitMode );
		}

		return true;
	}
	return false;
}

hsBool	pfGameUIInputInterface::InterpretInputEvent( plInputEventMsg *pMsg )
{
	hsBool		handled = false;


	/// The in-game UI has to do far more complicated control handling, so we just overload this entirely
	plKeyEventMsg *pKeyMsg = plKeyEventMsg::ConvertNoRef( pMsg );
	if( pKeyMsg )
	{
		// By default, we don't want the modifier keys treated as "handled", 'cause
		// we want the other interfaces to get them as well (unless we have a modal
		// as the top dialog).
		if( pKeyMsg->GetKeyCode() == KEY_SHIFT )
		{
			if( pKeyMsg->GetKeyDown() )
				fModifiers |= pfGameGUIMgr::kShiftDown;
			else
				fModifiers &= ~pfGameGUIMgr::kShiftDown;
		}
		else if( pKeyMsg->GetKeyCode() == KEY_CTRL )
		{
			if( pKeyMsg->GetKeyDown() )
				fModifiers |= pfGameGUIMgr::kCtrlDown;
			else
				fModifiers &= ~pfGameGUIMgr::kCtrlDown;
		}
		else if( pKeyMsg->GetKeyCode() == KEY_CAPSLOCK )
		{
			if( pKeyMsg->GetKeyDown() )
				fModifiers |= pfGameGUIMgr::kCapsDown;
			else
				fModifiers &= ~pfGameGUIMgr::kCapsDown;
		}
		else
		{
			// Sometimes I can't explain why Mathew does some of the things he does.
			// I going to replace his modifier flags (which I don't know why he thought he had to have his own)
			//   with the ones that are in the keymsg since they seem to be more accurate!
			fModifiers = 0;
			if ( pKeyMsg->GetShiftKeyDown() )
				fModifiers |= pfGameGUIMgr::kShiftDown;
			if ( pKeyMsg->GetCtrlKeyDown() )
				fModifiers |= pfGameGUIMgr::kCtrlDown;
			if ( pKeyMsg->GetCapsLockKeyDown() )
				fModifiers |= pfGameGUIMgr::kCapsDown;
			if( pKeyMsg->GetKeyDown() )
			{
				if( !pKeyMsg->GetRepeat() )
					handled = fGUIManager->IHandleKeyEvt( pfGameGUIMgr::kKeyDown, pKeyMsg->GetKeyCode(), fModifiers );
				else
					handled = fGUIManager->IHandleKeyEvt( pfGameGUIMgr::kKeyRepeat, pKeyMsg->GetKeyCode(), fModifiers );

				handled |= fGUIManager->IHandleKeyPress( plKeyboardDevice::KeyEventToChar( pKeyMsg ), fModifiers );
			}
			else
				handled = fGUIManager->IHandleKeyEvt( pfGameGUIMgr::kKeyUp, pKeyMsg->GetKeyCode(), fModifiers );
		}

		// We need to do early interception of a screenshot request, since they want
		// us to be able to take screen shots while in a modal GUI... whee
		// Also, this should only be run if the dialog didn't handle the command in
		// the first place (taking screenshots while the user is typing would be
		// awkward) and we must do it on key down because the key binding routines
		// also trigger on key-down and we don't want to be taking screen shots when
		// the user re-binds the screenshot command.
		// HACK HACK HACK
		if ((!handled) && (pKeyMsg->GetKeyDown()))
		{
			const plKeyBinding* keymap = plInputInterfaceMgr::GetInstance()->FindBindingByConsoleCmd("Game.KITakePicture");
			if (keymap)
			{
				unsigned keyFlags = 0;
				if (pKeyMsg->GetCtrlKeyDown())
					keyFlags |= plKeyCombo::kCtrl;
				if (pKeyMsg->GetShiftKeyDown())
					keyFlags |= plKeyCombo::kShift;
				plKeyCombo combo(pKeyMsg->GetKeyCode(), keyFlags);
				if ((keymap->GetKey1().IsSatisfiedBy(combo)) || (keymap->GetKey2().IsSatisfiedBy(combo)))
				{
					// tell the KI to take the shot
					plConsoleMsg * consoleMsg = NEWZERO(plConsoleMsg);
					consoleMsg->SetCmd(plConsoleMsg::kExecuteLine);
					consoleMsg->SetString("Game.KITakePicture");
					consoleMsg->Send(nil, true);
				}
			}
		}

		bool modal = fGUIManager->IModalBlocking();
		return handled || modal; // we "handle" it if we are modal, even if it didn't do anything
	}

	plMouseEventMsg *pMouseMsg = plMouseEventMsg::ConvertNoRef( pMsg );
	if( pMouseMsg && fManager->IsClickEnabled() )
	{
		if( pMouseMsg->GetButton() == kLeftButtonDown )
		{
			handled = fGUIManager->IHandleMouse( pfGameGUIMgr::kMouseDown, pMouseMsg->GetXPos(), pMouseMsg->GetYPos(), fModifiers, &fCurrentCursor );
			if (handled)
				fButtonState |= kLeftButtonDown;
		}
		else if( pMouseMsg->GetButton() == kLeftButtonUp )
		{
			handled = fGUIManager->IHandleMouse( pfGameGUIMgr::kMouseUp, pMouseMsg->GetXPos(), pMouseMsg->GetYPos(), fModifiers, &fCurrentCursor );
			if ((handled) || (fButtonState & kLeftButtonDown)) // even if we didn't handle the mouse up, if we think the button is still down, we should clear our flag
				fButtonState &= ~kLeftButtonDown;
		}
		else if( pMouseMsg->GetButton() == kLeftButtonDblClk )
			handled = fGUIManager->IHandleMouse( pfGameGUIMgr::kMouseDblClick, pMouseMsg->GetXPos(), pMouseMsg->GetYPos(), fModifiers, &fCurrentCursor );
		else if( fButtonState & kLeftButtonDown )
			handled = fGUIManager->IHandleMouse( pfGameGUIMgr::kMouseDrag, pMouseMsg->GetXPos(), pMouseMsg->GetYPos(), fModifiers, &fCurrentCursor );
		else
			handled = fGUIManager->IHandleMouse( pfGameGUIMgr::kMouseMove, pMouseMsg->GetXPos(), pMouseMsg->GetYPos(), fModifiers, &fCurrentCursor );

		fHaveInterestingCursor = handled;
		return handled;
	}

	return false;
}	

UInt32	pfGameUIInputInterface::GetCurrentCursorID( void ) const
{
	if( fCurrentCursor == 0 )
	{
		if ( pfGameGUIMgr::GetInstance() )
			return pfGameGUIMgr::GetInstance()->GetDefaultCursor();
		else
			return kCursorUp;
	}

	return fCurrentCursor;
}

hsScalar pfGameUIInputInterface::GetCurrentCursorOpacity( void ) const
{
	if ( pfGameGUIMgr::GetInstance() )
		return pfGameGUIMgr::GetInstance()->GetCursorOpacity();
	else
		return 1.f;
}

//////////////////////////////////////////////////////////////////////////////
//// Tag Stuff ///////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

extern pfGUITag	gGUITags[];		// From pfGUITagDefs.cpp

//// GetDialogFromTag ////////////////////////////////////////////////////////

pfGUIDialogMod	*pfGameGUIMgr::GetDialogFromTag( UInt32 tagID )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( fDialogs[ i ]->GetTagID() == tagID )
			return fDialogs[ i ];
	}

	return nil;
}

//// GetDialogFromString ////////////////////////////////////////////////////////

pfGUIDialogMod	*pfGameGUIMgr::GetDialogFromString( const char *name )
{
	int		i;


	for( i = 0; i < fDialogs.GetCount(); i++ )
	{
		if( stricmp( fDialogs[ i ]->GetName(), name ) == 0 )
			return fDialogs[ i ];
	}

	return nil;
}

//// GetControlFromTag ///////////////////////////////////////////////////////

pfGUIControlMod	*pfGameGUIMgr::GetControlFromTag( pfGUIDialogMod *dlg, UInt32 tagID )
{
	return dlg->GetControlFromTag( tagID );
}

//// GetNumTags //////////////////////////////////////////////////////////////

UInt32			pfGameGUIMgr::GetNumTags( void )
{
	UInt32		count;


	for( count = 0; gGUITags[ count ].fID != 0; count++ );
	return count;	
}

//// GetTag //////////////////////////////////////////////////////////////////

pfGUITag		*pfGameGUIMgr::GetTag( UInt32 tagIndex )
{
	UInt32		count;

	
	for( count = 0; gGUITags[ count ].fID != 0; count++ );
	hsAssert( tagIndex < count, "Bad index to GetTag()" );
			
	return &gGUITags[ tagIndex ];
}

UInt32		pfGameGUIMgr::GetHighestTag( void )
{
	UInt32	i, id = 1;


	for( i = 0; gGUITags[ i ].fID != 0; i++ )
	{
		if( id < gGUITags[ i ].fID )
			id = gGUITags[ i ].fID;
	}

	return id;
}


void pfGameGUIMgr::SetAspectRatio(hsScalar aspectratio)
{
	hsScalar oldAspectRatio = fAspectRatio;

	// don't allow the aspectratio below 4:3
	hsScalar fourThree = 4.0f/3.0f;
	fAspectRatio = aspectratio < fourThree ? fourThree : aspectratio;

	if (fAspectRatio != oldAspectRatio)
	{
		// need to tell dialogs to update
		int i;
		for (i = 0; i < fDialogs.GetCount(); i++)
		{
			if (fDialogs[i])
				fDialogs[i]->UpdateAspectRatio();
		}
	}
}
