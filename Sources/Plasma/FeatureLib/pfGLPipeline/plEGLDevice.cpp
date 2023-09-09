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

#include "plEGLDevice.h"

#ifdef USE_EGL

bool plEGLDevice::Enumerate(hsG3DDeviceRecord& record)
{
    bool result = false;
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

        if (epoxy_gl_version() < 33)
            break;

        record.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        record.SetDriverName("EGL");
        record.SetDeviceDesc(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        record.SetDriverDesc(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        record.SetDriverVersion(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        result = true;
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

    return result;
}


plEGLDevice* plEGLDevice::TryInit(hsWindowHndl window, hsWindowHndl device, ST::string& error)
{
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;

    do {
        if (!eglBindAPI(EGL_OPENGL_API)) {
            error = ST_LITERAL("Could not bind to OpenGL API");
            break;
        }

        /* Set up the display */
        display = eglGetDisplay(device);
        if (display == EGL_NO_DISPLAY) {
            error = ST_LITERAL("Could not get the display");
            break;
        }

        if (!eglInitialize(display, nullptr, nullptr)) {
            error = ST_LITERAL("Could not initialize the display");
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
            error = ST_LITERAL("Could not choose appropriate config");
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
            error = ST_LITERAL("Unable to create rendering context");
            break;
        }

        /* Set up the rendering surface */
        surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType)window, nullptr);
        if (surface == EGL_NO_SURFACE) {
            error = ST_LITERAL("Unable to create rendering surface");
            break;
        }

        /* Associate everything */
        if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
            error = ST_LITERAL("Failed to attach EGL context to surface");
            break;
        }

        // Successfully initialized
        return new plEGLDevice(window, device, display, context, surface);
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

    return nullptr;
}


plEGLDevice::plEGLDevice(hsWindowHndl window, hsWindowHndl device, EGLDisplay display, EGLContext context, EGLSurface surface)
    : plGLDeviceImpl(window, device), fDisplay(display), fContext(context), fSurface(surface)
{ }

void plEGLDevice::Shutdown()
{
    eglDestroySurface(fDisplay, fSurface);
    fSurface = EGL_NO_SURFACE;

    eglDestroyContext(fDisplay, fContext);
    fContext = EGL_NO_CONTEXT;

    eglTerminate(fDisplay);
    fDisplay = EGL_NO_DISPLAY;
}

bool plEGLDevice::BeginRender(ST::string& error)
{
    if (eglMakeCurrent(fDisplay, fSurface, fSurface, fContext) == EGL_FALSE) {
        error = ST_LITERAL("Failed to attach EGL context to surface");
        return false;
    }

    return true;
}

bool plEGLDevice::EndRender(ST::string& error)
{
    if (eglSwapBuffers(fDisplay, fSurface) == EGL_FALSE) {
        error = ST_LITERAL("Failed to swap buffers");
        return false;
    }

    return true;
}
#endif // USE_EGL
