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

#include "plClientUnix.h"
#include "plInputSDL.h"
#include "plLoginSDL.h"

#include "HeadSpin.h"
#include "hsDebug.h"
#include "hsEndian.h"
#include "hsFILELock.h"
#include "plCmdParser.h"
#include "plFileSystem.h"
#include "plPipeline.h"
#include "plProduct.h"

#include "plClient.h"
#include "plClientLoader.h"

#include "pnEncryption/plChallengeHash.h"
#include "pnNetBase/pnNbSrvs.h"

#include "plFile/plEncryptedStream.h"
#include "plMessage/plDisplayScaleChangedMsg.h"
#include "plMessageBox/hsMessageBox.h"
#include "plNetClient/plNetClientMgr.h"
#include "plNetClientComm/plNetClientComm.h"
#include "plNetGameLib/plNetGameLib.h"
#include "plPhysX/plPXSimulation.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plPipeline/pl3DPipeline.h"
#include "plResMgr/plLocalization.h"
#include "plResMgr/plVersion.h"
#include "plStatusLog/plStatusLog.h"

#include "pfConsoleCore/pfConsoleEngine.h"
#include "pfConsoleCore/pfServerIni.h"
#include "pfPasswordStore/pfPasswordStore.h"

#ifdef USE_X11
#   include "pfDisplayHelpers/plX11DisplayHelper.h"
#endif

#ifdef USE_WAYLAND
#   include "pfDisplayHelpers/plWaylandDisplayHelper.h"
#endif

#include <SDL3/SDL.h>
#include <curl/curl.h>

#include <string_theory/format>
#include <string_theory/string>

#include <termios.h>
#include <unistd.h>

#include <cstdlib>
#include <regex>
#include <system_error>
#include <unordered_map>

//
// Globals
//
plClientLoader gClient;
SDL_Window*    gWindow = nullptr;

extern bool gDataServerLocal;
extern bool gPythonLocal;
extern bool gSDLLocal;

//
// Command line
//
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
    kArgX11
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
    { kCmdArgFlagged  | kCmdTypeBool,       "X11",             kArgX11 },
};

static uint32_t ParseRendererArgument(const ST::string& requested)
{
    using namespace ST::literals;

    static std::unordered_map<ST::string, uint32_t, ST::hash_i, ST::equal_i> args{
        {"opengl"_st, hsG3DDeviceSelector::kDevTypeOpenGL},
        {"gl"_st, hsG3DDeviceSelector::kDevTypeOpenGL},
        {"vulkan"_st, hsG3DDeviceSelector::kDevTypeVulkan},
        {"vk"_st, hsG3DDeviceSelector::kDevTypeVulkan}};

    auto it = args.find(requested);
    if (it != args.end())
        return it->second;
    return hsG3DDeviceSelector::kDevTypeUnknown;
}

//
// Debug logging
//
static plStatusLog* s_debugLog = nullptr;

static void IStatusMessageProc(const ST::string& msg)
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    s_debugLog->AddLine(msg);

    // Also to the terminal. Vulkan validation output and the renderer's own
    // warnings come through here, and a client that files them away where
    // nobody is looking is a client that appears to fail silently.
    fputs(msg.c_str(), stderr);
    fputc('\n', stderr);
    fflush(stderr);
#endif
}

static void DebugInit()
{
#if defined(HS_DEBUGGING) || !defined(PLASMA_EXTERNAL_RELEASE)
    plStatusLogMgr& mgr = plStatusLogMgr::GetInstance();
    s_debugLog = mgr.CreateStatusLog(30, "plasmadbg.log", plStatusLog::kFilledBackground |
                 plStatusLog::kDeleteForMe | plStatusLog::kAlignToTop | plStatusLog::kTimestamp);
    hsSetStatusMessageProc(IStatusMessageProc);
#endif
}

//
// Display helper
//
// plClient::InitPipeline and IDetectAudioVideoSettings both dereference
// plDisplayHelper::GetInstance(), so this has to be set before the client is
// inited. Which helper we pick also decides whether the GL enumerator is
// willing to try GLX -- see plGLEnumerate::Enumerate.
//
static bool ISetupDisplayHelper()
{
    plDisplayHelper* helper = nullptr;
    const char* videoDriver = SDL_GetCurrentVideoDriver();

#ifdef USE_WAYLAND
    if (videoDriver && strcmp(videoDriver, "wayland") == 0)
        helper = new plWaylandDisplayHelper();
#endif

#ifdef USE_X11
    if (!helper && videoDriver && strcmp(videoDriver, "x11") == 0)
        helper = new plX11DisplayHelper();
#endif

    if (!helper) {
        hsMessageBox(ST::format("Unsupported SDL video driver: {}",
                                videoDriver ? videoDriver : "(none)"),
                     ST_LITERAL("Error"), hsMessageBoxNormal);
        return false;
    }

    plDisplayHelper::SetInstance(helper);
    return true;
}

//
// Single instance
//
// Two clients sharing a user data directory will fight over the log files and
// the saved account name. Windows uses a named mutex for this; the portable
// equivalent is an advisory lock on a file that lives beside that data. The
// lock is released by the kernel when the process exits, however it exits.
//
static bool IClaimSingleInstance()
{
    plFileSystem::CreateDir(plFileSystem::GetUserDataPath(), true);

    // Deliberately never closed: the lock lives as long as the process does.
    static FILE* lockFile = plFileSystem::Open(
        plFileName::Join(plFileSystem::GetUserDataPath(), "plClient.lock"), "wb");

    // No lock file means no lock, which is worth a log line but not worth
    // refusing to start over.
    if (!lockFile) {
        hsStatusMessage("Could not open the instance lock file.");
        return true;
    }

    static hsFILELock lock(lockFile);
    try {
        return lock.try_lock();
    } catch (const std::system_error& exc) {
        hsStatusMessageF("Could not lock the instance lock file: {}", exc.what());
        return true;
    }
}

//
// Optional general.ini
//
// Console commands that have to run before the client is inited, most notably
// App.SetLanguage. Matches the Win32 frontend.
//
static void IExecGeneralIni()
{
    plFileName path = plFileName::Join(plFileSystem::GetInitPath(), "general.ini");
    if (!plFileInfo(path).Exists())
        return;

    pfConsoleEngine tempConsole;
    tempConsole.ExecuteFile(path);
}

//
// Credentials
//
// There is no login dialog on this platform. The account name lives in a
// one-line file under the init path and the password lives in the system
// keyring via pfPasswordStore, exactly as on the other platforms. If either is
// missing we ask on the terminal and, once the server has accepted them, save
// the answers.
//
static plFileName IAccountNameFile()
{
    return plFileName::Join(plFileSystem::GetInitPath(), "login.cfg");
}

static ST::string ILoadAccountName()
{
    FILE* fp = plFileSystem::Open(IAccountNameFile(), "r");
    if (!fp)
        return {};

    char buf[kMaxAccountNameLength];
    memset(buf, 0, sizeof(buf));
    char* line = fgets(buf, sizeof(buf), fp);
    fclose(fp);
    if (!line)
        return {};

    return ST::string(buf).before_first('\n').trim();
}

static void ISaveAccountName(const ST::string& account)
{
    plFileSystem::CreateDir(plFileSystem::GetInitPath(), true);

    FILE* fp = plFileSystem::Open(IAccountNameFile(), "w");
    if (!fp) {
        hsStatusMessage("Could not save the account name.");
        return;
    }
    fprintf(fp, "%s\n", account.c_str());
    fclose(fp);
}

static ST::string IPromptLine(const char* prompt, bool echo)
{
    fputs(prompt, stdout);
    fflush(stdout);

    termios saved{};
    const bool isTTY = isatty(STDIN_FILENO) != 0;
    if (!echo && isTTY && tcgetattr(STDIN_FILENO, &saved) == 0) {
        termios quiet = saved;
        quiet.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &quiet);
    } else {
        // Nothing to restore.
        memset(&saved, 0, sizeof(saved));
    }

    char buf[256];
    memset(buf, 0, sizeof(buf));
    char* line = fgets(buf, sizeof(buf), stdin);

    if (!echo && isTTY) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved);
        fputc('\n', stdout);
    }

    if (!line)
        return {};
    return ST::string(buf).before_first('\n').trim();
}

static void IHashPassword(const ST::string& username, const ST::string& password,
                          ShaDigest& namePassHash)
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
        uint32_t* dest = reinterpret_cast<uint32_t*>(namePassHash);
        const uint32_t* from = reinterpret_cast<const uint32_t*>(shasum.GetValue());

        dest[0] = hsToBE32(from[0]);
        dest[1] = hsToBE32(from[1]);
        dest[2] = hsToBE32(from[2]);
        dest[3] = hsToBE32(from[3]);
        dest[4] = hsToBE32(from[4]);
    } else {
        //  Domain-based Usernames...
        CryptHashPassword(username, password, namePassHash);
    }
}

/**
 * Writes the language choice back out for the next run.
 *
 * The Win32 login dialog does the same thing at the same point, and this is the
 * file IExecGeneralIni() reads on the way in.
 */
static void ISaveGeneralIni()
{
    plFileSystem::CreateDir(plFileSystem::GetInitPath(), true);

    plFileName path = plFileName::Join(plFileSystem::GetInitPath(), "general.ini");
    ST::string contents = ST::format("App.SetLanguage {}\n",
                                     plLocalization::GetLanguageName(plLocalization::GetLanguage()));

    std::unique_ptr<hsStream> ini = plEncryptedStream::OpenEncryptedFileWrite(path);
    if (ini)
        ini->WriteString(contents);
    else
        hsStatusMessage("Could not save general.ini.");
}

/**
 * Collects the account name and password and hands them to the net layer.
 *
 * `forcePrompt` ignores the stored password and asks again -- what a failed
 * attempt needs, since the stored one is exactly what just did not work. The
 * saved account name is offered as the default rather than discarded, because
 * a rejected login is far more often the wrong password than the wrong name.
 *
 * Nothing is written to the password store here. A password is only worth
 * keeping once the server has agreed with it; saving it up front means one typo
 * is cached forever, every later run fails with it, and there is no way to
 * correct it from inside the client.
 */
static bool ISetupCredentials(bool forcePrompt, ST::string* outAccount,
                              ST::string* outPassword, bool* outRemember,
                              plLoginSDL* login, const ST::string& failure,
                              bool allowSilent)
{
    ST::string account = ILoadAccountName();
    ST::string password;

    if (!forcePrompt && !account.empty())
        password = pfPasswordStore::Instance()->GetPassword(account);

    if (login) {
        // A stored password that has already been rejected is not worth
        // offering back, but the account name still is.
        if (forcePrompt)
            password = ST::string();

        *outRemember = !password.empty();

        // The window otherwise always comes up, even when both fields could be
        // filled in from what was saved -- it is the only chance anyone gets to
        // log in as somebody else, change the language, or stop the password
        // being remembered. -SkipLoginDialog is what trades that away for going
        // straight in, and only when there is a complete saved login to go in
        // with. A rejection falls back to the window on the next attempt, all
        // matching winmain.cpp:1223-1241.
        if (!allowSilent || account.empty() || password.empty()) {
            login->SetMessage(failure);
            if (!login->Prompt(&account, &password, outRemember))
                return false;
            ISaveGeneralIni();
        }
    } else {
        if (account.empty()) {
            account = IPromptLine("Account name: ", true);
            if (account.empty())
                return false;
        } else if (forcePrompt) {
            // Enter keeps the saved name. Answering the retry with a blank line
            // should not be the thing that quits the client.
            ST::string answer = IPromptLine(ST::format("Account name [{}]: ", account).c_str(), true);
            if (!answer.empty())
                account = answer;
        }

        if (forcePrompt)
            password = ST::string();

        if (password.empty()) {
            password = IPromptLine(ST::format("Password for {}: ", account).c_str(), false);
            if (password.empty())
                return false;
        }

        *outRemember = true;
    }

    ShaDigest namePassHash;
    IHashPassword(account, password, namePassHash);

    NetCommSetAccountUsernamePassword(account, namePassHash);

    // FIXME: The server has no OS string for this platform. Until one exists,
    // claim to be the Windows client so existing shards accept the login.
    NetCommSetAuthTokenAndOS(nullptr, u"win");

    *outAccount = account;
    *outPassword = password;
    return true;
}

static bool IAuthenticate(ENetError* result, plLoginSDL* login)
{
    if (!NetCliAuthQueryConnected())
        NetCommConnect();

    NetCommAuthenticate(nullptr);

    while (!NetCommIsLoginComplete()) {
        NetCommUpdate();

        // Keep the login window drawing while the server thinks about it,
        // otherwise it sits there looking hung for the whole round trip.
        if (login)
            login->Tick(ST_LITERAL("Authenticating..."));

        SDL_Delay(10);
    }

    *result = NetCommGetAuthResult();
    return !IS_NET_ERROR(*result);
}

//
// Event pump
//
void PumpSDLEvents()
{
    // The engine calls this as its message pump proc, which can happen on the
    // plClientLoader worker thread during resource loading. SDL_PollEvent is
    // main-thread only, and the main thread may be blocked waiting on that very
    // worker, so bouncing the call over would deadlock. Drop it instead; the
    // window is not visible during that window of time anyway.
    if (!SDL_IsMainThread())
        return;

    SDL_Event evt;
    while (SDL_PollEvent(&evt))
        plInputSDL::HandleEvent(evt);
}

PF_CONSOLE_LINK_ALL()

int main(int argc, char* argv[])
{
    PF_CONSOLE_INIT_ALL()

    std::vector<ST::string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; i++)
        args.push_back(ST::string::from_utf8(argv[i]));

    plCmdParser cmdParser(s_cmdLineArgs, std::size(s_cmdLineArgs));
    cmdParser.Parse(args);

    // RenderDoc cannot capture the native Wayland presentation path reliably.
    // This must be selected before SDL_INIT_VIDEO so SDL creates an X11 window
    // and the Vulkan backend consequently creates an Xlib/XCB surface.
    if (cmdParser.IsSpecified(kArgX11))
        SDL_SetHintWithPriority(SDL_HINT_VIDEO_DRIVER, "x11", SDL_HINT_OVERRIDE);

#ifndef PLASMA_EXTERNAL_RELEASE
    if (cmdParser.IsSpecified(kArgLocalData)) {
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
#endif

    plFileName serverIni = "server.ini";
    if (cmdParser.IsSpecified(kArgServerIni))
        serverIni = cmdParser.GetString(kArgServerIni);

    // Redirect hsStatusMessage to plasmadbg.log
    DebugInit();
    hsStatusMessageF("Plasma 2.0.{}.{} - {}", PLASMA2_MAJOR_VERSION, PLASMA2_MINOR_VERSION,
                     plProduct::ProductString());

    IExecGeneralIni();

    if (!plFileInfo(serverIni).Exists()) {
        hsMessageBox(ST_LITERAL("No server.ini file found. Please check your URU installation."),
                     ST_LITERAL("Error"), hsMessageBoxNormal);
        return EXIT_FAILURE;
    }

    try {
        pfServerIni::Load(serverIni);
    } catch (const pfServerIniParseException& exc) {
        hsMessageBox(ST::format("Error in server.ini file. Please check your URU installation.\n{}",
                                exc.what()), ST_LITERAL("Error"), hsMessageBoxNormal);
        return EXIT_FAILURE;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        hsMessageBox(ST::format("Failed to initialize SDL: {}", SDL_GetError()),
                     ST_LITERAL("Error"), hsMessageBoxNormal);
        return EXIT_FAILURE;
    }

    hsStatusMessageF("SDL video driver: {}", SDL_GetCurrentVideoDriver());

    // Now that there is a video subsystem, a refusal can actually say why.
    if (!IClaimSingleInstance()) {
        hsMessageBox(ST::format("Another copy of {} is already running.", plProduct::LongName()),
                     ST_LITERAL("Error"), hsMessageBoxNormal);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // The player is holding a mouse, not typing, so the idle timer never resets.
    SDL_DisableScreenSaver();

    if (!ISetupDisplayHelper()) {
        SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_WindowFlags windowFlags = SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE |
                                  SDL_WINDOW_HIGH_PIXEL_DENSITY;
#ifdef PLASMA_PIPELINE_VULKAN
    // The window has to commit to a rendering API before the device selector has
    // run, so a Vulkan-capable build always asks for a Vulkan window. This also
    // gets SDL to load the Vulkan library, which the enumerator needs.
    windowFlags |= SDL_WINDOW_VULKAN;
#endif

    // Created hidden; plClientLoader::StartClient() reveals it by way of
    // plClient::ShowClientWindow(), matching the Win32 frontend.
    gWindow = SDL_CreateWindow(plProduct::LongName().c_str(),
                               hsG3DDeviceSelector::kDefaultWidth,
                               hsG3DDeviceSelector::kDefaultHeight,
                               windowFlags);
    if (!gWindow) {
        hsMessageBox(ST::format("Failed to create the game window: {}", SDL_GetError()),
                     ST_LITERAL("Error"), hsMessageBoxNormal);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Begin initializing the client in the background.
    gClient.SetClientWindow(static_cast<hsWindowHndl>(gWindow));
    gClient.SetClientDisplay(plDisplayHelper::GetInstance()->DefaultDisplay());
    gClient.Init();

    NetCliAuthAutoReconnectEnable(false);
    NetCommStartup();
    curl_global_init(CURL_GLOBAL_ALL);

    // A rejected password is nearly always a typo or a stale saved one, and
    // quitting the whole client over it means relaunching to try again -- with
    // the same stored password, so it fails identically. Ask again instead, for
    // as long as answers keep coming; an empty line at either prompt is how you
    // say you are done.
    bool needExit = true;
    {
        // The terminal prompt stays as the fallback for when there is no
        // usable window -- a broken display, or a build being driven from a
        // script -- but the window is what a player should get.
        plLoginSDL loginWindow;
        plLoginSDL* login = loginWindow.IsValid() ? &loginWindow : nullptr;
        ST::string failure;

        const bool skipLoginDialog = cmdParser.IsSpecified(kArgSkipLoginDialog);

        for (unsigned attempt = 0; ; ++attempt) {
            ST::string account, password;
            bool remember = true;
            if (!ISetupCredentials(attempt > 0, &account, &password, &remember, login, failure,
                                   skipLoginDialog && attempt == 0))
                break;

            ENetError auth;
            if (IAuthenticate(&auth, login)) {
                // Only now is it known to be worth remembering.
                ISaveAccountName(account);
                if (remember)
                    pfPasswordStore::Instance()->SetPassword(account, password);
                else
                    pfPasswordStore::Instance()->SetPassword(account, ST::string());
                needExit = false;
                break;
            }

            failure = ST::format("Authentication failed: {}", NetErrorToString(auth));
            if (!login)
                fprintf(stderr, "%s\n", failure.c_str());
        }
    }

    curl_global_cleanup();

    if (needExit) {
        gClient.ShutdownStart();
        gClient.ShutdownEnd();
        NetCommShutdown();
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    NetCliAuthAutoReconnectEnable(true);

    // Wait out the rest of client init. The window is still hidden, so there is
    // nothing to pump here.
    if (!gClient.IsInited())
        gClient.Wait();

    if (!gClient) {
        hsMessageBox(ST_LITERAL("Failed to initialize plClient"), ST_LITERAL("Error"),
                     hsMessageBoxNormal);
        NetCommShutdown();
        SDL_DestroyWindow(gWindow);
        SDL_Quit();
        return EXIT_FAILURE;
    }

    // Tell the engine about the backing scale before anything draws.
    {
        plDisplayScaleChangedMsg* msg =
            new plDisplayScaleChangedMsg(SDL_GetWindowPixelDensity(gWindow));
        msg->Send();
    }

    if (cmdParser.IsSpecified(kArgSkipIntroMovies))
        gClient->SetFlag(plClient::kFlagSkipIntroMovies);

    gClient->WindowActivate(true);
    gClient->SetMessagePumpProc(PumpSDLEvents);
    gClient.StartClient();

    SDL_StartTextInput(gWindow);

    // ShowClientWindow synchronized the first frame before BeginGame. Drain
    // anything generated while the intro was playing before entering the
    // regular event-and-render loop.
    PumpSDLEvents();

    while (!gClient->GetDone()) {
        PumpSDLEvents();
        if (gClient->GetDone())
            break;
        gClient->MainLoop();
    }

    gClient.ShutdownStart();
    gClient.ShutdownEnd();
    NetCommShutdown();

    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    SDL_Quit();

    return EXIT_SUCCESS;
}
