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
#ifndef plLoadAgeMsg_INC
#define plLoadAgeMsg_INC

#include "../pnMessage/plMessage.h"
#include "../plUUID/plUUID.h"
#include "hsUtils.h"

//
// A msg which is sent to the networking system to cause an age to be loaded or unloaded
//
class plKey;
class hsStream;
class hsResMgr;
class plLoadAgeMsg : public plMessage
{
protected:
	char* fAgeFilename;				// the age to load/unload
	plUUID fAgeGuid;
	hsBool fUnload;			// true if we want to unload the age
	int	fPlayerID;
public:
	plLoadAgeMsg() : fAgeFilename(nil), fUnload(false), fPlayerID(-1){ }
	virtual ~plLoadAgeMsg() { delete [] fAgeFilename;  }

	CLASSNAME_REGISTER( plLoadAgeMsg );
	GETINTERFACE_ANY( plLoadAgeMsg, plMessage );

	void SetAgeFilename(const char* a) { delete [] fAgeFilename; fAgeFilename=a?hsStrcpy(a):nil; }
	char* GetAgeFilename() const { return fAgeFilename; }

	void SetAgeGuid( const plUUID * v ) { fAgeGuid.CopyFrom( v ); }
	const plUUID * GetAgeGuid() const { return &fAgeGuid; }

	void SetLoading(hsBool l) { fUnload=!l; }
	hsBool GetLoading() const { return !fUnload; }

	void SetPlayerID(int p) { fPlayerID=p; }
	int GetPlayerID() const { return fPlayerID;	}
	
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr);
	void Write(hsStream* stream, hsResMgr* mgr);

	void ReadVersion(hsStream* s, hsResMgr* mgr);
	void WriteVersion(hsStream* s, hsResMgr* mgr);
};

//
// Internal msg, sent by NetClientMgr to unload an age when linking out.
// Should not be used for other purposes
//
class plLinkOutUnloadMsg : public plLoadAgeMsg
{
public:
	plLinkOutUnloadMsg() { fUnload=true; }

	CLASSNAME_REGISTER( plLinkOutUnloadMsg );
	GETINTERFACE_ANY( plLinkOutUnloadMsg, plLoadAgeMsg );	
};

//
// Internal msg, used by NetClientMgr. 
// (we send another to the avatar that linked)
// Not meant to go over the wire.
//
class plLinkInDoneMsg : public plMessage
{
public:

	CLASSNAME_REGISTER( plLinkInDoneMsg );
	GETINTERFACE_ANY( plLinkInDoneMsg, plMessage );	

	void Read(hsStream* stream, hsResMgr* mgr) { IMsgRead(stream, mgr);	}
	void Write(hsStream* stream, hsResMgr* mgr) { IMsgWrite(stream, mgr);	}

	void ReadVersion(hsStream* stream, hsResMgr* mgr) { IMsgRead(stream, mgr);	};
	void WriteVersion(hsStream* stream, hsResMgr* mgr) {  IMsgWrite(stream, mgr);	};

};

#endif		// plLoadAgeMsg
