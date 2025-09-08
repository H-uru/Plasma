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

// We need to define these once and only one for Metal somewhere
// in a cpp file before the Metal-cpp include (via plMetalDevice)
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "plMetalDevice.h"

#include "hsDarwin.h"
#include "hsDebug.h"
#include "hsThread.h"

#include "plDrawable/plGBufferGroup.h"
#include "plGImage/hsCodecManager.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plPipeline/plRenderTarget.h"

#include "pfMetalPipeline/plMetalPipeline.h"
#include "pfMetalPipeline/plMetalPipelineState.h"
#include "pfMetalPipeline/ShaderSrc/ShaderTypes.h"

/// Macros for getting/setting data in a vertex buffer
template<typename T>
static inline void inlCopy(uint8_t*& src, uint8_t*& dst)
{
    T* src_ptr = reinterpret_cast<T*>(src);
    T* dst_ptr = reinterpret_cast<T*>(dst);
    *dst_ptr = *src_ptr;
    src += sizeof(T);
    dst += sizeof(T);
}

static inline void inlCopy(const uint8_t*& src, uint8_t*& dst, size_t sz)
{
    memcpy(dst, src, sz);
    src += sz;
    dst += sz;
}

template<typename T>
static inline const uint8_t* inlExtract(const uint8_t* src, T* val)
{
    const T* ptr = reinterpret_cast<const T*>(src);
    *val = *ptr++;
    return reinterpret_cast<const uint8_t*>(ptr);
}

template<>
inline const uint8_t* inlExtract<hsPoint3>(const uint8_t* src, hsPoint3* val)
{
    const float* src_ptr = reinterpret_cast<const float*>(src);
    float* dst_ptr = reinterpret_cast<float*>(val);
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr = 1.f;
    return reinterpret_cast<const uint8_t*>(src_ptr);
}

template<>
inline const uint8_t* inlExtract<hsVector3>(const uint8_t* src, hsVector3* val)
{
    const float* src_ptr = reinterpret_cast<const float*>(src);
    float* dst_ptr = reinterpret_cast<float*>(val);
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr++ = *src_ptr++;
    *dst_ptr = 0.f;
    return reinterpret_cast<const uint8_t*>(src_ptr);
}

template<typename T, size_t N>
static inline void inlSkip(uint8_t*& src)
{
    src += sizeof(T) * N;
}

template<typename T>
static inline uint8_t* inlStuff(uint8_t* dst, const T* val)
{
    T* ptr = reinterpret_cast<T*>(dst);
    *ptr++ = *val;
    return reinterpret_cast<uint8_t*>(ptr);
}

bool plMetalDevice::InitDevice()
{
    fCommandQueue = fMetalDevice->newCommandQueue();

    // Only known tiler on Apple devices are Apple GPUs.
    // Apple recommends a family check for tile memory support.
    fSupportsTileMemory = fMetalDevice->supportsFamily(MTL::GPUFamilyApple1);
    
    // Only known tiler on Apple devices are Apple GPUs.
    // Apple recommends a family check for tile memory support.
    fSupportsTileMemory = fMetalDevice->supportsFamily(MTL::GPUFamilyApple1);
    fSupportsDXTTextures = fMetalDevice->supportsBCTextureCompression();
    
    if (!fSupportsDXTTextures) {
        hsDebugPrintToTerminal(ST::format("Render device \"%s\" does not support DXT textures. Falling back on software decompression. Performance will be slower.", fMetalDevice->name()->cString(NS::StringEncoding::UTF8StringEncoding)));
    }

    // set up all the depth stencil states
    MTL::DepthStencilDescriptor* depthDescriptor = MTL::DepthStencilDescriptor::alloc()->init();

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

    LoadLibrary();
}

void plMetalDevice::Shutdown()
{
    // FIXME: Should Metal adopt Shutdown like OGL?
    hsAssert(0, "Shutdown not implemented for Metal rendering");
}

void plMetalDevice::SetMaxAnsiotropy(uint8_t maxAnsiotropy)
{
    // setup the material pass samplers
    // load them all at once and then let the shader pick

    if (maxAnsiotropy == 0)
        maxAnsiotropy = 1;

    if (fSamplerStates[0] != nullptr) {
        ReleaseSamplerStates();
    }

    MTL::SamplerDescriptor* samplerDescriptor = MTL::SamplerDescriptor::alloc()->init();
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
    samplerDescriptor->release();
}

void plMetalDevice::SetMSAASampleCount(uint8_t sampleCount)
{
    while (sampleCount != 1) {
        if (fMetalDevice->supportsTextureSampleCount(sampleCount)) {
            break;
        }
        sampleCount--;
    }

    fSampleCount = sampleCount;
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

void plMetalDevice::Clear(bool shouldClearColor, simd_float4 clearColor, bool shouldClearDepth, float clearDepth)
{
    /*
     In Metal, a clear is an argument to the drawable loading operation,
     not an operation that can be done freely at any time. So lets handle
     a clear two ways:
     1) If we're in the middle of a rendering pass, manually clear.
     2) If we're at the begining of a render pass, note the clear color
     we should use to clear the framebuffer at load.
     */

    if (fCurrentRenderTargetCommandEncoder) {
        // We're mid flight, we'll need to manually paint the clear color

        half4 clearColor;
        clearColor[0] = clearColor.r;
        clearColor[1] = clearColor.g;
        clearColor[2] = clearColor.b;
        clearColor[3] = clearColor.a;
        plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalClearPipelineState(this, shouldClearColor, shouldClearDepth).GetRenderPipelineState();

        const MTL::RenderPipelineState* pipelineState = linkedPipeline->pipelineState;
        CurrentRenderCommandEncoder()->setRenderPipelineState(pipelineState);

        float clearCoords[8] = {
            -1, -1,
            1, -1,
            -1, 1,
            1, 1};
        float clearDepth = 1.0f;
        CurrentRenderCommandEncoder()->setDepthStencilState(fNoZReadStencilState);

        CurrentRenderCommandEncoder()->setCullMode(MTL::CullModeNone);
        CurrentRenderCommandEncoder()->setVertexBytes(&clearCoords, sizeof(clearCoords), 0);
        CurrentRenderCommandEncoder()->setFragmentBytes(&clearColor, sizeof(clearColor), 0);
        CurrentRenderCommandEncoder()->setFragmentBytes(&clearDepth, sizeof(float), 1);
        CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
    } else {
        // Render has not started yet! Note which clear color we should use
        // for clearing the render buffer when we load it.

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
        
        /* 
         Clear needs to count as a render operation, but Metal treats
         it as an argument when starting a new render encoder. If a
         render pass only cleared, but  never rendered any content,
         the clear would never happen because no render encoder would
         be created.
         
         Force render encoder creation to force the clear to happen.
         */
        
        CurrentRenderCommandEncoder();
    }
}

void plMetalDevice::BeginNewRenderPass()
{
    // lazily create the screen render encoder if it does not yet exist
    if (!fCurrentOffscreenCommandBuffer && !fCurrentRenderTargetCommandEncoder) {
        SetRenderTarget(nullptr);
    }

    if (fCurrentRenderTargetCommandEncoder) {
        // if we have an existing render target, submit it's commands and release it
        // if we need to come back to this render target, we can always create a new render
        // pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nullptr;
    }

    MTL::RenderPassDescriptor* renderPassDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
    renderPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);

    if (fCurrentRenderTarget) {
        renderPassDescriptor->colorAttachments()->object(0)->setClearColor(MTL::ClearColor(fClearRenderTargetColor.x, fClearRenderTargetColor.y, fClearRenderTargetColor.z, fClearRenderTargetColor.w));
        if (fShouldClearRenderTarget) {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionClear);
        } else {
            renderPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionLoad);
        }

        if (fCurrentRenderTarget->GetZDepth()) {
            plMetalRenderTargetRef* deviceTarget = (plMetalRenderTargetRef*)fCurrentRenderTarget->GetDeviceRef();
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
            // We only need the intermediate texture for post processing on
            // non-tilers. Tilers can direct read/write on the fragment texture.
            if (NeedsPostprocessing() && !SupportsTileMemory()) {
                renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentUnprocessedOutputTexture);
            } else {
                renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentOutputTexture);
            }
        } else {
            renderPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentFragmentMSAAOutputTexture);

            // if we need postprocessing, output to the intermediate main pass texture
            // otherwise we can go straight to the drawable
            
            // We only need the intermediate texture for post processing on
            // non-tilers. Tilers can direct read/write on the fragment texture.
            if (NeedsPostprocessing() && !SupportsTileMemory()) {
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
    if ((!fCurrentRenderTarget && !target) && fCurrentRenderTargetCommandEncoder) {
        return;
    }
    if (fCurrentRenderTargetCommandEncoder) {
        // if we have an existing render target, submit it's commands and release it
        // if we need to come back to this render target, we can always create a new render
        // pass descriptor and submit more commands
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nullptr;
    }

    if (fCurrentOffscreenCommandBuffer) {
        if (fCurrentRenderTarget && fCurrentRenderTarget->GetFlags() & plRenderTarget::kIsOffscreen) {
            // if our target was offscreen, go ahead and blit back. Something will want this data.
            MTL::BlitCommandEncoder* blitEncoder = fCurrentOffscreenCommandBuffer->blitCommandEncoder();
            blitEncoder->synchronizeResource(fCurrentFragmentOutputTexture);
            blitEncoder->endEncoding();
        }
        fCurrentOffscreenCommandBuffer->commit();
        if (fCurrentRenderTarget && fCurrentRenderTarget->GetFlags() & plRenderTarget::kIsOffscreen) {
            // if it's an offscreen buffer, wait for completion
            // something is probably going to want to syncronously grab data
            fCurrentOffscreenCommandBuffer->waitUntilCompleted();
        }
        fCurrentOffscreenCommandBuffer->release();
        fCurrentOffscreenCommandBuffer = nullptr;
    }

    fCurrentRenderTarget = target;

    if (fCurrentRenderTarget && fShouldClearRenderTarget == false) {
        // clear if a clear color wasn't already set
        fClearRenderTargetColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
        fShouldClearRenderTarget = true;
        fClearRenderTargetDepth = 1.0;
    }

    if (fCurrentRenderTarget) {
        if (!target->GetDeviceRef()) {
            fPipeline->MakeRenderTargetRef(target);
        }
        plMetalRenderTargetRef* deviceTarget = (plMetalRenderTargetRef*)target->GetDeviceRef();
        fCurrentOffscreenCommandBuffer = fCommandQueue->commandBuffer();
        fCurrentOffscreenCommandBuffer->retain();
        fCurrentFragmentOutputTexture = deviceTarget->fTexture;

        if (deviceTarget->fDepthBuffer) {
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
    : fErrorMsg(),
      fActiveThread(hsThread::ThisThreadHash()),
      fCurrentDrawable(),
      fCommandQueue(),
      fCurrentRenderTargetCommandEncoder(),
      fCurrentDrawableDepthTexture(),
      fCurrentFragmentOutputTexture(),
      fCurrentCommandBuffer(),
      fCurrentOffscreenCommandBuffer(),
      fCurrentRenderTarget(),
      fNewPipelineStateMap(),
      fCurrentFragmentMSAAOutputTexture(),
      fCurrentUnprocessedOutputTexture(),
      fGammaLUTTexture(),
      fGammaAdjustState(),
      fBlitCommandBuffer(),
      fBlitCommandEncoder()
{
    fClearRenderTargetColor = {0.0, 0.0, 0.0, 1.0};
    fClearDrawableColor = {0.0, 0.0, 0.0, 1.0};
    fSamplerStates[0] = nullptr;
}

void plMetalDevice::SetViewport()
{
    CurrentRenderCommandEncoder()->setViewport({(double)fPipeline->GetViewTransform().GetViewPortLeft(),
                                                (double)fPipeline->GetViewTransform().GetViewPortTop(),
                                                (double)fPipeline->GetViewTransform().GetViewPortWidth(),
                                                (double)fPipeline->GetViewTransform().GetViewPortHeight(),
                                                0.f, 1.f});
}

bool plMetalDevice::BeginRender()
{
    if (fActiveThread == hsThread::ThisThreadHash()) {
        return true;
    }

    fActiveThread = hsThread::ThisThreadHash();

    return true;
}

static uint32_t IGetBufferFormatSize(uint8_t format)
{
    uint32_t size = sizeof(hsPoint3) * 2 + sizeof(hsColor32) * 2; // Position and normal, and two packed colors

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

void plMetalDevice::SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, plMetalDevice::VertexBufferRef* vRef)
{
    uint8_t format = owner->GetVertexFormat();

    if (format & plGBufferGroup::kSkinIndices) {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights; // Should do nothing, but just in case...
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

void plMetalDevice::CheckStaticVertexBuffer(plMetalDevice::VertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");

    if (!vRef->GetBuffer()) {
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

    if (ref->GetBuffer()) {
        hsAssert(size <= ref->GetBuffer()->length(), "Allocated buffer does not fit fill data");
    }

    if (!size) {
        return;
    }

    MTL::Buffer* metalBuffer = fMetalDevice->newBuffer(size, GetDefaultStorageMode());
    ref->SetBuffer(metalBuffer);
    uint8_t* buffer = (uint8_t*)ref->GetBuffer()->contents();

    if (ref->fData) {
        memcpy(buffer, ref->fData + vertStart, size);
    } else {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");

        uint8_t* ptr = buffer;

        const uint32_t        vertSmallSize = group->GetVertexLiteStride() - sizeof(hsPoint3) * 2;
        uint8_t*              srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData(idx);

        const size_t numCells = group->GetNumCells(idx);
        for (size_t i = 0; i < numCells; i++) {
            plGBufferCell* cell = group->GetCell(idx, i);

            if (cell->fColorStart == uint32_t(-1)) {
                /// Interleaved, do straight copy
                memcpy(ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize);
                ptr += cell->fLength * vertSize;
                hsAssert(size <= cell->fLength * vertSize, "Interleaved copy size mismatch");
            } else {
                hsStatusMessage("Non interleaved data");

                /// Separated, gotta interleave
                uint8_t*        tempVPtr = srcVPtr + cell->fVtxStart;
                plGBufferColor* tempCPtr = srcCPtr + cell->fColorStart;
                int             j;
                for (j = 0; j < cell->fLength; j++) {
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

        hsAssert((ptr - buffer) == size, "Didn't fill the buffer?");
    }

    metalBuffer->release();

    /// Unlock and clean up
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plMetalDevice::FillVolatileVertexBufferRef(plMetalDevice::VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    size_t  uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(hsPoint3);
    uint8_t numWeights = (group->GetVertexFormat() & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < ref->fCount; ++i) {
        inlCopy<hsPoint3>(src, dst); // pre-pos

        src += numWeights * sizeof(float); // weights

        if (group->GetVertexFormat() & plGBufferGroup::kSkinIndices)
            inlSkip<uint32_t, 1>(src); // indices
        
        inlCopy<hsVector3>(src, dst); // pre-normal
        inlCopy<uint32_t>(src, dst); // diffuse
        inlCopy<uint32_t>(src, dst); // specular

        // UVWs
        memcpy(dst, src, uvChanSize);
        src += uvChanSize;
        dst += uvChanSize;
    }
}

void plMetalDevice::SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, plMetalDevice::IndexBufferRef* iRef)
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

void plMetalDevice::CheckIndexBuffer(plMetalDevice::IndexBufferRef* iRef)
{
    if (!iRef->GetBuffer() && iRef->fCount) {
        iRef->SetVolatile(false);

        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

void plMetalDevice::FillIndexBufferRef(plMetalDevice::IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t fullSize = owner->GetIndexBufferCount(idx) * sizeof(uint16_t);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);

    if (!size) {
        return;
    }

    iRef->PrepareForWrite();
    MTL::Buffer* indexBuffer = iRef->GetBuffer();
    if (!indexBuffer || indexBuffer->length() < fullSize) {
        indexBuffer = fMetalDevice->newBuffer(fullSize, GetDefaultStorageMode());
        iRef->SetBuffer(indexBuffer);
        indexBuffer->release();
    }

    memcpy(((uint16_t*)indexBuffer->contents()) + startIdx, owner->GetIndexBufferData(idx) + startIdx, size);
    if (indexBuffer->storageMode() == MTL::StorageModeManaged) {
        indexBuffer->didModifyRange(NS::Range(startIdx, size));
    }

    iRef->SetDirty(false);
}

void plMetalDevice::SetupTextureRef(plBitmap* img, plMetalDevice::TextureRef* tRef)
{
    tRef->fOwner = img;

    plBitmap* imageToCheck = img;

    // if it's a cubic texture, check the first face. The root img will give a false format that will cause us to decode wrong.
    plCubicEnvironmap* cubicImg = dynamic_cast<plCubicEnvironmap*>(img);
    if (cubicImg) {
        imageToCheck = cubicImg->GetFace(0);
    }

    if (imageToCheck->IsCompressed()) {
        if (fSupportsDXTTextures) {
            switch (imageToCheck->fDirectXInfo.fCompressionType) {
                case plBitmap::DirectXInfo::kDXT1:
                    tRef->fFormat = MTL::PixelFormatBC1_RGBA;
                    break;
                case plBitmap::DirectXInfo::kDXT5:
                    tRef->fFormat = MTL::PixelFormatBC3_RGBA;
                    break;
            }
        } else {
            tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
        }
    } else {
        switch (imageToCheck->fUncompressedInfo.fType) {
            case plBitmap::UncompressedInfo::kRGB8888:
                tRef->fFormat = MTL::PixelFormatBGRA8Unorm;
                break;
            case plBitmap::UncompressedInfo::kRGB4444:
                // we'll convert this on load to 8 bits per channel
                // Metal doesn't support 4 bits per channel on all hardware
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
    fCurrentFragmentOutputTexture = nullptr;

    if (fGammaAdjustState)
        fGammaAdjustState->release();
    fGammaAdjustState = nullptr;
}

void plMetalDevice::SetFramebufferFormat(MTL::PixelFormat format)
{
    if (fFramebufferFormat != format) {
        ReleaseFramebufferObjects();
        fFramebufferFormat = format;
    }
}

void plMetalDevice::CheckTexture(plMetalDevice::TextureRef* tRef)
{
    if (!tRef->fTexture) {
        tRef->SetDirty(true);
    }
}

uint plMetalDevice::ConfigureAllowedLevels(plMetalDevice::TextureRef* tRef, plMipmap* mipmap)
{
    if (mipmap->IsCompressed()) {
        mipmap->SetCurrLevel(tRef->fLevels);
        while ((mipmap->GetCurrWidth() | mipmap->GetCurrHeight()) & 0x03) {
            tRef->fLevels--;
            hsAssert(tRef->fLevels >= 0, "How was this ever compressed?");
            if (tRef->fLevels < 0) {
                tRef->fLevels = -1;
                break;
            }
            mipmap->SetCurrLevel(tRef->fLevels);
        }
    }
}

void plMetalDevice::PopulateTexture(plMetalDevice::TextureRef* tRef, plMipmap* img, uint slice)
{
    if (img->IsCompressed() && fSupportsDXTTextures) {
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
        NS::UInteger width = tRef->fTexture->width();
        NS::UInteger height = tRef->fTexture->height();
#endif

        if (tRef->fLevels == -1) {
            hsAssert(1, "Bad texture found");
            return;
        }

        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);
#if HACK_LEVEL_SIZE
            NS::UInteger levelWidth = (width / exp2(lvl));
            NS::UInteger levelHeight = (height / exp2(lvl));
#else
            NS::UInteger levelWidth = img->GetCurrWidth();
            NS::UInteger levelHeight = img->GetCurrHeight();
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
        if (img->IsCompressed()) {
            img = hsCodecManager::Instance().CreateUncompressedMipmap(img, 8);
        } else {
            // hsCodecManager returns a new strong reference
            img->Ref();
        }
        
        for (int lvl = 0; lvl <= tRef->fLevels; lvl++) {
            img->SetCurrLevel(lvl);

            if (img->GetCurrLevelPtr()) {
                if (img->fUncompressedInfo.fType == plBitmap::UncompressedInfo::kRGB4444) {
                    struct RGBA4444Component
                    {
                        unsigned r : 4;
                        unsigned g : 4;
                        unsigned b : 4;
                        unsigned a : 4;
                    };

                    RGBA4444Component* in = (RGBA4444Component*)img->GetCurrLevelPtr();
                    auto out = std::make_unique<simd_uint4[]>(img->GetCurrHeight() * img->GetCurrWidth());

                    for (int i = 0; i < (img->GetCurrWidth() * img->GetCurrHeight()); i++) {
                        out[i].r = in[i].r;
                        out[i].g = in[i].g;
                        out[i].b = in[i].b;
                        out[i].a = in[i].a;
                    }

                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, out.get(), img->GetCurrWidth() * 4, 0);
                } else {
                    tRef->fTexture->replaceRegion(MTL::Region::Make2D(0, 0, img->GetCurrWidth(), img->GetCurrHeight()), img->GetCurrLevel(), slice, img->GetCurrLevelPtr(), img->GetCurrWidth() * 4, 0);
                }
            } else {
                hsAssert(0, "Texture with no image data?\n");
            }
        }
        
        img->UnRef();
    }
    
    CFStringRef name = CFStringCreateWithSTString(img->GetKeyName());
    tRef->fTexture->setLabel(reinterpret_cast<const NS::String *>(name));
    CFRelease(name);
    tRef->SetDirty(false);
}

void plMetalDevice::MakeTextureRef(plMetalDevice::TextureRef* tRef, plMipmap* img)
{
    if (!img->GetImage()) {
        return;
    }

    if (tRef->fTexture) {
        tRef->fTexture->release();
    }

    tRef->fLevels = img->GetNumLevels() - 1;
    // FIXME: Is this texture check actually needed
    // if(!tRef->fTexture) {
    ConfigureAllowedLevels(tRef, img);

    bool textureIsValid = tRef->fLevels > 0;

    // texture doesn't exist yet, create it
    bool                    supportsMipMap = tRef->fLevels && textureIsValid;
    MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::texture2DDescriptor(tRef->fFormat, img->GetWidth(), img->GetHeight(), supportsMipMap);
    descriptor->setUsage(MTL::TextureUsageShaderRead);

    // Metal gets mad if we set this with 0, only set it if we know there are mipmaps
    if (supportsMipMap) {
        descriptor->setMipmapLevelCount(tRef->fLevels + 1);
    }

    descriptor->setStorageMode(GetDefaultStorageMode());

    tRef->fTexture = fMetalDevice->newTexture(descriptor);
    PopulateTexture(tRef, img, 0);
    //}

    tRef->SetDirty(false);
}

void plMetalDevice::MakeCubicTextureRef(plMetalDevice::TextureRef* tRef, plCubicEnvironmap* img)
{
    MTL::TextureDescriptor* descriptor = MTL::TextureDescriptor::textureCubeDescriptor(tRef->fFormat, img->GetFace(0)->GetWidth(), tRef->fLevels != 0);

    if (tRef->fLevels != 0) {
        descriptor->setMipmapLevelCount(tRef->fLevels + 1);
    }
    descriptor->setUsage(MTL::TextureUsageShaderRead);

    tRef->fTexture = fMetalDevice->newTexture(descriptor);

    static constexpr uint kFaceMapping[] = {
        1, // kLeftFace
        0, // kRightFace
        4, // kFrontFace
        5, // kBackFace
        2, // kTopFace
        3  // kBottomFace
    };
    for (size_t i = 0; i < 6; i++) {
        PopulateTexture(tRef, img->GetFace(i), kFaceMapping[i]);
    }

    tRef->SetDirty(false);
}

void plMetalDevice::SetProjectionMatrix(const hsMatrix44& src)
{
    fMatrixProj = hsMatrix2SIMD(src);
}

void plMetalDevice::SetWorldToCameraMatrix(const hsMatrix44& src)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);

    fMatrixW2C = hsMatrix2SIMD(src);
    fMatrixC2W = hsMatrix2SIMD(inv);
}

void plMetalDevice::SetLocalToWorldMatrix(const hsMatrix44& src)
{
    hsMatrix44 inv;
    src.GetInverse(&inv);

    fMatrixL2W = hsMatrix2SIMD(src);
    fMatrixW2L = hsMatrix2SIMD(inv);
}

void plMetalDevice::CreateNewCommandBuffer(CA::MetalDrawable* drawable)
{
    fCurrentCommandBuffer = fCommandQueue->commandBuffer();
    fCurrentCommandBuffer->retain();

    SetFramebufferFormat(drawable->texture()->pixelFormat());

    bool depthNeedsRebuild = fCurrentDrawableDepthTexture == nullptr;
    depthNeedsRebuild |= drawable->texture()->width() != fCurrentDrawableDepthTexture->width() || drawable->texture()->height() != fCurrentDrawableDepthTexture->height();

    // cache the depth buffer, we'll just clear it every time.
    if (depthNeedsRebuild) {
        if (fCurrentDrawableDepthTexture) {
            fCurrentDrawableDepthTexture->release();
            fCurrentFragmentMSAAOutputTexture->release();
        }

        MTL::TextureDescriptor* depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float_Stencil8,
                                                                                                     drawable->texture()->width(),
                                                                                                     drawable->texture()->height(),
                                                                                                     false);
        if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
            depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
        } else {
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
        }
        depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);

        if (fSampleCount != 1) {
            // MSSA depth and color output
            depthTextureDescriptor->setSampleCount(fSampleCount);
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            depthTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
            if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
                depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
            } else {
                depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            }
            fCurrentDrawableDepthTexture = fMetalDevice->newTexture(depthTextureDescriptor);

            MTL::TextureDescriptor* msaaColorTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(drawable->texture()->pixelFormat(),
                                                                                                             drawable->texture()->width(),
                                                                                                             drawable->texture()->height(),
                                                                                                             false);
            msaaColorTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
            if (fMetalDevice->supportsFamily(MTL::GPUFamilyApple1) && fSampleCount == 1) {
                msaaColorTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
            } else {
                msaaColorTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            }
            msaaColorTextureDescriptor->setTextureType(MTL::TextureType2DMultisample);
            msaaColorTextureDescriptor->setSampleCount(fSampleCount);
            fCurrentFragmentMSAAOutputTexture = fMetalDevice->newTexture(msaaColorTextureDescriptor);
        } else {
            fCurrentDrawableDepthTexture = fMetalDevice->newTexture(depthTextureDescriptor);
        }
    }
    
    // We only need to allocate an intermediate texture if we don't have tile memory.
    if (!SupportsTileMemory()) {
        // Do we need to create a unprocessed output texture?
        // If the depth needs to be rebuilt - we probably need to rebuild this one too
        if ((fCurrentUnprocessedOutputTexture && depthNeedsRebuild) || (fCurrentUnprocessedOutputTexture == nullptr && NeedsPostprocessing())) {
            MTL::TextureDescriptor* mainPassDescriptor = MTL::TextureDescriptor::texture2DDescriptor(drawable->texture()->pixelFormat(), drawable->texture()->width(), drawable->texture()->height(), false);
            mainPassDescriptor->setStorageMode(MTL::StorageModePrivate);
            mainPassDescriptor->setUsage(MTL::TextureUsageShaderRead | MTL::TextureUsageRenderTarget);
            fCurrentUnprocessedOutputTexture->release();
            fCurrentUnprocessedOutputTexture = fMetalDevice->newTexture(mainPassDescriptor);
        }
    }

    fCurrentDrawable = drawable->retain();
}

void plMetalDevice::StartPipelineBuild(plMetalPipelineRecord& record, std::condition_variable** condOut)
{
    fConditionMap[record] = new std::condition_variable();
    if (condOut) {
        *condOut = fConditionMap[record];
    }

    if (fNewPipelineStateMap[record] != nullptr) {
        // The shader is already compiled.
        return;
    }

    std::shared_ptr<plMetalPipelineState> pipelineState = record.state;

    MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    descriptor->setLabel(pipelineState->GetDescription());

    const MTL::Function* vertexFunction = pipelineState->GetVertexFunction(fShaderLibrary);
    const MTL::Function* fragmentFunction = pipelineState->GetFragmentFunction(fShaderLibrary);
    descriptor->setVertexFunction(vertexFunction);
    descriptor->setFragmentFunction(fragmentFunction);

    descriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
    pipelineState->ConfigureBlend(descriptor->colorAttachments()->object(0));

    MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
    pipelineState->ConfigureVertexDescriptor(vertexDescriptor);
    descriptor->setVertexDescriptor(vertexDescriptor);
    descriptor->setDepthAttachmentPixelFormat(record.depthFormat);
    descriptor->colorAttachments()->object(0)->setPixelFormat(record.colorFormat);

    descriptor->setSampleCount(record.sampleCount);

    NS::Error* error;
    fMetalDevice->newRenderPipelineState(descriptor, ^(MTL::RenderPipelineState* pipelineState, NS::Error* error) {
        if (error) {
            // leave the condition in place for now, we don't want to
            // retry if the shader is defective. the condition will
            // prevent retries
            hsAssert(0, error->localizedDescription()->cString(NS::UTF8StringEncoding));
        } else {
            plMetalLinkedPipeline* linkedPipeline = new plMetalLinkedPipeline();
            linkedPipeline->pipelineState = pipelineState->retain();
            linkedPipeline->fragFunction = fragmentFunction;
            linkedPipeline->vertexFunction = vertexFunction;

            fNewPipelineStateMap[record] = linkedPipeline;
            // signal that we're done
            fConditionMap[record]->notify_all();
        }
    });

    descriptor->release();
}

plMetalDevice::plMetalLinkedPipeline* plMetalDevice::PipelineState(plMetalPipelineState* pipelineState)
{
    MTL::PixelFormat depthFormat = fCurrentDepthFormat;
    MTL::PixelFormat colorFormat = fCurrentFragmentOutputTexture->pixelFormat();

    plMetalPipelineRecord record = {
        depthFormat,
        colorFormat,
        CurrentTargetSampleCount()};

    record.state = std::shared_ptr<plMetalPipelineState>(pipelineState->Clone());

    plMetalLinkedPipeline* renderState = fNewPipelineStateMap[record];

    // if it exists, return it, we're done
    if (renderState) {
        return renderState;
    }

    // check and see if we're already building it. If so, wait.
    // Note: even if it already exists, this lock will be kept, and it will
    // let us through. This is to prevent race conditions where the render state
    // was null, but maybe in the time it took us to get here the state compiled.
    std::condition_variable* alreadyBuildingCondition = fConditionMap[record];
    if (alreadyBuildingCondition) {
        std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
        alreadyBuildingCondition->wait(lock);

        // should be returning the render state here, if not it failed to build
        // we'll allow the null return
        return fNewPipelineStateMap[record];
    }

    // it doesn't exist, start a build and wait
    // only render thread is allowed to start builds,
    // shouldn't be race conditions here
    StartPipelineBuild(record, &alreadyBuildingCondition);
    std::unique_lock<std::mutex> lock(fPipelineCreationMtx);
    alreadyBuildingCondition->wait(lock);

    // should be returning the render state here, if not it failed to build
    // we'll allow the null return
    return fNewPipelineStateMap[record];
}

std::condition_variable* plMetalDevice::PrewarmPipelineStateFor(plMetalPipelineState* pipelineState)
{
    MTL::PixelFormat depthFormat = fCurrentDepthFormat;
    MTL::PixelFormat colorFormat = fCurrentFragmentOutputTexture->pixelFormat();

    plMetalPipelineRecord record = {
        depthFormat,
        colorFormat,
        CurrentTargetSampleCount()};

    record.state = std::shared_ptr<plMetalPipelineState>(pipelineState->Clone());
    // only render thread is allowed to prewarm, no race conditions around
    // fConditionMap creation
    if (!fNewPipelineStateMap[record] && fConditionMap[record]) {
        std::condition_variable* condOut;
        StartPipelineBuild(record, &condOut);
        return condOut;
    }
    return nullptr;
}

bool plMetalDevice::plMetalPipelineRecord::operator==(const plMetalPipelineRecord& p) const
{
    return depthFormat == p.depthFormat &&
           colorFormat == p.colorFormat &&
           sampleCount == p.sampleCount &&
           state->operator==(*p.state);
}

MTL::CommandBuffer* plMetalDevice::GetCurrentCommandBuffer() const
{
    if (fCurrentOffscreenCommandBuffer) {
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


    // Post processing will end the main render pass.
    // On Apple Silicon - this code will attempt to combine render passes,
    // but past this point developer should not rely on the main render pass
    // being available.
    PreparePostProcessing();
    PostprocessIntoDrawable();
    FinalizePostProcessing();

    fCurrentCommandBuffer->presentDrawable(fCurrentDrawable);
    fCurrentCommandBuffer->commit();
    fCurrentCommandBuffer->release();
    fCurrentCommandBuffer = nullptr;

    fCurrentDrawable->release();
    fCurrentDrawable = nullptr;

    // Reset the clear colors for the next pass
    // Metal clears on framebuffer load - so don't cause a clear
    // command in this pass to affect the next pass.
    fClearRenderTargetColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    fClearDrawableColor = simd_make_float4(0.0f, 0.0f, 0.0f, 1.0f);
    fShouldClearRenderTarget = false;
    fShouldClearDrawable = false;
    fClearRenderTargetDepth = 1.0;
    fClearDrawableDepth = 1.0;
}

MTL::SamplerState* plMetalDevice::SampleStateForClampFlags(hsGMatState::hsGMatClampFlags sampleState) const
{
    return fSamplerStates[sampleState];
}

void plMetalDevice::CreateGammaAdjustState()
{
    MTL::RenderPipelineDescriptor* gammaDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();

    gammaDescriptor->setVertexFunction(fShaderLibrary->newFunction(MTLSTR("gammaCorrectVertex"))->autorelease());
    if (SupportsTileMemory()) {
        // Tiler GPU version does an in place transform
        // Because it's in place we need to describe all main pass buffers including depth and MSAA
        gammaDescriptor->colorAttachments()->object(0)->setPixelFormat(fCurrentFragmentOutputTexture->pixelFormat());
        gammaDescriptor->setDepthAttachmentPixelFormat(fCurrentDepthFormat);
        gammaDescriptor->setSampleCount(CurrentTargetSampleCount());
        gammaDescriptor->setFragmentFunction(fShaderLibrary->newFunction(MTLSTR("gammaCorrectFragmentInPlace"))->autorelease());
    } else {
        gammaDescriptor->colorAttachments()->object(0)->setPixelFormat(fFramebufferFormat);
        gammaDescriptor->setFragmentFunction(fShaderLibrary->newFunction(MTLSTR("gammaCorrectFragment"))->autorelease());
    }

    NS::Error* error;
    fGammaAdjustState->release();
    fGammaAdjustState = fMetalDevice->newRenderPipelineState(gammaDescriptor, &error);
    gammaDescriptor->release();
}

void plMetalDevice::PostprocessIntoDrawable()
{
    if (!NeedsPostprocessing()) {
        return;
    }
    
    if (!fGammaAdjustState) {
        CreateGammaAdjustState();
    }

    MTL::RenderCommandEncoder* gammaAdjustEncoder;
    if (SupportsTileMemory()) {
        // On tilers we can read/write directly on the framebuffer, carry on, no new render pass needed.
        gammaAdjustEncoder = CurrentRenderCommandEncoder();
    } else {
        // On non-tilers, we need to create a new render pass to use our old render target as a texture
        // source and the output drawable as the target to do post-processing.
        MTL::RenderPassDescriptor* gammaPassDescriptor = MTL::RenderPassDescriptor::renderPassDescriptor();
        gammaPassDescriptor->colorAttachments()->object(0)->setLoadAction(MTL::LoadActionDontCare);
        gammaPassDescriptor->colorAttachments()->object(0)->setTexture(fCurrentDrawable->texture());
        gammaPassDescriptor->colorAttachments()->object(0)->setStoreAction(MTL::StoreActionStore);
        
        gammaAdjustEncoder = fCurrentCommandBuffer->renderCommandEncoder(gammaPassDescriptor);
    }

    gammaAdjustEncoder->setRenderPipelineState(fGammaAdjustState);

    static const float fullFrameCoords[16] = {
        // first pair is vertex, second pair is texture
        -1, -1, 0, 1,
        1, -1, 1, 1,
        -1, 1, 0, 0,
        1, 1, 1, 0
    };
    gammaAdjustEncoder->setVertexBytes(&fullFrameCoords, sizeof(fullFrameCoords), 0);
    gammaAdjustEncoder->setFragmentTexture(fCurrentUnprocessedOutputTexture, 0);
    gammaAdjustEncoder->setFragmentTexture(fGammaLUTTexture, 1);
    gammaAdjustEncoder->drawPrimitives(MTL::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
    
    // On non-tilers - we created a render pass that we own,
    // and we're responsible for ending it
    if (!SupportsTileMemory()) {
        gammaAdjustEncoder->endEncoding();
    }
}

void plMetalDevice::PreparePostProcessing()
{
    // If we're on a tiler GPU - we don't need to create a new
    // render pass. Keep the main render pass alive.
    if (!SupportsTileMemory()) {
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
}

void plMetalDevice::FinalizePostProcessing()
{
    // If we were on a tiler, post processing took ownership of the main
    // render pass so we're responsible for finalizing it.
    if (SupportsTileMemory()) {
        fCurrentRenderTargetCommandEncoder->endEncoding();
        fCurrentRenderTargetCommandEncoder->release();
        fCurrentRenderTargetCommandEncoder = nil;
    }
}

size_t plMetalDevice::plMetalPipelineRecordHashFunction ::operator()(plMetalPipelineRecord const& s) const noexcept
{
    size_t value = std::hash<MTL::PixelFormat>()(s.depthFormat);
    value ^= std::hash<MTL::PixelFormat>()(s.colorFormat);
    value ^= std::hash<plMetalPipelineState>()(*s.state);
    value ^= std::hash<NS::UInteger>()(s.sampleCount);
    return value;
}

MTL::RenderCommandEncoder* plMetalDevice::CurrentRenderCommandEncoder()
{
    // return the current render command encoder
    // if a framebuffer wasn't set, assume screen, emulating GL
    if (fCurrentRenderTargetCommandEncoder) {
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

CA::MetalDrawable* plMetalDevice::GetCurrentDrawable() const
{
    return fCurrentDrawable;
}

void plMetalDevice::BlitTexture(MTL::Texture* src, MTL::Texture* dst)
{
    // FIXME: BlitTexture current unused - this used to create private GPU only textures through a copy from a CPU texture.
    if (fBlitCommandEncoder == nullptr) {
        fBlitCommandBuffer = fCommandQueue->commandBuffer()->retain();
        // enqueue so we go to the front of the line before render
        fBlitCommandBuffer->enqueue();
        fBlitCommandEncoder = fBlitCommandBuffer->blitCommandEncoder()->retain();
    }

    fBlitCommandEncoder->copyFromTexture(src, 0, 0, MTL::Origin(0, 0, 0), MTL::Size(src->width(), src->height(), 0), dst, 0, 0, MTL::Origin(0, 0, 0));
}
