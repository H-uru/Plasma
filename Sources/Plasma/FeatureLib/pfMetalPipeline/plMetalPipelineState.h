//
//  plMetalPipelineState.hpp
//  plPipeline
//
//  Created by Colin Cornaby on 3/10/22.
//

#ifndef plMetalPipelineState_hpp
#define plMetalPipelineState_hpp

#include <stdio.h>
#include <Metal/Metal.hpp>

#include "plMetalDevice.h"
#include "plMetalMaterialShaderRef.h"
#include "plSurface/plShaderTable.h"

class plMetalPipelineState {
public:
    plMetalPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef);
    plMetalDevice::plMetalLinkedPipeline* GetRenderPipelineState();
    void PrewarmRenderPipelineState();
    bool operator==(const plMetalPipelineState& p) const {
        if ((&p)->GetID() != this->GetID()) {
            return false;
        } else {
            return p.fNumUVs == fNumUVs && p.fNumWeights == fNumWeights && p.fHasSkinIndices == fHasSkinIndices && IsEqual(p);
        }
    }
    virtual size_t GetHash() const;
    virtual bool IsEqual(const plMetalPipelineState &p) const = 0;
    virtual uint16_t GetID() const { return 0; };
    virtual plMetalPipelineState* Clone() = 0;
    
    //
    virtual const MTL::Function*  GetVertexFunction(MTL::Library* library) = 0;
    virtual const MTL::Function*  GetFragmentFunction(MTL::Library* library) = 0;
    virtual const NS::String*     GetDescription() = 0;
    
    virtual void            ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor) = 0;
    void ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor);
    
    void ConfigureBlendMode(const uint32_t blendMode, MTL::RenderPipelineColorAttachmentDescriptor *descriptor);
protected:
    plMetalDevice* fDevice;
    uint8_t fNumUVs;
    uint8_t fNumWeights;
    bool    fHasSkinIndices;
    virtual void GetFunctionConstants(MTL::FunctionConstantValues*) const;
    MTL::FunctionConstantValues* MakeFunctionConstants() {
        MTL::FunctionConstantValues* constants = MTL::FunctionConstantValues::alloc()->init()->autorelease();
        this->GetFunctionConstants(constants);
        return constants;
    }
};

struct plMetalMaterialPassDescription {
    uint8_t     passTypes[8];
    uint32_t    blendModes[8];
    uint32_t    miscFlags[8];
    uint8_t     numLayers;
    
    bool operator==(const plMetalMaterialPassDescription &p) const {
        bool match = numLayers == p.numLayers && memcmp(passTypes, p.passTypes, sizeof(passTypes)) == 0 && memcmp(blendModes, p.blendModes, sizeof(blendModes)) == 0 && memcmp(miscFlags, p.miscFlags, sizeof(miscFlags)) == 0;
        return match;
    }
    
    size_t GetHash() const {
        std::size_t value = std::hash<uint8_t>()(numLayers);
        
        for(int i=0;i<8;i++){
            value ^= std::hash<uint32_t>()( blendModes[i] );
        }
        
        for(int i=0;i<8;i++){
            value ^= std::hash<uint32_t>()( miscFlags[i] );
        }
        
        for(int i=0;i<8;i++){
            value ^= std::hash<uint8_t>()( passTypes[i] );
        }
        
        return value;
    }
    
    void Populate(plLayerInterface* layPtr, uint8_t index);
};

template<>
struct std::hash<plMetalMaterialPassDescription>
{
    std::size_t operator()(plMetalMaterialPassDescription const& s) const noexcept
    {
        return s.GetHash();
    }
};

class plMetalMaterialPassPipelineState: public plMetalPipelineState {
public:
    plMetalMaterialPassPipelineState(plMetalDevice* device, const plMetalVertexBufferRef *vRef, const plMetalMaterialPassDescription &description);
    virtual size_t GetHash() const override;
    MTL::Function*  GetVertexFunction(MTL::Library* library) override;
    MTL::Function*  GetFragmentFunction(MTL::Library* library) override;
    
    virtual const NS::String*     GetDescription() override;
    
    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) override;
    
    virtual bool IsEqual(const plMetalPipelineState &p) const override;
    
    virtual uint16_t GetID() const override { return 1; };
    
    virtual plMetalPipelineState* Clone() override {
        return new plMetalMaterialPassPipelineState(*this);
    }
    ~plMetalMaterialPassPipelineState();
    virtual void GetFunctionConstants(MTL::FunctionConstantValues*) const override;
protected:
    plMetalMaterialPassDescription fPassDescription;
};

class plMetalRenderShadowCasterPipelineState: public plMetalPipelineState {
public:
    plMetalRenderShadowCasterPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef)
    : plMetalPipelineState(device, vRef) {
        
    }
    const MTL::Function*  GetVertexFunction(MTL::Library* library) override;
    const MTL::Function*  GetFragmentFunction(MTL::Library* library) override;
    
    const NS::String* GetDescription() override {
        return NS::MakeConstantString("Shadow Caster Pipeline");
    };
    
    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) override  {
        descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
    };
    virtual uint16_t GetID() const override { return 2; };
    
    bool IsEqual(const plMetalPipelineState &p) const override {
        //nothing to add at this level
        return true;
    }
    
    
    virtual plMetalPipelineState* Clone() override {
        return new plMetalRenderShadowCasterPipelineState(*this);
    }
    
};

class plMetalRenderShadowPipelineState: public plMetalMaterialPassPipelineState {
public:
    plMetalRenderShadowPipelineState(plMetalDevice* device, plMetalVertexBufferRef *vRef, const plMetalMaterialPassDescription &description)
    : plMetalMaterialPassPipelineState(device, vRef, description) {
    }
    
    const NS::String* GetDescription() override {
        return NS::MakeConstantString("Shadow Span Render Pipeline");
    };
    MTL::Function*  GetFragmentFunction(MTL::Library* library) override;
    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) override;
    virtual uint16_t GetID() const override { return 3; } ;
    
    virtual plMetalPipelineState* Clone() override {
        return new plMetalRenderShadowPipelineState(*this);
    }
};

class plMetalDynamicMaterialPipelineState: public plMetalPipelineState {
public:
    plMetalDynamicMaterialPipelineState(plMetalDevice* device, const plMetalVertexBufferRef *vRef, uint32_t blendMode, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID)
    :   plMetalPipelineState(device, vRef),
        fVertexShaderID(vertexShaderID),
        fFragmentShaderID(fragmentShaderID),
        fBlendMode(blendMode) {
        
    };
    
    virtual plMetalPipelineState* Clone() override {
        return new plMetalDynamicMaterialPipelineState(*this);
    }
    
    bool IsEqual(const plMetalPipelineState &p) const override {
        const plMetalDynamicMaterialPipelineState* dynamicState = static_cast<const plMetalDynamicMaterialPipelineState*>(&p);
        if (!dynamicState) {
            return false;
        }
        return dynamicState->fFragmentShaderID == fFragmentShaderID && dynamicState->fVertexShaderID == fVertexShaderID;
    }
    
    const MTL::Function* GetVertexFunction(MTL::Library *library) override;
    const MTL::Function* GetFragmentFunction(MTL::Library *library) override;
    
    const NS::String *GetDescription() override {
        return NS::MakeConstantString("Dynamic Shader");
    }
    
    void ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) override {
        ConfigureBlendMode(fBlendMode, descriptor);
    }
protected:
    plShaderID::ID  fVertexShaderID;
    plShaderID::ID  fFragmentShaderID;
    uint32_t        fBlendMode;
};

template<>
struct std::hash<plMetalPipelineState>
{
    std::size_t operator()(plMetalPipelineState const& s) const noexcept
    {
        return s.GetHash();
    }
};

#endif /* plMetalPipelineState_hpp */
