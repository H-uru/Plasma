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
//	pfKI Header																//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfKI_h
#define _pfKI_h

#include "hsTypes.h"
#include "../pnKeyedObject/hsKeyedObject.h"


//// Class Definition ////////////////////////////////////////////////////////

class plKIYesNoBox;
class plKIAddEditBox;
class plKIMainProc;
class plKIMiniProc;
class plPlayerBookProc;
class pfKITextVaultCallback;

class pfKI : public hsKeyedObject 
{
	protected:

		plKIYesNoBox	*fYesNoProc;
		plKIAddEditBox	*fAddEditProc;
		plKIMainProc	*fMainProc;
		plKIMiniProc	*fMiniProc;

		plPlayerBookProc	*fPBProc;

		pfKITextVaultCallback	*fKIVaultCallback;

		static pfKI	*fInstance;

		void	IInitPlayerBook( void );

	public:

		pfKI();
		~pfKI();

		CLASSNAME_REGISTER( pfKI );
		GETINTERFACE_ANY( pfKI, plReceiver );

		virtual hsBool	MsgReceive( plMessage *msg );

		void	Init( void );

		static pfKI	*GetInstance( void ) { return fInstance; }
};


#endif //_pfKI_h

