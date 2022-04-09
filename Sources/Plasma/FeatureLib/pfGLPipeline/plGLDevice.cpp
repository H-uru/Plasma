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

#include <epoxy/gl.h>

#include "hsThread.h"

#include "plGLDevice.h"
#include "plGLPipeline.h"

#include "plStatusLog/plStatusLog.h"

#pragma region EGL_Init
#ifdef USE_EGL
#include <epoxy/egl.h>

void InitEGLDevice(plGLDevice* dev)
{
    EGLNativeDisplayType device = static_cast<EGLNativeDisplayType>((void*)dev->fDevice);
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    do {
        if (!eglBindAPI(EGL_OPENGL_API)) {
            dev->fErrorMsg = "Could not bind to OpenGL API";
            break;
        }

        /* Set up the display */
        display = eglGetDisplay(device);
        if (display == EGL_NO_DISPLAY) {
            dev->fErrorMsg = "Could not get the display";
            break;
        }

        if (!eglInitialize(display, nullptr, nullptr)) {
            dev->fErrorMsg = "Could not initialize the display";
            break;
        }

        /* Set up the config attributes for EGL */
        EGLConfig config;
        EGLint config_count;
        EGLint config_attrs[] = {
            EGL_BUFFER_SIZE, 24,
            EGL_DEPTH_SIZE, 24,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_NONE
        };

        if (!eglChooseConfig(display, config_attrs, &config, 1, &config_count) || config_count != 1) {
            dev->fErrorMsg = "Could not choose appropriate config";
            break;
        }

        /* Set up the GL context */
        EGLint ctx_attrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
            EGL_NONE
        };

        context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attrs);
        if (context == EGL_NO_CONTEXT) {
            dev->fErrorMsg = "Unable to create rendering context";
            break;
        }

        /* Set up the rendering surface */
        surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)dev->fWindow, nullptr);
        if (surface == EGL_NO_SURFACE) {
            dev->fErrorMsg = "Unable to create rendering surface";
            break;
        }

        /* Associate everything */
        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
            dev->fErrorMsg = "Failed to attach EGL context to surface";
            break;
        }

        // Successfully initialized:
        dev->fDisplay = display;
        dev->fContext = context;
        dev->fSurface = surface;
        dev->fContextType = plGLDevice::kEGL;
        return;
    } while (0);

    // Cleanup for failure case:
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
#pragma endregion EGL_Init

#pragma region WGL_Init
#ifdef HS_BUILD_FOR_WIN32
#include "hsWindows.h"
#include <epoxy/wgl.h>

void InitWGLDevice(plGLDevice* dev)
{
    HDC dc = static_cast<HDC>((HANDLE)dev->fDevice);
    HGLRC ctx = nullptr;

    do {
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_GENERIC_ACCELERATED | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 24;
        pfd.cDepthBits = 24;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int format = ChoosePixelFormat(dc, &pfd);
        if (!format) {
            dev->fErrorMsg = "Could not find appropriate pixel config";
            break;
        }

        if (!SetPixelFormat(dc, format, &pfd)) {
            dev->fErrorMsg = "Could not set appropriate pixel config";
            break;
        }

        ctx = wglCreateContext(dc);
        if (!ctx) {
            dev->fErrorMsg = "Unable to create rendering context";
            break;
        }

        if (!wglMakeCurrent(dc, ctx)) {
            dev->fErrorMsg = "Failed to attach WGL context to surface";
            break;
        }

        // Successfully initialized:
        dev->fContext = ctx;
        dev->fContextType = plGLDevice::kWGL;
        return;
    } while (0);

    // Cleanup for failure case:
    if (ctx) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(ctx);
    }
}
#endif // HS_BUILD_FOR_WIN32
#pragma endregion WGL_Init

#pragma region CGL_Init
#ifdef HS_BUILD_FOR_MACOS
#include <AvailabilityMacros.h>
#include <OpenGL/OpenGL.h>
void InitCGLDevice(plGLDevice* dev)
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
            dev->fErrorMsg = "Could not choose appropriate config";
            break;
        }

        if (CGLCreateContext(pix, nullptr, &ctx) != kCGLNoError) {
            dev->fErrorMsg = "Unable to create rendering context";
            break;
        }

        if (CGLSetCurrentContext(ctx) != kCGLNoError) {
            dev->fErrorMsg = "Failed to attach CGL context to surface";
            break;
        }

        CGLReleasePixelFormat(pix);

        // Successfully initialized:
        dev->fContext = ctx;
        dev->fContextType = plGLDevice::kCGL;
        return;
    } while (0);

    // Cleanup for failure case:
    if (ctx) {
        CGLSetCurrentContext(nullptr);
        CGLReleaseContext(ctx);
    }

    if (pix)
        CGLReleasePixelFormat(pix);

    IGNORE_WARNINGS_END
}
#endif // HS_BUILD_FOR_MACOS
#pragma endregion CGL_Init

#ifdef HS_DEBUGGING
static void GLAPIENTRY plGLDebugLog(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (severity <= GL_DEBUG_SEVERITY_MEDIUM) { // Yes, higher is a lower enum value
        plStatusLog::AddLineSF("pipeline.log", "[GL] {}{}", (type == GL_DEBUG_TYPE_ERROR ? "** ERROR **: " : ""), message);
    }
}
#endif

plGLDevice::plGLDevice()
    : fErrorMsg(), fPipeline(), fContextType(kNone), fWindow(), fDevice(),
    fDisplay(), fSurface(), fContext(), fActiveThread()
{
}

bool plGLDevice::InitDevice()
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
    if (epoxy_has_egl())
        InitEGLDevice(this);
#endif

#ifdef HS_BUILD_FOR_WIN32
    if (fContextType == kNone)
        InitWGLDevice(this);
#endif

#ifdef HS_BUILD_FOR_MACOS
    if (fContextType == kNone)
        InitCGLDevice(this);
#endif

    // If we still don't have a valid context type set by this point, we've
    // failed to initialize so we need to exit.
    if (fContextType == kNone)
        return false;

    plStatusLog::AddLineSF("pipeline.log", "Initialized with OpenGL {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

#ifdef HS_DEBUGGING
    if (epoxy_gl_version() >= 43) {
        glEnable(GL_DEBUG_OUTPUT);

        // Turn off low-severity messages
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
        glDebugMessageCallback(plGLDebugLog, 0);
    }
#endif

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    return true;
}

void plGLDevice::SetRenderTarget(plRenderTarget* target)
{
}

void plGLDevice::SetViewport()
{
}

bool plGLDevice::BeginRender()
{
#ifdef HS_BUILD_FOR_WIN32
    // Best practice, apparently, is to get and release the DC every time we need it.
    // A DC is only valid on one thread at a time.
    fDevice = static_cast<hsWindowHndl>((HANDLE)GetDC(fWindow));
#endif

    if (fActiveThread == hsThread::ThisThreadHash()) {
        return true;
    }

    fActiveThread = hsThread::ThisThreadHash();

    // Initialize OpenGL
    if (!InitDevice()) {
        plStatusLog::AddLineS("pipeline.log", GetErrorString());
        return false;
    }

    return true;
}

bool plGLDevice::EndRender()
{
    if (fPipeline->fCurrRenderTarget != nullptr) {
        return true;
    }

#ifdef USE_EGL
    if (fContextType == kEGL) {
        if (eglSwapBuffers(static_cast<EGLDisplay>(fDisplay), static_cast<EGLSurface>(fSurface)) == EGL_FALSE) {
            fErrorMsg = "Failed to swap buffers";
            return false;
        }
        return true;
    } else
#endif

#ifdef HS_BUILD_FOR_WIN32
    if (fContextType == kWGL) {
        SwapBuffers(static_cast<HDC>((HANDLE)fDevice));
    }

    ReleaseDC(fWindow, static_cast<HDC>((HANDLE)fDevice));
    fDevice = nullptr;
#endif

    return false;
}

void plGLDevice::SetProjectionMatrix(const hsMatrix44& src)
{
}

void plGLDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
}

void plGLDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
}
