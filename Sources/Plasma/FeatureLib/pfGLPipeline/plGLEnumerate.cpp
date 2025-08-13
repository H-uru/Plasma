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

#include <epoxy/gl.h>
#include <vector>

#include "plGLPipeline.h"

bool fillDeviceRecord(hsG3DDeviceRecord& devRec, const ST::string& driverName, hsDisplayHndl display)
{
    if (epoxy_gl_version() < 33)
        return false;

    devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
    devRec.SetDriverName(driverName);

    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    devRec.SetDeviceDesc(renderer);

    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    devRec.SetDriverDesc(vendor);

    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    devRec.SetDriverVersion(version);

    devRec.SetCap(hsG3DDeviceSelector::kCapsMipmap);
    devRec.SetCap(hsG3DDeviceSelector::kCapsPerspective);
    devRec.SetCap(hsG3DDeviceSelector::kCapsCompressTextures);
    devRec.SetCap(hsG3DDeviceSelector::kCapsDoesSmallTextures);

    devRec.SetLayersAtOnce(8);

    plDisplayHelper* displayHelper = plDisplayHelper::GetInstance();
    if (displayHelper) {
        for (const auto& mode : displayHelper->GetSupportedDisplayModes(display)) {
            hsG3DDeviceMode devMode;
            devMode.SetWidth(mode.Width);
            devMode.SetHeight(mode.Height);
            devMode.SetColorDepth(mode.ColorDepth);
            devRec.GetModes().emplace_back(std::move(devMode));
        }
        devRec.SetDefaultModeIndex(0);
    } else {
        // Just make a fake mode so the device selector will let it through
        hsG3DDeviceMode devMode;
        devMode.SetWidth(hsG3DDeviceSelector::kDefaultWidth);
        devMode.SetHeight(hsG3DDeviceSelector::kDefaultHeight);
        devMode.SetColorDepth(hsG3DDeviceSelector::kDefaultDepth);
        devRec.GetModes().emplace_back(devMode);
    }

    return true;
}


#pragma region EGL_Enumerate
#ifdef USE_EGL
#include <epoxy/egl.h>

void plEGLEnumerate(std::vector<hsG3DDeviceRecord>& records, hsDisplayHndl displayHndl = EGL_DEFAULT_DISPLAY)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    do {
        display = eglGetDisplay(static_cast<EGLNativeDisplayType>(displayHndl));
        if (eglGetError() == EGL_SUCCESS && display != EGL_NO_DISPLAY) {
            EGLBoolean initialized = eglInitialize(display, nullptr, nullptr);
            if (eglGetError() != EGL_SUCCESS || initialized != EGL_TRUE) {
                display = EGL_NO_DISPLAY;
            }
        }

        if (display == EGL_NO_DISPLAY) {
            display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
            if (eglGetError() != EGL_SUCCESS || display == EGL_NO_DISPLAY)
                break;

            EGLBoolean initialized = eglInitialize(display, nullptr, nullptr);
            if (eglGetError() != EGL_SUCCESS || initialized != EGL_TRUE)
                break;
        }

        if (!eglBindAPI(EGL_OPENGL_API))
            break;

        /* Set up the config attributes for EGL */
        EGLConfig config;
        EGLint config_count;
        EGLint config_attrs[] = {
            EGL_BUFFER_SIZE, 24,
            EGL_DEPTH_SIZE, 24,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_NONE
        };

        if (!eglChooseConfig(display, config_attrs, &config, 1, &config_count) || config_count != 1)
            break;

        EGLint ctx_attrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE
        };

        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attrs);
        if (context == EGL_NO_CONTEXT)
            break;

        EGLint pbuf_attrs[] = {
            EGL_WIDTH, 800,
            EGL_HEIGHT, 600,
            EGL_NONE
        };

        surface = eglCreatePbufferSurface(display, config, pbuf_attrs);
        if (surface == EGL_NO_SURFACE)
            break;

        if (!eglMakeCurrent(display, surface, surface, context))
            break;

        hsG3DDeviceRecord devRec;
        if (fillDeviceRecord(devRec, ST_LITERAL("EGL OpenGL API"), displayHndl))
            records.emplace_back(devRec);
    } while (0);

    // Cleanup:
    if (surface != EGL_NO_SURFACE) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, surface);
    }

    if (context != EGL_NO_CONTEXT)
        eglDestroyContext(display, context);

    if (display != EGL_NO_DISPLAY)
        eglTerminate(display);
}
#endif // USE_EGL
#pragma endregion EGL_Enumerate


#pragma region WGL_Enumerate
#ifdef HS_BUILD_FOR_WIN32
#include "hsWindows.h"
#include <epoxy/wgl.h>

void plWGLEnumerate(std::vector<hsG3DDeviceRecord>& records, hsDisplayHndl displayHndl = INVALID_HANDLE_VALUE)
{
    HINSTANCE inst = GetModuleHandleW(nullptr);
    LPCWSTR className = L"GLTestClass";
    HWND wnd = nullptr;
    HDC dc = nullptr;
    HGLRC ctx = nullptr;

    do {
        WNDCLASSW tempClass = {};
        tempClass.lpfnWndProc = DefWindowProcW;
        tempClass.hInstance = inst;
        tempClass.lpszClassName = className;
        tempClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

        if (!RegisterClassW(&tempClass))
            break;

        wnd = CreateWindowExW(0, className, L"OpenGL Test Window", 0,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                nullptr, nullptr, inst, nullptr);
        if (!wnd)
            break;

        dc = GetDC(wnd);
        if (!dc)
            break;

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;

        int format = ChoosePixelFormat(dc, &pfd);
        if (!format)
            break;

        if (!SetPixelFormat(dc, format, &pfd))
            break;

        ctx = wglCreateContext(dc);
        if (!ctx)
            break;

        if (!wglMakeCurrent(dc, ctx))
            break;

        hsG3DDeviceRecord devRec;
        if (fillDeviceRecord(devRec, ST_LITERAL("opengl32.dll"), displayHndl))
            records.emplace_back(devRec);
    } while (0);

    // Cleanup:
    if (ctx) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(ctx);
    }

    if (dc)
        ReleaseDC(wnd, dc);

    if (wnd)
        DestroyWindow(wnd);

    UnregisterClassW(className, inst);
}
#endif // HS_BUILD_FOR_WIN32
#pragma endregion WGL_Enumerate


#pragma region CGL_Enumerate
#ifdef HS_BUILD_FOR_MACOS
#include <AvailabilityMacros.h>
#include <CoreGraphics/CoreGraphics.h>
#include <OpenGL/OpenGL.h>

void plCGLEnumerate(std::vector<hsG3DDeviceRecord>& records, CGDirectDisplayID displayHndl = kCGNullDirectDisplay)
{
    IGNORE_WARNINGS_BEGIN("deprecated-declarations")
    CGLPixelFormatObj pix = nullptr;
    CGLContextObj ctx = nullptr;

    if (displayHndl == kCGNullDirectDisplay)
        displayHndl = CGMainDisplayID();

    do {
        CGLPixelFormatAttribute attribs[] = {
            kCGLPFAAccelerated,
            kCGLPFANoRecovery,
            kCGLPFADoubleBuffer,
            kCGLPFADisplayMask, static_cast<CGLPixelFormatAttribute>(CGDisplayIDToOpenGLDisplayMask(displayHndl)),
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
            // OpenGL profiles introduced in 10.7 - These must be the last args
            kCGLPFAOpenGLProfile, static_cast<CGLPixelFormatAttribute>(kCGLOGLPVersion_3_2_Core),
#endif
            static_cast<CGLPixelFormatAttribute>(0),
        };

        int nPix = 0;
        if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || nPix == 0) {
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070 && MAC_OS_X_VERSION_MIN_REQUIRED < 1070
            nPix = 0;
            size_t attrs_count = std::size(attribs);

            // Try without the OpenGL profile
            attribs[attrs_count - 2] = static_cast<CGLPixelFormatAttribute>(0);
            attribs[attrs_count - 3] = static_cast<CGLPixelFormatAttribute>(0);

            if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || nPix == 0)
                // Intentional fallthrough
#endif
            break;
        }

        if (CGLCreateContext(pix, nullptr, &ctx) != kCGLNoError)
            break;

        if (CGLSetCurrentContext(ctx) != kCGLNoError)
            break;

        hsG3DDeviceRecord devRec;
        if (fillDeviceRecord(devRec, ST_LITERAL("OpenGL.framework"), displayHndl))
            records.emplace_back(std::move(devRec));
    } while (0);

    // Cleanup:
    if (ctx) {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(ctx);
    }

    if (pix)
        CGLReleasePixelFormat(pix);
    IGNORE_WARNINGS_END
}
#endif // HS_BUILD_FOR_MACOS
#pragma endregion CGL_Enumerate

void plGLEnumerate::Enumerate(std::vector<hsG3DDeviceRecord>& records)
{
#ifdef USE_EGL
    // The USE_EGL define tells us whether the epoxy library includes support
    // for attempting to use EGL on the current platform, but we still need to
    // check if EGL is actually available at runtime.
    //
    // On Windows, this may be true in cases like the PowerVR SDK or when using
    // ANGLE.
    //
    // On Linux, this should be true with mesa or nvidia drivers.
    if (epoxy_has_egl()) {
        plEGLEnumerate(records);
    }
#endif

#ifdef HS_BUILD_FOR_WIN32
    plWGLEnumerate(records);
#endif

#ifdef HS_BUILD_FOR_MACOS
    plCGLEnumerate(records);
#endif
}
