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

#include <string_theory/format>

#include "plAgeLoader.h"
#include "plFile/plEncryptedStream.h"
#include "plFile/plStreamSource.h"
#include "plFile/plSecureStream.h"
#include "plMessage/plResPatcherMsg.h"
#include "pfPatcher/pfPatcher.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plResMgr/plResManager.h"

extern bool gDataServerLocal;
bool gPythonLocal = false;
bool gSDLLocal = false;

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

void plResPatcher::OnCompletion(ENetError result, const ST::string& status)
{
    ST::string error;
    if (IS_NET_ERROR(result))
        error = ST::format("Update Failed: {}\n{}", NetErrorAsString(result), status);
    plgDispatch::Dispatch()->MsgQueue(new plResPatcherMsg(IS_NET_SUCCESS(result), error));
}

void plResPatcher::OnFileDownloadBegin(const plFileName& file)
{
    fProgress->SetTitle(ST::format("Downloading {}...", file.GetFileName()));

    if (file.GetFileExt().compare_i("prp") == 0) {
        plResManager* mgr = static_cast<plResManager*>(hsgResMgr::ResMgr());
        if (mgr)
            mgr->RemoveSinglePage(file);
    }
}

void plResPatcher::OnFileDownloaded(const plFileName& file)
{
    if (file.GetFileExt().compare_i("prp") == 0) {
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

void plResPatcher::OnProgressTick(uint64_t dl, uint64_t total, const ST::string& msg)
{
    if (dl && total) {
        fProgress->SetLength(float(total));
        fProgress->SetHowMuch(float(dl));
    }

    ST::string status = ST::format("{} / {}",
        plFileSystem::ConvertFileSize(dl),
        plFileSystem::ConvertFileSize(total)
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
    if (!fRequestedGameCode && (!gPythonLocal || !gSDLLocal)) {
        patcher->OnGameCodeDiscovery(std::bind(&plResPatcher::OnGameCodeDiscovered, this, std::placeholders::_1, std::placeholders::_2));

        // There is a very special case for local data, and that is the SDL. The SDL is a contract that we have with the
        // server. If the client and server have different ideas about what the SDL is, then we're really up poop creek.
        // So, we *always* ask for the server's SDL unless we really, really, really don't want it.
        patcher->RequestGameCode(!gPythonLocal, !gSDLLocal);
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

void plResPatcher::Update(const std::vector<ST::string>& manifests)
{
    InitProgress();
    pfPatcher* patcher = CreatePatcher();
    if (!gDataServerLocal)
        patcher->RequestManifest(manifests);
    patcher->Start(); // whoosh... off it goes
}

void plResPatcher::Update(const ST::string& manifest)
{
    InitProgress();
    pfPatcher* patcher = CreatePatcher();
    if (!gDataServerLocal)
        patcher->RequestManifest(manifest);
    patcher->Start(); // whoosh... off it goes
}

