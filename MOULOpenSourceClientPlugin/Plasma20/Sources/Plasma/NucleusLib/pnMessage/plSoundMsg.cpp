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
#include "plSoundMsg.h"
#include "hsStream.h"

plSoundMsg::~plSoundMsg()
{
	ClearCmd();
}


void plSoundMsg::ClearCmd() 
{ 
	plMessageWithCallbacks::Clear();
	fCmd.Clear(); 
}


void plSoundMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessageWithCallbacks::Read(stream, mgr);

	fCmd.Read(stream);
	stream->ReadSwap(&fBegin);
	stream->ReadSwap(&fEnd);
	fLoop = stream->ReadBool();
	fPlaying = stream->ReadBool();
	stream->ReadSwap(&fSpeed);
	stream->ReadSwap(&fTime);
	stream->ReadSwap(&fIndex);
	stream->ReadSwap(&fRepeats);
	stream->ReadSwap(&fNameStr);
	stream->ReadSwap(&fVolume);
	fFadeType = (plSoundMsg::FadeType)stream->ReadByte();
}

void plSoundMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessageWithCallbacks::Write(stream, mgr);

	fCmd.Write(stream);
	stream->WriteSwap(fBegin);
	stream->WriteSwap(fEnd);
	stream->WriteBool(fLoop);
	stream->WriteBool(fPlaying);
	stream->WriteSwap(fSpeed);
	stream->WriteSwap(fTime);
	stream->WriteSwap(fIndex);
	stream->WriteSwap(fRepeats);
	stream->WriteSwap(fNameStr);
	stream->WriteSwap(fVolume);
	stream->WriteByte( (UInt8)fFadeType );
}
