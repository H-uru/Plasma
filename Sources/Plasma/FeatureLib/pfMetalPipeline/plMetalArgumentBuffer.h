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

#ifndef plMetalArgumentBuffer_hpp
#define plMetalArgumentBuffer_hpp

#include <stdio.h>
#include <Metal/Metal.hpp>

#include "ShaderTypes.h"

// Define our own enum so our code doesn't do weird things
// if tier 3 shows up.
enum class plMetalArgumentBufferTier : NS::UInteger
{
    Tier1 = MTL::ArgumentBuffersTier1,
#ifdef METAL_3_SDK
    Tier2 = MTL::ArgumentBuffersTier2,
#endif
};

template <class T>
class plMetalArgumentBuffer
{
public:
    plMetalArgumentBuffer(MTL::Device* device, size_t numElements) : _device(device), _numElements(numElements), _encoder(nullptr)
    {
        _currentBufferIndex = -1;
        auto tier = std::max(device->argumentBuffersSupport(), MTL::ArgumentBuffersTier2);
        _tier = plMetalArgumentBufferTier(tier);
        _buffer[0] = nullptr;
        _buffer[1] = nullptr;
        _buffer[2] = nullptr;
    };
    constexpr MTL::Buffer* GetBuffer()
    {
        if (_currentBufferIndex == -1) {
            return nullptr;
        }
        return _buffer[_currentBufferIndex];
    }
    T*     ValueAt(size_t i) { return _value[i]; }
    size_t GetNumElements() { return _numElements; }
    ~plMetalArgumentBuffer()
    {
        _buffer[0]->release();
        _buffer[1]->release();
        _buffer[2]->release();
        _encoder->release();
    }

protected:
    T*                        _value;
    MTL::Buffer*              _buffer[3];
    MTL::Device*              _device;
    // Encoder for tier 1 buffers, don't use in tier 2
    MTL::ArgumentEncoder*     _encoder;
    size_t                    _numElements;
    plMetalArgumentBufferTier _tier;
    size_t                    _currentBufferIndex;
    virtual NS::Array*        GetArgumentDescriptors() = 0;

    void ConfigureBuffer()
    {
        _currentBufferIndex = (_currentBufferIndex + 1) % 3;
        switch (_tier) {
            case plMetalArgumentBufferTier::Tier1:
                ConfigureTier1();
                break;
#ifdef METAL_3_SDK
            case plMetalArgumentBufferTier::Tier2:
                ConfigureTier2();
                break;
#endif
        }
    }

private:
    void ConfigureTier1()
    {
        if (!_encoder)
            _encoder = _device->newArgumentEncoder(GetArgumentDescriptors());
        if (!_buffer[_currentBufferIndex])
            _buffer[_currentBufferIndex] = _device->newBuffer(_encoder->encodedLength() * _numElements, MTL::CPUCacheModeWriteCombined);
        // raw access to buffer not available in tier 1 argument buffers
        _value = nullptr;
    }
#ifdef METAL_3_SDK
    void ConfigureTier2()
    {
        if (!_buffer[_currentBufferIndex])
            _buffer[_currentBufferIndex] = _device->newBuffer(sizeof(T) * _numElements, MTL::CPUCacheModeWriteCombined);
        _value = static_cast<T*>(_buffer[_currentBufferIndex]->contents());
        // Encoders are not used in tier 2 argument buffers
        _encoder = nullptr;
    }
#endif
};

struct plMetalBumpMapping
{
    uint8_t             dTangentUIndex;
    uint8_t             dTangentVIndex;
    float               scale;
    MTL::Texture*       texture;
    MTL::SamplerState*  sampler;
};

class plMetalBumpArgumentBuffer final : public plMetalArgumentBuffer<plMetalBumpmap>
{
public:
    void Set(const std::vector<plMetalBumpMapping>& bumps);
    plMetalBumpArgumentBuffer(MTL::Device* device, size_t numElements);
    void Bind(MTL::RenderCommandEncoder* encoder);
    bool CheckBuffer(const std::vector<plMetalBumpMapping>& bumps);

protected:
    virtual NS::Array* GetArgumentDescriptors() override;

private:
    std::vector<plMetalBumpMapping> _bumps;
};

#endif /* plMetalArgumentBuffer_hpp */
