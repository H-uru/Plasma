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
*   $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglFile.h
*   
***/

#ifdef PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLFILE_H
#error "Header $/Plasma20/Sources/Plasma/PubUtilLib/plNetGameLib/Private/plNglFile.h included more than once"
#endif
#define PLASMA20_SOURCES_PLASMA_PUBUTILLIB_PLNETGAMELIB_PRIVATE_PLNGLFILE_H


/*****************************************************************************
*
*   Client file functions
*
***/


//============================================================================
// Connect
//============================================================================
void NetCliFileStartConnect (
	const wchar *	fileAddrList[],
	unsigned		fileAddrCount,
	bool			isPatcher = false
);
void NetCliFileStartConnectAsServer (
	const wchar *	fileAddrList[],
	unsigned		fileAddrCount,
	unsigned		serverType,
	unsigned		serverBuildId
);

//============================================================================
// Disconnect
//============================================================================
void NetCliFileDisconnect ();

//============================================================================
// File server related messages
//============================================================================
typedef void (*FNetCliFileBuildIdRequestCallback)(
	ENetError		result,
	void *			param,
	unsigned		buildId
);
void NetCliFileBuildIdRequest (
	FNetCliFileBuildIdRequestCallback	callback,
	void *								param
);
typedef void (*FNetCliFileBuildIdUpdateCallback)(unsigned buildId);
void NetCliFileRegisterBuildIdUpdate (FNetCliFileBuildIdUpdateCallback callback);

//============================================================================
// Manifest
//============================================================================
struct NetCliFileManifestEntry {
	wchar		clientName[MAX_PATH]; // path and file on client side (for comparison)
	wchar		downloadName[MAX_PATH]; // path and file on server side (for download)
	wchar		md5[MAX_PATH];
	wchar		md5compressed[MAX_PATH]; // md5 for the compressed file
	unsigned	fileSize;
	unsigned	zipSize;
	unsigned	flags;
};
typedef void (*FNetCliFileManifestRequestCallback)(
	ENetError						result,
	void *							param,
	const wchar						group[],
	const NetCliFileManifestEntry	manifest[],
	unsigned						entryCount
);
void NetCliFileManifestRequest (
	FNetCliFileManifestRequestCallback	callback,
	void *								param,
	const wchar							group[], // the group of files you want (empty or nil = all)
	unsigned							buildId = 0 // 0 = get latest, other = get particular build (servers only)
);

//============================================================================
// File Download
//============================================================================
typedef void (*FNetCliFileDownloadRequestCallback)(
	ENetError       result,
	void *          param,
	const wchar     filename[],
	hsStream *      writer
);
void NetCliFileDownloadRequest (
	const wchar							filename[],
	hsStream *							writer,
	FNetCliFileDownloadRequestCallback	callback,
	void *								param,
	unsigned							buildId = 0 // 0 = get latest, other = get particular build (servers only)
);
