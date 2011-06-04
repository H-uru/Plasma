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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbKeys.cpp
*   
***/

#include "../Pch.h"
#pragma hdrstop

// Auth Server
byte kAuthDhNData[kNetDiffieHellmanKeyBits / 8] = {0};
byte kAuthDhXData[kNetDiffieHellmanKeyBits / 8] = {0};

// CSR Server
byte kCsrDhNData[kNetDiffieHellmanKeyBits / 8] = {0};
byte kCsrDhXData[kNetDiffieHellmanKeyBits / 8] = {0};

// Game Server
byte kGameDhNData[kNetDiffieHellmanKeyBits / 8] = {0};
byte kGameDhXData[kNetDiffieHellmanKeyBits / 8] = {0};

// GateKeeper Server
byte kGateKeeperDhNData[kNetDiffieHellmanKeyBits / 8] = {0};
byte kGateKeeperDhXData[kNetDiffieHellmanKeyBits / 8] = {0};
