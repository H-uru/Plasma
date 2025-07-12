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

bool fillDeviceRecord(hsG3DDeviceRecord& devRec)
{
    if (epoxy_gl_version() < 33)
        return false;

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

    // Just make a fake mode so the device selector will let it through
    hsG3DDeviceMode devMode;
    devMode.SetWidth(hsG3DDeviceSelector::kDefaultWidth);
    devMode.SetHeight(hsG3DDeviceSelector::kDefaultHeight);
    devMode.SetColorDepth(hsG3DDeviceSelector::kDefaultDepth);
    devRec.GetModes().emplace_back(devMode);

    return true;
}


#ifdef USE_EGL
#include <epoxy/egl.h>

void plEGLEnumerate(std::vector<hsG3DDeviceRecord>& records)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    do {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        if (display == EGL_NO_DISPLAY)
            break;

        if (!eglInitialize(display, nullptr, nullptr))
            break;

        if (!eglBindAPI(EGL_OPENGL_API))
            break;

        GLint numConfigs = 0;
        if (!eglGetConfigs(display, nullptr, 0, &numConfigs) || numConfigs == 0)
            break;

        std::vector<EGLConfig> configs(numConfigs);
        if (!eglGetConfigs(display, configs.data(), configs.size(), &numConfigs))
            break;

        EGLint ctx_attrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE
        };

        context = eglCreateContext(display, configs[0], EGL_NO_CONTEXT, ctx_attrs);
        if (context == EGL_NO_CONTEXT)
            break;

        surface = eglCreatePbufferSurface(display, configs[0], nullptr);
        if (surface == EGL_NO_SURFACE)
            break;

        if (!eglMakeCurrent(display, surface, surface, context))
            break;

        hsG3DDeviceRecord devRec;
        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        devRec.SetDriverName("EGL");

        if (fillDeviceRecord(devRec))
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


#ifdef HS_BUILD_FOR_WIN32
#include "hsWindows.h"
#include <epoxy/wgl.h>

void plWGLEnumerate(std::vector<hsG3DDeviceRecord>& records)
{
    ATOM cls = 0;
    HWND wnd = nullptr;
    HDC dc = nullptr;
    HGLRC ctx = nullptr;

    do {
        WNDCLASSW tempClass = {};
        tempClass.lpfnWndProc = DefWindowProc;
        tempClass.hInstance = GetModuleHandle(nullptr);
        tempClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        tempClass.lpszClassName = L"GLTestClass";
        tempClass.style = CS_OWNDC;

        cls = RegisterClassW(&tempClass);
        if (!cls)
            break;

        wnd = CreateWindowExW(WS_EX_NOACTIVATE, reinterpret_cast<LPCWSTR>(cls),
                L"OpenGL Test Window",
                WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE,
                CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        if (!wnd)
            break;

        dc = GetDC(wnd);
        if (!dc)
            break;

        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 24;
        pfd.iLayerType = PFD_MAIN_PLANE;

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
        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        devRec.SetDriverName("opengl32.dll");

        if (fillDeviceRecord(devRec))
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

    if (cls)
        UnregisterClassW(reinterpret_cast<LPCWSTR>(cls), GetModuleHandle(nullptr));
}
#endif // HS_BUILD_FOR_WIN32


#ifdef HS_BUILD_FOR_MACOS
#include <CoreGraphics/CoreGraphics.h>
#include <OpenGL/OpenGL.h>

void plCGLEnumerate(std::vector<hsG3DDeviceRecord>& records)
{
    IGNORE_WARNINGS_BEGIN("deprecated-declarations")
    CGLPixelFormatObj pix = nullptr;
    CGLContextObj ctx = nullptr;
    
    CGDirectDisplayID mainDisplay = CGMainDisplayID();

    do {
        CGLPixelFormatAttribute attribs[8] = {
            kCGLPFAAccelerated,
            kCGLPFANoRecovery,
            kCGLPFADoubleBuffer,
            kCGLPFAOpenGLProfile, (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
            kCGLPFADisplayMask, (CGLPixelFormatAttribute) CGDisplayIDToOpenGLDisplayMask(mainDisplay),
            (CGLPixelFormatAttribute) 0
        };

        int nPix = 0;
        if (CGLChoosePixelFormat(attribs, &pix, &nPix) != kCGLNoError || nPix == 0)
            break;

        if (CGLCreateContext(pix, nullptr, &ctx) != kCGLNoError)
            break;

        CGLSetCurrentContext(ctx);

        hsG3DDeviceRecord devRec;
        devRec.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        devRec.SetDriverName("OpenGL.framework");
        
        plDisplayHelper* displayHelper = plDisplayHelper::CurrentDisplayHelper();
        for (const auto& mode : displayHelper->GetSupportedDisplayModes(mainDisplay)) {
            hsG3DDeviceMode devMode;
            devMode.SetWidth(mode.Width);
            devMode.SetHeight(mode.Height);
            devMode.SetColorDepth(mode.ColorDepth);
            devRec.GetModes().emplace_back(devMode);
        }
        devRec.SetDefaultMode(devRec.GetModes().front());

        if (fillDeviceRecord(devRec))
            records.emplace_back(devRec);
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
