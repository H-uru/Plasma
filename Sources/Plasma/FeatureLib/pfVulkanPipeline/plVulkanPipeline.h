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

#ifndef _plVulkanPipeline_inc_
#define _plVulkanPipeline_inc_

#include "plVulkanDevice.h"

#include "plPipeline/hsG3DDeviceSelector.h"
#include "plPipeline/pl3DPipeline.h"
#include "plSurface/plShaderTable.h"

/**
 * Registers the Vulkan device enumerator during static init.
 *
 * plVulkanPipeline holds one of these as a static data member, defined at file
 * scope in the .cpp, which is what actually gets the enumerator registered --
 * the same trick pfGLPipeline and pfMetalPipeline use.
 */
class plVulkanEnumerate
{
public:
    plVulkanEnumerate()
    {
        hsG3DDeviceSelector::AddDeviceEnumerator(&plVulkanEnumerate::Enumerate);
    }

private:
    static void Enumerate(std::vector<hsG3DDeviceRecord>& records, hsDisplayHndl display);
};

class plPlate;
class plAuxSpan;
class plShader;
class plShadowCaster;
class plVulkanShaderRef;
class plShadowSlave;
class plVulkanMaterialShaderRef;
struct plVulkanPassInfo;

class plVulkanPipeline : public pl3DPipeline<plVulkanDevice>
{
    friend class plVulkanPlateManager;

public:
    plVulkanPipeline(hsDisplayHndl display, hsWindowHndl window,
                     const hsG3DDeviceModeRecord* devMode);
    ~plVulkanPipeline();

    CLASSNAME_REGISTER(plVulkanPipeline);
    GETINTERFACE_ANY(plVulkanPipeline, plPipeline);

    /* Everything below is pure virtual on plPipeline and not implemented by
     * pl3DPipeline, so it has to be answered here.
     */

    bool PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr = nullptr) override;
    bool PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr = nullptr) override;
    plTextFont* MakeTextFont(ST::string face, uint16_t size) override;
    void CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    void CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) override;
    bool CloseAccess(plAccessSpan& acc) override;
    void CheckTextureRef(plLayerInterface* lay) override;
    void PushRenderRequest(plRenderRequest* req) override;
    void PopRenderRequest(plRenderRequest* req) override;
    void ClearRenderTarget(plDrawable* d) override;
    void ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) override;
    hsGDeviceRef* MakeRenderTargetRef(plRenderTarget* owner) override;
    bool BeginRender() override;
    bool EndRender() override;
    void RenderScreenElements() override;
    bool IsFullScreen() const override;
    void Resize(uint32_t width, uint32_t height) override;
    void LoadResources() override;
    bool SetGamma(float eR, float eG, float eB) override;
    bool SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) override;
    bool SetGamma10(const uint16_t* const tabR, const uint16_t* const tabG,
                    const uint16_t* const tabB) override;
    bool Supports10BitGamma() const override { return fDevice.Supports10BitGamma(); }
    bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) override;
    plMipmap* ExtractMipMap(plRenderTarget* targ) override;
    void GetSupportedDisplayModes(std::vector<plDisplayMode>* res, int ColorDepth = 32) override;
    int GetMaxAnisotropicSamples() override;
    int GetMaxAntiAlias(int Width, int Height, int ColorDepth) override;
    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed,
                            int NumAASamples, int MaxAnisotropicSamples, bool vSync = false) override;
    void RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList) override;
    plRenderTarget* PopRenderTarget() override;
    void RegisterLight(plLightInfo* light) override;
    void UnRegisterLight(plLightInfo* light) override;

    /*** Not virtual, but part of the texture path ***/

    /**
     * Forgets what is currently bound.
     *
     * plVulkanTextFont binds its own pipeline and vertex buffer directly, so the
     * span path must not believe its cached bindings afterwards.
     */
    void InvalidateBoundState() { fState.Reset(); }

    void          CheckTextureRef(plBitmap* bitmap);
    hsGDeviceRef* MakeTextureRef(plBitmap* bitmap);
    void          IReloadTexture(plBitmap* bitmap, plVulkanTextureRef* ref);

    /*** The draw path, in plVulkanDraw.cpp ***/

    void ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W,
                          bool force = false);
    bool ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* spanBase);
    bool IRefreshDynVertices(plGBufferGroup* group, plVulkanVertexBufferRef* vRef);

    /**
     * Sets up one pass of a material; false means do not draw it.
     *
     * `allowShaders` is false for projection passes, which want the material's
     * geometry drawn through the fixed-function combiner whatever the layer
     * would normally ask for.
     */
    bool IHandleMaterialPass(VkCommandBuffer cmd, hsGMaterial* material, size_t pass,
                             const plSpan* span, const plVulkanVertexBufferRef* vRef,
                             bool allowShaders = true, bool overrideLiteModel = false);

    /*** Aux spans, in plVulkanAuxSpans.cpp ***/

    /** Draws the extra geometry a span carries -- decals, ripples, footprints. */
    void IRenderAuxSpans(VkCommandBuffer cmd, const plSpan& span);
    void IRenderAuxSpan(VkCommandBuffer cmd, const plSpan& span, const plAuxSpan* aux);
    bool ICheckAuxBuffers(const plAuxSpan* span);

    void IRenderBufferSpan(const plIcicle& span, hsGDeviceRef* vb, hsGDeviceRef* ib,
                           hsGMaterial* material, uint32_t vStart, uint32_t vLength,
                           uint32_t iStart, uint32_t iLength);

    /** Draws one 2D overlay. Ported from plMetalPipeline::IDrawPlate (:2517). */
    void IDrawPlate(plPlate* plate);

    bool ISoftwareVertexBlend(plDrawableSpans* drawable, const std::vector<int16_t>& visList);
    void IBlendVertBuffer(plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                          const uint8_t* src, uint8_t format, uint32_t srcStride,
                          uint8_t* dest, uint32_t destStride, uint32_t count,
                          uint16_t localUVWChans);

    /*** Override and piggyback layers, in plVulkanMaterial.cpp ***/

    /*** The plShader programmable path, in plVulkanShaderPath.cpp ***/

    /** Whether both halves of a plShader pair have a GLSL implementation. */
    bool IShaderPairSupported(plShaderID::ID vertexID, plShaderID::ID fragmentID);

    /**
     * Binds a programmable pass and hands back where its constants landed.
     *
     * Both offsets are always written, even when a shader has no constants.
     */
    bool ISetShaders(VkCommandBuffer cmd, const plVulkanVertexBufferRef* vRef,
                     const hsGMatState& state, plShader* vShader, plShader* pShader,
                     uint32_t* vertexConstOffset, uint32_t* fragmentConstOffset);

    /** Fills the registers a shader wants taken from pipeline state. */
    void ISetPipeConsts(plShader* shader);

    const hsMatrix44* IGetStageTransform(uint32_t stage);
    uint32_t          IUploadShaderConsts(plShader* shader);

    bool IBindShaderPassResources(VkCommandBuffer cmd, hsGMaterial* material, size_t pass,
                                  const plSpan* span, uint32_t vertexConstOffset,
                                  uint32_t fragmentConstOffset);

    /*** Avatar clothing composite, in plVulkanAvatar.cpp ***/

    /** Rebuilds the skin texture of every outfit that changed. */
    void IPreprocessAvatarTextures();

    void IDrawClothingQuad(VkCommandBuffer cmd, uint8_t passKind, float x, float y,
                           float w, float h, float uOff, float vOff, plMipmap* tex,
                           const hsColorRGBA& tint);

    /*** Projected lights, in plVulkanProjections.cpp ***/

    /** Starts a fresh light set, keeping the span's for when this pass is done. */
    void SaveCurrentLightSources();
    void RestoreCurrentLightSources();

    /** Draws every over-all projector once, after the material's passes. */
    void IRenderProjections(VkCommandBuffer cmd, const plVulkanVertexBufferRef* vRef,
                            uint32_t iStart, uint32_t iLength);

    void IRenderProjection(VkCommandBuffer cmd, plLightInfo* li,
                           const plVulkanVertexBufferRef* vRef,
                           uint32_t iStart, uint32_t iLength);

    /** Draws every per-pass projector over the pass just rendered. */
    void IRenderProjectionEach(VkCommandBuffer cmd, hsGMaterial* material, size_t pass,
                               const plSpan& span, const plVulkanVertexBufferRef* vRef,
                               uint32_t iStart, uint32_t iLength);

    void IPushProjPiggyBack(plLayerInterface* li);
    void IPopProjPiggyBacks();

    /*** Shadows, in plVulkanShadows.cpp ***/

    /** Renders every submitted shadow slave into a shadow map. */
    void IPreprocessShadows();

    /** Projects this frame's shadow maps onto a span that receives them. */
    void IRenderShadowsOntoSpan(VkCommandBuffer cmd, const plSpan* span, hsGMaterial* mat,
                                const plVulkanVertexBufferRef* vRef,
                                uint32_t iStart, uint32_t iLength);

    /** The layer budget ILayersAtOnce has to leave free for piggybacks. */
    uint32_t GetMaxPiggyBacks() const { return fMaxPiggyBacks; }

    /** Wraps a layer in fOverBaseLayer. Must be matched by IPopOverBaseLayer. */
    plLayerInterface* IPushOverBaseLayer(plLayerInterface* li);
    plLayerInterface* IPopOverBaseLayer(plLayerInterface* li);

    /** Wraps a layer in fOverAllLayer. Must be matched by IPopOverAllLayer. */
    plLayerInterface* IPushOverAllLayer(plLayerInterface* li);
    plLayerInterface* IPopOverAllLayer(plLayerInterface* li);

    /** Pushes a material's own piggybacks, which are always light maps. */
    void   IPushPiggyBacks(hsGMaterial* mat);
    void   IPopPiggyBacks();
    size_t ISetNumActivePiggyBacks();

    bool IIsViewLeftHanded();
    void IHandleZMode(VkCommandBuffer cmd, const hsGMatState& state);
    void ISetCullMode(VkCommandBuffer cmd, const hsGMatState& state);
    void ICalcLighting(const plLayerInterface* layer, const plSpan* span,
                       const hsGMatState& state);

    /*** Lighting, in plVulkanLights.cpp ***/

    /** Writes every registered light into this frame's light buffer. */
    void LoadLightsOnDevice();

    /** Picks the strongest lights for a span, fading the ones near the cut. */
    void ISelectLights(const plSpan* span, bool proj);
    void ILoadLight(plLightInfo* light);
    void IScaleLight(plLightInfo* light, float scale);

    hsGDeviceRef* IMakeLightRef(plLightInfo* owner);

private:
    /** True between a successful BeginRender and its EndRender. */
    bool fInFrame;

    /**
     * Whether the window is fullscreen.
     *
     * Starts from graphics.ini and follows ResetDisplayDevice; IsFullScreen has to
     * answer with the current state, not the launch state.
     */
    bool fIsFullscreen;

    /** Intrusive list of the fonts MakeTextFont has handed out. */
    plTextFont* fTextFontRefList;

    /** Intrusive list of live offscreen targets. */
    plVulkanRenderTargetRef* fRenderTargetRefList;

    /**
     * Intrusive list of the pass decompositions cached on materials.
     *
     * A material can outlive the pipeline that decomposed it, so every ref is
     * orphaned at shutdown rather than left holding a dangling pipeline.
     */
    plVulkanMaterialShaderRef* fMaterialRefList;

    /** Intrusive list of the plShaders that have been handed a device ref. */
    plVulkanShaderRef* fShaderRefList;

    /**
     * Layers pushed as override wrappers, innermost last.
     *
     * plLayerInterface::Attach does not record what was attached, so the pop
     * side needs this to know what to detach.
     */
    std::vector<plLayerInterface*> fOverLayerStack;

    /** Every registered light, in the order they land in the frame buffer. */
    plVulkanLightRef* fLightRefList;

    /*** Shadows ***/

    /** Releases the frame-local shadow submissions and their retained casters. */
    void IClearShadowSlaves();

    bool IRenderShadowCaster(plShadowSlave* slave);
    bool IPrepShadowCaster(const plShadowCaster* caster);
    void IRenderShadowCasterSpan(VkCommandBuffer cmd, plShadowSlave* slave,
                                 plDrawableSpans* drawable, const plIcicle& span);
    bool IPushShadowCastState(plShadowSlave* slave);
    bool IPopShadowCastState(plShadowSlave* slave);
    void ISetupShadowState(plShadowSlave* slave, plShadowState& shadowState);

    /**
     * Decides which material layer supplies the shadow's alpha.
     *
     * Returns -1 when none does, and writes that layer's misc flags to
     * \p miscFlags -- the vertex stage needs them to build coordinate set 2.
     * Ported from ISetupShadowRcvTextureStages (plMetalPipeline.cpp:3886-3939).
     */
    int32_t ISetupShadowRcvTextureStages(hsGMaterial* mat, uint32_t* miscFlags);

    /** Points uvTransforms 0 and 1 at this slave's map and falloff LUT. */
    const plVulkanRenderTargetRef* ISetupShadowSlaveTextures(plShadowSlave* slave);

    void IMakeRenderTargetPools();
    void IReleaseRenderTargetPools();
    void IResetRenderTargetPools();

    /** A free target of the requested size, or the next size down. */
    plRenderTarget* IFindRenderTarget(uint32_t& width, uint32_t& height, bool ortho);

    /**
     * Shadow map pools, one per power-of-two size.
     *
     * These are recycled within a frame, not across: IResetRenderTargetPools
     * hands the same targets out again next frame.
     */
    std::vector<plRenderTarget*> fRenderTargetPool512;
    std::vector<plRenderTarget*> fRenderTargetPool256;
    std::vector<plRenderTarget*> fRenderTargetPool128;
    std::vector<plRenderTarget*> fRenderTargetPool64;
    std::vector<plRenderTarget*> fRenderTargetPool32;

    enum
    {
        kMaxRenderTargetNext = 10
    };

    /** How far into each pool this frame has got. */
    uint32_t fRenderTargetNext[kMaxRenderTargetNext];

    /**
     * A zeroed shadow-state block, allocated once per frame.
     *
     * Every draw binds the shadow-state descriptor whether or not it is a
     * shadow, so non-shadow draws point at this.
     */
    plVulkanDevice::plScratchAlloc fEmptyShadowState;

    /**
     * This frame's light buffer, and where in the scratch ring it lives.
     *
     * Written once per frame by LoadLightsOnDevice and indexed per draw, which
     * keeps the per-draw payload to the active list rather than the whole scene.
     */
    plVulkanDevice::plScratchAlloc fLightBuffer;
    uint32_t                       fLightBufferCount;

    /** The lights reaching the span being drawn. */
    std::vector<plShaderActiveLight> fLights;

    /**
     * Light sets a projection pass pushed aside.
     *
     * A projection enables only its own light, but the pass after it has to be
     * lit exactly as the pass before was, so the span's set is stacked here
     * rather than recomputed.
     */
    std::vector<std::vector<plShaderActiveLight>> fLightStack;

    /** Projected lights, split by whether they cover everything or one span. */
    std::vector<plLightInfo*> fProjAll;
    std::vector<plLightInfo*> fProjEach;

    /**
     * Frame counter, used to skin each drawable at most once per frame.
     *
     * plMetalPipeline::fRenderCnt; bumped in BeginRender.
     */
    uint32_t fRenderCnt;

    /**
     * Generation counter for volatile buffers.
     *
     * A volatile buffer whose fRefTime does not match this has not been written
     * yet this frame and needs refilling. Bumped once per frame in BeginRender,
     * the same role plMetalPipeline::fVtxRefTime plays.
     */
    uint32_t fVtxRefTime;

    /** Written per span, uploaded to the scratch ring per draw. */
    VertexUniforms               fCurrentUniforms;
    plMaterialLightingDescriptor fCurrentMaterial;

    /**
     * Redundant-bind filter, the equivalent of plMetalPipelineCurrentState
     * (plMetalPipeline.h:284-303).
     *
     * Everything here has to be cleared whenever the rendering scope changes,
     * because a new command buffer or render pass loses all bindings.
     */
    struct plVulkanCurrentState
    {
        VkPipeline fPipeline;
        VkBuffer   fVertexBuffer;
        VkBuffer   fIndexBuffer;

        void Reset()
        {
            fPipeline = VK_NULL_HANDLE;
            fVertexBuffer = VK_NULL_HANDLE;
            fIndexBuffer = VK_NULL_HANDLE;
        }
    } fState;

    static plVulkanEnumerate enumerator;
};

#endif // _plVulkanPipeline_inc_
