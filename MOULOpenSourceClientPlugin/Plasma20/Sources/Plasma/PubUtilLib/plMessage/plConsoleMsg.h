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

////// TEMP HACK TO GET CONSOLE INIT EXECUTION ON AGE LOAD WORKING

#ifndef plConsoleMsg_inc
#define plConsoleMsg_inc

#include "../pnMessage/plMessage.h"

class plEventCallbackMsg;

class plConsoleMsg : public plMessage
{
protected:

	UInt32		fCmd;
	char		*fString;

public:

	enum 
	{
		kExecuteFile,
		kAddLine,
		kExecuteLine
	};

	plConsoleMsg() : plMessage(nil, nil, nil), fCmd( 0 ), fString( nil ) { SetBCastFlag(kBCastByExactType); }
	plConsoleMsg( UInt32 cmd, const char *str ) : 
				plMessage(nil, nil, nil), fCmd( cmd ), fString(hsStrcpy(str))
				{ SetBCastFlag( kBCastByExactType ); }
	
	~plConsoleMsg() { FREE(fString); }

	CLASSNAME_REGISTER( plConsoleMsg );
	GETINTERFACE_ANY( plConsoleMsg, plMessage );

	UInt32		GetCmd( void ) const { return fCmd; }
	const char	*GetString( void ) const { return fString; };
	
	void SetCmd (UInt32 cmd) { fCmd = cmd; }
	void SetString (const char str[]) { FREE(fString); fString = hsStrcpy(str); }

	virtual void Read(hsStream* s, hsResMgr* mgr) 
	{ 
		plMessage::IMsgRead(s, mgr); 
		s->ReadSwap(&fCmd);
		// read string
		plMsgCStringHelper::Peek(fString, s);				
	}
	
	virtual void Write(hsStream* s, hsResMgr* mgr) 
	{ 
		plMessage::IMsgWrite(s, mgr);
		s->WriteSwap(fCmd);
		// write cmd/string
		plMsgCStringHelper::Poke(fString, s);		
	}
};

#endif // plConsole_inc
