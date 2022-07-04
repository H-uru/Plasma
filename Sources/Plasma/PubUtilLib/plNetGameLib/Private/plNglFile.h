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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

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
    const ST::string  fileAddrList[],
    unsigned        fileAddrCount,
    bool            isPatcher = false
);

//============================================================================
// Disconnect
//============================================================================
void NetCliFileDisconnect ();

//============================================================================
// File server related messages
//============================================================================
typedef void (*FNetCliFileBuildIdRequestCallback)(
    ENetError       result,
    void *          param,
    unsigned        buildId
);
void NetCliFileBuildIdRequest (
    FNetCliFileBuildIdRequestCallback   callback,
    void *                              param
);
typedef void (*FNetCliFileBuildIdUpdateCallback)(unsigned buildId);
void NetCliFileRegisterBuildIdUpdate (FNetCliFileBuildIdUpdateCallback callback);

//============================================================================
// Manifest
//============================================================================
struct NetCliFileManifestEntry {
    char16_t    clientName[kNetDefaultStringSize]; // path and file on client side (for comparison)
    char16_t    downloadName[kNetDefaultStringSize]; // path and file on server side (for download)
    char16_t    md5[32];
    char16_t    md5compressed[32]; // md5 for the compressed file
    unsigned    fileSize;
    unsigned    zipSize;
    unsigned    flags;
};
typedef void (*FNetCliFileManifestRequestCallback)(
    ENetError                       result,
    void *                          param,
    const char16_t                  group[],
    const NetCliFileManifestEntry   manifest[],
    unsigned                        entryCount
);
void NetCliFileManifestRequest (
    FNetCliFileManifestRequestCallback  callback,
    void *                              param,
    const char16_t                      group[], // the group of files you want (empty or nil = all)
    unsigned                            buildId = 0 // 0 = get latest, other = get particular build (servers only)
);

//============================================================================
// File Download
//============================================================================
typedef void (*FNetCliFileDownloadRequestCallback)(
    ENetError           result,
    void *              param,
    const plFileName &  filename,
    hsStream *          writer
);
void NetCliFileDownloadRequest (
    const plFileName &                  filename,
    hsStream *                          writer,
    FNetCliFileDownloadRequestCallback  callback,
    void *                              param,
    unsigned                            buildId = 0 // 0 = get latest, other = get particular build (servers only)
);
