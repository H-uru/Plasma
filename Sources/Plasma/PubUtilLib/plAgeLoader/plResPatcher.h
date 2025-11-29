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

#include "plFileSystem.h"
#include "pnNetBase/pnNbError.h"

#include <memory>
#include <vector>

class plOperationProgress;

/**
 * Plasma Resource Patcher
 * This is a thin wrapper around \sa pfPatcher that ensures updates are propagated to the rest of the engine.
 */
class plResPatcher
{
    plOperationProgress* fProgress;
    static plResPatcher* fInstance;
    bool                 fRequestedGameCode;

    friend class plAgeLoader;

    void OnCompletion(ENetError, const ST::string& msg);
    void OnFileDownloadBegin(const plFileName& file);
    void OnFileDownloaded(const plFileName& file);
    bool OnGameCodeDiscovered(const plFileName& file, class hsStream* stream);
    void OnProgressTick(uint64_t dl, uint64_t total, const ST::string& msg);

    std::unique_ptr<class pfPatcher> CreatePatcher();
    void InitProgress();

public:
    static plResPatcher* GetInstance();
    static void Shutdown();

public:
    plResPatcher();
    ~plResPatcher();

    void Update(const std::vector<ST::string>& manifests);
    void Update(const ST::string& manifest);
};


#endif // _plResPatcher_h
