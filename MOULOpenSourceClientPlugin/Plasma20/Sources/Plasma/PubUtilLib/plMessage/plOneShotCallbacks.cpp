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
#include "plOneShotCallbacks.h"
#include "hsStream.h"
#include "hsUtils.h"
#include "hsResMgr.h"

plOneShotCallbacks::plOneShotCallbacks()
{
}

plOneShotCallbacks::~plOneShotCallbacks()
{
	int size = fCallbacks.size();
	for (int i = 0; i < size; i++)
		delete [] fCallbacks[i].fMarker;
	fCallbacks.clear();
}

void plOneShotCallbacks::AddCallback(const char *marker, plKey &receiver, Int16 user)
{
	fCallbacks.push_back(plOneShotCallback(hsStrcpy(marker), receiver, user));
}

int plOneShotCallbacks::GetNumCallbacks()
{
	return fCallbacks.size();
}

plOneShotCallbacks::plOneShotCallback& plOneShotCallbacks::GetCallback(int i)
{
	return fCallbacks[i];
}

void plOneShotCallbacks::Read(hsStream* stream, hsResMgr* mgr)
{
	int size = stream->ReadSwap32();
	fCallbacks.reserve(size);
	for (int i = 0; i < size; i++)
	{
		char *marker = stream->ReadSafeString();
		plKey receiver = mgr->ReadKey(stream);
		Int16 user = stream->ReadSwap16();

		fCallbacks.push_back(plOneShotCallback(marker, receiver, user));
	}
}

void plOneShotCallbacks::Write(hsStream* stream, hsResMgr* mgr)
{
	int size = fCallbacks.size();
	stream->WriteSwap32(size);
	for (int i = 0; i < size; i++)
	{
		stream->WriteSafeString(fCallbacks[i].fMarker);
		mgr->WriteKey(stream, fCallbacks[i].fReceiver);
		stream->WriteSwap16(fCallbacks[i].fUser);
	}
}