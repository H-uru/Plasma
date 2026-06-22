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
#include <unordered_set>
#include <string_theory/stdio>

#include <curl/curl.h>
#include <regex>
#include <termios.h>
#include <unistd.h>

#include "HeadSpin.h"
#include "plCmdParser.h"
#include "hsDebug.h"
#include "hsEndian.h"
#include "plPipeline.h"
#include "plProduct.h"
#include "pcSmallRect.h"
#include "hsStream.h"

#include "plClient.h"
#include "plClientLoader.h"
#include "plClientWindow.h"

#include "pnEncryption/plChallengeHash.h"
#include "pnNetBase/pnNbSrvs.h"

#include "plInputCore/plInputManager.h"
#include "plMessage/plInputEventMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plPhysX/plPXSimulation.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plProgressMgr/plProgressMgr.h"
#include "plResMgr/plVersion.h"
#include "plStatusLog/plStatusLog.h"

#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfPasswordStore/pfPasswordStore.h"

#ifdef USE_X11
#   include "plX11ClientWindow.h"
#   include "pfDisplayHelpers/plX11DisplayHelper.h"
#endif

extern bool gDataServerLocal;
extern bool gPythonLocal;
extern bool gSDLLocal;

static plClientLoader gClient;
static plClientWindow* gWindow = nullptr;
static hsSemaphore statusFlag;

enum
{
    kArgSkipLoginDialog,
    kArgServerIni,
    kArgLocalData,
    kArgLocalPython,
    kArgLocalSDL,
    kArgPlayerId,
    kArgStartUpAgeName,
    kArgPvdFile,
    kArgSkipIntroMovies,
    kArgRenderer,
    kArgUsername
};

static const plCmdArgDef s_cmdLineArgs[] = {
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipLoginDialog", kArgSkipLoginDialog },
    { kCmdArgFlagged  | kCmdTypeString,     "ServerIni",       kArgServerIni },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalData",       kArgLocalData   },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalPython",     kArgLocalPython },
    { kCmdArgFlagged  | kCmdTypeBool,       "LocalSDL",        kArgLocalSDL },
    { kCmdArgFlagged  | kCmdTypeInt,        "PlayerId",        kArgPlayerId },
    { kCmdArgFlagged  | kCmdTypeString,     "Age",             kArgStartUpAgeName },
    { kCmdArgFlagged  | kCmdTypeString,     "PvdFile",         kArgPvdFile },
    { kCmdArgFlagged  | kCmdTypeBool,       "SkipIntroMovies", kArgSkipIntroMovies },
    { kCmdArgFlagged  | kCmdTypeString,     "Renderer",        kArgRenderer },
    { kCmdArgFlagged  | kCmdTypeString,     "Username",        kArgUsername },
};

//
// For error logging
//
static plStatusLog* s_DebugLog = nullptr;

static void _StatusMessageProc(const ST::string& msg)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    s_DebugLog->AddLine(msg);
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

template<typename... _Args>
static void DebugMsg(const char* format, _Args&&... args)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    s_DebugLog->AddLineF(plStatusLog::kYellow, format, std::forward<_Args>(args)...);
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

static void DebugInit()
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    plStatusLogMgr& mgr = plStatusLogMgr::GetInstance();
    s_DebugLog = mgr.CreateStatusLog(30, "plasmadbg.log", plStatusLog::kFilledBackground |
                 plStatusLog::kDeleteForMe | plStatusLog::kAlignToTop | plStatusLog::kTimestamp);
    hsSetStatusMessageProc(_StatusMessageProc);
#endif // defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
}

void plClient::ShowClientWindow()
{
    gWindow->ShowClientWindow();
}

void plClient::IResizeNativeDisplayDevice(int width, int height, bool windowed)
{
    gWindow->ResizeClientWindow(width, height, windowed);
}

void plClient::FlashWindow()
{
    gWindow->FlashWindow();
}

void plClient::IUpdateProgressIndicator(plOperationProgress* progress)
{
    gWindow->UpdateProgressIndicator(progress);
}

void plClient::IChangeResolution(int width, int height) {}


static void PumpMessageQueueProc()
{
    gWindow->ProcessEvents();
}

PF_CONSOLE_LINK_ALL();

static hsSsize_t getpassword(char* password)
{
    struct termios old_t, new_t;
    hsSsize_t nsize;

    if (tcgetattr(fileno(stdin), &old_t) != 0)
        return -1;

    new_t = old_t;
    new_t.c_lflag &= ~ECHO;

    if (tcsetattr(fileno(stdin), TCSAFLUSH, &new_t) != 0)
        return -1;

    nsize = fscanf(stdin, "%s", password);

    (void)tcsetattr(fileno(stdin), TCSAFLUSH, &old_t);
    fprintf(stdout, "\n");

    return nsize;
}

static void CalculateHash(const ST::string& username, const ST::string& password, ShaDigest& hash)
{
    //  Hash username and password before sending over the 'net.
    //  -- Legacy compatibility: @gametap (and other usernames with domains in them) need
    //     to be hashed differently.
    static const std::regex re_domain("[^@]+@([^.]+\\.)*([^.]+)\\.[^.]+");
    std::cmatch match;
    std::regex_search(username.c_str(), match, re_domain);
    if (match.empty() || ST::string(match[2].str()).compare_i("gametap") == 0) {
        //  Plain Usernames...
        plSHA1Checksum shasum(password.size(), reinterpret_cast<const uint8_t*>(password.c_str()));
        uint32_t* dest = reinterpret_cast<uint32_t*>(hash);
        const uint32_t* from = reinterpret_cast<const uint32_t*>(shasum.GetValue());

        dest[0] = hsToBE32(from[0]);
        dest[1] = hsToBE32(from[1]);
        dest[2] = hsToBE32(from[2]);
        dest[3] = hsToBE32(from[3]);
        dest[4] = hsToBE32(from[4]);
    }
    else {
        //  Domain-based Usernames...
        CryptHashPassword(username, password, hash);
    }
}

static size_t CurlCallback(void *buffer, size_t size, size_t nmemb, void *param)
{
    static char status[256];

    strncpy(status, (const char *)buffer, std::min<size_t>(size * nmemb, 256));
    status[255] = 0;

    fprintf(stdout, "%s\n\n", status);
    statusFlag.Signal();

    return size * nmemb;
}

static bool ConsoleLoginScreen(const ST::string& cliUsername)
{
    std::thread statusThread = std::thread([]() {
        ST::string statusUrl = GetServerStatusUrl();
        if (statusUrl.empty()) {
            statusFlag.Signal();
            return;
        }

        CURL* hCurl = curl_easy_init();

        // For reporting errors
        char curlError[CURL_ERROR_SIZE];
        curl_easy_setopt(hCurl, CURLOPT_ERRORBUFFER, curlError);
        curl_easy_setopt(hCurl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(hCurl, CURLOPT_MAXREDIRS, 5);
        curl_easy_setopt(hCurl, CURLOPT_URL, statusUrl.c_str());
        curl_easy_setopt(hCurl, CURLOPT_USERAGENT, "UruClient/1.0");
        curl_easy_setopt(hCurl, CURLOPT_WRITEFUNCTION, &CurlCallback);

        if (curl_easy_perform(hCurl) != 0) {
            fprintf(stderr, "%s\n\n", curlError);
            statusFlag.Signal();
        }

        curl_easy_cleanup(hCurl);
    });

    statusFlag.Wait();
    statusThread.join();

    ST::string username = cliUsername;
    if (cliUsername.empty()) {
        fprintf(stdout, "[Use Ctrl+D to cancel]\nUsername or Email: ");
        fflush(stdout);

        char tmpUsername[kMaxAccountNameLength];
        if (fscanf(stdin, "%s", tmpUsername) != 1) {
            return false;
        }
        username = tmpUsername;
    }

    pfPasswordStore* store = pfPasswordStore::Instance();
    ST::string password = store->GetPassword(username);

    if (!password.empty() && cliUsername.empty()) {
        fprintf(stdout, "Use saved password? [y/n] ");
        fflush(stdout);
        char c;
        fscanf(stdin, " %c", &c);
        if (c == 'n' || c == 'N') {
            password = ST_LITERAL("");
        }
        fprintf(stdout, "\n");
    }

    if (password.empty()) {
        fprintf(stdout, "Password: ");
        fflush(stdout);

        char tmpPassword[kMaxPasswordLength];
        getpassword(tmpPassword);
        password = tmpPassword;

        fprintf(stdout, "Save password? [y/n] ");
        fflush(stdout);
        char c;
        fscanf(stdin, " %c", &c);
        if (c == 'y' || c == 'Y') {
            store->SetPassword(username, password);
        }
        fprintf(stdout, "\n");
    }

    ShaDigest namePassHash;
    CalculateHash(username, password, namePassHash);

    NetCommSetAccountUsernamePassword(username, namePassHash);

    char16_t platform[] = u"linux";
    NetCommSetAuthTokenAndOS(nullptr, platform);

    if (!NetCliAuthQueryConnected())
        NetCommConnect();

    NetCommAuthenticate(nullptr);

    while (!NetCommIsLoginComplete()) {
        NetCommUpdate();
    }

    ENetError result = NetCommGetAuthResult();

    if (!IS_NET_SUCCESS(result)) {
        ST::printf(stdout, "{}\n", NetErrorToString(result));
        return false;
    }

    return true;
}

static uint32_t ParseRendererArgument(const ST::string& requested)
{
    using namespace ST::literals;

    static std::unordered_set<ST::string, ST::hash_i, ST::equal_i> dx_args {
        "directx"_st, "direct3d"_st, "dx"_st, "d3d"_st
    };

    static std::unordered_set<ST::string, ST::hash_i, ST::equal_i> gl_args {
        "opengl"_st, "gl"_st
    };

    if (dx_args.find(requested) != dx_args.end())
        return hsG3DDeviceSelector::kDevTypeDirect3D;

    if (gl_args.find(requested) != gl_args.end())
        return hsG3DDeviceSelector::kDevTypeOpenGL;

    return hsG3DDeviceSelector::kDevTypeUnknown;
}

int main(int argc, const char** argv)
{
    PF_CONSOLE_INIT_ALL();
    setlocale(LC_ALL, "");

    std::vector<ST::string> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        args.push_back(ST::string::from_utf8(argv[i]));
    }

    plCmdParser cmdParser(s_cmdLineArgs, std::size(s_cmdLineArgs));
    cmdParser.Parse(args);

    bool doIntroDialogs = true;
    ST::string cliUsername;
#ifndef PLASMA_EXTERNAL_RELEASE
    if (cmdParser.IsSpecified(kArgSkipLoginDialog))
        doIntroDialogs = false;
    if (cmdParser.IsSpecified(kArgLocalData))
    {
        gDataServerLocal = true;
        gPythonLocal = true;
    }
    if (cmdParser.IsSpecified(kArgLocalPython))
        gPythonLocal = true;
    if (cmdParser.IsSpecified(kArgLocalSDL))
        gSDLLocal = true;
    if (cmdParser.IsSpecified(kArgPlayerId))
        NetCommSetIniPlayerId(cmdParser.GetInt(kArgPlayerId));
    if (cmdParser.IsSpecified(kArgStartUpAgeName))
        NetCommSetIniStartUpAge(cmdParser.GetString(kArgStartUpAgeName));
    if (cmdParser.IsSpecified(kArgPvdFile))
        plPXSimulation::SetDefaultDebuggerEndpoint(cmdParser.GetString(kArgPvdFile));
    if (cmdParser.IsSpecified(kArgRenderer))
        gClient.SetRequestedRenderingBackend(ParseRendererArgument(cmdParser.GetString(kArgRenderer)));
    if (cmdParser.IsSpecified(kArgUsername))
        cliUsername = cmdParser.GetString(kArgUsername);
#endif

    plFileName serverIni = "server.ini";
    if (cmdParser.IsSpecified(kArgServerIni))
        serverIni = cmdParser.GetString(kArgServerIni);

    gWindow = new plClientWindow();
    if (!gWindow->PreInit()) {
        hsMessageBox(ST_LITERAL("Failed to pre-initialize plClient"), ST_LITERAL("Error"), hsMessageBoxNormal);
        return 1;
    }

    // Load an optional general.ini
    plFileName gipath = plFileName::Join(plFileSystem::GetInitPath(), "general.ini");
    FILE *generalini = plFileSystem::Open(gipath, "rb");
    if (generalini)
    {
        fclose(generalini);
        pfConsoleEngine tempConsole;
        tempConsole.ExecuteFile(gipath);
    }

    // Set up to log errors by using hsDebugMessage
    DebugInit();
    DebugMsg("Plasma 2.0.{}.{} - {}", PLASMA2_MAJOR_VERSION, PLASMA2_MINOR_VERSION, plProduct::ProductString());

    FILE *serverIniFile = plFileSystem::Open(serverIni, "rb");
    if (serverIniFile)
    {
        fclose(serverIniFile);
        pfConsoleEngine tempConsole;
        tempConsole.ExecuteFile(serverIni);
    }
    else
    {
        hsMessageBox(ST_LITERAL("No server.ini file found.  Please check your URU installation."), ST_LITERAL("Error"), hsMessageBoxNormal);
        return 1;
    }

    if (!gWindow->CreateClientWindow()) {
        hsMessageBox(ST_LITERAL("Failed to initialize plClient"), ST_LITERAL("Error"), hsMessageBoxNormal);
        return 1;
    }

    gClient.SetClientWindow(gWindow->GetWindowHandle());
    gClient.SetClientDisplay(gWindow->GetDisplayHandle());
    gClient.Init();

    NetCliAuthAutoReconnectEnable(false);
    NetCommStartup();

    curl_global_init(CURL_GLOBAL_ALL);

    // Login stuff
    if (!ConsoleLoginScreen(cliUsername)) {
        gClient.ShutdownStart();
        gClient.ShutdownEnd();
        NetCommShutdown();

        return 0;
    }

    curl_global_cleanup();

    NetCliAuthAutoReconnectEnable(true);

    // We should quite frankly be done initing the client by now. But, if not, spawn the good old
    // "Starting URU, please wait..." dialog (not so yay)
    if (!gClient.IsInited()) {
        gWindow->ShowLoadingSplashScreen();
        gClient.Wait();
        gWindow->HideLoadingSplashScreen();
    }

    // Main loop
    if (gClient && !gClient->GetDone()) {
        // Must be done here due to the plClient* dereference.
        if (cmdParser.IsSpecified(kArgSkipIntroMovies))
            gClient->SetFlag(plClient::kFlagSkipIntroMovies);

        gClient->SetMessagePumpProc(PumpMessageQueueProc);
        gClient.StartClient();

        while (true) {
            bool isDone = gClient->MainLoop() || gWindow->ProcessEvents();

            if (isDone) {
                gClient.ShutdownStart();
                break;
            }
        }
    }

    gClient.ShutdownEnd();
    NetCommShutdown();

    return 0;
}

