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
#include "plFileSystem.h"
#include "hsStream.h"
#include "hsThread.h"
#include "hsTimer.h"

#include "pnEncryption/plChecksum.h"
#include "pnNetBase/pnNbError.h"

#include "plAudioCore/plAudioFileReader.h"
#include "plCompression/plZlibStream.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plStatusLog/plStatusLog.h"

template<typename... _Args>
static inline void PatcherLogGreen(const char* format, _Args&&... args)
{
    pfPatcher::GetLog()->AddLineF(plStatusLog::kGreen, format, std::forward<_Args>(args)...);
}

template<typename... _Args>
static inline void PatcherLogRed(const char* format, _Args&&... args)
{
    pfPatcher::GetLog()->AddLineF(plStatusLog::kRed, format, std::forward<_Args>(args)...);
}

template<typename... _Args>
static inline void PatcherLogWhite(const char* format, _Args&&... args)
{
    pfPatcher::GetLog()->AddLineF(plStatusLog::kWhite, format, std::forward<_Args>(args)...);
}

template<typename... _Args>
static inline void PatcherLogYellow(const char* format, _Args&&... args)
{
    pfPatcher::GetLog()->AddLineF(plStatusLog::kYellow, format, std::forward<_Args>(args)...);
}

// ===================================================

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

// ===================================================

struct pfPatcherQueuedFile
{
    enum class Type
    {
        kManifestHash,
        kSoundDecompress,
    };

    Type fType;
    plFileName fClientPath;
    plFileName fServerPath;
    plMD5Checksum fChecksum;
    uint32_t fFileSize;
    uint32_t fZipSize;
    uint32_t fFlags;

    pfPatcherQueuedFile(Type t, const NetCliFileManifestEntry& file)
    : fType(t), fClientPath(plFileName(ST::string::from_utf16(file.clientName)).Normalize()),
          fServerPath(ST::string::from_utf16(file.downloadName)), fChecksum(),
          fFileSize(file.fileSize), fZipSize(file.zipSize), fFlags(file.flags)
    {
        ST::string temp(file.md5, std::size(file.md5));
        fChecksum.SetFromHexString(temp.c_str());
    }

    pfPatcherQueuedFile(Type t, plFileName path, uint32_t flags=0)
        : fType(t), fClientPath(std::move(path)), fChecksum(), fFileSize(), fZipSize(), fFlags(flags)
    { }

    pfPatcherQueuedFile(const pfPatcherQueuedFile& copy) = delete;

    pfPatcherQueuedFile& operator =(const pfPatcherQueuedFile& copy) = delete;
};

// ===================================================

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

    std::deque<Request> fRequests;
    std::deque<pfPatcherQueuedFile> fQueuedFiles;

    std::recursive_mutex fRequestMut;
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
    volatile bool fWantPython;
    volatile bool fWantSDL;

    uint64_t fCurrBytes;
    uint64_t fTotalBytes;

    pfPatcherWorker();
    ~pfPatcherWorker();

    void OnQuit() override;

    void EndPatch(ENetError result, const ST::string& msg={});
    bool IssueRequest();
    void Run() override;
    void IHashFile(pfPatcherQueuedFile& file);
    void IDecompressSound(const pfPatcherQueuedFile& sound) const;
    void ProcessFile();
    void WhitelistFile(const plFileName& file, bool justDownloaded, hsStream* s=nullptr);
    void EnqueuePreloaderLists();
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
        float secs = hsTimer::GetSeconds<float>() - fDLStartTime;
        auto bytesPerSec = uint64_t(fBytesWritten / secs);
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
        : fParent(parent), fFilename(filename), fFlags(), fBytesWritten(), fDLStartTime(), plZlibStream()
    {
        fParent->fTotalBytes += size;
        fOutput = std::make_unique<hsRAMStream>();
    }

    pfPatcherStream(pfPatcherWorker* parent, const pfPatcherQueuedFile& file)
        : fParent(parent), fFilename(file.fClientPath.Normalize()), fFlags(file.fFlags), fBytesWritten(), fDLStartTime(), plZlibStream()
    {
        // ugh. eap removed the compressed flag in his fail manifests
        if (file.fServerPath.GetFileExt().compare_i("gz") == 0) {
            fFlags |= kFlagZipped;
            parent->fTotalBytes += file.fZipSize;
        } else {
            parent->fTotalBytes += file.fFileSize;
        }
    }

    void Begin()
    {
        fDLStartTime = hsTimer::GetSeconds<float>();
        if (!fOutput)
            Open(fFilename, "wb");
    }

    bool Open(const plFileName& filename, const char* mode) override
    {
        hsAssert(filename == fFilename, "trying to save to a different file, eh?");
        bool retVal = plZlibStream::Open(filename, mode);
        if (!retVal)
            PatcherLogRed("\tPhailed to open %s: '%s'", filename.AsString().c_str(), strerror(errno));
        return retVal;
    }

    void Close()
    {
        if (hsCheckBits(fFlags, kFlagZipped))
            plZlibStream::Close();
        fOutput.reset();
    }

    uint32_t Write(uint32_t count, const void* buf) override
    {
        // tick whatever progress bar we have
        IUpdateProgress(count);

        // write the appropriate blargs
        if (hsCheckBits(fFlags, kFlagZipped))
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }

    bool AtEnd() override { return fOutput->AtEnd(); }
    uint32_t GetEOF() override { return fOutput->GetEOF(); }
    uint32_t GetPosition() const override { return fOutput->GetPosition(); }
    uint32_t Read(uint32_t count, void* buf) override { return fOutput->Read(count, buf); }
    void Rewind() override { fOutput->Rewind(); }
    void FastFwd() override { fOutput->FastFwd(); }
    void SetPosition(uint32_t pos) override { fOutput->SetPosition(pos); }
    void Skip(uint32_t deltaByteCount) override { fOutput->Skip(deltaByteCount); }

    uint32_t GetFlags() const { return fFlags; }
    plFileName GetFileName() const { return fFilename; }
    bool IsRedistUpdate() const { return hsCheckBits(fFlags, kRedistUpdate); }
    bool IsSelfPatch() const { return hsCheckBits(fFlags, kSelfPatch); }
    bool RequiresSfxCache() const { return hsCheckBits(fFlags, kSndFlagCacheSplit) || hsCheckBits(fFlags, kSndFlagCacheStereo); }
    void Unlink() const { plFileSystem::Unlink(fFilename); }
};

// ===================================================

static void IAuthThingDownloadCB(ENetError result, void* param, const plFileName& filename, hsStream* writer)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded Legacy File '{}'", filename);
        patcher->IssueRequest();

        // Now, we pass our RAM-backed file to the game code handlers. In the main client,
        // this will trickle down and add a new friend to plStreamSource. This should never
        // happen in any other app...
        writer->Rewind();
        patcher->WhitelistFile(filename, true, writer);
    } else {
        PatcherLogRed("\tDownloaded Failed: File '{}'", filename);
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
            hsLockGuard(patcher->fRequestMut);
            for (unsigned i = 0; i < infoCount; ++i) {
                PatcherLogYellow("\tEnqueuing Legacy File '{}'", infoArr[i].filename);

                plFileName fn = ST::string::from_utf16(infoArr[i].filename);
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

static void IHandleManifestDownload(pfPatcherWorker* patcher, const char16_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    PatcherLogGreen("\tDownloaded Manifest '{}'", group);
    {
        hsLockGuard(patcher->fFileMut);
        for (unsigned i = 0; i < entryCount; ++i)
            patcher->fQueuedFiles.emplace_back(pfPatcherQueuedFile::Type::kManifestHash, manifest[i]);
        patcher->fFileSignal.Signal();
    }
    patcher->IssueRequest();
}

static void IPreloaderManifestDownloadCB(ENetError result, void* param, const char16_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result)) {
        IHandleManifestDownload(patcher, group, manifest, entryCount);
    } else {
        patcher->EnqueuePreloaderLists();
        patcher->IssueRequest();
    }
}

static void IFileManifestDownloadCB(ENetError result, void* param, const char16_t group[], const NetCliFileManifestEntry manifest[], unsigned entryCount)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);

    if (IS_NET_SUCCESS(result))
        IHandleManifestDownload(patcher, group, manifest, entryCount);
    else {
        PatcherLogRed("\tDownload Failed: Manifest '{}'", group);
        patcher->EndPatch(result, ST::string::from_utf16(group));
    }
}

static void IFileThingDownloadCB(ENetError result, void* param, const plFileName& filename, hsStream* writer)
{
    pfPatcherWorker* patcher = static_cast<pfPatcherWorker*>(param);
    pfPatcherStream* stream = static_cast<pfPatcherStream*>(writer);

    // We need to explicitly close any underlying streams NOW because we
    // might be about to signal the client that this file needs to be acted
    // on, eg installed, decompressed from ogg to wave, etc. We can't wait
    // for hsStream's RAII to close the stream at the end of this function or
    // the callback code may crash due to either a permissions error or the
    // zlib decompression not being complete.
    stream->Close();

    if (IS_NET_SUCCESS(result)) {
        PatcherLogGreen("\tDownloaded File '{}'", stream->GetFileName());
        patcher->WhitelistFile(stream->GetFileName(), true);
        if (patcher->fSelfPatch && stream->IsSelfPatch())
            patcher->fSelfPatch(stream->GetFileName());
        if (patcher->fRedistUpdateDownloaded && stream->IsRedistUpdate())
            patcher->fRedistUpdateDownloaded(stream->GetFileName());

        // Punt the SFX decompression to the patcher thread (this is the main/draw thread)
        if (stream->RequiresSfxCache()) {
            hsLockGuard(patcher->fFileMut);
            patcher->fQueuedFiles.emplace_back(pfPatcherQueuedFile::Type::kSoundDecompress,
                                               stream->GetFileName(), stream->GetFlags());
            patcher->fFileSignal.Signal();
        }
        patcher->IssueRequest();
    } else {
        PatcherLogRed("\tDownloaded Failed: File '{}'", stream->GetFileName());
        stream->Unlink();
        patcher->EndPatch(result, filename.AsString());
    }

    delete stream;
}

// ===================================================

pfPatcherWorker::pfPatcherWorker() :
    fStarted(false), fCurrBytes(0), fTotalBytes(0), fRequestActive(true), fParent(nullptr),
    fWantPython(), fWantSDL()
{ }

pfPatcherWorker::~pfPatcherWorker()
{
    {
        hsLockGuard(fRequestMut);
        std::for_each(fRequests.begin(), fRequests.end(),
            [] (const Request& req) {
                delete req.fStream;
            }
        );
        fRequests.clear();
    }

    {
        hsLockGuard(fFileMut);
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
            PatcherLogRed("\tNetwork Error: {}", NetErrorToString(result));
            PatcherLogWhite("--- Patch Killed by Error ---");
        }
    }

    fStarted = false;
    fFileSignal.Signal();
}

bool pfPatcherWorker::IssueRequest()
{
    hsLockGuard(fRequestMut);
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
            NetCliFileManifestRequest(IFileManifestDownloadCB, this, req.fName.to_utf16().data());
            break;
        case Request::kSecurePreloader:
            // so, yeah, this is usually the "SecurePreloader" manifest on the file server...
            // except on legacy servers, this may not exist, so we need to fall back without nuking everything!
            NetCliFileManifestRequest(IPreloaderManifestDownloadCB, this, req.fName.to_utf16().data());
            break;
        case Request::kAuthFile:
            // ffffffuuuuuu
            req.fStream->Begin();
            if (fFileBeginDownload)
                fFileBeginDownload(req.fStream->GetFileName());

            NetCliAuthFileRequest(req.fName, req.fStream, IAuthThingDownloadCB, this);
            break;
        case Request::kPythonList:
            NetCliAuthFileListRequest(u"Python", u"pak", IGotAuthFileList, this);
            break;
        case Request::kSdlList:
            NetCliAuthFileListRequest(u"SDL", u"sdl", IGotAuthFileList, this);
            break;
        DEFAULT_FATAL(req.fType);
    }

    fRequests.pop_front();
    return true;
}

void pfPatcherWorker::Run()
{
    SetThisThreadName(ST_LITERAL("pfPatcherWorker"));
    // So here's the rub:
    // We have one or many manifests in the fRequests deque. We begin issuing those requests one-by one, starting here.
    // As we receive the answer, the NetCli thread populates fQueuedFiles and pings the fFileSignal semaphore, then issues the next request...
    // In this non-UI/non-Net thread, we do the stutter-prone/time-consuming IO/hashing operations. (Typically, the UI thread == Net thread)
    // As we find files that need updating, we add them to fRequests.
    // If there is no net request from ME when we find a file, we issue the request
    // Once a file is downloaded, the next request is issued.
    // When there are no files in my deque and no requests in my deque, we exit without errors.
    PatcherLogWhite("--- Patch Started ({} requests) ---", fRequests.size());
    fStarted = true;
    IssueRequest();

    // Now, work until we're done processing files
    do {
        fFileSignal.Wait();

        hsLockGuard(fFileMut);
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

void pfPatcherWorker::IHashFile(pfPatcherQueuedFile& file)
{
    // Only accept game code if we want it
    if (!fWantPython && file.fClientPath.GetFileExt().compare_i("pak") == 0) {
        PatcherLogRed("\tDeclined unwanted Python code '{}'", file.fClientPath);
        return;
    }
    if (!fWantSDL && file.fClientPath.GetFileExt().compare_i("sdl") == 0) {
        PatcherLogRed("\tDeclined unwanted SDL '{}'", file.fClientPath);
        return;
    }

    // Check to see if ours matches
    plFileInfo mine(file.fClientPath);
    if (mine.FileSize() == file.fFileSize) {
        plMD5Checksum cliMD5(file.fClientPath);
        if (cliMD5 == file.fChecksum) {
            WhitelistFile(file.fClientPath, false);
            return;
        }
    }

    // It's different... but do we want it?
    if (fFileDownloadDesired) {
        if (!fFileDownloadDesired(file.fClientPath)) {
            PatcherLogRed("\tDeclined '{}'", file.fClientPath);
            return;
        }
    }

    // If you got here, they're different and we want it.
    PatcherLogYellow("\tEnqueuing '{}'", file.fServerPath);
    plFileSystem::CreateDir(file.fClientPath.StripFileName());

    // If someone registered for SelfPatch notifications, then we should probably
    // let them handle the gruntwork... Otherwise, go nuts!
    if (fSelfPatch) {
        if (file.fClientPath == plFileSystem::GetCurrentAppPath().GetFileName()) {
            file.fClientPath += ".tmp"; // don't overwrite myself!
            file.fFlags |= kSelfPatch;
        }
    }

    pfPatcherStream* s = new pfPatcherStream(this, file);
    {
        hsLockGuard(fRequestMut);
        fRequests.emplace_back(file.fServerPath.AsString(), Request::kFile, s);
    }
}

void pfPatcherWorker::IDecompressSound(const pfPatcherQueuedFile& file) const
{
    PatcherLogGreen("\tDecompressing SFX '{}'", file.fClientPath);
    if (hsCheckBits(file.fFlags, kSndFlagCacheSplit))
        plAudioFileReader::CacheFile(file.fClientPath, true);
    if (hsCheckBits(file.fFlags, kSndFlagCacheStereo))
        plAudioFileReader::CacheFile(file.fClientPath, false);
}

void pfPatcherWorker::ProcessFile()
{
    do {
        pfPatcherQueuedFile& file = fQueuedFiles.front();
        switch (file.fType) {
        case pfPatcherQueuedFile::Type::kManifestHash:
            IHashFile(file);
            break;
        case pfPatcherQueuedFile::Type::kSoundDecompress:
            IDecompressSound(file);
            break;
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
                hsUNIXStream* newStream = new hsUNIXStream;
                newStream->Open(file, "rb");
                stream = newStream;
            }

            // if something terrible goes wrong (eg bad encryption), we can exit sanely
            // callback eats stream
            if (!fGameCodeDiscovered(file, stream))
                EndPatch(kNetErrInternalError, "SecurePreloader failed.");
        }
    } else {
        // no dad gum memory leaks, m'kay?
        delete stream;
    }
}

void pfPatcherWorker::EnqueuePreloaderLists()
{
    PatcherLogYellow("\tWARNING: *** Falling back to AuthSrv file lists to get game code ***");

    hsLockGuard(fRequestMut);
    if (fWantPython)
        fRequests.emplace_back(ST::string(), pfPatcherWorker::Request::kPythonList);
    if (fWantSDL)
        fRequests.emplace_back(ST::string(), pfPatcherWorker::Request::kSdlList);
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
    fWorker->fOnComplete = std::move(cb);
}

void pfPatcher::OnFileDownloadBegin(FileDownloadFunc cb)
{
    fWorker->fFileBeginDownload = std::move(cb);
}

void pfPatcher::OnFileDownloadDesired(FileDesiredFunc cb)
{
    fWorker->fFileDownloadDesired = std::move(cb);
}

void pfPatcher::OnFileDownloaded(FileDownloadFunc cb)
{
    fWorker->fFileDownloaded = std::move(cb);
}

void pfPatcher::OnGameCodeDiscovery(GameCodeDiscoverFunc cb)
{
    fWorker->fGameCodeDiscovered = std::move(cb);
}

void pfPatcher::OnProgressTick(ProgressTickFunc cb)
{
    fWorker->fProgressTick = std::move(cb);
}

void pfPatcher::OnRedistUpdate(FileDownloadFunc cb)
{
    fWorker->fRedistUpdateDownloaded = std::move(cb);
}

void pfPatcher::OnSelfPatch(FileDownloadFunc cb)
{
    fWorker->fSelfPatch = std::move(cb);
}

// ===================================================

void pfPatcher::RequestGameCode(bool python, bool sdl)
{
    fWorker->fWantPython = python;
    fWorker->fWantSDL = sdl;

    hsLockGuard(fWorker->fRequestMut);
    if (NetCliFileQueryConnected()) {
        fWorker->fRequests.emplace_back("SecurePreloader", pfPatcherWorker::Request::kSecurePreloader);
    } else {
        fWorker->EnqueuePreloaderLists();
    }
}

void pfPatcher::RequestManifest(const ST::string& mfs)
{
    hsLockGuard(fWorker->fRequestMut);
    fWorker->fRequests.emplace_back(mfs, pfPatcherWorker::Request::kManifest);
}

void pfPatcher::RequestManifest(const std::vector<ST::string>& mfs)
{
    hsLockGuard(fWorker->fRequestMut);
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
