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
#include "plPlasmaServers.h"
#include "hsStream.h"
#include "hsUtils.h"

bool plPlasmaServers::GetServerInfo()
{
	bool ret = true;

	hsUNIXStream si;
	if (si.Open("\\\\dirtcake\\ServerInfo\\ServerInfo.txt", "rb"))
	{
		char line[256];

		// Make sure we've got the latest version
		if (si.ReadLn(line, sizeof(line)))
		{
			int version = atoi(line);
			si.ReadLn(line, sizeof(line));
			if (version != 4)
			{
				char errorMsg[512];
				sprintf(errorMsg, "This installer is out of date.\nPlease get the latest version from:\n\n%s", line);
				hsMessageBox(errorMsg, "Error", hsMessageBoxNormal, hsMessageBoxIconError);
				ret = false;
			}
		}
		else
			ret = false;

		// Read in the servers, one per line
		while (ret && si.ReadLn(line, sizeof(line)))
		{
			ServerInfo info;

			info.fServerAddress = strtok(line, ",");
			info.fServerName = strtok(nil, ",");
			info.fURLBase = strtok(nil, ",");
			info.fOutputDir = strtok(nil, ",");
			info.fCurrentDir = strtok(nil, ",");
			info.fCodeDir = strtok(nil, ",");

			fServers.push_back(info);
		}

		si.Close();
	}
	else
	{
		hsMessageBox("Couldn't find server info", "Error", hsMessageBoxNormal, hsMessageBoxIconError);
		ret = false;
	}

	return ret;
}
