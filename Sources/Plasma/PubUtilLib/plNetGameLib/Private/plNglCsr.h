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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglCsr.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLCSR_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglCsr.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLCSR_H


/*****************************************************************************
*
*   Client-side CSR functions
*
***/

typedef void (*FNetCliCsrConnectedCallback) (
	void *		param,
	unsigned	latestBuildId
);
void NetCliCsrStartConnect (
	const wchar *				addrList[],
	unsigned					addrCount,
	FNetCliCsrConnectedCallback	callback = nil,
	void *						param = nil
);
void NetCliCsrDisconnect ();

typedef void (*FNetCliCsrLoginCallback)(
	ENetError		result,
	void *			param,
	const Uuid &	csrId,
	unsigned		csrFlags
);
void NetCliCsrLoginRequest (
	const wchar				csrName[],
	const ShaDigest &		namePassHash,
	FNetCliCsrLoginCallback	callback,
	void *					param
);

typedef void (*FNetCliCsrSetTicketFilterCallback)(
	ENetError		result,
	void *			param
);
void NetCliCsrSetTicketFilter (
	const wchar							filterSpec[],
	FNetCliCsrSetTicketFilterCallback	callback,
	void *								param
);

