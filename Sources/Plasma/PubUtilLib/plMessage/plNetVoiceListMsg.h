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

#ifndef plNetVoiceListMsg_inc
#define plNetVoiceListMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsTemplates.h"

class plNetVoiceListMsg : public plMessage
{
protected:

	hsTArray<UInt32>	fClientIDs;
	int					fCmd;
	plKey				fRemoved;

public:

	enum 
	{
		kForcedListenerMode,
		kDistanceMode,
	};

	plNetVoiceListMsg() : plMessage(nil, nil, nil), fCmd( 0 ) { SetBCastFlag(kBCastByExactType); }
	plNetVoiceListMsg( UInt32 cmd ) : 
				plMessage(nil, nil, nil), fCmd( cmd )
				{ SetBCastFlag( kBCastByExactType ); }
	
	~plNetVoiceListMsg() { ; }

	CLASSNAME_REGISTER( plNetVoiceListMsg );
	GETINTERFACE_ANY( plNetVoiceListMsg, plMessage );

	UInt32		GetCmd( void ) { return fCmd; }
	hsTArray<UInt32>* GetClientList( void ) { return &fClientIDs; };
	plKey GetRemovedKey() { return fRemoved; }
	void SetRemovedKey(plKey& k) { fRemoved = k; }
	virtual void Read(hsStream* s, hsResMgr* mgr); 
	
	virtual void Write(hsStream* s, hsResMgr* mgr); 
};



#endif // plNetVoiceListMsg_inc
