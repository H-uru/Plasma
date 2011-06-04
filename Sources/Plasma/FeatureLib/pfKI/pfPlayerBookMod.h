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
//	pfPlayerBookMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfPlayerBookMod_h
#define _pfPlayerBookMod_h

#include "../pnModifier/plSingleModifier.h"

class plMessage;
class pfGUICheckBoxCtrl;
class pfGUIButtonMod;
class pfPlayerBookProc;

class pfPlayerBookMod : public plSingleModifier
{
	protected:

		// We have six preview panes, each with a GUI check box control.
		// We have to have pointers to all six checkboxes so we can attach
		// procs to them, as well as all six dynamic layers that are the
		// preview panes.

		pfGUICheckBoxCtrl	*fCheckBoxes[ 6 ];
		plKey				fDynLayerKeys[ 6 ];

		// Also got a load and save button somewhere
		pfGUIButtonMod		*fLoadButton, *fSaveButton;

		pfPlayerBookProc	*fPBProc;
		
		enum
		{
			kRefCheckBox,
			kRefLoadButton,
			kRefSaveButton
		};

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

	public:

		pfPlayerBookMod();
		virtual ~pfPlayerBookMod();

		CLASSNAME_REGISTER( pfPlayerBookMod );
		GETINTERFACE_ANY( pfPlayerBookMod, plSingleModifier );

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

};

#endif // _pfPlayerBookMod_h
