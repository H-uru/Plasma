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

// Aux spans.
//
// A drawable can carry extra geometry to be drawn immediately after its own --
// that is what plDynaDecal uses for footprints, bullet hits and the water ripples
// WaveRip.vert displaces. The geometry lives in its own buffer group with its own
// material, so it is a small self-contained render rather than another pass over
// the span.
//
// Ported from plMetalPipeline.cpp:1424-1523.

#include "plVulkanPipeline.h"

#include "plVulkanDevice.h"
#include "plVulkanDeviceRef.h"
#include "plVulkanMaterialShaderRef.h"

#include "plDrawable/plAuxSpan.h"
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plSpanTypes.h"
#include "plPipeDebugFlags.h"
#include "plSurface/hsGMaterial.h"

// Ported from plMetalPipeline::ICheckAuxBuffers (:1424-1449).
bool plVulkanPipeline::ICheckAuxBuffers(const plAuxSpan* span)
{
    plGBufferGroup* group = span->fGroup;

    plVulkanVertexBufferRef* vRef =
        static_cast<plVulkanVertexBufferRef*>(group->GetVertexBufferRef(span->fVBufferIdx));
    if (!vRef)
        return true;

    plVulkanIndexBufferRef* iRef =
        static_cast<plVulkanIndexBufferRef*>(group->GetIndexBufferRef(span->fIBufferIdx));
    if (!iRef)
        return true;

    // A volatile buffer whose generation is stale has not been written this frame.
    if (vRef->Expired(fVtxRefTime))
        IRefreshDynVertices(group, vRef);

    return false;
}

// Ported from plMetalPipeline::IRenderAuxSpan (:1470-1523).
void plVulkanPipeline::IRenderAuxSpan(VkCommandBuffer cmd, const plSpan& span,
                                     const plAuxSpan* aux)
{
    CheckVertexBufferRef(aux->fGroup, aux->fVBufferIdx);
    CheckIndexBufferRef(aux->fGroup, aux->fIBufferIdx);
    ICheckAuxBuffers(aux);

    plVulkanVertexBufferRef* vRef =
        static_cast<plVulkanVertexBufferRef*>(aux->fGroup->GetVertexBufferRef(aux->fVBufferIdx));
    plVulkanIndexBufferRef* iRef =
        static_cast<plVulkanIndexBufferRef*>(aux->fGroup->GetIndexBufferRef(aux->fIBufferIdx));

    if (!vRef || !iRef || !vRef->GetBuffer().IsValid() || !iRef->GetBuffer().IsValid())
        return;

    hsGMaterial* material = aux->fMaterial;
    if (!material)
        return;

    plVulkanMaterialShaderRef* mRef =
        static_cast<plVulkanMaterialShaderRef*>(material->GetDeviceRef());
    if (!mRef) {
        mRef = new plVulkanMaterialShaderRef(material, this);
        material->SetDeviceRef(mRef);
        mRef->UnRef();
    }
    if (!mRef->IsLinked())
        mRef->Link(&fMaterialRefList);

    mRef->CheckMaterialRef();

    const VkBuffer vertexBuffer = vRef->GetBuffer().fBuffer;
    fState.fVertexBuffer = vertexBuffer;
    const VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cmd, 0, 1, &vertexBuffer, &offset);

    const VkBuffer indexBuffer = iRef->GetBuffer().fBuffer;
    fState.fIndexBuffer = indexBuffer;
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);

    // A decal that overrides the lighting model wants flat white ambient and
    // diffuse: it is a stamp on the surface, not a lit surface of its own. That
    // has to be applied inside IHandleMaterialPass, after it calculates lighting
    // but before it uploads the block.
    const bool overrideLiteModel = (aux->fFlags & plAuxSpan::kOverrideLiteModel) != 0;

    for (size_t pass = 0; pass < mRef->GetNumPasses(); pass++) {
        if (!IHandleMaterialPass(cmd, material, pass, &span, vRef, true, overrideLiteModel))
            continue;

        vkCmdDrawIndexed(cmd, aux->fILength, 1, aux->fIStartIdx, 0, 0);
    }
}

// Ported from plMetalPipeline::IRenderAuxSpans (:1451-1463).
void plVulkanPipeline::IRenderAuxSpans(VkCommandBuffer cmd, const plSpan& span)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoAuxSpans))
        return;

    plVulkanDebugLabel label(fDevice, cmd, ST_LITERAL("Render aux spans"));

    // Aux geometry is authored in world space, so it draws with no local
    // transform and the span's is put back afterwards.
    ISetLocalToWorld(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());

    for (size_t i = 0; i < span.GetNumAuxSpans(); i++)
        IRenderAuxSpan(cmd, span, span.GetAuxSpan(i));

    ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);
}
