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

#include "plResPatcher.h"
#include "hsResMgr.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plCompression/plZlibStream.h"
#include "pnEncryption/plChecksum.h"
#include "plMessage/plResPatcherMsg.h"
#include "pnNetBase/pnNbError.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plResMgr/plResManager.h"
#include "plStatusLog/plStatusLog.h"

/////////////////////////////////////////////////////////////////////////////

class plResDownloadStream : public plZlibStream
{
    plOperationProgress* fProgress;
    plFileName fFilename;
    bool fIsZipped;

public:
    plResDownloadStream(plOperationProgress* prog, const plFileName& reqFile)
        : fProgress(prog)
    { 
        fIsZipped = reqFile.GetFileExt().CompareI("gz") == 0;
    }

    virtual bool Open(const plFileName& filename, const char* mode)
    {
        fFilename = filename;
        return plZlibStream::Open(filename, mode);
    }

    virtual uint32_t Write(uint32_t count, const void* buf)
    {
        fProgress->Increment((float)count);
        if (fIsZipped)
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }

    plFileName GetFileName() const { return fFilename; }
    bool IsZipped() const { return fIsZipped; }
    void Unlink() const { plFileSystem::Unlink(fFilename); }
};

/////////////////////////////////////////////////////////////////////////////

static void FileDownloaded(
    ENetError           result,
    void*               param,
    const plFileName &  filename,
    hsStream*           writer)
{
    plResPatcher* patcher = (plResPatcher*)param;
    plFileName file = filename;
    if (((plResDownloadStream*)writer)->IsZipped())
        file = file.StripFileExt(); // Kill off .gz
    writer->Close();

    switch (result)
    {
        case kNetSuccess:
        {
            PatcherLog(kStatus, "    Download Complete: %s", file.AsString().c_str());
            
            // If this is a PRP, then we need to add it to the ResManager
            plFileName clientPath = static_cast<plResDownloadStream*>(writer)->GetFileName();
            if (clientPath.GetFileExt().CompareI("prp") == 0)
            {
                plResManager* clientResMgr = static_cast<plResManager*>(hsgResMgr::ResMgr());
                clientResMgr->AddSinglePage(clientPath);
            }

            // Continue down the warpath
            patcher->IssueRequest();
            delete writer;
            return;
        }
        case kNetErrFileNotFound:
            PatcherLog(kError, "    Download Failed: %s not found", file.AsString().c_str());
            break;
        default:
            char* error = hsWStringToString(NetErrorToString(result));
            PatcherLog(kError, "    Download Failed: %s", error);
            delete[] error;
            break;
    }

    // Failure case
    static_cast<plResDownloadStream*>(writer)->Unlink();
    patcher->Finish(false);
    delete writer;
}

static void ManifestDownloaded(
    ENetError                     result,
    void*                         param,
    const wchar_t                 group[],
    const NetCliFileManifestEntry manifest[],
    uint32_t                      entryCount)
{
    plResPatcher* patcher = (plResPatcher*)param;
    plString name = plString::FromWchar(group);
    if (IS_NET_SUCCESS(result))
        PatcherLog(kInfo, "    Downloaded manifest %s", name.c_str());
    else {
        PatcherLog(kError, "    Failed to download manifest %s", name.c_str());
        patcher->Finish(false);
        return;
    }

    for (uint32_t i = 0; i < entryCount; ++i)
    {
        const NetCliFileManifestEntry mfs = manifest[i];
        plFileName fileName = plString::FromWchar(mfs.clientName);
        plFileName downloadName = plString::FromWchar(mfs.downloadName);

        // See if the files are the same
        // 1. Check file size before we do time consuming md5 operations
        // 2. Do wasteful md5. We should consider implementing a CRC instead.
        if (plFileInfo(fileName).FileSize() == mfs.fileSize)
        {
            plMD5Checksum cliMD5(fileName);
            plMD5Checksum srvMD5;
            srvMD5.SetFromHexString(plString::FromWchar(mfs.md5, 32).c_str());

            if (cliMD5 == srvMD5)
                continue;
            else
                PatcherLog(kInfo, "    Enqueueing %s: MD5 Checksums Differ", fileName.AsString().c_str());
        } else
            PatcherLog(kInfo, "    Enqueueing %s: File Sizes Differ", fileName.AsString().c_str());

        // If we're still here, then we need to update the file.
        float size = mfs.zipSize ? (float)mfs.zipSize : (float)mfs.fileSize;
        patcher->GetProgress()->SetLength(size + patcher->GetProgress()->GetMax());
        patcher->RequestFile(downloadName, fileName);
    }

    patcher->IssueRequest();
}

/////////////////////////////////////////////////////////////////////////////

static char*  sLastError              = nil;
plResPatcher* plResPatcher::fInstance = nil;

plResPatcher* plResPatcher::GetInstance()
{
    if (!fInstance)
        fInstance = new plResPatcher;
    return fInstance;
}

void plResPatcher::Shutdown()
{
    // Better not call this while we're patching
    delete fInstance;
}

/////////////////////////////////////////////////////////////////////////////

plResPatcher::plResPatcher()
    : fPatching(false), fProgress(nil) { }

plResPatcher::~plResPatcher()
{
    if (fProgress)
        delete fProgress;
}

void plResPatcher::IssueRequest()
{
    if (!fPatching) return;
    if (fRequests.empty())
        // Wheee!
        Finish();
    else {
        Request req = fRequests.front();
        fRequests.pop();

        plString title;
        if (req.fType == kManifest)
        {
            PatcherLog(kMajorStatus, "    Downloading manifest... %s", req.fFile.AsString().c_str());
            title = plString::Format("Checking %s for updates...", req.fFile.AsString().c_str());
            NetCliFileManifestRequest(ManifestDownloaded, this, req.fFile.AsString().ToWchar());
        } else if (req.fType == kFile) {
            PatcherLog(kMajorStatus, "    Downloading file... %s", req.fFriendlyName.AsString().c_str());
            title = plString::Format("Downloading... %s", req.fFriendlyName.GetFileName().c_str());

            // If this is a PRP, we need to unload it from the ResManager

            if (req.fFriendlyName.GetFileExt().CompareI("prp") == 0)
                ((plResManager*)hsgResMgr::ResMgr())->RemoveSinglePage(req.fFriendlyName);

            plFileSystem::CreateDir(req.fFriendlyName.StripFileName(), true);
            plResDownloadStream* stream = new plResDownloadStream(fProgress, req.fFile);
            if (stream->Open(req.fFriendlyName, "wb"))
                NetCliFileDownloadRequest(req.fFile, stream, FileDownloaded, this);
            else {
                PatcherLog(kError, "    Unable to create file %s", req.fFriendlyName.AsString().c_str());
                Finish(false);
            }
        }

        fProgress->SetTitle(title.c_str());
    }
}

void plResPatcher::Finish(bool success)
{
    while (fRequests.size())
        fRequests.pop();

    fPatching = false;
    if (success)
        PatcherLog(kHeader, "--- Patch Completed Successfully ---");
    else
    {
        PatcherLog(kHeader, "--- Patch Killed by Error ---");
        if (fProgress)
            fProgress->SetAborting();
    }
    delete fProgress; fProgress = nil;

    plResPatcherMsg* pMsg = new plResPatcherMsg(success, sLastError);
    delete[] sLastError; sLastError = nil;
    pMsg->Send(); // whoosh... off it goes
}

void plResPatcher::RequestFile(const plFileName& srvName, const plFileName& cliName)
{
    fRequests.push(Request(srvName, kFile, cliName));
}

void plResPatcher::RequestManifest(const plString& age)
{
    fRequests.push(Request(age, kManifest));
}

void plResPatcher::Start()
{
    hsAssert(!fPatching, "Too many calls to plResPatcher::Start");
    fPatching = true;
    PatcherLog(kHeader, "--- Patch Started (%i requests) ---", fRequests.size());
    fProgress = plProgressMgr::GetInstance()->RegisterOperation(0.0, "Checking for updates...",
        plProgressMgr::kUpdateText, false, true);
    IssueRequest();
}

/////////////////////////////////////////////////////////////////////////////

void PatcherLog(PatcherLogType type, const char* format, ...)
{
    uint32_t color = 0;
    switch (type)
    {
    case kHeader:       color = plStatusLog::kWhite;    break;
    case kInfo:         color = plStatusLog::kBlue;     break;
    case kMajorStatus:  color = plStatusLog::kYellow;   break;
    case kStatus:       color = plStatusLog::kGreen;    break;
    case kError:        color = plStatusLog::kRed;      break;
    }

    static plStatusLog* gStatusLog = nil;
    if (!gStatusLog)
    {
        gStatusLog = plStatusLogMgr::GetInstance().CreateStatusLog(
            20,
            "patcher.log",
            plStatusLog::kFilledBackground | plStatusLog::kAlignToTop | plStatusLog::kDeleteForMe);
    }

    va_list args;
    va_start(args, format);

    if (type == kError)
    {
        sLastError = new char[1024]; // Deleted by Finish(false)
        vsnprintf(sLastError, 1024, format, args);
        gStatusLog->AddLine(sLastError, color);
    } else
        gStatusLog->AddLineV(color, format, args);

    va_end(args);
}
