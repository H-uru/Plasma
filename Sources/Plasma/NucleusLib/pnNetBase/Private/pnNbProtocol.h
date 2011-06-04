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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbProtocol.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBPROTOCOL_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbProtocol.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBPROTOCOL_H


/*****************************************************************************
*
*   Net protocols
*
***/

const unsigned kNetProtocolServerBit = 0x80;

// These codes may not be changed unless ALL servers and clients are
// simultaneously replaced; so basically forget it =)
enum ENetProtocol {
	kNetProtocolNil                 = 0,

	// For test applications
	kNetProtocolDebug               = 1,
	
	// Client connections
	kNetProtocolCli2GateKeeper		= 2,
	kNetProtocolCli2Csr				= 3,
	kNetProtocolCli2Auth            = 4,
	kNetProtocolCli2Game            = 5,
	kNetProtocolCli2File			= 6,
	kNetProtocolCli2Unused_01		= 7,

	// Server connections
	kNetProtocolSrvConn				= 0 | kNetProtocolServerBit,
	kNetProtocolSrv2Mcp             = 1 | kNetProtocolServerBit,
	kNetProtocolSrv2Vault           = 2 | kNetProtocolServerBit,
	kNetProtocolSrv2Db              = 3 | kNetProtocolServerBit,
	kNetProtocolSrv2State			= 4 | kNetProtocolServerBit,
	kNetProtocolSrv2Log				= 5 | kNetProtocolServerBit,
	kNetProtocolSrv2Score			= 6 | kNetProtocolServerBit,
};

// NOTE: When adding a new net protocol, be sure to update
// NetProtocolToString as well.  Unfortunately, the compiler
// cannot enforce this since the protocol values are not
// numerically sequential.
const wchar * NetProtocolToString (ENetProtocol protocol);
