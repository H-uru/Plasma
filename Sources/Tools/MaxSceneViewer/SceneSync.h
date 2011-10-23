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
#ifndef SCENE_SYNC_H
#define SCENE_SYNC_H

#include "Max.h"
#include "notify.h"

#include <vector>
#include <set>

class plMaxNode;
class hsSemaphore;
class SceneWatcher;
class plSceneNode;

#include "pnKeyedObject/plUoid.h"
#include "pnKeyedObject/plKey.h"

class SceneSync
{
protected:
    SceneWatcher *fSceneWatcher;
    hsSemaphore *fUpdateSignal;
    const char *fPipeName;
    int fTimerID;
    int fUpdateFreq;
    
    SceneSync();

public:
    static SceneSync& Instance();

    // Get the path where the current Max file will be exported to (not including "dat")
    bool GetOutputDir(char *buf);

    bool IsClientRunning();

    // Is there valid data on disk that we can load into the ResMgr or do we need to reconvert?
    bool CanLoadOldResMgr();

    // Create client data
    bool CreateClientData();

    void SetUpdateFreq(int freq);   // In milliseconds

    // Start updating the client
    bool BeginClientSync(const char *semaphoreName, const char *pipeName);
    // Stop updating the client.  If abort is true, don't try saving, something went wrong
    void EndClientSync(bool abort);

protected:
    bool SaveResMgr();

    void IShutdownClient();

    // Reconvert any dirty nodes to sync the Plasma database and the Max one
    bool Update();

    void AddSceneNodes(std::set<plSceneNode*>& sceneNodes, std::vector<plUoid>& delUoids, std::vector<plKey>& newKeys);

    bool IStartWatching(bool forceWatch=false);
    bool IStopWatching();

    // Called by open and close scene.
    bool IReadNodeMap(const char *dir);
    bool IWriteNodeMap(const char *dir);

    void IShutdown();

    void IDeletePath(const char *path);
    void IClearDirtyRecur(plMaxNode *node);

    static void INotify(void *param, NotifyInfo *info);

    static void CALLBACK ITimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
};

#endif //SCENE_SYNC_H
