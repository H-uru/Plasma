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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbEchoMsgs.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBECHOMSGS_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetBase/Private/pnNbEchoMsgs.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETBASE_PRIVATE_PNNBECHOMSGS_H


//============================================================================
// Begin Echo server data types
//============================================================================
namespace Echo {


/*****************************************************************************
*
*   @@@: Shared data types for echo server/client demo
*
***/


const unsigned kEchoMsgFlagBroadcast	= 1<<0;
const unsigned kEchoMsgFlagEchoBack		= 1<<1;


enum EEchoNetBufferType {
	kEchoNetBufferInvalidType,
	kEchoNetBufferMsg,
	kNumEchoNetBufferTypes
};

enum EEchoMsgType {
	kEchoInvalidMsgType,
	kEchoJoinRequestMsg,
	kEchoJoinReplyMsg,
	kEchoPlayerJoinedMsg,
	kEchoPlayerLeftMsg,
	kEchoChatMsg,
	kEchoGameDataBufferMsg,
	kNumEchoMsgTypes
};
COMPILER_ASSERT(kNumEchoMsgTypes < (byte)-1);


//============================================================================
// Begin packed data structures
//============================================================================
#include <pshpack1.h>

struct EchoMsg {
	EchoMsg (const EEchoMsgType & type, unsigned flags = 0)
	:	type((byte)type)
	,	flags(flags)
	{ }
	
	byte					type;
	dword					flags;
};

struct EchoJoinRequestMsg : EchoMsg {
	EchoJoinRequestMsg ()
	:	EchoMsg(kEchoJoinRequestMsg)
	{ }

	wchar					playerName[64];
};

struct EchoJoinReplyMsg : EchoMsg {
	EchoJoinReplyMsg ()
	:	EchoMsg(kEchoJoinReplyMsg)
	{ }
	
	dword					playerId;
};

struct EchoPlayerJoinedMsg : EchoMsg {
	EchoPlayerJoinedMsg ()
	:	EchoMsg(kEchoPlayerJoinedMsg, kEchoMsgFlagBroadcast)
	{ }
	
	dword					playerId;
	wchar					playerName[64];
};

struct EchoPlayerLeftMsg : EchoMsg {
	EchoPlayerLeftMsg ()
	:	EchoMsg(kEchoPlayerLeftMsg, kEchoMsgFlagBroadcast)
	{ }

	dword					playerId;
};

struct EchoChatMsg : EchoMsg {
	EchoChatMsg ()
	:	EchoMsg(kEchoChatMsg, kEchoMsgFlagBroadcast|kEchoMsgFlagEchoBack)
	{ }
	
	dword					fromPlayerId;
	dword					msgChars;
	wchar					msgBuffer[1];	// [msgChars], actually
	// no more fields after variable length allocation
};

struct EchoGameDataBufferMsg : EchoMsg {
	EchoGameDataBufferMsg ()
	:	EchoMsg(kEchoGameDataBufferMsg, kEchoMsgFlagBroadcast|kEchoMsgFlagEchoBack)
	{ }

	dword					fromPlayerId;
	dword					bufferBytes;
	byte					bufferData[1];	// [bufferBytes], actually
	// no more fields after variable length allocation
};

//============================================================================
// End packed data structures
//============================================================================
#include <poppack.h>


//============================================================================
// End Echo server data types
//============================================================================
} using namespace Echo;
