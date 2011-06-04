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
#include "plAccountUpdateMsg.h"
#include "hsStream.h"

plAccountUpdateMsg::plAccountUpdateMsg()
{
	fUpdateType = 0;
}

plAccountUpdateMsg::plAccountUpdateMsg(unsigned updateType)
{
	fUpdateType = updateType;
}

void plAccountUpdateMsg::Read(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgRead(stream, mgr);
	fUpdateType = stream->ReadSwap32();
	fResult		= stream->ReadSwap32();
	fPlayerInt	= stream->ReadSwap32();
}

void plAccountUpdateMsg::Write(hsStream* stream, hsResMgr* mgr)
{
	plMessage::IMsgWrite(stream, mgr);
	stream->WriteSwap32(fUpdateType);
	stream->WriteSwap32(fResult);
	stream->WriteSwap32(fPlayerInt);
}

unsigned plAccountUpdateMsg::GetUpdateType()
{
	return fUpdateType;
}

void plAccountUpdateMsg::SetUpdateType(unsigned type)
{
	fUpdateType = type;
}

unsigned plAccountUpdateMsg::GetResult()
{
	return fResult;
}

void plAccountUpdateMsg::SetResult(unsigned result)
{
	fResult = result;
}

unsigned plAccountUpdateMsg::GetPlayerInt()
{
	return fPlayerInt;
}

void plAccountUpdateMsg::SetPlayerInt(unsigned playerInt)
{
	fPlayerInt = playerInt;
}

