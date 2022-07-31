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

#include <string_theory/format>
#include "plMetalMaterialShaderRef.h"

#include "HeadSpin.h"
#include "hsBitVector.h"

#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plPipeline.h"
#include "plPipeDebugFlags.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plRenderTarget.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include "hsGMatState.inl"

#include "plMetalDevice.h"
#include "plMetalPipeline.h"

plMetalMaterialShaderRef::plMetalMaterialShaderRef(hsGMaterial* mat, plMetalPipeline *pipe) :
fPipeline { pipe },
fMaterial { mat },
fFragFunction(),
fNumPasses(0)
{
    fDevice = pipe->fDevice.fMetalDevice;
    fFragFunction = pipe->fFragFunction;
    CheckMateralRef();
}

plMetalMaterialShaderRef::~plMetalMaterialShaderRef()
{
    Release();
}

void plMetalMaterialShaderRef::Release()
{
    for(auto & buffer : fPassArgumentBuffers) {
        buffer->release();
        buffer = nil;
    }
    fPassArgumentBuffers.clear();
    
    fNumPasses = 0;
}

void plMetalMaterialShaderRef::CheckMateralRef()
{
    if(fNumPasses == 0) {
        ILoopOverLayers();
        
        for (size_t i = 0; i < fMaterial->GetNumLayers(); i++) {
            plLayerInterface* layer = fMaterial->GetLayer(i);
            if (!layer) {
                continue;
            }

            fPipeline->CheckTextureRef(layer);
        }
    }
}

//fast encode doesn't support piggybacks or push over layers, but it does use preloaded data on the GPU so it's much faster. Use this encoder if there are no piggybacks or pushover layers
void plMetalMaterialShaderRef::FastEncodeArguments(MTL::RenderCommandEncoder *encoder, VertexUniforms *vertexUniforms, uint pass)
{
    size_t i = 0;
    for (i = GetPassIndex(pass); i < GetPassIndex(pass) + fPassLengths[pass]; i++) {
        plLayerInterface* layer = fMaterial->GetLayer(i);
        
        if (!layer) {
            continue;
        }

        fPipeline->CheckTextureRef(layer);
        
        plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());

        if (!img) {
            continue;
        }
        
        plMetalTextureRef* texRef = (plMetalTextureRef*)img->GetDeviceRef();

        //if (!texRef->fTexture) {
          //  continue;
        //}
        
        assert(i - GetPassIndex(pass) >= 0);
        EncodeTransform(layer, &vertexUniforms->uvTransforms[i - GetPassIndex(pass)]);
        IBuildLayerTexture(encoder, i - GetPassIndex(pass), layer);
    }
    
    encoder->setFragmentBuffer(fPassArgumentBuffers[pass], 0, BufferIndexFragArgBuffer);
}

void plMetalMaterialShaderRef::EncodeArguments(MTL::RenderCommandEncoder *encoder, VertexUniforms *vertexUniforms, uint pass, plMetalFragmentShaderDescription* passDescription, std::vector<plLayerInterface*> *piggyBacks, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> preEncodeTransform, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> postEncodeTransform)
{
    
    std::vector<plLayerInterface*> layers = GetLayersForPass(pass);
    
    if(piggyBacks) {
        layers.insert(layers.end(), piggyBacks->begin(), piggyBacks->end());
    }
    
    plMetalFragmentShaderArgumentBuffer uniforms;
    
    IHandleMaterial(GetPassIndex(pass), passDescription, &uniforms, piggyBacks,
    [&](plLayerInterface* layer, uint32_t index) {
        layer = preEncodeTransform(layer, index);
        IBuildLayerTexture(encoder, index, layer);
        
        plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());
        
        assert(index - GetPassIndex(pass) >= 0);
        EncodeTransform(layer, &vertexUniforms->uvTransforms[index]);
        
        return layer;
    }, [&](plLayerInterface* layer, uint32_t index) {
        layer = postEncodeTransform(layer, index);
        return layer;
    });
    
    encoder->setFragmentBytes(&uniforms, sizeof(plMetalFragmentShaderArgumentBuffer), BufferIndexFragArgBuffer);
}

void plMetalMaterialShaderRef::EncodeTransform(plLayerInterface* layer, UVOutDescriptor *transform) {
    matrix_float4x4 tXfm;
    hsMatrix2SIMD(layer->GetTransform(), &tXfm);
    transform->transform = tXfm;
    transform->UVWSrc = layer->GetUVWSrc();
}

//This is old - supporting the plate code.
//FIXME: Replace the plate codes path to texturing
void plMetalMaterialShaderRef::prepareTextures(MTL::RenderCommandEncoder *encoder, uint pass)
{

    plLayerInterface* layer = fMaterial->GetLayer(pass);
    if (!layer) {
        return;
    }
    fPipeline->CheckTextureRef(layer);

    // Load the image
    plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());

    if (!img) {
        return;
    }

    plMetalTextureRef* texRef = (plMetalTextureRef*)img->GetDeviceRef();

    if (!texRef->fTexture) {
        return;
    }
    
    if (plCubicEnvironmap::ConvertNoRef(layer->GetTexture()) != nullptr) {
    } else if (plMipmap::ConvertNoRef(layer->GetTexture()) != nullptr || plRenderTarget::ConvertNoRef(layer->GetTexture()) != nullptr) {
            encoder->setFragmentTexture(texRef->fTexture, Texture);
    }
}

void plMetalMaterialShaderRef::ILoopOverLayers()
{
    size_t j = 0;
    size_t pass = 0;

    for (j = 0; j < fMaterial->GetNumLayers(); )
    {
        size_t currLayer = j;
        
        //Create "fast encode" buffers
        //Fast encode can be used when there are no piggybacks or pushover layers. We'll load as much of the
        //base state of this layer as we can onto the GPU. Using fast encode, the renderer can avoid encoding
        //a lot of the render state, it will be on the GPU already.
        //I'd like to encode more data here, and use a heap. The heap hasn't happened yet because heaps are
        //private memory, and we don't have a window yet for a blit phase into private memory.
        MTL::Buffer *argumentBuffer = fDevice->newBuffer(sizeof(plMetalFragmentShaderArgumentBuffer), MTL::ResourceStorageModeManaged);
        
        plMetalFragmentShaderArgumentBuffer *layerBuffer = (plMetalFragmentShaderArgumentBuffer *)argumentBuffer->contents();
        
        plMetalFragmentShaderDescription passDescription;
        
        j = IHandleMaterial(currLayer, &passDescription, layerBuffer, nullptr,
                            [](plLayerInterface* layer, uint32_t index) {
                                return layer;
                            },
                            [](plLayerInterface* layer, uint32_t index) {
                                return layer;
                            });

        if (j == -1)
            break;
        
        passDescription.CacheHash();
        fFragmentShaderDescriptions.push_back(passDescription);
        
        std::vector<plLayerInterface*> layers(j);
        
        pass++;
        
        //encode the colors for this pass into our buffer for fast rendering
        for(int layerOffset = 0; layerOffset < j - currLayer; layerOffset ++) {
            plLayerInterface* layer = fMaterial->GetLayer(currLayer + layerOffset);
            layers[layerOffset] = layer;
            IBuildLayerTexture(NULL, layerOffset, layer);
        }
        
        fPasses.push_back(layers);
        
        argumentBuffer->didModifyRange(NS::Range(0, argumentBuffer->length()));
        
        fPassArgumentBuffers.push_back(argumentBuffer);
        
        fPassIndices.push_back(currLayer);
        fPassLengths.push_back(j - currLayer);
        fNumPasses++;

#if 0
        ISetFogParameters(fMaterial->GetLayer(iCurrMat));
#endif
    }
}

const hsGMatState plMetalMaterialShaderRef::ICompositeLayerState(const plLayerInterface* layer)
{
    hsGMatState state;
    state.Composite(layer->GetState(), fPipeline->GetMaterialOverride(true), fPipeline->GetMaterialOverride(false));
    return state;
}

void plMetalMaterialShaderRef::IBuildLayerTexture(MTL::RenderCommandEncoder *encoder, uint32_t offsetFromRootLayer, plLayerInterface* layer)
{
    fPipeline->CheckTextureRef(layer);
    plBitmap* texture = layer->GetTexture();
    
    if (texture != nullptr && encoder) {
        plMetalTextureRef *deviceTexture = (plMetalTextureRef *)texture->GetDeviceRef();
        if(!deviceTexture) {
            //FIXME: Better way to address missing textures than null pointers
            encoder->setFragmentTexture(nullptr, FragmentShaderArgumentAttributeCubicTextures + offsetFromRootLayer);
            encoder->setFragmentTexture(nullptr, FragmentShaderArgumentAttributeTextures + offsetFromRootLayer);
            return;
        }
        hsAssert(offsetFromRootLayer <= 8, "Too many layers requested");
        if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
            encoder->setFragmentTexture(deviceTexture->fTexture, FragmentShaderArgumentAttributeCubicTextures + offsetFromRootLayer);
        } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
            encoder->setFragmentTexture(deviceTexture->fTexture, FragmentShaderArgumentAttributeTextures + offsetFromRootLayer);
        }
        
    }
}

uint32_t plMetalMaterialShaderRef::ILayersAtOnce(uint32_t which)
{
    uint32_t currNumLayers = 1;

    plLayerInterface* lay = fMaterial->GetLayer(which);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoMultitexture)) {
        return currNumLayers;
    }

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (lay->GetMiscFlags() & hsGMatState::kMiscBumpChans)) {
        currNumLayers = 2;
        return currNumLayers;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) ||
        (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner)) {
        return currNumLayers;
    }

    int i;
    int maxLayers = 8;
    if (which + maxLayers > fMaterial->GetNumLayers()) {
        maxLayers = fMaterial->GetNumLayers() - which;
    }

    for (i = currNumLayers; i < maxLayers; i++) {
        plLayerInterface* lay = fMaterial->GetLayer(which + i);

        // Ignoring max UVW limit

        if ((lay->GetMiscFlags() & hsGMatState::kMiscBindNext) && (i+1 >= maxLayers)) {
            break;
        }

        if (lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere) {
            break;
        }

        if (!(fMaterial->GetLayer(which + i - 1)->GetMiscFlags() & hsGMatState::kMiscBindNext) && !ICanEatLayer(lay)) {
            break;
        }

        currNumLayers++;
    }

    return currNumLayers;
}

bool plMetalMaterialShaderRef::ICanEatLayer(plLayerInterface* lay)
{
    if (!lay->GetTexture()) {
        return false;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendNoColor) ||
        (lay->GetBlendFlags() & hsGMatState::kBlendAddColorTimesAlpha) ||
        (lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner)) {
        return false;
    }

    if ((lay->GetBlendFlags() & hsGMatState::kBlendAlpha) && (lay->GetAmbientColor().a < 1.f)) {
        return false;
    }

    if (!(lay->GetZFlags() & hsGMatState::kZNoZWrite)) {
        return false;
    }

    return true;
}

uint32_t plMetalMaterialShaderRef::IHandleMaterial(uint32_t layer, plMetalFragmentShaderDescription *passDescription, plMetalFragmentShaderArgumentBuffer *uniforms, std::vector<plLayerInterface*> *piggybacks, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> preEncodeTransform, std::function<plLayerInterface* (plLayerInterface*, uint32_t)> postEncodeTransform)
{
    if (!fMaterial || layer >= fMaterial->GetNumLayers() || !fMaterial->GetLayer(layer)) {
        return -1;
    }

    if (false /*ISkipBumpMap(fMaterial, layer)*/) {
        return -1;
    }
    
    memset(passDescription, 0, sizeof(plMetalFragmentShaderDescription));

    // Ignoring the bit about ATI Radeon and UVW limits

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoDecals) && (fMaterial->GetCompositeFlags() & hsGMaterial::kCompDecal)) {
        return -1;
    }

    // Ignoring the bit about self-rendering cube maps

    plLayerInterface* currLay = /*IPushOverBaseLayer*/ fMaterial->GetLayer(layer);
    currLay = preEncodeTransform(currLay, 0);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW) && (currLay->GetMiscFlags() & hsGMatState::kMiscBumpDu)) {
        currLay = fMaterial->GetLayer(++layer);
    }

    //currLay = IPushOverAllLayer(currLay);

    hsGMatState state = ICompositeLayerState(currLay);

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagDisableSpecular)) {
        state.fShadeFlags &= ~hsGMatState::kShadeSpecular;
    }

    // Stuff about ZInc

    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagNoAlphaBlending)) {
        state.fBlendFlags &= ~hsGMatState::kBlendMask;
    }

    if ((fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || fPipeline->IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (state.fMiscFlags & hsGMatState::kMiscBumpChans) ) {
        switch (state.fMiscFlags & hsGMatState::kMiscBumpChans)
        {
            case hsGMatState::kMiscBumpDu:
                break;
            case hsGMatState::kMiscBumpDv:
                if (!(fMaterial->GetLayer(layer-2)->GetBlendFlags() & hsGMatState::kBlendAdd))
                {
                    state.fBlendFlags &= ~hsGMatState::kBlendMask;
                    state.fBlendFlags |= hsGMatState::kBlendMADD;
                }
                break;
            case hsGMatState::kMiscBumpDw:
                if (!(fMaterial->GetLayer(layer-1)->GetBlendFlags() & hsGMatState::kBlendAdd))
                {
                    state.fBlendFlags &= ~hsGMatState::kBlendMask;
                    state.fBlendFlags |= hsGMatState::kBlendMADD;
                }
                break;
            default:
                break;
        }
    }

    uint32_t currNumLayers = ILayersAtOnce(layer);

    if (state.fMiscFlags & (hsGMatState::kMiscBumpDu | hsGMatState::kMiscBumpDw)) {
        //ISetBumpMatrices(currLay);
    }
    
    passDescription->Populate(currLay, 0);
    
    postEncodeTransform(currLay, 0);
    
    int32_t i = 1;
    for (i = 1; i < currNumLayers; i++)
    {
        plLayerInterface* layPtr = fMaterial->GetLayer(layer + i);
        if (!layPtr) {
            return -1;
        }
        layPtr = preEncodeTransform(layPtr, i);
        
        passDescription->Populate(layPtr,  i);
        
        layPtr = postEncodeTransform(layPtr, i);
    }
    
    if(piggybacks) {
        for (int32_t currPiggyback = 0; currPiggyback < piggybacks->size(); currPiggyback++)
        {

            plLayerInterface* layPtr = piggybacks->at(currPiggyback);
            if (!layPtr) {
                return -1;
            }
            layPtr = preEncodeTransform(layPtr, i + currPiggyback);
            
            passDescription->Populate(layPtr, i + currPiggyback);
            
            layPtr = postEncodeTransform(layPtr, i + currPiggyback);
        }
    }
    
    passDescription->numLayers = ( piggybacks ? piggybacks->size() : 0 ) + currNumLayers;
    
    if (state.fBlendFlags & (hsGMatState::kBlendTest | hsGMatState::kBlendAlpha | hsGMatState::kBlendAddColorTimesAlpha) &&
        !(state.fBlendFlags & hsGMatState::kBlendAlphaAlways))
    {
        // AlphaTestHigh is used for reducing sort artifacts on textures that
        // are mostly opaque or transparent, but have regions of translucency
        // in transition. Like a texture for a bush billboard. It lets there be
        // some transparency falloff, but quit drawing before it gets so
        // transparent that draw order problems (halos) become apparent.
        if (state.fBlendFlags & hsGMatState::kBlendAlphaTestHigh) {
            uniforms->alphaThreshold = 64.f/255.f;
        } else {
            uniforms->alphaThreshold = 1.f/255.f;
        }
    } else {
        uniforms->alphaThreshold = 0.f;
    }
    
    return layer + currNumLayers;
}
