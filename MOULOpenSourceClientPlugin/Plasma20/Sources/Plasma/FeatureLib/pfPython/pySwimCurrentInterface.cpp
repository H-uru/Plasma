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
#include "pySwimCurrentInterface.h"
#include "../plAvatar/plSwimRegion.h"

pySwimCurrentInterface::pySwimCurrentInterface(plKey key)
{
	fSwimCurrentKey = key;
}

pySwimCurrentInterface::pySwimCurrentInterface(pyKey& key)
{
	fSwimCurrentKey = key.getKey();
}

hsScalar pySwimCurrentInterface::getNearDist()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		return circ->fPullNearDistSq;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		return straight->fNearDist;
	}
	else
	{
		return 0;
	}
}

void pySwimCurrentInterface::setNearDist(hsScalar val)
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		circ->fPullNearDistSq = val;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		straight->fNearDist = val;
	}
}

hsScalar pySwimCurrentInterface::getFarDist()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		return circ->fPullFarDistSq;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		return straight->fFarDist;
	}
	else
	{
		return 0;
	}
}

void pySwimCurrentInterface::setFarDist(hsScalar val)
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		circ->fPullFarDistSq = val;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		straight->fFarDist = val;
	}
}

hsScalar pySwimCurrentInterface::getNearVel()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		return circ->fPullNearVel;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		return straight->fNearVel;
	}
	else
	{
		return 0;
	}
}

void pySwimCurrentInterface::setNearVel(hsScalar val)
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		circ->fPullNearVel = val;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		straight->fNearVel = val;
	}
}

hsScalar pySwimCurrentInterface::getFarVel()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		return circ->fPullFarVel;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		return straight->fFarVel;
	}
	else
	{
		return 0;
	}
}

void pySwimCurrentInterface::setFarVel(hsScalar val)
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		circ->fPullFarVel = val;
	}
	else if (plSwimStraightCurrentRegion* straight = plSwimStraightCurrentRegion::ConvertNoRef(obj))
	{
		straight->fFarVel = val;
	}
}

hsScalar pySwimCurrentInterface::getRotation()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		return circ->fRotation;
	}
	else
	{
		return 0;
	}
}

void pySwimCurrentInterface::setRotation(hsScalar val)
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimCircularCurrentRegion* circ = plSwimCircularCurrentRegion::ConvertNoRef(obj))
	{
		circ->fRotation = val;
	}
}

void pySwimCurrentInterface::enable()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimRegionInterface* regInt = plSwimRegionInterface::ConvertNoRef(obj))
	{
		regInt->SetProperty(plSwimRegionInterface::kDisable, 0);
	}
}

void pySwimCurrentInterface::disable()
{
	hsKeyedObject* obj = fSwimCurrentKey->ObjectIsLoaded();

	if (plSwimRegionInterface* regInt = plSwimRegionInterface::ConvertNoRef(obj))
	{
		regInt->SetProperty(plSwimRegionInterface::kDisable, 1);
	}
}