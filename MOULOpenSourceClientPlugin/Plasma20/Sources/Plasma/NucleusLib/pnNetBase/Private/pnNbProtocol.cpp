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
/*****************************************************************************
*
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbProtocol.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Exports
*
***/

//============================================================================
const wchar * NetProtocolToString (ENetProtocol protocol) {

	#define PROTOCOL_STRING(p)	{ p, L#p }
	static struct { ENetProtocol protocol; const wchar *name; } s_protocols[] = {
		PROTOCOL_STRING(kNetProtocolNil),

		// For test applications
		PROTOCOL_STRING(kNetProtocolDebug),

		// Client connections
		{ kNetProtocolCli2Csr,  L"GateKeeper Server" },
		{ kNetProtocolCli2Csr,  L"Csr Server" },
		{ kNetProtocolCli2Auth, L"Auth Server" },
		{ kNetProtocolCli2Game, L"Game Server" },
		{ kNetProtocolCli2File, L"File Server" },
		PROTOCOL_STRING(kNetProtocolCli2Unused_01),

		// Server connections
		PROTOCOL_STRING(kNetProtocolSrvConn),
		PROTOCOL_STRING(kNetProtocolSrv2Mcp),
		PROTOCOL_STRING(kNetProtocolSrv2Vault),
		PROTOCOL_STRING(kNetProtocolSrv2Db),
		PROTOCOL_STRING(kNetProtocolSrv2State),
		PROTOCOL_STRING(kNetProtocolSrv2Log),
		PROTOCOL_STRING(kNetProtocolSrv2Score),
	};
	#undef PROTOCOL_STRING

	for (unsigned i = 0; i < arrsize(s_protocols); ++i)
		if (s_protocols[i].protocol == protocol)
			return s_protocols[i].name;

	return L"Unknown protocol id";	
}
