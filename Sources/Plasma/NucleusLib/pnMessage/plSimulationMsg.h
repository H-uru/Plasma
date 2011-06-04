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
#ifndef PLSIMULATIONMSG_H
#define PLSIMULATIONMSG_H

#include "../pnMessage/plMessage.h"

// PLSIMULATIONMSG
// Virtual base class for all messages which are specific to the simulation interface
// (and the simulation pool objects)
// Most messages which are intended to go through the Simulation interface to the pool
// objects (without being explictly relayed by the Simulation interface) should subclass
// from this
class plSimulationMsg : public plMessage
{
public:
	// pass-through constructors
	plSimulationMsg() : plMessage() {};
	plSimulationMsg(const plKey &sender, const plKey &receiver, const double *time) : plMessage(sender, receiver, time) {};

CLASSNAME_REGISTER( plSimulationMsg );
	GETINTERFACE_ANY( plSimulationMsg, plMessage );

	virtual void Read(hsStream *stream, hsResMgr *mgr);
	virtual void Write(hsStream *stream, hsResMgr *mgr);
};

#endif // PLSIMULATIONMSG_H
