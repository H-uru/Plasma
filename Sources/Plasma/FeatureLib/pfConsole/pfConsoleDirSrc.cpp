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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  pfConsoleDirSrc Functions                                               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "pfConsoleDirSrc.h"

#include <string_theory/string_stream>

#include "HeadSpin.h"
#include "hsExceptions.h"
#include "plMessageBox/hsMessageBox.h"


//// ParseDirectory //////////////////////////////////////////////////////////

bool pfConsoleDirSrc::ParseDirectory(const plFileName& path, const char* mask /* = L"*.*" */)
{
    hsAssert(fEngine != nullptr, "Cannot do a dir execute without an engine!");

    std::vector<plFileName> files = plFileSystem::ListDir(path, mask);
    for (auto iter = files.begin(); iter != files.end(); ++iter)
    {
        plFileName name = iter->GetFileName();
        if (AlreadyProcessedFile(path, name))
            continue;
        AddProcessedFile(path, name);
        if (!fEngine->ExecuteFile(*iter))
        {
            // Change the following line once we have a better way of reporting
            // errors in the parsing
            ST::string_stream error, caption;

            caption << "Error parsing " << name.AsString();
            error << fEngine->GetErrorMsg() << ":\n\nCommand: '" << fEngine->GetLastErrorLine()
                  << "'\n\nPress OK to continue parsing files.";

            hsMessageBox(error.to_string(), caption.to_string(), hsMessageBoxNormal);

            SetCheckProcessedFiles(true);
            return false;
        }
    }

    SetCheckProcessedFiles(true);
    return true;
}

void pfConsoleDirSrc::ResetProcessedFiles()
{
    int i;
    for(i=0;i<fProcessedFiles.size(); i++)
        delete fProcessedFiles[i];
    fProcessedFiles.clear();
}

//
// note: this n^2 linear search should be replaced with something
// faster if we have lots of init files and turn on the checkProcessing option.
//
bool pfConsoleDirSrc::AlreadyProcessedFile(const plFileName& path, const plFileName& file)
{
    if (fCheckProcessedFiles)
    {
        int i;
        for (i=0; i<fProcessedFiles.size(); i++)
        {
            if (file == fProcessedFiles[i]->fFile && path == fProcessedFiles[i]->fPath)
                return true;
        }
    }
    return false;
}

void pfConsoleDirSrc::AddProcessedFile(const plFileName& path, const plFileName& file)
{
    fProcessedFiles.push_back(new FileName(path, file));
}
