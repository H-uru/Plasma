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
//	plInputInterface.cpp - A single layer on the input interface stack		//
//																			//
//// History /////////////////////////////////////////////////////////////////
//																			//
//	2.20.02 mcn	- Created.													//
//																			//
//// Note on GetPriorityLevel() //////////////////////////////////////////////
//																			//
//	The inputInterfaceMgr uses GetPriorityLevel() to place each interface	//
//	into the stack relative to the other interfaces. Current priority		//
//	levels are:																//
//		Console				- 100											//
//		GUI system			- 75											//
//		Avatar input		- 50											//
//		Scene interaction	- 25											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plInputInterface_h
#define _plInputInterface_h

#include "hsRefCnt.h"
#include "hsTemplates.h"
#include "hsBitVector.h"
// Needed for UNIX Build
//  only windows will let you predeclare an enum
#include "../../NucleusLib/pnInputCore/plKeyDef.h"
#include "../../NucleusLib/pnInputCore/plControlEventCodes.h"


//// Class Definition ////////////////////////////////////////////////////////

class hsStream;
class hsResMgr;
class plInputEventMsg;
class plInputInterfaceMgr;
class plMessage;
class plKeyMap;
class plCtrlCmd;
class plKeyBinding;

class plInputInterface : public hsRefCnt
{
	friend class plInputInterfaceMgr;

	protected:

		enum Priorities
		{
			kConsolePriority = 100,
			kGUISystemPriority = 75,
			kDebugCmdPrioity = 60,
			kSceneInteractionPriority = 50,
			kTelescopeInputPriority = 26,
			kAvatarInputPriority = 25,
		};

		plInputInterfaceMgr	*fManager;

		plKeyMap				*fControlMap;
		hsTArray<plCtrlCmd *>	*fMessageQueue;

		hsBitVector		fKeyControlFlags;
		hsBitVector		fKeyControlsFrom2ndKeyFlags;
		hsBitVector		fDisabledControls;
		hsBool			fEnabled;

		void		ISetMessageQueue( hsTArray<plCtrlCmd *> *queue ) { fMessageQueue = queue; }
		plKeyMap	*IGetControlMap( void ) const { return fControlMap; }
		hsBool		IOwnsControlCode( ControlEventCode code );
		hsBool		IVerifyShiftKey( plKeyDef key, int index );

		hsBool	IHasKeyControlFlag(int f) const	{ return fKeyControlFlags.IsBitSet(f); }
		void	ISetKeyControlFlag(int f)		{ fKeyControlFlags.SetBit(f); }
		void	IClearKeyControlFlag(int which)	{ fKeyControlFlags.ClearBit( which ); }
		void	IDisableControl(int which)		{ fDisabledControls.SetBit(which); }
		void	IEnableControl(int which)		{ fDisabledControls.ClearBit(which); }

		// The binding lost focus/priority. Behave as though they released the key and send
		// a deactivate message for the control code.
		void	IDeactivateBinding(const plKeyBinding *binding);


		// Gets called once per IUpdate(), just like normal IEval()s
		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ) { return false; }

		// Override to handle special-cased control messages of your own (same as receiving them via a message, but if you process them, nobody else gets them). Return false if you don't handle it.
		virtual hsBool	IHandleCtrlCmd( plCtrlCmd *cmd ) { return false; }

		// Override to let the input interfaces control when a binding is truly active. If this function returns false,
		// ProcessKeyBindings will ignore the keypress for the given control. This way, the interfaces can be selective
		// about which bindings are active when. By default, always returns true, since there are very few and rare
		// cases where you'd want to return false
		virtual hsBool	IControlCodeEnabled( ControlEventCode code );

		// Some helpers for derived classes to avoid including the manager unnecessariliy

	public:

		plInputInterface();
		virtual ~plInputInterface();

		enum Cursors
		{
			kNullCursor	= 0,
			kCursorUp,
			kCursorLeft,
			kCursorRight,
			kCursorDown,
			kCursorPoised,
			kCursorClicked,
			kCursorUnClicked,
			kCursorHidden,
			kCursorOpen,
			kCursorGrab,
			kCursorArrow,
			kCursor4WayDraggable,
			kCursor4WayDragging,
			kCursorUpDownDraggable,
			kCursorUpDownDragging,
			kCursorLeftRightDraggable,
			kCursorLeftRightDragging,
			kCursorOfferBook,
			kCursorOfferBookHilite,
			kCursorOfferBookClicked,
			kCursorClickDisabled,
			kCursorHand,
			kCursorUpward,
		};

		virtual void	Read( hsStream* s, hsResMgr* mgr );
		virtual void	Write( hsStream* s, hsResMgr* mgr );

		// Returns the priority of this interface layer, based on the Priorities enum
		virtual UInt32		GetPriorityLevel( void ) const = 0;

		// Returns true if the message was handled, false if not and we want to pass it on to others in the stack
		virtual hsBool		InterpretInputEvent( plInputEventMsg *pMsg ) = 0;

		// Returns the currently active mouse cursor for this layer, as defined in pnMessage/plCursorChangeMsg.h
		virtual UInt32		GetCurrentCursorID( void ) const = 0;

		// Returns the current opacity that this layer wants the cursor to be, from 0 (xparent) to 1 (opaque)
		virtual hsScalar	GetCurrentCursorOpacity( void ) const { return 1.f; }

		// Returns true if this layer is wanting to change the mouse, false if it isn't interested
		virtual hsBool		HasInterestingCursorID( void ) const = 0;

		// Gets called by the manager. If you want a message to come to you, set your manager as the destination
		virtual hsBool		MsgReceive( plMessage *msg ) { return false; }

		// Any initialization that requires a pointer to the manager needs to be done on Init()/Shutdown()
		virtual void		Init( plInputInterfaceMgr *manager ) { fManager = manager; RestoreDefaultKeyMappings(); }
		virtual void		Shutdown( void ) {}

		// Gets called when any of the key mappings are changed, so that the interface layer can refresh the ones its interested in
		virtual void		RefreshKeyMap( void ) {}

		// Called when the interface manager is setting all key mappings to default
		virtual void		RestoreDefaultKeyMappings( void ) {}

		// Called on each interface layer that gets missed when processing inputEvents in the manager (i.e. you either get this call or InterpretInputEvent)
		virtual void		MissedInputEvent( plInputEventMsg *pMsg ) {}

		// Non-virtual, can't override--processes an inputEventMsg to see if we can handle it via a key binding (if so, InterpretInputEvent won't be called)
		hsBool				ProcessKeyBindings( plInputEventMsg *keyMsg );

		void		SetEnabled( hsBool e ) { fEnabled = e; }
		hsBool		IsEnabled( void ) const { return fEnabled; }
		
		// clear all keys from map
		virtual void	ClearKeyMap(); 
		
		// reset clickable state
		virtual void ResetClickableState() {;}
};


#endif // _plInputInterface_h
