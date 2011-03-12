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
#include "plClothingMsg.h"
#include "hsResMgr.h"
#include "hsBitVector.h"

void plClothingMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);

	fCommands = stream->ReadSwap32();
	if (stream->ReadBool())
		fItemKey = mgr->ReadKey(stream);
	fColor.Read(stream);
	fLayer = stream->ReadByte();
	fDelta = stream->ReadByte();
	fWeight = stream->ReadSwapScalar();
}

void plClothingMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);

	stream->WriteSwap32(fCommands);
	stream->WriteBool(fItemKey != nil);
	if (fItemKey)
		mgr->WriteKey(stream, fItemKey);
	fColor.Write(stream);
	stream->WriteByte(fLayer);
	stream->WriteByte(fDelta);
	stream->WriteSwapScalar(fWeight);
}

enum ClothingFlags
{
	kClothingCommands,
	kClothingItemKey,
	kClothingColor,
};

void plClothingMsg::ReadVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgReadVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.Read(s);

	if (contentFlags.IsBitSet(kClothingCommands))
		fCommands = s->ReadSwap32();
	if (contentFlags.IsBitSet(kClothingItemKey))
	{
		if (s->ReadBool())
			fItemKey = mgr->ReadKey(s);
	}
	if (contentFlags.IsBitSet(kClothingColor))
		fColor.Read(s);
}

void plClothingMsg::WriteVersion(hsStream* s, hsResMgr* mgr)
{
	plMessage::IMsgWriteVersion(s, mgr);

	hsBitVector contentFlags;
	contentFlags.SetBit(kClothingCommands);
	contentFlags.SetBit(kClothingItemKey);
	contentFlags.SetBit(kClothingColor);
	contentFlags.Write(s);

	// kClothingCommands
	s->WriteSwap32(fCommands);
	// kClothingItemKey
	s->WriteBool(fItemKey != nil);
	if (fItemKey)
		mgr->WriteKey(s, fItemKey);
	// kClothingColor
	fColor.Write(s);
}

/////////////////////////////////////////////////////////////////////////////////

plClothingUpdateBCMsg::plClothingUpdateBCMsg() { SetBCastFlag(plMessage::kBCastByExactType); }

void plClothingUpdateBCMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
}

void plClothingUpdateBCMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
}
