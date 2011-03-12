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
#include "plInstallerReg.h"
#include <windows.h>

static HKEY GetInstallerKey()
{
	HKEY hSoftKey = NULL;
	HKEY hCompanyKey = NULL;
	HKEY hAppKey = NULL;
	
	if(RegOpenKeyEx(HKEY_CURRENT_USER, "software", 0, KEY_WRITE|KEY_READ,
		&hSoftKey) == ERROR_SUCCESS)
	{
		DWORD dw;
		if(RegCreateKeyEx(hSoftKey, "Cyan", 0, REG_NONE,
			REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
			&hCompanyKey, &dw) == ERROR_SUCCESS)
		{
			RegCreateKeyEx(hCompanyKey, "PlasmaInstaller", 0, REG_NONE,
				REG_OPTION_NON_VOLATILE, KEY_WRITE|KEY_READ, NULL,
				&hAppKey, &dw);
		}
	}

	if (hSoftKey != NULL)
		RegCloseKey(hSoftKey);
	if (hCompanyKey != NULL)
		RegCloseKey(hCompanyKey);
	
	return hAppKey;
}

static void WriteRegString(const char* valueName, const char* value)
{
	HKEY installKey = GetInstallerKey();
	RegSetValueEx(installKey, valueName, 0, REG_SZ, (const BYTE*)value, strlen(value)+1);
	RegCloseKey(installKey);
}

static bool ReadRegString(const char* valueName, char* value, DWORD size)
{
	HKEY installKey = GetInstallerKey();
	bool ret = (RegQueryValueEx(installKey, valueName, NULL, NULL, (LPBYTE)value, &size) == ERROR_SUCCESS);
	RegCloseKey(installKey);
	return ret;
}

void plInstallerReg::SetClientDir(const char* dir)
{
	WriteRegString("Client", dir);
}

void plInstallerReg::SetMaxDir(const char* dir)
{
	WriteRegString("3dsmax", dir);
}

const char* plInstallerReg::GetClientDir()
{
	static char dir[MAX_PATH];
	if (!ReadRegString("Client", dir, sizeof(dir)))
		strcpy(dir, "C:\\PlasmaClient");
	return dir;
}

const char* plInstallerReg::GetMaxDir()
{
	static char dir[MAX_PATH];
	dir[0] = '\0';
	ReadRegString("3dsmax", dir, sizeof(dir));
	return dir;
}
