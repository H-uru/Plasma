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

#ifndef _pfPatcher_inc_
#define _pfPatcher_inc_

#include <functional>
#include <memory>
#include <vector>

#include "pnNetBase/pnNbError.h"

class plFileName;
class plFilePath;
class plMD5Checksum;
class plStatusLog;
class hsStream;

/** Plasma File Patcher
 *  This is used to patch the client with one or many manifests at once. It assumes that
 *  we have permission to modify the game files, so be sure that you do! We memory manage
 *  ourselves, so create a pfPatcher, add your manifests, and Start!
 */
class pfPatcher
{
    std::unique_ptr<struct pfPatcherWorker> fWorker;

public:
    static plStatusLog* GetLog();

public:
    /** Represents a function that takes the status and an optional message on completion. */
    typedef std::function<void(ENetError, const ST::string&)> CompletionFunc;

    /** Represents a function that takes (const plFileName&) and approves it. */
    typedef std::function<bool(const plFileName&)> FileDesiredFunc;

    /** Represents a function that takes (const plFileName&) on an interesting file operation. */
    typedef std::function<void(const plFileName&)> FileDownloadFunc;

    /** Represents a function that takes (const plFileName&, hsStream*) on game code discovery.
     *  You are responsible for closing and deleting the provided stream.
     */
    typedef std::function<bool(const plFileName&, hsStream*)> GameCodeDiscoverFunc;

    /** Represents a function that takes (bytesDLed, totalBytes, statsStr) as a progress indicator. */
    typedef std::function<void(uint64_t, uint64_t, const ST::string&)> ProgressTickFunc;
    
    /** Represents a function that takes (const plFileName&) and returns the executable inside the
     *  macOS application bundle at that path.
     */
    typedef std::function<plFileName(const plFileName&)> FindBundleExeFunc;

    pfPatcher();
    ~pfPatcher();

    /** Set a callback that will be fired when the patcher needs to find an executable file
     *  within an executable bundle. This only occurs on the macOS client and is
     *  specific to macOS executable application bundles.
     */
    void OnFindBundleExe(FindBundleExeFunc cb);

    /** Set a callback that will be fired when the patcher finishes its dirty work.
     *  \remarks This may be called from any thread, so make sure your callback is
     *  thread safe!
     */
    void OnCompletion(CompletionFunc cb);

    /** Set a callback that will be fired when the patcher issues a download request to the server.
     *  \remarks This will be called from the network thread.
     */
    void OnFileDownloadBegin(FileDownloadFunc cb);

    /** Set a callback that will be fired when the patcher wants to download a file. You are
     *  given the ability to approve or veto the download. With great power comes great responsibility...
     *  \remarks This will be called from the patcher thread.
     */
    void OnFileDownloadDesired(FileDesiredFunc cb);

    /** Set a callback that will be fired when the patcher has finished downloading a file from the server.
     *  \remarks This will be called from the network thread.
     */
    void OnFileDownloaded(FileDownloadFunc cb);

    /** This is called when the patcher discovers an up-to-date Python package or SDL file.
    *   \remarks This can be called from any thread when the patcher downloads or encounters an up-to-date
    *   python package or SDL file that the server knows about.
    */
    void OnGameCodeDiscovery(GameCodeDiscoverFunc cb);

    /** Set a callback that will be fired when the patcher receives a chunk from the server. The status string
     *  will contain the current download speed.
     *  \remarks This will be called from the network thread.
     */
    void OnProgressTick(ProgressTickFunc cb);

    /** Set a callback that will be fired when the patcher downloads an updated redistributable. Such as
     *  the Visual C++ runtime (vcredist_x86.exe). You are responsible for installing it.
     *  \remarks This will be called from the network thread.
     */
    void OnRedistUpdate(FileDownloadFunc cb);

    /** This is called when the current application has been updated. */
    void OnSelfPatch(FileDownloadFunc cb);

    void RequestGameCode(bool python = true, bool sdl = true);
    void RequestManifest(const ST::string& mfs);
    void RequestManifest(const std::vector<ST::string>& mfs);

    /** Start patching the requested manifests in a new thread. After calling this method,
     *  the pfPatcher should be destroyed - the newly started thread will memory-manage itself
     *  and the pfPatcher object cannot be reused.
     */
    void Start();
};

#endif // _pfPatcher_inc_
