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

#ifndef plMetalPipelineState_h
#define plMetalPipelineState_h

#include <stdio.h>

#include <Metal/Metal.hpp>

#include "plMetalDevice.h"
#include "plSurface/plShaderTable.h"

enum plMetalPipelineType
{
    // Unknown is for abstract types, don't use it
    Unknown = 0,
    MaterialShader,
    ShadowCaster,
    ShadowRender,
    Clear,
    Dynamic,
    Text,
    Plate
};

//MARK: Base pipeline state

class plMetalPipelineState
{
public:
    plMetalPipelineState(plMetalDevice* device);
    virtual ~plMetalPipelineState() = default;
    
    plMetalDevice::plMetalLinkedPipeline* GetRenderPipelineState();
    void                                  PrewarmRenderPipelineState();
    bool                                  operator==(const plMetalPipelineState& p) const
    {
        if ((&p)->GetID() != GetID()) {
            return false;
        } else {
            return IsEqual(p);
        }
    }
    virtual size_t                GetHash() const;
    virtual bool                  IsEqual(const plMetalPipelineState& p) const = 0;
    virtual uint16_t              GetID() const { return plMetalPipelineType::Unknown; };
    virtual plMetalPipelineState* Clone() = 0;

    //
    virtual const MTL::Function* GetVertexFunction(MTL::Library* library) = 0;
    virtual const MTL::Function* GetFragmentFunction(MTL::Library* library) = 0;
    virtual const NS::String*    GetDescription() = 0;

    virtual void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) = 0;
    virtual void ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor) = 0;

protected:
    plMetalDevice*               fDevice;
    virtual void                 GetFunctionConstants(MTL::FunctionConstantValues*) const = 0;
    MTL::FunctionConstantValues* MakeFunctionConstants() const
    {
        MTL::FunctionConstantValues* constants = MTL::FunctionConstantValues::alloc()->init()->autorelease();
        GetFunctionConstants(constants);
        return constants;
    }
};

//MARK: Abstract FVF vertex shader program parent type

class plMetalRenderSpanPipelineState : public plMetalPipelineState
{
public:
    plMetalRenderSpanPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef);
    bool IsEqual(const plMetalPipelineState& p) const override
    {
        const plMetalRenderSpanPipelineState* renderSpanPipelineSate = static_cast<const plMetalRenderSpanPipelineState*>(&p);
        if (!renderSpanPipelineSate) {
            return false;
        }
        return renderSpanPipelineSate->fNumUVs == fNumUVs && renderSpanPipelineSate->fNumWeights == fNumWeights && renderSpanPipelineSate->fHasSkinIndices == fHasSkinIndices;
    };
    size_t GetHash() const override;

    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override = 0;
    void ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor) override;

    void ConfigureBlendMode(const uint32_t blendMode, MTL::RenderPipelineColorAttachmentDescriptor* descriptor);

protected:
    uint8_t                      fNumUVs;
    uint8_t                      fNumWeights;
    bool                         fHasSkinIndices;
    void                 GetFunctionConstants(MTL::FunctionConstantValues*) const override;
    MTL::FunctionConstantValues* MakeFunctionConstants()
    {
        MTL::FunctionConstantValues* constants = MTL::FunctionConstantValues::alloc()->init()->autorelease();
        GetFunctionConstants(constants);
        return constants;
    }
};

//MARK: Fixed function emulating material program

struct plMetalFragmentShaderDescription
{
    uint8_t  fPassTypes[8];
    uint32_t fBlendModes[8];
    uint32_t fMiscFlags[8];
    uint8_t  fNumLayers;

    size_t hash;

    bool operator==(const plMetalFragmentShaderDescription& p) const
    {
        bool match = fNumLayers == p.fNumLayers && memcmp(fPassTypes, p.fPassTypes, sizeof(fPassTypes)) == 0 && memcmp(fBlendModes, p.fBlendModes, sizeof(fBlendModes)) == 0 && memcmp(fMiscFlags, p.fMiscFlags, sizeof(fMiscFlags)) == 0;
        return match;
    }

    void CacheHash()
    {
        if (!hash)
            hash = GetHash();
    }

    size_t GetHash() const
    {
        if (hash)
            return hash;

        std::size_t value = std::hash<uint8_t>()(fNumLayers);
        value ^= std::hash<uint8_t>()(fNumLayers);

        for (int i = 0; i < 8; i++) {
            value ^= std::hash<uint32_t>()(fBlendModes[i]);
        }

        for (int i = 0; i < 8; i++) {
            value ^= std::hash<uint32_t>()(fMiscFlags[i]);
        }

        for (int i = 0; i < 8; i++) {
            value ^= std::hash<uint8_t>()(fPassTypes[i]);
        }

        return value;
    }

    void Populate(const plLayerInterface* layPtr, const uint8_t index);
    void PopulateTextureInfo(const plLayerInterface* layPtr, const uint8_t index);
};

template <>
struct std::hash<plMetalFragmentShaderDescription>
{
    size_t operator()(plMetalFragmentShaderDescription const& s) const noexcept
    {
        return s.GetHash();
    }
};

class plMetalMaterialPassPipelineState : public plMetalRenderSpanPipelineState
{
public:
    plMetalMaterialPassPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef, const plMetalFragmentShaderDescription& description);
    size_t GetHash() const override;
    MTL::Function* GetVertexFunction(MTL::Library* library) override;
    MTL::Function* GetFragmentFunction(MTL::Library* library) override;

    const NS::String* GetDescription() override;

    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override;

    bool IsEqual(const plMetalPipelineState& p) const override;

    uint16_t GetID() const override { return plMetalPipelineType::MaterialShader; };

    plMetalPipelineState* Clone() override
    {
        return new plMetalMaterialPassPipelineState(*this);
    }
    ~plMetalMaterialPassPipelineState();
    void GetFunctionConstants(MTL::FunctionConstantValues*) const override;

protected:
    plMetalFragmentShaderDescription fFragmentShaderDescription;
};

//MARK: Shadow casting program

class plMetalRenderShadowCasterPipelineState : public plMetalRenderSpanPipelineState
{
public:
    plMetalRenderShadowCasterPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef)
        : plMetalRenderSpanPipelineState(device, vRef)
    {
    }
    const MTL::Function* GetVertexFunction(MTL::Library* library) override;
    const MTL::Function* GetFragmentFunction(MTL::Library* library) override;

    const NS::String* GetDescription() override
    {
        return MTLSTR("Shadow Caster Pipeline");
    };

    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override
    {
        descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    };
    uint16_t GetID() const override { return plMetalPipelineType::ShadowCaster; };

    plMetalPipelineState* Clone() override
    {
        return new plMetalRenderShadowCasterPipelineState(*this);
    }
};

//MARK: Shadow rendering program

class plMetalRenderShadowPipelineState : public plMetalMaterialPassPipelineState
{
public:
    plMetalRenderShadowPipelineState(plMetalDevice* device, plMetalVertexBufferRef* vRef, const plMetalFragmentShaderDescription& description)
        : plMetalMaterialPassPipelineState(device, vRef, description)
    {
    }

    const NS::String* GetDescription() override
    {
        return MTLSTR("Shadow Span Render Pipeline");
    };
    MTL::Function*   GetVertexFunction(MTL::Library* library) override;
    MTL::Function*   GetFragmentFunction(MTL::Library* library) override;
    void             ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override;
    uint16_t GetID() const override { return plMetalPipelineType::ShadowRender; };

    plMetalPipelineState* Clone() override
    {
        return new plMetalRenderShadowPipelineState(*this);
    }
};

//MARK: Shader based render programs

class plMetalDynamicMaterialPipelineState : public plMetalRenderSpanPipelineState
{
public:
    plMetalDynamicMaterialPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef, uint32_t blendMode, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID)
        : plMetalRenderSpanPipelineState(device, vRef),
          fVertexShaderID(vertexShaderID),
          fFragmentShaderID(fragmentShaderID),
          fBlendMode(blendMode)
    {
    };
    
    uint16_t GetID() const override { return plMetalPipelineType::Dynamic; };

    plMetalPipelineState* Clone() override
    {
        return new plMetalDynamicMaterialPipelineState(*this);
    }

    bool IsEqual(const plMetalPipelineState& p) const override
    {
        const plMetalDynamicMaterialPipelineState* dynamicState = static_cast<const plMetalDynamicMaterialPipelineState*>(&p);
        if (!dynamicState) {
            return false;
        }
        return plMetalRenderSpanPipelineState::IsEqual(p) && dynamicState->fFragmentShaderID == fFragmentShaderID && dynamicState->fVertexShaderID == fVertexShaderID && dynamicState->fBlendMode == fBlendMode;
    }

    size_t GetHash() const override
    {
        std::size_t value = std::hash<plShaderID::ID>()(fFragmentShaderID);
        value ^= std::hash<plShaderID::ID>()(fVertexShaderID);
        value ^= std::hash<plShaderID::ID>()(fVertexShaderID);
        value ^= std::hash<uint32_t>()(fBlendMode);

        return value ^ plMetalRenderSpanPipelineState::GetHash();
    }

    const MTL::Function* GetVertexFunction(MTL::Library* library) override;
    const MTL::Function* GetFragmentFunction(MTL::Library* library) override;

    const NS::String* GetDescription() override
    {
        return MTLSTR("Dynamic Shader");
    }

    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override
    {
        ConfigureBlendMode(fBlendMode, descriptor);
    }

protected:
    plShaderID::ID fVertexShaderID;
    plShaderID::ID fFragmentShaderID;
    uint32_t       fBlendMode;
};

template <>
struct std::hash<plMetalPipelineState>
{
    std::size_t operator()(plMetalPipelineState const& s) const noexcept
    {
        return s.GetHash();
    }
};

//MARK: Clear buffer program

class plMetalClearPipelineState : public plMetalPipelineState
{
public:
    plMetalClearPipelineState(plMetalDevice* device, bool shouldClearColor, bool shouldClearDepth) : plMetalPipelineState(device)
    {
        fShouldClearDepth = shouldClearDepth;
        fShouldClearColor = shouldClearColor;
    }

    bool IsEqual(const plMetalPipelineState& p) const override
    {
        const plMetalClearPipelineState* clearState = static_cast<const plMetalClearPipelineState*>(&p);
        if (!clearState) {
            return false;
        }
        return clearState->fShouldClearDepth == fShouldClearDepth && fShouldClearColor == clearState->fShouldClearColor;
    };

    uint16_t              GetID() const override { return plMetalPipelineType::Clear; };
    plMetalPipelineState* Clone() override
    {
        return new plMetalClearPipelineState(*this);
    };

    const MTL::Function* GetVertexFunction(MTL::Library* library) override
    {
        return library->newFunction(MTLSTR("clearVertex"));
    };
    
    const MTL::Function* GetFragmentFunction(MTL::Library* library) override
    {
        return library->newFunction(MTLSTR("clearFragment"),
                                    MakeFunctionConstants(),
                                    (NS::Error**)nullptr)
            ->autorelease();
    };
    const NS::String* GetDescription() override
    {
        return MTLSTR("Clear");
    };

    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) override
    {
        descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
    };

    void ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor) override
    {
        vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
        vertexDescriptor->attributes()->object(0)->setOffset(0);
        vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
        vertexDescriptor->layouts()->object(0)->setStride(8);
        vertexDescriptor->layouts()->object(0)->setStepFunction(MTL::VertexStepFunctionPerVertex);
        vertexDescriptor->layouts()->object(0)->setStepRate(1);
    };

    void GetFunctionConstants(MTL::FunctionConstantValues* values) const override
    {
        values->setConstantValue(&fShouldClearDepth, MTL::DataTypeBool, NS::UInteger(0));
        values->setConstantValue(&fShouldClearColor, MTL::DataTypeBool, NS::UInteger(1));
    }

    size_t GetHash() const override
    {
        size_t value = plMetalPipelineState::GetHash();
        value ^= std::hash<bool>()(fShouldClearColor);
        value ^= std::hash<bool>()(fShouldClearDepth);

        return value;
    }

private:
    bool fShouldClearColor;
    bool fShouldClearDepth;
};

#endif /* plMetalPipelineState_h */
