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
*   $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2File/pnNpCli2File.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2FILE_PNNPCLI2FILE_H
#error "Header $/Plasma20/Sources/Plasma/NucleusLib/pnNetProtocol/Private/Protocols/Cli2File/pnNpCli2File.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_NUCLEUSLIB_PNNETPROTOCOL_PRIVATE_PROTOCOLS_CLI2FILE_PNNPCLI2FILE_H


// Because SrvFile must remain backward compatible with all client builds,
// the following enum values may never change under any circumstances.

// kNetProtocolCli2File messages
enum {
	// Global
	kCli2File_PingRequest				= 0,

	// File server-related
	kCli2File_BuildIdRequest			= 10,
	// 11 through 19 skipped

	// Cache-related
	kCli2File_ManifestRequest			= 20,
	kCli2File_FileDownloadRequest		= 21,
	kCli2File_ManifestEntryAck			= 22,
	kCli2File_FileDownloadChunkAck		= 23,
	// 24 through 29 skipped

	kCli2File_UNUSED_1					= 30,
};

enum {
	// Global
	kFile2Cli_PingReply					= 0,

	// File server-related
	kFile2Cli_BuildIdReply				= 10,
	kFile2Cli_BuildIdUpdate				= 11,
	// 12 through 19 skipped

	// Cache-related
	kFile2Cli_ManifestReply				= 20,
	kFile2Cli_FileDownloadReply			= 21,
	// 22 through 29 skipped

	kFile2Cli_UNUSED_1					= 30,
};

// we have a constant build id so all clients can connect to us, no matter what version
static const unsigned	kFileSrvBuildId = 0;


//============================================================================
// BEGIN PACKED DATA STRUCTURES
//============================================================================
#include <PshPack1.h>


/*****************************************************************************
*
*   Cli2File connect packet
*
***/

struct Cli2File_ConnData {
	dword		dataBytes;
	dword		buildId;
	unsigned	serverType;
};
struct Cli2File_Connect {
	AsyncSocketConnectPacket    hdr;
	Cli2File_ConnData           data;
};

struct Cli2File_MsgHeader {
	dword		messageBytes;
	dword       messageId;
};


/*****************************************************************************
*
*   Cli2File message structures
*
***/

// PingRequest
struct Cli2File_PingRequest : Cli2File_MsgHeader {
	dword       pingTimeMs;
};

// BuildIdRequest
struct Cli2File_BuildIdRequest : Cli2File_MsgHeader {
	dword		transId;
};

// ManifestRequest
struct Cli2File_ManifestRequest : Cli2File_MsgHeader {
	dword		transId;
	wchar		group[MAX_PATH];
	unsigned	buildId; // 0 = newest
};
struct Cli2File_ManifestEntryAck : Cli2File_MsgHeader {
	dword		transId;
	dword		readerId;
};

// FileDownloadRequest
struct Cli2File_FileDownloadRequest : Cli2File_MsgHeader {
	dword		transId;
	wchar		filename[MAX_PATH];
	unsigned	buildId; // 0 = newest
};
struct Cli2File_FileDownloadChunkAck : Cli2File_MsgHeader {
	dword		transId;
	dword		readerId;
};


/*****************************************************************************
*
*   File2Cli message structures
*
***/

// PingReply
struct File2Cli_PingReply : Cli2File_MsgHeader {
	dword       pingTimeMs;
};

// BuildIdReply
struct File2Cli_BuildIdReply : Cli2File_MsgHeader {
	dword		transId;
	ENetError	result;
	unsigned	buildId;
};

// BuildIdUpdate
struct File2Cli_BuildIdUpdate : Cli2File_MsgHeader {
	unsigned	buildId;
};

// ManifestReply
struct File2Cli_ManifestReply : Cli2File_MsgHeader {
	dword		transId;
	ENetError	result;
	dword		readerId;
	dword		numFiles;			// total number of files
	dword		wcharCount;			// size of the buffer
	wchar		manifestData[1];	// manifestData[wcharCount], actually
};

// FileDownloadReply
struct File2Cli_FileDownloadReply : Cli2File_MsgHeader {
	dword		transId;
	ENetError	result;
	dword		readerId;
	dword		totalFileSize;
	dword		byteCount;
	byte		fileData[1];		// fileData[byteCount], actually
};


//============================================================================
// END PACKED DATA STRUCTURES
//============================================================================
#include <PopPack.h>
