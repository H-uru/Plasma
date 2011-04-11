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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbSrvs.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBSRVS_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbSrvs.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBSRVS_H


/*****************************************************************************
*
*   Server types
*
***/

// These codes may not be changed unless ALL servers and clients are
// simultaneously replaced; so basically forget it =)
enum ESrvType {
    kSrvTypeNone        = 0,

    kSrvTypeClient      = 1,
    kSrvTypeAuth        = 2,
    kSrvTypeGame        = 3,
    kSrvTypeVault       = 4,
    kSrvTypeDb          = 5,
    kSrvTypeMcp         = 6,
    kSrvTypeState		= 7,
    kSrvTypeFile		= 8,
    kSrvTypeLog			= 9,
    kSrvTypeDll			= 10,
	kSrvTypeScore		= 11,
    kSrvTypeCsr			= 12,
	kSrvTypeGateKeeper  = 13,

    kNumSrvTypes,

    // Enforce network message field size
    kNetSrvForceDword = (unsigned)-1
};


/*****************************************************************************
*
*   Front-end server hostnames
*
***/

unsigned GetAuthSrvHostnames (const wchar *** addrs);	// returns addrCount
void SetAuthSrvHostname (const wchar addr[]);
bool AuthSrvHostnameOverride ();

unsigned GetFileSrvHostnames (const wchar *** addrs);	// returns addrCount
void SetFileSrvHostname (const wchar addr[]);
bool FileSrvHostnameOverride ();

unsigned GetCsrSrvHostnames (const wchar *** addrs);	// returns addrCount
void SetCsrSrvHostname (const wchar addr[]);
bool CsrSrvHostnameOverride ();

unsigned GetGateKeeperSrvHostnames (const wchar *** addrs);	// returns addrCount
void SetGateKeeperSrvHostname (const wchar addr[]);
bool GateKeeperSrvHostnameOverride ();
