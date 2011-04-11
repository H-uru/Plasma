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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetLog/pnNlApi.cpp
*   
***/

#include "Pch.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private Data
*
***/

struct EventHash {
	unsigned	eventType;
	ESrvType	srvType;

    inline EventHash (
		unsigned	eventType,
		ESrvType	srvType
    );

    inline dword GetHash () const;
    inline bool operator== (const EventHash & rhs) const;
};

struct NetLogEventHash : EventHash {
	const NetLogEvent *				event;

	NetLogEventHash(
		unsigned	eventType,
		ESrvType	srvType,
		const NetLogEvent *event
	);
	HASHLINK(NetLogEventHash)	link;	
};

static CCritSect		s_critsect;
static ESrvType			s_srvType;
static HASHTABLEDECL(NetLogEventHash, EventHash, link) s_registeredEvents; 


/*****************************************************************************
*
*   NetLogEventHash
*
***/

//============================================================================
NetLogEventHash::NetLogEventHash (
	unsigned	eventType,
	ESrvType	srvType,
	const NetLogEvent *event
) : EventHash(eventType, srvType),
	event(event)
{
}


/*****************************************************************************
*
*   Event Hash
*
***/

//============================================================================
inline EventHash::EventHash (
    unsigned	eventType,
	ESrvType	srvType
) : eventType(eventType)
,   srvType(srvType)
{
}

//============================================================================
inline dword EventHash::GetHash () const {
    CHashValue hash(this, sizeof(*this));
    return hash.GetHash();
}

//============================================================================
inline bool EventHash::operator== (const EventHash & rhs) const {
    return  
		eventType == rhs.eventType &&
		srvType    == rhs.srvType;
}
/*****************************************************************************
*
*   Private Functions
*
***/

//============================================================================
static void NetLogUnRegisterEvents () {
	HASHTABLEDECL(NetLogEventHash, EventHash, link) tempHashTable;
	s_critsect.Enter();
	{
		while(NetLogEventHash *hash = s_registeredEvents.Head()) {
			tempHashTable.Add(hash);
		}
	}
	s_critsect.Leave();

	while(NetLogEventHash *hash = tempHashTable.Head()) {
		delete hash;
	}
}


/*****************************************************************************
*
*   Public Functions
*
***/

//============================================================================
void NetLogInitialize (ESrvType srvType) {
	s_srvType = srvType;
	if(s_srvType == kSrvTypeLog)
		NetLogSrvInitialize();
	else 
		NetLogCliInitialize(srvType);
}

//============================================================================
void NetLogShutdown () {
	if(s_srvType == kSrvTypeLog)
		NetLogSrvShutdown();
	else
		NetLogCliShutdown();
	NetLogUnRegisterEvents();	
}

//============================================================================
void NetLogDestroy () {
	if(s_srvType == kSrvTypeLog)
		NetLogSrvDestroy();
	else 
		NetLogCliDestroy();
}

//============================================================================
void NetLogRegisterEvents (const NetLogEvent events[], unsigned count) {
	NetLogEventHash *hash;
	HASHTABLEDECL(NetLogEventHash, EventHash, link) tempHashTable;

	for(unsigned i = 0; i < count; ++i) {
		hash = NEW(NetLogEventHash)(events[i].logEventType, events[i].srvType, &events[i]);
		tempHashTable.Add(hash);
	}
	s_critsect.Enter();
	{
		while(NetLogEventHash *hash = tempHashTable.Head()) {
			s_registeredEvents.Add(hash);
		}
	}
	s_critsect.Leave();
}

//============================================================================
const NetLogEvent *NetLogFindEvent (unsigned type, ESrvType srvType) {
	NetLogEventHash *hash;
	s_critsect.Enter();
	{
		hash = s_registeredEvents.Find(EventHash(type, srvType));
	}
	s_critsect.Leave();
	return hash ? hash->event : nil;
}

//============================================================================
void NetLogSendEvent (
	unsigned type
	...
) {
	const NetLogEvent *event = NetLogFindEvent(type, s_srvType);

	if(event) {
		va_list args;
		va_start(args, type);
		NetLogCliSendEvent(*event, args);
		va_end(args);
	}
	else {
		LogMsg( kLogError, "unable to log event, event not found SrvType: %d EventType: %d.", s_srvType, type);
	}
}
