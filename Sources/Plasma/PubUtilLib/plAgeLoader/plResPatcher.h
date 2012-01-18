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
#ifndef plResPatcher_h_inc
#define plResPatcher_h_inc

#include "HeadSpin.h"
#include <queue>
#include <string>

class plOperationProgress;

class plResPatcher
{
    enum { kManifest, kFile };
    struct Request
    {
        std::wstring fFile;
        std::wstring fFriendlyName;
        uint8_t      fType;

        Request(const wchar_t* file, uint8_t type, const wchar_t* friendly = nil)
            : fFile(file), fType(type)
        {
            if (friendly)
                fFriendlyName = std::wstring(friendly);
        }
    };

    static plResPatcher*       fInstance;
    std::queue<Request>        fRequests;
    plOperationProgress*       fProgress;
    bool                       fPatching;

    plResPatcher();
    ~plResPatcher();

public:
    static plResPatcher* GetInstance();
    static void Shutdown();

    plOperationProgress* GetProgress() { return fProgress; }

    void Finish(bool success = true);
    void IssueRequest();
    void RequestFile(const wchar_t* file, const wchar_t* friendlyName);
    void RequestManifest(const wchar_t* age);
    void Start();
};

enum PatcherLogType
{
    kHeader,
    kInfo,
    kMajorStatus,
    kStatus,
    kError,
};
void PatcherLog(PatcherLogType type, const char* format, ...);
#endif // _plResPatcher_h
