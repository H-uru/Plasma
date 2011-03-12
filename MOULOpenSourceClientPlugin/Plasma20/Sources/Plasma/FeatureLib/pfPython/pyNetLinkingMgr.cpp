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

#ifdef BUILDING_PYPLASMA
# error "pyNetLinkingMgr is not compatible with pyPlasma.pyd. Use BUILDING_PYPLASMA macro to ifdef out unwanted headers."
#endif

#include "hsStlUtils.h"

#include "pyNetLinkingMgr.h"
#include "../plNetClient/plNetLinkingMgr.h"
#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"

#include "pyAgeInfoStruct.h"
#include "pyAgeLinkStruct.h"

hsBool pyNetLinkingMgr::IsEnabled( void ) const
{
	return plNetLinkingMgr::GetInstance()->IsEnabled();
}

void pyNetLinkingMgr::SetEnabled( hsBool b )
{
	plNetLinkingMgr::GetInstance()->SetEnabled( b?true:false );
}

void pyNetLinkingMgr::LinkToAge( pyAgeLinkStruct & link, const char* linkAnim )
{
	plNetLinkingMgr::GetInstance()->LinkToAge( link.GetAgeLink(), linkAnim );
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

void pyNetLinkingMgr::LinkPlayerHere( UInt32 playerID )
{
	plNetLinkingMgr::GetInstance()->LinkPlayerHere( playerID );
}

void pyNetLinkingMgr::LinkPlayerToAge( pyAgeLinkStruct & link, UInt32 playerID )
{
	plNetLinkingMgr::GetInstance()->LinkPlayerToAge( link.GetAgeLink(), playerID );
}

void pyNetLinkingMgr::LinkToPlayersAge( UInt32 playerID )
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
