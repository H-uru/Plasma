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

#include "plDrawable/plGBufferGroup.h"
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

static float kIdentityMatrix[16] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

GLfloat* hsMatrix2GL(const hsMatrix44& src, GLfloat* dst)
{
    if (src.fFlags & hsMatrix44::kIsIdent)
        return static_cast<GLfloat*>(memcpy(dst, kIdentityMatrix, sizeof(GLfloat) * 16));
    else
        return static_cast<GLfloat*>(memcpy(dst, src.fMap, sizeof(GLfloat) * 16));
}

plGLDevice::plGLDevice()
    : fErrorMsg(), fPipeline(), fContextType(kNone), fWindow(), fDevice(),
    fDisplay(), fSurface(), fContext(), fActiveThread(), fCurrentProgram()
{
    memcpy(fMatrixL2W, kIdentityMatrix, sizeof(GLfloat) * 16);
    memcpy(fMatrixW2C, kIdentityMatrix, sizeof(GLfloat) * 16);
    memcpy(fMatrixProj, kIdentityMatrix, sizeof(GLfloat) * 16);
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
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);

    return true;
}

void plGLDevice::SetRenderTarget(plRenderTarget* target)
{
    SetViewport();
}

void plGLDevice::SetViewport()
{
    glViewport(fPipeline->GetViewTransform().GetViewPortLeft(),
               fPipeline->GetViewTransform().GetViewPortTop(),
               fPipeline->GetViewTransform().GetViewPortWidth(),
               fPipeline->GetViewTransform().GetViewPortHeight());
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

void plGLDevice::SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, VertexBufferRef* vRef)
{
    uint8_t format = owner->GetVertexFormat();

    // All indexed skinning is currently done on CPU, so the source data
    // will have indices, but we strip them out for the D3D buffer.
    if (format & plGBufferGroup::kSkinIndices) {
        hsStatusMessage("Have to deal with skinning :(");
#if 0
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
#endif
    }


    uint32_t vertSize = owner->GetVertexSize(); //IGetBufferFormatSize(format); // vertex stride
    uint32_t numVerts = owner->GetVertBufferCount(idx);

    vRef->fOwner = owner;
    vRef->fCount = numVerts;
    vRef->fVertexSize = vertSize;
    vRef->fFormat = format;
    vRef->fRefTime = 0;

    vRef->SetDirty(true);
    vRef->SetRebuiltSinceUsed(true);
    vRef->fData = nullptr;

    vRef->SetVolatile(vRef->Volatile() || owner->AreVertsVolatile());

    vRef->fIndex = idx;

    owner->SetVertexBufferRef(idx, vRef);
    hsRefCnt_SafeUnRef(vRef);
}

void plGLDevice::CheckStaticVertexBuffer(VertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");

    if (!vRef->fRef) {
        glGenBuffers(1, &vRef->fRef);

        // Fill in the vertex data.
        FillStaticVertexBufferRef(vRef, owner, idx);

        // This is currently a no op, but this would let the buffer know it can
        // unload the system memory copy, since we have a managed version now.
        owner->PurgeVertBuffer(idx);
    }
}

void plGLDevice::FillStaticVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    if (!ref->fRef)
        // We most likely already warned about this earlier, best to just quietly return now
        return;

    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;
    if (!size)
        return;

    glBindBuffer(GL_ARRAY_BUFFER, ref->fRef);

    if (ref->fData) {
        glBufferData(GL_ARRAY_BUFFER, size, ref->fData + vertStart, GL_STATIC_DRAW);
    } else {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");

        uint8_t* buffer = new uint8_t[size];
        uint8_t* ptr = buffer;
        const uint32_t vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t* srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData(idx);

        const int numCells = group->GetNumCells(idx);
        for (int i = 0; i < numCells; i++) {
            plGBufferCell* cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1)) {
                /// Interleaved, do straight copy
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
            } else {
                hsStatusMessage("Non interleaved data");

                /// Separated, gotta interleave
                uint8_t* tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;

                for (int j = 0; j < cell->fLength; j++)
                {
                    memcpy(ptr, tempVPtr, sizeof(hsPoint3) * 2);
                    ptr += sizeof(hsPoint3) * 2;
                    tempVPtr += sizeof(hsPoint3) * 2;

                    memcpy(ptr, &tempCPtr->fDiffuse, sizeof(uint32_t));
                    ptr += sizeof(uint32_t);
                    memcpy(ptr, &tempCPtr->fSpecular, sizeof(uint32_t));
                    ptr += sizeof(uint32_t);

                    memcpy(ptr, tempVPtr, vertSmallSize);
                    ptr += vertSmallSize;
                    tempVPtr += vertSmallSize;
                    tempCPtr++;
                }
            }
        }

        hsAssert((ptr - buffer) == size, "Didn't fill the buffer?");
        glBufferData(GL_ARRAY_BUFFER, size, buffer, GL_STATIC_DRAW);

        delete[] buffer;
    }

    /// Unlock and clean up
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plGLDevice::FillVolatileVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    hsStatusMessage("Trying to fill volatile vertex buffer ref!");
}

void plGLDevice::SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, IndexBufferRef* iRef)
{
    uint32_t numIndices = owner->GetIndexBufferCount(idx);
    iRef->fCount = numIndices;
    iRef->fOwner = owner;
    iRef->fIndex = idx;
    iRef->fRefTime = 0;

    iRef->SetDirty(true);
    iRef->SetRebuiltSinceUsed(true);

    owner->SetIndexBufferRef(idx, iRef);
    hsRefCnt_SafeUnRef(iRef);

    iRef->SetVolatile(owner->AreIdxVolatile());
}

void plGLDevice::CheckIndexBuffer(IndexBufferRef* iRef)
{
    if (!iRef->fRef && iRef->fCount) {
        iRef->SetVolatile(false);

        glGenBuffers(1, &iRef->fRef);

        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plGLDevice::FillIndexBufferRef(IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);

    if (!size)
        return;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iRef->fRef);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, owner->GetIndexBufferData(idx) + startIdx, GL_STATIC_DRAW);

    iRef->SetDirty(false);
}

void plGLDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    hsMatrix2GL(src, fMatrixProj);
}

void plGLDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    hsMatrix2GL(src, fMatrixW2C);
}

void plGLDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
    hsMatrix2GL(src, fMatrixL2W);
}
