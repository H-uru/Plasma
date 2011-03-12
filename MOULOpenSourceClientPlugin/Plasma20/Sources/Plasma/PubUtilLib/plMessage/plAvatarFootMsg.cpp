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
#include "plAvatarFootMsg.h"

#include "hsStream.h"

plAvatarFootMsg::plAvatarFootMsg()
:	fArmature(nil),
	fIsLeft(false);
{
}

plAvatarFootMsg::plAvatarFootMsg(const plKey& s, plArmatureMod *armature, plAvBrain *brain, hsBool isLocal, hsBool isLeft)
:	plArmatureUpdateMsg(s, isLocal, true, armature, brain),
	fIsLeft(isLeft)
{
	SetBCastFlag(plMessage::kBCastByExactType);
}

void plAvatarFootMsg::Read(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}

void plAvatarFootMsg::Write(hsStream* s, hsResMgr* mgr)
{
	hsAssert(false, "This message is not supposed to travel over the network or persist in a file.");
}