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

#ifndef plMetalDevice_hpp
#define plMetalDevice_hpp

//We need to define these once for Metal somewhere in a cpp file
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <string_theory/format>
#include "plMetalDevice.h"
#include "plMetalPipeline.h"
#include "ShaderTypes.h"


#include "hsThread.h"
#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plPipeline/plRenderTarget.h"

#include "plMetalPipelineState.h"

matrix_float4x4* hsMatrix2SIMD(const hsMatrix44& src, matrix_float4x4* dst, bool swapOrder)
{
    if (src.fFlags & hsMatrix44::kIsIdent)
    {
        memcpy(dst, &matrix_identity_float4x4, sizeof(float) * 16);
    }
    else
    {
        //SIMD is column major, hsMatrix44 is row major.
        //We need to flip.
        if(swapOrder) {
            dst->columns[0][0] = src.fMap[0][0];
            dst->columns[1][0] = src.fMap[0][1];
            dst->columns[2][0] = src.fMap[0][2];
            dst->columns[3][0] = src.fMap[0][3];
            
            dst->columns[0][1] = src.fMap[1][0];
            dst->columns[1][1] = src.fMap[1][1];
            dst->columns[2][1] = src.fMap[1][2];
            dst->columns[3][1] = src.fMap[1][3];
            
            dst->columns[0][2] = src.fMap[2][0];
            dst->columns[1][2] = src.fMap[2][1];
            dst->columns[2][2] = src.fMap[2][2];
            dst->columns[3][2] = src.fMap[2][3];
            
            dst->columns[0][3] = src.fMap[3][0];
            dst->columns[1][3] = src.fMap[3][1];
            dst->columns[2][3] = src.fMap[3][2];
            dst->columns[3][3] = src.fMap[3][3];
        } else {
            memcpy(dst, &src.fMap, sizeof(matrix_float4x4));
        }
    }

    return dst;
}


bool plMetalDevice::InitDevice()
{
    //FIXME: Should Metal adopt InitDevice like OGL?
    hsAssert(0, "InitDevice not implemented for Metal rendering");
}

void plMetalDevice::Shutdown()
{
    //FIXME: Should Metal adopt Shutdown like OGL?
    hsAssert(0, "Shutdown not implemented for Metal rendering");
}


void plMetalDevice::SetMaxAnsiotropy(uint8_t maxAnsiotropy)
{
    //setup the material pass samplers
    //load them all at once and then let the shader pick
    
    if (maxAnsiotropy == 0)
        maxAnsiotropy = 1;
    
    if(fSamplerStates[0] != nullptr) {
        ReleaseSamplerStates();
    }
    
    MTL::SamplerDescriptor *samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
    samplerDescriptor->setMaxAnisotropy(maxAnsiotropy);
    samplerDescriptor->setMinFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMagFilter(MTL::SamplerMinMagFilterLinear);
    samplerDescriptor->setMipFilter(MTL::SamplerMipFilterLinear);
    
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeRepeat);
    fSamplerStates[0] = fMetalDevice->newSamplerState(samplerDescriptor);
    
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeRepeat);
    fSamplerStates[1] = fMetalDevice->newSamplerState(samplerDescriptor);
    
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeRepeat);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    fSamplerStates[2] = fMetalDevice->newSamplerState(samplerDescriptor);
    
    samplerDescriptor->setSAddressMode(MTL::SamplerAddressModeClampToEdge);
    samplerDescriptor->setTAddressMode(MTL::SamplerAddressModeClampToEdge);
    fSamplerStates[3] = fMetalDevice->newSamplerState(samplerDescriptor);
}

void plMetalDevice::SetMSAASampleCount(uint8_t sampleCount)
{
    //Plasma has some MSAA levels that don't completely correspond to what Metal can do
    //Best fit them to levels Metal can do. Once they are best fit see if the hardware
    //is capable.
    
    uint8_t actualSampleCount = 1;
    if (sampleCount == 6) {
        actualSampleCount = 8;
    } else if (sampleCount == 4) {
        actualSampleCount = 4;
    } else if (sampleCount == 2) {
        actualSampleCount = 2;
    }
    
    while (actualSampleCount != 1) {
        if (fMetalDevice->supportsTextureSampleCount(actualSampleCount)) {
            break;
        }
        actualSampleCount /= 2;
    }
    
    fSampleCount = actualSampleCount;
}

void plMetalDevice::ReleaseSamplerStates()
{
    fSamplerStates[0]->release();
    fSamplerStates[0] = nullptr;
    
    fSamplerStates[1]->release();
    fSamplerStates[1] = nullptr;
    
    fSamplerStates[2]->release();
    fSamplerStates[2] = nullptr;
    
    fSamplerStates[3]->release();
    fSamplerStates[3] = nullptr;
}

void plMetalDevice::Clear(bool shouldClearColor, simd_float4 clearColor, bool shouldClearDepth, float clearDepth) {
    
    //Plasma may clear a target and draw at different times.
    //This is specifically trouble with the drawable clear
    //Plasma might clear the drawable, and then go off and do
    //off screen stuff. Metal doesn't work that way, we need to
    //draw and clear at the same time. So if it's a clear for the
    //current drawable, remember that and perform the clear when
    //we're actually drawing to screen.
    
    if (fCurrentRenderTargetCommandEncoder) {
        half4 halfClearColor;
        halfClearColor[0] = clearColor.r;
        halfClearColor[1] = clearColor.g;
        halfClearColor[2] = clearColor.b;
        halfClearColor[3] = clearColor.a;
        plMetalDevice::plMetalLinkedPipeline *linkedPipeline = plMetalClearPipelineState(this, shouldClearColor, shouldClearDepth).GetRenderPipelineState();
        
        const MTL::RenderPipelineState *pipelineState = linkedPipeline->pipelineState;
        CurrentRenderCommandEncoder()->setRenderPipelineState(pipelineState);
        
        float clearCoords[8] = {
            -1, -1,
            1, -1,
            -1, 1,
            1, 1
        };
        float clearDepth = 1.0f;
        CurrentRenderCommandEncoder()->setDepthStencilState(fNoZReadStencilState);
        
        CurrentRenderCommandEncoder()->setCullMode(MTL::CullModeNone);
        CurrentRenderCommandEncoder()->setVertexBytes(&clearCoords, sizeof(clearCoords), 0);
        CurrentRenderCommandEncoder()->setFragmentBytes(&halfClearColor, sizeof(halfClearColor), 0);
        CurrentRenderCommandEncoder()->setFragmentBytes(&clearDepth, sizeof(float), 1);
        CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
    } else {
        if (shouldClearColor) {
            if (fCurrentRenderTarget) {
                fClearRenderTargetColor = clearColor;
                fShouldClearRenderTarget = shouldClearColor;
                if (shouldClearDepth) {
                    fClearRenderTargetDepth = clearDepth;
                }
            } else {
                fClearDrawableColor = clearColor;
                fShouldClearDrawable = shouldClearColor;
                if (shouldClearDepth) {
                    fClearDrawableDepth = clearDepth;
                }
            }
        }
    }
    
}

void plMetalDevice::BeginNewRenderPass() {
    
    //printf("Beginning new render pass\n");
    
    //lazilly create the screen render encoder if it does not yet exist
    if (!fCurrentOffscreenCommandBuffer && !fCurrentRenderTargetCommandEncoder) {
        SetRenderTarget(NULL);
    }
    
    if (fCurrentRenderTargetCommandEncoder) {
        //if we have an existing render target, submit it's commands and release it
        //if we need to come back to this render target, we can always create a new render
        //pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
    
    MTL::RenderPassDescriptor *renderPassDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
    renderPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    if (fCurrentRenderTarget) {
        renderPassDescriptor->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(fClearRenderTargetColor.x, fClearRenderTargetColor.y, fClearRenderTargetColor.z, fClearRenderTargetColor.w));
        if (fShouldClearRenderTarget) {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        } else {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
        }
        
        if ( fCurrentRenderTarget->GetZDepth() ) {
            plMetalRenderTargetRef* deviceTarget= (plMetalRenderTargetRef *)fCurrentRenderTarget->GetDeviceRef();
            renderPassDescriptor->depthAttachment()->setTexture(deviceTarget->fDepthBuffer);
            renderPassDescriptor->depthAttachment()->setClearDepth(fClearRenderTargetDepth);
            renderPassDescriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
            renderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        }
        
        renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentOutputTexture);
        
        fCurrentRenderTargetCommandEncoder = fCurrentOffscreenCommandBuffer->renderCommandEncoder(renderPassDescriptor)->retain();
    } else {
        renderPassDescriptor->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(fClearDrawableColor.x, fClearDrawableColor.y, fClearDrawableColor.z, fClearDrawableColor.w));
        if (fShouldClearDrawable) {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        } else {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
        }
        
        renderPassDescriptor->depthAttachment()->setClearDepth(fClearDrawableDepth);
        renderPassDescriptor->depthAttachment()->setLoadAction(MTL::LoadActionClear);
        renderPassDescriptor->depthAttachment()->setTexture(fCurrentDrawableDepthTexture);
        renderPassDescriptor->depthAttachment()->setStoreAction(MTL::StoreActionDontCare);
        
        
        if (fSampleCount == 1) {
            renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentOutputTexture);
        } else {
            renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentMSAAOutputTexture);
            
            //if we need postprocessing, output to the main pass texture
            //otherwise we can go straight to the drawable
            if (NeedsPostprocessing()) {
                renderPassDescriptor->colorAttachments()->object(0)->setResolveTexture(fCurrentUnprocessedOutputTexture);
            } else {
                renderPassDescriptor->colorAttachments()->object(0)->setResolveTexture(fCurrentFragmentOutputTexture);
            }
            
            renderPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionMultisampleResolve);
        }
        
        fCurrentRenderTargetCommandEncoder = fCurrentCommandBuffer->renderCommandEncoder(renderPassDescriptor)->retain();
    }
    
    fCurrentRenderTargetCommandEncoder->setFragmentSamplerStates(fSamplerStates, NS::Range::Make(0, 4));
}

void plMetalDevice::SetRenderTarget(plRenderTarget* target)
{
    /*
     If we're being asked to set the render target to the current drawable,
     but we're being asked to set the render target to the drawable, don't do anything.
     We used to allow starting new passes on the same drawable but that would break
     memoryless buffers on Apple Silicon that don't survive between passes.
     */
    if((!fCurrentRenderTarget && !target) && fCurrentRenderTargetCommandEncoder) {
        return;
    }
    if( fCurrentRenderTargetCommandEncoder ) {
        //if we have an existing render target, submit it's commands and release it
        //if we need to come back to this render target, we can always create a new render
        //pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
    
    if( fCurrentOffscreenCommandBuffer ) {
        if (fCurrentRenderTarget && fCurrentRenderTarget->GetFlags() & plRenderTarget::kIsOffscreen) {
            //if our target was offscreen, go ahead and blit back. Something will want this data.
            MTL::BlitCommandEncoder* blitEncoder = fCurrentOffscreenCommandBuffer->blitCommandEncoder();
            blitEncoder->synchronizeResource(fCurrentFragmentOutputTexture);
            blitEncoder->endEncoding();
        }
        fCurrentOffscreenCommandBuffer->commit();
        if (fCurrentRenderTarget && fCurrentRenderTarget->GetFlags() & plRenderTarget::kIsOffscreen) {
            //if it's an offscreen buffer, wait for completion
            //something is probably going to want to syncronously grab data
            fCurrentOffscreenCommandBuffer->waitUntilCompleted();
        }
        fCurrentOffscreenCommandBuffer->release();
        fCurrentOffscreenCommandBuffer = nil;
    }
    
    fCurrentRenderTarget = target;
    
    if ( fCurrentRenderTarget && fShouldClearRenderTarget == false ) {
        // clear if a clear color wasn't already set
        fClearRenderTargetColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
        fShouldClearRenderTarget = true;
        fClearRenderTargetDepth = 1.0;
    }
    
    if(fCurrentRenderTarget) {
        if(!target->GetDeviceRef()) {
            fPipeline->MakeRenderTargetRef(target);
        }
        plMetalRenderTargetRef *deviceTarget= (plMetalRenderTargetRef *)target->GetDeviceRef();
        fCurrentOffscreenCommandBuffer = fCommandQueue->commandBuffer();
        fCurrentOffscreenCommandBuffer->retain();
        fCurrentFragmentOutputTexture = deviceTarget->fTexture;
        
        if(deviceTarget->fDepthBuffer) {
            fCurrentDepthFormat = MTL::PixelFormatDepth32Float_Stencil8;
        } else {
            fCurrentDepthFormat = MTL::PixelFormatInvalid;
        }
    } else {
        fCurrentFragmentOutputTexture = fCurrentDrawable->texture();
        fCurrentDepthFormat = MTL::PixelFormatDepth32Float_Stencil8;
    }
}

plMetalDevice::plMetalDevice()
:   fErrorMsg(nullptr),
    fActiveThread(hsThread::ThisThreadHash()),
    fCurrentDrawable(nullptr),
    fCommandQueue(nullptr),
    fCurrentRenderTargetCommandEncoder(nullptr),
    fCurrentDrawableDepthTexture(nullptr),
    fCurrentFragmentOutputTexture(nullptr),
    fCurrentCommandBuffer(nullptr),
    fCurrentOffscreenCommandBuffer(nullptr),
    fCurrentRenderTarget(nullptr),
    fNewPipelineStateMap(),
    fCurrentFragmentMSAAOutputTexture(nullptr),
    fCurrentUnprocessedOutputTexture(nullptr),
    fGammaLUTTexture(nullptr),
    fGammaAdjustState(nullptr),
    fBlitCommandBuffer(nullptr),
    fBlitCommandEncoder(nullptr)
    {
    fClearRenderTargetColor = {0.0, 0.0, 0.0, 1.0};
    fClearDrawableColor = {0.0, 0.0, 0.0, 1.0};
    fSamplerStates[0] = nullptr;
        
    fMetalDevice = MTL::CreateSystemDefaultDevice();
    fCommandQueue = fMetalDevice->newCommandQueue();
    
    //set up all the depth stencil states
    MTL::DepthStencilDescriptor *depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(true);
    depthDescriptor->setLabel(NS::String::string("No Z Read", NS::UTF8StringEncoding));
    fNoZReadStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthDescriptor->setDepthWriteEnabled(false);
    depthDescriptor->setLabel(NS::String::string("No Z Write", NS::UTF8StringEncoding));
    fNoZWriteStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionAlways);
    depthDescriptor->setDepthWriteEnabled(false);
    depthDescriptor->setLabel(NS::String::string("No Z Read or Write", NS::UTF8StringEncoding));
    fNoZReadOrWriteStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionLessEqual);
    depthDescriptor->setLabel(NS::String::string("Z Read and Write", NS::UTF8StringEncoding));
    depthDescriptor->setDepthWriteEnabled(true);
    fDefaultStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->setDepthCompareFunction(MTL::CompareFunctionGreaterEqual);
    depthDescriptor->setLabel(NS::String::string("Reverse Z", NS::UTF8StringEncoding));
    depthDescriptor->setDepthWriteEnabled(true);
    fReverseZStencilState = fMetalDevice->newDepthStencilState(depthDescriptor);
        
    depthDescriptor->release();
}

void plMetalDevice::SetViewport() {
    CurrentRenderCommandEncoder()->setViewport({ (double)fPipeline->GetViewTransform().GetViewPortLeft(),
        (double)fPipeline->GetViewTransform().GetViewPortTop(),
        (double)fPipeline->GetViewTransform().GetViewPortWidth(),
        (double)fPipeline->GetViewTransform().GetViewPortHeight(),
        0.f, 1.f });
}

bool plMetalDevice::BeginRender() {
    if (fActiveThread == hsThread::ThisThreadHash()) {
        return true;
    }

    fActiveThread = hsThread::ThisThreadHash();
    
    return true;
}

static uint32_t  IGetBufferFormatSize(uint8_t format)
{
    uint32_t  size = sizeof( float ) * 6 + sizeof( uint32_t ) * 2; // Position and normal, and two packed colors

    switch (format & plGBufferGroup::kSkinWeightMask)
    {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            size += sizeof(float);
            break;
        default:
            hsAssert( false, "Invalid skin weight value in IGetBufferFormatSize()" );
    }

    size += sizeof( float ) * 3 * plGBufferGroup::CalcNumUVs(format);

    return size;
}

void plMetalDevice::SetupVertexBufferRef(plGBufferGroup *owner, uint32_t idx, plMetalDevice::VertexBufferRef *vRef)
{
    uint8_t format = owner->GetVertexFormat();
    
    if (format & plGBufferGroup::kSkinIndices) {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
    }
    
    uint32_t vertSize = vertSize = IGetBufferFormatSize(format); // vertex stride
    uint32_t numVerts = owner->GetVertBufferCount(idx);

    vRef->fOwner = owner;
    vRef->fCount = numVerts;
    vRef->fVertexSize = vertSize;
    vRef->fFormat = format;
    vRef->fRefTime = 0;

    vRef->SetDirty(true);
    vRef->SetRebuiltSinceUsed(true);
    vRef->fData = nullptr;

    vRef->SetVolatile(vRef->Volatile() || owner->AreVertsVolatile());

    vRef->fIndex = idx;
    
    const uint32_t vertStart = owner->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = owner->GetVertBufferEnd(idx) * vertSize - vertStart;
    
    owner->SetVertexBufferRef(idx, vRef);

    hsRefCnt_SafeUnRef(vRef);
}

void plMetalDevice::CheckStaticVertexBuffer(plMetalDevice::VertexBufferRef *vRef, plGBufferGroup *owner, uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");
    
    if (!vRef->GetBuffer())
    {
        FillVertexBufferRef(vRef, owner, idx);
        
        // This is currently a no op, but this would let the buffer know it can
        // unload the system memory copy, since we have a managed version now.
        owner->PurgeVertBuffer(idx);
    }
}

void plMetalDevice::FillVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;
    
    if(ref->GetBuffer()) {
        assert(size <= ref->GetBuffer()->length());
    }
    
    if (!size)
    {
        return;
    }
    
    MTL::Buffer* metalBuffer = fMetalDevice->newBuffer(size, MTL::StorageModeManaged);
    ref->SetBuffer(metalBuffer);
    uint8_t* buffer = (uint8_t*) ref->GetBuffer()->contents();

    if (ref->fData)
    {
        memcpy(buffer, ref->fData + vertStart, size);
    }
    else
    {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");
        
        uint8_t* ptr = buffer;

        const uint32_t vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t* srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData(idx);

        const int numCells = group->GetNumCells(idx);
        int i;
        for (i = 0; i < numCells; i++)
        {
            plGBufferCell* cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1))
            {
                /// Interleaved, do straight copy
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
                assert(size <= cell->fLength * vertSize);
            }
            else
            {
                hsStatusMessage("Non interleaved data");

                /// Separated, gotta interleave
                uint8_t* tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;
                int j;
                for( j = 0; j < cell->fLength; j++ )
                {
                    memcpy( ptr, tempVPtr, sizeof( hsPoint3 ) * 2 );
                    ptr += sizeof( hsPoint3 ) * 2;
                    tempVPtr += sizeof( hsPoint3 ) * 2;

                    memcpy( ptr, &tempCPtr->fDiffuse, sizeof( uint32_t ) );
                    ptr += sizeof( uint32_t );
                    memcpy( ptr, &tempCPtr->fSpecular, sizeof( uint32_t ) );
                    ptr += sizeof( uint32_t );

                    memcpy( ptr, tempVPtr, vertSmallSize );
                    ptr += vertSmallSize;
                    tempVPtr += vertSmallSize;
                    tempCPtr++;
                }
            }
        }

        hsAssert((ptr - buffer) == size, "Didn't fill the buffer?");
    }
    
    metalBuffer->release();

    /// Unlock and clean up
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plMetalDevice::FillVolatileVertexBufferRef(plMetalDevice::VertexBufferRef *ref, plGBufferGroup *group, uint32_t idx)
{
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    size_t uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(float) * 3;
    uint8_t numWeights = (group->GetVertexFormat() & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < ref->fCount; ++i) {
        memcpy(dst, src, sizeof(hsPoint3)); // pre-pos
        dst += sizeof(hsPoint3);
        src += sizeof(hsPoint3);

        src += numWeights * sizeof(float); // weights

        if (group->GetVertexFormat() & plGBufferGroup::kSkinIndices)
            src += sizeof(uint32_t); // indices

        memcpy(dst, src, sizeof(hsVector3)); // pre-normal
        dst += sizeof(hsVector3);
        src += sizeof(hsVector3);

        memcpy(dst, src, sizeof(uint32_t) * 2); // diffuse & specular
        dst += sizeof(uint32_t) * 2;
        src += sizeof(uint32_t) * 2;

        // UVWs
        memcpy(dst, src, uvChanSize);
        src += uvChanSize;
        dst += uvChanSize;
    }
}

void plMetalDevice::SetupIndexBufferRef(plGBufferGroup *owner, uint32_t idx, plMetalDevice::IndexBufferRef *iRef)
{
    uint32_t numIndices = owner->GetIndexBufferCount(idx);
    iRef->fCount = numIndices;
    iRef->fOwner = owner;
    iRef->fIndex = idx;
    iRef->fRefTime = 0;

    iRef->SetDirty(true);
    iRef->SetRebuiltSinceUsed(true);

    owner->SetIndexBufferRef(idx, iRef);
    hsRefCnt_SafeUnRef(iRef);

    iRef->SetVolatile(owner->AreIdxVolatile());
}

void plMetalDevice::CheckIndexBuffer(plMetalDevice::IndexBufferRef *iRef)
{
    if(!iRef->GetBuffer() && iRef->fCount) {
        iRef->SetVolatile(false);
        
        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plMetalDevice::FillIndexBufferRef(plMetalDevice::IndexBufferRef *iRef, plGBufferGroup *owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);

    if (!size)
    {
        return;
    }
    
    iRef->PrepareForWrite();
    MTL::Buffer* indexBuffer = iRef->GetBuffer();
    if(!indexBuffer || indexBuffer->length() < size) {
        indexBuffer = fMetalDevice->newBuffer(size, MTL::ResourceStorageModeManaged);
        iRef->SetBuffer(indexBuffer);
        indexBuffer->release();
    }
    
    memcpy(indexBuffer->contents(), owner->GetIndexBufferData(idx), size);
    indexBuffer->didModifyRange(NS::Range(0, size));

    iRef->SetDirty(false);
}

void plMetalDevice::SetupTextureRef(plBitmap *img, plMetalDevice::TextureRef *tRef)
{
    tRef->fOwner = img;
    
    plBitmap* imageToCheck = img;
    
    //if it's a cubic texture, check the first face. The root img will give a false format that will cause us to decode wrong.
    plCubicEnvironmap* cubicImg = dynamic_cast<plCubicEnvironmap*>(img);
    if(cubicImg) {
        imageToCheck = cubicImg->GetFace(0);
    }

    if (imageToCheck->IsCompressed()) {
        switch (imageToCheck->fDirectXInfo.fCompressionType) {
        case plBitmap::DirectXInfo::kDXT1:
                tRef->fFormat = MTL::PixelFormatBC1_RGBA;
            break;
        case plBitmap::DirectXInfo::kDXT5:
                tRef->fFormat = MTL::PixelFormatBC3_RGBA;
            break;
        }
    } else {
        switch (imageToCheck->fUncompressedInfo.fType) {
        case plBitmap::UncompressedInfo::kRGB8888:
            tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
            break;
        case plBitmap::UncompressedInfo::kRGB4444:
            //we'll convert this on load to 8 bits per channel
            //Metal doesn't support 4 bits per channel on all hardware
            tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
            break;
        case plBitmap::UncompressedInfo::kRGB1555:
            tRef->fFormat = MTL::PixelFormatBGR5A1Unorm;
            break;
        case plBitmap::UncompressedInfo::kInten8:
            tRef->fFormat = MTL::PixelFormatR8Uint;
            break;
        case plBitmap::UncompressedInfo::kAInten88:
            tRef->fFormat = MTL::PixelFormatRG8Uint;
            break;
        }
    }

    tRef->SetDirty(true);

    img->SetDeviceRef(tRef);
    hsRefCnt_SafeUnRef(tRef);
}

void plMetalDevice::ReleaseFramebufferObjects()
{
    if (fCurrentUnprocessedOutputTexture)
        fCurrentUnprocessedOutputTexture->release();
    fCurrentFragmentOutputTexture = nil;
    
    if (fGammaAdjustState)
        fGammaAdjustState->release();
    fGammaAdjustState = nil;
}

void plMetalDevice::SetFramebufferFormat(MTL::PixelFormat format)
{
    if (fFramebufferFormat != format) {
        ReleaseFramebufferObjects();
        fFramebufferFormat = format;
    }
}

void plMetalDevice::CheckTexture(plMetalDevice::TextureRef *tRef)
{
    if (!tRef->fTexture)
    {
        tRef->SetDirty(true);
    }
}

uint plMetalDevice::ConfigureAllowedLevels(plMetalDevice::TextureRef *tRef, plMipmap *mipmap)
{
    if (mipmap->IsCompressed()) {
        mipmap->SetCurrLevel(tRef->fLevels);
        while ((mipmap->GetCurrWidth() | mipmap->GetCurrHeight()) & 0x03) {
            tRef->fLevels--;
            hsAssert(tRef->fLevels >= 0, "How was this ever compressed?" );
            if(tRef->fLevels < 0) {
                tRef->fLevels = -1;
                break;
            }
            mipmap->SetCurrLevel(tRef->fLevels);
        }
    }
}

void plMetalDevice::PopulateTexture(plMetalDevice::TextureRef *tRef, plMipmap *img, uint slice)
{
    if (img->IsCompressed()) {
        /*
         Some cubic assets have inconsistant mipmap sizes between their faces.
         The DX pipeline maintains seperate structures noting the expected
         mipmap sizes, and ignores the actual face sizes. This hack
         makes the Metal pipeline ignore the actual face sizes and behave
         as if all face sizes are equivelent to the first face. It does this
         by computing the expected mipmap sizes on the fly.
         This hack could be disabled if cube maps in the assets were
         fixed to be consistant.
         */
#define HACK_LEVEL_SIZE 1
        
#if HACK_LEVEL_SIZE
        uint width = tRef->fTexture->width();
        uint height = tRef->fTexture->height();
#endif
        
        if (tRef->fLevels == -1) {
            hsAssert(1, "Bad texture found");
            return;
        }
        
        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);
#if HACK_LEVEL_SIZE
            uint levelWidth = (width / exp2(lvl));
            uint levelHeight = (height / exp2(lvl));
#else
            uint levelWidth = img->GetCurrWidth();
            uint levelHeight = img->GetCurrHeight();
#endif
            
            switch (img->fDirectXInfo.fCompressionType) {
                case plBitmap::DirectXInfo::kDXT1:
                        tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, levelWidth, levelHeight), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), levelWidth * 2, 0);
                    break;
                case plBitmap::DirectXInfo::kDXT5:
                        tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 4, 0);
                    break;
                }
        }
    } else {
        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);
            
            if(img->GetCurrLevelPtr()) {
                if(img->fUncompressedInfo.fType == plBitmap::UncompressedInfo::kRGB4444) {
                    
                    struct RGBA4444Component {
                        unsigned r:4;
                        unsigned g:4;
                        unsigned b:4;
                        unsigned a:4;
                    };
                    
                    RGBA4444Component *in = (RGBA4444Component *)img->GetCurrLevelPtr();
                    simd_uint4 *out = (simd_uint4 *) malloc(img->GetCurrHeight() * img->GetCurrWidth() * 4);
                    
                    for(int i=0; i<(img->GetCurrWidth() * img->GetCurrHeight()); i++) {
                        out[i].r = in[i].r;
                        out[i].g = in[i].g;
                        out[i].b = in[i].b;
                        out[i].a = in[i].a;
                    }
                    
                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, out, img->GetCurrWidth() * 4, 0);
                    
                    free(out);
                } else {
                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 4, 0);
                }
            } else {
                printf("Texture with no image data?\n");
            }
        }
    }
    tRef->fTexture->setLabel(NS::String::string(img->GetKeyName().c_str(), NS::UTF8StringEncoding));
    tRef->SetDirty(false);
}

void plMetalDevice::MakeTextureRef(plMetalDevice::TextureRef* tRef, plMipmap* img)
{
    if (!img->GetImage()) {
        return;
    }
    
    if(tRef->fTexture) {
        tRef->fTexture->release();
    }
    
    tRef->fLevels = img->GetNumLevels() - 1;
    //if(!tRef->fTexture) {
        ConfigureAllowedLevels(tRef, img);
        
        bool textureIsValid = tRef->fLevels > 0;
        
        //texture doesn't exist yet, create it
        bool supportsMipMap = tRef->fLevels && textureIsValid;
        MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::texture2DDescriptor(tRef->fFormat, img->GetWidth(), img->GetHeight(), supportsMipMap);
        descriptor->setUsage(MTL::TextureUsageShaderRead);

    //Metal gets mad if we set this with 0, only set it if we know there are mipmaps
    if(supportsMipMap) {
        descriptor->setMipmapLevelCount(tRef->fLevels + 1);
    }

    //if device has unified memory, set storage mode to shared
    if(fMetalDevice->supportsFamily(MTL::GPUFamilyApple1)) {
        descriptor->setStorageMode(MTL::StorageModeShared);
    } else {
        descriptor->setStorageMode(MTL::StorageModeManaged);
    }
    
    
    tRef->fTexture = fMetalDevice->newTexture(descriptor);
    PopulateTexture( tRef, img, 0);
    if(!fMetalDevice->supportsFamily(MTL::GPUFamilyApple1)) {
        descriptor->setStorageMode(MTL::StorageModePrivate);
        MTL::Texture* privateTexture = fMetalDevice->newTexture(descriptor);
        BlitTexture(tRef->fTexture, privateTexture);
        tRef->fTexture->autorelease();
        tRef->fTexture = privateTexture;
    }
    //}
    
    
    tRef->SetDirty(false);
}

void plMetalDevice::MakeCubicTextureRef(plMetalDevice::TextureRef *tRef, plCubicEnvironmap *img)
{
    MTL::TextureDescriptor *descriptor = MTL::TextureDescriptor::textureCubeDescriptor(tRef->fFormat, img->GetFace(0)->GetWidth(), tRef->fLevels != 0);
    
    if (tRef->fLevels != 0) {
        descriptor->setMipmapLevelCount(tRef->fLevels + 1);
    }
    descriptor->setUsage(MTL::TextureUsageShaderRead);
    //if device has unified memory, set storage mode to shared
    if(fMetalDevice->supportsFamily(MTL::GPUFamilyApple1)) {
        descriptor->setStorageMode(MTL::StorageModeShared);
    }
    
    tRef->fTexture = fMetalDevice->newTexture(descriptor);
    
    static const uint kFaceMapping[] = {
        1, // kLeftFace
        0, // kRightFace
        4, // kFrontFace
        5, // kBackFace
        2, // kTopFace
        3  // kBottomFace
    };
    for (size_t i = 0; i < 6; i++) {
        PopulateTexture( tRef, img->GetFace(i), kFaceMapping[i]);
    }
    
    tRef->SetDirty(false);
}

void plMetalDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    hsMatrix2SIMD(src, &fMatrixProj);
}

void plMetalDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);
    
    hsMatrix2SIMD(src, &fMatrixW2C);
    hsMatrix2SIMD(inv, &fMatrixC2W);
}

void plMetalDevice::SetLocalToWorldMatrix(const hsMatrix44& src, bool swapOrder)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);
    
    hsMatrix2SIMD(src, &fMatrixL2W, swapOrder);
    hsMatrix2SIMD(inv, &fMatrixW2L, swapOrder);
}

void plMetalDevice::CreateNewCommandBuffer(CA::MetalDrawable* drawable)
{
    fCurrentCommandBuffer = fCommandQueue->commandBuffer();
    fCurrentCommandBuffer->retain();
    
    SetFramebufferFormat(drawable->texture()->pixelFormat());
    
    bool depthNeedsRebuild = fCurrentDrawableDepthTexture == nullptr;
    depthNeedsRebuild |= drawable->texture()->width() != fCurrentDrawableDepthTexture->width() || drawable->texture()->height() != fCurrentDrawableDepthTexture->height();
    
    //cache the depth buffer, we'll just clear it every time.
    if(depthNeedsRebuild) {
        if(fCurrentDrawableDepthTexture) {
            fCurrentDrawableDepthTexture->release();
            fCurrentFragmentMSAAOutputTexture->release();
        }
        
        MTL::TextureDescriptor *depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float_Stencil8,
                                                        drawable->texture()->width(),
                                                        drawable->texture()->height(),
                                                        false);
        if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
            depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
        }   else {
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
        }
        depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
        
        if (fSampleCount != 1) {
            //MSSA depth and color output
            depthTextureDescriptor->setSampleCount(fSampleCount);
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            depthTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
            if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
                depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
            }   else {
                depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            }
            fCurrentDrawableDepthTexture = fMetalDevice->newTexture(depthTextureDescriptor);
            
            MTL::TextureDescriptor *msaaColorTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(drawable->texture()->pixelFormat(),
                                                                                                             drawable->texture()->width(),
                                                                                                             drawable->texture()->height(),
                                                                                                             false);
            msaaColorTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
            if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
                msaaColorTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
            }   else {
                msaaColorTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            }
            msaaColorTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
            msaaColorTextureDescriptor->setSampleCount(fSampleCount);
            fCurrentFragmentMSAAOutputTexture = fMetalDevice->newTexture(msaaColorTextureDescriptor);
        } else {
            fCurrentDrawableDepthTexture = fMetalDevice->newTexture(depthTextureDescriptor);
        }
    }
    
    //Do we need to create a unprocessed output texture?
    //If the depth needs to be rebuilt - we probably need to rebuild this one too
    if ((fCurrentUnprocessedOutputTexture && depthNeedsRebuild) || (fCurrentUnprocessedOutputTexture == nullptr && NeedsPostprocessing())) {
        MTL::TextureDescriptor* mainPassDescriptor = MTL::TextureDescriptor::texture2DDescriptor(drawable->texture()->pixelFormat(), drawable->texture()->width(), drawable->texture()->height(), false);
        mainPassDescriptor->setStorageMode(MTL::StorageModePrivate);
        fCurrentUnprocessedOutputTexture->release();
        fCurrentUnprocessedOutputTexture = fMetalDevice->newTexture(mainPassDescriptor);
    }
    
    fCurrentDrawable = drawable->retain();
}

void plMetalDevice::StartPipelineBuild(plMetalPipelineRecord& record, std::condition_variable **condOut) {
    
    __block std::condition_variable *newCondition = new std::condition_variable();
    fConditionMap[record] = newCondition;
    if(condOut) {
        *condOut = newCondition;
    }
    
    if (fNewPipelineStateMap[record] != NULL) {
        return fNewPipelineStateMap[record];
    }
    
    MTL::Library *library = fMetalDevice->newDefaultLibrary();
    
    std::shared_ptr<plMetalPipelineState> pipelineState = record.state;
    
    MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    descriptor->setLabel(pipelineState->GetDescription());
    
    const MTL::Function* vertexFunction = pipelineState->GetVertexFunction(library);
    const MTL::Function* fragmentFunction = pipelineState->GetFragmentFunction(library);
    descriptor->setVertexFunction(vertexFunction);
    descriptor->setFragmentFunction(fragmentFunction);
    
    descriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineState->ConfigureBlend(descriptor->colorAttachments()->object(0));
    
    MTL::VertexDescriptor *vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    pipelineState->ConfigureVertexDescriptor(vertexDescriptor);
    descriptor->setVertexDescriptor(vertexDescriptor);
    descriptor->setDepthAttachmentPixelFormat(record.depthFormat);
    descriptor->colorAttachments()->object(0)->setPixelFormat(record.colorFormat);
    
    descriptor->setSampleCount(record.sampleCount);
    
    NS::Error* error;
    fMetalDevice->newRenderPipelineState(descriptor, ^(MTL::RenderPipelineState *pipelineState, NS::Error *error){
        if (error) {
            //leave the condition in place for now, we don't want to
            //retry if the shader is defective. the condition will
            //prevent retries
            hsAssert(0, error->localizedDescription()->cString(NS::UTF8StringEncoding));
        } else {
            plMetalLinkedPipeline *linkedPipeline = new plMetalLinkedPipeline();
            linkedPipeline->pipelineState = pipelineState->retain();
            linkedPipeline->fragFunction = fragmentFunction;
            linkedPipeline->vertexFunction = vertexFunction;
            
            fNewPipelineStateMap[record] = linkedPipeline;
            //signal that we're done
            newCondition->notify_all();
        }
    });
    
    descriptor->release();
    library->release();
}

plMetalDevice::plMetalLinkedPipeline* plMetalDevice::PipelineState(plMetalPipelineState* pipelineState) {
    
    MTL::PixelFormat depthFormat = fCurrentDepthFormat;
    MTL::PixelFormat colorFormat = fCurrentFragmentOutputTexture->pixelFormat();
    
    plMetalPipelineRecord record = {
        depthFormat,
        colorFormat,
        CurrentTargetSampleCount()
    };
    
    record.state = std::shared_ptr<plMetalPipelineState>(pipelineState->Clone());
    
    plMetalLinkedPipeline* renderState = fNewPipelineStateMap[record];
    
    //if it exists, return it, we're done
    if(renderState) {
        return renderState;
    }
    
    //check and see if we're already building it. If so, wait.
    //Note: even if it already exists, this lock will be kept, and it will
    //let us through. This is to prevent race conditions where the render state
    //was null, but maybe in the time it took us to get here the state compiled.
    std::condition_variable *alreadyBuildingCondition = fConditionMap[record];
    if(alreadyBuildingCondition) {
        std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
        alreadyBuildingCondition->wait(lock);
        
        //should be returning the render state here, if not it failed to build
        //we'll allow the null return
        return fNewPipelineStateMap[record];
    }
    
    //it doesn't exist, start a build and wait
    //only render thread is allowed to start builds,
    //shouldn't be race conditions here
    StartPipelineBuild(record, &alreadyBuildingCondition);
    std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
    alreadyBuildingCondition->wait(lock);
    
    //should be returning the render state here, if not it failed to build
    //we'll allow the null return
    return fNewPipelineStateMap[record];
}

std::condition_variable* plMetalDevice::PrewarmPipelineStateFor(plMetalPipelineState* pipelineState)
{
    MTL::PixelFormat depthFormat = fCurrentDepthFormat;
    MTL::PixelFormat colorFormat = fCurrentFragmentOutputTexture->pixelFormat();
    
    plMetalPipelineRecord record = {
        depthFormat,
        colorFormat,
        CurrentTargetSampleCount()
    };
    
    record.state = std::shared_ptr<plMetalPipelineState>(pipelineState->Clone());
    //only render thread is allowed to prewarm, no race conditions around
    //fConditionMap creation
    if(!fNewPipelineStateMap[record] && fConditionMap[record]) {
        std::condition_variable *condOut;
        StartPipelineBuild(record, &condOut);
        return condOut;
    }
    return nullptr;
}

bool plMetalDevice::plMetalPipelineRecord::operator==(const plMetalPipelineRecord &p) const {
    return depthFormat == p.depthFormat &&
    colorFormat == p.colorFormat &&
    sampleCount == p.sampleCount &&
    state->operator==(*p.state);
}

MTL::CommandBuffer* plMetalDevice::GetCurrentCommandBuffer()
{
    if(fCurrentOffscreenCommandBuffer) {
        return fCurrentOffscreenCommandBuffer;
    }
    return fCurrentCommandBuffer;
}

void plMetalDevice::SubmitCommandBuffer()
{
    if (fBlitCommandEncoder) {
        fBlitCommandEncoder->endEncoding();
        fBlitCommandBuffer->commit();
        
        fBlitCommandBuffer->release();
        fBlitCommandEncoder->release();
        
        fBlitCommandBuffer = nullptr;
        fBlitCommandEncoder = nullptr;
    }
    
    fCurrentRenderTargetCommandEncoder->endEncoding();
    fCurrentRenderTargetCommandEncoder->release();
    fCurrentRenderTargetCommandEncoder = nil;
    
    if( NeedsPostprocessing() ) {
        PostprocessIntoDrawable();
    }
    
    fCurrentCommandBuffer->presentDrawable(fCurrentDrawable);
    fCurrentCommandBuffer->commit();
    //as we more tightly manage resource sync we may be able to avoid waiting for the frame to complete
    //fCurrentCommandBuffer->waitUntilCompleted();
    fCurrentCommandBuffer->release();
    fCurrentCommandBuffer = nil;
    
    fCurrentDrawable->release();
    fCurrentDrawable = nil;
    
    fClearRenderTargetColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    fClearDrawableColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    fShouldClearRenderTarget = false;
    fShouldClearDrawable = false;
    fClearRenderTargetDepth = 1.0;
    fClearDrawableDepth = 1.0;
}

void plMetalDevice::CreateGammaAdjustState() {
    MTL::RenderPipelineDescriptor *gammaDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    MTL::Library* library = fMetalDevice->newDefaultLibrary();
    
    gammaDescriptor->setVertexFunction(library->newFunction(NS::MakeConstantString("gammaCorrectVertex"))->autorelease());
    gammaDescriptor->setFragmentFunction(library->newFunction(NS::MakeConstantString("gammaCorrectFragment"))->autorelease());
    
    library->release();
    
    gammaDescriptor->colorAttachments()->object(0)->setPixelFormat(fFramebufferFormat);
    
    NS::Error *error;
    fGammaAdjustState->release();
    fGammaAdjustState = fMetalDevice->newRenderPipelineState(gammaDescriptor, &error);
}

void plMetalDevice::PostprocessIntoDrawable() {
    
    if (!fGammaAdjustState) {
        CreateGammaAdjustState();
    }
    
    //Gamma adjust
    MTL::RenderPassDescriptor* gammaPassDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
    gammaPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionDontCare);
    gammaPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentDrawable->texture());
    gammaPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
    
    MTL::RenderCommandEncoder* gammaAdjustEncoder = fCurrentCommandBuffer->renderCommandEncoder(gammaPassDescriptor);
    
    gammaAdjustEncoder->setRenderPipelineState(fGammaAdjustState);
    
    static const float fullFrameCoords[16] = {
        //first pair is vertex, second pair is texture
        -1, -1, 0, 1,
        1, -1, 1, 1,
        -1, 1, 0, 0,
        1, 1, 1, 0
    };
    gammaAdjustEncoder->setVertexBytes(&fullFrameCoords, sizeof(fullFrameCoords), 0);
    gammaAdjustEncoder->setFragmentTexture(fCurrentUnprocessedOutputTexture, 0);
    gammaAdjustEncoder->setFragmentTexture(fGammaLUTTexture, 1);
    gammaAdjustEncoder->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
    gammaAdjustEncoder->endEncoding();
}

std::size_t plMetalDevice::plMetalPipelineRecordHashFunction ::operator()(plMetalPipelineRecord const& s) const noexcept
{
    std::size_t value = std::hash<MTL::PixelFormat>()(s.depthFormat);
    value ^= std::hash<MTL::PixelFormat>()(s.colorFormat);
    value ^= std::hash<plMetalPipelineState>()(*s.state);
    value ^= std::hash<NS::UInteger>()(s.sampleCount);
    return value;
}

MTL::RenderCommandEncoder* plMetalDevice::CurrentRenderCommandEncoder()
{
    //return the current render command encoder
    //if a framebuffer wasn't set, assume screen, emulating GL
    if(fCurrentRenderTargetCommandEncoder) {
        return fCurrentRenderTargetCommandEncoder;
    }
    
    if (!fCurrentRenderTargetCommandEncoder) {
        BeginNewRenderPass();
        
        if (fCurrentRenderTarget) {
            fClearRenderTargetColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            fShouldClearRenderTarget = false;
            fClearRenderTargetDepth = 1.0;
        } else {
            fClearDrawableColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
            fShouldClearDrawable = false;
            fClearDrawableDepth = 1.0;
        }
    }
    
    return fCurrentRenderTargetCommandEncoder;
}

CA::MetalDrawable* plMetalDevice::GetCurrentDrawable()
{
    return fCurrentDrawable;
}

void plMetalDevice::BlitTexture(MTL::Texture* src, MTL::Texture* dst)
{
    if (fBlitCommandEncoder == nullptr) {
        fBlitCommandBuffer = fCommandQueue->commandBuffer()->retain();
        //enqueue so we go to the front of the line before render
        fBlitCommandBuffer->enqueue();
        fBlitCommandEncoder = fBlitCommandBuffer->blitCommandEncoder()->retain();
    }
    
    fBlitCommandEncoder->copyFromTexture(src, dst);
}

#endif
