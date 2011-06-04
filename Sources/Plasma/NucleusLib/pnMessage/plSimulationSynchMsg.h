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
#ifndef PLSIMULATIONSYNCHMSG_h
#define PLSIMULATIONSYNCHMSG_h

#include "plMessage.h"

// PLSIMULATIONSYNCHMSG
// periodically sent from one simulated object to another to ensure consistent state
// this is a completely virtual message type
// more meaningful sub-classes are created by specific physics systems
class plSimulationSynchMsg : public plMessage
{
public:

	// ???
	// This message is not really creatable: it's abstract. It's designed to sneak
	// havok-specific data through the generalized simulation logic
	CLASSNAME_REGISTER( plSimulationSynchMsg );
	GETINTERFACE_ANY( plSimulationSynchMsg, plMessage );

	// Don't be fooled: this class is *not* to be instantiated.

	void Read(hsStream *stream, hsResMgr *mgr);
	void Write(hsStream *stream, hsResMgr *mgr);
};


inline void plSimulationSynchMsg::Read(hsStream *s, hsResMgr *mgr)
{
	hsAssert(false, "plSimulationSynchMsg should never be instantiated directly");
}

inline void plSimulationSynchMsg::Write(hsStream *s, hsResMgr *mgr)
{
	hsAssert(false, "plSimulationSynchMsg should never be instantiated directly");
}

#endif	// PLSIMULATIONSYNCHMSG_h
