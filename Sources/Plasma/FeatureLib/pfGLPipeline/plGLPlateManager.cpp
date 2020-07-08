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
#include "plGLPlateManager.h"
#include "plGLPipeline.h"

plGLPlateManager::plGLPlateManager(plGLPipeline* pipe)
    : plPlateManager(pipe), fBuffers { 0, 0, 0 }
{}

plGLPlateManager::~plGLPlateManager()
{
    IReleaseGeometry();
}

void plGLPlateManager::ICreateGeometry()
{
    plPlateVertex verts[4];

    verts[0].fPoint.Set(-0.5f, -0.5f, 0.0f);
    verts[0].fNormal.Set(0.0f, 0.0f, 1.0f);
    verts[0].fColor = 0xffffffff;
    verts[0].fUV.Set(0.0f, 0.0f, 0.0f);

    verts[1].fPoint.Set(-0.5f, 0.5f, 0.0f);
    verts[1].fNormal.Set(0.0f, 0.0f, 1.0f);
    verts[1].fColor = 0xffffffff;
    verts[1].fUV.Set(0.0f, 1.0f, 0.0f);

    verts[2].fPoint.Set(0.5f, -0.5f, 0.0f);
    verts[2].fNormal.Set(0.0f, 0.0f, 1.0f);
    verts[2].fColor = 0xffffffff;
    verts[2].fUV.Set(1.0f, 0.0f, 0.0f);

    verts[3].fPoint.Set(0.5f, 0.5f, 0.0f);
    verts[3].fNormal.Set(0.0f, 0.0f, 1.0f);
    verts[3].fColor = 0xffffffff;
    verts[3].fUV.Set(1.0f, 1.0f, 0.0f);

    uint16_t indices[6] = {0, 1, 2, 1, 2, 3};


    GLuint vbo, ibo, vao = 0;

    if (epoxy_gl_version() >= 45) {
        glCreateBuffers(1, &vbo);
        glObjectLabel(GL_BUFFER, vbo, -1, "plPlate/VBO");
        glNamedBufferStorage(vbo, sizeof(verts), verts, 0);

        glCreateBuffers(1, &ibo);
        glObjectLabel(GL_BUFFER, ibo, -1, "plPlate/IBO");
        glNamedBufferStorage(ibo, sizeof(indices), indices, 0);

        glCreateVertexArrays(1, &vao);
        glObjectLabel(GL_VERTEX_ARRAY, vao, -1, "plPlate/VAO");

        glVertexArrayVertexBuffer(vao, 0, vbo, 0, sizeof(plPlateVertex));
        glVertexArrayElementBuffer(vao, ibo);

        glEnableVertexArrayAttrib(vao, kVtxPosition);
        glEnableVertexArrayAttrib(vao, kVtxNormal);
        glEnableVertexArrayAttrib(vao, kVtxColor);
        glEnableVertexArrayAttrib(vao, kVtxUVWSrc0);

        glVertexArrayAttribFormat(vao, kVtxPosition, 3, GL_FLOAT, GL_FALSE, offsetof(plPlateVertex, fPoint));
        glVertexArrayAttribFormat(vao, kVtxNormal, 3, GL_FLOAT, GL_FALSE, offsetof(plPlateVertex, fNormal));
        glVertexArrayAttribFormat(vao, kVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, offsetof(plPlateVertex, fColor));
        glVertexArrayAttribFormat(vao, kVtxUVWSrc0, 3, GL_FLOAT, GL_FALSE, offsetof(plPlateVertex, fUV));

        glVertexArrayAttribBinding(vao, kVtxPosition, 0);
        glVertexArrayAttribBinding(vao, kVtxNormal, 0);
        glVertexArrayAttribBinding(vao, kVtxColor, 0);
        glVertexArrayAttribBinding(vao, kVtxUVWSrc0, 0);
    } else {
        if (epoxy_gl_version() >= 30) {
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);
        }

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        if (epoxy_gl_version() >= 30) {
            glEnableVertexAttribArray(kVtxPosition);
            glVertexAttribPointer(kVtxPosition, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), 0);

            glEnableVertexAttribArray(kVtxNormal);
            glVertexAttribPointer(kVtxNormal, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), (void*)(sizeof(float) * 3));

            glEnableVertexAttribArray(kVtxColor);
            glVertexAttribPointer(kVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(plPlateVertex), (void*)(sizeof(float) * 3 * 2));

            glEnableVertexAttribArray(kVtxUVWSrc0);
            glVertexAttribPointer(kVtxUVWSrc0, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), (void*)((sizeof(float) * 3 * 2) + sizeof(uint32_t)));

            glBindVertexArray(0);
        }
    }

    fBuffers = {vbo, ibo, vao };
}

void plGLPlateManager::IReleaseGeometry()
{
    if (epoxy_gl_version() >= 30 && fBuffers.ARef) {
        if (epoxy_gl_version() >= 45) {
            glDisableVertexArrayAttrib(fBuffers.ARef, kVtxPosition);
            glDisableVertexArrayAttrib(fBuffers.ARef, kVtxNormal);
            glDisableVertexArrayAttrib(fBuffers.ARef, kVtxColor);
            glDisableVertexArrayAttrib(fBuffers.ARef, kVtxUVWSrc0);
        }

        glDeleteVertexArrays(1, &fBuffers.ARef);
        fBuffers.ARef = 0;
    }

    if (fBuffers.VRef) {
        glDeleteBuffers(1, &fBuffers.VRef);
        fBuffers.VRef = 0;
    }

    if (fBuffers.IRef) {
        glDeleteBuffers(1, &fBuffers.IRef);
        fBuffers.IRef = 0;
    }
}

void plGLPlateManager::IDrawToDevice(plPipeline* pipe)
{
    plGLPipeline* glPipe = static_cast<plGLPipeline*>(pipe);
    uint32_t scrnWidthDiv2  = fOwner->Width() >> 1;
    uint32_t scrnHeightDiv2 = fOwner->Height() >> 1;
    plPlate* plate = nullptr;

    if (fBuffers.VRef == 0 || fBuffers.IRef == 0)
        return;

    if (epoxy_gl_version() >= 30) {
        if (fBuffers.ARef == 0)
            return;

        glBindVertexArray(fBuffers.ARef);
    } else {
        glBindBuffer(GL_ARRAY_BUFFER, fBuffers.VRef);

        glVertexAttribPointer(kVtxPosition, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), 0);
        glEnableVertexAttribArray(kVtxPosition);

        glVertexAttribPointer(kVtxNormal, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(kVtxNormal);

        glVertexAttribPointer(kVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(plPlateVertex), (void*)(sizeof(float) * 3 * 2));
        glEnableVertexAttribArray(kVtxColor);

        glVertexAttribPointer(kVtxUVWSrc0, 3, GL_FLOAT, GL_FALSE, sizeof(plPlateVertex), (void*)((sizeof(float) * 3 * 2) + sizeof(uint32_t)));
        glEnableVertexAttribArray(kVtxUVWSrc0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fBuffers.IRef);
    }

    glPipe->fDevice.SetWorldToCameraMatrix(hsMatrix44::IdentityMatrix());

    for (plate = fPlates; plate != nullptr; plate = plate->GetNext()) {
        if (plate->IsVisible())
            glPipe->IDrawPlate(plate);
    }
}
