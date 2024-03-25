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

#include "plClientLauncher.h"

#include "HeadSpin.h"
#include "plCmdParser.h"
#include "plFileSystem.h"
#include "plProduct.h"
#include "hsThread.h"
#include "hsTimer.h"

#include "pnAsyncCore/pnAsyncCore.h"

#include "plMessageBox/hsMessageBox.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plStatusLog/plStatusLog.h"

#include "pfPatcher/plManifests.h"
#include "pfPatcher/pfPatcher.h"

#include "pfConsoleCore/pfConsoleEngine.h"
PF_CONSOLE_LINK_FILE(Core)

#include <algorithm>
#include <curl/curl.h>
#include <deque>
#include <thread>
#include <chrono>

plClientLauncher::ErrorFunc s_errorProc = nullptr; // don't even ask, cause I'm not happy about this.

const int kNetTransTimeout = 5 * 60 * 1000;        // 5m
const int kShardStatusUpdateTime = 5;              // 5s
const int kAsyncCoreShutdownTime = 2 * 1000;       // 2s
const std::chrono::milliseconds kNetCoreUpdateSleepTime(10);    // 10ms

// ===================================================

class plShardStatus : public hsThread
{
    double        fLastUpdate;
    hsEvent       fUpdateEvent;
    char          fCurlError[CURL_ERROR_SIZE];

public:
    plClientLauncher::StatusFunc fShardFunc;

    plShardStatus() : fLastUpdate() { }

    void Run() override;
    void Shutdown();
    void Update();
};

static size_t ICurlCallback(void* buffer, size_t size, size_t nmemb, void* thread)
{
    static char status[256];

    size_t count = std::min<size_t>(size * nmemb, std::size(status) - 1);
    memcpy(status, buffer, count);
    status[count] = 0;
    static_cast<plShardStatus*>(thread)->fShardFunc(status);
    return size * nmemb;
}

void plShardStatus::Run()
{
    SetThisThreadName(ST_LITERAL("plShardStatus"));

    ST::string url = GetServerStatusUrl();

    // initialize CURL
    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, fCurlError);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "UruClient/1.0");
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, ICurlCallback);

    // we want to go ahead and run once
    fUpdateEvent.Signal();

    // loop until we die!
    do {
        fUpdateEvent.Wait();
        if (GetQuit())
            break;

        if (!url.empty() && curl_easy_perform(curl.get()))
            fShardFunc(fCurlError);
        fLastUpdate = hsTimer::GetSysSeconds();
    } while (!GetQuit());
}

void plShardStatus::Shutdown()
{
    SetQuit(true);;
    fUpdateEvent.Signal();
}

void plShardStatus::Update()
{
    double now = hsTimer::GetSysSeconds();
    if ((now - fLastUpdate) >= kShardStatusUpdateTime)
        fUpdateEvent.Signal();
}

// ===================================================

class plRedistUpdater : public hsThread
{
    bool fSuccess;

public:
    plClientLauncher* fParent;
    plClientLauncher::InstallRedistFunc fInstallProc;
    std::deque<plFileName> fRedistQueue;

    plRedistUpdater()
        : fParent(), fSuccess(true)
    { }

    ~plRedistUpdater()
    {
        // If anything is left in the deque, it was not installed.
        // We should unlink them so the next launch will redownload and install them.
        std::for_each(fRedistQueue.begin(), fRedistQueue.end(),
            [] (const plFileName& file) {
                plFileSystem::Unlink(file);
            }
        );
    }

    void OnQuit() override
    {
        // If we succeeded, then we should launch the game client...
        if (fSuccess)
            fParent->LaunchClient();
    }

    void Run() override
    {
        SetThisThreadName(ST_LITERAL("plRedistUpdater"));

        while (!fRedistQueue.empty()) {
            if (fInstallProc(fRedistQueue.back()))
                fRedistQueue.pop_back();
            else {
                s_errorProc(kNetErrInternalError, fRedistQueue.back().AsString());
                fSuccess = false;
                return;
            }
        }
    }

    void Start() override
    {
        if (fRedistQueue.empty())
            OnQuit();
        else
            hsThread::Start();
    }
};

// ===================================================

plClientLauncher::plClientLauncher() :
    fFlags(0),
    fServerIni("server.ini"),
    fPatcherFactory(nullptr),
    fClientExecutable(plManifest::ClientExecutable()),
    fStatusThread(new plShardStatus()),
    fInstallerThread(new plRedistUpdater()),
    fNetCoreState(kNetCoreInactive)
{
    pfPatcher::GetLog()->AddLine(plProduct::ProductString());
}

plClientLauncher::~plClientLauncher() { }

// ===================================================

ST::string plClientLauncher::GetAppArgs() const
{
    // If -Repair was specified, there are no args for the next call...
    if (hsCheckBits(fFlags, kRepairGame)) {
        return ST::string();
    }

    ST::string_stream ss;
    ss << "-ServerIni=";
    ss << fServerIni.AsString();

    // optional args
    if (hsCheckBits(fFlags, kClientImage))
        ss << " -Image";
    if (hsCheckBits(fFlags, kPatchOnly))
        ss << " -PatchOnly";
    if (hsCheckBits(fFlags, kSkipLoginDialog))
        ss << " -SkipLoginDialog";
    if (hsCheckBits(fFlags, kSkipIntroMovies))
        ss << " -SkipIntroMovies";

    return ss.to_string();
}

void plClientLauncher::IOnPatchComplete(ENetError result, const ST::string& msg)
{
    if (IS_NET_SUCCESS(result)) {
        // a couple of options
        // 1. we self-patched and didn't update anything. patch the main client.
        // 2. we self-patched and did things and stuff... re-run myself.
        // 3. we patched the client... run it.
        if (!hsCheckBits(fFlags, kHaveSelfPatched) && (fClientExecutable == plManifest::ClientExecutable())) {
            // case 1
            hsSetBits(fFlags, kHaveSelfPatched);
            PatchClient();
        } else {
            // cases 2 & 3 -- update any redistributables, then launch the client.
            fInstallerThread->fParent = this;
            fInstallerThread->Start();
        }
    } else if (s_errorProc)
        s_errorProc(result, msg);
}

bool plClientLauncher::IApproveDownload(const plFileName& file)
{
    // So, for a repair, what we want to do is quite simple.
    // That is: download everything that is NOT in the root directory.
    if (hsCheckBits(fFlags, kGameDataOnly)) {
        plFileName path = file.StripFileName();
        return path.IsValid();
    }

    // If we have successfully self-patched, don't accept any manifest
    // entries matching the patcher, in case the patcher appears in the
    // client manifests...
    if (hsCheckBits(fFlags, kHaveSelfPatched)) {
        return file.AsString().compare_i(plManifest::PatcherExecutable().AsString()) != 0;
    }

    // Passes a sniff test!
    return true;
}

void plClientLauncher::LaunchClient() const
{
    if (fStatusFunc)
        fStatusFunc("Launching...");
    fLaunchClientFunc(fClientExecutable, GetAppArgs());
}

void plClientLauncher::PatchClient()
{
    if (fStatusFunc) {
        if (hsCheckBits(fFlags, kGameDataOnly))
            fStatusFunc("Verifying game data...");
        else
            fStatusFunc("Checking for updates...");
    }
    hsAssert(fPatcherFactory, "why is the patcher factory nil?");

    pfPatcher* patcher = fPatcherFactory();
    patcher->OnCompletion(std::bind(&plClientLauncher::IOnPatchComplete, this, std::placeholders::_1, std::placeholders::_2));
    patcher->OnFileDownloadDesired(std::bind(&plClientLauncher::IApproveDownload, this, std::placeholders::_1));
    patcher->OnSelfPatch([&](const plFileName& file) { fClientExecutable = file; });
    patcher->OnRedistUpdate([&](const plFileName& file) { fInstallerThread->fRedistQueue.push_back(file); });

    // Let's get 'er done.
    if (hsCheckBits(fFlags, kHaveSelfPatched)) {
        if (hsCheckBits(fFlags, kClientImage))
            patcher->RequestManifest(plManifest::ClientImageManifest());
        else
            patcher->RequestManifest(plManifest::ClientManifest());
    } else
        patcher->RequestManifest(plManifest::PatcherManifest());
    patcher->Start();
}

bool plClientLauncher::CompleteSelfPatch(const std::function<void()>& waitProc) const
{
    if (hsCheckBits(fFlags, kHaveSelfPatched))
        return false;

    ST::string myExe = plFileSystem::GetCurrentAppPath().GetFileName();
    if (myExe.compare_i(plManifest::PatcherExecutable().AsString()) != 0) {
        waitProc();

        // so now we need to unlink the old patcher, and move ME into that fool's place...
        // then we can continue on our merry way!
        if (!plFileSystem::Unlink(plManifest::PatcherExecutable())) {
            hsMessageBox(ST_LITERAL("Failed to delete old patcher executable!"), ST_LITERAL("Error"), hsMessageBoxNormal, hsMessageBoxIconError);
            return true;
        }
        if (!plFileSystem::Move(plFileSystem::GetCurrentAppPath(), plManifest::PatcherExecutable())) {
            hsMessageBox(ST_LITERAL("Failed to move patcher executable!"), ST_LITERAL("Error"), hsMessageBoxNormal, hsMessageBoxIconError);
            return true;
        }

        // Now, execute the new patcher...
        fLaunchClientFunc(plManifest::PatcherExecutable(), GetAppArgs() + " -NoSelfPatch");
        return true;
    }
    return false;
}

// ===================================================

static void IGotFileServIPs(ENetError result, void* param, const ST::string& addr)
{
    plClientLauncher* launcher = static_cast<plClientLauncher*>(param);
    NetCliGateKeeperDisconnect();

    if (IS_NET_SUCCESS(result)) {
        // bah... why do I even bother
        ST::string eapSucks[] = { addr };
        NetCliFileStartConnect(eapSucks, 1, true);

        // Who knows if we will actually connect. So let's start updating.
        launcher->PatchClient();
    } else if (s_errorProc)
        s_errorProc(result, "Failed to get FileServ addresses");
}

static void IEapSucksErrorProc(ENetProtocol protocol, ENetError error)
{
    if (s_errorProc) {
        ST::string msg = ST::format("Protocol: {}", NetProtocolToString(protocol));
        s_errorProc(error, msg);
    }
}

void plClientLauncher::InitializeNetCore()
{
    // initialize shard status
    hsTimer::SetRealTime(true);
    fStatusThread->Start();

    // init eap...
    AsyncCoreInitialize();

    NetClientInitialize();
    NetClientSetErrorHandler(IEapSucksErrorProc);
    NetClientSetTransTimeoutMs(kNetTransTimeout);

    // Gotta grab the filesrvs from the gate
    const ST::string* addrs;
    uint32_t num = GetGateKeeperSrvHostnames(addrs);

    NetCliGateKeeperStartConnect(addrs, num);
    NetCliGateKeeperFileSrvIpAddressRequest(IGotFileServIPs, this, true);

    // Windows is getting a little unreliable about reporting its own state, so we keep
    // track of whether or not we are active now.
    fNetCoreState = kNetCoreActive;
}

// ===================================================

bool plClientLauncher::PumpNetCore() const
{
    // this ain't net core, but it needs to be pumped :(
    hsTimer::IncSysSeconds();

    if (fNetCoreState == kNetCoreActive) {
        // pump eap
        NetClientUpdate();

        // pump shard status
        fStatusThread->Update();
    }

    // don't nom all the CPU... kthx
    std::this_thread::sleep_for(kNetCoreUpdateSleepTime);
    return fNetCoreState != kNetCoreShutdown;
}

void plClientLauncher::ShutdownNetCore()
{
    // shutdown shard status
    fStatusThread->Shutdown();

    // unhook the neterr callback at this point because all transactions
    // will fail when we call NetClientDestroy
    s_errorProc = nullptr;

    // shutdown eap
    NetCliGateKeeperDisconnect();
    NetCliFileDisconnect();
    NetClientDestroy();

    // shutdown eap (part deux)
    AsyncCoreDestroy(kAsyncCoreShutdownTime);

    // Denote shutdown
    fNetCoreState = kNetCoreShutdown;
}

// ===================================================

bool plClientLauncher::LoadServerIni() const
{
    PF_CONSOLE_INITIALIZE(Core);

    pfConsoleEngine console;
    return console.ExecuteFile(fServerIni);
}

void plClientLauncher::ParseArguments()
{
#define APPLY_FLAG(arg, flag) \
    if (cmdParser.GetBool(arg)) \
        fFlags |= flag;

    enum { kArgServerIni, kArgNoSelfPatch, kArgImage, kArgRepairGame, kArgPatchOnly,
           kArgSkipLoginDialog, kArgSkipIntroMovies };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgFlagged | kCmdTypeString, "ServerIni", kArgServerIni },
        { kCmdArgFlagged | kCmdTypeBool, "NoSelfPatch", kArgNoSelfPatch },
        { kCmdArgFlagged | kCmdTypeBool, "Image", kArgImage },
        { kCmdArgFlagged | kCmdTypeBool, "Repair", kArgRepairGame },
        { kCmdArgFlagged | kCmdTypeBool, "PatchOnly", kArgPatchOnly },
        { kCmdArgFlagged | kCmdTypeBool, "SkipLoginDialog", kArgSkipLoginDialog },
        { kCmdArgFlagged | kCmdTypeBool, "SkipIntroMovies", kArgSkipIntroMovies }
    };

    std::vector<ST::string> args;
    args.reserve(__argc);
    for (size_t i = 0; i < __argc; i++) {
        args.push_back(ST::string::from_utf8(__argv[i]));
    }

    plCmdParser cmdParser(cmdLineArgs, std::size(cmdLineArgs));
    cmdParser.Parse(args);

    // cache 'em
    if (cmdParser.IsSpecified(kArgServerIni))
        fServerIni = cmdParser.GetString(kArgServerIni);
    APPLY_FLAG(kArgNoSelfPatch, kHaveSelfPatched);
    APPLY_FLAG(kArgImage, kClientImage);
    APPLY_FLAG(kArgRepairGame, kRepairGame);
    APPLY_FLAG(kArgPatchOnly, kPatchOnly);
    APPLY_FLAG(kArgSkipLoginDialog, kSkipLoginDialog);
    APPLY_FLAG(kArgSkipIntroMovies, kSkipIntroMovies);

    // last chance setup
    if (hsCheckBits(fFlags, kPatchOnly))
        fClientExecutable = plFileName();
    else if (hsCheckBits(fFlags, kRepairGame))
        fClientExecutable = plManifest::PatcherExecutable();

#undef APPLY_FLAG
}

void plClientLauncher::SetErrorProc(ErrorFunc proc)
{
    s_errorProc = std::move(proc);
}

void plClientLauncher::SetInstallerProc(InstallRedistFunc proc)
{
    fInstallerThread->fInstallProc = std::move(proc);
}

void plClientLauncher::SetShardProc(StatusFunc proc)
{
    fStatusThread->fShardFunc = std::move(proc);
}
