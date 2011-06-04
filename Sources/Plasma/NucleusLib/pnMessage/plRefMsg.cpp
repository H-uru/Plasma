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

#include "hsTypes.h"
#include "plRefMsg.h"
#include "hsStream.h"

#include "hsResMgr.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnKeyedObject/hsKeyedObject.h"

plRefMsg::plRefMsg()
: fRef(nil), fOldRef(nil), fContext(0)
{
}

plRefMsg::plRefMsg(const plKey &r, UInt8 c)
: plMessage(nil, r, nil), fRef(nil), fOldRef(nil), fContext(c)
{
	if( !fContext )
		fContext = kOnCreate;
}

plRefMsg::~plRefMsg()
{
	// Un ref fref and foldref.
}

plRefMsg& plRefMsg::SetRef(hsKeyedObject* ref)
{
	fRef = ref;  // ref count here! paulg
	return *this;
}

plRefMsg& plRefMsg::SetOldRef(hsKeyedObject* oldRef)
{
	fOldRef = oldRef;
	// Ref here! 
	return *this;
}

void plRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	stream->ReadSwap(&fContext);

	plKey key;
	key = mgr->ReadKey(stream);
	fRef = (key ? key->GetObjectPtr() : nil);
	key = mgr->ReadKey(stream);
	fOldRef = (key ? key->GetObjectPtr() : nil);
}

void plRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteSwap(fContext);

	mgr->WriteKey(stream, (fRef ? fRef->GetKey() : nil));
	mgr->WriteKey(stream, (fOldRef ? fOldRef->GetKey() : nil));
}


void plGenRefMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plRefMsg::Read(stream, mgr);
	stream->ReadSwap(&fType);
	fWhich = stream->ReadSwap32();
}

void plGenRefMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plRefMsg::Write(stream, mgr);
	stream->WriteSwap(fType);
	stream->WriteSwap32(fWhich);
}
