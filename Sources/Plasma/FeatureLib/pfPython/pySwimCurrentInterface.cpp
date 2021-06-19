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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "pyKey.h"

#include "pySwimCurrentInterface.h"
#include "plAvatar/plSwimRegion.h"

pySwimCurrentInterface::pySwimCurrentInterface(plKey key)
{
    fSwimCurrentKey = std::move(key);
}

pySwimCurrentInterface::pySwimCurrentInterface(pyKey& key)
{
    fSwimCurrentKey = key.getKey();
}

float pySwimCurrentInterface::getNearDist()
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

void pySwimCurrentInterface::setNearDist(float val)
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

float pySwimCurrentInterface::getFarDist()
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

void pySwimCurrentInterface::setFarDist(float val)
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

float pySwimCurrentInterface::getNearVel()
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

void pySwimCurrentInterface::setNearVel(float val)
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

float pySwimCurrentInterface::getFarVel()
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

void pySwimCurrentInterface::setFarVel(float val)
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

float pySwimCurrentInterface::getRotation()
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

void pySwimCurrentInterface::setRotation(float val)
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
