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

#include "HeadSpin.h"
#include "plCmdParser.h"
#include "plFileSystem.h"
#include "plProduct.h"

#include "plFilePatcher.h"

#include <cstdio>
#include <vector>
#include <string_theory/format>
#include <string_theory/stdio>

#if HS_BUILD_FOR_WIN32
#   include "hsWindows.h"
#   include <io.h>
#   define isatty _isatty

    void GetConsoleWidth(size_t& width)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
        width = size_t(csbi.srWindow.Right - csbi.srWindow.Left);
    }
#else
#   include <sys/ioctl.h>
#   include <sys/termios.h>
#   include <unistd.h>

    void GetConsoleWidth(size_t& width)
    {
        struct winsize w;
        ioctl(fileno(stdout), TIOCGWINSZ, &w);
        if (w.ws_col == 0) {
            w.ws_col = 80;
        }
        width = size_t(w.ws_col) - 1;
    }
#endif

static void PrintUsage(const ST::string& exeName)
{
    ST::printf("Plasma File Patcher\n{}\n\n", plProduct::ProductString());

    ST::printf("File patcher for the Plasma client and game data.\n\n");

    ST::printf("Usage:\n");
    ST::printf("\t{} [options]\n\n", exeName);

    ST::printf("Available options:\n");
    ST::printf("\t--ServerIni file.ini\tSpecify the server config file\n");
    ST::printf("\t--DataOnly\t\tPatch only the game data files\n");
    ST::printf("\t--ClientOnly\t\tPatch only the client executable files\n");
    ST::printf("\t--help\t\t\tDisplay this help message\n");
    ST::printf("\t--quiet\t\t\tDo not display download progress\n\n");

    ST::printf("By default, updates to both game data and client executables will be\ndownloaded, and it will use \"server.ini\" for server connection details.\n");
}

static size_t consoleWidth = 0;
static ST::string lastProgress;

static void FileDownloadBegin(const plFileName& file)
{
    // To account for terminal resizing, we want to refresh this value
    // periodically, but not every time we draw to the screen, so let's grab it
    // whenever we start downloading a new file (which is probably still
    // excessive tbh)
    GetConsoleWidth(consoleWidth);

    size_t fill = consoleWidth - file.GetSize();
    ST::printf("\r{}{}\n{}", file, ST::string::fill(fill, ' '), lastProgress);
    fflush(stdout);
}

static void FileDownloadProgress(uint64_t bytesDown, uint64_t bytesTotal, const ST::string& stats)
{
    size_t availWidth = consoleWidth - stats.size() - 3;
    size_t progWidth = size_t(double(bytesDown) / double(bytesTotal) * availWidth);

    lastProgress = ST::format("|{}{}| {}", ST::string::fill(progWidth, '='), ST::string::fill(availWidth - progWidth, ' '), stats);
    ST::printf("\r{}", lastProgress);
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    enum { kArgServerIni, kArgDataOnly, kArgClientOnly, kArgQuiet, kArgHelp1, kArgHelp2 };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgFlagged | kCmdTypeString,  "ServerIni",    kArgServerIni },
        { kCmdArgFlagged | kCmdTypeBool,    "DataOnly",     kArgDataOnly },
        { kCmdArgFlagged | kCmdTypeBool,    "ClientOnly",   kArgClientOnly },
        { kCmdArgFlagged | kCmdTypeBool,    "quiet",        kArgQuiet },
        { kCmdArgFlagged | kCmdTypeBool,    "help",         kArgHelp1 },
        { kCmdArgFlagged | kCmdTypeBool,    "?",            kArgHelp2 },
    };

    std::vector<ST::string> args;
    args.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        args.emplace_back(ST::string::from_utf8(argv[i]));
    }

    plCmdParser cmdParser(cmdLineArgs, std::size(cmdLineArgs));
    if (cmdParser.Parse(args)) {
        if (cmdParser.GetBool(kArgHelp1) || cmdParser.GetBool(kArgHelp2)) {
            plFileName exeName = plFileName(cmdParser.GetProgramName());
            PrintUsage(exeName.GetFileName());
            return 0;
        }
    } else {
        ST::printf(stderr, "An error occurred while parsing the provided arguments.\n");
        ST::printf(stderr, "Use the --help option to display usage information.\n");
        return 1;
    }

    plFileName serverIni = ST_LITERAL("server.ini");
    if (cmdParser.IsSpecified(kArgServerIni))
        serverIni = cmdParser.GetString(kArgServerIni);

    plFilePatcher patcher(serverIni);
    uint32_t flags = 0;

    if (cmdParser.GetBool(kArgDataOnly))
        flags |= plFilePatcher::kPatchData;
    if (cmdParser.GetBool(kArgClientOnly))
        flags |= plFilePatcher::kPatchClient;

    // The options aren't mutually exclusive, so if we've ended up telling it
    // not to patch anything, we'll set it back to patching everything.
    if (!flags)
        flags = plFilePatcher::kPatchEverything;

    patcher.SetPatcherFlags(flags);

    if (!cmdParser.GetBool(kArgQuiet) && isatty(fileno(stdout))) {
        patcher.SetDownloadBeginCallback(&FileDownloadBegin);
        patcher.SetProgressCallback(&FileDownloadProgress);
    }

    bool result = patcher.Patch();
    if (!result) {
        ST::printf(stderr, "File Patching failed: {}\n", patcher.GetError());
        return 1;
    }

    if (!cmdParser.GetBool(kArgQuiet) && isatty(fileno(stdout))) {
        ST::printf("\r{}\r", ST::string::fill(consoleWidth, ' '));
        fflush(stdout);
    }
    return 0;
}
