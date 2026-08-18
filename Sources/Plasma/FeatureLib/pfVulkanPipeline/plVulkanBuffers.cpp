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

// Geometry upload. Ported from plMetalDevice.cpp:510-731.
//
// Vertex and index buffers are host-visible and persistently mapped, which is
// the direct analogue of the shared storage mode the Metal backend uses. On a
// discrete card that means the GPU reads geometry over PCIe rather than from
// VRAM. It is the simple, obviously-correct thing while the draw path is being
// brought up; staging these into device-local memory is a contained change once
// there is something to measure.
//
// Textures are a different matter and do get staged: a BC-compressed image in
// optimal tiling cannot be host-mapped at all.

#include "plVulkanDevice.h"

#include "hsGeometry3.h"

#include "plDrawable/plGBufferGroup.h"

#include <string_theory/format>

#include <cstring>

template <typename T>
static inline void inlCopy(uint8_t*& src, uint8_t*& dst)
{
    memcpy(dst, src, sizeof(T));
    src += sizeof(T);
    dst += sizeof(T);
}

template <typename T, size_t N>
static inline void inlSkip(uint8_t*& src)
{
    src += sizeof(T) * N;
}

/** Stride of the layout as it reaches the GPU. Ported from plMetalDevice.cpp:510. */
static uint32_t IGetBufferFormatSize(uint8_t format)
{
    // Position and normal, then the two packed colors.
    uint32_t size = sizeof(hsPoint3) * 2 + sizeof(uint32_t) * 2;

    switch (format & plGBufferGroup::kSkinWeightMask) {
    case plGBufferGroup::kSkinNoWeights:
        break;
    case plGBufferGroup::kSkin1Weight:
        size += sizeof(float);
        break;
    default:
        hsAssert(false, "Invalid skin weight value in IGetBufferFormatSize()");
    }

    size += sizeof(hsPoint3) * plGBufferGroup::CalcNumUVs(format);

    return size;
}

void plVulkanDevice::SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx,
                                          VertexBufferRef* vRef)
{
    uint8_t format = owner->GetVertexFormat();

    // Anything with skin indices is blended on the CPU by ISoftwareVertexBlend,
    // so the weights and indices never reach the GPU and the ref's format drops
    // them. Everything downstream -- the pipeline key especially -- must read
    // the format from here, not from the group.
    if (format & plGBufferGroup::kSkinIndices) {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
    }

    vRef->fOwner = owner;
    vRef->fCount = owner->GetVertBufferCount(idx);
    vRef->fVertexSize = IGetBufferFormatSize(format);
    vRef->fFormat = format;
    vRef->fRefTime = 0;

    vRef->SetDirty(true);
    vRef->SetRebuiltSinceUsed(true);
    vRef->fData = nullptr;

    vRef->SetVolatile(vRef->Volatile() || owner->AreVertsVolatile());
    vRef->fIndex = idx;

    owner->SetVertexBufferRef(idx, vRef);
    vRef->UnRef();
}

void plVulkanDevice::CheckStaticVertexBuffer(VertexBufferRef* vRef, plGBufferGroup* owner,
                                             uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a static vertex buffer for a volatile ref");

    if (!vRef->GetBuffer().IsValid()) {
        FillVertexBufferRef(vRef, owner, idx);

        // Lets the group drop its system memory copy now that we hold one.
        owner->PurgeVertBuffer(idx);
    }
}

void plVulkanDevice::FillVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;

    if (!size)
        return;

    ref->PrepareForWrite();

    plVulkanBuffer buffer = ref->GetBuffer();
    if (!buffer.IsValid() || buffer.fSize < size) {
        buffer = CreateBuffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, true,
                              ST::format("vertex buffer {}", idx));
        if (!buffer.IsValid())
            return;
        ref->SetBuffer(this, buffer);
    }

    uint8_t* dst = static_cast<uint8_t*>(buffer.fMapped);

    if (ref->fData) {
        memcpy(dst, ref->fData + vertStart, size);
    } else {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size,
                 "Trailing dead space on non-interleaved data not supported");

        // Cells whose fColorStart is -1 are already interleaved and copy
        // straight across; the rest keep position/normal and color in separate
        // arrays and have to be woven together here.
        const uint32_t  vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t*        srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* srcCPtr = group->GetColorBufferData(idx);

        uint8_t* ptr = dst;
        const size_t numCells = group->GetNumCells(idx);
        for (size_t i = 0; i < numCells; i++) {
            plGBufferCell* cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1)) {
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
            } else {
                uint8_t*        tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;

                for (uint32_t j = 0; j < cell->fLength; j++) {
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

        hsAssert(size_t(ptr - dst) == size, "Didn't fill the buffer?");
    }

    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plVulkanDevice::FillVolatileVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group,
                                                 uint32_t idx)
{
    // Repacks the group's layout into the ref's, dropping the weights and skin
    // indices that the CPU blender has already consumed.
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    const size_t  uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(hsPoint3);
    const uint8_t numWeights = (group->GetVertexFormat() & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < ref->fCount; ++i) {
        inlCopy<hsPoint3>(src, dst); // position

        src += numWeights * sizeof(float);

        if (group->GetVertexFormat() & plGBufferGroup::kSkinIndices)
            inlSkip<uint32_t, 1>(src);

        inlCopy<hsVector3>(src, dst); // normal
        inlCopy<uint32_t>(src, dst);  // diffuse
        inlCopy<uint32_t>(src, dst);  // specular

        memcpy(dst, src, uvChanSize);
        src += uvChanSize;
        dst += uvChanSize;
    }
}

void plVulkanDevice::SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, IndexBufferRef* iRef)
{
    iRef->fCount = owner->GetIndexBufferCount(idx);
    iRef->fOwner = owner;
    iRef->fIndex = idx;
    iRef->fRefTime = 0;

    iRef->SetDirty(true);
    iRef->SetRebuiltSinceUsed(true);

    owner->SetIndexBufferRef(idx, iRef);
    iRef->UnRef();

    iRef->SetVolatile(owner->AreIdxVolatile());
}

void plVulkanDevice::CheckIndexBuffer(IndexBufferRef* iRef)
{
    if (!iRef->GetBuffer().IsValid() && iRef->fCount) {
        iRef->SetVolatile(false);
        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plVulkanDevice::FillIndexBufferRef(IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx)
{
    const uint32_t startIdx = owner->GetIndexBufferStart(idx);
    const uint32_t fullSize = owner->GetIndexBufferCount(idx) * sizeof(uint16_t);
    const uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);

    if (!size)
        return;

    iRef->PrepareForWrite();

    plVulkanBuffer buffer = iRef->GetBuffer();
    if (!buffer.IsValid() || buffer.fSize < fullSize) {
        buffer = CreateBuffer(fullSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, true,
                              ST::format("index buffer {}", idx));
        if (!buffer.IsValid())
            return;
        iRef->SetBuffer(this, buffer);
    }

    // Indices are absolute into the group's vertex buffer, so the destination
    // offset matches the source offset.
    memcpy(static_cast<uint16_t*>(buffer.fMapped) + startIdx,
           owner->GetIndexBufferData(idx) + startIdx, size);

    iRef->SetDirty(false);
}
