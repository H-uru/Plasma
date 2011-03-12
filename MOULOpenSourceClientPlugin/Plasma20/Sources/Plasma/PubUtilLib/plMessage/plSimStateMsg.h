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
// Messages that change physics-specific state directly.
// Compare to plSimInfluenceMsg, which changes movement-related state indirectly, via forces.
#ifndef PLSIMSTATEMSG_INC
#define PLSIMSTATEMSG_INC

#include "../pnMessage/plSimulationMsg.h"

// use a nil key to return to main world
// otherwise pass in the key of the world you're going to.
class plSubWorldMsg : public plSimulationMsg
{
public:
	plSubWorldMsg()
	{
		fWorldKey = nil;
	}
	plSubWorldMsg(const plKey &sender, const plKey &receiver, const plKey &worldKey)
		: plSimulationMsg(sender, receiver, 0)
	{
		fWorldKey = worldKey;
		SetBCastFlag(plMessage::kNetPropagate);
	}

	plKey fWorldKey;

	CLASSNAME_REGISTER(plSubWorldMsg);
	GETINTERFACE_ANY( plSubWorldMsg, plSimulationMsg);

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif // PLSIMSTATEMSG_INC
