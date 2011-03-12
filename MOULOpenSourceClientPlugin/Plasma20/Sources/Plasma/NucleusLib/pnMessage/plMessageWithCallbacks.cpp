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
#include "hsResMgr.h"
#include "plMessageWithCallbacks.h"
#include "plEventCallbackMsg.h"
#include "../pnNetCommon/plSynchedObject.h"
#include "plgDispatch.h"
#include "hsBitVector.h"

plMessageWithCallbacks::~plMessageWithCallbacks()
{
	Clear();
}

void plMessageWithCallbacks::AddCallback(plMessage* e) 
{ 
	hsRefCnt_SafeRef(e); 

	// make sure callback msgs have the same net propagate properties as the container msg
	e->SetBCastFlag(plMessage::kNetPropagate, HasBCastFlag(plMessage::kNetPropagate));

	fCallbacks.Append(e); 
}

void plMessageWithCallbacks::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	
	Clear();

	// read count
	int n = stream->ReadSwap32();
	fCallbacks.SetCount(n);

	// read callbacks
	int i;
	for( i = 0; i < n; i++ )
	{
		fCallbacks[i] = plMessage::ConvertNoRef(mgr->ReadCreatable(stream));
	}
}

void plMessageWithCallbacks::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	// write count
	int n=fCallbacks.GetCount();
	stream->WriteSwap32(n);

	// write callbacks
	int i;
	for( i = 0; i < fCallbacks.GetCount(); i++ )
		mgr->WriteCreatable( stream, fCallbacks[i] );
}

enum MsgWithCallbacksFlags
{
	kMsgWithCBsCallbacks,
};

void plMessageWithCallbacks::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	Clear();

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kMsgWithCBsCallbacks))
	{
		// read count
		int n = s->ReadSwap32();
		fCallbacks.SetCount(n);

		for (int i = 0; i < n; i++)
			fCallbacks[i] = plMessage::ConvertNoRef(mgr->ReadCreatableVersion(s));
	}
}

void plMessageWithCallbacks::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kMsgWithCBsCallbacks);
	contentFlags.Write(s);

	// write count
	int n = fCallbacks.GetCount();
	s->WriteSwap32(n);

	// write callbacks
	for (int i = 0; i < n; i++)
		mgr->WriteCreatableVersion(s, fCallbacks[i]);
}

void plMessageWithCallbacks::Clear() 
{ 
	int i;
	for( i = 0; i < fCallbacks.GetCount(); i++ )
		hsRefCnt_SafeUnRef(fCallbacks[i]);
	fCallbacks.SetCount(0);
}

void plMessageWithCallbacks::SendCallbacks()
{
	int i;
	for (i = fCallbacks.GetCount() - 1; i >= 0; i--)
	{
		plgDispatch::MsgSend(fCallbacks[i]);
		fCallbacks.Remove(i);
	}
}