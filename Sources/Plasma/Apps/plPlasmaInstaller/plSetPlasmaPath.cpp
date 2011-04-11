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
#include "HeadSpin.h"
#include <windows.h>

static HKEY GetEnvironKey()
{
	HKEY hSystemKey = NULL;
	HKEY hControlSetKey = NULL;
	HKEY hControlKey = NULL;
	HKEY hSessionKey = NULL;
	HKEY hEnvironKey = NULL;

	if ((RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM", 0, KEY_READ, &hSystemKey) == ERROR_SUCCESS) &&
		(RegOpenKeyEx(hSystemKey, "CurrentControlSet", 0, KEY_READ, &hControlSetKey) == ERROR_SUCCESS) &&
		(RegOpenKeyEx(hControlSetKey, "Control", 0, KEY_READ, &hControlKey) == ERROR_SUCCESS) &&
		(RegOpenKeyEx(hControlKey, "Session Manager", 0, KEY_READ, &hSessionKey) == ERROR_SUCCESS))
	{
		RegOpenKeyEx(hSessionKey, "Environment", 0, KEY_READ | KEY_WRITE, &hEnvironKey);
	}

	if (hSystemKey != NULL)
		RegCloseKey(hSystemKey);
	if (hControlSetKey != NULL)
		RegCloseKey(hControlSetKey);
	if (hControlKey != NULL)
		RegCloseKey(hControlKey);
	if (hSessionKey != NULL)
		RegCloseKey(hSessionKey);

	return hEnvironKey;
}

void SetPlasmaPath(const char* plasmaPath)
{
	bool pathSet = false;

	HKEY hEnvironKey = GetEnvironKey();
	if (hEnvironKey)
	{
		// Make sure the PlasmaGameDir var is in the path
		DWORD size = 0;
		if (ERROR_SUCCESS == RegQueryValueEx(hEnvironKey, "Path", NULL, NULL, NULL, &size))
		{
			char* oldPath = new char[size];
			static const char* kPlasmaVar = "%PlasmaGameDir%";

			if (ERROR_SUCCESS == RegQueryValueEx(hEnvironKey, "Path", NULL, NULL, (BYTE*)oldPath, &size))
			{
				pathSet = (strstr(oldPath, kPlasmaVar) != NULL);

				if (!pathSet)
				{
					char* newPath = new char[size+strlen(kPlasmaVar)+1];
					strcpy(newPath, oldPath);
					strcat(newPath, ";");
					strcat(newPath, kPlasmaVar);

					RegSetValueEx(hEnvironKey, "Path", 0, REG_EXPAND_SZ, (BYTE*)newPath, strlen(newPath)+1);

					delete [] newPath;
				}
			}

			delete [] oldPath;
		}

		// Set the PlasmaGameDir var
		RegSetValueEx(hEnvironKey, "PlasmaGameDir", 0, REG_SZ, (BYTE*)plasmaPath, strlen(plasmaPath)+1);

		// Notify command prompts and stuff that environ changed
		DWORD ret;
		SendMessageTimeout(HWND_BROADCAST,
							WM_SETTINGCHANGE,
							0,
							(LPARAM)"Environment",
							SMTO_ABORTIFHUNG,
							5000,
							&ret);
	}
}
