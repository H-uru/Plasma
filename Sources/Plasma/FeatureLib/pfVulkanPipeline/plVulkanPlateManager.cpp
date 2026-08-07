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

#include "plVulkanPlateManager.h"
#include "plVulkanPipeline.h"

#include <string_theory/string>

#include <cstring>

namespace
{
    /** Interleaved to match the plate pipeline's vertex input. */
    struct plPlateVertex
    {
        float fX, fY;
        float fU, fV, fW;
    };

    // The unit quad every plate is drawn with, from
    // plMetalPlateManager::ICreateGeometry.
    const plPlateVertex kQuad[4] = {
        { -0.5f, -0.5f, 0.f, 0.f, 0.f },
        { -0.5f,  0.5f, 0.f, 1.f, 0.f },
        {  0.5f, -0.5f, 1.f, 0.f, 0.f },
        {  0.5f,  0.5f, 1.f, 1.f, 0.f },
    };

    const uint16_t kQuadIndices[6] = { 0, 1, 2, 1, 2, 3 };
}

plVulkanPlateManager::plVulkanPlateManager(plVulkanPipeline* pipe)
    : plPlateManager(pipe)
{
    ICreateGeometry();
}

plVulkanPlateManager::~plVulkanPlateManager()
{
    IReleaseGeometry();
}

void plVulkanPlateManager::ICreateGeometry()
{
    plVulkanPipeline* pipeline = static_cast<plVulkanPipeline*>(fOwner);
    plVulkanDevice& device = pipeline->fDevice;

    if (fVertexBuffer.IsValid())
        return;

    fVertexBuffer = device.CreateBuffer(sizeof(kQuad), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                                        ST_LITERAL("plate quad vertices"));
    fIndexBuffer = device.CreateBuffer(sizeof(kQuadIndices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true,
                                       ST_LITERAL("plate quad indices"));

    if (fVertexBuffer.IsValid())
        memcpy(fVertexBuffer.fMapped, kQuad, sizeof(kQuad));
    if (fIndexBuffer.IsValid())
        memcpy(fIndexBuffer.fMapped, kQuadIndices, sizeof(kQuadIndices));
}

void plVulkanPlateManager::IReleaseGeometry()
{
    plVulkanPipeline* pipeline = static_cast<plVulkanPipeline*>(fOwner);
    if (!pipeline)
        return;

    pipeline->fDevice.RetireBuffer(fVertexBuffer);
    pipeline->fDevice.RetireBuffer(fIndexBuffer);

    fVertexBuffer = plVulkanBuffer{};
    fIndexBuffer = plVulkanBuffer{};
}

void plVulkanPlateManager::EncodeDraw(VkCommandBuffer cmd)
{
    if (!fVertexBuffer.IsValid() || !fIndexBuffer.IsValid())
        return;

    plVulkanPipeline* pipeline = static_cast<plVulkanPipeline*>(fOwner);
    plVulkanDebugLabel label(pipeline->fDevice, cmd, ST_LITERAL("Draw plate"));

    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &fVertexBuffer.fBuffer, &offset);
    vkCmdBindIndexBuffer(cmd, fIndexBuffer.fBuffer, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(cmd, 6, 1, 0, 0, 0);
}

void plVulkanPlateManager::IDrawToDevice(plPipeline* pipe)
{
    plVulkanPipeline* pipeline = static_cast<plVulkanPipeline*>(pipe);

    for (plPlate* plate = fPlates; plate != nullptr; plate = plate->GetNext()) {
        if (plate->IsVisible())
            pipeline->IDrawPlate(plate);
    }
}
