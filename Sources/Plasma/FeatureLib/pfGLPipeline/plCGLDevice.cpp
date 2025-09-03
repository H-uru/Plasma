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

#include "plCGLDevice.h"

#ifdef HS_BUILD_FOR_MACOS
#include <AvailabilityMacros.h>

bool plCGLDevice::Enumerate(hsG3DDeviceRecord& record)
{
    bool result = false;

    IGNORE_WARNINGS_BEGIN("deprecated-declarations")
    CGLPixelFormatObj pix = nullptr;
    CGLContextObj ctx = nullptr;

    do {
        CGLPixelFormatAttribute attribs[6] = {
            kCGLPFAAccelerated,
            kCGLPFANoRecovery,
            kCGLPFADoubleBuffer,
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
            // OpenGL profiles introduced in 10.7
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
#endif
            (CGLPixelFormatAttribute) 0
        };

        int nPix = 0;
        if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || nPix == 0)
            break;

        if (CGLCreateContext(pix, nullptr, &ctx) != kCGLNoError)
            break;

        if (CGLSetCurrentContext(ctx) != kCGLNoError)
            break;

        if (epoxy_gl_version() < 33)
            break;

        record.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        record.SetDriverName("OpenGL.framework");
        record.SetDeviceDesc(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        record.SetDriverDesc(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        record.SetDriverVersion(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        plDisplayHelper* displayHelper = plDisplayHelper::GetInstance();
        for (const auto& mode : displayHelper->GetSupportedDisplayModes(mainDisplay)) {
            hsG3DDeviceMode devMode;
            devMode.SetWidth(mode.Width);
            devMode.SetHeight(mode.Height);
            devMode.SetColorDepth(mode.ColorDepth);
            record.GetModes().emplace_back(std::move(devMode));
        }
        record.SetDefaultModeIndex(0);

        result = true;
    } while (0);

    // Cleanup:
    if (ctx) {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(ctx);
    }

    if (pix)
        CGLReleasePixelFormat(pix);

    IGNORE_WARNINGS_END

    return result;
}


plCGLDevice* plCGLDevice::TryInit(hsWindowHndl window, hsWindowHndl device, ST::string& error)
{
    IGNORE_WARNINGS_BEGIN("deprecated-declarations")

    CGLPixelFormatObj pix = nullptr;
    CGLContextObj ctx = nullptr;

    do {
        CGLPixelFormatAttribute attribs[6] = {
            kCGLPFAAccelerated,
            kCGLPFANoRecovery,
            kCGLPFADoubleBuffer,
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
            // OpenGL profiles introduced in 10.7
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
#endif
            (CGLPixelFormatAttribute) 0
        };

        int nPix = 0;
        if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || nPix == 0) {
            error = ST_LITERAL("Could not choose appropriate config");
            break;
        }

        if (CGLCreateContext(pix, nullptr, &ctx) != kCGLNoError) {
            error = ST_LITERAL("Unable to create rendering context");
            break;
        }

        if (CGLSetCurrentContext(ctx) != kCGLNoError) {
            error = ST_LITERAL("Failed to attach CGL context to surface");
            break;
        }

        CGLReleasePixelFormat(pix);

        // Successfully initialized
        return new plCGLDevice(window, device, ctx);
    } while (0);

    // Cleanup for failure case:
    if (ctx) {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(ctx);
    }

    if (pix)
        CGLReleasePixelFormat(pix);

    IGNORE_WARNINGS_END

    return nullptr;
}


plCGLDevice::plCGLDevice(hsWindowHndl window, hsWindowHndl device, CGLContextObj context)
    : plGLDeviceImpl(window, device), fContext(context)
{ }

void plCGLDevice::Shutdown()
{
    IGNORE_WARNINGS_BEGIN("deprecated-declarations")

    CGLSetCurrentContext(nullptr);
    CGLReleaseContext(fContext);
    fContext = nullptr;

    IGNORE_WARNINGS_END
}

bool plCGLDevice::BeginRender(ST::string& error)
{
    return true;
}

bool plCGLDevice::EndRender(ST::string& error)
{
    return true;
}

#endif // HS_BUILD_FOR_MACOS

