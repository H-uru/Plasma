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
#ifndef PL_NET_SERVERS_H
#define PL_NET_SERVERS_H

//
//  Server Declarations and Constants
//

#include "hsTypes.h"

//
// Windows Class Name for All Servers
//
#define PARABLE_WINCLASSNAME		"ParableServer"
#define PARABLE_PLS_WINCLASSNAME	"ParableServer_PLS"


class plNetServerConstants
{
public:
	enum ServerTypes  // Changing the order WILL affect Data including Databases!!!
	{				
		kInvalidLo,
		kAgent,
		kLobby,
		kGame,
		kVault,
		kAuth,
		kAdmin,
		kLookup,

		kClient,
		kInvalidHi,
	};
private:

	static const char* ServerPrograms[];

public:
	static const char* GetServerExe(int type) { return (type > kInvalidLo && type < kInvalidHi)?ServerPrograms[type-1]:nil; }
	static const char* GetServerName(int type);
	static UInt16 GetPort(int type);
	static const char* GetServerTypeStr(int type)
	{
		switch(type)
		{
		case kAgent:	return "kAgent";
		case kLobby:	return "kLobby";
		case kGame:		return "kGame";
		case kVault:	return "kVault";
		case kAuth:		return "kAuth";
		case kAdmin:	return "kAdmin";
		case kLookup:	return "kLookup";
		case kClient:	return "kClient";
		default: return "???";
		}
	}
};


class plNetServerAgentConstants
{
public:
	static const char* GetName() { return "Server_Agent"; }
	static const UInt16 GetPort() { return 4800; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kAgent; }
};


class plNetLookupServerConstants
{
public:
	static const char* GetName() { return "Lookup_Server"; }
	static const UInt16 GetPort() { return 2000; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kLookup; }
};


class plNetLobbyServerConstants
{
public:
	static const char* GetName() { return "Generated_Lobby"; }
	static const UInt16 GetPort() { return 5000; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kLobby; }
};


class plNetVaultServerConstants
{
public:
	static const char* GetName() { return "Vault_Server"; }
	static const UInt16 GetPort() { return 2001; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kVault; }
};


class plNetAuthServerConstants
{
public:
	static const char* GetName() { return "Auth_Server"; }
	static const UInt16 GetPort() { return 2002; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kAuth; }
};


class plNetAdminServerConstants
{
public:
	static const char* GetName() { return "Admin_Server"; }
	static const UInt16 GetPort() { return 2003; }
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kAdmin; }
};

class plNetGameServerConstants
{
public:
	static const char* GetName() { return "Game_Server"; }
	static const UInt16 GetLowPort() { return 5001;}
	static const UInt16 GetHighPort() { return 6001;}
	static const plNetServerConstants::ServerTypes GetType() { return plNetServerConstants::kGame; }
};


#endif //PL_NET_SERVERS_H
