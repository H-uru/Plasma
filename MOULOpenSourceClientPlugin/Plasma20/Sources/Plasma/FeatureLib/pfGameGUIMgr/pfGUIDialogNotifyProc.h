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
//	pfGUIDialogNotifyProc Header											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDialogNotifyProc_h
#define _pfGUIDialogNotifyProc_h

#include "pfGUIDialogHandlers.h"
#include "../pnKeyedObject/plKey.h"

class plGUIControlMod;

//// pfGUIDialogNotifyProc Definition ////////////////////////////////////////
//	Helper dialog proc that takes all control events and turns them into
//	notify messages that get sent out.

class pfGUIDialogNotifyProc : public pfGUIDialogProc
{
	protected:
		
		plKey	fReceiver;

		void	ISendNotify( plKey ctrlKey, UInt32 event );

	public:

		pfGUIDialogNotifyProc( plKey &r );

		virtual void	DoSomething( pfGUIControlMod *ctrl );
		virtual void	OnInit( void );
		virtual void	OnShow( void );
		virtual void	OnHide( void );
		virtual void	OnDestroy( void );
		virtual void	OnCtrlFocusChange( pfGUIControlMod *oldCtrl, pfGUIControlMod *newCtrl );
		virtual void	OnControlEvent( ControlEvt event );
		virtual void	OnInterestingEvent( pfGUIControlMod *ctrl );
};

#endif // _pfGUIDialogNotifyProc_h
