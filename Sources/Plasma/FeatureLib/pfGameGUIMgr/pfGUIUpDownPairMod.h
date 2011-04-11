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
//	pfGUIUpDownPairMod Header												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIUpDownPairMod_h
#define _pfGUIUpDownPairMod_h

#include "pfGUIValueCtrl.h"

class plMessage;
class pfGUIButtonMod;
class pfUpDownBtnProc;

class pfGUIUpDownPairMod : public pfGUIValueCtrl
{
	friend class pfUpDownBtnProc;

	protected:

		enum
		{
			kRefUpControl = kRefDerivedStart,
			kRefDownControl
		};

		pfGUIButtonMod	*fUpControl, *fDownControl;
		pfUpDownBtnProc	*fButtonProc;


		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()
		virtual void	IUpdate( void );

	public:

		pfGUIUpDownPairMod();
		virtual ~pfGUIUpDownPairMod();

		CLASSNAME_REGISTER( pfGUIUpDownPairMod );
		GETINTERFACE_ANY( pfGUIUpDownPairMod, pfGUIValueCtrl );


		virtual hsBool	MsgReceive( plMessage* pMsg );

		virtual void	Update( void );

		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	SetRange( hsScalar min, hsScalar max );
		virtual void	SetCurrValue( hsScalar v );

		/// Export ONLY

		void	SetControls( pfGUIButtonMod *up, pfGUIButtonMod *down ) { fUpControl = up; fDownControl = down; }
};

#endif // _pfGUIUpDownPairMod_h
