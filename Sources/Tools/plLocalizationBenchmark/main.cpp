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

#include <chrono>
#include <string_theory/stdio>

#include "plCmdParser.h"
#include "plFileSystem.h"

#include "pfLocalizationMgr/pfLocalizationMgr.h"

enum CmdLineArgs
{
    kArgCount,
    kArgDirectory,
};

static const plCmdArgDef s_cmdLineArgs[] = {
    { (kCmdTypeUint | kCmdArgFlagged), "Count", kArgCount },
    { (kCmdTypeString | kCmdArgOptional), "Directory", kArgDirectory },
};

using ClockT = std::chrono::steady_clock;

#ifdef HS_BUILD_FOR_WIN32
int wmain(int argc, const wchar_t* argv[])
#else
int main(int argc, const char* argv[])
#endif
{
    std::vector<ST::string> args;
    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);

    plCmdParser parser(s_cmdLineArgs, std::size(s_cmdLineArgs));
    parser.Parse(args);

    plFileName locDir;
    if (parser.IsSpecified(kArgDirectory))
        locDir = parser.GetString(kArgDirectory);
    else
        locDir = plFileSystem::GetCWD();

    if (!locDir.IsValid() || !plFileInfo(locDir).IsDirectory()) {
        ST::printf(stderr, "The directory '{}' does not exist.\n", locDir);
        return 1;
    }

    int32_t count = 1000;
    if (parser.IsSpecified(kArgCount))
        count = parser.GetInt(kArgCount);
    if (count <= 0) {
        ST::printf(stderr, "Cannot iterate less than 1 time.\n");
        return 1;
    }

    ST::printf("Parsing the localization database from '{}'...\n", locDir);

    auto elapsed = ClockT::duration::zero();
    for (int32_t i = 0; i < count; ++i) {
        ST::printf("\r... Running iteration {} of {}", i + 1, count);
        auto begin = ClockT::now();
        pfLocalizationMgr::Initialize(locDir);
        auto end = ClockT::now();
        elapsed += end - begin;

        // Who cares how long this takes...
        pfLocalizationMgr::Shutdown();
    }

    ST::printf("\n... Done!\n\n");

    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
    auto avg_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed / count);
    auto total_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed);
    auto avg_sec = std::chrono::duration_cast<std::chrono::duration<double>>(elapsed / count);

    ST::printf("Results:\n");
    ST::printf("Total: {.4f} seconds ({} us)\n", total_sec.count(), total_us.count());
    ST::printf("Average: {.4f} seconds ({} us)\n", avg_sec.count(), avg_us.count());
    ST::printf("Have a nice day!\n");
    return 0;
}
