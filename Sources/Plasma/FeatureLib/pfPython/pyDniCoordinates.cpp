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

#include "pyDniCoordinates.h"

#include "hsGeometry3.h"

#ifndef BUILDING_PYPLASMA
#include "plAvatar/plAvatarMgr.h"
#endif
#include "plVault/plDniCoordinateInfo.h"

pyDniCoordinates::pyDniCoordinates(plDniCoordinateInfo* coord)
{
    fCoords = new plDniCoordinateInfo;
    if (coord) {
        // copy their coords into our copy
        fCoords->SetTorans(coord->GetTorans());
        fCoords->SetHSpans(coord->GetHSpans());
        fCoords->SetVSpans(coord->GetVSpans());
    }
}


pyDniCoordinates::pyDniCoordinates()
{
    fCoords = new plDniCoordinateInfo;
    fCoords->SetTorans(0);
    fCoords->SetHSpans(0);
    fCoords->SetVSpans(0);
}

pyDniCoordinates::~pyDniCoordinates()
{
    if (fCoords)
        delete(fCoords);
}

int pyDniCoordinates::GetHSpans() const
{
    if ( fCoords )
    {
        return fCoords->GetHSpans();
    }
    return 0;
}

int pyDniCoordinates::GetVSpans() const
{
    if ( fCoords )
    {
        return fCoords->GetVSpans();
    }
    return 0;
}

int pyDniCoordinates::GetTorans() const
{
    if ( fCoords )
    {
        return fCoords->GetTorans();
    }
    return 0;
}

void pyDniCoordinates::UpdateCoordinates()
{
#ifndef BUILDING_PYPLASMA
    if (fCoords)
    {
        // reset GPS to 0,0,0 in case there is no maintainers marker in this age
        fCoords->SetTorans(0);
        fCoords->SetHSpans(0);
        fCoords->SetVSpans(0);
        plAvatarMgr::GetInstance()->GetDniCoordinate(fCoords);
    }
#endif
}

void pyDniCoordinates::FromPoint(const hsPoint3& pt)
{
#ifndef BUILDING_PYPLASMA
    if (fCoords)
    {
        // reset GPS to 0,0,0 in case there is no maintainers marker in this age
        fCoords->SetTorans(0);
        fCoords->SetHSpans(0);
        fCoords->SetVSpans(0);
        plAvatarMgr::GetInstance()->PointToDniCoordinate(pt, fCoords);
    }
#endif
}
