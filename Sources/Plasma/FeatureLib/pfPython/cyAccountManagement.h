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
#ifndef cyAccountManagement_h
#define cyAccountManagement_h

/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAccountManagement
//
// PURPOSE: Python wrapper for account management functions
//

#include <python.h>
#include "hsTypes.h"
#include "hsStlUtils.h"

class cyAccountManagement
{
public:
	static void			AddPlasmaMethods(std::vector<PyMethodDef> &methods);
	static void			AddPlasmaConstantsClasses(PyObject *m);

	static bool			IsSubscriptionActive();
	static PyObject*	GetPlayerList();
	static std::wstring	GetAccountName();
	static void			CreatePlayer(const char* playerName, const char* avatar, const char* invitationCode);
	static void			CreatePlayerW(const wchar_t* playerName, const wchar_t* avatar, const wchar_t* invitationCode);
	static void			DeletePlayer(unsigned playerId);
	static void			SetActivePlayer(unsigned playerId);
	static bool			IsActivePlayerSet();
	static void			UpgradeVisitorToExplorer(unsigned playerId);
	static void			ChangePassword(const char* password);
};


#endif
