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
        enum { kFile, kManifest };

        plString fName;
        uint8_t fType;
        class pfPatcherStream* fStream;

        Request(const plString& name, uint8_t type, class pfPatcherStream* s=nullptr) :
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
    };

    std::deque<Request> fRequests;
    std::deque<NetCliFileManifestEntry> fQueuedFiles;

    hsMutex fRequestMut;
    hsMutex fFileMut;
    hsSemaphore fFileSignal;

    pfPatcher::CompletionFunc fOnComplete;
    pfPatcher::FileDownloadFunc fFileBeginDownload;
    pfPatcher::FileDownloadFunc fFileDownloaded;
    pfPatcher::ProgressTickFunc fProgressTick;

    pfPatcher* fParent;
    volatile bool fStarted;
    volatile bool fRequestActive;

    uint64_t fCurrBytes;
    uint64_t fTotalBytes;

    pfPatcherWorker();
    ~pfPatcherWorker();

    void OnQuit();

    void EndPatch(ENetError result, const plString& msg=plString::Null);
    bool IssueRequest();
    virtual hsError Run();
    void ProcessFile();
};

// ===================================================

class pfPatcherStream : public plZlibStream
{
    pfPatcherWorker* fParent;
    plFileName fFilename;
    uint32_t fFlags;

    uint64_t fBytesWritten;
    float fDLStartTime;

    plString IMakeStatusMsg() const
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
    pfPatcherStream(pfPatcherWorker* parent, const plFileName& filename, const NetCliFileManifestEntry& entry)
        : fParent(parent), fFlags(entry.flags), fBytesWritten(0)
    {
        // ugh. eap removed the compressed flag in his fail manifests
        if (filename.GetFileExt().CompareI("gz") == 0) {
            fFlags |= pfPatcherWorker::kFlagZipped;
            parent->fTotalBytes += entry.zipSize;
        } else
            parent->fTotalBytes += entry.fileSize;
    }

    virtual bool Open(const plFileName& filename, const char* mode)
    {
        fFilename = filename;
        return plZlibStream::Open(filename, mode);
    }

    virtual uint32_t Write(uint32_t count, const void* buf)
    {
        // tick whatever progress bar we have
        IUpdateProgress(count);

        // write the appropriate blargs
        if (hsCheckBits(fFlags, pfPatcherWorker::kFlagZipped))
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }

    void Begin() { fDLStartTime = hsTimer::GetSysSeconds(); }
    plFileName GetFileName() const { return fFilename; }
    void Unlink() const { plFileSystem::Unlink(fFilename); }
};

// ===================================================

static void IFileManifestDownloadCB(ENetError result, void* param, const wchar_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded Manifest '%S'", group);
        {
            hsTempMutexLock lock(patcher->fFileMut);
            for (unsigned i = 0; i < entryCount; ++i)
                patcher->fQueuedFiles.push_back(manifest[i]);
            patcher->fFileSignal.Signal();
        }
        patcher->IssueRequest();
    } else {
        PatcherLogRed("\tDownload Failed: Manifest '%S'", group);
        patcher->EndPatch(result, plString::FromWchar(group));
    }
}

static void IFileThingDownloadCB(ENetError result, void* param, const plFileName& filename, hsStream* writer)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);
    pfPatcherStream* stream = static_cast<pfPatcherStream*>(writer);
    stream->Close();

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded File '%s'", stream->GetFileName().AsString().c_str());
        if (patcher->fFileDownloaded)
            patcher->fFileDownloaded(stream->GetFileName());
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
    fStarted(false), fCurrBytes(0), fTotalBytes(0), fRequestActive(true)
{ }

pfPatcherWorker::~pfPatcherWorker()
{
    {
        hsTempMutexLock lock(fRequestMut);
        std::for_each(fRequests.begin(), fRequests.end(),
            [] (const Request& req) {
                if (req.fStream) req.fStream->Close();
                delete req.fStream;
            }
        );
        fRequests.clear();
    }

    {
        hsTempMutexLock lock(fFileMut);
        fQueuedFiles.clear();
    }
}

void pfPatcherWorker::OnQuit()
{
    // the thread's Run() has exited sanely... now we can commit hara-kiri
    delete fParent;
}

void pfPatcherWorker::EndPatch(ENetError result, const plString& msg)
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
    hsTempMutexLock lock(fRequestMut);
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
            NetCliFileManifestRequest(IFileManifestDownloadCB, this, req.fName.ToWchar());
            break;
        DEFAULT_FATAL(req.fType);
    }

    fRequests.pop_front();
    return true;
}

hsError pfPatcherWorker::Run()
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

        hsTempMutexLock fileLock(fFileMut);
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
    return hsOK;
}

void pfPatcherWorker::ProcessFile()
{
    do {
        const NetCliFileManifestEntry& entry = fQueuedFiles.front();

        // eap sucks
        plString clName = plString::FromWchar(entry.clientName);
        plString dlName = plString::FromWchar(entry.downloadName);

        // Check to see if ours matches
        plFileInfo mine(clName);
        if (mine.FileSize() == entry.fileSize) {
            plMD5Checksum cliMD5(clName);
            plMD5Checksum srvMD5;
            srvMD5.SetFromHexString(plString::FromWchar(entry.md5, 32).c_str());

            if (cliMD5 == srvMD5) {
                fQueuedFiles.pop_front();
                continue;
            }
        }

        // If you got here, they're different.
        PatcherLogYellow("\tEnqueuing '%S'", entry.clientName);
        plFileSystem::CreateDir(plFileName(clName).StripFileName());

        pfPatcherStream* s = new pfPatcherStream(this, dlName, entry);
        s->Open(clName, "wb");

        hsTempMutexLock lock(fRequestMut);
        fRequests.push_back(Request(dlName, Request::kFile, s));
        fQueuedFiles.pop_front();

        if (!fRequestActive)
            IssueRequest();
    } while (!fQueuedFiles.empty());
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

void pfPatcher::OnFileDownloaded(FileDownloadFunc cb)
{
    fWorker->fFileDownloaded = cb;
}

void pfPatcher::OnProgressTick(ProgressTickFunc cb)
{
    fWorker->fProgressTick = cb;
}

// ===================================================

void pfPatcher::RequestManifest(const plString& mfs)
{
    hsTempMutexLock lock(fWorker->fRequestMut);
    fWorker->fRequests.push_back(pfPatcherWorker::Request(mfs, pfPatcherWorker::Request::kManifest));
}

void pfPatcher::RequestManifest(const std::vector<plString>& mfs)
{
    hsTempMutexLock lock(fWorker->fRequestMut);
    std::for_each(mfs.begin(), mfs.end(),
        [&] (const plString& name) {
            fWorker->fRequests.push_back(pfPatcherWorker::Request(name, pfPatcherWorker::Request::kManifest));
        }
    );
}

bool pfPatcher::Start()
{
    hsAssert(!fWorker->fStarted, "pfPatcher is one-use only. kthx.");
    if (!fWorker->fStarted) {
        fWorker->fParent = this; // wheeeee circular
        fWorker->Start();
        return true;
    }
    return false;
}

