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
#ifndef _plMetalPipeline_inc_
#define _plMetalPipeline_inc_

#include "plPipeline/pl3DPipeline.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plMetalDevice.h"
#include <QuartzCore/QuartzCore.hpp>
#include <Metal/Metal.hpp>
#include "ShaderTypes.h"

class plIcicle;
class plPlate;
class plMetalMaterialShaderRef;
class plAuxSpan;
class plMetalVertexShader;
class plMetalFragmentShader;
class plShadowCaster;

const uint kMaxSkinWeightsPerMaterial = 3;

class plMetalEnumerate
{
public:
    plMetalEnumerate() {
        hsG3DDeviceSelector::AddDeviceEnumerator(&plMetalEnumerate::Enumerate);
    }

private:
    static void Enumerate(std::vector<hsG3DDeviceRecord>& records);
};

//// Helper Classes ///////////////////////////////////////////////////////////

//// The RenderPrimFunc lets you have one function which does a lot of stuff
// around the actual call to render whatever type of primitives you have, instead
// of duplicating everything because the one line to render is different.
class plRenderPrimFunc
{
public:
    virtual bool RenderPrims() const = 0; // return true on error
};

class plMetalPipeline : public pl3DPipeline<plMetalDevice>
{
public:
   //The actual client should set this callback so we can retrieve drawables from the window server
    std::function<CA::MetalDrawable *()>    currentDrawableCallback;
    //caching the frag function here so that the shader compiler can quickly access it
    MTL::Function*                          fFragFunction;
    
protected:

    friend class plMetalDevice;
    friend class plMetalPlateManager;
    friend class plMetalMaterialShaderRef;
    friend class plRenderTriListFunc;

    plMetalMaterialShaderRef*  fMatRefList;
    plMetalRenderTargetRef*    fRenderTargetRefList;
    
public:
    plMetalPipeline(hsWindowHndl display, hsWindowHndl window, const hsG3DDeviceModeRecord *devMode);
    virtual ~plMetalPipeline();
    
    CLASSNAME_REGISTER(plMetalPipeline);
    GETINTERFACE_ANY(plMetalPipeline, plPipeline);
    
    /* All of these virtual methods are not implemented by pl3DPipeline and
     * need to be re-implemented here!
     */

    /*** VIRTUAL METHODS ***/
    bool PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;
    bool PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;
    plTextFont* MakeTextFont(char* face, uint16_t size) override;
    bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) override;
    bool CloseAccess(plAccessSpan& acc) override;
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
    bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) override;
    plMipmap* ExtractMipMap(plRenderTarget* targ) override;
    void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) override;
    int GetMaxAnisotropicSamples() override;
    int GetMaxAntiAlias(int Width, int Height, int ColorDepth) override;
    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync = false) override;
    void RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList) override;
    void ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W);
    bool ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* spanBase);
    bool IRefreshDynVertices(plGBufferGroup* group, plMetalVertexBufferRef* vRef);
    void IRenderBufferSpan(const plIcicle& span, hsGDeviceRef* vb,
                                         hsGDeviceRef* ib, hsGMaterial* material,
                                         uint32_t vStart, uint32_t vLength,
                           uint32_t iStart, uint32_t iLength);
    void IRenderAuxSpan(const plSpan& span, const plAuxSpan* aux);
    void IRenderAuxSpans(const plSpan& span);
    bool IHandleMaterial(hsGMaterial *material, uint32_t pass, const plSpan *currSpan, const plMetalVertexBufferRef* vRef, const bool allowShaders = true);
    plMetalDevice* GetMetalDevice();
    
    // Create and/or Refresh geometry buffers
    void            CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    void            CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    void            CheckTextureRef(plLayerInterface* lay) override;
    void            CheckTextureRef(plBitmap* bitmap);
    hsGDeviceRef    *MakeTextureRef(plBitmap* bitmap);
    void            IReloadTexture( plBitmap* bitmap, plMetalTextureRef *ref );
    
    void ISetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, plMetalVertexBufferRef* vRef);
    uint32_t  IGetBufferFormatSize( uint8_t format ) const;
private:
    MTL::RenderPipelineState*       fPipelineState;
    VertexUniforms*                 fCurrentRenderPassUniforms;
    
    //cache to prevent oversetting, Metal won't catch this for us and will encode extra work
    const MTL::RenderPipelineState*       fCurrentPipelineState;
    MTL::DepthStencilState*         fCurrentDepthStencilState;
    
    void FindFragFunction();
    
    void ISelectLights(const plSpan* span, plMetalMaterialShaderRef* mRef, bool proj = false);
    void IEnableLight(plMetalMaterialShaderRef* mRef, size_t i, plLightInfo* light);
    void IDisableLight(plMetalMaterialShaderRef* mRef, size_t i);
    void IScaleLight(plMetalMaterialShaderRef* mRef, size_t i, float scale);
    void ICalcLighting(plMetalMaterialShaderRef* mRef, const plLayerInterface* currLayer, const plSpan* currSpan);
    void IHandleBlendMode(hsGMatState flags);
    void IHandleZMode(hsGMatState flags);
    
    void IDrawPlate(plPlate* plate);
    void IPreprocessAvatarTextures();
    void IDrawClothingQuad(float x, float y, float w, float h,
                           float uOff, float vOff, plMipmap *tex);
    void IClearShadowSlaves();
    
    void IReleaseDynDeviceObjects();
    bool ICreateDynDeviceObjects();
    
    bool IIsViewLeftHanded();
    void ISetCullMode(bool flip = false);
    
    plLayerInterface* IPushOverBaseLayer(plLayerInterface* li);
    plLayerInterface* IPopOverBaseLayer(plLayerInterface* li);
    plLayerInterface* IPushOverAllLayer(plLayerInterface* li);
    plLayerInterface* IPopOverAllLayer(plLayerInterface* li);
    
    void IPushPiggyBacks(hsGMaterial* mat);
    void IPopPiggyBacks();
    void IPushProjPiggyBack(plLayerInterface* li);
    void IPopProjPiggyBacks();
    int ISetNumActivePiggyBacks();
    bool ICheckAuxBuffers(const plAuxSpan* span);
    
    void ISetPipeConsts(plShader* shader);
    bool ISetShaders(const plMetalVertexBufferRef * vRef, const hsGMatState blendMode, plShader* vShader, plShader* pShader);
    
    bool ISoftwareVertexBlend(plDrawableSpans* drawable, const std::vector<int16_t>& visList);
    void IBlendVertBuffer(plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                                 const uint8_t* src, uint8_t format, uint32_t srcStride,
                                 uint8_t* dest, uint32_t destStride, uint32_t count,
                                 uint16_t localUVWChans);
    
    plMetalVertexShader*       fVShaderRefList;
    plMetalFragmentShader*       fPShaderRefList;
    MTL::CullMode               fCurrentCullMode;
    
    bool IPrepShadowCaster(const plShadowCaster* caster);
    bool IRenderShadowCaster(plShadowSlave* slave);
    void IPreprocessShadows();
    bool IPushShadowCastState(plShadowSlave* slave);
    plRenderTarget* IFindRenderTarget(uint32_t& width, uint32_t& height, bool ortho);
    bool IPopShadowCastState(plShadowSlave* slave);
    void IResetRenderTargetPools();
    void IRenderShadowCasterSpan(plShadowSlave* slave, plDrawableSpans* drawable, const plIcicle& span);
    plMetalTextureRef* fULutTextureRef;
    void ISetupShadowLight(plShadowSlave* slave);
    void IMakeRenderTargetPools();
    hsGDeviceRef* SharedRenderTargetRef(plRenderTarget* share, plRenderTarget *owner);
    void IRenderShadowsOntoSpan(const plRenderPrimFunc& render, const plSpan* span, hsGMaterial* mat, plMetalVertexBufferRef *vRef);
    void ISetupShadowRcvTextureStages(hsGMaterial* mat);
    void ISetupShadowSlaveTextures(plShadowSlave* slave);
    void ISetShadowLightState(hsGMaterial* mat);
    void IDisableLightsForShadow();
    void IReleaseRenderTargetPools();
    void IRenderProjectionEach(const plRenderPrimFunc& render, hsGMaterial* material, int iPass, const plSpan& span, const plMetalVertexBufferRef* vRef);
    
    // Shadows
    std::vector<plRenderTarget*>       fRenderTargetPool512;
    std::vector<plRenderTarget*>       fRenderTargetPool256;
    std::vector<plRenderTarget*>       fRenderTargetPool128;
    std::vector<plRenderTarget*>       fRenderTargetPool64;
    std::vector<plRenderTarget*>       fRenderTargetPool32;
    enum { kMaxRenderTargetNext = 10 };
    uint32_t                        fRenderTargetNext[kMaxRenderTargetNext];
    
    std::vector<plLightInfo*>  fProjEach;
    std::vector<plLightInfo*>  fProjAll;
    
    void PushCurrentLightSources();
    void PopCurrentLightSources();
    std::vector<plMetalShaderLightSource *> fLightSourceStack;
    
    static plMetalEnumerate enumerator;
};

#endif // _plGLPipeline_inc_
