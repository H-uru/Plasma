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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbKeys.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBKEYS_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbKeys.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBKEYS_H

// Auth Server
static const unsigned kAuthDhGValue = 41;
extern byte kAuthDhNData[kNetDiffieHellmanKeyBits / 8];
extern byte kAuthDhXData[kNetDiffieHellmanKeyBits / 8];

// CSR Server
static const unsigned kCsrDhGValue = 97;
extern byte kCsrDhNData[kNetDiffieHellmanKeyBits / 8];
extern byte kCsrDhXData[kNetDiffieHellmanKeyBits / 8];

// Game Server
static const unsigned kGameDhGValue = 73;
extern byte kGameDhNData[kNetDiffieHellmanKeyBits / 8];
extern byte kGameDhXData[kNetDiffieHellmanKeyBits / 8];

// GateKeeper Server
static const unsigned kGateKeeperDhGValue = 4;
extern byte kGateKeeperDhNData[kNetDiffieHellmanKeyBits / 8];
extern byte kGateKeeperDhXData[kNetDiffieHellmanKeyBits / 8];
