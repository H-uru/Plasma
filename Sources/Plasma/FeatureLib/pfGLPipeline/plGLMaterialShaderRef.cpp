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

#include "plGLMaterialShaderRef.h"

#include <epoxy/gl.h>

#include "HeadSpin.h"
#include "plSurface/hsGMaterial.h"

plGLMaterialShaderRef::~plGLMaterialShaderRef()
{
    Release();
}

void plGLMaterialShaderRef::Release()
{
    if (fVertShaderRef) {
        glDeleteShader(fVertShaderRef);
        fVertShaderRef = 0;
    }

    if (fFragShaderRef) {
        glDeleteShader(fFragShaderRef);
        fFragShaderRef = 0;
    }

    if (fRef) {
        glDeleteProgram(fRef);
        fRef = 0;
    }

    SetDirty(true);
}


void plGLMaterialShaderRef::ICompile()
{
    const char* vs_src = "#version 100"
                     "\n"
                     "\n" "uniform mat4 uMatrixL2W;"
                     "\n" "uniform mat4 uMatrixW2C;"
                     "\n" "uniform mat4 uMatrixProj;"
                     "\n"
                     "\n" "attribute vec3 aVtxPosition;"
                     "\n" "attribute vec3 aVtxNormal;"
                     "\n" "attribute vec4 aVtxColor;"
                     "\n"
                     "\n" "varying vec3 vVtxNormal;"
                     "\n" "varying vec4 vVtxColor;"
                     "\n"
                     "\n" "void main() {"
                     "\n" "    vVtxNormal = aVtxNormal;"
                     "\n" "    vVtxColor = aVtxColor.zyxw;"
                     "\n"
                     "\n" "    vec4 pos = uMatrixL2W * vec4(aVtxPosition, 1.0);"
                     "\n" "         pos = uMatrixW2C * pos;"
                     "\n" "         pos = uMatrixProj * pos;"
                     "\n"
                     "\n" "    gl_Position = pos;"
                     "\n" "}";

    const char* fs_src = "#version 100"
                     "\n"
                     "\n" "varying lowp vec3 vVtxNormal;"
                     "\n" "varying mediump vec4 vVtxColor;"
                     "\n"
                     "\n" "void main() {"
                     "\n" "    gl_FragColor = vVtxColor;"
                     "\n" "}";

    fVertShaderRef = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(fVertShaderRef, 1, &vs_src, nullptr);
    glCompileShader(fVertShaderRef);

#ifdef HS_DEBUGGING
    {
        GLint compiled = 0;
        glGetShaderiv(fVertShaderRef, GL_COMPILE_STATUS, &compiled);
        if (compiled == 0) {
            GLint length = 0;
            glGetShaderiv(fVertShaderRef, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char* log = new char[length];
                glGetShaderInfoLog(fVertShaderRef, length, &length, log);
                hsStatusMessage(log);
            }
        }
    }
#endif

    fFragShaderRef = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fFragShaderRef, 1, &fs_src, nullptr);
    glCompileShader(fFragShaderRef);

#ifdef HS_DEBUGGING
    {
        GLint compiled = 0;
        glGetShaderiv(fFragShaderRef, GL_COMPILE_STATUS, &compiled);
        if (compiled == 0) {
            GLint length = 0;
            glGetShaderiv(fFragShaderRef, GL_INFO_LOG_LENGTH, &length);
            if (length) {
                char* log = new char[length];
                glGetShaderInfoLog(fFragShaderRef, length, &length, log);
                hsStatusMessage(log);
            }
        }
    }
#endif

    fRef = glCreateProgram();
    glAttachShader(fRef, fVertShaderRef);
    glAttachShader(fRef, fFragShaderRef);

    glLinkProgram(fRef);

#ifdef HS_DEBUGGING
    GLenum e;
    if ((e = glGetError()) != GL_NO_ERROR) {
        hsStatusMessage(ST::format("Prg Link failed {}", uint32_t(e)).c_str());
    }
#endif
}
