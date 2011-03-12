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
#include "plComponentExt.h"
#include "../pnKeyedObject/plKey.h"

#define RESPONDER_CID Class_ID(0x46b83f3e, 0x7d5e5d17)

class plResponderComponentExt : public plComponentExt
{
public:
	// All classes derived from plResponderComponent can be picked from the activator component,
	// because they can convert to the responder type.
	int CanConvertToType(Class_ID obtype)
	{ return (obtype == RESPONDER_CID) ? 1 : plComponentExt::CanConvertToType(obtype); }
};

int ResponderGetActivatorCount(plComponentBase *comp);
plComponentBase *ResponderGetActivator(plComponentBase *comp, int idx);

namespace Responder
{
	// Pass in a responder component and a node it is attached to, and you will
	// get the key to the responder modifier
	plKey GetKey(plComponentBase *comp, plMaxNodeBase *node);
}
