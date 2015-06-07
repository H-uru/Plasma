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

#include "plPipeDebugFlags.h"
#include "plProfile.h"
#include "plPipeline/hsWinRef.h"
#include "plStatusLog/plStatusLog.h"

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
    : pl3DPipeline(devMode), fMatRefList()
{
    plStatusLog::AddLineS("pipeline.log", "Constructing plGLPipeline");
    plStatusLog::AddLineSF("pipeline.log", "Driver vendor: {}", devMode->GetDevice()->GetDriverDesc());

    fDevice.fWindow = window;
    fDevice.fDevice = display;
    fDevice.fPipeline = this;

    fPlateMgr = new plGLPlateManager(this);
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

void plGLPipeline::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    // First, do we have a device ref at this index?
    typename DeviceType::VertexBufferRef* vRef = (typename DeviceType::VertexBufferRef*)owner->GetVertexBufferRef(idx);

    // If not
    if (!vRef) {
        // Make the blank ref
        vRef = new typename DeviceType::VertexBufferRef();
        fDevice.SetupVertexBufferRef(owner, idx, vRef);
    }

    if (!vRef->IsLinked())
        vRef->Link(&fVtxBuffRefList);

    // One way or another, we now have a vbufferref[idx] in owner.
    // Now, does it need to be (re)filled?
    // If the owner is volatile, then we hold off. It might not
    // be visible, and we might need to refill it again if we
    // have an overrun of our dynamic buffer.
    if (!vRef->Volatile()) {
        // If it's a static buffer, allocate a vertex buffer for it.
        fDevice.CheckStaticVertexBuffer(vRef, owner, idx);

        // Might want to remove this assert, and replace it with a dirty check
        // if we have static buffers that change very seldom rather than never.
        hsAssert(!vRef->IsDirty(), "Non-volatile vertex buffers should never get dirty");
    } else {
        // Make sure we're going to be ready to fill it.

#if 0
        if (!vRef->fData && (vRef->fFormat != owner->GetVertexFormat()))
        {
            vRef->fData = new uint8_t[vRef->fCount * vRef->fVertexSize];
            fDevice.FillVolatileVertexBufferRef(vRef, owner, idx);
        }
#endif
    }
}

void plGLPipeline::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    typename DeviceType::IndexBufferRef* iRef = (typename DeviceType::IndexBufferRef*)owner->GetIndexBufferRef(idx);

    if (!iRef) {
        // Create one from scratch.
        iRef = new typename DeviceType::IndexBufferRef();
        fDevice.SetupIndexBufferRef(owner, idx, iRef);
    }

    if (!iRef->IsLinked())
        iRef->Link(&fIdxBuffRefList);

    // Make sure it has all resources created.
    fDevice.CheckIndexBuffer(iRef);

    // If it's dirty, refill it.
    if (iRef->IsDirty())
        fDevice.FillIndexBufferRef(iRef, owner, idx);
}

bool plGLPipeline::OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly)
{
    return false;
}

bool plGLPipeline::CloseAccess(plAccessSpan& acc)
{
    return false;
}

void plGLPipeline::CheckTextureRef(plLayerInterface* lay)
{}

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
{}

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

        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClearDepth(clearDepth);

        glClear(masks);
    }
}

hsGDeviceRef* plGLPipeline::MakeRenderTargetRef(plRenderTarget* owner)
{
    return nullptr;
}

bool plGLPipeline::BeginRender()
{
    // TODO: Device Init/Reset stuff here

    // offset transform
    RefreshScreenMatrices();

    // If this is the primary BeginRender, make sure we're really ready.
    if (fInSceneDepth++ == 0) {
        fDevice.BeginRender();

        hsColorRGBA clearColor = GetClearColor();

        glDepthMask(GL_TRUE);
        //glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        glClearColor(0.f, 0.f, 0.5f, 1.f);
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
{}

bool plGLPipeline::IsFullScreen() const
{
    return false;
}

void plGLPipeline::Resize(uint32_t width, uint32_t height)
{}

void plGLPipeline::LoadResources()
{}

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

        if (material != nullptr) {
            // First, do we have a device ref at this index?
            plGLMaterialShaderRef* mRef = static_cast<plGLMaterialShaderRef*>(material->GetDeviceRef());

            if (mRef == nullptr) {
                mRef = new plGLMaterialShaderRef(material);
                material->SetDeviceRef(mRef);

                //glUseProgram(mRef->fRef);
                //fDevice.fCurrentProgram = mRef->fRef;
            }

            if (!mRef->IsLinked())
                mRef->Link(&fMatRefList);

            glUseProgram(mRef->fRef);
            fDevice.fCurrentProgram = mRef->fRef;
            LOG_GL_ERROR_CHECK("Use Program failed")

            // TODO: Figure out how to use VAOs properly :(
            GLuint vao;
            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            // What do we change?

            plProfile_BeginTiming(SpanTransforms);
            ISetupTransforms(ice, tempIce, lastL2W);
            plProfile_EndTiming(SpanTransforms);

            // Turn on this spans lights and turn off the rest.
            //IEnableLights( &tempIce );

            // Check that the underlying buffers are ready to go.
            //plProfile_BeginTiming(CheckDyn);
            //ICheckDynBuffers(drawable, drawable->GetBufferGroup(tempIce.fGroupIdx), &tempIce);
            //plProfile_EndTiming(CheckDyn);

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
        }

        // Restart our search...
        i = j;
    }

    plProfile_EndTiming(RenderSpan);
    /// All done!
}


void plGLPipeline::ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W)
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

    if (fDevice.fCurrentProgram) {
        /* Push the matrices into the GLSL shader now */
        GLint uniform = glGetUniformLocation(fDevice.fCurrentProgram, "uMatrixProj");
        glUniformMatrix4fv(uniform, 1, GL_TRUE, fDevice.fMatrixProj);

        uniform = glGetUniformLocation(fDevice.fCurrentProgram, "uMatrixW2C");
        glUniformMatrix4fv(uniform, 1, GL_TRUE, fDevice.fMatrixW2C);

        uniform = glGetUniformLocation(fDevice.fCurrentProgram, "uMatrixL2W");
        glUniformMatrix4fv(uniform, 1, GL_TRUE, fDevice.fMatrixL2W);
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

    GLint posAttrib = glGetAttribLocation(fDevice.fCurrentProgram, "aVtxPosition");
    if (posAttrib != -1) {
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, 0);
    }

    GLint norAttrib = glGetAttribLocation(fDevice.fCurrentProgram, "aVtxNormal");
    if (norAttrib != -1) {
        glEnableVertexAttribArray(norAttrib);
        glVertexAttribPointer(norAttrib, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, (void*)(sizeof(float) * 3));
    }

    GLint colAttrib = glGetAttribLocation(fDevice.fCurrentProgram, "aVtxColor");
    if (colAttrib != -1) {
        glEnableVertexAttribArray(colAttrib);
        glVertexAttribPointer(colAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, vRef->fVertexSize, (void*)(sizeof(float) * 3 * 2));
    }

    LOG_GL_ERROR_CHECK("Vertex Attributes failed")

    for (int i = 0; i < mRef->GetNumUVs(); i++) {
        ST::string name = ST::format("aVtxUVWSrc{}", i);

        GLint uvwAttrib = glGetAttribLocation(fDevice.fCurrentProgram, name.c_str());
        if (uvwAttrib != -1) {
            glEnableVertexAttribArray(uvwAttrib);
            glVertexAttribPointer(uvwAttrib, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, (void*)((sizeof(float) * 3 * 2) + (sizeof(uint32_t) * 2) + (sizeof(float) * 3 * i)));
        }

        LOG_GL_ERROR_CHECK("UVW Attributes failed")
    }

    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, vRef->fVertexSize, 0);
    glVertexAttribPointer(colAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, vRef->fVertexSize, (void*)(sizeof(float) * 3 * 2));


    /* Index Buffer stuff and drawing */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iRef->fRef);

    plGLRenderTriListFunc render(&fDevice, 0, vStart, vLength, iStart, iLength);

    plProfile_EndTiming(RenderBuff);

    // TEMP
    render.RenderPrims();

    LOG_GL_ERROR_CHECK("Render failed")
}
