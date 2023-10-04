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

#include <set>

#include "plResCollector.h"
#include "plMtlCollector.h"
#include "MaxExport/plExportProgressBar.h"

void plResCollector::Collect()
{
    Interface *ip = GetCOREInterface();

    // Get the directory to copy files to
    MCHAR pathBuf[MAX_PATH]{};
    ip->ChooseDirectory(ip->GetMAXHWnd(),
                        _M("Choose a folder to copy the resources to"),
                        pathBuf,
                        nullptr);
    if (*pathBuf == _M('\0'))
        return;

    ST::string path = M2ST(pathBuf);

    // Make sure the directory ends with a slash
    if (path.back() != '\\' && path.back() != '/')
        path += '\\';

    // Make a list of all the textures
    TexNameSet texNames;
    plMtlCollector::GetAllTextures(texNames);

    plExportProgressBar bar;
    bar.Start(_M("Copy Files"), texNames.size()+1);

    // Copy each texture to the output directory
    for (const auto& texName : texNames) {
        plFileName texPath(texName);

        if (bar.Update(ST2M(texPath.GetFileNameNoExt())))
            return;

        plFileSystem::Copy(texPath, plFileName::Join(path, texPath.GetFileName()));
    }

    // Get the filename to save to
    auto& maxFile = ip->GetCurFileName();
    auto& filePath =  ip->GetCurFilePath();

    if (maxFile.isNull())
        return;

    if (bar.Update(maxFile))
        return;

    // If we need to save, do it now
    if (IsSaveRequired())
        ip->SaveToFile(filePath);

    // Copy the max file to the output directory
    plFileSystem::Copy(M2ST(filePath), plFileName::Join(path, M2ST(maxFile)));
}
