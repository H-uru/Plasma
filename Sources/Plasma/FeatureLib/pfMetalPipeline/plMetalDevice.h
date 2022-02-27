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

#include "HeadSpin.h"

#include "plMetalDeviceRef.h"
#include "hsMatrix44.h"

#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>

#include <unordered_map>
#include <condition_variable>

#include "plSurface/plShader.h"
#include "plSurface/plShaderTable.h"

class plMetalPipeline;
class plRenderTarget;
class plBitmap;
class plMipmap;
class plCubicEnvironmap;
class plLayerInterface;

matrix_float4x4* hsMatrix2SIMD(const hsMatrix44& src, matrix_float4x4* dst, bool swapOrder = true);

class plMetalDevice
{
    
    friend plMetalPipeline;
    friend class plMetalMaterialShaderRef;
    friend class plMetalPlateManager;
    
public:
    typedef plMetalVertexBufferRef VertexBufferRef;
    typedef plMetalIndexBufferRef  IndexBufferRef;
    typedef plMetalTextureRef      TextureRef;
    
public:
    plMetalPipeline*       fPipeline;
    
    hsWindowHndl        fDevice;
    hsWindowHndl        fWindow;
    
    const char*         fErrorMsg;
    
    MTL::RenderCommandEncoder*  CurrentRenderCommandEncoder();
    MTL::Device*                fMetalDevice;
    MTL::CommandQueue*          fCommandQueue;
    MTL::Buffer*                fCurrentIndexBuffer;
    
    size_t                      fActiveThread;
    matrix_float4x4             fMatrixProj;
    matrix_float4x4             fMatrixL2W;
    matrix_float4x4             fMatrixW2L;
    matrix_float4x4             fMatrixW2C;
    matrix_float4x4             fMatrixC2W;
   
public:
    
    struct plMetalLinkedPipeline {
        MTL::RenderPipelineState    *pipelineState;
        MTL::Function               *fragFunction;
        MTL::Function               *vertexFunction;
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

    void SetupTextureRef(plLayerInterface* layer, plBitmap* img, TextureRef* tRef);
    void CheckTexture(TextureRef* tRef);
    void MakeTextureRef(TextureRef* tRef, plLayerInterface* layer, plMipmap* img);
    void MakeCubicTextureRef(TextureRef* tRef, plLayerInterface* layer, plCubicEnvironmap* img);
    
    
    const char* GetErrorString() const { return fErrorMsg; }
    
    void SetProjectionMatrix(const hsMatrix44& src);
    void SetWorldToCameraMatrix(const hsMatrix44& src);
    void SetLocalToWorldMatrix(const hsMatrix44& src, bool swapOrder = true);
    
    void SetClearColor(simd_float4 clearColor) { fClearColor = clearColor; };
    
    void PopulateTexture(plMetalDevice::TextureRef *tRef, plMipmap *img, uint slice);
    uint ConfigureAllowedLevels(plMetalDevice::TextureRef *tRef, plMipmap *mipmap);
    std::condition_variable * prewarmPipelineStateFor(plMetalVertexBufferRef * vRef, uint32_t blendFlags, uint32_t numLayers, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID, bool forShadows = false);
    ///Returns the proper pipeline state for the given vertex and fragment buffers, and the current drawable. These states should not be reused between drawables.
    plMetalLinkedPipeline* pipelineStateFor(const plMetalVertexBufferRef * vRef, uint32_t blendFlags, uint32_t numLayers, plShaderID::ID vertexShaderID, plShaderID::ID fragmentShaderID, int forShadows = 0);
    
    //stencil states are expensive to make, they should be cached
    //FIXME: There should be a function to pair these with hsGMatState
    MTL::DepthStencilState *fNoZReadStencilState;
    MTL::DepthStencilState *fNoZWriteStencilState;
    MTL::DepthStencilState *fNoZReadOrWriteStencilState;
    MTL::DepthStencilState *fReverseZStencilState;
    MTL::DepthStencilState *fDefaultStencilState;
    
    ///Create a new command buffer to encode all the operations needed to draw a frame
    //Currently requires a CA drawable and not a Metal drawable. In since CA drawable is only abstract implementation I know about, not sure where we would find others?
    void CreateNewCommandBuffer(CA::MetalDrawable* drawable);
    MTL::CommandBuffer* GetCurrentCommandBuffer();
    CA::MetalDrawable* GetCurrentDrawable();
    ///Submit the command buffer to the GPU and draws all the render passes. Clears the current command buffer.
    void SubmitCommandBuffer();
    void Clear(bool shouldClearColor, simd_float4 clearColor, bool shouldClearDepth, float clearDepth);
private:
    
    //internal struct for tracking which Metal state goes with which set of
    //fragment/vertex pass attributes. This allows for shader program reuse.
    //Hashable so we can use a std::unordered_map for storage
    struct plPipelineStateAtrributes {
        uint numUVs;
        uint numLayers;
        uint numWeights;
        bool hasSkinIndices;
        plShaderID::ID vertexShaderID;
        plShaderID::ID fragmentShaderID;
        //the specific blend mode flag, not the entire set of flags from a material
        //these are defined as mutually exclusive anyway
        //0 implies no blend flag set
        uint32_t blendFlags;
        MTL::PixelFormat outputFormat;
        MTL::PixelFormat depthFormat;
        int forShadows;
        
        bool operator==(const plPipelineStateAtrributes &p) const {
            return numUVs == p.numUVs && numWeights == p.numWeights && blendFlags == p.blendFlags && hasSkinIndices == p.hasSkinIndices && outputFormat == p.outputFormat && vertexShaderID == p.vertexShaderID && fragmentShaderID == p.fragmentShaderID && depthFormat == p.depthFormat && forShadows == p.forShadows && numUVs == p.numUVs && numLayers == p.numLayers;
        }
        
        plPipelineStateAtrributes(const plPipelineStateAtrributes &attributes) {
            memcpy(this, &attributes, sizeof(plPipelineStateAtrributes));
        }
        
        plPipelineStateAtrributes(const plMetalVertexBufferRef * vRef, const uint32_t blendFlags, const MTL::PixelFormat outputPixelFormat, const MTL::PixelFormat outputDepthFormat, const plShaderID::ID vertexShaderID, const plShaderID::ID fragmentShaderID, const int forShadows, const uint numLayers);
    };
    
    struct plPipelineStateAtrributesHashFunction
    {
        std::size_t operator() (plPipelineStateAtrributes const & key) const
        {
            std::size_t h1 = std::hash<uint>()(key.numUVs);
            std::size_t h2 = std::hash<uint>()(key.numWeights);
            std::size_t h3 = std::hash<uint32_t>()(key.blendFlags);
            std::size_t h4 = std::hash<bool>()(key.hasSkinIndices);
            std::size_t h5 = std::hash<bool>()(key.outputFormat);
            std::size_t h6 = std::hash<bool>()(key.vertexShaderID);
            std::size_t h7 = std::hash<bool>()(key.fragmentShaderID);
            std::size_t h8 = std::hash<bool>()(key.depthFormat);
            std::size_t h9 = std::hash<bool>()(key.forShadows);
            std::size_t h10 = std::hash<bool>()(key.numLayers);
     
            return h1 ^ h2 ^ h3 ^ h4 ^ h5 ^ h6 ^ h7 ^ h8 ^ h9 ^ h10;
        }
    };
    
    std::unordered_map<plPipelineStateAtrributes, plMetalLinkedPipeline *, plPipelineStateAtrributesHashFunction> fPipelineStateMap;
    //the condition map allows consumers of pipeline states to wait until the pipeline state is ready
    std::unordered_map<plPipelineStateAtrributes, std::condition_variable *, plPipelineStateAtrributesHashFunction> fConditionMap;
    void StartRenderPipelineBuild(plPipelineStateAtrributes &attributes, std::condition_variable **condOut);
    std::mutex fPipelineCreationMtx;
    
private:
    //these are internal bits for backing the current render pass
    //private because the functions should be used to keep a consistant
    //render pass state
    MTL::CommandBuffer*         fCurrentCommandBuffer;
    MTL::CommandBuffer*         fCurrentOffscreenCommandBuffer;
    MTL::RenderCommandEncoder*  fCurrentRenderTargetCommandEncoder;
    MTL::Texture*               fCurrentDrawableDepthTexture;
    MTL::Texture*               fCurrentFragmentOutputTexture;
    CA::MetalDrawable*          fCurrentDrawable;
    MTL::PixelFormat            fCurrentDepthFormat;
    simd_float4                 fClearColor;
    bool                        fShouldClearColor;
    float                       fClearDepth;
    plRenderTarget*             fCurrentRenderTarget;
    
    void BeginNewRenderPass();
};

#endif
