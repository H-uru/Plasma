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
#include "hsThread.h"
#include "hsWindows.h"

class plClientLoader : private hsThread
{
    class plClient* fClient;
    hsWindowHndl fWindow;
    hsDisplayHndl fDisplay;
    uint32_t fDevType;

    void OnQuit() override
    {
        SetQuit(true);
    }

    /** Does the heavy lifting of client init */
    void Run() override;

public:
    plClientLoader() : fClient(), fWindow(), fDisplay(), fDevType() { }

    /**
     * Initializes the client asyncrhonouslynn including: loading the localization, 
     * registry, dispatcher, etc.
     */
    void Init()
    {
        hsAssert(fClient == nullptr, "trying to init the client more than once?");
        Start();
    }

    /**
     * Returns whether or not the client init is done
     */
    bool IsInited() const { return GetQuit(); }

    /**
     * Sets the client window handle.
     */
    void SetClientWindow(hsWindowHndl hWnd) { fWindow = hWnd; }

    /**
     * Sets the client display handle.
     */
    void SetClientDisplay(hsDisplayHndl hDC) { fDisplay = hDC; }

    /**
     * Sets the preferred rendering backend for the client pipeline.
     */
    void SetRequestedRenderingBackend(uint32_t devType) { fDevType = devType; }

    /**
     * Initial shutdown request received from Windows (or something)... start tear down
     */
    void ShutdownStart();

    /**
     * Window mess cleaned up, time to commit hara-kiri
     */
    void ShutdownEnd();

    /**
     * Launches the client window and starts the game.
     * This will block if the client is not initialized.
     */
    void StartClient();

    /**
     * Waits for the client to finish initing
     */
    void Wait() { Stop(); }

    /** Returns the current plClient instance */
    plClient* operator ->() const { return fClient; }

    /** Returns whether or not the client is non-null */
    operator bool() const { return fClient != nullptr; }
};

