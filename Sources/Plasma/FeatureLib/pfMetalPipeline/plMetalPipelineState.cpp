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
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plMetalDevice.h"
#include "plMetalMaterialShaderRef.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plRenderTarget.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

size_t plMetalPipelineState::GetHash() const
{
    return std::hash<uint8_t>()(GetID());
}

plMetalPipelineState::plMetalPipelineState(plMetalDevice* device)
    : fDevice(device)
{
}

plMetalRenderSpanPipelineState::plMetalRenderSpanPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef)
    : plMetalPipelineState(device)
{
    fNumUVs = plGBufferGroup::CalcNumUVs(vRef->fFormat);
    fNumWeights = (vRef->fFormat & plGBufferGroup::kSkinWeightMask) >> 4;
    fHasSkinIndices = (vRef->fFormat & plGBufferGroup::kSkinIndices);
}

void plMetalRenderSpanPipelineState::GetFunctionConstants(MTL::FunctionConstantValues* constants) const
{
    ushort numUVs = fNumUVs;
    constants->setConstantValue(&numUVs, MTL::DataTypeUShort, FunctionConstantNumUVs);
    constants->setConstantValue(&fNumWeights, MTL::DataTypeUChar, FunctionConstantNumWeights);
}

size_t plMetalRenderSpanPipelineState::GetHash() const
{
    size_t h1 = std::hash<uint8_t>()(fNumUVs);
    size_t h2 = std::hash<uint8_t>()(fNumWeights);
    size_t h3 = std::hash<bool>()(fHasSkinIndices);

    return h1 ^ h2 ^ h3 ^ plMetalPipelineState::GetHash();
}

plMetalDevice::plMetalLinkedPipeline* plMetalPipelineState::GetRenderPipelineState()
{
    return fDevice->PipelineState(this);
}

void plMetalPipelineState::PrewarmRenderPipelineState()
{
    fDevice->PrewarmPipelineStateFor(this);
}

plMetalMaterialPassPipelineState::plMetalMaterialPassPipelineState(plMetalDevice* device, const plMetalVertexBufferRef* vRef, const plMetalFragmentShaderDescription& description)
    : plMetalRenderSpanPipelineState(device, vRef)
{
    fFragmentShaderDescription = description;
    fFragmentShaderDescription.CacheHash();
}

void plMetalMaterialPassPipelineState::GetFunctionConstants(MTL::FunctionConstantValues* constants) const
{
    plMetalRenderSpanPipelineState::GetFunctionConstants(constants);
    constants->setConstantValue(&fFragmentShaderDescription.fNumLayers, MTL::DataTypeUChar, FunctionConstantNumLayers);
    constants->setConstantValues(&fFragmentShaderDescription.fPassTypes, MTL::DataTypeUChar, NS::Range(FunctionConstantSources, 8));
    constants->setConstantValues(&fFragmentShaderDescription.fBlendModes, MTL::DataTypeUInt, NS::Range(FunctionConstantBlendModes, 8));
    constants->setConstantValues(&fFragmentShaderDescription.fMiscFlags, MTL::DataTypeUInt, NS::Range(FunctionConstantLayerFlags, 8));
}

size_t plMetalMaterialPassPipelineState::GetHash() const
{
    size_t value = plMetalRenderSpanPipelineState::GetHash();
    value ^= fFragmentShaderDescription.GetHash();

    return value;
}

void plMetalRenderSpanPipelineState::ConfigureVertexDescriptor(MTL::VertexDescriptor* vertexDescriptor)
{
    int vertOffset = 0;
    int skinWeightOffset = vertOffset + sizeof(hsPoint3);
    if (fHasSkinIndices) {
        skinWeightOffset += sizeof(uint32_t);
    }
    int normOffset = skinWeightOffset + (sizeof(float) * fNumWeights);
    int colorOffset = normOffset + sizeof(hsPoint3);
    int baseUvOffset = colorOffset + (sizeof(uint32_t) * 2);
    int stride = baseUvOffset + sizeof(hsPoint3) * fNumUVs;

    vertexDescriptor->attributes()->object(VertexAttributePosition)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributePosition)->setOffset(vertOffset);

    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setFormat(MTL::VertexFormatFloat3);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeNormal)->setOffset(normOffset);

    if (fNumWeights > 0) {
        int weightOneOffset = skinWeightOffset;

        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setFormat(MTL::VertexFormatFloat);
        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setBufferIndex(0);
        vertexDescriptor->attributes()->object(VertexAttributeWeights)->setOffset(weightOneOffset);
    }

    for (int i = 0; i < fNumUVs; i++) {
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord + i)->setFormat(MTL::VertexFormatFloat3);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord + i)->setBufferIndex(0);
        vertexDescriptor->attributes()->object(VertexAttributeTexcoord + i)->setOffset(baseUvOffset + (i * sizeof(hsPoint3)));
    }

    vertexDescriptor->attributes()->object(VertexAttributeColor)->setFormat(MTL::VertexFormatUChar4);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setBufferIndex(0);
    vertexDescriptor->attributes()->object(VertexAttributeColor)->setOffset(colorOffset);

    vertexDescriptor->layouts()->object(VertexAttributePosition)->setStride(stride);
}

void plMetalRenderSpanPipelineState::ConfigureBlendMode(const uint32_t blendMode, MTL::RenderPipelineColorAttachmentDescriptor* descriptor)
{
    if (blendMode & hsGMatState::kBlendNoColor) {
        // printf("glBlendFunc(GL_ZERO, GL_ONE);\n");
        descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
        descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
        descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
        descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
        return;
    }
    switch (blendMode & hsGMatState::kBlendMask) {
        // Detail is just a special case of alpha, handled in construction of the texture
        // mip chain by making higher levels of the chain more transparent.
        case hsGMatState::kBlendDetail:
        case hsGMatState::kBlendAlpha:
            if (blendMode & hsGMatState::kBlendInvertFinalAlpha) {
                if (blendMode & hsGMatState::kBlendAlphaPremultiplied) {
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                } else {
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                }
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            } else {
                if (blendMode & hsGMatState::kBlendAlphaPremultiplied) {
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
                } else {
                    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                }
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            }
            break;

        // Multiply the final color onto the frame buffer.
        case hsGMatState::kBlendMult:
            if (blendMode & hsGMatState::kBlendInvertFinalColor) {
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOneMinusSourceColor);
            } else {
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorSourceColor);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorSourceColor);
            }
            break;

        // Add final color to FB.
        case hsGMatState::kBlendAdd:
            descriptor->setRgbBlendOperation(MTL::BlendOperationAdd);
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
            break;

        // Multiply final color by FB color and add it into the FB.
        case hsGMatState::kBlendMADD:
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorDestinationColor);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
            break;

        // Final color times final alpha, added into the FB.
        case hsGMatState::kBlendAddColorTimesAlpha:
            if (blendMode & hsGMatState::kBlendInvertFinalAlpha) {
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            } else {
                descriptor->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
                descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorSourceAlpha);
                descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOne);
                descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            }
            break;

        // Overwrite final color onto FB
        case 0:
            descriptor->setRgbBlendOperation(MTL::BlendOperationAdd);
            descriptor->setAlphaBlendOperation(MTL::BlendOperationAdd);
            descriptor->setSourceRGBBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorZero);
            descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
            descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
            break;

        default: {
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
        } break;
    }
}

MTL::Function* plMetalMaterialPassPipelineState::GetVertexFunction(MTL::Library* library)
{
    NS::Error*                   error = nullptr;
    MTL::FunctionConstantValues* constants = MTL::FunctionConstantValues::alloc()->init()->autorelease();
    GetFunctionConstants(constants);
    MTL::Function* function = library->newFunction(
                                         NS::String::string("pipelineVertexShader", NS::ASCIIStringEncoding),
                                         MakeFunctionConstants(),
                                         &error)
                                  ->autorelease();
    return function;
}

MTL::Function* plMetalMaterialPassPipelineState::GetFragmentFunction(MTL::Library* library)
{
    return library->newFunction(
                      NS::String::string("pipelineFragmentShader", NS::ASCIIStringEncoding),
                      MakeFunctionConstants(),
                      (NS::Error**)nullptr)
        ->autorelease();
}

plMetalMaterialPassPipelineState::~plMetalMaterialPassPipelineState()
{
}

const NS::String* plMetalMaterialPassPipelineState::GetDescription()
{
    return MTLSTR("Material Pipeline");
}

void plMetalMaterialPassPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor)
{
    uint32_t blendMode = fFragmentShaderDescription.fBlendModes[0];
    ConfigureBlendMode(blendMode, descriptor);
}

void plMetalFragmentShaderDescription::Populate(const plLayerInterface* layPtr, const uint8_t index)
{
    fBlendModes[index] = layPtr->GetBlendFlags();
    fMiscFlags[index] = layPtr->GetMiscFlags();
    PopulateTextureInfo(layPtr, index);
}

void plMetalFragmentShaderDescription::PopulateTextureInfo(const plLayerInterface* layPtr, const uint8_t index)
{
    plBitmap* texture = layPtr->GetTexture();
    if (texture != nullptr) {
        if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
            fPassTypes[index] = PassTypeCubicTexture;
        } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
            fPassTypes[index] = PassTypeTexture;
        } else {
            fPassTypes[index] = PassTypeColor;
        }

    } else {
        fPassTypes[index] = PassTypeColor;
    }
}

bool plMetalMaterialPassPipelineState::IsEqual(const plMetalPipelineState& p) const
{
    return plMetalRenderSpanPipelineState::IsEqual(p) && static_cast<const plMetalMaterialPassPipelineState*>(&p)->fFragmentShaderDescription == fFragmentShaderDescription;
}

MTL::Function* plMetalRenderShadowPipelineState::GetVertexFunction(MTL::Library* library)
{
    return library->newFunction(
                      NS::String::string("shadowCastVertexShader", NS::ASCIIStringEncoding),
                      MakeFunctionConstants(),
                      (NS::Error**)nullptr)
        ->autorelease();
}

MTL::Function* plMetalRenderShadowPipelineState::GetFragmentFunction(MTL::Library* library)
{
    return library->newFunction(
                      NS::String::string("shadowCastFragmentShader", NS::ASCIIStringEncoding),
                      MakeFunctionConstants(),
                      (NS::Error**)nullptr)
        ->autorelease();
}

void plMetalRenderShadowPipelineState::ConfigureBlend(MTL::RenderPipelineColorAttachmentDescriptor* descriptor)
{
    descriptor->setSourceRGBBlendFactor(MTL::BlendFactorZero);
    descriptor->setSourceAlphaBlendFactor(MTL::BlendFactorOne);
    descriptor->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceColor);
    descriptor->setDestinationAlphaBlendFactor(MTL::BlendFactorZero);
}

const MTL::Function* plMetalRenderShadowCasterPipelineState::GetVertexFunction(MTL::Library* library)
{
    NS::Error*     error = nullptr;
    MTL::Function* function = library->newFunction(
                                         MTLSTR("shadowVertexShader"),
                                         MakeFunctionConstants(),
                                         &error)
                                  ->autorelease();
    return function;
}

const MTL::Function* plMetalRenderShadowCasterPipelineState::GetFragmentFunction(MTL::Library* library)
{
    NS::Error*     error = nullptr;
    MTL::Function* function = library->newFunction(
                                         MTLSTR("shadowFragmentShader"),
                                         MakeFunctionConstants(),
                                         &error)
                                  ->autorelease();
    return function;
}

const MTL::Function* plMetalDynamicMaterialPipelineState::GetVertexFunction(MTL::Library* library)
{
    MTL::FunctionConstantValues* functionConstants = MakeFunctionConstants();
    MTL::Function*               vertFunction;
    // map the original engine vertex shader id to the pixel shader function
    switch (fVertexShaderID) {
        case plShaderID::vs_WaveFixedFin7:
            vertFunction = library->newFunction(
                NS::String::string("vs_WaveFixedFin7", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_CompCosines:
            vertFunction = library->newFunction(
                NS::String::string("vs_CompCosines", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_BiasNormals:
            vertFunction = library->newFunction(
                NS::String::string("vs_BiasNormals", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_GrassShader:
            vertFunction = library->newFunction(
                NS::String::string("vs_GrassShader", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_WaveDecEnv_7:
            vertFunction = library->newFunction(
                NS::String::string("vs_WaveDecEnv_7", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_WaveDec1Lay_7:
            vertFunction = library->newFunction(
                NS::String::string("vs_WaveDec1Lay_7", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::vs_WaveRip7:
            vertFunction = library->newFunction(
                NS::String::string("vs_WaveRip7", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        default:
            hsAssert(0, "unknown shader requested");
    }
    return vertFunction;
}

const MTL::Function* plMetalDynamicMaterialPipelineState::GetFragmentFunction(MTL::Library* library)
{
    MTL::FunctionConstantValues* functionConstants = MakeFunctionConstants();
    MTL::Function*               fragFunction;
    // map the original engine pixel shader id to the pixel shader function
    switch (fFragmentShaderID) {
        case plShaderID::ps_WaveFixed:
            fragFunction = library->newFunction(
                NS::String::string("ps_WaveFixed", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_MoreCosines:
            fragFunction = library->newFunction(
                NS::String::string("ps_CompCosines", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_BiasNormals:
            fragFunction = library->newFunction(
                NS::String::string("ps_BiasNormals", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_GrassShader:
            fragFunction = library->newFunction(
                NS::String::string("ps_GrassShader", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_WaveDecEnv:
            fragFunction = library->newFunction(
                NS::String::string("ps_WaveDecEnv", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_CbaseAbase:
            fragFunction = library->newFunction(
                NS::String::string("ps_CbaseAbase", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        case plShaderID::ps_WaveRip:
            fragFunction = library->newFunction(
                NS::String::string("ps_WaveRip", NS::ASCIIStringEncoding),
                functionConstants,
                (NS::Error**)nullptr);
            break;
        default:
            hsAssert(0, "unknown shader requested");
    }
    return fragFunction;
}
