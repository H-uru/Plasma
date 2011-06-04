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
#include "pyPlayer.h"

pyPlayer::pyPlayer() // only used by python glue, do NOT call
{
	fAvatarKey = nil;
	fPlayerName = "";
	fPlayerID = 0;
	fDistSq = -1;
	fIsCCR = false;
	fIsServer = false;
}

pyPlayer::pyPlayer(pyKey& avKey, const char* pname, UInt32 pid, hsScalar distsq)
{
	fAvatarKey = avKey.getKey();
	fPlayerName = pname;
	fPlayerID = pid;
	fDistSq = distsq;
	fIsCCR = false;
	fIsServer = false;
}

pyPlayer::pyPlayer(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq)
{
	fAvatarKey = avKey;
	fPlayerName = pname;
	fPlayerID = pid;
	fDistSq = distsq;
	fIsCCR = false;
	fIsServer = false;
}

// another way to create a player with just a name and number
pyPlayer::pyPlayer(const char* pname, UInt32 pid)
{
	fAvatarKey = nil;
	fPlayerName = pname;
	fPlayerID = pid;
	fDistSq = -1;
	fIsCCR = false;
	fIsServer = false;
}

void pyPlayer::Init(plKey avKey, const char* pname, UInt32 pid, hsScalar distsq) // used by python glue, do NOT call
{
	fAvatarKey = avKey;
	fPlayerName = pname;
	fPlayerID = pid;
	fDistSq = distsq;
	fIsCCR = false;
	fIsServer = false;
}

void pyPlayer::SetCCRFlag(hsBool state)
{
	fIsCCR = state;
}

hsBool pyPlayer::IsCCR()
{
	return fIsCCR;
}

void pyPlayer::SetServerFlag(hsBool state)
{
	fIsServer = state;
}

hsBool pyPlayer::IsServer()
{
	return fIsServer;
}
