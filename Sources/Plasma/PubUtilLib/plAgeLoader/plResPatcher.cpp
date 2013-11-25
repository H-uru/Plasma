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
#include "plgDispatch.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plFile/plEncryptedStream.h"
#include "plFile/plStreamSource.h"
#include "plFile/plSecureStream.h"
#include "plMessage/plResPatcherMsg.h"
#include "pfPatcher/pfPatcher.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plResMgr/plResManager.h"

extern bool gDataServerLocal;
bool gSkipPreload = false;

/////////////////////////////////////////////////////////////////////////////

plResPatcher* plResPatcher::fInstance = nullptr;

plResPatcher* plResPatcher::GetInstance()
{
    if (!fInstance)
        fInstance = new plResPatcher;
    return fInstance;
}

void plResPatcher::Shutdown()
{
    delete fInstance;
}

/////////////////////////////////////////////////////////////////////////////

void plResPatcher::OnCompletion(ENetError result, const plString& status)
{
    plString error = plString::Null;
    if (IS_NET_ERROR(result))
        error = plString::Format("Update Failed: %S\n%s", NetErrorAsString(result), status.c_str());
    plgDispatch::Dispatch()->MsgQueue(new plResPatcherMsg(IS_NET_SUCCESS(result), error));
}

void plResPatcher::OnFileDownloadBegin(const plFileName& file)
{
    fProgress->SetTitle(plString::Format("Downloading %s...", file.GetFileName().c_str()));

    if (file.GetFileExt().CompareI("prp") == 0) {
        plResManager* mgr = static_cast<plResManager*>(hsgResMgr::ResMgr());
        if (mgr)
            mgr->RemoveSinglePage(file);
    }
}

void plResPatcher::OnFileDownloaded(const plFileName& file)
{
    if (file.GetFileExt().CompareI("prp") == 0) {
        plResManager* mgr = static_cast<plResManager*>(hsgResMgr::ResMgr());
        if (mgr)
            mgr->AddSinglePage(file);
    }
}

bool plResPatcher::OnGameCodeDiscovered(const plFileName& file, hsStream* stream)
{
    plSecureStream* ss = new plSecureStream(false, plStreamSource::GetInstance()->GetEncryptionKey());
    if (ss->Open(stream)) {
        plStreamSource::GetInstance()->InsertFile(file, ss);

        // SecureStream will hold a decrypted buffer...
        stream->Close();
        delete stream;
    } else
        plStreamSource::GetInstance()->InsertFile(file, stream);

    return true; // ASSume success for now...
}

void plResPatcher::OnProgressTick(uint64_t dl, uint64_t total, const plString& msg)
{
    if (dl && total) {
        fProgress->SetLength(total);
        fProgress->SetHowMuch(dl);
    }

    plString status = plString::Format("%s / %s",
        plFileSystem::ConvertFileSize(dl).c_str(),
        plFileSystem::ConvertFileSize(total).c_str()
    );

    fProgress->SetStatusText(status);
    fProgress->SetInfoText(msg);
}

pfPatcher* plResPatcher::CreatePatcher()
{
    pfPatcher* patcher = new pfPatcher;
    patcher->OnCompletion(std::bind(&plResPatcher::OnCompletion, this, std::placeholders::_1, std::placeholders::_2));
    patcher->OnFileDownloadBegin(std::bind(&plResPatcher::OnFileDownloadBegin, this, std::placeholders::_1));
    patcher->OnFileDownloaded(std::bind(&plResPatcher::OnFileDownloaded, this, std::placeholders::_1));
    patcher->OnProgressTick(std::bind(&plResPatcher::OnProgressTick, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // sneaky hax: do the old SecurePreloader thing.... except here
    if (!fRequestedGameCode && !gSkipPreload) {
        patcher->OnGameCodeDiscovery(std::bind(&plResPatcher::OnGameCodeDiscovered, this, std::placeholders::_1, std::placeholders::_2));
        patcher->RequestGameCode();
        fRequestedGameCode = true;
    }

    return patcher;
}

void plResPatcher::InitProgress()
{
    // this is deleted in plAgeLoader::MsgReceive for thread safety
    fProgress = plProgressMgr::GetInstance()->RegisterOperation(0.f, nullptr, plProgressMgr::kUpdateText);
}

/////////////////////////////////////////////////////////////////////////////

plResPatcher::plResPatcher()
    : fProgress(nullptr), fRequestedGameCode(false) { }

plResPatcher::~plResPatcher()
{
    delete fProgress;
}

void plResPatcher::Update(const std::vector<plString>& manifests)
{
    if (gDataServerLocal)
        plgDispatch::Dispatch()->MsgSend(new plResPatcherMsg());
     else {
        InitProgress();
        pfPatcher* patcher = CreatePatcher();
        patcher->RequestManifest(manifests);
        patcher->Start(); // whoosh... off it goes
    }
}

void plResPatcher::Update(const plString& manifest)
{
    if (gDataServerLocal)
        plgDispatch::Dispatch()->MsgSend(new plResPatcherMsg());
    else {
        InitProgress();
        pfPatcher* patcher = CreatePatcher();
        patcher->RequestManifest(manifest);
        patcher->Start(); // whoosh... off it goes
    }
}

