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

#include "MaxAPI.h"

#include "plMaxCFGFile.h"
#include "plFile/plBrowseFolder.h"

plFileName plMaxConfig::GetPluginIni()
{
    // Get the plugin CFG dir
    return plFileName::Join(M2ST(GetCOREInterface()->GetDir(APP_PLUGCFG_DIR)), "PlasmaMAX2.ini");
}

plFileName plMaxConfig::GetClientPath(bool getNew, bool quiet)
{
    // Get the plugin CFG dir
    plFileName plugDir = GetPluginIni();

    // Get the saved path
    wchar_t buffer[MAX_PATH];
    uint32_t len = GetPrivateProfileStringW(L"SceneViewer", L"Directory", L"", buffer, MAX_PATH,
                                            plugDir.WideString().data());
    plFileName plasmaPath = ST::string::from_wchar(buffer);

    // If we didn't find a path, or we want a new one, ask the user for one
    if ((len == 0 || getNew) && !quiet)
    {
        // If the user selects one, save it
        plasmaPath = plBrowseFolder::GetFolder(plasmaPath, "Select your Plasma 2.0 client folder");
        if (plasmaPath.IsValid())
            WritePrivateProfileStringW(L"SceneViewer", L"Directory", plasmaPath.WideString().data(),
                                       plugDir.WideString().data());
    }

    // Return the path if we got one
    return plasmaPath;
}

void plMaxConfig::SetClientPath(const plFileName &path)
{
    plFileName plugDir = GetPluginIni();
    WritePrivateProfileStringW(L"SceneViewer", L"Directory", path.WideString().data(),
                               plugDir.WideString().data());
}

bool plMaxConfig::AssetManInterfaceDisabled()
{
#ifdef MAXASS_AVAILABLE
    static bool inited = false;
    static bool disabled = false;

    if (!inited)
    {
        wchar_t configstr[MAX_PATH];
        configstr[0] = '\0';

        plFileName plugDir = GetPluginIni();
        uint32_t len = GetPrivateProfileStringW(L"AssetMan", L"Disable", L"", configstr, MAX_PATH,
                                                plugDir.WideString().data());

        if (wcscmp(configstr, L"1") == 0 || wcsicmp(configstr, L"true") == 0)
            disabled = true;
        else
            disabled = false;

        inited = true;
    }

    return disabled;
#else
    return true;
#endif
}
