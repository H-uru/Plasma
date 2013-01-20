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

#include "pnEncryption/plChecksum.h"
#include "plResMgr/plPagePatcher.h"
#include "plResMgr/plRegistryNode.h"
#include "plStatusLog/plStatusLog.h"

bool g_Validate = false;
plFileName g_OldPage, g_NewPage;
plFileName g_PatchPage = "prp.pat";
plStatusLog* g_Log;

static void DoMd5(const plFileName& file, const char* tag)
{
    plMD5Checksum hash(file);
    g_Log->AddLineF("%s Page: %s", tag, hash.GetAsHexString());
}

static bool CheckFile(const plFileName& file)
{
    return plFileInfo(file).Exists();
}

static void PrintUsage()
{
    g_Log->AddLine("plPatchUtil [-validate] <old> <new> [output]\n");
    g_Log->AddLine("Generates a patch for a given pair of plRegistryPageNodes");
    g_Log->AddLine("Optionally will attempt to eat dogfood to test the implementation");
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

    g_Log = plStatusLogMgr::GetInstance().CreateStatusLog(
        0,
        "patcher.log",
        plStatusLog::kStdout | plStatusLog::kDeleteForMe
    );

    if (!ParseArguments(argc, argv))
        return 1;

    if (!CheckFile(g_OldPage))
    {
        g_Log->AddLineF("File Not Found: %s", g_OldPage.AsString().c_str());
        return 1;
    }
    if (!CheckFile(g_NewPage))
    {
        g_Log->AddLineF("File Not Found: %s", g_NewPage.AsString().c_str());
        return 1;
    }

    // Spit out md5sums
    DoMd5(g_OldPage, "Old");
    DoMd5(g_NewPage, "New");

    // Do the dirty.
    plRegistryPageNode* patch = plPagePatcher::GeneratePatch(g_OldPage, g_NewPage);
    if (!patch)
        return 1;
    plPagePatcher::WriteAndClear(patch, g_PatchPage);
    delete patch;

    if (g_Validate)
    {
        plFileName fn = g_PatchPage + ".prp";
        plRegistryPageNode* roundtrip = plPagePatcher::PatchPage(g_OldPage, g_PatchPage);
        if (!roundtrip)
            return 1;
        plPagePatcher::WriteAndClear(roundtrip, fn);
        delete roundtrip;

        // Verification
        if (plPagePatcher::ValidatePatch(g_NewPage, fn))
            return 0;
        else
            return 1;
    }

    return 0;
}

