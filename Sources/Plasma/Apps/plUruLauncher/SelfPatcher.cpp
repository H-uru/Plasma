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
*   $/Plasma20/Sources/Plasma/Apps/plUruLauncher/SelfPatcher.cpp
*   
***/

#include "Pch.h"
#include "plStatusLog/plStatusLog.h"
#pragma hdrstop


/*****************************************************************************
*
*   Private Data
*
***/

#ifndef PLASMA_EXTERNAL_RELEASE
    static const wchar_t s_manifest[] = L"InternalPatcher";
#else
    static const wchar_t s_manifest[] = L"ExternalPatcher";
#endif

class SelfPatcherStream : public plZlibStream {
    public:
        virtual uint32_t  Write(uint32_t byteCount, const void* buffer);
        static plLauncherInfo *info;
        static unsigned totalBytes;
        static unsigned progress;
};

unsigned SelfPatcherStream::totalBytes = 0;
unsigned SelfPatcherStream::progress = 0;

static bool         s_downloadComplete;
static long         s_numFiles;
static ENetError    s_patchResult;
static bool         s_updated;
static wchar_t        s_newPatcherFile[MAX_PATH];


/*****************************************************************************
*
*   Private Functions
*
***/

//============================================================================
static void NetErrorHandler (ENetProtocol protocol, ENetError error) {
    plString msg = plString::Format("NetErr: %S", NetErrorToString(error));
    plStatusLog::AddLineS("patcher.log", msg.c_str());

    if (IS_NET_SUCCESS(s_patchResult))
        s_patchResult = error;
    s_downloadComplete = true;

    switch(error) {
        case kNetErrServerBusy:
            MessageBox(0, "Due to the high demand, the server is currently busy. Please try again later, or for alternative download options visit: http://www.mystonline.com/play/", "UruLauncher", MB_OK);
            s_patchResult = kNetErrServerBusy;
            s_downloadComplete = true;
        break;
    }
}

//============================================================================
static void DownloadCallback (
    ENetError       result,
    void *          param,
    const wchar_t     filename[],
    hsStream *      writer
) {
    if(IS_NET_ERROR(result)) {
        switch (result) {
            case kNetErrTimeout:
                writer->Rewind();
                NetCliFileDownloadRequest(filename, writer, DownloadCallback, param);
            break;
            
            default:
                plString msg = plString::Format("Error getting patcher file: %S", NetErrorToString(result));
                plStatusLog::AddLineS("patcher.log", msg.c_str());

                if (IS_NET_SUCCESS(s_patchResult))
                    s_patchResult = result;
            break;
        }
        return;
    }

    writer->Close();
    delete writer;
    AtomicAdd(&s_numFiles, -1);

    if(!s_numFiles) {
        s_downloadComplete = true;
        s_updated = true;
    }
}

//============================================================================
static bool MD5Check (const char filename[], const wchar_t md5[]) {
    // Do md5 check
    char md5copy[MAX_PATH];
    plMD5Checksum existingMD5(filename);
    plMD5Checksum latestMD5;

    StrToAnsi(md5copy, md5, arrsize(md5copy));
    latestMD5.SetFromHexString(md5copy);
    return (existingMD5 == latestMD5);
}

//============================================================================
static void ManifestCallback (
    ENetError                       result,
    void *                          param,
    const wchar_t                     group[],
    const NetCliFileManifestEntry   manifest[],
    unsigned                        entryCount
) {
    if(IS_NET_ERROR(result)) {
        switch (result) {
            case kNetErrTimeout:
                NetCliFileManifestRequest(ManifestCallback, nil, s_manifest);
            break;
            
            default:
                plString msg = plString::Format("Error getting patcher manifest: %S", NetErrorToString(result));
                plStatusLog::AddLineS("patcher.log", msg.c_str());

                if (IS_NET_SUCCESS(s_patchResult))
                    s_patchResult = result;
            break;
        }
        return;
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    if (entryCount == 0)  { // dataserver does not contain a patcher
        s_downloadComplete = true;
        return;
    }
#endif

    char ansi[MAX_PATH];

    // MD5 check current patcher against value in manifest
    ASSERT(entryCount == 1);
    wchar_t curPatcherFile[MAX_PATH];
    PathGetProgramName(curPatcherFile, arrsize(curPatcherFile));
    StrToAnsi(ansi, curPatcherFile, arrsize(ansi));
    if (!MD5Check(ansi, manifest[0].md5)) {
//      MessageBox(GetTopWindow(nil), "MD5 failed", "Msg", MB_OK);
        SelfPatcherStream::totalBytes += manifest[0].zipSize;

        AtomicAdd(&s_numFiles, 1);
        SetText("Downloading new patcher...");

        StrToAnsi(ansi, s_newPatcherFile, arrsize(ansi));
        SelfPatcherStream * stream = NEWZERO(SelfPatcherStream);
        if (!stream->Open(ansi, "wb"))
            ErrorAssert(__LINE__, __FILE__, "Failed to create file: %s, errno: %u", ansi, errno);

        NetCliFileDownloadRequest(manifest[0].downloadName, stream, DownloadCallback, nil);
    }
    else {
        s_downloadComplete = true;
    }
}

//============================================================================
static void FileSrvIpAddressCallback (
    ENetError       result,
    void *          param,
    const wchar_t     addr[]
) {
    NetCliGateKeeperDisconnect();

    if (IS_NET_ERROR(result)) {
        plString msg = plString::Format("FileSrvIpAddressRequest failed: %S", NetErrorToString(result));
        plStatusLog::AddLineS("patcher.log", msg.c_str());

        s_patchResult = result;
        s_downloadComplete = true;
    }
    
    // Start connecting to the server
    const char* caddr = hsWStringToString(addr);
    NetCliFileStartConnect(&caddr, 1, true);
    delete[] caddr;

    PathGetProgramDirectory(s_newPatcherFile, arrsize(s_newPatcherFile));
    GetTempFileNameW(s_newPatcherFile, kPatcherExeFilename, 0, s_newPatcherFile);
    plFileUtils::RemoveFile(s_newPatcherFile);

    NetCliFileManifestRequest(ManifestCallback, nil, s_manifest);
}

//============================================================================
static bool SelfPatcherProc (bool * abort, plLauncherInfo *info) {

    bool patched = false;
    s_downloadComplete = false;
    s_patchResult = kNetSuccess;

    NetClientInitialize();
    NetClientSetErrorHandler(NetErrorHandler);

    const char** addrs;
    unsigned count;

    count = GetGateKeeperSrvHostnames(&addrs);

    // Start connecting to the server
    NetCliGateKeeperStartConnect(addrs, count);

    // request a file server ip address
    NetCliGateKeeperFileSrvIpAddressRequest(FileSrvIpAddressCallback, nil, true);

    while(!s_downloadComplete && !*abort) {
        NetClientUpdate();
        AsyncSleep(10);
    }   

    NetCliFileDisconnect();
    NetClientUpdate();

    // Shutdown the client/server networking subsystem
    NetClientDestroy();

    if (s_downloadComplete && !*abort && s_updated && IS_NET_SUCCESS(s_patchResult)) {

        // launch new patcher
        STARTUPINFOW        si;
        PROCESS_INFORMATION pi;
        memset(&si, 0, sizeof(si));
        memset(&pi, 0, sizeof(pi));
        si.cb = sizeof(si);

        wchar_t cmdline[MAX_PATH];
        StrPrintf(cmdline, arrsize(cmdline), L"%s %s", s_newPatcherFile, info->cmdLine);

        // we have only successfully patched if we actually launch the new version of the patcher
        patched = CreateProcessW(
            NULL,
            cmdline,
            NULL,
            NULL,
            FALSE, 
            DETACHED_PROCESS,
            NULL,
            NULL,
            &si,
            &pi
        );
        SetReturnCode(pi.dwProcessId);
        CloseHandle( pi.hThread );
        CloseHandle( pi.hProcess );
        ASSERT(patched);
    }

    return patched;
}


/*****************************************************************************
*
*   ProgressStream Functions
*
***/

//============================================================================
uint32_t SelfPatcherStream::Write(uint32_t byteCount, const void* buffer) {
    progress += byteCount;
    float p = (float)progress / (float)totalBytes * 100;        // progress
    SetProgress( (int)p );
    return plZlibStream::Write(byteCount, buffer);
}


/*****************************************************************************
*
*   Protected Functions
*
***/

//============================================================================
// if return value is true, there was an update and the patcher should be shutdown, so the new patcher can take over
bool SelfPatch (bool noSelfPatch, bool * abort, ENetError * result, plLauncherInfo *info) {
    bool patched = false;
    if (!noSelfPatch) {
        SetText("Checking for patcher update...");
        patched = SelfPatcherProc(abort, info);
    }
    *result = s_patchResult;
    return patched;
}

