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
#include "hsStream.h"
#include "plLoadAgeMsg.h"
#include "hsResMgr.h"
#include "hsUtils.h"
#include "hsBitVector.h"

void plLoadAgeMsg::Read(hsStream* stream, hsResMgr* mgr)	
{	
	plMessage::IMsgRead(stream, mgr);	

	delete [] fAgeFilename;
	
	// read agename
	UInt8 len;
	stream->ReadSwap(&len);
	if (len)
	{
		fAgeFilename=TRACKED_NEW char[len+1];
		stream->Read(len, fAgeFilename);
		fAgeFilename[len]=0;
	}
	fUnload = stream->ReadBool();
	stream->ReadSwap(&fPlayerID);
	fAgeGuid.Read(stream);
}

void plLoadAgeMsg::Write(hsStream* stream, hsResMgr* mgr)	
{	
	plMessage::IMsgWrite(stream, mgr);	

	// write agename
	UInt8 len=fAgeFilename?hsStrlen(fAgeFilename):0;
	stream->WriteSwap(len);
	if (len)
	{
		stream->Write(len, fAgeFilename);
	}
	stream->WriteBool(fUnload);
	stream->WriteSwap(fPlayerID);
	fAgeGuid.Write(stream);
}

enum LoadAgeFlags
{
	kLoadAgeAgeName,
	kLoadAgeUnload,
	kLoadAgePlayerID,
	kLoadAgeAgeGuid,
};
	
void plLoadAgeMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kLoadAgeAgeName))
	{
		// read agename
		delete [] fAgeFilename;
		fAgeFilename = s->ReadSafeString();
	}

	if (contentFlags.IsBitSet(kLoadAgeUnload))
		fUnload = s->ReadBool();

	if (contentFlags.IsBitSet(kLoadAgePlayerID))
		s->ReadSwap(&fPlayerID);

	if (contentFlags.IsBitSet(kLoadAgeAgeGuid))
		fAgeGuid.Read(s);
}

void plLoadAgeMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kLoadAgeAgeName);
	contentFlags.SetBit(kLoadAgeUnload);
	contentFlags.SetBit(kLoadAgePlayerID);
	contentFlags.SetBit(kLoadAgeAgeGuid);
	contentFlags.Write(s);

	// kLoadAgeAgeName
	s->WriteSafeString(fAgeFilename);
	// kLoadAgeUnload
	s->WriteBool(fUnload);
	// kLoadAgePlayerID
	s->WriteSwap(fPlayerID);
	// kLoadAgeAgeGuid
	fAgeGuid.Write(s);
}
