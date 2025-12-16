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

#include "plMetalArgumentBuffer.h"

NS::Array* plMetalBumpArgumentBuffer::GetArgumentDescriptors()
{
    MTL::ArgumentDescriptor* descriptors[3];
    descriptors[0] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[0]->setIndex(dTangentIndexID);
    descriptors[0]->setDataType(MTL::DataTypeChar2);

    descriptors[1] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[1]->setIndex(textureID);
    descriptors[1]->setDataType(MTL::DataTypeTexture);

    descriptors[2] = MTL::ArgumentDescriptor::argumentDescriptor();
    descriptors[2]->setIndex(dScaleID);
    descriptors[2]->setDataType(MTL::DataTypeFloat);

    NS::Array* array = NS::Array::array((const NS::Object* const*)descriptors, 3);
    return array;
}

plMetalBumpArgumentBuffer::plMetalBumpArgumentBuffer(MTL::Device* device, size_t numElements) : plMetalArgumentBuffer<plMetalBumpmap>(device, numElements)
{
    _bumps.resize(numElements);
}

void plMetalBumpArgumentBuffer::Set(const std::vector<plMetalBumpMapping>& bumps)
{
    if (CheckBuffer(bumps)) {
        return;
    }
    ConfigureBuffer();
    int i = 0;
    if (_tier == plMetalArgumentBufferTier::Tier1) {
        for (const auto& bump : bumps) {
            _encoder->setArgumentBuffer(GetBuffer(), 0, i);
            _encoder->setTexture(bump.texture, textureID);
            uint8_t* cotangentUBuffer = static_cast<uint8_t*>(_encoder->constantData(dTangentIndexID));
            memcpy(cotangentUBuffer, &bump.dTangentUIndex, sizeof(uint8_t) * 2);
            float* scaleBuffer = static_cast<float*>(_encoder->constantData(dScaleID));
            memcpy(scaleBuffer, &bump.scale, sizeof(float));

            // FIXME: We can't track this with a tier 1 buffer
            _bumps[i] = bump;
            i++;
        }
    }
#ifdef METAL_3_SDK
    else {
        for (const auto& bump : bumps) {
            _value[i].bumpTexture = bump.texture->gpuResourceID();
            _value[i].dTangentIndex = simd::make_char2(bump.dTangentUIndex, bump.dTangentVIndex);
            _value[i].scale = bump.scale;

            _bumps[i] = bump;
            i++;
        }
    }
#endif
    if (GetBuffer()->storageMode() == MTL::StorageModeManaged) {
        GetBuffer()->didModifyRange(NS::Range(0, GetBuffer()->length()));
    }
}

void plMetalBumpArgumentBuffer::Bind(MTL::RenderCommandEncoder* encoder)
{
    for (const auto& bump : _bumps) {
        // These textures can't go into a heap because they're shared by multiple
        // materials, boo. Mark them as needing to be resident one at a time.
        encoder->useResource(bump.texture, MTL::ResourceUsageRead, MTL::RenderStageFragment);
    }
    encoder->setVertexBuffer(GetBuffer(), 0, BumpState);
    encoder->setFragmentBuffer(GetBuffer(), 0, BumpState);
}

bool plMetalBumpArgumentBuffer::CheckBuffer(const std::vector<plMetalBumpMapping>& bumps)
{
    if (bumps.size() != _numElements || GetBuffer() == nullptr) {
        return false;
    }
    int i = 0;
    for (auto& bump : bumps) {
        if (bump.texture != _bumps[i].texture)
            return false;
        if (bump.dTangentUIndex != _bumps[i].dTangentUIndex ||
            bump.dTangentVIndex != _bumps[i].dTangentVIndex) {
            return false;
        }
        if (bump.scale != _bumps[i].scale) {
            return false;
        }
        i++;
    }
    return true;
}
