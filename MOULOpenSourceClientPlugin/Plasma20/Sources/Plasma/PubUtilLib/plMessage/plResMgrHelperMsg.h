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

#ifndef _plResMgrHelperMsg_h
#define _plResMgrHelperMsg_h

#include "hsTypes.h"
#include "hsStream.h"
#include "../pnMessage/plMessage.h"
#include "../plResMgr/plResManagerHelper.h"

class plResManagerHelper;
class plResMgrHelperMsg : public plMessage
{
protected:

	friend class plResManagerHelper;

	plResPageKeyRefList	*fKeyList;

	UInt8		fCommand;

public:

	enum Commands
	{
		kKeyRefList,
		kUpdateDebugScreen,
		kEnableDebugScreen,
		kDisableDebugScreen
	};

	plResMgrHelperMsg( UInt8 command = 0 ) : plMessage(nil, nil, nil), fKeyList( nil ) { fCommand = command; }
	~plResMgrHelperMsg() { delete fKeyList; }

	CLASSNAME_REGISTER( plResMgrHelperMsg );
	GETINTERFACE_ANY( plResMgrHelperMsg, plMessage );

	virtual void Read(hsStream* s, hsResMgr* mgr) 
	{ 
		hsAssert( false, "This should never get read" );
	}
	
	virtual void Write(hsStream* s, hsResMgr* mgr) 
	{ 
		hsAssert( false, "This should never get written" );
	}

	UInt8	GetCommand( void ) const { return fCommand; }
};

#endif // _plResMgrHelperMsg_h
