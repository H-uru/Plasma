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
#ifndef plAgeLoadedMsg_INC
#define plAgeLoadedMsg_INC

#include "../pnUtils/pnUtils.h"
#include "../pnMessage/plMessage.h"

//
// A msg sent locally when pending pages are done loading or unloading.
//
class hsResMgr;
class hsStream;
class plAgeLoadedMsg : public plMessage
{
public:
	CLASSNAME_REGISTER( plAgeLoadedMsg );
	GETINTERFACE_ANY( plAgeLoadedMsg, plMessage );

	plAgeLoadedMsg() : fLoaded(true) { SetBCastFlag(kBCastByType); }

	// True if the pages finished loading, false if they finished unloading
	bool fLoaded;

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgRead(stream, mgr);	 fLoaded = stream->Readbool(); }
	void Write(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgWrite(stream, mgr); stream->Writebool(fLoaded); }
};

// A msg sent locally when panding pages are done loaded and it's now ok to join the game
class plAgeLoaded2Msg : public plMessage
{
public:
	CLASSNAME_REGISTER( plAgeLoaded2Msg );
	GETINTERFACE_ANY( plAgeLoaded2Msg, plMessage );

	plAgeLoaded2Msg() { SetBCastFlag(kBCastByType); }
	
	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgRead(stream, mgr);	 }
	void Write(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgWrite(stream, mgr); }
};

//
// A msg sent locally when pending pages are beginning to load or unload.
//
class plAgeBeginLoadingMsg : public plMessage
{
public:
	CLASSNAME_REGISTER(plAgeBeginLoadingMsg);
	GETINTERFACE_ANY(plAgeBeginLoadingMsg, plMessage);

	plAgeBeginLoadingMsg() : fLoading(true) { SetBCastFlag(kBCastByType); }

	// True if the pages are starting to load, false if they are starting to unload
	bool fLoading;

	// IO 
	void Read(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgRead(stream, mgr);	 fLoading = stream->Readbool(); }
	void Write(hsStream* stream, hsResMgr* mgr)	{ plMessage::IMsgWrite(stream, mgr); stream->Writebool(fLoading); }
};

//
// A msg sent locally when initial age state has been loaded
//
class plInitialAgeStateLoadedMsg : public plMessage
{
public:
   CLASSNAME_REGISTER( plInitialAgeStateLoadedMsg );
   GETINTERFACE_ANY( plInitialAgeStateLoadedMsg, plMessage );

   plInitialAgeStateLoadedMsg() { SetBCastFlag(kBCastByType);   }

   // IO 
   void Read(hsStream* stream, hsResMgr* mgr) {	plMessage::IMsgRead(stream, mgr); }
   void Write(hsStream* stream, hsResMgr* mgr)	{	plMessage::IMsgWrite(stream, mgr); }
};

#endif		// plAgeLoadedMsg
