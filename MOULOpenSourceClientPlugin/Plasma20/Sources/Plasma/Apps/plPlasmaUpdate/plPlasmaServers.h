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
#ifndef plPlasmaServers_h_inc
#define plPlasmaServers_h_inc

#include "hsTypes.h"
#include <vector>

class plPlasmaServers
{
protected:
	class ServerInfo
	{
	public:
		std::string fServerAddress;
		std::string fServerName;
		std::string fURLBase;
		std::string fOutputDir;
		std::string fCurrentDir;
		std::string fCodeDir;
	};
	std::vector<ServerInfo> fServers;

public:
	bool GetServerInfo();

	int GetNumServers() { return fServers.size(); }
	std::string& GetServerAddress(int i) { return fServers[i].fServerAddress; }
	std::string& GetServerName(int i) { return fServers[i].fServerName; }
	std::string& GetServerURLBase(int i) { return fServers[i].fURLBase; }
	std::string& GetServerOutputDir(int i) { return fServers[i].fOutputDir; }
	std::string& GetServerCurrentDir(int i) { return fServers[i].fCurrentDir; }
	std::string& GetServerCodeDir(int i) { return fServers[i].fCodeDir; }
};

#endif // plPlasmaServers_h_inc
