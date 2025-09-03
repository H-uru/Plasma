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

#include "plWGLDevice.h"

#ifdef HS_BUILD_FOR_WIN32

bool plWGLDevice::Enumerate(hsG3DDeviceRecord& record)
{
    bool result = false;
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
        tempClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

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

        if (epoxy_gl_version() < 33)
            break;

        record.SetG3DDeviceType(hsG3DDeviceSelector::kDevTypeOpenGL);
        record.SetDriverName("opengl32.dll");
        record.SetDeviceDesc(reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        record.SetDriverDesc(reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        record.SetDriverVersion(reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        result = true;
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

    return result;
}


plWGLDevice* plWGLDevice::TryInit(hsWindowHndl window, hsWindowHndl device, ST::string& error)
{
    HDC dc = static_cast<HDC>((HANDLE)device);
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
            error = ST_LITERAL("Could not find appropriate pixel config");
            break;
        }

        if (!SetPixelFormat(dc, format, &pfd)) {
            error = ST_LITERAL("Could not set appropriate pixel config");
            break;
        }

        ctx = wglCreateContext(dc);
        if (!ctx) {
            error = ST_LITERAL("Unable to create rendering context");
            break;
        }

        if (!wglMakeCurrent(dc, ctx)) {
            error = ST_LITERAL("Failed to attach WGL context to surface");
            break;
        }

        // Successfully initialized
        return new plWGLDevice(window, device, ctx);
    } while (0);

    // Cleanup for failure case:
    if (ctx) {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(ctx);
    }
    return nullptr;
}


plWGLDevice::plWGLDevice(hsWindowHndl window, hsWindowHndl device, HGLRC context)
    : plGLDeviceImpl(window, device), fContext(context)
{ }

void plWGLDevice::Shutdown()
{
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(fContext);
    fContext = nullptr;
}

bool plWGLDevice::BeginRender(ST::string& error)
{
    // Best practice, apparently, is to get and release the DC every time we need it.
    // A DC is only valid on one thread at a time.
    fDevice = static_cast<hsWindowHndl>((HANDLE)GetDC(fWindow));

    return true;
}

bool plWGLDevice::EndRender(ST::string& error)
{
    SwapBuffers(static_cast<HDC>((HANDLE)fDevice));

    ReleaseDC(fWindow, static_cast<HDC>((HANDLE)fDevice));
    fDevice = nullptr;
    return true;
}

#endif // HS_BUILD_FOR_WIN32
