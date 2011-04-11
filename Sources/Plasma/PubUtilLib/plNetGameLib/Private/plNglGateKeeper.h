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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglGateKeeper.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLGATEKEEPER_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglGateKeeper.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLGATEKEEPER_H


/*****************************************************************************
*
*   Client gatekeeper functions
*
***/


//============================================================================
// Connect
//============================================================================
void NetCliGateKeeperStartConnect (
    const wchar *   gateKeeperAddrList[],
    unsigned        gateKeeperAddrCount
);

bool NetCliGateKeeperQueryConnected ();
void NetCliGateKeeperAutoReconnectEnable (bool enable);	// is enabled by default

// Called after the gatekeeper/client connection is encrypted
typedef void (*FNetCliGateKeeperConnectCallback)();
void NetCliGateKeeperSetConnectCallback (
	FNetCliGateKeeperConnectCallback callback
);


//============================================================================
// Disconnect
//============================================================================
void NetCliGateKeeperDisconnect ();
void NetCliGateKeeperUnexpectedDisconnect ();


//============================================================================
// Ping
//============================================================================
typedef void (*FNetCliGateKeeperPingRequestCallback)(
    ENetError   result,
    void *      param,
    unsigned    pingAtMs,
    unsigned	replyAtMs,
    unsigned	payloadbytes,
    const byte	payload[]
);
void NetCliGateKeeperPingRequest (
    unsigned								pingTimeMs,
    unsigned								payloadBytes,	// max 64k (pnNetCli enforced upon send)
    const void *							payload,
    FNetCliGateKeeperPingRequestCallback	callback,
    void *									param
);


//============================================================================
// FileSrvIpAddress
//============================================================================
typedef void (*FNetCliGateKeeperFileSrvIpAddressRequestCallback)(
    ENetError   result,
    void *      param,
	const wchar	addr[]
);

void NetCliGateKeeperFileSrvIpAddressRequest (
	FNetCliGateKeeperFileSrvIpAddressRequestCallback	callback,
    void *												param,
	bool												isPatcher
);


//============================================================================
// AuthSrvIpAddress
//============================================================================
typedef void (*FNetCliGateKeeperAuthSrvIpAddressRequestCallback)(
    ENetError   result,
    void *      param,
	const wchar	addr[]
);

void NetCliGateKeeperAuthSrvIpAddressRequest (
    FNetCliGateKeeperAuthSrvIpAddressRequestCallback	callback,
    void *												param
);



