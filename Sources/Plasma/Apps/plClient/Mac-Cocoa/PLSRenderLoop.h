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

#import <AppKit/AppKit.h>
#import <QuartzCore/QuartzCore.h>

#import <Metal/Metal.hpp>
#import <functional>
#import <optional>

#import "pfMetalPipeline/plMetalRenderDestination.h"

typedef std::function<void()> plRenderCallback;

class plMetalRenderLoopType
{
public:
    virtual void StartRenderLoop() = 0;
    virtual void StopRenderLoop() = 0;
    virtual void SetRenderCallback(std::function<void()> callback) = 0;
};

template <typename T>
class plRenderLoop : public plMetalRenderLoopType
{
public:
    plRenderLoop(T provider) : _provider(provider) {}
    void StartRenderLoop() override { _provider.StartRenderLoop(); }
    void StopRenderLoop() override { _provider.StopRenderLoop(); }
    void SetRenderCallback(std::function<void()> callback) override { _provider.SetRenderCallback(callback); }

private:
    T _provider;
};

/*
 Legacy render loops are based on CVDisplayLinkRef. This render loop supports
 both OpenGL and Metal - but doesn't provide any performance monitoring for
 Metal rendering. They are only available on macOS. CVDisplayLinkRef is not
 available on iOS or visionOS. They are useful for running on older versions of
 macOS where CADisplayLink is unsupported. This render loop is maintained in
 C++ so tha it can be compiled on versions of macOS that predate Swift.s
 */

class plLegacyRenderLoop
{
public:
    plLegacyRenderLoop(NSWindow* window);
    void StartRenderLoop();
    void StopRenderLoop();
    void SetRenderCallback(plRenderCallback callback);

private:
    NSWindow*             _window;
    std::function<void()> _callback;
    CVDisplayLinkRef      _displayLink;
    dispatch_source_t     _displaySource;
};
