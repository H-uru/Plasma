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

#ifndef _plClientLauncher_inc_
#define _plClientLauncher_inc_

#include "plFileSystem.h"
#include "pnNetBase/pnNbError.h"

#include <functional>
#include <memory>

class plClientLauncher
{
public:
    typedef std::function<std::unique_ptr<class pfPatcher>()> CreatePatcherFunc;
    typedef std::function<void(ENetError, const ST::string&)> ErrorFunc;
    typedef std::function<bool(const plFileName&)> InstallRedistFunc;
    typedef std::function<void(const plFileName&, const ST::string&)> LaunchClientFunc;
    typedef std::function<void(const ST::string&)> StatusFunc;

private:
    enum Flags
    {
        kHaveSelfPatched = 1<<0,
        kClientImage = 1<<1,
        kGameDataOnly = 1<<2,
        kPatchOnly = 1<<3,
        kSkipLoginDialog = 1<<4,
        kSkipIntroMovies = 1<<5,

        kRepairGame = kHaveSelfPatched | kClientImage | kGameDataOnly,
    };

    enum NetCoreState
    {
        kNetCoreInactive,
        kNetCoreActive,
        kNetCoreShutdown,
    };

    uint32_t    fFlags;
    plFileName  fServerIni;
    NetCoreState fNetCoreState;

    plFileName fClientExecutable;

    std::unique_ptr<class plShardStatus>   fStatusThread;
    std::unique_ptr<class plRedistUpdater> fInstallerThread;

    CreatePatcherFunc fPatcherFactory;
    LaunchClientFunc  fLaunchClientFunc;
    StatusFunc        fStatusFunc;

    ST::string GetAppArgs() const;

    void IOnPatchComplete(ENetError result, const ST::string& msg);
    bool IApproveDownload(const plFileName& file);
    void IGotFileServIPs(ENetError result, const ST::string& addr);

public:
    plClientLauncher();
    ~plClientLauncher();

    /** Launch whatever client we think is appropriate. Please note that you should not call this unless you know
     *  absolutely without question what you are doing!
     */
    void LaunchClient() const;

    /** Begin the next logical patch operation. We are internally tracking if this is a self patch or a client patch.
     *  All you need to do is make certain the doggone callbacks are set so that your UI will update. In theory, you
     *  should never call this from your UI code since we manage this state for you.
     */
    void PatchClient();

    /** Attempt to complete a self-patch left in progress by an older launcher. Specifically, we want to rename 
     *  the launcher to something sane (UruLauncher.exe.tmp -> UruLauncher.exe). If we complete a self-patch in 
     *  here, then we need to relaunch ourselves so that the game client will look like what the server expects.
     *  \returns True if a self-patch was completed. False if not.
     */
    bool CompleteSelfPatch(const std::function<void()>& waitProc) const;

    /** Start eap's weird network subsystem and the shard status pinger.
     *  \remarks Please note that this will also enqueue the first patch.
     */
    void InitializeNetCore();

    /** This pumps eap's network subsystem and runs any queued transaction completion callbacks.
     *  The thread that you call this from will be the thread that all your UI updates come from.
     *  So be certain that you've thought that through!
     *  \remarks This method will cause the thread to sleep so that we don't hog the CPU.
     */
    bool PumpNetCore() const;

    /** Shutdown eap's netcore and purge any other crap that needs to happen while the app is
     *  visible. In other words, tear down evil threaded crap.
     */
    void ShutdownNetCore();

    /** Load the server configuration file. Note that you MUST parse the command
     *  arguments before calling this function!
     */
    void LoadServerIni() const;

    /** Parse the command line options. */
    void ParseArguments();

    /** Set a callback function that is called on a network error.
     *  \remarks This will be called from the network thread.
     */
    void SetErrorProc(ErrorFunc proc);

    /** Set a callback that will execute and wait for redistributable installers.
    *  \remarks This will be called from a worker thread.
    */
    void SetInstallerProc(InstallRedistFunc proc);

    /** Set a patcher factory. */
    void SetPatcherFactory(CreatePatcherFunc factory) { fPatcherFactory = std::move(factory); }

    /** Set a callback that launches an arbitrary executable.
     *  \remarks This will be called from an arbitrary thread.
     */
    void SetLaunchClientProc(LaunchClientFunc proc) { fLaunchClientFunc = std::move(proc); }

    /** Set a callback that displays the shard status.
     *  \remarks This will be called from a worker thread.
     */
    void SetShardProc(StatusFunc proc);

    /** Set a callback that displays the patcher status.
     *  \remarks This will be called from the network thread. Note that any time
     *  this is called, you should consider it a state reset (so undo your progress bars).
     */
    void SetStatusProc(StatusFunc proc) { fStatusFunc = std::move(proc); }
};

#endif // _plClientLauncher_inc_
