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
#include "plFile/plFileUtils.h"
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
    char* fFilename;
    bool fIsZipped;

public:
    plResDownloadStream(plOperationProgress* prog, const wchar_t* reqFile)
        : fProgress(prog), fFilename(nil)
    { 
        fIsZipped = wcscmp(plFileUtils::GetFileExt(reqFile), L"gz") == 0;
    }

    ~plResDownloadStream()
    {
        if (fFilename)
            delete[] fFilename;
    }

    hsBool Open(const char* filename, const char* mode)
    {
        fFilename = hsStrcpy(filename);
        return plZlibStream::Open(filename, mode);
    }

    uint32_t Write(uint32_t count, const void* buf)
    {
        fProgress->Increment((float)count);
        if (fIsZipped)
            return plZlibStream::Write(count, buf);
        else
            return fOutput->Write(count, buf);
    }

    bool IsZipped() const { return fIsZipped; }
    void Unlink() const { plFileUtils::RemoveFile(fFilename); }
};

/////////////////////////////////////////////////////////////////////////////

static void FileDownloaded(
    ENetError       result,
    void*           param,
    const wchar_t   filename[],
    hsStream*       writer) 
{
    plResPatcher* patcher = (plResPatcher*)param;
    char* name = hsWStringToString(filename);
    if (((plResDownloadStream*)writer)->IsZipped())
        plFileUtils::StripExt(name); // Kill off .gz
    writer->Close();

    switch (result)
    {
        case kNetSuccess:
            PatcherLog(kStatus, "    Download Complete: %s", name);
            
            // If this is a PRP, then we need to add it to the ResManager
            if (stricmp(plFileUtils::GetFileExt(name), "prp") == 0)
                ((plResManager*)hsgResMgr::ResMgr())->AddSinglePage(name);

            // Continue down the warpath
            patcher->IssueRequest();
            delete[] name;
            delete writer;
            return;
        case kNetErrFileNotFound:
            PatcherLog(kError, "    Download Failed: %s not found", name);
            break;
        default:
            char* error = hsWStringToString(NetErrorToString(result));
            PatcherLog(kError, "    Download Failed: %s", error);
            delete[] error;
            break;
    }

    // Failure case
    ((plResDownloadStream*)writer)->Unlink();
    patcher->Finish(false);
    delete[] name;
    delete writer;
}

static void ManifestDownloaded(
    ENetError                     result, 
    void*                         param, 
    const wchar_t                 group[], 
    const NetCliFileManifestEntry manifest[], 
    uint32_t                        entryCount)
{
    plResPatcher* patcher = (plResPatcher*)param;
    char* name = hsWStringToString(group);
    if (IS_NET_SUCCESS(result))
        PatcherLog(kInfo, "    Downloaded manifest %s", name);
    else {
        PatcherLog(kError, "    Failed to download manifest %s", name);
        patcher->Finish(false);
        delete[] name;
        return;
    }

    for (uint32_t i = 0; i < entryCount; ++i)
    {
        const NetCliFileManifestEntry mfs = manifest[i];
        char* fileName = hsWStringToString(mfs.clientName);

        // See if the files are the same
        // 1. Check file size before we do time consuming md5 operations
        // 2. Do wasteful md5. We should consider implementing a CRC instead.
        if (plFileUtils::GetFileSize(fileName) == mfs.fileSize)
        {
            plMD5Checksum cliMD5(fileName);
            plMD5Checksum srvMD5;
            char* eapSucksString = hsWStringToString(mfs.md5);
            srvMD5.SetFromHexString(eapSucksString);
            delete[] eapSucksString;

            if (cliMD5 == srvMD5)
            {
                delete[] fileName;
                continue;
            } else
                PatcherLog(kInfo, "    Enqueueing %s: MD5 Checksums Differ", fileName);
        } else
            PatcherLog(kInfo, "    Enqueueing %s: File Sizes Differ", fileName);

        // If we're still here, then we need to update the file.
        patcher->GetProgress()->SetLength((float)mfs.fileSize + patcher->GetProgress()->GetMax());
        patcher->RequestFile(mfs.downloadName, mfs.clientName);
    }

    patcher->IssueRequest();
    delete[] name;
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

        std::wstring title;
        if (req.fType == kManifest)
        {
            char* eapSucksString = hsWStringToString(req.fFile.c_str());
            PatcherLog(kMajorStatus, "    Downloading manifest... %s", eapSucksString);
            xtl::format(title, L"Checking %s for updates...", req.fFile.c_str());
            NetCliFileManifestRequest(ManifestDownloaded, this, req.fFile.c_str());
            delete[] eapSucksString;
        } else if (req.fType == kFile) {
            char* eapSucksString = hsWStringToString(req.fFriendlyName.c_str());
            PatcherLog(kMajorStatus, "    Downloading file... %s", eapSucksString);
            xtl::format(title, L"Downloading... %s", plFileUtils::GetFileName(req.fFriendlyName.c_str()));

            // If this is a PRP, we need to unload it from the ResManager
            if (stricmp(plFileUtils::GetFileExt(eapSucksString), "prp") == 0)
                ((plResManager*)hsgResMgr::ResMgr())->RemoveSinglePage(eapSucksString);

            plFileUtils::EnsureFilePathExists(req.fFriendlyName.c_str());
            plResDownloadStream* stream = new plResDownloadStream(fProgress, req.fFile.c_str());
            if(stream->Open(eapSucksString, "wb"))
                NetCliFileDownloadRequest(req.fFile.c_str(), stream, FileDownloaded, this);
            else {
                PatcherLog(kError, "    Unable to create file %s", eapSucksString);
                Finish(false);
            }
            delete[] eapSucksString;
        }

        char* hack = hsWStringToString(title.c_str());
        fProgress->SetTitle(hack);
        delete[] hack;
    }
}

void plResPatcher::Finish(bool success)
{
    while (fRequests.size())
        fRequests.pop();
    if (fProgress) {
        delete fProgress;
        fProgress = nil;
    }

    fPatching = false;
    if (success)
        PatcherLog(kHeader, "--- Patch Completed Successfully ---");
    else
        PatcherLog(kHeader, "--- Patch Killed by Error ---");

    plResPatcherMsg* pMsg = new plResPatcherMsg(success, sLastError);
    pMsg->Send(); // whoosh... off it goes
    if (sLastError)
    {
        delete[] sLastError;
        sLastError = nil;
    }
}

void plResPatcher::RequestFile(const wchar_t* srvName, const wchar_t* cliName)
{
    fRequests.push(Request(srvName, kFile, cliName));
}

void plResPatcher::RequestManifest(const wchar_t* age)
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
        vsprintf(sLastError, format, args);
        gStatusLog->AddLine(sLastError, color);
    } else
        gStatusLog->AddLineV(color, format, args);

    va_end(args);
}
