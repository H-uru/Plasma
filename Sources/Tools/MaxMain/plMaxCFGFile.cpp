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
#include "plMaxCFGFile.h"
#include "max.h"
#include "hsTypes.h"
#include "../plFile/plBrowseFolder.h"

const char *plMaxConfig::GetPluginIni()
{
	// Get the plugin CFG dir
	static char plugDir[MAX_PATH];
	strcpy(plugDir, GetCOREInterface()->GetDir(APP_PLUGCFG_DIR));
	strcat(plugDir, "\\PlasmaMAX2.ini");

	return plugDir;
}

const char *plMaxConfig::GetClientPath(bool getNew, bool quiet)
{
	static char plasmaPath[MAX_PATH];
	plasmaPath[0] = '\0';
	
	// Get the plugin CFG dir
	const char *plugDir = GetPluginIni();

	// Get the saved path
	UInt32 len = GetPrivateProfileString("SceneViewer", "Directory", "", plasmaPath, MAX_PATH, plugDir);

	// If we didn't find a path, or we want a new one, ask the user for one
	if ((len == 0 || getNew) && !quiet)
	{
		// If the user selects one, save it
		if (plBrowseFolder::GetFolder(plasmaPath, plasmaPath, "Specify your client folder"))
			WritePrivateProfileString("SceneViewer", "Directory", plasmaPath, plugDir);
	}

	// Return the path if we got one
	if (plasmaPath[0] != '\0')
	{
		// Make sure the path ends with a slash
		char lastChar = plasmaPath[strlen(plasmaPath)-1];
		if (lastChar != '/' && lastChar != '\\')
			strcat(plasmaPath, "\\");
		
		return plasmaPath;
	}

	return nil;
}

void plMaxConfig::SetClientPath(const char *path)
{
	const char *plugDir = GetPluginIni();
	WritePrivateProfileString("SceneViewer", "Directory", path, plugDir);
}

bool plMaxConfig::AssetManInterfaceDisabled()
{
	static bool inited = false;
	static bool disabled = false;

	if (!inited)
	{
		char configstr[MAX_PATH];
		configstr[0] = '\0';
		
		const char *plugDir = GetPluginIni();
		UInt32 len = GetPrivateProfileString("AssetMan", "Disable", "", configstr, MAX_PATH, plugDir);

		if (strcmp(configstr, "1") == 0 || stricmp(configstr, "true") == 0)
			disabled = true;
		else
			disabled = false;

		inited = true;
	}

	return disabled;
}
