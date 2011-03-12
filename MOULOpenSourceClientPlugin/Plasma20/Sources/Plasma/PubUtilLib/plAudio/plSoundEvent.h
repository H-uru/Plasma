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
//	plSoundEvent - Event node for handling callback thread stuff			//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSoundEvent_h
#define _plSoundEvent_h

#include "hsTemplates.h"

class plEventCallbackMsg;
class plSound;

//// plSoundEvent ////////////////////////////////////////////////////////////
//	Storage class for an event node.

class plSoundEvent
{
public:

	enum Types
	{
		kStart,
		kStop,
		kTime,
		kLoop
	};

	plSoundEvent( Types type, plSound *owner );
	plSoundEvent( Types type, UInt32 bytePos, plSound *owner );
	plSoundEvent();
	~plSoundEvent();

	void	AddCallback( plEventCallbackMsg *msg );
	hsBool	RemoveCallback( plEventCallbackMsg *msg );

	UInt32	GetNumCallbacks( void ) const;
	int		GetType( void ) const;
	void	SetType( Types type );
	UInt32	GetTime( void ) const;

	void	SendCallbacks( void );

	static Types	GetTypeFromCallbackMsg( plEventCallbackMsg *msg );

protected:

	Types		fType;
	UInt32		fBytePosTime;
	plSound		*fOwner;

	hsTArray<plEventCallbackMsg	*>	fCallbacks;
	hsTArray<UInt8>					fCallbackEndingFlags;
};


#endif //_plSoundEvent_h
