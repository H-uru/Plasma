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

#include "pfSecurePreloader.h"

#include "hsStream.h"
#include "plgDispatch.h"
#include "plCompression/plZlibStream.h"
#include "pnEncryption/plChecksum.h"
#include "plFile/plFileUtils.h"
#include "plFile/plSecureStream.h"
#include "plFile/plStreamSource.h"
#include "plMessage/plNetCommMsgs.h"
#include "plMessage/plPreloaderMsg.h"
#include "plProgressMgr/plProgressMgr.h"

extern hsBool gDataServerLocal;
pfSecurePreloader* pfSecurePreloader::fInstance = nil;

/////////////////////////////////////////////////////////////////////

typedef std::pair<const wchar_t*, const wchar_t*> WcharPair;

struct AuthRequestParams
{
    pfSecurePreloader* fThis;
    std::queue<WcharPair> fFileGroups;

    AuthRequestParams(pfSecurePreloader* parent)
        : fThis(parent) { }
};

/////////////////////////////////////////////////////////////////////

void ProcAuthDownloadParams(AuthRequestParams* params);

void GotAuthSrvManifest(
    ENetError                   result,
    void*                       param,
    const NetCliAuthFileInfo    infoArr[],
    uint32_t                      infoCount
) {
    AuthRequestParams* arp = (AuthRequestParams*)param;
    if (IS_NET_ERROR(result))
    {
        FATAL("Failed to get AuthSrv manifest!");
        arp->fThis->Terminate();
        delete arp;
    } else {
        arp->fThis->PreloadManifest(infoArr, infoCount);
        ProcAuthDownloadParams(arp);
    }
}

void GotFileSrvManifest(
    ENetError                     result, 
    void*                         param, 
    const wchar_t                 group[], 
    const NetCliFileManifestEntry manifest[], 
    uint32_t                        entryCount
) {
    pfSecurePreloader* sp = (pfSecurePreloader*)param;
    if (result == kNetErrFileNotFound)
    {
        AuthRequestParams* params = new AuthRequestParams(sp);
        params->fFileGroups.push(WcharPair(L"Python", L"pak"));
        params->fFileGroups.push(WcharPair(L"SDL", L"sdl"));
        ProcAuthDownloadParams(params);
        return;
    } else if (!entryCount) {
        FATAL("SecurePreloader manifest empty!");
        sp->Terminate();
        return;
    }

    sp->PreloadManifest(manifest, entryCount);
}

void FileDownloaded(
    ENetError       result,
    void*           param,
    const wchar_t   filename[],
    hsStream*       writer
) {
    pfSecurePreloader* sp = (pfSecurePreloader*)param;
    if (IS_NET_ERROR(result))
    {
        FATAL("SecurePreloader download failed");
        sp->Terminate();
    } else {
        sp->FilePreloaded(filename, writer);
    }
}

void ProcAuthDownloadParams(AuthRequestParams* params)
{
    // Request the "manifests" until there are none left, then download the files
    if (params->fFileGroups.empty())
    {
        params->fThis->PreloadNextFile();
        delete params;
    } else {
        WcharPair wp = params->fFileGroups.front();
        params->fFileGroups.pop();
        NetCliAuthFileListRequest(wp.first, wp.second, GotAuthSrvManifest, params);
    }
}

/////////////////////////////////////////////////////////////////////

class pfSecurePreloaderStream : public plZlibStream
{
    plOperationProgress* fProgress;
    bool                 fIsZipped;

public:

    pfSecurePreloaderStream(plOperationProgress* prog, bool zipped)
        : fProgress(prog), fIsZipped(zipped), plZlibStream()
    { 
        fOutput = new hsRAMStream;
    }

    ~pfSecurePreloaderStream()
    {
        delete fOutput;
        fOutput = nil;
        plZlibStream::Close();
    }

    hsBool AtEnd() { return fOutput->AtEnd(); }
    uint32_t GetEOF() { return fOutput->GetEOF(); }
    uint32_t GetPosition() const { return fOutput->GetPosition(); }
    uint32_t GetSizeLeft() const { return fOutput->GetSizeLeft(); }
    uint32_t Read(uint32_t count, void* buf) { return fOutput->Read(count, buf); }
    void Rewind() { fOutput->Rewind(); }
    void SetPosition(uint32_t pos) { fOutput->SetPosition(pos); }
    void Skip(uint32_t deltaByteCount) { fOutput->Skip(deltaByteCount); }

    uint32_t Write(uint32_t count, const void* buf)
    {
        if (fProgress)
            fProgress->Increment((float)count);
        if (fIsZipped)
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }
};

/////////////////////////////////////////////////////////////////////

pfSecurePreloader::pfSecurePreloader()
    : fProgress(nil), fLegacyMode(false)
{ }

pfSecurePreloader::~pfSecurePreloader()
{
    while (fDownloadEntries.size())
    {
        free((void*)fDownloadEntries.front());
        fDownloadEntries.pop();
    }

    while (fManifestEntries.size())
    {
        free((void*)fManifestEntries.front());
        fManifestEntries.pop();
    }
}

hsRAMStream* pfSecurePreloader::LoadToMemory(const wchar_t* file) const
{
    if (!plFileUtils::FileExists(file))
        return nil;

    hsUNIXStream s;
    hsRAMStream* ram = new hsRAMStream;
    s.Open(file);
    
    uint32_t loadLen = 1024 * 1024;
    uint8_t* buf = new uint8_t[loadLen];
    while (uint32_t read = s.Read(loadLen, buf))
        ram->Write(read, buf);
    delete[] buf;

    s.Close();
    ram->Rewind();
    return ram;
}

void pfSecurePreloader::SaveFile(hsStream* file, const wchar_t* name) const
{
    hsUNIXStream s;
    s.Open(name, L"wb");
    uint32_t pos = file->GetPosition();
    file->Rewind();

    uint32_t loadLen = 1024 * 1024;
    uint8_t* buf = new uint8_t[loadLen];
    while (uint32_t read = file->Read(loadLen, buf))
        s.Write(read, buf);
    file->SetPosition(pos);
    s.Close();
    delete[] buf;
}

bool pfSecurePreloader::IsZipped(const wchar_t* filename) const
{
    return wcscmp(plFileUtils::GetFileExt(filename), L"gz") == 0;
}

void pfSecurePreloader::PreloadNextFile()
{
    if (fManifestEntries.empty())
    {
        Finish();
        return;
    }

    const wchar_t* filename = fDownloadEntries.front();
    hsStream* s = new pfSecurePreloaderStream(fProgress, IsZipped(filename));
    
    // Thankfully, both callbacks have the same arguments
    if (fLegacyMode)
        NetCliAuthFileRequest(filename, s, FileDownloaded, this);
    else
        NetCliFileDownloadRequest(filename, s, FileDownloaded, this);
}

void pfSecurePreloader::Init()
{
    RegisterAs(kSecurePreloader_KEY);
    // TODO: If we're going to support reconnects, then let's do it right.
    // Later...
    //plgDispatch::Dispatch()->RegisterForExactType(plNetCommAuthConnectedMsg::Index(), GetKey());
}

void pfSecurePreloader::Start()
{
#ifndef PLASMA_EXTERNAL_RELEASE
    // Using local data? Move along, move along...
    if (gDataServerLocal)
    {
        Finish();
        return;
    }
#endif

    NetCliAuthGetEncryptionKey(fEncryptionKey, 4);
    
    // TODO: Localize
    fProgress = plProgressMgr::GetInstance()->RegisterOperation(0.0f, "Checking for Updates", plProgressMgr::kUpdateText, false, true);

    // Now, we need to fetch the "SecurePreloader" manifest from the file server, which will contain the python and SDL files.
    // We're basically reimplementing plResPatcher here, except preferring to keep everything in memory, then flush to disk 
    // when we're done. If this fails, then we shall download everything from the AuthSrv like in the old days.
    NetCliFileManifestRequest(GotFileSrvManifest, this, L"SecurePreloader");
}

void pfSecurePreloader::Terminate()
{
    FATAL("pfSecurePreloader failure");

    plPreloaderMsg* msg = new plPreloaderMsg;
    msg->fSuccess = false;
    plgDispatch::Dispatch()->MsgSend(msg);
}

void pfSecurePreloader::Finish()
{
    plPreloaderMsg* msg = new plPreloaderMsg;
    msg->fSuccess = true;
    plgDispatch::Dispatch()->MsgSend(msg);
}

void pfSecurePreloader::Shutdown()
{
    SetInstance(nil);
    if (fProgress)
    {
        delete fProgress;
        fProgress = nil;
    }

    // Takes care of UnReffing us
    UnRegisterAs(kSecurePreloader_KEY);
}

void pfSecurePreloader::PreloadManifest(const NetCliAuthFileInfo manifestEntries[], uint32_t entryCount)
{
    uint32_t totalBytes = 0;
    if (fProgress)
        totalBytes = (uint32_t)fProgress->GetMax();
    fLegacyMode = true;

    for (uint32_t i = 0; i < entryCount; ++i)
    {
        const NetCliAuthFileInfo mfs = manifestEntries[i];
        fDownloadEntries.push(wcsdup(mfs.filename));
        if (IsZipped(mfs.filename))
        {
            wchar_t* name = wcsdup(mfs.filename);
            plFileUtils::StripExt(name);
            fManifestEntries.push(name);
            
        } else
            fManifestEntries.push(wcsdup(mfs.filename));

        totalBytes += mfs.filesize;
    }

    if (fProgress)
    {
        fProgress->SetLength((float)totalBytes);
        fProgress->SetTitle("Downloading...");
    }
}

void pfSecurePreloader::PreloadManifest(const NetCliFileManifestEntry manifestEntries[], uint32_t entryCount)
{
    uint32_t totalBytes = 0;
    for (uint32_t i = 0; i < entryCount; ++i)
    {
        const NetCliFileManifestEntry mfs = manifestEntries[i];
        bool fetchMe = true;
        hsRAMStream* s = nil;

        if (plFileUtils::FileExists(mfs.clientName))
        {
            s = LoadToMemory(mfs.clientName);
            if (s)
            {
                // Damn this
                const char* md5 = hsWStringToString(mfs.md5);
                plMD5Checksum srvHash;
                srvHash.SetFromHexString(md5);
                delete[] md5;

                // Now actually copare the hashes
                plMD5Checksum lclHash;
                lclHash.CalcFromStream(s);
                fetchMe = (srvHash != lclHash);
            }
        }

        if (fetchMe)
        {
            fManifestEntries.push(wcsdup(mfs.clientName));
            fDownloadEntries.push(wcsdup(mfs.downloadName));
            if (IsZipped(mfs.downloadName))
                totalBytes += mfs.zipSize;
            else
                totalBytes += mfs.fileSize;
        } else {
            plSecureStream* ss = new plSecureStream(s, fEncryptionKey);
            plStreamSource::GetInstance()->InsertFile(mfs.clientName, ss);
        }

        if (s)
            delete s;
    }

    if (totalBytes && fProgress)
    {
        fProgress->SetLength((float)totalBytes);
        fProgress->SetTitle("Downloading...");
    }

    // This method uses only one manifest, so we're good to go now!
    PreloadNextFile();
}

void pfSecurePreloader::FilePreloaded(const wchar_t* file, hsStream* stream)
{
    // Clear out queue
    fDownloadEntries.pop();
    const wchar_t* clientName = fManifestEntries.front(); // Stolen by plStreamSource
    fManifestEntries.pop();

    if (!fLegacyMode) // AuthSrv data caching is useless
    {
        plFileUtils::EnsureFilePathExists(clientName);
        SaveFile(stream, clientName);
    }

    plSecureStream* ss = new plSecureStream(stream, fEncryptionKey);
    plStreamSource::GetInstance()->InsertFile(clientName, ss);
    delete stream;        // SecureStream holds its own decrypted buffer

    // Continue down the warpath
    PreloadNextFile();
}

pfSecurePreloader* pfSecurePreloader::GetInstance()
{
    if (!fInstance)
        fInstance = new pfSecurePreloader;
    return fInstance; 
}
