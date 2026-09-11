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
#include "hsMain.inl"
#include "plProduct.h"

#include <exception>
#include <string_theory/stdio>
#include <Python.h>

#include "pfPython/cyPythonInterface.h"
#include "pfPython/pyObjectRef.h"
#include "pfPython/plPythonCallable.h"

class CWDGuard
{
    plFileName fOldCWD;

public:
    CWDGuard(const plFileName& newCWD)
        : fOldCWD(plFileSystem::GetCWD())
    {
        plFileSystem::SetCWD(newCWD);
    }

    CWDGuard(const CWDGuard&) = delete;
    CWDGuard(CWDGuard&&) = delete;

    ~CWDGuard()
    {
        plFileSystem::SetCWD(fOldCWD);
    }
};

static void PrintHeader()
{
    ST::printf("plGeneratePythonStubs\n");
    ST::printf("{}\n\n", plProduct::ProductString());
}

static void PrintHelp()
{
    ST::printf("Available options:\n");
    ST::printf("\t--Client [directory]\tSpecify the directory containing the Plasma game client\n");
    ST::printf("\t--Stubs [directory]\tSpecify the directory to output the Python stubs\n");
    ST::printf("\t--help\t\t\tDisplay this help message.\n");
}

static bool CheckScriptsDir(const plFileName& scriptsDir)
{
    if (!scriptsDir.IsValid())
        return false;

    plFileInfo info(scriptsDir);
    if (!info.Exists())
        return false;
    if (!info.IsDirectory())
        return false;

    plFileInfo systemDirInfo(plFileName::Join(scriptsDir, "system"));
    if (!systemDirInfo.Exists())
        return false;
    if (!systemDirInfo.IsDirectory())
        return false;

    return true;
}

static int GenerateStubs(const plFileName& clientPath, const plFileName& stubsDir)
{
    try {
        // CWD *must* be the client directory for initPython() to work properly.
        {
            CWDGuard cwdGuard(clientPath);
            PythonInterface::initPython();
        }

        pyObjectRef stubModule = PythonInterface::ImportModule("generate_stubs");
        if (!stubModule)
            throw std::runtime_error("Failed to import generate_stubs.py");

        pyObjectRef generateFunc = PythonInterface::GetModuleItem("run", stubModule.Get());
        if (!generateFunc)
            throw std::runtime_error("Could not find generate_stubs.run()");

        if (!PyCallable_Check(generateFunc.Get()))
            throw std::runtime_error("generate_stubs.run is not callable!");

        ST::printf("Generating stubs...\n");
        pyObjectRef result;
        if (stubsDir.IsValid())
            result = plPython::CallObject(generateFunc, stubsDir.AsString());
        else
            result = plPython::CallObject(generateFunc);

        if (!result) {
            PyErr_Print();
            throw std::runtime_error("generate_stubs.run() failed!");
        }
    } catch (const std::runtime_error& err) {
        ST::printf(stderr, "ERROR: {}\n", err.what());
        PythonInterface::finiPython();
        return 1;
    }

    PythonInterface::finiPython();
    return 0;
}

int hsMain(std::vector<ST::string> args)
{
    PrintHeader();

    enum { kArgClientPath, kArgStubsPath, kArgHelp };
    const plCmdArgDef cmdLineArgs[] = {
        { kCmdArgFlagged | kCmdTypeString, "Client", kArgClientPath},
        { kCmdArgFlagged | kCmdTypeString, "Stubs", kArgStubsPath },
        { kCmdArgFlagged | kCmdTypeBool, "help", kArgHelp },
    };

    plCmdParser parser(cmdLineArgs, std::size(cmdLineArgs));
    if (!parser.Parse(args)) {
        ST::printf(stderr, "An error occurred while parsing the provided arguments.\n");
        ST::printf(stderr, "Use the --help option to display usage information.\n");
        return 1;
    }

    if (parser.IsSpecified(kArgHelp)) {
        PrintHelp();
        return 0;
    }

    plFileName clientPath = parser.GetString(kArgClientPath);
    plFileInfo clientPathInfo(clientPath);
    if (!(clientPathInfo.Exists() && clientPathInfo.IsDirectory())) {
        ST::printf(stderr, "The specified client directory is invalid.\n");
        return 1;
    }

    plFileName pythonPath = plFileName::Join(clientPath, "python");
    if (!CheckScriptsDir(pythonPath)) {
        ST::printf(stderr, "The specified client directory does not appear to have a valid Plasma Python directory.\n");
        return 1;
    }

    return GenerateStubs(clientPath, parser.GetString(kArgStubsPath));
}
