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
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//  plGLPipeline Class Functions                                             //
//  plPipeline derivative for OpenGL                                         //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <epoxy/gl.h>
#include <string_theory/string>

#include "HeadSpin.h"
#include "hsWindows.h"

#include "plGLMaterialShaderRef.h"
#include "plGLPipeline.h"
#include "plGLPlateManager.h"

#include "hsGMatState.inl"
#include "plPipeDebugFlags.h"
#include "plPipeResReq.h"
#include "plProfile.h"
#include "pnMessage/plPipeResMakeMsg.h"
#include "plGLight/plLightInfo.h"
#include "plPipeline/hsWinRef.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plDebugText.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plStatusLog/plStatusLog.h"
#include "plSurface/plLayer.h"

#ifdef HS_SIMD_INCLUDE
#  include HS_SIMD_INCLUDE
#endif

typedef plGLDevice DeviceType;

plProfile_Extern(DrawFeedTriangles);
plProfile_Extern(DrawPrimStatic);
plProfile_Extern(TotalTexSize);
plProfile_Extern(LayChange);
plProfile_Extern(DrawTriangles);
plProfile_Extern(MatChange);
plProfile_Extern(RenderSpan);
plProfile_Extern(MergeCheck);
plProfile_Extern(MergeSpan);
plProfile_Extern(SpanTransforms);
plProfile_Extern(SpanFog);
plProfile_Extern(SelectLights);
plProfile_Extern(SelectProj);
plProfile_Extern(CheckDyn);
plProfile_Extern(CheckStat);
plProfile_Extern(RenderBuff);
plProfile_Extern(RenderPrim);
plProfile_Extern(PlateMgr);
plProfile_Extern(DebugText);
plProfile_Extern(Reset);

// Adding a nil RenderPrim for turning off drawing
static plRenderNilFunc sRenderNil;

class plGLRenderTriListFunc : public plRenderTriListFunc<plGLDevice>
{
public:
    plGLRenderTriListFunc(plGLDevice* device, int baseVertexIndex, int vStart, int vLength, int iStart, int iNumTris)
        : plRenderTriListFunc(device, baseVertexIndex, vStart, vLength, iStart, iNumTris) {}

    bool RenderPrims() const override
    {
        plProfile_IncCount(DrawFeedTriangles, fNumTris);
        plProfile_IncCount(DrawTriangles, fNumTris);
        plProfile_Inc(DrawPrimStatic);

        glDrawElements(GL_TRIANGLES, fNumTris, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16_t) * fIStart));
        return true; // TODO: Check for GL Error
    }
};


plGLEnumerate plGLPipeline::enumerator;

plGLPipeline::plGLPipeline(hsDisplayHndl display, hsWindowHndl window, const hsG3DDeviceModeRecord *devMode)
    : pl3DPipeline(devMode), fMatRefList(), fRenderTargetRefList()
{
    plStatusLog::AddLineS("pipeline.log", "Constructing plGLPipeline");
    plStatusLog::AddLineSF("pipeline.log", "Driver vendor: {}", devMode->GetDevice()->GetDriverDesc());

    fDevice.fWindow = window;
    fDevice.fDevice = display;
    fDevice.fPipeline = this;

    fPlateMgr = new plGLPlateManager(this);
}

plGLPipeline::~plGLPipeline()
{
    if (plGLPlateManager* pm = static_cast<plGLPlateManager*>(fPlateMgr))
        pm->IReleaseGeometry();

    fDevice.Shutdown();
}

bool plGLPipeline::PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(drawable);
    if (!ds)
        return false;

    if ((ds->GetType() & fView.GetDrawableTypeMask()) == 0)
        return false;

    fView.GetVisibleSpans(ds, visList, visMgr);

    // TODO: Implement bounds debugging stuff here

    return !visList.empty();
}

bool plGLPipeline::PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    plDrawableSpans* ice = plDrawableSpans::ConvertNoRef(drawable);
    if (!ice)
        return false;

    // Find our lights
    ICheckLighting(ice, visList, visMgr);

    // Sort our faces
    if (ice->GetNativeProperty(plDrawable::kPropSortFaces))
        ice->SortVisibleSpans(visList, this);

    // Prep for render. This is gives the drawable a chance to
    // do any last minute updates for its buffers, including
    // generating particle tri lists.
    ice->PrepForRender(this);

    // Other stuff that we're ignoring for now...

    return true;
}

plTextFont* plGLPipeline::MakeTextFont(ST::string face, uint16_t size)
{
    return nullptr;
}

bool plGLPipeline::OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly)
{
    return false;
}

bool plGLPipeline::CloseAccess(plAccessSpan& acc)
{
    return false;
}

void plGLPipeline::PushRenderRequest(plRenderRequest* req)
{
    // Save these, since we want to copy them to our current view
    hsMatrix44 l2w = fView.GetLocalToWorld();
    hsMatrix44 w2l = fView.GetWorldToLocal();

    plFogEnvironment defFog = fView.GetDefaultFog();

    fViewStack.push(fView);

    SetViewTransform(req->GetViewTransform());

    PushRenderTarget(req->GetRenderTarget());
    fView.fRenderState = req->GetRenderState();

    fView.fRenderRequest = req;
    hsRefCnt_SafeRef(fView.fRenderRequest);

    SetDrawableTypeMask(req->GetDrawableMask());
    SetSubDrawableTypeMask(req->GetSubDrawableMask());

    float depth = req->GetClearDepth();
    fView.SetClear(&req->GetClearColor(), &depth);

#if 0
    if (req->GetFogStart() < 0)
    {
        fView.SetDefaultFog(defFog);
    }
    else
    {
        defFog.Set(req->GetYon() * (1.f - req->GetFogStart()), req->GetYon(), 1.f, &req->GetClearColor());
        fView.SetDefaultFog(defFog);
        fCurrFog.fEnvPtr = nullptr;
    }
#endif

    if (req->GetOverrideMat())
        PushOverrideMaterial(req->GetOverrideMat());

    // Set from our saved ones...
    fView.SetWorldToLocal(w2l);
    fView.SetLocalToWorld(l2w);

    RefreshMatrices();

    if (req->GetIgnoreOccluders())
        fView.SetMaxCullNodes(0);

    fView.fCullTreeDirty = true;
}

void plGLPipeline::PopRenderRequest(plRenderRequest* req)
{
    if (req->GetOverrideMat())
        PopOverrideMaterial(nullptr);

    hsRefCnt_SafeUnRef(fView.fRenderRequest);
    fView = fViewStack.top();
    fViewStack.pop();

#if 0
    // Force the next thing drawn to update the fog settings.
    fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    fCurrFog.fEnvPtr = nullptr;
#endif

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;
}

void plGLPipeline::ClearRenderTarget(plDrawable* d)
{
    plDrawableSpans* src = plDrawableSpans::ConvertNoRef(d);

    ClearRenderTarget();

    if (!src)
        return;
}

void plGLPipeline::ClearRenderTarget(const hsColorRGBA* col, const float* depth)
{
    if (fView.fRenderState & (kRenderClearColor | kRenderClearDepth)) {
        hsColorRGBA clearColor = col ? *col : GetClearColor();
        float clearDepth = depth ? *depth : fView.GetClearDepth();

        GLuint masks = 0;
        if (fView.fRenderState & kRenderClearColor)
            masks |= GL_COLOR_BUFFER_BIT;
        if (fView.fRenderState & kRenderClearDepth)
            masks |= GL_DEPTH_BUFFER_BIT;

        glDepthMask(GL_TRUE);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClearDepth(1.0);

        glClear(masks);
    }
}

hsGDeviceRef* plGLPipeline::MakeRenderTargetRef(plRenderTarget* owner)
{
    plGLRenderTargetRef* ref = nullptr;
    GLuint depthBuffer = -1;

    // If we have Shader Model 3 and support non-POT textures, let's make reflections the pipe size
#if 1
    if (plDynamicCamMap* camMap = plDynamicCamMap::ConvertNoRef(owner)) {
        //if ((plQuality::GetCapability() > plQuality::kPS_2) && fSettings.fD3DCaps & kCapsNpotTextures)
            camMap->ResizeViewport(IGetViewTransform());
    }
#endif

    /// Check--is this renderTarget really a child of a cubicRenderTarget?
    if (owner->GetParent()) {
        /// This'll create the deviceRefs for all of its children as well
        MakeRenderTargetRef(owner->GetParent());
        return owner->GetDeviceRef();
    }

    // If we already have a rendertargetref, we just need it filled out with D3D resources.
    if (owner->GetDeviceRef())
        ref = (plGLRenderTargetRef*)owner->GetDeviceRef();

    /// Create the render target now
    // Start with the depth surface.
    // Note that we only ever give a cubic rendertarget a single shared depth buffer,
    // since we only render one face at a time. If we were rendering part of face X, then part
    // of face Y, then more of face X, then they would all need their own depth buffers.
    if (owner->GetZDepth() && (owner->GetFlags() & (plRenderTarget::kIsTexture | plRenderTarget::kIsOffscreen))) {
        if (epoxy_gl_version() >= 45) {
            glCreateRenderbuffers(1, &depthBuffer);
            glNamedRenderbufferStorage(depthBuffer, GL_DEPTH24_STENCIL8, owner->GetWidth(), owner->GetHeight());
        } else {
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, owner->GetWidth(), owner->GetHeight());
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
        }
    }

    // See if it's a cubic render target.
    // Primary consumer here is the vertex/pixel shader water.
    if (plCubicRenderTarget* cubicRT = plCubicRenderTarget::ConvertNoRef(owner)) {
        /// And create the ref (it'll know how to set all the flags)
        //if (ref)
        //    ref->Set(surfFormat, 0, owner);
        //else
        //    ref = new plGLRenderTargetRef(surfFormat, 0, owner);

        // TODO: The rest
    }

    // Not a cubic, is it a texture render target? These are currently used
    // primarily for shadow map generation.
    else if (owner->GetFlags() & plRenderTarget::kIsTexture) {
        /// Create a normal texture
        if (!ref)
            ref = new plGLRenderTargetRef();

        ref->fOwner = owner;
        ref->fDepthBuffer = depthBuffer;
        ref->fMapping = GL_TEXTURE_2D;

        if (epoxy_gl_version() >= 45) {
            glCreateTextures(GL_TEXTURE_2D, 1, &ref->fRef);
            glTextureStorage2D(ref->fRef, 1, GL_RGBA8, owner->GetWidth(), owner->GetHeight());
            glTextureParameteri(ref->fRef, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(ref->fRef, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glCreateFramebuffers(1, &ref->fFrameBuffer);
            glNamedFramebufferTexture(ref->fFrameBuffer, GL_COLOR_ATTACHMENT0, ref->fRef, 0);
            if (ref->fDepthBuffer != -1)
                glNamedFramebufferRenderbuffer(ref->fFrameBuffer, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ref->fDepthBuffer);
        } else {
            glGenTextures(1, &ref->fRef);
            glBindTexture(GL_TEXTURE_2D, ref->fRef);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, owner->GetWidth(), owner->GetHeight());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            glGenFramebuffers(1, &ref->fFrameBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, ref->fFrameBuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ref->fRef, 0);
            if (ref->fDepthBuffer != -1)
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, ref->fDepthBuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    // Not a texture either, must be a plain offscreen.
    // Offscreen isn't currently used for anything.
    else if (owner->GetFlags() & plRenderTarget::kIsOffscreen) {
        /// Create a blank surface
        //if (ref)
        //    ref->Set(surfFormat, 0, owner);
        //else
        //    ref = new plGLRenderTargetRef(surfFormat, 0, owner);
    }

    // Keep it in a linked list for ready destruction.
    if (owner->GetDeviceRef() != ref) {
        owner->SetDeviceRef(ref);
        // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
        hsRefCnt_SafeUnRef(ref);
        if (ref != nullptr && !ref->IsLinked())
            ref->Link(&fRenderTargetRefList);
    } else {
        if (ref != nullptr && !ref->IsLinked())
            ref->Link(&fRenderTargetRefList);
    }

    // Mark as not dirty so it doesn't get re-created
    if (ref != nullptr)
        ref->SetDirty(false);

    return ref;
}

bool plGLPipeline::BeginRender()
{
    // TODO: Device Init/Reset stuff here

    // offset transform
    RefreshScreenMatrices();

    // If this is the primary BeginRender, make sure we're really ready.
    if (fInSceneDepth++ == 0) {
        fDevice.BeginRender();

        fVtxRefTime++;

        hsColorRGBA clearColor = GetClearColor();

        glDepthMask(GL_TRUE);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClearDepth(1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    fRenderCnt++;

    // Would probably rather this be an input.
    fTime = hsTimer::GetSysSeconds();

    return false;
}

bool plGLPipeline::EndRender()
{
    bool retVal = false;

    // Actually end the scene
    if (--fInSceneDepth == 0) {
        retVal = fDevice.EndRender();

        IClearShadowSlaves();
    }

    // Do this last, after we've drawn everything
    // Just letting go of things we're done with for the frame.
    hsRefCnt_SafeUnRef(fCurrMaterial);
    fCurrMaterial = nullptr;

    for (int i = 0; i < 8; i++) {
        if (fLayerRef[i]) {
            hsRefCnt_SafeUnRef(fLayerRef[i]);
            fLayerRef[i] = nullptr;
        }
    }

    return retVal;
}

void plGLPipeline::RenderScreenElements()
{
    bool reset = false;

    if (fView.HasCullProxy())
        Draw(fView.GetCullProxy());

    hsGMatState tHack = PushMaterialOverride(hsGMatState::kMisc, hsGMatState::kMiscWireFrame, false);
    hsGMatState ambHack = PushMaterialOverride(hsGMatState::kShade, hsGMatState::kShadeWhite, true);

    plProfile_BeginTiming(PlateMgr);
    // Plates
    if (fPlateMgr) {
        fPlateMgr->DrawToDevice(this);
        reset = true;
    }
    plProfile_EndTiming(PlateMgr);

    PopMaterialOverride(ambHack, true);
    PopMaterialOverride(tHack, false);

    plProfile_BeginTiming(DebugText);
    /// Debug text
    if (fDebugTextMgr && plDebugText::Instance().IsEnabled()) {
        fDebugTextMgr->DrawToDevice(this);
        reset = true;
    }
    plProfile_EndTiming(DebugText);

    plProfile_BeginTiming(Reset);
    if (reset)
        fView.fXformResetFlags = fView.kResetAll; // Text destroys view transforms
    plProfile_EndTiming(Reset);
}

bool plGLPipeline::IsFullScreen() const
{
    return false;
}

void plGLPipeline::Resize(uint32_t width, uint32_t height)
{}

void plGLPipeline::LoadResources()
{
    if (plGLPlateManager* pm = static_cast<plGLPlateManager*>(fPlateMgr)) {
        pm->IReleaseGeometry();
    }
}

bool plGLPipeline::SetGamma(float eR, float eG, float eB)
{
    return false;
}

bool plGLPipeline::SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB)
{
    return false;
}

bool plGLPipeline::CaptureScreen(plMipmap* dest, bool flipVertical, uint16_t desiredWidth, uint16_t desiredHeight)
{
    return false;
}

plMipmap* plGLPipeline::ExtractMipMap(plRenderTarget* targ)
{
    return nullptr;
}

void plGLPipeline::GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth)
{}

int plGLPipeline::GetMaxAnisotropicSamples()
{
    return 0;
}

int plGLPipeline::GetMaxAntiAlias(int Width, int Height, int ColorDepth)
{
    return 0;
}

void plGLPipeline::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync)
{}

void plGLPipeline::RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList)
{
    plProfile_BeginTiming(RenderSpan);

    hsMatrix44 lastL2W;
    hsGMaterial* material;
    const std::vector<plSpan*>& spans = ice->GetSpanArray();

    //plProfile_IncCount(EmptyList, visList.empty());

    /// Set this (*before* we do our TestVisibleWorld stuff...)
    lastL2W.Reset();
    ISetLocalToWorld(lastL2W, lastL2W);   // This is necessary; otherwise, we have to test for
                                          // the first transform set, since this'll be identity
                                          // but the actual device transform won't be (unless
                                          // we do this)

    /// Loop through our spans, combining them when possible
    for (size_t i = 0; i < visList.size(); )
    {
        if (GetOverrideMaterial() != nullptr)
            material = GetOverrideMaterial();
        else
            material = ice->GetMaterial(spans[visList[i]]->fMaterialIdx);

        /// It's an icicle--do our icicle merge loop
        plIcicle tempIce(*((plIcicle*)spans[visList[i]]));

        // Start at i + 1, look for as many spans as we can add to tempIce
        size_t j;
        for (j = i + 1; j < visList.size(); j++)
        {
            if (GetOverrideMaterial())
                tempIce.fMaterialIdx = spans[visList[j]]->fMaterialIdx;

            plProfile_BeginTiming(MergeCheck);
            if (!spans[visList[j]]->CanMergeInto(&tempIce)) {
                plProfile_EndTiming(MergeCheck);
                break;
            }
            plProfile_EndTiming(MergeCheck);
            //plProfile_Inc(SpanMerge);

            plProfile_BeginTiming(MergeSpan);
            spans[visList[j]]->MergeInto(&tempIce);
            plProfile_EndTiming(MergeSpan);
        }

        LOG_GL_ERROR_CHECK("RenderSpans pre-material failed");

        if (material != nullptr) {
            // First, do we have a device ref at this index?
            plGLMaterialShaderRef* mRef = static_cast<plGLMaterialShaderRef*>(material->GetDeviceRef());

            if (mRef == nullptr) {
                mRef = new plGLMaterialShaderRef(material, this);
                material->SetDeviceRef(mRef);
            }

            if (!mRef->IsLinked())
                mRef->Link(&fMatRefList);

            glUseProgram(mRef->fRef);
            fDevice.fCurrentProgram = mRef->fRef;
            LOG_GL_ERROR_CHECK(ST::format("Use Program with material \"{}\" failed", material->GetKeyName()));

            GLuint vao = 0;
            if (epoxy_gl_version() >= 30) {
                // TODO: Figure out how to use VAOs properly :(
                glGenVertexArrays(1, &vao);
                glBindVertexArray(vao);
            }

            // What do we change?

            plProfile_BeginTiming(SpanTransforms);
            ISetupTransforms(ice, tempIce, mRef, lastL2W);
            plProfile_EndTiming(SpanTransforms);

            // Check that the underlying buffers are ready to go.
            plProfile_BeginTiming(CheckDyn);
            ICheckDynBuffers(ice, ice->GetBufferGroup(tempIce.fGroupIdx), &tempIce);
            plProfile_EndTiming(CheckDyn);

            plProfile_BeginTiming(CheckStat);
            plGBufferGroup* grp = ice->GetBufferGroup(tempIce.fGroupIdx);
            CheckVertexBufferRef(grp, tempIce.fVBufferIdx);
            CheckIndexBufferRef(grp, tempIce.fIBufferIdx);
            plProfile_EndTiming(CheckStat);

            // Draw this span now
            IRenderBufferSpan( tempIce,
                                ice->GetVertexRef( tempIce.fGroupIdx, tempIce.fVBufferIdx ),
                                ice->GetIndexRef( tempIce.fGroupIdx, tempIce.fIBufferIdx ),
                                material,
                                tempIce.fVStartIdx, tempIce.fVLength,   // These are used as our accumulated range
                                tempIce.fIPackedIdx, tempIce.fILength );

            if (epoxy_gl_version() >= 30)
                glDeleteVertexArrays(1, &vao);
        }

        // Restart our search...
        i = j;
    }

    plProfile_EndTiming(RenderSpan);
    /// All done!
}


void plGLPipeline::ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, plGLMaterialShaderRef* mRef, hsMatrix44& lastL2W)
{
    if (span.fNumMatrices) {
        if (span.fNumMatrices <= 2) {
            ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);
            lastL2W = span.fLocalToWorld;
        } else {
            lastL2W.Reset();
            ISetLocalToWorld(lastL2W, lastL2W);
            fView.fLocalToWorldLeftHanded = span.fLocalToWorld.GetParity();
        }
    } else if (lastL2W != span.fLocalToWorld) {
        ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);
        lastL2W = span.fLocalToWorld;
    } else {
        fView.fLocalToWorldLeftHanded = lastL2W.GetParity();
    }

#if 0
    if (span.fNumMatrices == 2) {
        D3DXMATRIX  mat;
        IMatrix44ToD3DMatrix(mat, drawable->GetPaletteMatrix(span.fBaseMatrix+1));
        fD3DDevice->SetTransform(D3DTS_WORLDMATRIX(1), &mat);
        fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
    } else {
        fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    }
#endif

    if (mRef) {
        /* Push the matrices into the GLSL shader now */
        glUniformMatrix4fv(mRef->uMatrixProj, 1, GL_TRUE, fDevice.fMatrixProj);
        glUniformMatrix4fv(mRef->uMatrixW2C, 1, GL_TRUE, fDevice.fMatrixW2C);
        glUniformMatrix4fv(mRef->uMatrixL2W, 1, GL_TRUE, fDevice.fMatrixL2W);
        glUniformMatrix4fv(mRef->uMatrixW2L, 1, GL_TRUE, fDevice.fMatrixW2L);

        if (mRef->uMatrixC2W != -1)
            glUniformMatrix4fv(mRef->uMatrixC2W, 1, GL_TRUE, fDevice.fMatrixC2W);
    }
}

void plGLPipeline::IRenderBufferSpan(const plIcicle& span,
                                     hsGDeviceRef* vb, hsGDeviceRef* ib,
                                     hsGMaterial* material,
                                     uint32_t vStart, uint32_t vLength,
                                     uint32_t iStart, uint32_t iLength)
{
    plProfile_BeginTiming(RenderBuff);

    typename DeviceType::VertexBufferRef* vRef = (typename DeviceType::VertexBufferRef*)vb;
    typename DeviceType::IndexBufferRef* iRef = (typename DeviceType::IndexBufferRef*)ib;
    plGLMaterialShaderRef* mRef = static_cast<plGLMaterialShaderRef*>(material->GetDeviceRef());

    if (!vRef->fRef || !iRef->fRef) {
        plProfile_EndTiming(RenderBuff);

        hsAssert(false, "Trying to render a nil buffer pair!");
        return;
    }

    LOG_GL_ERROR_CHECK("PRE Render failed");

    mRef->SetupTextureRefs();

    /* Vertex Buffer stuff */
    glBindBuffer(GL_ARRAY_BUFFER, vRef->fRef);

    glEnableVertexAttribArray(kVtxPosition);
    glVertexAttribPointer(kVtxPosition, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, 0);

    size_t weight_offset = 0;
    switch (vRef->fFormat & plGBufferGroup::kSkinWeightMask)
    {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            weight_offset += sizeof(float);
            break;
        case plGBufferGroup::kSkin2Weights:
            weight_offset += sizeof(float) * 2;
            break;
        case plGBufferGroup::kSkin3Weights:
            weight_offset += sizeof(float) * 3;
            break;
        default:
            hsAssert( false, "Bad skin weight value in GBufferGroup" );
    }

    if (vRef->fFormat & plGBufferGroup::kSkinIndices) {
        weight_offset += sizeof(uint32_t);
    }

    glEnableVertexAttribArray(kVtxNormal);
    glVertexAttribPointer(kVtxNormal, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, (void*)((sizeof(float) * 3) + weight_offset));

    glEnableVertexAttribArray(kVtxColor);
    glVertexAttribPointer(kVtxColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, vRef->fVertexSize, (void*)((sizeof(float) * 3 * 2) + weight_offset));

    int numUVs = vRef->fOwner->GetNumUVs();
    for (int i = 0; i < numUVs; i++) {
        glEnableVertexAttribArray(kVtxUVWSrc + i);
        glVertexAttribPointer(kVtxUVWSrc + i, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, (void*)((sizeof(float) * 3 * 2) + (sizeof(uint32_t) * 2) + (sizeof(float) * 3 * i) + weight_offset));
    }

    LOG_GL_ERROR_CHECK("Vertex Attributes failed")

    /* Index Buffer stuff and drawing */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iRef->fRef);

    plGLRenderTriListFunc render(&fDevice, 0, vStart, vLength, iStart, iLength);

    plProfile_EndTiming(RenderBuff);

    // Turn on this spans lights and turn off the rest.
    ISelectLights(&span, mRef);

    for (size_t pass = 0; pass < mRef->GetNumPasses(); pass++) {
        // Set uniform to pass
        if (mRef->uPassNumber != -1)
            glUniform1i(mRef->uPassNumber, pass);

        glUniform1f(mRef->uInvertVtxAlpha, 0.f);

        plLayerInterface* lay = material->GetLayer(mRef->GetPassIndex(pass));

        ICalcLighting(mRef, lay, &span);

        hsGMatState s;
        s.Composite(lay->GetState(), fMatOverOn, fMatOverOff);

        IHandleZMode(s);
        IHandleBlendMode(s);

        if (s.fBlendFlags & (hsGMatState::kBlendTest | hsGMatState::kBlendAlpha | hsGMatState::kBlendAddColorTimesAlpha) &&
            !(s.fBlendFlags & hsGMatState::kBlendAlphaAlways))
        {
            // AlphaTestHigh is used for reducing sort artifacts on textures that
            // are mostly opaque or transparent, but have regions of translucency
            // in transition. Like a texture for a bush billboard. It lets there be
            // some transparency falloff, but quit drawing before it gets so
            // transparent that draw order problems (halos) become apparent.
            if (s.fBlendFlags & hsGMatState::kBlendAlphaTestHigh)
                glUniform1f(mRef->uAlphaThreshold, 64.f/255.f);
            else
                glUniform1f(mRef->uAlphaThreshold, 0.f);
        } else {
            glUniform1f(mRef->uAlphaThreshold, 0.f);
        }

        if (s.fMiscFlags & hsGMatState::kMiscTwoSided) {
            glDisable(GL_CULL_FACE);
        } else {
            glEnable(GL_CULL_FACE);
        }

        // TEMP
        render.RenderPrims();
    }

    LOG_GL_ERROR_CHECK("Render failed")
}

bool plGLPipeline::ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* spanBase)
{
    if (!(spanBase->fTypeMask & plSpan::kVertexSpan))
        return false;

    // If we arent' an trilist, we're toast.
    if (!(spanBase->fTypeMask & plSpan::kIcicleSpan))
        return false;

    plIcicle* span = (plIcicle*)spanBase;

    DeviceType::VertexBufferRef* vRef = (DeviceType::VertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx);
    if (!vRef)
        return true;

    DeviceType::IndexBufferRef* iRef = (DeviceType::IndexBufferRef*)group->GetIndexBufferRef(span->fIBufferIdx);
    if (!iRef)
        return true;

    // If our vertex buffer ref is volatile and the timestamp is off
    // then it needs to be refilled
    if (vRef->Expired(fVtxRefTime))
        IRefreshDynVertices(group, vRef);

    if (iRef->IsDirty()) {
        fDevice.FillIndexBufferRef(iRef, group, span->fIBufferIdx);
        iRef->SetRebuiltSinceUsed(true);
    }

    return false; // No error
}

bool plGLPipeline::IRefreshDynVertices(plGBufferGroup* group, plGLVertexBufferRef* vRef)
{
    ptrdiff_t size = (group->GetVertBufferEnd(vRef->fIndex) - group->GetVertBufferStart(vRef->fIndex)) * vRef->fVertexSize;
    if (!size)
        return false; // No error, just nothing to do.

    hsAssert(size > 0, "Bad start and end counts in a group");

    if (!vRef->fRef)
        glGenBuffers(1, &vRef->fRef);

    glBindBuffer(GL_ARRAY_BUFFER, vRef->fRef);

    uint8_t* vData;
    if (vRef->fData)
        vData = vRef->fData;
    else
        vData = group->GetVertBufferData(vRef->fIndex) + group->GetVertBufferStart(vRef->fIndex) * vRef->fVertexSize;

    glBufferData(GL_ARRAY_BUFFER, size, vData, GL_DYNAMIC_DRAW);

    vRef->fRefTime = fVtxRefTime;
    vRef->SetDirty(false);

    return false;
}

void plGLPipeline::IHandleZMode(hsGMatState flags)
{
    switch (flags.fZFlags & hsGMatState::kZMask)
    {
        case hsGMatState::kZClearZ:
            glDepthFunc(GL_ALWAYS);
            glDepthMask(GL_TRUE);
            break;
        case hsGMatState::kZNoZRead:
            glDepthFunc(GL_ALWAYS);
            glDepthMask(GL_TRUE);
            break;
        case hsGMatState::kZNoZWrite:
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZClearZ:
            glDepthFunc(GL_ALWAYS);
            glDepthMask(GL_TRUE);
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite:
            glDepthFunc(GL_ALWAYS);
            glDepthMask(GL_FALSE);
            break;
        case 0:
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_TRUE);
            break;
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite:
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead:
            hsAssert(false, "Illegal combination of Z Buffer modes (Clear but don't write)");
            break;
    }

    if (flags.fZFlags & hsGMatState::kZIncLayer) {
        int32_t int_value = 8;
        float value = *(float*)(&int_value);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-1.f, 1.f);
    } else {
        glPolygonOffset(0.f, 0.f);
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
}

void plGLPipeline::IHandleBlendMode(hsGMatState flags)
{
    // No color, just writing out Z values.
    if (flags.fBlendFlags & hsGMatState::kBlendNoColor) {
        glBlendFunc(GL_ZERO, GL_ONE);
        flags.fBlendFlags |= 0x80000000;
    } else {
        switch (flags.fBlendFlags & hsGMatState::kBlendMask)
        {
            // Detail is just a special case of alpha, handled in construction of the texture
            // mip chain by making higher levels of the chain more transparent.
            case hsGMatState::kBlendDetail:
            case hsGMatState::kBlendAlpha:
                if (flags.fBlendFlags & hsGMatState::kBlendInvertFinalAlpha) {
                    if (flags.fBlendFlags & hsGMatState::kBlendAlphaPremultiplied) {
                        glBlendFunc(GL_ONE, GL_SRC_ALPHA);
                    } else {
                        glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);
                    }
                } else {
                    if (flags.fBlendFlags & hsGMatState::kBlendAlphaPremultiplied) {
                        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
                    } else {
                        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    }
                }
                break;

            // Multiply the final color onto the frame buffer.
            case hsGMatState::kBlendMult:
                if (flags.fBlendFlags & hsGMatState::kBlendInvertFinalColor) {
                    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                } else {
                    glBlendFunc(GL_ZERO, GL_SRC_COLOR);
                }
                break;

            // Add final color to FB.
            case hsGMatState::kBlendAdd:
                glBlendFunc(GL_ONE, GL_ONE);
                break;

            // Multiply final color by FB color and add it into the FB.
            case hsGMatState::kBlendMADD:
                glBlendFunc(GL_DST_COLOR, GL_ONE);
                break;

            // Final color times final alpha, added into the FB.
            case hsGMatState::kBlendAddColorTimesAlpha:
                if (flags.fBlendFlags & hsGMatState::kBlendInvertFinalAlpha) {
                    glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);
                } else {
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                }
                break;

            // Overwrite final color onto FB
            case 0:
                glBlendFunc(GL_ONE, GL_ZERO);
                break;

            default:
                {
                    hsAssert(false, "Too many blend modes specified in material");

#if 0
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
                    }
#endif
                }
                break;
        }
    }
}


void plGLPipeline::ICalcLighting(plGLMaterialShaderRef* mRef, const plLayerInterface* currLayer, const plSpan* currSpan)
{
    //plProfile_Inc(MatLightState);

    GLint e;

    if (IsDebugFlagSet(plPipeDbg::kFlagAllBright)) {
        glUniform4f(mRef->uGlobalAmbient,  1.0, 1.0, 1.0, 1.0);

        glUniform4f(mRef->uMatAmbientCol,  1.0, 1.0, 1.0, 1.0);
        glUniform4f(mRef->uMatDiffuseCol,  1.0, 1.0, 1.0, 1.0);
        glUniform4f(mRef->uMatEmissiveCol, 1.0, 1.0, 1.0, 1.0);
        glUniform4f(mRef->uMatSpecularCol, 1.0, 1.0, 1.0, 1.0);

        glUniform1f(mRef->uMatAmbientSrc,  1.0);
        glUniform1f(mRef->uMatDiffuseSrc,  1.0);
        glUniform1f(mRef->uMatEmissiveSrc, 1.0);
        glUniform1f(mRef->uMatSpecularSrc, 1.0);

        return;
    }

    hsGMatState state;
    state.Composite(currLayer->GetState(), fMatOverOn, fMatOverOff);

    uint32_t mode = (currSpan != nullptr) ? (currSpan->fProps & plSpan::kLiteMask) : plSpan::kLiteMaterial;

    if (state.fMiscFlags & hsGMatState::kMiscBumpChans) {
        mode = plSpan::kLiteMaterial;
        state.fShadeFlags |= hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite;
    }

    /// Select one of our three lighting methods
    switch (mode) {
        case plSpan::kLiteMaterial:     // Material shading
        {
            if (state.fShadeFlags & hsGMatState::kShadeWhite) {
                glUniform4f(mRef->uGlobalAmbient, 1.0, 1.0, 1.0, 1.0);
                glUniform4f(mRef->uMatAmbientCol, 1.0, 1.0, 1.0, 1.0);
            } else if (IsDebugFlagSet(plPipeDbg::kFlagNoPreShade)) {
                glUniform4f(mRef->uGlobalAmbient, 0.0, 0.0, 0.0, 1.0);
                glUniform4f(mRef->uMatAmbientCol, 0.0, 0.0, 0.0, 1.0);
            } else {
                hsColorRGBA amb = currLayer->GetPreshadeColor();
                glUniform4f(mRef->uGlobalAmbient, amb.r, amb.g, amb.b, 1.0);
                glUniform4f(mRef->uMatAmbientCol, amb.r, amb.g, amb.b, 1.0);
            }

            hsColorRGBA dif = currLayer->GetRuntimeColor();
            glUniform4f(mRef->uMatDiffuseCol, dif.r, dif.g, dif.b, currLayer->GetOpacity());

            hsColorRGBA em = currLayer->GetAmbientColor();
            glUniform4f(mRef->uMatEmissiveCol, em.r, em.g, em.b, 1.0);

            // Set specular properties
            if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
                hsColorRGBA spec = currLayer->GetSpecularColor();
                glUniform4f(mRef->uMatSpecularCol, spec.r, spec.g, spec.b, 1.0);
#if 0
                mat.Power = currLayer->GetSpecularPower();
#endif
            } else {
                glUniform4f(mRef->uMatSpecularCol, 0.0, 0.0, 0.0, 0.0);
            }

            glUniform1f(mRef->uMatDiffuseSrc,  1.0);
            glUniform1f(mRef->uMatEmissiveSrc, 1.0);
            glUniform1f(mRef->uMatSpecularSrc, 1.0);

            if (state.fShadeFlags & hsGMatState::kShadeNoShade) {
                glUniform1f(mRef->uMatAmbientSrc, 1.0);
            } else {
                glUniform1f(mRef->uMatAmbientSrc, 0.0);
            }

            break;
        }

        case plSpan::kLiteVtxPreshaded:  // Vtx preshaded
        {
            glUniform4f(mRef->uGlobalAmbient, 0.0, 0.0, 0.0, 0.0);
            glUniform4f(mRef->uMatAmbientCol, 0.0, 0.0, 0.0, 0.0);
            glUniform4f(mRef->uMatDiffuseCol, 0.0, 0.0, 0.0, 0.0);
            glUniform4f(mRef->uMatEmissiveCol, 0.0, 0.0, 0.0, 0.0);
            glUniform4f(mRef->uMatSpecularCol, 0.0, 0.0, 0.0, 0.0);

            glUniform1f(mRef->uMatDiffuseSrc,  0.0);
            glUniform1f(mRef->uMatAmbientSrc,  1.0);
            glUniform1f(mRef->uMatSpecularSrc, 1.0);

            if (state.fShadeFlags & hsGMatState::kShadeEmissive)
                glUniform1f(mRef->uMatEmissiveSrc, 0.0);
            else
                glUniform1f(mRef->uMatEmissiveSrc, 1.0);
            break;
        }

        case plSpan::kLiteVtxNonPreshaded:      // Vtx non-preshaded
        {
            glUniform4f(mRef->uMatAmbientCol, 0.0, 0.0, 0.0, 0.0);
            glUniform4f(mRef->uMatDiffuseCol, 0.0, 0.0, 0.0, 0.0);

            hsColorRGBA em = currLayer->GetAmbientColor();
            glUniform4f(mRef->uMatEmissiveCol, em.r, em.g, em.b, 1.0);

            // Set specular properties
            if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
                hsColorRGBA spec = currLayer->GetSpecularColor();
                glUniform4f(mRef->uMatSpecularCol, spec.r, spec.g, spec.b, 1.0);
#if 0
                mat.Power = currLayer->GetSpecularPower();
#endif
            } else {
                glUniform4f(mRef->uMatSpecularCol, 0.0, 0.0, 0.0, 0.0);
            }

            hsColorRGBA amb = currLayer->GetPreshadeColor();
            glUniform4f(mRef->uGlobalAmbient, amb.r, amb.g, amb.b, amb.a);

            glUniform1f(mRef->uMatAmbientSrc,  0.0);
            glUniform1f(mRef->uMatDiffuseSrc,  0.0);
            glUniform1f(mRef->uMatEmissiveSrc, 1.0);
            glUniform1f(mRef->uMatSpecularSrc, 1.0);
            break;
        }
    }

    // Piggy-back some temporary fog stuff on the lighting...
    const plFogEnvironment* fog = (currSpan ? (currSpan->fFogEnvironment ? currSpan->fFogEnvironment : &fView.GetDefaultFog()) : nullptr);
    uint8_t type = fog ? fog->GetType() : plFogEnvironment::kNoFog;
    hsColorRGBA color;

    switch (type) {
        case plFogEnvironment::kLinearFog:
        {
            float start, end;
            fog->GetPipelineParams(&start, &end, &color);

            if (mRef->uFogExponential != -1)
                glUniform1i(mRef->uFogExponential, 0);
            if (mRef->uFogValues != -1)
                glUniform2f(mRef->uFogValues, start, end);
            if (mRef->uFogColor != -1)
                glUniform3f(mRef->uFogColor, color.r, color.g, color.b);
            break;
        }
        case plFogEnvironment::kExpFog:
        case plFogEnvironment::kExp2Fog:
        {
            float density;
            float power = (type == plFogEnvironment::kExp2Fog) ? 2.0f : 1.0f;
            fog->GetPipelineParams(&density, &color);

            if (mRef->uFogExponential != -1)
                glUniform1i(mRef->uFogExponential, 1);
            if (mRef->uFogValues != -1)
                glUniform2f(mRef->uFogValues, power, density);
            if (mRef->uFogColor != -1)
                glUniform3f(mRef->uFogColor, color.r, color.g, color.b);
            break;
        }
        default:
            if (mRef->uFogExponential != -1)
                glUniform1i(mRef->uFogExponential, 0);
            if (mRef->uFogValues != -1)
                glUniform2f(mRef->uFogValues, 0.0, 0.0);
            if (mRef->uFogColor != -1)
                glUniform3f(mRef->uFogColor, 0.0, 0.0, 0.0);
            break;
    }
}

void plGLPipeline::ISelectLights(const plSpan* span, plGLMaterialShaderRef* mRef, bool proj)
{
    const size_t numLights = 8;
    size_t i = 0;
    int32_t startScale;
    float threshhold;
    float overHold = 0.3;
    float scale;

    if  (!IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagNoApplyProjLights) && proj) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagOnlyApplyProjLights) && !proj))
    {
        std::vector<plLightInfo*>& spanLights = span->GetLightList(proj);

        for (i = 0; i < spanLights.size() && i < numLights; i++) {
            IEnableLight(mRef, i, spanLights[i]);
        }
        startScale = i;

        /// Attempt #2: Take some of the n strongest lights (below a given threshhold) and
        /// fade them out to nothing as they get closer to the bottom. This way, they fade
        /// out of existence instead of pop out.

        if (i < spanLights.size() - 1 && i > 0) {
            threshhold = span->GetLightStrength(i, proj);
            i--;
            overHold = threshhold * 1.5f;

            if (overHold > span->GetLightStrength(0, proj)) {
                overHold = span->GetLightStrength(0, proj);
            }

            for (; i > 0 && span->GetLightStrength(i, proj) < overHold; i--) {
                scale = (overHold - span->GetLightStrength(i, proj)) / (overHold - threshhold);

                IScaleLight(mRef, i, (1 - scale) * span->GetLightScale(i, proj));
            }
            startScale = i + 1;
        }


        /// Make sure those lights that aren't scaled....aren't
        for (i = 0; i < startScale; i++) {
            IScaleLight(mRef, i, span->GetLightScale(i, proj));
        }
    }

    for (; i < numLights; i++) {
        IDisableLight(mRef, i);
    }
}

void plGLPipeline::IEnableLight(plGLMaterialShaderRef* mRef, size_t i, plLightInfo* light)
{
    hsColorRGBA amb = light->GetAmbient();
    glUniform4f(mRef->uLampSources[i].ambient, amb.r, amb.g, amb.b, amb.a);

    hsColorRGBA diff = light->GetDiffuse();
    glUniform4f(mRef->uLampSources[i].diffuse, diff.r, diff.g, diff.b, diff.a);

    hsColorRGBA spec = light->GetSpecular();
    glUniform4f(mRef->uLampSources[i].specular, spec.r, spec.g, spec.b, spec.a);

    plDirectionalLightInfo* dirLight = nullptr;
    plOmniLightInfo* omniLight = nullptr;
    plSpotLightInfo* spotLight = nullptr;

    if ((dirLight = plDirectionalLightInfo::ConvertNoRef(light)) != nullptr)
    {
        hsVector3 lightDir = dirLight->GetWorldDirection();
        glUniform4f(mRef->uLampSources[i].position, lightDir.fX, lightDir.fY, lightDir.fZ, 0.0);
        glUniform3f(mRef->uLampSources[i].direction, lightDir.fX, lightDir.fY, lightDir.fZ);

        glUniform1f(mRef->uLampSources[i].constAtten, 1.0f);
        glUniform1f(mRef->uLampSources[i].linAtten, 0.0f);
        glUniform1f(mRef->uLampSources[i].quadAtten, 0.0f);
    }
    else if ((omniLight = plOmniLightInfo::ConvertNoRef(light)) != nullptr)
    {
        hsPoint3 pos = omniLight->GetWorldPosition();
        glUniform4f(mRef->uLampSources[i].position, pos.fX, pos.fY, pos.fZ, 1.0);

        // TODO: Maximum Range

        glUniform1f(mRef->uLampSources[i].constAtten, omniLight->GetConstantAttenuation());
        glUniform1f(mRef->uLampSources[i].linAtten, omniLight->GetLinearAttenuation());
        glUniform1f(mRef->uLampSources[i].quadAtten, omniLight->GetQuadraticAttenuation());

        if (!omniLight->GetProjection() && (spotLight = plSpotLightInfo::ConvertNoRef(omniLight)) != nullptr) {
            hsVector3 lightDir = spotLight->GetWorldDirection();
            glUniform3f(mRef->uLampSources[i].direction, lightDir.fX, lightDir.fY, lightDir.fZ);

            float falloff = spotLight->GetFalloff();
            float theta = cosf(spotLight->GetSpotInner());
            float phi = cosf(spotLight->GetProjection() ? hsConstants::half_pi<float> : spotLight->GetSpotOuter());

            glUniform3f(mRef->uLampSources[i].spotProps, falloff, theta, phi);
        } else {
            glUniform3f(mRef->uLampSources[i].spotProps, 0.0, 0.0, 0.0);
        }
    }
    else {
        IDisableLight(mRef, i);
    }
}

void plGLPipeline::IDisableLight(plGLMaterialShaderRef* mRef, size_t i)
{
    glUniform4f(mRef->uLampSources[i].position, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(mRef->uLampSources[i].ambient, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(mRef->uLampSources[i].diffuse, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform4f(mRef->uLampSources[i].specular, 0.0f, 0.0f, 0.0f, 0.0f);
    glUniform1f(mRef->uLampSources[i].constAtten, 1.0f);
    glUniform1f(mRef->uLampSources[i].linAtten, 0.0f);
    glUniform1f(mRef->uLampSources[i].quadAtten, 0.0f);
    glUniform1f(mRef->uLampSources[i].scale, 0.0f);
}

void plGLPipeline::IScaleLight(plGLMaterialShaderRef* mRef, size_t i, float scale)
{
    scale = int(scale * 1.e1f) * 1.e-1f;
    glUniform1f(mRef->uLampSources[i].scale, scale);
}

void plGLPipeline::IDrawPlate(plPlate* plate)
{
    hsGMaterial* material = plate->GetMaterial();

    // To override the transform done by the z-bias
    static float projMat[16] = {
         1.0f,  0.0f,  0.0f,  0.0f,
         0.0f, -1.0f,  0.0f,  0.0f,
         0.0f,  0.0f,  2.0f, -2.0f,
         0.0f,  0.0f,  1.0f,  0.0f
    };

    /// Set up the transform directly
    fDevice.SetLocalToWorldMatrix(plate->GetTransform());

    //IPushPiggyBacks(material);

    // First, do we have a device ref at this index?
    plGLMaterialShaderRef* mRef = (plGLMaterialShaderRef*)material->GetDeviceRef();

    if (mRef == nullptr) {
        mRef = new plGLMaterialShaderRef(material, this);
        material->SetDeviceRef(mRef);
    }

    if (!mRef->IsLinked())
        mRef->Link(&fMatRefList);

    glUseProgram(mRef->fRef);
    fDevice.fCurrentProgram = mRef->fRef;

    mRef->SetupTextureRefs();

    /* Push the matrices into the GLSL shader now */
    glUniformMatrix4fv(mRef->uMatrixProj, 1, GL_TRUE, projMat);
    glUniformMatrix4fv(mRef->uMatrixW2C, 1, GL_TRUE, fDevice.fMatrixW2C);
    glUniformMatrix4fv(mRef->uMatrixC2W, 1, GL_TRUE, fDevice.fMatrixC2W);
    glUniformMatrix4fv(mRef->uMatrixL2W, 1, GL_TRUE, fDevice.fMatrixL2W);

    if (mRef->uMatrixW2L != -1)
        glUniformMatrix4fv(mRef->uMatrixW2L, 1, GL_TRUE, fDevice.fMatrixW2L);

    glUniform4f(mRef->uGlobalAmbient,  1.0, 1.0, 1.0, 1.0);

    glUniform4f(mRef->uMatAmbientCol,  1.0, 1.0, 1.0, 1.0);
    glUniform4f(mRef->uMatDiffuseCol,  1.0, 1.0, 1.0, 1.0);
    glUniform4f(mRef->uMatEmissiveCol, 1.0, 1.0, 1.0, 1.0);
    glUniform4f(mRef->uMatSpecularCol, 1.0, 1.0, 1.0, 1.0);

    glUniform1f(mRef->uMatAmbientSrc,  1.0);
    glUniform1f(mRef->uMatDiffuseSrc,  1.0);
    glUniform1f(mRef->uMatEmissiveSrc, 1.0);
    glUniform1f(mRef->uMatSpecularSrc, 1.0);

    glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_SHORT, (GLvoid*)(sizeof(uint16_t) * 0));
}
