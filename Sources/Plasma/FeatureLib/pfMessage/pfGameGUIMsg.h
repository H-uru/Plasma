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
//	pfGameGUIMsg Header 													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGameGUIMsg_h
#define _pfGameGUIMsg_h

#include "hsTypes.h"
#include "hsStream.h"
#include "../pnMessage/plMessage.h"

class pfGameGUIMsg : public plMessage
{
	protected:

		UInt8	fCommand;
		char	fString[ 128 ];		
		char	*fAge;

	public:
		enum 
		{
			kShowDialog,
			kHideDialog,
			kLoadDialog
		};

		pfGameGUIMsg() : plMessage( nil, nil, nil ) { SetBCastFlag( kBCastByExactType ); fAge = nil; }
		pfGameGUIMsg( plKey &receiver, UInt8 command ) : plMessage( nil, nil, nil ) { AddReceiver( receiver ); fCommand = command; fAge = nil; }
		~pfGameGUIMsg() { delete [] fAge; }

		CLASSNAME_REGISTER( pfGameGUIMsg );
		GETINTERFACE_ANY( pfGameGUIMsg, plMessage );

		virtual void Read(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgRead( s, mgr ); 
			s->ReadSwap( &fCommand );
			s->Read( sizeof( fString ), fString );
			fAge = s->ReadSafeString();
		}
		
		virtual void Write(hsStream* s, hsResMgr* mgr) 
		{ 
			plMessage::IMsgWrite( s, mgr ); 
			s->WriteSwap( fCommand );
			s->Write( sizeof( fString ), fString );
			s->WriteSafeString( fAge );
		}

		UInt8		GetCommand( void ) { return fCommand; }

		void		SetString( const char *str ) { hsStrncpy( fString, str, sizeof( fString ) - 1 ); }
		const char	*GetString( void ) { return fString; }

		void		SetAge( const char *str ) { delete [] fAge; if( str == nil ) fAge = nil; else fAge = hsStrcpy( str ); }
		const char	*GetAge( void ) { return fAge; }
};

#endif // _pfGameGUIMsg_h
