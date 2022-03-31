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

#include "plMetalPipelineState.h"
#include "plDrawable/plGBufferGroup.h"
#include "plSurface/plLayerInterface.h"
#include "plSurface/hsGMaterial.h"
#include "plMetalDevice.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plRenderTarget.h"
#include "plMetalDevice.h"

plMetalPipelineState::plMetalPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef)
:   fDevice(device)
{
    fNumUVs = plGBufferGroup::CalcNumUVs(vRef->fFormat);
    fNumWeights = (vRef->fFormat & plGBufferGroup::kSkinWeightMask) >> 4;
    fHasSkinIndices = (vRef->fFormat & plGBufferGroup::kSkinIndices);
}

void plMetalPipelineState::GetFunctionConstants(MTL::FunctionConstantValues* constants) const
{
    ushort numUVs = fNumUVs;
    constants->setConstantValue(&numUVs, MTL::DataTypeUShort, FunctionConstantNumUVs);
    constants->setConstantValue(&fNumWeights, MTL::DataTypeUChar, FunctionConstantNumWeights);
}

size_t plMetalPipelineState::GetHash() const {
    std::size_t h1 = std::hash<uint8_t>()(fNumUVs);
    std::size_t h2 = std::hash<uint8_t>()(fNumWeights);
    std::size_t h3 = std::hash<bool>()(fHasSkinIndices);
    std::size_t h4 = std::hash<uint8_t>()(GetID());

    return h1 ^ h2 ^ h3 ^ h4;
}

plMetalDevice::plMetalLinkedPipeline* plMetalPipelineState::GetRenderPipelineState() {
    return fDevice->PipelineState(this);
}

void plMetalPipelineState::PrewarmRenderPipelineState() {
    fDevice->PrewarmPipelineStateFor(this);
}


plMetalMaterialPassPipelineState::plMetalMaterialPassPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef, const plMetalMaterialPassDescription &description)
:   plMetalPipelineState(device, vRef) {
    fPassDescription = description;
}

void plMetalMaterialPassPipelineState::GetFunctionConstants(MTL::FunctionConstantValues* constants) const
{
    plMetalPipelineState::GetFunctionConstants(constants);
    constants->setConstantValue(&fPassDescription.numLayers, MTL::DataTypeUChar, FunctionConstantNumLayers);
    constants->setConstantValues(&fPassDescription.passTypes, MTL::DataTypeUChar, NS::Range(FunctionConstantSources, 8));
    constants->setConstantValues(&fPassDescription.blendModes, MTL::DataTypeUInt, NS::Range(FunctionConstantBlendModes, 8));
    constants->setConstantValues(&fPassDescription.miscFlags, MTL::DataTypeUInt, NS::Range(FunctionConstantLayerFlags, 8));
}

size_t plMetalMaterialPassPipelineState::GetHash() const {
    std::size_t value = plMetalPipelineState::GetHash();
    value ^= fPassDescription.GetHash();

    return value;
}

void plMetalPipelineState::ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor) {
    int vertOffset = 0;
    int skinWeightOffset = vertOffset + (sizeof(float) * 3);
    if(this->fHasSkinIndices) {
        skinWeightOffset += sizeof(uint32_t);
    }
    int normOffset = skinWeightOffset + (sizeof(float) * this->fNumWeights);
    int colorOffset = normOffset + (sizeof(float) * 3);
    int baseUvOffset = colorOffset + (sizeof(uint32_t) * 2);
    int stride = baseUvOffset + (sizeof(float) * 3 * this->fNumUVs);
    
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setOffset(vertOffset);
    
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setOffset(normOffset);
    
    if(this->fNumWeights > 0) {
        int weightOneOffset = skinWeightOffset;
        
        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setFormat(MTL::VertexFormatFloat);
        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setBufferIndex(0);
        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setOffset(weightOneOffset);
    }
    
    for(int i=0; i<this->fNumUVs; i++) {
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setFormat(MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setBufferIndex(0);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord+i)->setOffset(baseUvOffset + (i * sizeof(float) * 3));
    }
    
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setFormat(MTL::VertexFormatUChar4);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setOffset(colorOffset);
    
    vertexDescriptor->layouts()->object(VertexAttributePosition)->setStride(stride);
}

void plMetalPipelineState::ConfigureBlendMode(const uint32_t blendMode, MTL::RenderPipelineColorAttachmentDescriptor *descriptor)
{
    if (blendMode & hsGMatState::kBlendNoColor) {
        //printf("glBlendFunc(GL_ZERO, GL_ONE);\n");
        descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
        descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
        descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
        return;
    }
    switch (blendMode & hsGMatState::kBlendMask)
    {
        // Detail is just a special case of alpha, handled in construction of the texture
        // mip chain by making higher levels of the chain more transparent.
        case hsGMatState::kBlendDetail:
        case hsGMatState::kBlendAlpha:
            if (blendMode & hsGMatState::kBlendInvertFinalAlpha) {
                if (blendMode & hsGMatState::kBlendAlphaPremultiplied) {
                    //printf("glBlendFunc(GL_ONE, GL_SRC_ALPHA);\n");
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                    descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
                    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                    descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                } else {
                    //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);\n");
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                    descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                    descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                }
            } else {
                if (blendMode & hsGMatState::kBlendAlphaPremultiplied) {
                    //printf("glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);\n");
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                } else {
                    //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);\n");
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                }
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            }
            break;

        // Multiply the final color onto the frame buffer.
        case hsGMatState::kBlendMult:
            if (blendMode & hsGMatState::kBlendInvertFinalColor) {
                //printf("glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);\n");
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceColor);
            } else {
                //printf("glBlendFunc(GL_ZERO, GL_SRC_COLOR);\n");
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceColor);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceColor);
            }
            break;

        // Add final color to FB.
        case hsGMatState::kBlendAdd:
            //printf("glBlendFunc(GL_ONE, GL_ONE);\n");
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            break;

        // Multiply final color by FB color and add it into the FB.
        case hsGMatState::kBlendMADD:
            //printf("glBlendFunc(GL_DST_COLOR, GL_ONE);\n");
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorDestinationColor);
            descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorDestinationColor);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            break;

        // Final color times final alpha, added into the FB.
        case hsGMatState::kBlendAddColorTimesAlpha:
            if (blendMode & hsGMatState::kBlendInvertFinalAlpha) {
                //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);\n");
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            } else {
                //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE);\n");
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            }
            break;

        // Overwrite final color onto FB
        case 0:
            //printf("glBlendFunc(GL_ONE, GL_ZERO);\n");
            descriptor->setRgbBlendOperation(MTL::BlendOperationAdd);
            //printf("glBlendFunc(GL_ONE, GL_ZERO);\n");
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
            descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
            
            /*descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
            descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);*/
            break;

        default:
            {
                /*hsAssert(false, "Too many blend modes specified in material");
                plLayer* lay = plLayer::ConvertNoRef(fCurrMaterial->GetLayer(fCurrLayerIdx)->BottomOfStack());
                if( lay )
                {
                    if( lay->GetBlendFlags() & hsGMatState::kBlendAlpha )
                    {
                        lay->SetBlendFlags((lay->GetBlendFlags() & ~hsGMatState::kBlendMask) | hsGMatState::kBlendAlpha);
                    }
                    else
                    {
                        lay->SetBlendFlags((lay->GetBlendFlags() & ~hsGMatState::kBlendMask) | hsGMatState::kBlendAdd);
                    }
                }*/
            }
            break;
    }
}

MTL::Function* plMetalMaterialPassPipelineState::GetVertexFunction(MTL::Library* library) {
    NS::Error* error = nullptr;
    MTL::FunctionConstantValues* constants = MTL::FunctionConstantValues::alloc()->init()->autorelease();
    this->GetFunctionConstants(constants);
    MTL::Function* function = library->newFunction(
                                NS::String::string("pipelineVertexShader", NS::ASCIIStringEncoding),
                                    MakeFunctionConstants(),
                                &error
                                )->autorelease();
    return function;
}

MTL::Function* plMetalMaterialPassPipelineState::GetFragmentFunction(MTL::Library* library) {
    return library->newFunction(
                                NS::String::string("pipelineFragmentShader", NS::ASCIIStringEncoding),
                                    MakeFunctionConstants(),
                                (NS::Error **)NULL
                             )->autorelease();
}

plMetalMaterialPassPipelineState::~plMetalMaterialPassPipelineState() {
}

const NS::String* plMetalMaterialPassPipelineState::GetDescription() {
    return NS::MakeConstantString("Material Pipeline");
}

void plMetalMaterialPassPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) {
    uint32_t blendMode = fPassDescription.blendModes[0];
    ConfigureBlendMode(blendMode, descriptor);
}

void plMetalMaterialPassDescription::Populate(plLayerInterface* layPtr, uint8_t index) {
    if (layPtr == nullptr) {
        blendModes[index] = 0;
        miscFlags[index] = 0;
        passTypes[index] = 0;
    }
    
    blendModes[index] = layPtr->GetBlendFlags();
    miscFlags[index] = layPtr->GetMiscFlags();
    
    plBitmap* texture = layPtr->GetTexture();
    if (texture != nullptr) {
        plMetalTextureRef* texRef = (plMetalTextureRef*)texture->GetDeviceRef();
        if(texRef->fTexture) {
        
        plMetalTextureRef *deviceTexture = (plMetalTextureRef *)texture->GetDeviceRef();
        if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
            passTypes[index] = PassTypeCubicTexture;
        } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
            passTypes[index] = PassTypeTexture;
        } else {
            passTypes[index] = PassTypeColor;
        }
        }
        
    } else {
        passTypes[index] = PassTypeColor;
    }
    
}

bool plMetalMaterialPassPipelineState::IsEqual(const plMetalPipelineState &p) const {
    return static_cast<const plMetalMaterialPassPipelineState*>(&p)->fPassDescription == this->fPassDescription;
}


MTL::Function* plMetalRenderShadowPipelineState::GetFragmentFunction(MTL::Library* library) {
    return library->newFunction(
                                NS::String::string("shadowCastFragmentShader", NS::ASCIIStringEncoding),
                                MakeFunctionConstants(),
                                (NS::Error **)NULL
                             )->autorelease();
}

void plMetalRenderShadowPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor *descriptor) {
    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
    descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
    descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
}

const MTL::Function* plMetalRenderShadowCasterPipelineState::GetVertexFunction(MTL::Library* library)
{
    NS::Error* error = nullptr;
    MTL::Function* function = library->newFunction(
                                NS::String::string("shadowVertexShader", NS::ASCIIStringEncoding),
                                    MakeFunctionConstants(),
                                &error
                                )->autorelease();
    return function;
}

const MTL::Function* plMetalRenderShadowCasterPipelineState::GetFragmentFunction(MTL::Library* library)
{
    NS::Error* error = nullptr;
    MTL::Function* function = library->newFunction(
                                NS::String::string("shadowFragmentShader", NS::ASCIIStringEncoding),
                                    MakeFunctionConstants(),
                                &error
                                )->autorelease();
    return function;
}

const MTL::Function* plMetalDynamicMaterialPipelineState::GetVertexFunction(MTL::Library *library) {
    MTL::FunctionConstantValues* functionConstants = MakeFunctionConstants();
    MTL::Function* vertFunction;
    switch(fVertexShaderID) {
        case plShaderID::vs_WaveFixedFin7:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_WaveFixedFin7", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_CompCosines:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_CompCosines", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_BiasNormals:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_BiasNormals", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_GrassShader:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_GrassShader", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_WaveDecEnv_7:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_WaveDecEnv_7", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_WaveDec1Lay_7:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_WaveDec1Lay_7", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::vs_WaveRip7:
            vertFunction = library->newFunction(
                                                               NS::String::string("vs_WaveRip7", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        default:
            hsAssert(0, "unknown shader requested");
    }
    return vertFunction;
}

const MTL::Function* plMetalDynamicMaterialPipelineState::GetFragmentFunction(MTL::Library *library) {
    MTL::FunctionConstantValues* functionConstants = MakeFunctionConstants();
    MTL::Function* fragFunction;
    switch(fFragmentShaderID) {
        case plShaderID::ps_WaveFixed:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_WaveFixed", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_MoreCosines:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_CompCosines", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_BiasNormals:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_BiasNormals", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_GrassShader:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_GrassShader", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_WaveDecEnv:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_WaveDecEnv", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_CbaseAbase:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_CbaseAbase", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        case plShaderID::ps_WaveRip:
            fragFunction = library->newFunction(
                                                               NS::String::string("ps_WaveRip", NS::ASCIIStringEncoding),
                                                functionConstants,
                                                               (NS::Error **)nullptr
                                                            );
            break;
        default:
            hsAssert(0, "unknown shader requested");
    }
    return fragFunction;
}
