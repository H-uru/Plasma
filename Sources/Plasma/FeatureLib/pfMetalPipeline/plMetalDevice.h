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
#ifndef _plMetalDevice_h_
#define _plMetalDevice_h_

#include <simd/simd.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <condition_variable>
#include <string_theory/string>
#include <unordered_map>

#include "HeadSpin.h"
#include "hsGMatState.h"
#include "hsMatrix44.h"
#include "plMetalDeviceRef.h"
#include "plSurface/plShader.h"
#include "plSurface/plShaderTable.h"

class plMetalPipeline;
class plRenderTarget;
class plBitmap;
class plMipmap;
class plCubicEnvironmap;
class plLayerInterface;
class plMetalPipelineState;

inline const matrix_float4x4& hsMatrix2SIMD(const hsMatrix44& src)
{
    // reinterpret_cast not allowed in constexpr
    return *reinterpret_cast<const simd_float4x4* >(src.fMap);
}

class plMetalDevice
{
    friend plMetalPipeline;
    friend class plMetalMaterialShaderRef;
    friend class plMetalPlateManager;
    friend class plMetalPipelineState;

public:
    typedef plMetalVertexBufferRef VertexBufferRef;
    typedef plMetalIndexBufferRef  IndexBufferRef;
    typedef plMetalTextureRef      TextureRef;

public:
    plMetalPipeline* fPipeline;

    hsWindowHndl fDevice;
    hsWindowHndl fWindow;

    ST::string fErrorMsg;

    MTL::RenderCommandEncoder* CurrentRenderCommandEncoder();
    MTL::Device*               fMetalDevice;
    MTL::CommandQueue*         fCommandQueue;
    MTL::Buffer*               fCurrentIndexBuffer;

    size_t          fActiveThread;
    matrix_float4x4 fMatrixProj;
    matrix_float4x4 fMatrixL2W;
    matrix_float4x4 fMatrixW2L;
    matrix_float4x4 fMatrixW2C;
    matrix_float4x4 fMatrixC2W;

public:
    struct plMetalLinkedPipeline
    {
        const MTL::RenderPipelineState* pipelineState;
        const MTL::Function*            fragFunction;
        const MTL::Function*            vertexFunction;
    };

    plMetalDevice();

    bool InitDevice();

    void Shutdown();

    /**
     * Set rendering to the specified render target.
     *
     * Null rendertarget is the primary. Invalidates the state as required by
     * experience, not documentation.
     */
    void SetRenderTarget(plRenderTarget* target);

    /** Translate our viewport into a GL viewport. */
    void SetViewport();

    bool BeginRender();

    /* Device Ref Functions **************************************************/
    void SetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, VertexBufferRef* vRef);
    void CheckStaticVertexBuffer(VertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx);
    void FillVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx);
    void FillVolatileVertexBufferRef(VertexBufferRef* ref, plGBufferGroup* group, uint32_t idx);
    void SetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, IndexBufferRef* iRef);
    void CheckIndexBuffer(IndexBufferRef* iRef);
    void FillIndexBufferRef(IndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx);

    void SetupTextureRef(plBitmap* img, TextureRef* tRef);
    void CheckTexture(TextureRef* tRef);
    void MakeTextureRef(TextureRef* tRef, plMipmap* img);
    void MakeCubicTextureRef(TextureRef* tRef, plCubicEnvironmap* img);

    ST::string GetErrorString() const { return fErrorMsg; }

    void SetProjectionMatrix(const hsMatrix44& src);
    void SetWorldToCameraMatrix(const hsMatrix44& src);
    void SetLocalToWorldMatrix(const hsMatrix44& src);

    void PopulateTexture(plMetalDevice::TextureRef* tRef, plMipmap* img, uint slice);
    uint ConfigureAllowedLevels(plMetalDevice::TextureRef* tRef, plMipmap* mipmap);

    // stencil states are expensive to make, they should be cached
    // FIXME: There should be a function to pair these with hsGMatState
    MTL::DepthStencilState* fNoZReadStencilState;
    MTL::DepthStencilState* fNoZWriteStencilState;
    MTL::DepthStencilState* fNoZReadOrWriteStencilState;
    MTL::DepthStencilState* fReverseZStencilState;
    MTL::DepthStencilState* fDefaultStencilState;
    uint8_t                 fSampleCount;

    /// Create a new command buffer to encode all the operations needed to draw a frame
    // Currently requires a CA drawable and not a Metal drawable. In since CA drawable is only abstract implementation I know about, not sure where we would find others?
    void                CreateNewCommandBuffer(CA::MetalDrawable* drawable);
    MTL::CommandBuffer* GetCurrentCommandBuffer() const;
    MTL::CommandBuffer* GetCurrentDrawableCommandBuffer() const { return fCurrentCommandBuffer; }
    CA::MetalDrawable*  GetCurrentDrawable() const;
    /// Submit the command buffer to the GPU and draws all the render passes. Clears the current command buffer.
    void                SubmitCommandBuffer();
    void                Clear(bool shouldClearColor, simd_float4 clearColor, bool shouldClearDepth, float clearDepth);

    void SetMaxAnsiotropy(uint8_t maxAnsiotropy);
    void SetMSAASampleCount(uint8_t sampleCount);

    MTL::SamplerState* SampleStateForClampFlags(hsGMatState::hsGMatClampFlags sampleState) const;

    NS::UInteger CurrentTargetSampleCount() const
    {
        if (fCurrentRenderTarget) {
            return 1;
        } else {
            return fSampleCount;
        }
    }

    void BlitTexture(MTL::Texture* src, MTL::Texture* dst);

    void EncodeBlur(MTL::CommandBuffer* commandBuffer, MTL::Texture* texture, float sigma);

    MTL::PixelFormat GetFramebufferFormat() const { return fFramebufferFormat; };
    
    MTL::Library* GetShaderLibrary() const { return fShaderLibrary; }
    
    static constexpr MTL::StorageMode GetDefaultStorageMode()
    {
#ifdef HS_BUILD_FOR_MACOS
        return MTL::StorageModeManaged;
#else
        return MTL::StorageModeShared;
#endif
    }

private:
    struct plMetalPipelineRecord
    {
        MTL::PixelFormat                      depthFormat;
        MTL::PixelFormat                      colorFormat;
        NS::UInteger                          sampleCount;
        std::shared_ptr<plMetalPipelineState> state;

        bool operator==(const plMetalPipelineRecord& p) const;
    };

    struct plMetalPipelineRecordHashFunction
    {
        std::size_t operator()(plMetalPipelineRecord const& s) const noexcept;
    };

    std::unordered_map<plMetalPipelineRecord, plMetalLinkedPipeline*, plMetalPipelineRecordHashFunction>   fNewPipelineStateMap;
    // the condition map allows consumers of pipeline states to wait until the pipeline state is ready
    std::unordered_map<plMetalPipelineRecord, std::condition_variable*, plMetalPipelineRecordHashFunction> fConditionMap;
    std::mutex                                                                                             fPipelineCreationMtx;
    void                                                                                                   StartPipelineBuild(plMetalPipelineRecord& record, std::condition_variable** condOut);
    std::condition_variable*                                                                               PrewarmPipelineStateFor(plMetalPipelineState* pipelineState);
    
    void SetOutputLayer(CA::MetalLayer* layer) { fLayer = layer; }
    CA::MetalLayer* GetOutputLayer() const { return fLayer; };
    
    hsDisplayHndl fDisplay;

protected:
    plMetalLinkedPipeline* PipelineState(plMetalPipelineState* pipelineState);

    MTL::Texture* fGammaLUTTexture;

    void SetFramebufferFormat(MTL::PixelFormat format);

private:
    MTL::PixelFormat fFramebufferFormat;

    // these are internal bits for backing the current render pass
    // private because the functions should be used to keep a consistant
    // render pass state
    MTL::CommandBuffer*        fCurrentCommandBuffer;
    MTL::CommandBuffer*        fCurrentOffscreenCommandBuffer;
    MTL::RenderCommandEncoder* fCurrentRenderTargetCommandEncoder;
    
    CA::MetalLayer*            fLayer;

    MTL::Texture* fCurrentDrawableDepthTexture;
    MTL::Texture* fCurrentFragmentOutputTexture;
    MTL::Texture* fCurrentUnprocessedOutputTexture;
    MTL::Texture* fCurrentFragmentMSAAOutputTexture;

    CA::MetalDrawable* fCurrentDrawable;
    MTL::PixelFormat   fCurrentDepthFormat;
    simd_float4        fClearRenderTargetColor;
    simd_float4        fClearDrawableColor;
    bool               fShouldClearRenderTarget;
    bool               fShouldClearDrawable;
    float              fClearRenderTargetDepth;
    float              fClearDrawableDepth;
    plRenderTarget*    fCurrentRenderTarget;
    MTL::SamplerState* fSamplerStates[4];

    MTL::CommandBuffer*      fBlitCommandBuffer;
    MTL::BlitCommandEncoder* fBlitCommandEncoder;
    
    MTL::Library* fShaderLibrary;
    
    void LoadLibrary();

    void BeginNewRenderPass();
    void ReleaseSamplerStates();
    void ReleaseFramebufferObjects();

    // Blur states
    std::unordered_map<float, NS::Object*> fBlurShaders;
    
    // MARK: - Post processing
private:
    bool NeedsPostprocessing() const
    {
        return fGammaLUTTexture != nullptr;
    }
    void                      PreparePostProcessing();
    void                      FinalizePostProcessing();
    void                      PostprocessIntoDrawable();
    void                      CreateGammaAdjustState();
    MTL::RenderPipelineState* fGammaAdjustState;

    // MARK:  - Device capabilities
private:
    /// Returns true if the device supports tile memory features such as directly writable render buffers.
    inline bool SupportsTileMemory() const { return fSupportsTileMemory; }
    bool fSupportsTileMemory;
};

#endif
