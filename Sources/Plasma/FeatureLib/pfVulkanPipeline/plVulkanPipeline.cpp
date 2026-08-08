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

#include "plVulkanPipeline.h"

#include <string_theory/format>

#include <cmath>

#include "plVulkanMaterialShaderRef.h"
#include "plVulkanShaderRef.h"
#include "plVulkanPlateManager.h"
#include "plVulkanTextFont.h"

#include "plMessage/plDeviceRecreateMsg.h"
#include "hsTimer.h"
#include "plPipeResReq.h"
#include "pnNetCommon/plNetApp.h"
#include "pnMessage/plPipeResMakeMsg.h"
#include "pnDispatch/plDispatch.h"

#include "pfCamera/plVirtualCamNeu.h"

#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plAccessSpan.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plPipeline/plDebugText.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plPipeline/plRenderTarget.h"
#include "plScene/plRenderRequest.h"
#include "plSurface/plLayerInterface.h"

plVulkanEnumerate plVulkanPipeline::enumerator;

plVulkanPipeline::plVulkanPipeline(hsDisplayHndl display, hsWindowHndl window,
                                   const hsG3DDeviceModeRecord* devMode)
    : pl3DPipeline(devMode), fInFrame(), fTextFontRefList(), fRenderTargetRefList(), fMaterialRefList(), fShaderRefList(),
      fLightRefList(), fLightBuffer(), fLightBufferCount(), fRenderCnt(), fVtxRefTime(),
      fCurrentUniforms(), fCurrentMaterial()
{
    // pl3DPipeline's constructor initializes fVtxBuffRefList and fIdxBuffRefList
    // but not fTextureRefList, so that one starts as garbage. Metal nulls all of
    // them by hand for the same reason.
    fTextureRefList = nullptr;
    fVtxBuffRefList = nullptr;
    fIdxBuffRefList = nullptr;

    // The shader's texture arrays are kMaxLayers wide, and Plasma allots half of
    // whatever a pipeline can bind at once to piggybacks.
    fMaxLayersAtOnce = kMaxLayers;
    fMaxPiggyBacks = fMaxLayersAtOnce >> 1;

    fIsFullscreen = !plPipeline::fInitialPipeParams.Windowed;

    fState.Reset();
    fDevice.fPipeline = this;
    fDevice.SetWindow(window);
    fDevice.SetDeviceName(devMode->GetDevice()->GetDeviceDesc());
    fDevice.SetVSync(fVSync);
    fDevice.SetMSAASampleCount(uint32_t(std::max(1, fInitialPipeParams.AntiAliasingAmount)));

    // A non-empty error string here makes plClient::InitPipeline throw this
    // pipeline away and fall back to the default device, so there is nothing to
    // do on failure but leave the message in place.
    if (!fDevice.InitDevice())
        return;

    fPlateMgr = new plVulkanPlateManager(this);
    fDebugTextMgr = new plDebugTextManager();

    IMakeRenderTargetPools();
}

plVulkanPipeline::~plVulkanPipeline()
{
    IClearShadowSlaves();
    hsRefCnt_SafeUnRef(fCurrMaterial);
    fCurrMaterial = nullptr;

    // Order matters. Metal can just drop these because MTL objects are
    // refcounted and outlive the device object; Vulkan resources are owned by
    // the allocator that Shutdown() destroys, so everything holding GPU memory
    // has to hand it back first.
    delete fPlateMgr;
    fPlateMgr = nullptr;

    delete fDebugTextMgr;
    fDebugTextMgr = nullptr;

    while (fTextFontRefList)
        delete fTextFontRefList;

    // Release() frees the GPU resources but deliberately leaves the ref itself
    // alive -- the engine object still owns it. So each one has to be unlinked
    // explicitly, or this walks the same head forever.
    auto releaseAll = [](auto*& head) {
        while (head) {
            auto* ref = head;
            // Unlink first so the head advances even if Release is destructive.
            ref->Unlink();
            ref->Release();
        }
    };

    releaseAll(fVtxBuffRefList);
    releaseAll(fIdxBuffRefList);
    releaseAll(fTextureRefList);
    // The pooled shadow maps own render target refs, so they have to go before
    // the ref list is walked.
    IReleaseRenderTargetPools();

    releaseAll(fRenderTargetRefList);

    while (fShaderRefList) {
        plVulkanShaderRef* ref = fShaderRefList;
        ref->Unlink();
        ref->Release();
    }

    // A material can outlive us, so its cached decomposition has to forget us
    // rather than be left pointing at a destroyed pipeline.
    while (fMaterialRefList) {
        plVulkanMaterialShaderRef* ref = fMaterialRefList;
        ref->Unlink();
        ref->Release();
        ref->Orphan();
    }

    fDevice.Shutdown();
}

bool plVulkanPipeline::BeginRender()
{
    // Note the inverted sense: true means "do not draw this frame".
    if (!fDevice.IsInited())
        return true;

    fState.Reset();
    fVtxRefTime++;
    plVulkanBufferPoolRef::SetFrameTime(fVtxRefTime);
    fRenderCnt++;

    // Layer animations are evaluated against the pipeline's frame time. Keep
    // this in step with the established DirectX and Metal backends.
    fTime = hsTimer::GetSysSeconds();

    fInFrame = fDevice.BeginFrame();
    if (!fInFrame) {
        IClearShadowSlaves();
        hsRefCnt_SafeUnRef(fCurrMaterial);
        fCurrMaterial = nullptr;
        return true;
    }

    // Every draw binds the shadow-state descriptor whether or not it is a
    // shadow, so one zeroed block per frame stands in for the rest.
    fEmptyShadowState = fDevice.AllocateScratch(sizeof(plShadowState));
    if (fEmptyShadowState.IsValid())
        memset(fEmptyShadowState.fMapped, 0, sizeof(plShadowState));

    // Both of these bind their own render targets and put them back, and both
    // have to finish before the scene starts sampling what they produced.
    IPreprocessAvatarTextures();
    IPreprocessShadows();

    return false;
}

bool plVulkanPipeline::EndRender()
{
    if (!fInFrame) {
        IClearShadowSlaves();
        hsRefCnt_SafeUnRef(fCurrMaterial);
        fCurrMaterial = nullptr;
        return true;
    }

    fDevice.EndFrame();
    fInFrame = false;
    fState.Reset();

    IClearShadowSlaves();

    hsRefCnt_SafeUnRef(fCurrMaterial);
    fCurrMaterial = nullptr;

    return false;
}

void plVulkanPipeline::ClearRenderTarget(const hsColorRGBA* col, const float* depth)
{
    const bool clearColorEnabled = (fView.fRenderState & kRenderClearColor) != 0;
    const bool clearDepthEnabled = (fView.fRenderState & kRenderClearDepth) != 0;
    if (!clearColorEnabled && !clearDepthEnabled)
        return;

    const hsColorRGBA clearColor = col ? *col : GetClearColor();
    const float clearDepth = depth ? *depth : GetClearDepth();

    // Render requests select the aspects independently. GUI post effects, for
    // example, clear depth so their 3-D controls are not tested against the
    // world, while preserving the title scene's color attachment.
    fDevice.Clear(clearColorEnabled ? &clearColor : nullptr,
                  clearDepthEnabled ? &clearDepth : nullptr);
}

void plVulkanPipeline::ClearRenderTarget(plDrawable* d)
{
    if (!plDrawableSpans::ConvertNoRef(d)) {
        ClearRenderTarget();
        return;
    }

    Draw(d);
}

// Ported from plMetalPipeline::Resize (plMetalPipeline.cpp:771-814).
void plVulkanPipeline::Resize(uint32_t width, uint32_t height)
{
    // The view transform has to be put back afterwards, so hold a copy.
    plViewTransform resetTransform = GetViewTransform();

    if (width != 0 && height != 0) {
        // Zero means "just recreate at the current size".
        IGetViewTransform().SetScreenSize(uint16_t(width), uint16_t(height));
        resetTransform.SetScreenSize(uint16_t(width), uint16_t(height));
    }

    // Rebuilds the swapchain and the depth buffer on the next BeginFrame.
    fDevice.Resize(width, height);

    // Restore, then push the new projection down. Without this the aspect ratio
    // stays at whatever the window was created with.
    SetViewTransform(resetTransform);
    IProjectionMatrixToDevice();

    plVirtualCam1::Refresh();

    // Anything holding device-specific resources needs to know they may have
    // gone away underneath it.
    plDeviceRecreateMsg* clean = new plDeviceRecreateMsg(this);
    plgDispatch::MsgSend(clean);
}

bool plVulkanPipeline::IsFullScreen() const
{
    return fIsFullscreen;
}

void plVulkanPipeline::GetSupportedDisplayModes(std::vector<plDisplayMode>* res, int ColorDepth)
{
    plDisplayHelper* helper = plDisplayHelper::GetInstance();
    if (!helper || !res)
        return;

    *res = helper->GetSupportedDisplayModes(helper->DefaultDisplay(), ColorDepth);
}

/*** Screen elements ********************************************************/

void plVulkanPipeline::RenderScreenElements()
{
    bool resetTransforms = false;

    if (fView.HasCullProxy())
        Draw(fView.GetCullProxy());

    const hsGMatState wireframe =
        PushMaterialOverride(hsGMatState::kMisc, hsGMatState::kMiscWireFrame, false);
    const hsGMatState white =
        PushMaterialOverride(hsGMatState::kShade, hsGMatState::kShadeWhite, true);

    if (fPlateMgr) {
        fPlateMgr->DrawToDevice(this);
        resetTransforms = true;
    }

    PopMaterialOverride(white, true);
    PopMaterialOverride(wireframe, false);

    if (fDebugTextMgr && plDebugText::Instance().IsEnabled()) {
        fDebugTextMgr->DrawToDevice(this);
        resetTransforms = true;
    }

    if (resetTransforms)
        fView.fXformResetFlags = fView.kResetAll;

    fState.Reset();
}

plTextFont* plVulkanPipeline::MakeTextFont(ST::string face, uint16_t size)
{
    plTextFont* font = new plVulkanTextFont(this, &fDevice);
    font->Create(std::move(face), size);
    font->Link(&fTextFontRefList);

    return font;
}

/*** Geometry refs, ported from plMetalPipeline.cpp:4310-4372 ****************/

void plVulkanPipeline::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    plVulkanVertexBufferRef* vRef =
        static_cast<plVulkanVertexBufferRef*>(owner->GetVertexBufferRef(idx));

    if (!vRef) {
        vRef = new plVulkanVertexBufferRef();
        fDevice.SetupVertexBufferRef(owner, idx, vRef);
    }

    if (!vRef->IsLinked())
        vRef->Link(&fVtxBuffRefList);

    // Volatile buffers are held off deliberately: the span may not even be
    // visible, and an overrun of the dynamic buffer would mean refilling again.
    if (!vRef->Volatile()) {
        fDevice.CheckStaticVertexBuffer(vRef, owner, idx);
        hsAssert(!vRef->IsDirty(), "Non-volatile vertex buffers should never get dirty");
    } else if (!vRef->fData && (vRef->fFormat != owner->GetVertexFormat())) {
        // The ref's format differs from the group's, which means the skinning
        // data has to be stripped out on the way in. Give it somewhere to land.
        vRef->fData = new uint8_t[vRef->fCount * vRef->fVertexSize];
        fDevice.FillVolatileVertexBufferRef(vRef, owner, idx);
    }
}

void plVulkanPipeline::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    plVulkanIndexBufferRef* iRef =
        static_cast<plVulkanIndexBufferRef*>(owner->GetIndexBufferRef(idx));

    if (!iRef) {
        iRef = new plVulkanIndexBufferRef();
        fDevice.SetupIndexBufferRef(owner, idx, iRef);
    }

    if (!iRef->IsLinked())
        iRef->Link(&fIdxBuffRefList);

    fDevice.CheckIndexBuffer(iRef);

    if (iRef->IsDirty())
        fDevice.FillIndexBufferRef(iRef, owner, idx);
}

/*** Texture refs, ported from plMetalPipeline.cpp:4246-4306 *****************/

void plVulkanPipeline::CheckTextureRef(plLayerInterface* layer)
{
    plBitmap* bitmap = layer->GetTexture();
    if (bitmap)
        CheckTextureRef(bitmap);
}

void plVulkanPipeline::CheckTextureRef(plBitmap* bitmap)
{
    // Render targets have their own device-ref type and are already GPU images.
    // Treating one as plVulkanTextureRef happens to find the image view, but
    // reads the cookie and cubic flag from unrelated render-target fields.
    if (plRenderTarget* target = plRenderTarget::ConvertNoRef(bitmap)) {
        auto* targetRef = static_cast<plVulkanRenderTargetRef*>(target->GetDeviceRef());
        if (!targetRef || targetRef->IsDirty() || targetRef->fImageView == VK_NULL_HANDLE)
            MakeRenderTargetRef(target);
        return;
    }

    plVulkanTextureRef* tRef = static_cast<plVulkanTextureRef*>(bitmap->GetDeviceRef());

    if (!tRef)
        tRef = static_cast<plVulkanTextureRef*>(MakeTextureRef(bitmap));

    if (tRef && tRef->IsDirty())
        IReloadTexture(bitmap, tRef);
}

hsGDeviceRef* plVulkanPipeline::MakeTextureRef(plBitmap* bitmap)
{
    plVulkanTextureRef* tRef = static_cast<plVulkanTextureRef*>(bitmap->GetDeviceRef());

    if (!tRef) {
        tRef = new plVulkanTextureRef();
        fDevice.SetupTextureRef(bitmap, tRef);
    }

    if (!tRef->IsLinked())
        tRef->Link(&fTextureRefList);

    fDevice.CheckTexture(tRef);

    if (tRef->IsDirty())
        IReloadTexture(bitmap, tRef);

    return tRef;
}

void plVulkanPipeline::IReloadTexture(plBitmap* bitmap, plVulkanTextureRef* ref)
{
    plMipmap* mip = plMipmap::ConvertNoRef(bitmap);
    if (mip) {
        fDevice.MakeTextureRef(ref, mip);
        return;
    }

    plCubicEnvironmap* cubic = plCubicEnvironmap::ConvertNoRef(bitmap);
    if (cubic) {
        fDevice.MakeCubicTextureRef(ref, cubic);
        return;
    }
}

// Runtime read/write access to a span's vertices, used by dynamic decals to find
// the surface they are stamping onto. Metal does not implement it either
// (plMetalPipeline.cpp:331-336); decals fall back to their own bookkeeping.
bool plVulkanPipeline::OpenAccess(plAccessSpan& dst, plDrawableSpans* d,
                                  const plVertexSpan* span, bool readOnly)
{
    if (!d || !span) {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    plGBufferGroup* group = d->GetBufferGroup(span->fGroupIdx);
    if (!group || group->AreVertsVolatile()) {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    CheckVertexBufferRef(group, span->fVBufferIdx);
    auto* ref = static_cast<plVulkanVertexBufferRef*>(
        group->GetVertexBufferRef(span->fVBufferIdx));
    if (!ref || !ref->GetBuffer().fMapped || span->fVLength == 0) {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    if (!readOnly)
        fDevice.WaitForIdle();

    const uint32_t stride = ref->fVertexSize;
    uint8_t* ptr = static_cast<uint8_t*>(ref->GetBuffer().fMapped) +
                   size_t(span->fVStartIdx) * stride;
    const int32_t offset = -int32_t(span->fVStartIdx) * int32_t(stride);
    const uint8_t numWeights = (ref->fFormat & plGBufferGroup::kSkinWeightMask) >> 4;

    plAccessVtxSpan& access = dst.AccessVtx();
    access.SetVertCount(uint16_t(span->fVLength));
    access.PositionStream(ptr, uint16_t(stride), offset);
    ptr += sizeof(hsPoint3);

    access.SetNumWeights(numWeights);
    if (numWeights) {
        access.WeightStream(ptr, uint16_t(stride), offset);
        ptr += numWeights * sizeof(float);
        if (ref->fFormat & plGBufferGroup::kSkinIndices) {
            access.WgtIndexStream(ptr, uint16_t(stride), offset);
            ptr += sizeof(uint32_t);
        } else {
            access.WgtIndexStream(nullptr, 0, offset);
        }
    }

    access.NormalStream(ptr, uint16_t(stride), offset);
    ptr += sizeof(hsVector3);
    access.DiffuseStream(ptr, uint16_t(stride), offset);
    ptr += sizeof(uint32_t);
    access.SpecularStream(ptr, uint16_t(stride), offset);
    ptr += sizeof(uint32_t);
    access.UVWStream(ptr, uint16_t(stride), offset);
    access.SetNumUVWs(plGBufferGroup::CalcNumUVs(ref->fFormat));
    access.SetVtxDeviceRef(ref);
    return true;
}

bool plVulkanPipeline::CloseAccess(plAccessSpan& acc)
{
    return acc.HasAccessVtx() && acc.AccessVtx().GetVtxDeviceRef() != nullptr;
}

// Ported from plMetalPipeline::PushRenderRequest (plMetalPipeline.cpp:340-379).
void plVulkanPipeline::PushRenderRequest(plRenderRequest* req)
{
    // The request's view transform replaces ours, but its local-to-world has to
    // survive, so hold it across the swap.
    hsMatrix44 l2w = fView.GetLocalToWorld();
    hsMatrix44 w2l = fView.GetWorldToLocal();

    fViewStack.push(fView);

    SetViewTransform(req->GetViewTransform());
    PushRenderTarget(req->GetRenderTarget());

    fView.fRenderRequest = req;
    fView.fRenderRequest->Ref();
    fView.fRenderState = req->GetRenderState();
    fView.SetDrawableTypeMask(req->GetDrawableMask());
    fView.SetSubDrawableTypeMask(req->GetSubDrawableMask());

    float depth = req->GetClearDepth();
    SetClear(&req->GetClearColor(), &depth);

    if (req->GetOverrideMat())
        PushOverrideMaterial(req->GetOverrideMat());

    ISetLocalToWorld(l2w, w2l);
    RefreshMatrices();

    if (req->GetIgnoreOccluders())
        SetMaxCullNodes(0);

    // A new target means none of the cached bindings survive.
    fState.Reset();
}

// Ported from plMetalPipeline::PopRenderRequest (plMetalPipeline.cpp:381-399).
void plVulkanPipeline::PopRenderRequest(plRenderRequest* req)
{
    if (req->GetOverrideMat())
        PopOverrideMaterial(nullptr);

    fState.Reset();

    hsRefCnt_SafeUnRef(fView.fRenderRequest);
    fView = fViewStack.top();
    fViewStack.pop();

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;
}

plRenderTarget* plVulkanPipeline::PopRenderTarget()
{
    plRenderTarget* target = pl3DPipeline::PopRenderTarget();
    fState.Reset();
    return target;
}

hsGDeviceRef* plVulkanPipeline::MakeRenderTargetRef(plRenderTarget* owner)
{
    if (!owner)
        return nullptr;

    // A planar reflection is sampled in screen space, so its attachment and
    // viewport must have the same dimensions as the view that samples it.
    if (plDynamicCamMap* camMap = plDynamicCamMap::ConvertNoRef(owner);
        camMap && camMap->IsReflection()) {
        camMap->ResizeViewport(IGetViewTransform());
    }

    // A face asks through its parent, since the six of them share one image.
    if (plCubicRenderTarget* parent = owner->GetParent()) {
        MakeRenderTargetRef(parent);
        return owner->GetDeviceRef();
    }

    auto refFor = [this](plRenderTarget* target) {
        plVulkanRenderTargetRef* ref =
            static_cast<plVulkanRenderTargetRef*>(target->GetDeviceRef());

        if (!ref) {
            ref = new plVulkanRenderTargetRef();
            ref->fOwner = target;
            target->SetDeviceRef(ref);
            ref->UnRef();
        }

        if (!ref->IsLinked())
            ref->Link(&fRenderTargetRefList);

        return ref;
    };

    plVulkanRenderTargetRef* ref = refFor(owner);
    const bool wantsDepth = owner->GetZDepth() != 0;

    if (plCubicRenderTarget* cubic = plCubicRenderTarget::ConvertNoRef(owner)) {
        plVulkanRenderTargetRef* faces[6];
        for (uint32_t i = 0; i < 6; i++)
            faces[i] = refFor(cubic->GetFace(i));

        if (!fDevice.CreateCubicRenderTarget(ref, faces, owner->GetWidth(), wantsDepth,
                                             owner->GetKeyName()))
            return nullptr;

        return ref;
    }

    if (!fDevice.CreateRenderTarget(ref, owner->GetWidth(), owner->GetHeight(), wantsDepth,
                                    owner->GetKeyName()))
        return nullptr;

    return ref;
}

// Ported from plMetalPipeline::LoadResources (:2967-3012). Called when
// plPipeResReq asks, which is what CheckResources decides.
void plVulkanPipeline::LoadResources()
{
    hsStatusMessageF("Begin Device Reload t={}", hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("Begin Device Reload");

    while (fActiveLights)
        UnRegisterLight(fActiveLights);

    while (fLightRefList) {
        plVulkanLightRef* ref = fLightRefList;
        ref->Unlink();
        ref->Release();
    }

    if (plVulkanPlateManager* pm = static_cast<plVulkanPlateManager*>(fPlateMgr))
        pm->IReleaseGeometry();

    IReleaseAvRTPool();

    // Anything holding a render target ref recreates it in response to this.
    plPipeRTMakeMsg* rtMake = new plPipeRTMakeMsg(this);
    rtMake->Send();

    if (plVulkanPlateManager* pm = static_cast<plVulkanPlateManager*>(fPlateMgr))
        pm->ICreateGeometry();

    plPipeGeoMakeMsg* defMake = new plPipeGeoMakeMsg(this, true);
    defMake->Send();

    // Last of the large allocations: this one takes whatever is left, and it
    // takes more when more is available.
    IFillAvRTPool();

    plPipeGeoMakeMsg* manMake = new plPipeGeoMakeMsg(this, false);
    manMake->Send();

    plPipeResReq::Clear();

    hsStatusMessageF("End Device Reload t={}", hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("End Device Reload");
}

bool plVulkanPipeline::SetGamma(float eR, float eG, float eB)
{
    uint16_t tabR[256];
    uint16_t tabG[256];
    uint16_t tabB[256];
    tabR[0] = tabG[0] = tabB[0] = 0;

    constexpr float kMinExponent = 0.1f;
    eR = 1.f / std::max(eR, kMinExponent);
    eG = 1.f / std::max(eG, kMinExponent);
    eB = 1.f / std::max(eB, kMinExponent);
    for (uint32_t i = 1; i < 256; ++i) {
        const float input = float(i) / 255.f;
        tabR[i] = uint16_t(std::pow(input, eR) * 65535.f);
        tabG[i] = uint16_t(std::pow(input, eG) * 65535.f);
        tabB[i] = uint16_t(std::pow(input, eB) * 65535.f);
    }

    return SetGamma(tabR, tabG, tabB);
}

bool plVulkanPipeline::SetGamma(const uint16_t* const tabR, const uint16_t* const tabG,
                                const uint16_t* const tabB)
{
    return fDevice.SetGammaLUT(tabR, tabG, tabB, 256);
}

bool plVulkanPipeline::SetGamma10(const uint16_t* const tabR,
                                  const uint16_t* const tabG,
                                  const uint16_t* const tabB)
{
    return fDevice.SetGammaLUT(tabR, tabG, tabB, 1024);
}

bool plVulkanPipeline::CaptureScreen(plMipmap* dest, bool flipVertical, uint16_t desiredWidth,
                                     uint16_t desiredHeight)
{
    if (!dest)
        return false;

    const uint32_t width = fDevice.Width();
    const uint32_t height = fDevice.Height();
    if (width == 0 || height == 0)
        return false;

    std::vector<uint32_t> pixels(size_t(width) * height);
    uint32_t capturedWidth = 0;
    uint32_t capturedHeight = 0;
    if (!fDevice.ReadSwapchainImage(pixels.data(), pixels.size() * sizeof(uint32_t),
                                    &capturedWidth, &capturedHeight)) {
        return false;
    }

    if (dest->GetWidth() != capturedWidth || dest->GetHeight() != capturedHeight ||
        dest->GetPixelSize() != 32) {
        dest->Reset();
        dest->Create(uint16_t(capturedWidth), uint16_t(capturedHeight),
                     plMipmap::kARGB32Config, 1);
    }

    for (uint32_t y = 0; y < capturedHeight; y++) {
        const uint32_t sourceY = flipVertical ? capturedHeight - 1 - y : y;
        uint32_t* output = dest->GetAddr32(0, y);
        const uint32_t* input = pixels.data() + size_t(sourceY) * capturedWidth;
        for (uint32_t x = 0; x < capturedWidth; x++)
            output[x] = input[x] | 0xff000000;
    }

    if (desiredWidth != 0 && desiredHeight != 0)
        return dest->ResizeNicely(desiredWidth, desiredHeight, plMipmap::kDefaultFilter);

    return true;
}

// Ported from plMetalPipeline::ExtractMipMap (:3466-3502).
plMipmap* plVulkanPipeline::ExtractMipMap(plRenderTarget* targ)
{
    if (!targ || plCubicRenderTarget::ConvertNoRef(targ))
        return nullptr;

    if (targ->GetPixelSize() != 32) {
        hsAssert(false, "Only RGBA8888 currently implemented");
        return nullptr;
    }

    plVulkanRenderTargetRef* ref = static_cast<plVulkanRenderTargetRef*>(targ->GetDeviceRef());
    if (!ref)
        return nullptr;

    const uint32_t width = targ->GetWidth();
    const uint32_t height = targ->GetHeight();

    plMipmap* mipMap = new plMipmap(width, height, plMipmap::kARGB32Config, 1);

    // The target is B8G8R8A8_UNORM and plMipmap's kARGB32Config is the same bytes
    // in the same order, so this needs no swizzle.
    if (!fDevice.ReadRenderTarget(ref, mipMap->GetAddr32(0, 0),
                                  size_t(width) * height * 4)) {
        delete mipMap;
        return nullptr;
    }

    // Render targets are drawn without an opaque background, so alpha comes back
    // as whatever was blended. Callers want an opaque image.
    for (uint32_t y = 0; y < height; y++) {
        uint32_t* row = mipMap->GetAddr32(0, y);
        for (uint32_t x = 0; x < width; x++)
            row[x] |= 0xff000000;
    }

    return mipMap;
}

int plVulkanPipeline::GetMaxAnisotropicSamples()
{
    return int(fDevice.MaxAnisotropy());
}

int plVulkanPipeline::GetMaxAntiAlias(int Width, int Height, int ColorDepth)
{
    return int(fDevice.MaxSampleCount());
}

void plVulkanPipeline::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed,
                                          int NumAASamples, int MaxAnisotropicSamples, bool vSync)
{
    fIsFullscreen = !Windowed;
    fVSync = vSync;
    Resize(uint32_t(Width), uint32_t(Height));
    fDevice.SetAnisotropy(uint32_t(MaxAnisotropicSamples));
    fDevice.SetMSAASampleCount(uint32_t(std::max(1, NumAASamples)));
    fDevice.SetVSync(vSync);
}
