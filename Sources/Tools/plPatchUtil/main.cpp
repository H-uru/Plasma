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
#include "plFileSystem.h"

#include <iostream>

#include "pnEncryption/plChecksum.h"
#include "plResMgr/plPagePatcher.h"
#include "plResMgr/plRegistryNode.h"
#include "plStatusLog/plStatusLog.h"

// Begin Stupid Shit
#include "pnAllCreatables.h"
#include "plResMgr/plResMgrCreatable.h"

#include "plMessage/plResMgrHelperMsg.h"
REGISTER_NONCREATABLE(plResMgrHelperMsg);

#include "plMessage/plResPatcherMsg.h"
REGISTER_NONCREATABLE(plResPatcherMsg);
// End Stupid Shit

bool g_Validate = false;
plFileName g_OldPage, g_NewPage;
plFileName g_PatchPage = "prp.pat";

static void DoMd5(const plFileName& file, const char* tag, plMD5Checksum* md5=nullptr)
{
    plMD5Checksum hash;
    if (!md5)
        md5 = &hash;
    md5->CalcFromFile(file);
    std::cout << tag << " Page: " << md5->GetAsHexString() << std::endl;
}

static bool CheckFile(const plFileName& file)
{
    return plFileInfo(file).Exists();
}

static void PrintUsage()
{
    std::cout << "plPatchUtil [-validate] <old> <new> [output]" << std::endl << std::endl;
    std::cout << "Generates a patch for a given pair of plRegistryPageNodes" << std::endl;
    std::cout << "Optionally will attempt to eat dogfood to test the implementation";
}

static bool ParseArguments(int argc, char** argv)
{
    // In lieu of a real argument parsing library (pnUtCmd doesn't look linux compatible)
    if (argc < 2)
    {
        PrintUsage();
        return false;
    }

    int nextArg = 1;
    if (stricmp(argv[nextArg], "-validate") == 0)
    {
        g_Validate = true;
        ++nextArg;
    }

    if ((argc - nextArg) < 2)
    {
        PrintUsage();
        return false;
    }

    g_OldPage = plFileName(argv[nextArg]); ++nextArg;
    g_NewPage = plFileName(argv[nextArg]); ++nextArg;
    if (argc > nextArg)
        g_PatchPage = plFileName(argv[nextArg]);
    return true;
}

int main(int argc, char** argv)
{
    if (!ParseArguments(argc, argv))
        return 1;

    if (!CheckFile(g_OldPage))
    {
        std::cout << "File Not Found: " << g_OldPage.AsString().c_str();
        return 1;
    }
    if (!CheckFile(g_NewPage))
    {
        std::cout << "File Not Found: " << g_NewPage.AsString().c_str();
        return 1;
    }

    plStatusLogMgr::GetInstance().CreateStatusLog(
        0,
        "patcher.log",
        plStatusLog::kStdout | plStatusLog::kDeleteForMe
    );

    // Spit out md5sums
    DoMd5(g_OldPage, "Old");
    plMD5Checksum newMD5;
    DoMd5(g_NewPage, "New", &newMD5);
    std::cout << std::endl;

    // Do the dirty.
    plRegistryPageNode* patch = plPagePatcher::GeneratePatch(g_OldPage, g_NewPage);
    if (!patch)
        return 1;
    plPagePatcher::WriteAndClear(patch, g_PatchPage);
    delete patch;

    if (g_Validate)
    {
        std::cout << std::endl;
        plFileName fn = g_PatchPage + ".prp";
        plRegistryPageNode* roundtrip = plPagePatcher::PatchPage(g_OldPage, g_PatchPage);
        if (!roundtrip)
            return 1;
        plPagePatcher::WriteAndClear(roundtrip, fn);
        delete roundtrip;
        std::cout << std::endl;

        // Verify hash
        plMD5Checksum patchedMD5;
        DoMd5(fn, "Validation", &patchedMD5);
        if (newMD5 == patchedMD5)
            std::cout << "Validation success!";
        else
            std::cout << "Validation FAILED";
    }

    return 0;
}

