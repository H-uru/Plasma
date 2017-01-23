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

#include <algorithm>
#include <deque>
#include <mutex>

#include "pfPatcher.h"

#include "HeadSpin.h"
#include "plCompression/plZlibStream.h"
#include "pnEncryption/plChecksum.h"
#include "plFileSystem.h"
#include "pnNetBase/pnNbError.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plStatusLog/plStatusLog.h"
#include "hsStream.h"
#include "hsThread.h"
#include "hsTimer.h"

// Some log helper defines
#define PatcherLogGreen(...) pfPatcher::GetLog()->AddLineF(plStatusLog::kGreen, __VA_ARGS__)
#define PatcherLogRed(...) pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, __VA_ARGS__)
#define PatcherLogWhite(...) pfPatcher::GetLog()->AddLineF(plStatusLog::kWhite, __VA_ARGS__)
#define PatcherLogYellow(...) pfPatcher::GetLog()->AddLineF(plStatusLog::kYellow, __VA_ARGS__)

/** Patcher grunt work thread */
struct pfPatcherWorker : public hsThread
{
    /** Represents a File/Auth download request */
    struct Request
    {
        enum { kFile, kManifest, kSecurePreloader, kAuthFile, kPythonList, kSdlList };

        ST::string fName;
        uint8_t fType;
        class pfPatcherStream* fStream;

        Request(const ST::string& name, uint8_t type, class pfPatcherStream* s=nullptr) :
            fName(name), fType(type), fStream(s)
        { }
    };

    /** Human readable file flags */
    enum FileFlags
    {
        // Sound files only
        kSndFlagCacheSplit          = 1<<0,
        kSndFlagStreamCompressed    = 1<<1,
        kSndFlagCacheStereo         = 1<<2,

        // Any file
        kFlagZipped                 = 1<<3,

        // Executable flags
        kRedistUpdate               = 1<<4,

        // Begin internal flags
        kLastManifestFlag           = 1<<5,
        kSelfPatch                  = 1<<6,
    };

    std::deque<Request> fRequests;
    std::deque<NetCliFileManifestEntry> fQueuedFiles;

    std::mutex fRequestMut;
    std::mutex fFileMut;
    hsSemaphore fFileSignal;

    pfPatcher::CompletionFunc fOnComplete;
    pfPatcher::FileDownloadFunc fFileBeginDownload;
    pfPatcher::FileDesiredFunc fFileDownloadDesired;
    pfPatcher::FileDownloadFunc fFileDownloaded;
    pfPatcher::GameCodeDiscoverFunc fGameCodeDiscovered;
    pfPatcher::ProgressTickFunc fProgressTick;
    pfPatcher::FileDownloadFunc fRedistUpdateDownloaded;
    pfPatcher::FileDownloadFunc fSelfPatch;

    pfPatcher* fParent;
    volatile bool fStarted;
    volatile bool fRequestActive;

    uint64_t fCurrBytes;
    uint64_t fTotalBytes;

    pfPatcherWorker();
    ~pfPatcherWorker();

    void OnQuit() HS_OVERRIDE;

    void EndPatch(ENetError result, const ST::string& msg=ST::null);
    bool IssueRequest();
    void Run() HS_OVERRIDE;
    void ProcessFile();
    void WhitelistFile(const plFileName& file, bool justDownloaded, hsStream* s=nullptr);
};

// ===================================================

class pfPatcherStream : public plZlibStream
{
    pfPatcherWorker* fParent;
    plFileName fFilename;
    uint32_t fFlags;

    uint64_t fBytesWritten;
    float fDLStartTime;

    ST::string IMakeStatusMsg() const
    {
        float secs = hsTimer::GetSysSeconds() - fDLStartTime;
        float bytesPerSec = fBytesWritten / secs;
        return plFileSystem::ConvertFileSize(bytesPerSec) + "/s";
    }

    void IUpdateProgress(uint32_t count)
    {
        fBytesWritten += count; // just this file
        fParent->fCurrBytes += count; // the entire everything

        // tick-tick-tick, tick-tick-tock
        if (fParent->fProgressTick)
            fParent->fProgressTick(fParent->fCurrBytes, fParent->fTotalBytes, IMakeStatusMsg());
    }

public:
    pfPatcherStream(pfPatcherWorker* parent, const plFileName& filename, uint64_t size)
        : fParent(parent), fFilename(filename), fFlags(0), fBytesWritten(0), fDLStartTime(0.f), plZlibStream()
    {
        fParent->fTotalBytes += size;
        fOutput = new hsRAMStream;
    }

    pfPatcherStream(pfPatcherWorker* parent, const plFileName& reqName, const plFileName& cliName, const NetCliFileManifestEntry& entry)
        : fParent(parent), fFilename(cliName.Normalize()), fFlags(entry.flags), fBytesWritten(0), plZlibStream()
    {
        // ugh. eap removed the compressed flag in his fail manifests
        if (reqName.GetFileExt().compare_i("gz") == 0) {
            fFlags |= pfPatcherWorker::kFlagZipped;
            parent->fTotalBytes += entry.zipSize;
        } else
            parent->fTotalBytes += entry.fileSize;
    }

    void Begin()
    {
        fDLStartTime = hsTimer::GetSysSeconds();
        if (!fOutput)
            Open(fFilename, "wb");
    }

    bool Open(const plFileName& filename, const char* mode) HS_OVERRIDE
    {
        hsAssert(filename == fFilename, "trying to save to a different file, eh?");
        bool retVal = plZlibStream::Open(filename, mode);
        if (!retVal)
            PatcherLogRed("\tPhailed to open %s: '%s'", filename.AsString().c_str(), strerror(errno));
        return retVal;
    }

    uint32_t Write(uint32_t count, const void* buf) HS_OVERRIDE
    {
        // tick whatever progress bar we have
        IUpdateProgress(count);

        // write the appropriate blargs
        if (hsCheckBits(fFlags, pfPatcherWorker::kFlagZipped))
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }

    bool AtEnd() HS_OVERRIDE { return fOutput->AtEnd(); }
    uint32_t GetEOF() HS_OVERRIDE { return fOutput->GetEOF(); }
    uint32_t GetPosition() const HS_OVERRIDE { return fOutput->GetPosition(); }
    uint32_t Read(uint32_t count, void* buf) HS_OVERRIDE { return fOutput->Read(count, buf); }
    void Rewind() HS_OVERRIDE { fOutput->Rewind(); }
    void SetPosition(uint32_t pos) HS_OVERRIDE { fOutput->SetPosition(pos); }
    void Skip(uint32_t deltaByteCount) HS_OVERRIDE { fOutput->Skip(deltaByteCount); }

    plFileName GetFileName() const { return fFilename; }
    bool IsRedistUpdate() const { return hsCheckBits(fFlags, pfPatcherWorker::kRedistUpdate); }
    bool IsSelfPatch() const { return hsCheckBits(fFlags, pfPatcherWorker::kSelfPatch); }
    void Unlink() const { plFileSystem::Unlink(fFilename); }
};

// ===================================================

static void IAuthThingDownloadCB(ENetError result, void* param, const plFileName& filename, hsStream* writer)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded Legacy File '%s'", filename.AsString().c_str());
        patcher->IssueRequest();

        // Now, we pass our RAM-backed file to the game code handlers. In the main client,
        // this will trickle down and add a new friend to plStreamSource. This should never
        // happen in any other app...
        writer->Rewind();
        patcher->WhitelistFile(filename, true, writer);
    } else {
        PatcherLogRed("\tDownloaded Failed: File '%s'", filename.AsString().c_str());
        patcher->EndPatch(result, filename.AsString());
    }
}

static void IGotAuthFileList(ENetError result, void* param, const NetCliAuthFileInfo infoArr[], unsigned infoCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result)) {
        // so everything goes directly into the Requests deque because AuthSrv lists
        // don't have any hashes attached. WHY did eap think this was a good idea?!?!
        {
            std::lock_guard<std::mutex> lock(patcher->fRequestMut);
            for (unsigned i = 0; i < infoCount; ++i) {
                PatcherLogYellow("\tEnqueuing Legacy File '%S'", infoArr[i].filename);

                plFileName fn = ST::string::from_wchar(infoArr[i].filename);
                plFileSystem::CreateDir(fn.StripFileName());

                // We purposefully do NOT Open this stream! This uses a special auth-file constructor that
                // utilizes a backing hsRAMStream. This will be fed to plStreamSource later...
                pfPatcherStream* s = new pfPatcherStream(patcher, fn, infoArr[i].filesize);
                patcher->fRequests.emplace_back(fn.AsString(), pfPatcherWorker::Request::kAuthFile, s);
            }
        }
        patcher->IssueRequest();
    } else {
        PatcherLogRed("\tSHIT! Some legacy manifest phailed");
        patcher->EndPatch(result, "SecurePreloader failed");
    }
}

static void IHandleManifestDownload(pfPatcherWorker* patcher, const wchar_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    PatcherLogGreen("\tDownloaded Manifest '%S'", group);
    {
        std::lock_guard<std::mutex> lock(patcher->fFileMut);
        for (unsigned i = 0; i < entryCount; ++i)
            patcher->fQueuedFiles.push_back(manifest[i]);
        patcher->fFileSignal.Signal();
    }
    patcher->IssueRequest();
}

static void IPreloaderManifestDownloadCB(ENetError result, void* param, const wchar_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result))
        IHandleManifestDownload(patcher, group, manifest, entryCount);
    else {
        PatcherLogYellow("\tWARNING: *** Falling back to AuthSrv file lists to get game code ***");

        // so, we need to ask the AuthSrv about our game code
        {
            std::lock_guard<std::mutex> lock(patcher->fRequestMut);
            patcher->fRequests.emplace_back(ST::null, pfPatcherWorker::Request::kPythonList);
            patcher->fRequests.emplace_back(ST::null, pfPatcherWorker::Request::kSdlList);
        }

        // continue pumping requests
        patcher->IssueRequest();
    }
}

static void IFileManifestDownloadCB(ENetError result, void* param, const wchar_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result))
        IHandleManifestDownload(patcher, group, manifest, entryCount);
    else {
        PatcherLogRed("\tDownload Failed: Manifest '%S'", group);
        patcher->EndPatch(result, ST::string::from_wchar(group));
    }
}

static void IFileThingDownloadCB(ENetError result, void* param, const plFileName& filename, hsStream* writer)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);
    pfPatcherStream* stream = static_cast<pfPatcherStream*>(writer);
    stream->Close();

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded File '%s'", stream->GetFileName().AsString().c_str());
        patcher->WhitelistFile(stream->GetFileName(), true);
        if (patcher->fSelfPatch && stream->IsSelfPatch())
            patcher->fSelfPatch(stream->GetFileName());
        if (patcher->fRedistUpdateDownloaded && stream->IsRedistUpdate())
            patcher->fRedistUpdateDownloaded(stream->GetFileName());
        patcher->IssueRequest();
    } else {
        PatcherLogRed("\tDownloaded Failed: File '%s'", stream->GetFileName().AsString().c_str());
        stream->Unlink();
        patcher->EndPatch(result, filename.AsString());
    }

    delete stream;
}

// ===================================================

pfPatcherWorker::pfPatcherWorker() :
    fStarted(false), fCurrBytes(0), fTotalBytes(0), fRequestActive(true), fParent(nullptr)
{ }

pfPatcherWorker::~pfPatcherWorker()
{
    {
        std::lock_guard<std::mutex> lock(fRequestMut);
        std::for_each(fRequests.begin(), fRequests.end(),
            [] (const Request& req) {
                if (req.fStream) req.fStream->Close();
                delete req.fStream;
            }
        );
        fRequests.clear();
    }

    {
        std::lock_guard<std::mutex> lock(fFileMut);
        fQueuedFiles.clear();
    }
}

void pfPatcherWorker::OnQuit()
{
    // the thread's Run() has exited sanely... now we can commit hara-kiri
    delete fParent;
}

void pfPatcherWorker::EndPatch(ENetError result, const ST::string& msg)
{
    // Guard against multiple calls
    if (fStarted) {
        // Send end status
        if (fOnComplete)
            fOnComplete(result, msg);

        // yay log hax
        if (IS_NET_SUCCESS(result))
            PatcherLogWhite("--- Patch Complete ---");
        else {
            PatcherLogRed("\tNetwork Error: %S", NetErrorToString(result));
            PatcherLogWhite("--- Patch Killed by Error ---");
        }
    }

    fStarted = false;
    fFileSignal.Signal();
}

bool pfPatcherWorker::IssueRequest()
{
    std::lock_guard<std::mutex> lock(fRequestMut);
    if (fRequests.empty()) {
        fRequestActive = false;
        fFileSignal.Signal(); // make sure the patch thread doesn't deadlock!
        return false;
    } else
        fRequestActive = true;

    const Request& req = fRequests.front();
    switch (req.fType) {
        case Request::kFile:
            req.fStream->Begin();
            if (fFileBeginDownload)
                fFileBeginDownload(req.fStream->GetFileName());

            NetCliFileDownloadRequest(req.fName, req.fStream, IFileThingDownloadCB, this);
            break;
        case Request::kManifest:
            NetCliFileManifestRequest(IFileManifestDownloadCB, this, req.fName.to_wchar());
            break;
        case Request::kSecurePreloader:
            // so, yeah, this is usually the "SecurePreloader" manifest on the file server...
            // except on legacy servers, this may not exist, so we need to fall back without nuking everything!
            NetCliFileManifestRequest(IPreloaderManifestDownloadCB, this, req.fName.to_wchar());
            break;
        case Request::kAuthFile:
            // ffffffuuuuuu
            req.fStream->Begin();
            if (fFileBeginDownload)
                fFileBeginDownload(req.fStream->GetFileName());

            NetCliAuthFileRequest(req.fName, req.fStream, IAuthThingDownloadCB, this);
            break;
        case Request::kPythonList:
            NetCliAuthFileListRequest(L"Python", L"pak", IGotAuthFileList, this);
            break;
        case Request::kSdlList:
            NetCliAuthFileListRequest(L"SDL", L"sdl", IGotAuthFileList, this);
            break;
        DEFAULT_FATAL(req.fType);
    }

    fRequests.pop_front();
    return true;
}

void pfPatcherWorker::Run()
{
    // So here's the rub:
    // We have one or many manifests in the fRequests deque. We begin issuing those requests one-by one, starting here.
    // As we receive the answer, the NetCli thread populates fQueuedFiles and pings the fFileSignal semaphore, then issues the next request...
    // In this non-UI/non-Net thread, we do the stutter-prone/time-consuming IO/hashing operations. (Typically, the UI thread == Net thread)
    // As we find files that need updating, we add them to fRequests.
    // If there is no net request from ME when we find a file, we issue the request
    // Once a file is downloaded, the next request is issued.
    // When there are no files in my deque and no requests in my deque, we exit without errors.

    PatcherLogWhite("--- Patch Started (%i requests) ---", fRequests.size());
    fStarted = true;
    IssueRequest();

    // Now, work until we're done processing files
    do {
        fFileSignal.Wait();

        std::lock_guard<std::mutex> fileLock(fFileMut);
        if (!fQueuedFiles.empty()) {
            ProcessFile();
            continue;
        }

        // This makes sure both queues are empty before exiting.
        if (!fRequestActive)
            if(!IssueRequest())
                break;
    } while (fStarted);

    EndPatch(kNetSuccess);
}

void pfPatcherWorker::ProcessFile()
{
    do {
        NetCliFileManifestEntry& entry = fQueuedFiles.front();

        // eap sucks
        plFileName clName = ST::string::from_wchar(entry.clientName);
        ST::string dlName = ST::string::from_wchar(entry.downloadName);

        // Check to see if ours matches
        plFileInfo mine(clName);
        if (mine.FileSize() == entry.fileSize) {
            plMD5Checksum cliMD5(clName);
            plMD5Checksum srvMD5;
            srvMD5.SetFromHexString(ST::string::from_wchar(entry.md5, 32).c_str());

            if (cliMD5 == srvMD5) {
                WhitelistFile(clName, false);
                fQueuedFiles.pop_front();
                continue;
            }
        }

        // It's different... but do we want it?
        if (fFileDownloadDesired) {
            if (!fFileDownloadDesired(clName)) {
                PatcherLogRed("\tDeclined '%S'", entry.clientName);
                fQueuedFiles.pop_front();
                continue;
            }
        }

        // If you got here, they're different and we want it.
        PatcherLogYellow("\tEnqueuing '%S'", entry.downloadName);
        plFileSystem::CreateDir(plFileName(clName).StripFileName());

        // If someone registered for SelfPatch notifications, then we should probably
        // let them handle the gruntwork... Otherwise, go nuts!
        if (fSelfPatch) {
            if (clName == plFileSystem::GetCurrentAppPath().GetFileName()) {
                clName += ".tmp"; // don't overwrite myself!
                entry.flags |= kSelfPatch;
            }
        }

        pfPatcherStream* s = new pfPatcherStream(this, dlName, clName, entry);
        {
            std::lock_guard<std::mutex> lock(fRequestMut);
            fRequests.emplace_back(dlName, Request::kFile, s);
        }
        fQueuedFiles.pop_front();

        if (!fRequestActive)
            IssueRequest();
    } while (!fQueuedFiles.empty());
}

void pfPatcherWorker::WhitelistFile(const plFileName& file, bool justDownloaded, hsStream* stream)
{
    // if this is a newly downloaded file, fire off a completion callback
    if (justDownloaded && fFileDownloaded)
        fFileDownloaded(file);

    // we want to whitelist our game code, so here we go...
    if (fGameCodeDiscovered) {
        ST::string ext = file.GetFileExt();
        if (ext.compare_i("pak") == 0 || ext.compare_i("sdl") == 0) {
            if (!stream) {
                stream = new hsUNIXStream;
                stream->Open(file, "rb");
            }

            // if something terrible goes wrong (eg bad encryption), we can exit sanely
            // callback eats stream
            if (!fGameCodeDiscovered(file, stream))
                EndPatch(kNetErrInternalError, "SecurePreloader failed.");
        }
    } else if (stream) {
        // no dad gum memory leaks, m'kay?
        stream->Close();
        delete stream;
    }
}

// ===================================================

plStatusLog* pfPatcher::GetLog()
{
    static plStatusLog* log = nullptr;
    if (!log)
    {
        log = plStatusLogMgr::GetInstance().CreateStatusLog(
            20,
            "patcher.log",
            plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kDeleteForMe);
    }
    return log;
}

pfPatcher::pfPatcher() : fWorker(new pfPatcherWorker) { }
pfPatcher::~pfPatcher() { }

// ===================================================

void pfPatcher::OnCompletion(CompletionFunc cb)
{
    fWorker->fOnComplete = cb;
}

void pfPatcher::OnFileDownloadBegin(FileDownloadFunc cb)
{
    fWorker->fFileBeginDownload = cb;
}

void pfPatcher::OnFileDownloadDesired(FileDesiredFunc cb)
{
    fWorker->fFileDownloadDesired = cb;
}

void pfPatcher::OnFileDownloaded(FileDownloadFunc cb)
{
    fWorker->fFileDownloaded = cb;
}

void pfPatcher::OnGameCodeDiscovery(GameCodeDiscoverFunc cb)
{
    fWorker->fGameCodeDiscovered = cb;
}

void pfPatcher::OnProgressTick(ProgressTickFunc cb)
{
    fWorker->fProgressTick = cb;
}

void pfPatcher::OnRedistUpdate(FileDownloadFunc cb)
{
    fWorker->fRedistUpdateDownloaded = cb;
}

void pfPatcher::OnSelfPatch(FileDownloadFunc cb)
{
    fWorker->fSelfPatch = cb;
}

// ===================================================

void pfPatcher::RequestGameCode()
{
    std::lock_guard<std::mutex> lock(fWorker->fRequestMut);
    fWorker->fRequests.emplace_back("SecurePreloader", pfPatcherWorker::Request::kSecurePreloader);
}

void pfPatcher::RequestManifest(const ST::string& mfs)
{
    std::lock_guard<std::mutex> lock(fWorker->fRequestMut);
    fWorker->fRequests.emplace_back(mfs, pfPatcherWorker::Request::kManifest);
}

void pfPatcher::RequestManifest(const std::vector<ST::string>& mfs)
{
    std::lock_guard<std::mutex> lock(fWorker->fRequestMut);
    std::for_each(mfs.begin(), mfs.end(),
        [&] (const ST::string& name) {
            fWorker->fRequests.emplace_back(name, pfPatcherWorker::Request::kManifest);
        }
    );
}

bool pfPatcher::Start()
{
    hsAssert(!fWorker->fStarted, "pfPatcher is one-use only. kthx.");
    if (!fWorker->fStarted) {
        fWorker->fParent = this; // wheeeee circular
        fWorker->StartDetached();
        return true;
    }
    return false;
}
