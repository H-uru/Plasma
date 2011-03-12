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
namespace plMaxConfig
{
	// Get the full path to the ini file to write settings to
	const char *GetPluginIni();

	// Gets the path to the Plasma working directory
	// If the user hasn't set one before, it prompts them to
	// Set getNew to true to force the user to set a new path
	// If a path is returned, it will end with a slash
	const char *GetClientPath(bool getNew=false, bool quiet=false);
	// For the rare case where you need to set the client path manually
	void SetClientPath(const char *path);

	// option to disable the plugin's assetman interface
	bool AssetManInterfaceDisabled();
}
