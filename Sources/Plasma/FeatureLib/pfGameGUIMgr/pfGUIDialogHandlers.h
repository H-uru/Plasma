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
//	pfGUIDialogHandlers Header												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDialogHandlers_h
#define _pfGUIDialogHandlers_h

#include "hsStream.h"
#include "pfGUIControlHandlers.h"

//// pfGUIDialogProc Definition //////////////////////////////////////////////
//	This works very much like the control proc objects. The idea is, if you
//	want to do some custom work on a dialog (and who doesn't?), you create an
//	object derived from this type, override the functions, and do as you
//	please. The class type also derives from the control proc type, meaning
//	that you can implement DoSomething() as well and use the same object for
//	both your dialog and your control procs. (DoSomething() is overloaded here
//	so that it's no longer pure virtual, so you can use it for only handling
//	dialogs if you prefer).

class pfGUIDialogMod;
class pfGUIDialogProc : public pfGUICtrlProcObject
{
	protected:

		pfGUIDialogMod	*fDialog;

	public:

		pfGUIDialogProc() { }
		virtual ~pfGUIDialogProc() { ; }

		// Called by the mgr--don't call yourself!
		void	SetDialog( pfGUIDialogMod *dlg ) { fDialog = dlg; }

		// Enums for OnControlEvent
		enum ControlEvt
		{
			kExitMode
		};

		//////// FUNCTIONS TO OVERLOAD ////////

		// Overloaded here so you don't have to unless you want to. Overload
		// it if you want to use this for a control handler as well.
		virtual void	DoSomething( pfGUIControlMod *ctrl ) {;}

		// Called on dialog init (i.e. first showing, before OnShow() is called), only ever called once
		virtual void	OnInit( void ) { ; }

		// Called before the dialog is shown, always after OnInit()
		virtual void	OnShow( void ) { ; }

		// Called before the dialog is hidden
		virtual void	OnHide( void ) { ; }

		// Called on the dialog's destructor, before it's unregistered with the game GUI manager
		virtual void	OnDestroy( void ) { ; }

		// Called when the dialog's focused control changes
		virtual void	OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl ) { ; }

		// Called when the key bound to a GUI event is pressed. Only called on the top modal dialog
		virtual void	OnControlEvent( ControlEvt event ) { ; }

		// Called when the GUI changes interesting state
		virtual void	OnInterestingEvent( pfGUIControlMod *ctrl ) { ; }
};

#endif // _pfGUIDialogHandlers_h
