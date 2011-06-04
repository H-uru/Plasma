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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetClient/plNetCliAgeLeaver.h
*   
***/

#ifndef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENT_PLNETCLIAGELEAVER_H
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENT_PLNETCLIAGELEAVER_H


#include "HeadSpin.h"
#include "../pnUtils/pnUtils.h"
#include "../pnNetBase/pnNetBase.h"


/*****************************************************************************
*
*   
*
***/

class plMessage;
struct plNCAgeLeaver;

enum ENCAgeLeaverNotify {
	kAgeLeaveComplete,		// notify --> NCAgeLeaveCompleteNotify *, after callback, leaver is destroyed
	kNumAgeLeaverNotifications
};

struct NCAgeLeaveCompleteNotify {
	bool			success;
	const char *	msg;
};


typedef void (* FNCAgeLeaverCallback)(
	plNCAgeLeaver *			leaver,
	unsigned				type,		// ENCAgeLeaverNotify
	void *					notify,
	void *					userState
);


void NCAgeLeaverCreate (
	plNCAgeLeaver **		leaver,
	bool					quitting,
	FNCAgeLeaverCallback	callback,
	void *					userState
);
bool NCAgeLeaverMsgReceive (	// returns true of message was processed
	plNCAgeLeaver *			leaver,
	plMessage *				msg
);
void NCAgeLeaverUpdate (
	plNCAgeLeaver *			leaver
);


#endif // PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETCLIENT_PLNETCLIAGELEAVER_H
