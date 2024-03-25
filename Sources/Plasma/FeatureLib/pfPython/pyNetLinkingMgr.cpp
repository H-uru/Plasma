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


#ifdef BUILDING_PYPLASMA
# error "pyNetLinkingMgr is not compatible with pyPlasma.pyd. Use BUILDING_PYPLASMA macro to ifdef out unwanted headers."
#endif

#include "pyNetLinkingMgr.h"

#include <string_theory/string>

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvatarMgr.h"
#include "plNetClient/plNetLinkingMgr.h"

#include "pyAgeLinkStruct.h"

bool pyNetLinkingMgr::IsEnabled() const
{
    return plNetLinkingMgr::GetInstance()->IsEnabled();
}

void pyNetLinkingMgr::SetEnabled( bool b ) const
{
    plNetLinkingMgr::GetInstance()->SetEnabled( b );
}

void pyNetLinkingMgr::LinkToAge(pyAgeLinkStruct & link, const ST::string& linkAnim, bool linkInSfx, bool linkOutSfx)
{
    plNetLinkingMgr::GetInstance()->LinkToAge(link.GetAgeLink(), linkAnim, linkInSfx, linkOutSfx);
}

void pyNetLinkingMgr::LinkToMyPersonalAge()
{
    plNetLinkingMgr::GetInstance()->LinkToMyPersonalAge();
}

void pyNetLinkingMgr::LinkToMyPersonalAgeWithYeeshaBook()
{
    // use special avatar's open my personal book and link
    plArmatureMod *avatar = plAvatarMgr::GetInstance()->GetLocalAvatar();
    avatar->PersonalLink();
}

void pyNetLinkingMgr::LinkToMyNeighborhoodAge()
{
    plNetLinkingMgr::GetInstance()->LinkToMyNeighborhoodAge();
}

void pyNetLinkingMgr::LinkPlayerHere( uint32_t playerID )
{
    plNetLinkingMgr::GetInstance()->LinkPlayerHere( playerID );
}

void pyNetLinkingMgr::LinkPlayerToAge( pyAgeLinkStruct & link, uint32_t playerID )
{
    plNetLinkingMgr::GetInstance()->LinkPlayerToAge( link.GetAgeLink(), playerID );
}

void pyNetLinkingMgr::LinkToPlayersAge( uint32_t playerID )
{
    plNetLinkingMgr::GetInstance()->LinkToPlayersAge( playerID );
}

PyObject* pyNetLinkingMgr::GetCurrAgeLink()
{
    return pyAgeLinkStructRef::New( *plNetLinkingMgr::GetInstance()->GetAgeLink() );
}

PyObject* pyNetLinkingMgr::GetPrevAgeLink()
{
    return pyAgeLinkStructRef::New( *plNetLinkingMgr::GetInstance()->GetPrevAgeLink() );
}
