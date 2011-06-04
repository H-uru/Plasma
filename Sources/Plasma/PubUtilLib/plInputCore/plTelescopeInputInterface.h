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
//	plTelescopeInputInterface													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plTelescopeInputInterface_h
#define _plTelescopeInputInterface_h

#include "plInputInterface.h"
#include "../pnKeyedObject/plKey.h"

//// Class Definition ////////////////////////////////////////////////////////
		
class plTelescopeInputInterface : public plInputInterface
{
	protected:

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty );

	public:

		plTelescopeInputInterface();
		virtual ~plTelescopeInputInterface();

		virtual void		RestoreDefaultKeyMappings( void );
		
		virtual hsBool	InterpretInputEvent( plInputEventMsg *pMsg );

		virtual hsBool	MsgReceive( plMessage *msg );

		virtual void	Init( plInputInterfaceMgr *manager );
		virtual void	Shutdown( void ) {;}

		// Returns the priority of this interface layer, based on the Priorities enum
		virtual UInt32	GetPriorityLevel( void ) const { return kTelescopeInputPriority; }

		// Returns the currently active mouse cursor for this layer, as defined in pnMessage/plCursorChangeMsg.h
		virtual UInt32		GetCurrentCursorID( void ) const { return kCursorUp; }

		// Returns the current opacity that this layer wants the cursor to be, from 0 (xparent) to 1 (opaque)
		virtual hsScalar	GetCurrentCursorOpacity( void ) const { return 1.f; }

		// Returns true if this layer is wanting to change the mouse, false if it isn't interested
		virtual hsBool		HasInterestingCursorID( void ) const { return false; }
};


#endif //_plTelescopeInputInterface_h
