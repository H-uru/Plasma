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

#include "plFilePatcher.h"

#include <string_theory/format>

#include "pnAsyncCore/pnAsyncCore.h"
#include "pnNetBase/pnNbSrvs.h"
#include "plNetGameLib/plNetGameLib.h"
#include "pfConsoleCore/pfServerIni.h"
#include "pfPatcher/plManifests.h"

constexpr int kNetTransTimeout = 5 * 60 * 1000;                     // 5m
constexpr int kAsyncCoreShutdownTime = 2 * 1000;                    // 2s
constexpr std::chrono::milliseconds kNetCoreUpdateSleepTime(10);    // 10ms

plFilePatcher::plFilePatcher(plFileName serverIni)
    : fFlags(kPatchEverything), fServerIni(std::move(serverIni)), fError(),
      fNetCoreState(kNetCoreInactive), fProgressFunc(), fDownloadFunc()
{
}

bool plFilePatcher::ILoadServerIni()
{
    try {
        pfServerIni::Load(fServerIni);
    } catch (const pfServerIniParseException& exc) {
        ISetNetError(ST::format("Error in server config file {}: {}", fServerIni, exc.what()));
        return false;
    }

    return true;
}

void plFilePatcher::IInitNetCore()
{
    AsyncCoreInitialize();

    NetClientInitialize();
    NetClientSetErrorHandler(std::bind(&plFilePatcher::IHandleNetError, this, std::placeholders::_1, std::placeholders::_2));
    NetClientSetTransTimeoutMs(kNetTransTimeout);

    fNetCoreState = kNetCoreActive;
}

bool plFilePatcher::IRunNetCore()
{
    if (fNetCoreState == kNetCoreActive) {
        NetClientUpdate();
    }

    // don't nom all the CPU... kthx
    std::this_thread::sleep_for(kNetCoreUpdateSleepTime);
    return fNetCoreState != kNetCoreShutdown;
}

void plFilePatcher::IFiniNetCore()
{
    NetCliGateKeeperDisconnect();
    NetCliFileDisconnect();
    NetClientDestroy();

    AsyncCoreDestroy(kAsyncCoreShutdownTime);
}

void plFilePatcher::IHandleNetError(ENetProtocol protocol, ENetError error)
{
    ISetNetError(ST::format("Patching failed: {} - {}", NetProtocolToString(protocol), NetErrorAsString(error)));
    ISignalNetCoreShutdown();
}

void plFilePatcher::IRequestFileSrvInfo()
{
    // Gotta grab the filesrvs from the gate
    const ST::string* addrs;
    uint32_t num = GetGateKeeperSrvHostnames(addrs);
    NetCliGateKeeperStartConnect(addrs, num);

    NetCliGateKeeperFileSrvIpAddressRequest(true, [this](auto result, const auto& addr) {
        IHandleFileSrvInfo(result, addr);
    });
}

void plFilePatcher::IHandleFileSrvInfo(ENetError result, const ST::string& addr)
{
    NetCliGateKeeperDisconnect();

    if (IS_NET_SUCCESS(result)) {
        ST::string ips[] = { addr };
        NetCliFileStartConnect(ips, 1, true);

        IRunPatcher();
    } else {
        ISetNetError(ST::format("Patching failed: {}", NetErrorAsString(result)));
        ISignalNetCoreShutdown();
    }
}

void plFilePatcher::IRunPatcher()
{
    pfPatcher* patcher = new pfPatcher();
    patcher->OnFileDownloadDesired(std::bind(&plFilePatcher::IApproveDownload, this, std::placeholders::_1));
    patcher->OnCompletion(std::bind(&plFilePatcher::IOnPatchComplete, this, std::placeholders::_1, std::placeholders::_2));

    if (fDownloadFunc)
        patcher->OnFileDownloadBegin(fDownloadFunc);

    if (fProgressFunc)
        patcher->OnProgressTick(fProgressFunc);

    // Request everything, and then we'll filter the file list before we fetch
    patcher->RequestManifest(plManifest::ClientImageManifest());
    patcher->Start();
}

bool plFilePatcher::IApproveDownload(const plFileName& file)
{
    if (fFlags == kPatchEverything)
        return true;

    plFileName path = file.StripFileName();
    bool isDataFile = path.IsValid();

    return fFlags == kPatchData ? isDataFile : !isDataFile;
}

void plFilePatcher::IOnPatchComplete(ENetError result, const ST::string& msg)
{
    if (IS_NET_SUCCESS(result)) {
        ISignalNetCoreShutdown();
    } else {
        ISetNetError(ST::format("Patching failed: {}", NetErrorAsString(result)));
        ISignalNetCoreShutdown();
    }
}

bool plFilePatcher::Patch()
{
    if (!ILoadServerIni()) {
        return false;
    }

    IInitNetCore();
    IRequestFileSrvInfo();

    while (true) {
        if (!IRunNetCore())
            break;
    }

    IFiniNetCore();

    return fError.empty();
}
