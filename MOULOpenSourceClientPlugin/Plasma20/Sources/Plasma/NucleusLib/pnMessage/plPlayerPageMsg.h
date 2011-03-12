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

#ifndef plPlayerPageMsg_inc
#define plPlayerPageMsg_inc

#include "../pnMessage/plMessage.h"
#include "hsResMgr.h"
#include "hsStream.h"

class plKey;

class plPlayerPageMsg : public plMessage
{
protected:

public:
	plPlayerPageMsg() : fPlayer(nil),fLocallyOriginated(false),fUnload(false),fLastOut(false),fClientID(-1){;}
	plPlayerPageMsg(const plKey &s, 
					const plKey &r, 
					const double* t) : fPlayer(nil),fLocallyOriginated(false),fUnload(false),fLastOut(false),fClientID(-1){;}
	
	CLASSNAME_REGISTER( plPlayerPageMsg );
	GETINTERFACE_ANY( plPlayerPageMsg, plMessage );


	plKey			fPlayer;
	hsBool			fLocallyOriginated;
	hsBool			fUnload;
	int				fClientID;
	hsBool			fLastOut;

	// IO
	void Read(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgRead(stream, mgr);
		fPlayer = mgr->ReadKey(stream);
		fLocallyOriginated = stream->ReadBool();
		fLastOut = stream->ReadBool();
		fUnload = stream->ReadBool();
		fClientID = stream->ReadSwap32();
	}
	void Write(hsStream* stream, hsResMgr* mgr)
	{
		plMessage::IMsgWrite(stream, mgr);
		mgr->WriteKey(stream, fPlayer);
		stream->WriteBool(fLocallyOriginated);
		stream->WriteBool(fLastOut);
		stream->WriteBool(fUnload);
		stream->WriteSwap32(fClientID);
	}
};

#endif // plPlayerPageMsg_inc

