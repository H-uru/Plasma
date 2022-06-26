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
#include "HeadSpin.h"
#include <string_theory/format>

#include <Metal/Metal.hpp>
#import <simd/simd.h>

#include "plQuality.h"

#include "plMetalPipeline.h"
#include "plMetalMaterialShaderRef.h"
#include "plMetalPlateManager.h"
#include "plMetalPipelineState.h"

#include "hsTimer.h"
#include "plPipeDebugFlags.h"
#include "plPipeResReq.h"

#include "pnNetCommon/plNetApp.h"   // for dbg logging
#include "pnMessage/plPipeResMakeMsg.h"
#include "plAvatar/plAvatarClothing.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGBufferGroup.h"
#include "plGImage/plMipmap.h"
#include "plGLight/plLightInfo.h"
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plDebugText.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plScene/plRenderRequest.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"
#include "pfCamera/plVirtualCamNeu.h"
#include "plMessage/plDeviceRecreateMsg.h"
#include "plgDispatch.h"
#include "plDrawable/plAuxSpan.h"
#include "plSurface/plLayerShadowBase.h"

#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"

#include "plGLight/plShadowSlave.h"
#include "plGLight/plShadowCaster.h"

#include "plTweak.h"

#include "plMetalVertexShader.h"
#include "plMetalFragmentShader.h"

#include "hsGMatState.inl"

#include "plProfile.h"

uint32_t  fDbgSetupInitFlags;     // HACK temp only

plProfile_CreateCounter("Feed Triangles", "Draw", DrawFeedTriangles);
plProfile_CreateCounter("Draw Prim Static", "Draw", DrawPrimStatic);
plProfile_CreateMemCounter("Total Texture Size", "Draw", TotalTexSize);
plProfile_CreateCounter("Layer Change", "Draw", LayChange);
plProfile_Extern(DrawTriangles);
plProfile_Extern(MatChange);

plProfile_CreateTimer("PrepShadows", "PipeT", PrepShadows);
plProfile_CreateTimer("PrepDrawable", "PipeT", PrepDrawable);
plProfile_CreateTimer("  Skin", "PipeT", Skin);
plProfile_CreateTimer("RenderSpan", "PipeT", RenderSpan);
plProfile_CreateTimer("  MergeCheck", "PipeT", MergeCheck);
plProfile_CreateTimer("  MergeSpan", "PipeT", MergeSpan);
plProfile_CreateTimer("  SpanTransforms", "PipeT", SpanTransforms);
plProfile_CreateTimer("  SpanFog", "PipeT", SpanFog);
plProfile_CreateTimer("  SelectLights", "PipeT", SelectLights);
plProfile_CreateTimer("  SelectProj", "PipeT", SelectProj);
plProfile_CreateTimer("  CheckDyn", "PipeT", CheckDyn);
plProfile_CreateTimer("  CheckStat", "PipeT", CheckStat);
plProfile_CreateTimer("  RenderBuff", "PipeT", RenderBuff);
plProfile_CreateTimer("  RenderPrim", "PipeT", RenderPrim);
plProfile_CreateTimer("PlateMgr", "PipeT", PlateMgr);
plProfile_CreateTimer("DebugText", "PipeT", DebugText);
plProfile_CreateTimer("Reset", "PipeT", Reset);

plProfile_CreateCounterNoReset("Reload", "PipeC", PipeReload);
plProfile_CreateCounter("AvRTPoolUsed", "PipeC", AvRTPoolUsed);
plProfile_CreateCounter("AvRTPoolCount", "PipeC", AvRTPoolCount);
plProfile_CreateCounter("AvRTPoolRes", "PipeC", AvRTPoolRes);
plProfile_CreateCounter("AvRTShrinkTime", "PipeC", AvRTShrinkTime);
plProfile_CreateCounter("NumSkin", "PipeC", NumSkin);

plMetalEnumerate plMetalPipeline::enumerator;

class plRenderTriListFunc : public plRenderPrimFunc
{
protected:
    plMetalDevice*        fDevice;
    int                 fBaseVertexIndex;
    int                 fVStart;
    int                 fVLength;
    int                 fIStart;
    int                 fNumTris;
public:
    plRenderTriListFunc(plMetalDevice* device, int baseVertexIndex,
                        int vStart, int vLength, int iStart, int iNumTris)
        : fDevice(device), fBaseVertexIndex(baseVertexIndex), fVStart(vStart), fVLength(vLength), fIStart(iStart), fNumTris(iNumTris) {}

    bool RenderPrims() const override;
};

bool plRenderTriListFunc::RenderPrims() const
{
    plProfile_IncCount(DrawFeedTriangles, fNumTris);
    plProfile_IncCount(DrawTriangles, fNumTris);
    plProfile_Inc(DrawPrimStatic);
    
    
    fDevice->CurrentRenderCommandEncoder()->setVertexBytes(fDevice->fPipeline->fCurrentRenderPassUniforms, sizeof(VertexUniforms), BufferIndexState);
    fDevice->CurrentRenderCommandEncoder()->drawIndexedPrimitives(MTL::PrimitiveTypeTriangle, fNumTris, MTL::IndexTypeUInt16, fDevice->fCurrentIndexBuffer, (sizeof(uint16_t) * fIStart));
}



plMetalPipeline::plMetalPipeline(hsWindowHndl display, hsWindowHndl window, const hsG3DDeviceModeRecord *devMode) :   pl3DPipeline(devMode), fRenderTargetRefList(), fMatRefList(), fPipelineState(nullptr), fCurrentRenderPassUniforms(nullptr), currentDrawableCallback(nullptr), fFragFunction(nullptr), fVShaderRefList(nullptr), fPShaderRefList(nullptr), fULutTextureRef(nullptr)
{
    fTextureRefList = nullptr;
    fVtxBuffRefList = nullptr;
    fIdxBuffRefList = nullptr;
    fMatRefList = nullptr;
    
    fCurrLayerIdx = 0;
    fDevice.fPipeline = this;
    
    //Compile the shaders and link our pipeline
    MTL::Library *library = fDevice.fMetalDevice->newDefaultLibrary();
    MTL::Function *fragFunction = library->newFunction(
                                                    NS::String::string("fragmentShader", NS::ASCIIStringEncoding)
                                                    );
    MTL::Function *vertFunction = library->newFunction(
                                                    NS::String::string("plateVertexShader", NS::ASCIIStringEncoding)
                                                    );
    MTL::RenderPipelineDescriptor *descriptor = MTL::RenderPipelineDescriptor::alloc()->init();
    descriptor->setFragmentFunction(fragFunction);
    descriptor->setVertexFunction(vertFunction);
    descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
    
    NS::Error *error;
    fPipelineState = fDevice.fMetalDevice->newRenderPipelineState(descriptor, &error);
    library->release();
    fragFunction->release();
    vertFunction->release();
    descriptor->release();
    
    fMaxLayersAtOnce = 8;
    
    // Alloc half our simultaneous textures to piggybacks.
    // Won't hurt us unless we try to many things at once.
    fMaxPiggyBacks = fMaxLayersAtOnce >> 1;
    
    // Less than 4 layers at once means we have to fallback on uv bumpmapping
    if (fMaxLayersAtOnce < 4)
        SetDebugFlag(plPipeDbg::kFlagBumpUV, true);
    //plDynamicCamMap::SetCapable(false);
    //plQuality::SetQuality(fDefaultPipeParams.VideoQuality);
    //plQuality::SetCapability(fDefaultPipeParams.VideoQuality);
    plQuality::SetCapability(plQuality::kPS_3);
    //plShadowCaster::EnableShadowCast(false);
    
    fDevice.SetMaxAnsiotropy(fInitialPipeParams.AnisotropicLevel);
    fDevice.SetMSAASampleCount(fInitialPipeParams.AntiAliasingAmount);
    
    fCurrentRenderPassUniforms = (VertexUniforms *) calloc(sizeof(VertexUniforms), sizeof(char));
    
    // RenderTarget pools are shared for our shadow generation algorithm.
    // Different sizes for different resolutions.
    ICreateDeviceObjects();
    ICreateDynDeviceObjects();
    IMakeRenderTargetPools();
}

plMetalPipeline::~plMetalPipeline()
{
    if (plMetalPlateManager* pm = static_cast<plMetalPlateManager*>(fPlateMgr))
    {
        pm->IReleaseGeometry();
    }
}

void plMetalPipeline::ICreateDeviceObjects() {
    fPlateMgr = new plMetalPlateManager(this);
}

bool plMetalPipeline::PreRender(plDrawable *drawable, std::vector<int16_t> &visList, plVisMgr *visMgr)
{
    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(drawable);
    if (!ds) {
        return false;
    }

    if ((ds->GetType() & fView.GetDrawableTypeMask()) == 0) {
        return false;
    }

    fView.GetVisibleSpans(ds, visList, visMgr);

    return visList.size() > 0;
}

bool plMetalPipeline::PrepForRender(plDrawable *drawable, std::vector<int16_t> &visList, plVisMgr *visMgr)
{
    plProfile_BeginTiming(PrepDrawable);

    plDrawableSpans* ice = plDrawableSpans::ConvertNoRef(drawable);
    if (!ice) {
        plProfile_EndTiming(PrepDrawable);
        return false;
    }

    // Find our lights
    ICheckLighting(ice, visList, visMgr);

    // Sort our faces
    if (ice->GetNativeProperty(plDrawable::kPropSortFaces)) {
        ice->SortVisibleSpans(visList, this);
    }

    // Prep for render. This is gives the drawable a chance to
    // do any last minute updates for its buffers, including
    // generating particle tri lists.
    ice->PrepForRender(this);
    
    // Any skinning necessary
    if (!ISoftwareVertexBlend(ice, visList)) {
        plProfile_EndTiming(PrepDrawable);
        return false;
    }

    // Other stuff that we're ignoring for now...

    plProfile_EndTiming(PrepDrawable);

    return true;
}

plTextFont *plMetalPipeline::MakeTextFont(char *face, uint16_t size) {
    return nullptr;
}

bool plMetalPipeline::OpenAccess(plAccessSpan &dst, plDrawableSpans *d, const plVertexSpan *span, bool readOnly) { return false; }

bool plMetalPipeline::CloseAccess(plAccessSpan &acc) { return false; }

void plMetalPipeline::PushRenderRequest(plRenderRequest *req)
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

    if (req->GetOverrideMat()) {
        PushOverrideMaterial(req->GetOverrideMat());
    }

    // Set from our saved ones...
    fView.SetWorldToLocal(w2l);
    fView.SetLocalToWorld(l2w);

    RefreshMatrices();

    if (req->GetIgnoreOccluders()) {
        fView.SetMaxCullNodes(0);
    }

    ResetMetalStateTracking();
}

void plMetalPipeline::PopRenderRequest(plRenderRequest *req)
{
    if (req->GetOverrideMat()) {
        PopOverrideMaterial(nil);
    }
    
    //new render target means we can't use the previous pipeline state
    //it won't be set yet on the new target
    //in theory we could have a stack of these so when we unwind we
    //could get the state back.
    ResetMetalStateTracking();

    hsRefCnt_SafeUnRef(fView.fRenderRequest);
    fView = fViewStack.top();
    fViewStack.pop();

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;
}

plRenderTarget* plMetalPipeline::PopRenderTarget() {
    pl3DPipeline::PopRenderTarget();
    ResetMetalStateTracking();
}

void plMetalPipeline::ClearRenderTarget(plDrawable *d)
{
    plDrawableSpans* src = plDrawableSpans::ConvertNoRef(d);

    if( !src )
    {
        ClearRenderTarget();
        return;
    }
    
    Draw(d);
}

void plMetalPipeline::ClearRenderTarget(const hsColorRGBA *col, const float *depth)
{
    if (fView.fRenderState & (kRenderClearColor | kRenderClearDepth)) {
        hsColorRGBA clearColor = col ? *col : GetClearColor();
        float clearDepth = depth ? *depth : fView.GetClearDepth();
        fDevice.Clear(fView.fRenderState & kRenderClearColor, {clearColor.r, clearColor.g, clearColor.b, clearColor.a}, fView.fRenderState & kRenderClearDepth, 1.0);
        ResetMetalStateTracking();
    }
}

hsGDeviceRef *plMetalPipeline::MakeRenderTargetRef(plRenderTarget *owner)
{
    plMetalRenderTargetRef* ref = nullptr;
    MTL::Texture *depthBuffer = nullptr;
    plCubicRenderTarget     *cubicRT;

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
        ref = (plMetalRenderTargetRef*)owner->GetDeviceRef();
    
    /// Create the render target now
    // Start with the depth surface.
    // Note that we only ever give a cubic rendertarget a single shared depth buffer,
    // since we only render one face at a time. If we were rendering part of face X, then part
    // of face Y, then more of face X, then they would all need their own depth buffers.
    if (owner->GetZDepth() && (owner->GetFlags() & (plRenderTarget::kIsTexture | plRenderTarget::kIsOffscreen))) {
        MTL::TextureDescriptor *depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float_Stencil8,
                                                                                                     owner->GetWidth(),
                                                                                                     owner->GetHeight(),
                                                                                                     false);
        if (fDevice.fMetalDevice->supportsFamily(MTL::GPUFamilyApple1)) {
            depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
        }   else {
            depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
        }
        depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
        depthBuffer = fDevice.fMetalDevice->newTexture(depthTextureDescriptor);
    }
    

    // See if it's a cubic render target.
    // Primary consumer here is the vertex/pixel shader water.
    cubicRT = plCubicRenderTarget::ConvertNoRef( owner );
    if( cubicRT )
    {
        if (!ref)
            ref = new plMetalRenderTargetRef();
        
        MTL::TextureDescriptor *textureDescriptor = MTL::TextureDescriptor::textureCubeDescriptor(MTL::PixelFormatBGRA8Unorm, owner->GetWidth(), false);
        textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead | MTL::TextureUsagePixelFormatView);
        textureDescriptor->setStorageMode(MTL::StorageModePrivate);
        
        plMetalDeviceRef *device = (plMetalDeviceRef *)owner->GetDeviceRef();
        MTL::Texture * texture = fDevice.fMetalDevice->newTexture(textureDescriptor);
        
        /// Create a CUBIC texture
        for( int i = 0; i < 6; i++ )
        {
            plRenderTarget          *face = cubicRT->GetFace( i );
            plMetalRenderTargetRef *fRef;

            if( face->GetDeviceRef() != nil )
            {
                fRef = (plMetalRenderTargetRef *)face->GetDeviceRef();
                if( !fRef->IsLinked() )
                    fRef->Link( &fRenderTargetRefList );
            }
            else
            {
                fRef = new plMetalRenderTargetRef();
                
                face->SetDeviceRef(fRef);
                ( (plMetalRenderTargetRef *)face->GetDeviceRef())->Link( &fRenderTargetRefList );
                // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
                hsRefCnt_SafeUnRef( face->GetDeviceRef() );
            }
            
            //in since the root texture has changed reload all the face textures
            static const uint kFaceMapping[] = {
                1, // kLeftFace
                0, // kRightFace
                4, // kFrontFace
                5, // kBackFace
                2, // kTopFace
                3  // kBottomFace
            };
            
            if(fRef->fTexture) {
                fRef->fTexture->release();
                fRef->fTexture = nullptr;
            }
            
            if(fRef->fDepthBuffer) {
                fRef->fDepthBuffer->release();
                fRef->fDepthBuffer = nullptr;
            }
            
            fRef->fTexture = texture->newTextureView(MTL::PixelFormatBGRA8Unorm, MTL::TextureType2D, NS::Range::Make(0, 1), NS::Range::Make(kFaceMapping[i], 1));
            //in since the depth buffer is shared each render target gets their own retain
            fRef->fDepthBuffer = depthBuffer->retain();
            fRef->SetDirty(false);
        }
        
        //if the ref already has an old texture, release it
        if(ref->fTexture)
            ref->fTexture->release();
        if(ref->fDepthBuffer)
            ref->fDepthBuffer->release();
        ref->fTexture = texture;
        ref->fDepthBuffer = depthBuffer;
        ref->fOwner = owner;
        
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
        ref->SetDirty(false);
        
        return ref;
    }
    else if (owner->GetFlags() & plRenderTarget::kIsTexture) {
        if (!ref)
            ref = new plMetalRenderTargetRef();
        
        MTL::TextureDescriptor *textureDescriptor = MTL::TextureDescriptor::alloc()->init();
        textureDescriptor->setWidth(owner->GetWidth());
        textureDescriptor->setHeight(owner->GetHeight());
        textureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
        textureDescriptor->setStorageMode(MTL::StorageModePrivate);
        
        plMetalDeviceRef *device = (plMetalDeviceRef *)owner->GetDeviceRef();
        MTL::Texture * texture = fDevice.fMetalDevice->newTexture(textureDescriptor);
        textureDescriptor->release();
        
        //if the ref already has an old texture, release it
        if(ref->fTexture)
            ref->fTexture->release();
        if(ref->fDepthBuffer)
            ref->fDepthBuffer->release();
        ref->fTexture = texture;
        ref->fDepthBuffer = depthBuffer;
        ref->fOwner = owner;
        
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
        
        return ref;
    }
    
    // Not a texture either, must be a plain offscreen.
    // Offscreen isn't currently used for anything.
    else if (owner->GetFlags() & plRenderTarget::kIsOffscreen) {
        /// Create a blank surface
        
            if (!ref)
                ref = new plMetalRenderTargetRef();
            
            MTL::TextureDescriptor *textureDescriptor = MTL::TextureDescriptor::alloc()->init();
            textureDescriptor->setWidth(owner->GetWidth());
            textureDescriptor->setHeight(owner->GetHeight());
            textureDescriptor->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
            textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
            textureDescriptor->setStorageMode(MTL::StorageModeManaged);
            
            plMetalDeviceRef *device = (plMetalDeviceRef *)owner->GetDeviceRef();
            MTL::Texture * texture = fDevice.fMetalDevice->newTexture(textureDescriptor);
            textureDescriptor->release();
            
            //if the ref already has an old texture, release it
            if(ref->fTexture)
                ref->fTexture->release();
            if(ref->fDepthBuffer)
                ref->fDepthBuffer->release();
            ref->fTexture = texture;
            ref->fDepthBuffer = depthBuffer;
            ref->fOwner = owner;
            
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
            
            return ref;
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

bool plMetalPipeline::BeginRender()
{
    fCurrentPool = NS::AutoreleasePool::alloc()->init();
    // offset transform
    RefreshScreenMatrices();
    
    ResetMetalStateTracking();
    
    // offset transform
    RefreshScreenMatrices();

    // If this is the primary BeginRender, make sure we're really ready.
    if (fInSceneDepth++ == 0) {
        /// If we have a renderTarget active, use its viewport
        fDevice.SetViewport();
        
        fDevice.BeginRender();

        fVtxRefTime++;
        plMetalBufferPoolRef::SetFrameTime(fVtxRefTime);

        // Render any shadow maps that have been submitted for this frame.
        IPreprocessShadows();
        IPreprocessAvatarTextures();
        
        CA::MetalDrawable *drawable = currentDrawableCallback(fDevice.fMetalDevice);
        if(!drawable) {
            fCurrentPool->release();
            return false;
        }
        fDevice.CreateNewCommandBuffer(drawable);
        drawable->release();
    }
    
    fCurrentCullMode = MTL::CullMode(-1);

    fRenderCnt++;

    // Would probably rather this be an input.
    fTime = hsTimer::GetSysSeconds();

    return false;
}

bool plMetalPipeline::EndRender()
{
    bool retVal = false;
    ResetMetalStateTracking();
    
    if (--fInSceneDepth == 0) {
        fDevice.SubmitCommandBuffer();
        
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
    fCurrentPool->release();

    return retVal;
}

void plMetalPipeline::RenderScreenElements() {
    bool reset = false;

    if (fView.HasCullProxy())
    {
        Draw(fView.GetCullProxy());
    }


    hsGMatState tHack = PushMaterialOverride(hsGMatState::kMisc, hsGMatState::kMiscWireFrame, false);
    hsGMatState ambHack = PushMaterialOverride(hsGMatState::kShade, hsGMatState::kShadeWhite, true);

    plProfile_BeginTiming(PlateMgr);
    // Plates
    if (fPlateMgr)
    {
        fPlateMgr->DrawToDevice(this);
        reset = true;
    }
    plProfile_EndTiming(PlateMgr);

    PopMaterialOverride(ambHack, true);
    PopMaterialOverride(tHack, false);

    plProfile_BeginTiming(DebugText);
    /// Debug text
    if (fDebugTextMgr && plDebugText::Instance().IsEnabled())
    {
        fDebugTextMgr->DrawToDevice(this);
        reset = true;
    }
    plProfile_EndTiming(DebugText);

    plProfile_BeginTiming(Reset);
    if (reset)
    {
        fView.fXformResetFlags = fView.kResetAll; // Text destroys view transforms
    }
    plProfile_EndTiming(Reset);
}

bool plMetalPipeline::IsFullScreen() const { return fDefaultPipeParams.Windowed; }

void plMetalPipeline::Resize(uint32_t width, uint32_t height)
{
    /*
     Resize had a bunch of notes on the DX version about how it was an old function, replaced by ResetDisplayDevice. I'll implement it for now, but consider moving over to ResetDisplayDevice.
     
     This function is cheaper than resetting the entire display device though.
     */
    hsMatrix44  w2c, c2w, proj;

    // Store some states that we *want* to restore back...
    plViewTransform resetTransform = GetViewTransform();

    // Destroy old
    IReleaseDeviceObjects();
    IReleaseDynDeviceObjects();
    
    // Reset width and height
    if( width != 0 && height != 0 )
    {
        // Width and height of zero mean just recreate
        fOrigWidth = width;
        fOrigHeight = height;
        IGetViewTransform().SetScreenSize((uint16_t)(fOrigWidth), (uint16_t)(fOrigHeight));
        resetTransform.SetScreenSize((uint16_t)(fOrigWidth), (uint16_t)(fOrigHeight));
    }
    else
    {
        // Just for debug
        hsStatusMessage( "Recreating the pipeline...\n" );
    }
    
    ICreateDeviceObjects();

    // Restore states
    SetViewTransform(resetTransform);
    IProjectionMatrixToDevice();
    
    plVirtualCam1::Refresh();
    
    ICreateDynDeviceObjects();

    /// Broadcast a message letting everyone know that we were recreated and that
    /// all device-specific stuff needs to be recreated
    plDeviceRecreateMsg* clean = new plDeviceRecreateMsg(this);
    plgDispatch::MsgSend(clean);
}


void plMetalPipeline::IReleaseDeviceObjects()
{
    IReleaseDynDeviceObjects();
    
    delete fPlateMgr;
    fPlateMgr = nullptr;
}

void plMetalPipeline::LoadResources()
{
    hsStatusMessageF("Begin Device Reload t=%f",hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("Begin Device Reload");
    
    if(fFragFunction == nil) {
        FindFragFunction();
    }

    if (plMetalPlateManager* pm = static_cast<plMetalPlateManager*>(fPlateMgr))
        pm->IReleaseGeometry();
    
    IReleaseDynamicBuffers();
    IReleaseAvRTPool();

    // Create all RenderTargets
    plPipeRTMakeMsg* rtMake = new plPipeRTMakeMsg(this);
    rtMake->Send();

    if (plMetalPlateManager* pm = static_cast<plMetalPlateManager*>(fPlateMgr))
        pm->ICreateGeometry();

    // Create all POOL_DEFAULT (sorted) index buffers in the scene.
    plPipeGeoMakeMsg* defMake = new plPipeGeoMakeMsg(this, true);
    defMake->Send();

    // This can be a bit of a mem hog and will use more mem if available, so
    // keep it last in the POOL_DEFAULT allocs.
    IFillAvRTPool();

    // Force a create of all our static vertex buffers.
    plPipeGeoMakeMsg* manMake = new plPipeGeoMakeMsg(this, false);
    manMake->Send();

    // Okay, we've done it, clear the request.
    plPipeResReq::Clear();

    plProfile_IncCount(PipeReload, 1);

    hsStatusMessageF("End Device Reload t=%f",hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("End Device Reload");
}

bool plMetalPipeline::SetGamma(float eR, float eG, float eB)
{
    uint16_t tabR[256];
    uint16_t tabG[256];
    uint16_t tabB[256];

    tabR[0] = tabG[0] = tabB[0] = 0L;

    plConst(float) kMinE(0.1f);
    if( eR > kMinE )
        eR = 1.f / eR;
    else
        eR = 1.f / kMinE;
    if( eG > kMinE )
        eG = 1.f / eG;
    else
        eG = 1.f / kMinE;
    if( eB > kMinE )
        eB = 1.f / eB;
    else
        eB = 1.f / kMinE;

    int i;
    for( i = 1; i < 256; i++ )
    {
        float orig = float(i) / 255.f;

        float gamm;
        gamm = pow(orig, eR);
        gamm *= float(uint16_t(-1));
        tabR[i] = uint16_t(gamm);

        gamm = pow(orig, eG);
        gamm *= float(uint16_t(-1));
        tabG[i] = uint16_t(gamm);

        gamm = pow(orig, eB);
        gamm *= float(uint16_t(-1));
        tabB[i] = uint16_t(gamm);
    }

    SetGamma(tabR, tabG, tabB);

    return true;
}

bool plMetalPipeline::SetGamma(const uint16_t *const tabR, const uint16_t *const tabG, const uint16_t *const tabB)
{
    //allocate a new buffer every time so we don't cause problems with a running render pass
    if(fDevice.fGammaLUTTexture) {
        fDevice.fGammaLUTTexture->release();
        fDevice.fGammaLUTTexture = nullptr;
    }
    
    MTL::TextureDescriptor* texDescriptor = MTL::TextureDescriptor::alloc()->init()->autorelease();
    texDescriptor->setTextureType(MTL::TextureType1DArray);
    texDescriptor->setWidth(256);
    texDescriptor->setPixelFormat(MTL::PixelFormatR16Uint);
    texDescriptor->setArrayLength(3);
    
    fDevice.fGammaLUTTexture = fDevice.fMetalDevice->newTexture(texDescriptor);
    
    fDevice.fGammaLUTTexture->replaceRegion(MTL::Region(0, 256), 0, 0, tabR, 256 * sizeof(uint16_t), 0);
    fDevice.fGammaLUTTexture->replaceRegion(MTL::Region(0, 256), 0, 1, tabG, 256 * sizeof(uint16_t), 0);
    fDevice.fGammaLUTTexture->replaceRegion(MTL::Region(0, 256), 0, 2, tabB, 256 * sizeof(uint16_t), 0);
    
    return true;
}

bool plMetalPipeline::CaptureScreen(plMipmap *dest, bool flipVertical, uint16_t desiredWidth, uint16_t desiredHeight)
{
    //FIXME: Screen capture
    return false;
}

plMipmap *plMetalPipeline::ExtractMipMap(plRenderTarget *targ)
{
    if( plCubicRenderTarget::ConvertNoRef(targ) )
        return nullptr;

    if( targ->GetPixelSize() != 32 )
    {
        hsAssert(false, "Only RGBA8888 currently implemented");
        return nullptr;
    }
    
    plMetalRenderTargetRef* ref = (plMetalRenderTargetRef*)targ->GetDeviceRef();
    if( !ref )
        return nullptr;
    
    const int width = targ->GetWidth();
    const int height = targ->GetHeight();

    plMipmap* mipMap = new plMipmap(width, height, plMipmap::kARGB32Config, 1);

    uint8_t* ptr = (uint8_t*)(ref->fTexture->buffer()->contents());
    const int pitch = ref->fTexture->width() * 4;
    
    ref->fTexture->getBytes(mipMap->GetAddr32(0, 0), pitch, MTL::Region(0, 0, width, height), 0);

    const uint32_t blackOpaque = 0xff000000;
    int y;
    for( y = 0; y < height; y++ )
    {
        uint32_t* destPtr = mipMap->GetAddr32(0, y);
        uint32_t* srcPtr = (uint32_t*)destPtr;
        int x;
        for( x = 0; x < width; x++ )
        {
            destPtr[x] = srcPtr[x] | blackOpaque;
        }
        ptr += pitch;
    }
    
    return mipMap;
}

void plMetalPipeline::GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth)
{
    /*
     There are decisions to make here.
     
     Modern macOS does not support "display modes." You panel runs at native resolution at all times, and you can over-render or under-render. But you never set the display mode of the panel, or get the display mode of the panel. Most games have a "scale slider."
     
     Note: There are legacy APIs for display modes for compatibility with older software. In since we're here writing a new renderer, lets do things the right way. The display mode APIs also have trouble with density. I.E. a 4k display might be reported as a 2k display if the window manager is running in a higher DPI mode.
     
     The basic approach should be to render at whatever the resolution of our output surface is. We're mostly doing that now (aspect ratio doesn't adjust.)
     
     Ideally we should support some sort of scaling/semi dynamic renderbuffer resolution thing. But don't mess with the window servers framebuffer size. macOS has accelerated resolution scaling like consoles do. Use that.
     */
    
    std::vector<plDisplayMode> supported;
    plDisplayMode mode;
    mode.Width = 800;
    mode.Height = 600;
    mode.ColorDepth = 32;
    supported.push_back(mode);
    
    *res = supported;
}

int plMetalPipeline::GetMaxAnisotropicSamples()
{
    //Metal always supports 16. There is no device check (as far as I know.)
    return 16;
}

int plMetalPipeline::GetMaxAntiAlias(int Width, int Height, int ColorDepth)
{
    //Metal devices may not support the full antialias range
    //return the max and we'll work it out later
    if (fDevice.fMetalDevice->supportsTextureSampleCount(8)) {
        return 8;
    }
    if (fDevice.fMetalDevice->supportsTextureSampleCount(4)) {
        return 4;
    }
    if (fDevice.fMetalDevice->supportsTextureSampleCount(2)) {
        return 2;
    }
    return 1;
}

void plMetalPipeline::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync)
{
    //FIXME: Whats this?
    //Seems like an entry point for passing in display settings.
    
    fDevice.SetMaxAnsiotropy(MaxAnisotropicSamples);
}

void plMetalPipeline::RenderSpans(plDrawableSpans *ice, const std::vector<int16_t> &visList)
{
    plProfile_BeginTiming(RenderSpan);

    hsMatrix44 lastL2W;
    size_t i, j;
    hsGMaterial* material;
    const std::vector<plSpan*>& spans = ice->GetSpanArray();

    //plProfile_IncCount(EmptyList, !visList.GetCount());

    /// Set this (*before* we do our TestVisibleWorld stuff...)
    lastL2W.Reset();
    ISetLocalToWorld(lastL2W, lastL2W);     // This is necessary; otherwise, we have to test for
                                            // the first transform set, since this'll be identity
                                            // but the actual device transform won't be (unless
                                            // we do this)


    /// Loop through our spans, combining them when possible
    for (i = 0; i < visList.size(); ) {
        if (GetOverrideMaterial() != nullptr) {
            material = GetOverrideMaterial();
        } else {
            material = ice->GetMaterial(spans[visList[i]]->fMaterialIdx);
        }

        /// It's an icicle--do our icicle merge loop
        plIcicle tempIce(*((plIcicle*)spans[visList[i]]));

        // Start at i + 1, look for as many spans as we can add to tempIce
        for (j = i + 1; j < visList.size(); j++) {
            if (GetOverrideMaterial()) {
                tempIce.fMaterialIdx = spans[visList[j]]->fMaterialIdx;
            }

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
            plMetalMaterialShaderRef* mRef = (plMetalMaterialShaderRef*)material->GetDeviceRef();

            if (mRef == nullptr) {
                mRef = new plMetalMaterialShaderRef(material, this);
                material->SetDeviceRef(mRef);
            }

            if (!mRef->IsLinked()) {
                mRef->Link(&fMatRefList);
            }
            
            hsGDeviceRef* vb = ice->GetVertexRef( tempIce.fGroupIdx, tempIce.fVBufferIdx );
            plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)vb;

            // What do we change?

            plProfile_BeginTiming(SpanTransforms);
            ISetupTransforms(ice, tempIce, lastL2W);
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
                                vb,
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

void plMetalPipeline::ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W)
{
    if (span.fNumMatrices) {
        if (span.fNumMatrices <= 2) {
            ISetLocalToWorld( span.fLocalToWorld, span.fWorldToLocal );
            lastL2W = span.fLocalToWorld;
        } else {
            lastL2W.Reset();
            ISetLocalToWorld( lastL2W, lastL2W );
            fView.fLocalToWorldLeftHanded = span.fLocalToWorld.GetParity();
        }
    } else if (lastL2W != span.fLocalToWorld) {
        ISetLocalToWorld( span.fLocalToWorld, span.fWorldToLocal );
        lastL2W = span.fLocalToWorld;
    } else {
        fView.fLocalToWorldLeftHanded = lastL2W.GetParity();
    }

    if( span.fNumMatrices == 2 )
    {
        matrix_float4x4 mat;
        hsMatrix2SIMD(drawable->GetPaletteMatrix(span.fBaseMatrix+1), &mat);
        fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&mat, sizeof(matrix_float4x4), BufferIndexBlendMatrix1);
    }

    fCurrentRenderPassUniforms->projectionMatrix = fDevice.fMatrixProj;
    fCurrentRenderPassUniforms->worldToCameraMatrix = fDevice.fMatrixW2C;
    fCurrentRenderPassUniforms->cameraToWorldMatrix = fDevice.fMatrixC2W;
    fCurrentRenderPassUniforms->localToWorldMatrix = fDevice.fMatrixL2W;
}

void plMetalPipeline::IRenderBufferSpan(const plIcicle& span, hsGDeviceRef* vb,
                                     hsGDeviceRef* ib, hsGMaterial* material,
                                     uint32_t vStart, uint32_t vLength,
                                     uint32_t iStart, uint32_t iLength)
{
    if(iLength == 0) {
        return;
    }
    
    plProfile_BeginTiming(RenderBuff);

    plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)vb;
    plMetalIndexBufferRef* iRef = (plMetalIndexBufferRef*)ib;
    plMetalMaterialShaderRef* mRef = (plMetalMaterialShaderRef*)material->GetDeviceRef();
    mRef->CheckMateralRef();

    if (!vRef || !vRef->GetBuffer() || !iRef->GetBuffer()) {
        plProfile_EndTiming(RenderBuff);

        hsAssert( false, ST::format("Trying to render a nil buffer pair! (Mat: {})", material->GetKeyName()).c_str() );
        return;
    }

    /* Index Buffer stuff and drawing */

    plRenderTriListFunc render(&fDevice, 0, vStart, vLength, iStart, iLength);

    plProfile_EndTiming(RenderBuff);

#if 1
    // Enable this for LayerAnimations, but the timing/speed seems wrong
    for (size_t i = 0; i < material->GetNumLayers(); i++) {
        plLayerInterface* lay = material->GetLayer(i);
        if (lay) {
            lay->Eval(fTime, fFrame, 0);
        }
    }
#endif
    
    // Turn on this spans lights and turn off the rest.
    ISelectLights(&span, mRef);
    
#ifdef _DEBUG
    fDevice.CurrentRenderCommandEncoder()->pushDebugGroup(NS::String::string(material->GetKeyName().c_str(), NS::UTF8StringEncoding));
#endif
    
    /* Vertex Buffer stuff */
    if(!vRef->GetBuffer()) {
        return;
    }
    if (fCurrentVertexBuffer != vRef->GetBuffer()) {
        fDevice.CurrentRenderCommandEncoder()->setVertexBuffer(vRef->GetBuffer(), 0, 0);
        fCurrentVertexBuffer = vRef->GetBuffer();
    }
    fDevice.fCurrentIndexBuffer = iRef->GetBuffer();
    
    IPushPiggyBacks(material);
    hsRefCnt_SafeAssign(fCurrMaterial, material);
    size_t pass;
    for (pass = 0; pass < mRef->GetNumPasses(); pass++) {
        
        if ( IHandleMaterial(material, pass, &span, vRef) ) {
            render.RenderPrims();
        }
        
        //Projection wants to do it's own lighting, push the current lighting state
        //so we can keep the same light calculations on the next pass
        PushCurrentLightSources();
        
        plProfile_BeginTiming(SelectProj);
        ISelectLights( &span, mRef, true );
        plProfile_EndTiming(SelectProj);
        
        // Take care of projections that get applied to each pass.
        if( fProjEach.size() && !(fView.fRenderState & kRenderNoProjection) )
        {
#ifdef _DEBUG
            fDevice.CurrentRenderCommandEncoder()->pushDebugGroup(NS::String::string("Render projections", NS::UTF8StringEncoding));
#endif
            IRenderProjectionEach(render, material, pass, span, vRef);
#ifdef _DEBUG
            fDevice.CurrentRenderCommandEncoder()->popDebugGroup();
#endif
        }
        //Revert the light state back to what we had before projections
        PopCurrentLightSources();
        
        if (IsDebugFlagSet(plPipeDbg::kFlagNoUpperLayers))
            pass = mRef->GetNumPasses();
    }
    
    IPopPiggyBacks();
    
    // Render any aux spans associated.
    if( span.GetNumAuxSpans() ) {
        IRenderAuxSpans(span);
        
        //aux spans will change the current vertex buffer, put ours back
        fDevice.CurrentRenderCommandEncoder()->setVertexBuffer(vRef->GetBuffer(), 0, 0);
        fCurrentVertexBuffer = vRef->GetBuffer();
    }
    
    

    // Only render projections and shadows if we successfully rendered the span.
    // j == -1 means we aborted render.
    if( pass >= 0 )
    {
        // Projections that get applied to the frame buffer (after all passes).
        //if( fLights.fProjAll.GetCount() && !(fView.fRenderState & kRenderNoProjection) )
        //    IRenderProjections(render);

        // Handle render of shadows onto geometry.
        if( fShadows.size() ) {
            //if we had to render aux spans, we probably changed the vertex and index buffer
            //reset those
            fCurrentVertexBuffer = vRef->GetBuffer();
            fDevice.fCurrentIndexBuffer = iRef->GetBuffer();
            
            IRenderShadowsOntoSpan(render, &span, material, vRef);
        }
    }
    
    if ( span.GetNumAuxSpans() || (pass >= 0 && fShadows.size()) ) {
    }
        
#ifdef _DEBUG
    fDevice.CurrentRenderCommandEncoder()->popDebugGroup();
#endif
}

// IRenderProjectionEach ///////////////////////////////////////////////////////////////////////////////////////
// Render any lights that are to be projected onto each pass of the object.
void plMetalPipeline::IRenderProjectionEach(const plRenderPrimFunc& render, hsGMaterial* material, int iPass, const plSpan& span, const plMetalVertexBufferRef* vRef)
{
    // If this is a bump map pass, forget it, we've already "done" per-pixel lighting.
    //if( fLayerState[iPass].fMiscFlags & (hsGMatState::kMiscBumpLayer | hsGMatState::kMiscBumpChans) )
    //    return;

    // Push the LayerShadowBase override. This sets the blend
    // to framebuffer as Add/ZNoWrite and AmbientColor = 0.
    static plLayerLightBase layLightBase;

    // For each projector:
    int k;
    for( k = 0; k < fProjEach.size(); k++ )
    {
        // Push it's projected texture as a piggyback.
        plLightInfo* li = fProjEach[k];
        plMetalMaterialShaderRef *mRef = (plMetalMaterialShaderRef *)material->GetDeviceRef();

        plLayerInterface* proj = li->GetProjection();
        hsAssert(proj, "A projector with no texture to project?");
        IPushProjPiggyBack(proj);

        // Enable the projecting light only.
        IEnableLight(mRef, 7, li);

        AppendLayerInterface(&layLightBase, false);

        IHandleMaterial( material, iPass, &span, vRef, false );
        
        //FIXME: Hard setting of light
        IScaleLight(mRef, 7, true);
        //mRef->encodeArguments(fDevice.CurrentRenderCommandEncoder(), fCurrentRenderPassUniforms, iPass, fActivePiggyBacks, &fPiggyBackStack, fOverBaseLayer);

        // Do the render with projection.
        render.RenderPrims();

        RemoveLayerInterface(&layLightBase, false);

        // Disable the projecting light
        IDisableLight(mRef, 7);

        // Pop it's projected texture off piggyback
        IPopProjPiggyBacks();

    }

}


// ICheckAuxBuffers ///////////////////////////////////////////////////////////////////////
// The AuxBuffers are associated with drawables for things to be drawn right after that
// drawable's contents. In particular, see the plDynaDecal, which includes things like
// water ripples, bullet hits, and footprints.
// This function just makes sure they are ready to be rendered, called right before
// the rendering.
bool plMetalPipeline::ICheckAuxBuffers(const plAuxSpan* span)
{
    plGBufferGroup* group = span->fGroup;

    plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx);
    if( !vRef )
        return true;

    plMetalIndexBufferRef* iRef = (plMetalIndexBufferRef*)group->GetIndexBufferRef(span->fIBufferIdx);
    if( !iRef )
        return true;

    // If our vertex buffer ref is volatile and the timestamp is off
    // then it needs to be refilled
    if( vRef->Expired(fVtxRefTime) )
    {
        IRefreshDynVertices(group, vRef);
    }

    return false; // No error
}

// IRenderAuxSpans ////////////////////////////////////////////////////////////////////////////
// Save and restore render state around calls to IRenderAuxSpan. This lets
// a list of aux spans get rendered with only one save/restore state.
void plMetalPipeline::IRenderAuxSpans(const plSpan& span)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoAuxSpans))
        return;

    ISetLocalToWorld(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());

    int i;
    for( i = 0; i < span.GetNumAuxSpans(); i++ )
        IRenderAuxSpan(span, span.GetAuxSpan(i));

    ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);

}

// IRenderAuxSpan //////////////////////////////////////////////////////////
// Aux spans (auxilliary) are geometry rendered immediately after, and therefore dependent, on
// other normal geometry. They don't have SceneObjects, Drawables, DrawInterfaces or
// any of that, and therefore don't correspond to any object in the scene.
// They are dynamic procedural decals. See plDynaDecal.cpp and plDynaDecalMgr.cpp.
// This is wrapped by IRenderAuxSpans, which makes sure state is restored to resume
// normal rendering after the AuxSpan is rendered.
void plMetalPipeline::IRenderAuxSpan(const plSpan& span, const plAuxSpan* aux)
{
    // Make sure the underlying resources are created and filled in with current data.
    CheckVertexBufferRef(aux->fGroup, aux->fVBufferIdx);
    CheckIndexBufferRef(aux->fGroup, aux->fIBufferIdx);
    ICheckAuxBuffers(aux);

    // Set to render from the aux spans buffers.
    plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)aux->fGroup->GetVertexBufferRef(aux->fVBufferIdx);

    if( !vRef )
        return;

    plMetalIndexBufferRef* iRef = (plMetalIndexBufferRef*)aux->fGroup->GetIndexBufferRef(aux->fIBufferIdx);

    if( !iRef )
        return;
    

    // Now just loop through the aux material, rendering in as many passes as it takes.
    hsGMaterial* material = aux->fMaterial;
    plMetalMaterialShaderRef* mRef = (plMetalMaterialShaderRef*)material->GetDeviceRef();
    
    if (mRef == nullptr) {
        mRef = new plMetalMaterialShaderRef(material, this);
        material->SetDeviceRef(mRef);
    }
    
    /* Vertex Buffer stuff */
    if(!vRef->GetBuffer()) {
        return;
    }
    fDevice.CurrentRenderCommandEncoder()->setVertexBuffer(vRef->GetBuffer(), 0, 0);
    fCurrentVertexBuffer = vRef->GetBuffer();
    fDevice.fCurrentIndexBuffer = iRef->GetBuffer();
    
    plRenderTriListFunc render(&fDevice, 0, aux->fVStartIdx, aux->fVLength, aux->fIStartIdx, aux->fILength);
    
    size_t pass;
    for (pass = 0; pass < mRef->GetNumPasses(); pass++) {
        IHandleMaterial(material, pass, &span, vRef);
        if( aux->fFlags & plAuxSpan::kOverrideLiteModel )
        {
            fCurrentRenderPassUniforms->ambientCol = {1.0f, 1.0f, 1.0f, 1.0f};
            
            fCurrentRenderPassUniforms->diffuseSrc = 1.0;
            fCurrentRenderPassUniforms->ambientSrc = 1.0;
            fCurrentRenderPassUniforms->emissiveSrc = 0.0;
            fCurrentRenderPassUniforms->specularSrc = 1.0;
        }
        
        render.RenderPrims();
    }

    /*HRESULT     r;

    r = fD3DDevice->SetStreamSource( 0, vRef->fD3DBuffer, 0, vRef->fVertexSize );
    hsAssert( r == D3D_OK, "Error trying to set the stream source!" );
    plProfile_Inc(VertexChange);

    fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = IGetBufferD3DFormat(vRef->fFormat));
    
    r = fD3DDevice->SetIndices( iRef->fD3DBuffer );
    hsAssert( r == D3D_OK, "Error trying to set the indices!" );

    plRenderTriListFunc render(fD3DDevice, iRef->fOffset, aux->fVStartIdx, aux->fVLength, aux->fIStartIdx, aux->fILength/3);
    int j;
    for( j = 0; j < material->GetNumLayers(); )
    {
        int iCurrMat = j;
        j = IHandleMaterial( material, iCurrMat, &span );
        if (j == -1)
            break;

        ISetShaders(material->GetLayer(iCurrMat)->GetVertexShader(), material->GetLayer(iCurrMat)->GetPixelShader());

        if( aux->fFlags & plAuxSpan::kOverrideLiteModel )
        {
            static D3DMATERIAL9 mat;
            fD3DDevice->SetRenderState(D3DRS_AMBIENT, 0xffffffff);

            fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
            fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
            fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
            fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

            fD3DDevice->SetMaterial( &mat );
        }

        render.RenderPrims();
    }*/

}

bool plMetalPipeline::IHandleMaterial(hsGMaterial *material, uint32_t pass, const plSpan *currSpan, const plMetalVertexBufferRef* vRef, const bool allowShaders)
{
    plMetalMaterialShaderRef* mRef = (plMetalMaterialShaderRef*)material->GetDeviceRef();
    
    fCurrLayerIdx = mRef->GetPassIndex(pass);
    //plLayerInterface* lay = material->GetLayer(mRef->GetPassIndex(pass));
    plLayerInterface *lay = material->GetLayer(mRef->GetPassIndex(pass));


    hsGMatState s;
    s.Composite(lay->GetState(), fMatOverOn, fMatOverOff);
    
    /*
     If the layer opacity is 0, don't draw it. This prevents it from contributing to the Z buffer.
     This can happen with some models like the fire marbles in the neighborhood that have some models
     for physics only, and then can block other rendering in the Z buffer.
     DX pipeline does this in ILoopOverLayers.
     */
    if( (s.fBlendFlags & hsGMatState::kBlendAlpha)
       &&lay->GetOpacity() <= 0
       &&(fCurrLightingMethod != plSpan::kLiteVtxPreshaded) ) {
        
        return false;
    }

    IHandleZMode(s);
    IHandleBlendMode(s);
    
    if (s.fMiscFlags & hsGMatState::kMiscTwoSided) {
        if(fCurrentCullMode != MTL::CullModeNone) {
            fDevice.CurrentRenderCommandEncoder()->setCullMode(MTL::CullModeNone);
            fCurrentCullMode =  MTL::CullModeNone;
        }
    } else {
        ISetCullMode();
    }
    
    
    //Some build passes don't allow shaders. Render the geometry and the provided material, but don't allow the shader path if instructed to. In the DX source, this would be done by the render phase setting the shaders to null after calling this. That won't work here in since our pipeline state has to know the shaders.
    if(lay->GetVertexShader() && allowShaders) {
        
        lay = IPushOverBaseLayer(lay);
        lay = IPushOverAllLayer(lay);
        
        //pure shader path
        plShader *vertexShader = lay->GetVertexShader();
        plShader *fragShader = lay->GetPixelShader();
        
        fCurrLay = lay;
        fCurrNumLayers = mRef->fPassLengths[pass];
        
        ISetShaders(vRef, s, vertexShader, fragShader);
        
        //FIXME: Programmable pipeline does not implement the full feature set
        /*
         The programmable pipeline doesn't do things like set the texture transform matrices,
         In practice, the transforms aren't set and used. Does it matter that the Metal
         implementation doesn't implemention the full inputs the DX version gets?
         
         If it is implemented, the same checks the DX version does should be also implemented.
         DX will set texture transforms, but then turn them off in the pipeline and manually
         manipulate texture co-ords in the shader.
         
         Texture setting should also _maybe_ be reconciled with the "fixed" pipeline. But
         the fixed pipeline uses indirect textures mapped to a buffer. That approach could
         work for the programmable pipeline too, but I'm planning changes to the fixed pipeline
         and the way it stores textures. So maybe things should be reconciled after that
         work is done.
         */
        
        for (size_t i = 0; i < material->GetNumLayers(); i++) {
            plLayerInterface* layer = material->GetLayer(i);
            if (!layer) {
                return false;
            }

            CheckTextureRef(layer);
            
            plBitmap* img = plBitmap::ConvertNoRef(layer->GetTexture());

            if (!img) {
                return false;
            }
            
            plMetalTextureRef* texRef = (plMetalTextureRef*)img->GetDeviceRef();

            if (!texRef->fTexture) {
                return false;
            }
            
            size_t idOffset = 0;
            //Metal doesn't like mixing 2D and cubic textures. If this is a cubic texture, make sure it lands in the right ID range.
            if(plCubicRenderTarget::ConvertNoRef( img )) {
                idOffset = FragmentShaderArgumentAttributeCubicTextures;
            }
            
            fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(texRef->fTexture, i + idOffset);
            
        }
        lay = IPopOverAllLayer(lay);
        lay = IPopOverBaseLayer(lay);
    } else {
        //"Fixed" path
        
        /*
         To compute correct lighting we need to add the pushover layers.
         The actual renderer will do it's own add and remove, so remove the
         pushover layer before we get to the actual layer loop.
         */
        lay = IPushOverBaseLayer(lay);
        lay = IPushOverAllLayer(lay);
        ICalcLighting(mRef, lay, currSpan);
        
        s.Composite(lay->GetState(), fMatOverOn, fMatOverOff);
        
        if (s.fBlendFlags & hsGMatState::kBlendInvertVtxAlpha)
            fCurrentRenderPassUniforms->invVtxAlpha = true;
        else
            fCurrentRenderPassUniforms->invVtxAlpha = false;
        
        std::vector<plLightInfo*>& spanLights = currSpan->GetLightList(false);
        
        int numActivePiggyBacks = 0;
        //FIXME: In the DX source, this check was done on the first layer. Does that mean the first layer of the material or the first layer of the pass?
        if( !(s.fMiscFlags & hsGMatState::kMiscBumpChans) && !(s.fShadeFlags & hsGMatState::kShadeEmissive) )
        {
            /// Tack lightmap onto last stage if we have one
            numActivePiggyBacks = fActivePiggyBacks;
            //if( numActivePiggyBacks > fMaxLayersAtOnce - fCurrNumLayers )
            //    numActivePiggyBacks = fMaxLayersAtOnce - fCurrNumLayers;
            
        }
        
        uint8_t sources[8];
        uint32_t blendModes[8];
        uint32_t miscFlags[8];
        uint8_t sampleTypes[8];
        memset(sources, 0, sizeof(sources));
        memset(blendModes, 0, sizeof(blendModes));
        memset(miscFlags, 0, sizeof(miscFlags));
        memset(sampleTypes, 0, sizeof(sampleTypes));
        
        lay = IPopOverAllLayer(lay);
        lay = IPopOverBaseLayer(lay);
        
        if(numActivePiggyBacks==0 && fOverBaseLayer == nullptr && fOverAllLayer == nullptr) {
            mRef->FastEncodeArguments(fDevice.CurrentRenderCommandEncoder(), fCurrentRenderPassUniforms, pass);
            
            mRef->GetSourceArray(sources, pass);
            mRef->GetBlendFlagArray(blendModes, pass);
            mRef->GetMiscFlagArray(miscFlags, pass);
            mRef->GetSampleTypeArray(sampleTypes, pass);
        } else {
        
            //Plasma pulls piggybacks from the rear first, pull the number of active piggybacks
            auto firstPiggyback = fPiggyBackStack.end() - numActivePiggyBacks;
            auto lastPiggyback = fPiggyBackStack.end();
            std::vector<plLayerInterface*> subPiggybacks(firstPiggyback, lastPiggyback);
            mRef->EncodeArguments(fDevice.CurrentRenderCommandEncoder(), fCurrentRenderPassUniforms, pass, &subPiggybacks,
                                  [&](plLayerInterface* layer, uint32_t index){
                if(index==0) {
                    layer = IPushOverBaseLayer(layer);
                }
                layer = IPushOverAllLayer(layer);
                
                plBitmap* texture = layer->GetTexture();
                if (texture != nullptr) {
                    plMetalTextureRef *deviceTexture = (plMetalTextureRef *)texture->GetDeviceRef();
                    if (plCubicEnvironmap::ConvertNoRef(texture) != nullptr || plCubicRenderTarget::ConvertNoRef(texture) != nullptr) {
                        sources[index] = PassTypeCubicTexture;
                    } else if (plMipmap::ConvertNoRef(texture) != nullptr || plRenderTarget::ConvertNoRef(texture) != nullptr) {
                        sources[index] = PassTypeTexture;
                    }
                    
                } else {
                    sources[index] = PassTypeColor;
                }
                blendModes[index] = layer->GetBlendFlags();
                miscFlags[index] = layer->GetMiscFlags();
                
                switch (layer->GetClampFlags()) {
                case hsGMatState::kClampTextureU:
                        sampleTypes[index] = 1;
                    break;
                case hsGMatState::kClampTextureV:
                        sampleTypes[index] = 2;
                    break;
                case hsGMatState::kClampTexture:
                        sampleTypes[index] = 3;
                    break;
                default:
                        sampleTypes[index] = 0;
                        break;
                }
                
                return layer;
            },
                                  [&](plLayerInterface* layer, uint32_t index){
                layer = IPopOverAllLayer(layer);
                if(index==0)
                    layer = IPopOverBaseLayer(layer);
                return layer;
            });
        }
        
         struct plMetalMaterialPassDescription passDescription;
         memcpy(passDescription.passTypes, sources, sizeof(sources));
         memcpy(passDescription.blendModes, blendModes, sizeof(blendModes));
         memcpy(passDescription.miscFlags, miscFlags, sizeof(miscFlags));
         memcpy(passDescription.sampleTypes, sampleTypes, sizeof(sampleTypes));
         passDescription.numLayers = numActivePiggyBacks + mRef->fPassLengths[pass];
         
         plMetalDevice::plMetalLinkedPipeline *linkedPipeline = plMetalMaterialPassPipelineState(&fDevice, vRef, passDescription).GetRenderPipelineState();
         const MTL::RenderPipelineState *pipelineState = linkedPipeline->pipelineState;
        
        /*plMetalDevice::plMetalLinkedPipeline *pipeline = fDevice.pipelineStateFor(vRef, s.fBlendFlags, numActivePiggyBacks + mRef->fPassLengths[pass], plShaderID::Unregistered, plShaderID::Unregistered, sources, blendModes, miscFlags);
        const MTL::RenderPipelineState *pipelineState = pipeline->pipelineState;*/
        if(fCurrentPipelineState != pipelineState) {
            fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(pipelineState);
            fCurrentPipelineState = pipelineState;
        }
    }
    
    return true;
}

// ISetPipeConsts //////////////////////////////////////////////////////////////////
// A shader can request that the pipeline fill in certain constants that are indeterminate
// until the pipeline is about to render the object the shader is applied to. For example,
// the object's local to world. A single shader may be used on multiple objects with
// multiple local to world transforms. This ensures the pipeline will shove the proper
// local to world into the shader immediately before the render.
// See plShader.h for the list of available pipe constants.
// Note that the lighting pipe constants are NOT implemented.
void plMetalPipeline::ISetPipeConsts(plShader* shader)
{
    int n = shader->GetNumPipeConsts();
    int i;
    for( i = 0; i < n; i++ )
    {
        const plPipeConst& pc = shader->GetPipeConst(i);
        switch( pc.fType )
        {
        case plPipeConst::kFogSet:
            {
                float set[4];
                //FIXME: Fog broken in dynamic pipeline
                //IGetVSFogSet(set);
                //shader->SetFloat4(pc.fReg, set);
            }
            break;
        case plPipeConst::kLayAmbient:
            {
                hsColorRGBA col = fCurrLay->GetAmbientColor();
                shader->SetColor(pc.fReg, col);
            }
            break;
        case plPipeConst::kLayRuntime:
            {
                hsColorRGBA col = fCurrLay->GetRuntimeColor();
                col.a = fCurrLay->GetOpacity();
                shader->SetColor(pc.fReg, col);
            }
            break;
        case plPipeConst::kLaySpecular:
            {
                hsColorRGBA col = fCurrLay->GetSpecularColor();
                shader->SetColor(pc.fReg, col);
            }
            break;
        case plPipeConst::kTex3x4_0:
        case plPipeConst::kTex3x4_1:
        case plPipeConst::kTex3x4_2:
        case plPipeConst::kTex3x4_3:
        case plPipeConst::kTex3x4_4:
        case plPipeConst::kTex3x4_5:
        case plPipeConst::kTex3x4_6:
        case plPipeConst::kTex3x4_7:
            {
                int stage = pc.fType - plPipeConst::kTex3x4_0;

                if( stage > fCurrNumLayers )
                {
                    // Ooops. This is bad, means the shader is expecting more layers than
                    // we actually have (or is just bogus). Assert and quietly continue.
                    hsAssert(false, "Shader asking for higher stage transform than we have");
                    continue;
                }
                const hsMatrix44& xfm = fCurrMaterial->GetLayer(fCurrLayerIdx + stage)->GetTransform();

                shader->SetMatrix34(pc.fReg, xfm);
            }
            break;
        case plPipeConst::kTex2x4_0:
        case plPipeConst::kTex2x4_1:
        case plPipeConst::kTex2x4_2:
        case plPipeConst::kTex2x4_3:
        case plPipeConst::kTex2x4_4:
        case plPipeConst::kTex2x4_5:
        case plPipeConst::kTex2x4_6:
        case plPipeConst::kTex2x4_7:
            {
                int stage = pc.fType - plPipeConst::kTex2x4_0;

                if( stage > fCurrNumLayers )
                {
                    // Ooops. This is bad, means the shader is expecting more layers than
                    // we actually have (or is just bogus). Assert and quietly continue.
                    hsAssert(false, "Shader asking for higher stage transform than we have");
                    continue;
                }
                const hsMatrix44& xfm = fCurrMaterial->GetLayer(fCurrLayerIdx + stage)->GetTransform();

                shader->SetMatrix24(pc.fReg, xfm);
            }
            break;
        case plPipeConst::kTex1x4_0:
        case plPipeConst::kTex1x4_1:
        case plPipeConst::kTex1x4_2:
        case plPipeConst::kTex1x4_3:
        case plPipeConst::kTex1x4_4:
        case plPipeConst::kTex1x4_5:
        case plPipeConst::kTex1x4_6:
        case plPipeConst::kTex1x4_7:
            {
                int stage = pc.fType - plPipeConst::kTex1x4_0;

                if( stage > fCurrNumLayers )
                {
                    // Ooops. This is bad, means the shader is expecting more layers than
                    // we actually have (or is just bogus). Assert and quietly continue.
                    hsAssert(false, "Shader asking for higher stage transform than we have");
                    continue;
                }
                const hsMatrix44& xfm = fCurrMaterial->GetLayer(fCurrLayerIdx + stage)->GetTransform();

                shader->SetFloat4(pc.fReg, xfm.fMap[0]);
            }
            break;
        case plPipeConst::kLocalToNDC:
            {
                hsMatrix44 cam2ndc = IGetCameraToNDC();
                hsMatrix44 world2cam = GetViewTransform().GetWorldToCamera();

                hsMatrix44 local2ndc = cam2ndc * world2cam * GetLocalToWorld();

                shader->SetMatrix44(pc.fReg, local2ndc);
            }
            break;

        case plPipeConst::kCameraToNDC:
            {
                hsMatrix44 cam2ndc = IGetCameraToNDC();

                shader->SetMatrix44(pc.fReg, cam2ndc);
            }
            break;

        case plPipeConst::kWorldToNDC:
            {
                hsMatrix44 cam2ndc = IGetCameraToNDC();
                hsMatrix44 world2cam = GetViewTransform().GetWorldToCamera();

                hsMatrix44 world2ndc = cam2ndc * world2cam;

                shader->SetMatrix44(pc.fReg, world2ndc);
            }
            break;

        case plPipeConst::kLocalToWorld:
            shader->SetMatrix34(pc.fReg, GetLocalToWorld());
            break;

        case plPipeConst::kWorldToLocal:
            shader->SetMatrix34(pc.fReg, GetWorldToLocal());
            break;

        case plPipeConst::kWorldToCamera:
            {
                hsMatrix44 world2cam = GetViewTransform().GetWorldToCamera();

                shader->SetMatrix34(pc.fReg, world2cam);
            }
            break;

        case plPipeConst::kCameraToWorld:
            {
                hsMatrix44 cam2world = GetViewTransform().GetCameraToWorld();

                shader->SetMatrix34(pc.fReg, cam2world);
            }
            break;

        case plPipeConst::kLocalToCamera:
            {
                hsMatrix44 world2cam = GetViewTransform().GetWorldToCamera();

                hsMatrix44 local2cam = world2cam * GetLocalToWorld();

                shader->SetMatrix34(pc.fReg, local2cam);
            }
            break;

        case plPipeConst::kCameraToLocal:
            {
                hsMatrix44 cam2world = GetViewTransform().GetCameraToWorld();

                hsMatrix44 cam2local = GetWorldToLocal() * cam2world;

                shader->SetMatrix34(pc.fReg, cam2local);
            }
            break;

        case plPipeConst::kCamPosWorld:
            {
                shader->SetVectorW(pc.fReg, GetViewTransform().GetCameraToWorld().GetTranslate(), 1.f);
            }
            break;

        case plPipeConst::kCamPosLocal:
            {
                hsPoint3 localCam = GetWorldToLocal() * GetViewTransform().GetCameraToWorld().GetTranslate();

                shader->SetVectorW(pc.fReg, localCam, 1.f);
            }
            break;

        case plPipeConst::kObjPosWorld:
            {
                shader->SetVectorW(pc.fReg, GetLocalToWorld().GetTranslate(), 1.f);
            }
            break;

        // UNIMPLEMENTED
        case plPipeConst::kDirLight1:
        case plPipeConst::kDirLight2:
        case plPipeConst::kDirLight3:
        case plPipeConst::kDirLight4:
        case plPipeConst::kPointLight1:
        case plPipeConst::kPointLight2:
        case plPipeConst::kPointLight3:
        case plPipeConst::kPointLight4:
            break;
        }
    }
}

// ISetShaders /////////////////////////////////////////////////////////////////////////////////////
// Setup to render using the input vertex and pixel shader. Either or both may
// be nil, in which case the fixed function pipeline is indicated.
// Any Pipe Constants the non-FFP shader wants will be set here.
// Lastly, all constants will be set (as a block) for any non-FFP vertex or pixel shader.
bool plMetalPipeline::ISetShaders(const plMetalVertexBufferRef * vRef, const hsGMatState blendMode, plShader* vShader, plShader* pShader)
{
    hsAssert(vShader, "Can't handle programmable passes without vShader");
    hsAssert(pShader, "Can't handle programmable passes without pShader");
    plShaderID::ID vertexShaderID = vShader->GetDecl()->GetID();
    plShaderID::ID fragmentShaderID = pShader->GetDecl()->GetID();
    
    plMetalDevice::plMetalLinkedPipeline *pipeline = plMetalDynamicMaterialPipelineState(&fDevice, vRef, blendMode.fBlendFlags, vertexShaderID, fragmentShaderID).GetRenderPipelineState();
    if(fCurrentPipelineState != pipeline->pipelineState) {
        fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(pipeline->pipelineState);
        fCurrentPipelineState = pipeline->pipelineState;
    }
    
    if( vShader )
    {
        hsAssert(vShader->IsVertexShader(), "Wrong type shader as vertex shader");
        ISetPipeConsts(vShader);
        
        plMetalVertexShader* vRef = (plMetalVertexShader*)vShader->GetDeviceRef();
        if( !vRef )
        {
            vRef = new plMetalVertexShader(vShader);
            hsRefCnt_SafeUnRef(vRef);
        }
        if( !vRef->IsLinked() )
            vRef->Link(&fVShaderRefList);
        
        vRef->ISetConstants(this);
    }

    if( pShader )
    {
        hsAssert(pShader->IsPixelShader(), "Wrong type shader as pixel shader");

        ISetPipeConsts(pShader);
        
        plMetalFragmentShader* pRef = (plMetalFragmentShader*)pShader->GetDeviceRef();
        if( !pRef )
        {
            pRef = new plMetalFragmentShader(pShader);
            hsRefCnt_SafeUnRef(pRef);
        }
        if( !pRef->IsLinked() )
            pRef->Link(&fPShaderRefList);
        
        pRef->ISetConstants(this);
    }

    /*if( vsHandle != fSettings.fCurrVertexShader )
    {
        HRESULT hr = fD3DDevice->SetVertexShader(fSettings.fCurrVertexShader = vsHandle);
        hsAssert(!FAILED(hr), "Error setting vertex shader");
    }

    if( psHandle != fSettings.fCurrPixelShader )
    {
        HRESULT hr = fD3DDevice->SetPixelShader(fSettings.fCurrPixelShader = psHandle);
        hsAssert(!FAILED(hr), "Error setting pixel shader");
    }*/

    // Handle cull mode here, because current cullmode is dependent on
    // the handedness of the LocalToCamera AND whether we are twosided.
    ISetCullMode();

    return true;
}

bool plMetalPipeline::ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* spanBase)
{
    if (!(spanBase->fTypeMask & plSpan::kVertexSpan))
        return false;
    // If we arent' an trilist, we're toast.
    if (!(spanBase->fTypeMask & plSpan::kIcicleSpan))
        return false;

    plIcicle* span = (plIcicle*)spanBase;

    plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx);
    if (!vRef)
        return true;

    plMetalIndexBufferRef* iRef = (plMetalIndexBufferRef*)group->GetIndexBufferRef(span->fIBufferIdx);
    if (!iRef)
        return true;

    // If our vertex buffer ref is volatile and the timestamp is off
    // then it needs to be refilled
    //MTL::PurgeableState bufferState = vRef->fVertexBuffer->setPurgeableState(MTL::PurgeableStateNonVolatile);
    if (vRef->Expired(fVtxRefTime)) {
        IRefreshDynVertices(group, vRef);
        //fDevice.GetCurrentCommandBuffer()->addCompletedHandler( ^(MTL::CommandBuffer *buffer) {
            //vRef->fVertexBuffer->setPurgeableState(MTL::PurgeableStateVolatile);
        //});
    }

    if (iRef->IsDirty()) {
        fDevice.FillIndexBufferRef(iRef, group, span->fIBufferIdx);
        iRef->SetRebuiltSinceUsed(true);
    }

    return false; // No error
}

bool plMetalPipeline::IRefreshDynVertices(plGBufferGroup* group, plMetalVertexBufferRef* vRef)
{
    ptrdiff_t size = (group->GetVertBufferEnd(vRef->fIndex) - group->GetVertBufferStart(vRef->fIndex)) * vRef->fVertexSize;
    if (!size)
        return false; // No error, just nothing to do.

    hsAssert(size > 0, "Bad start and end counts in a group");

    if (!vRef->GetBuffer())
    {
        hsAssert(size > 0, "Being asked to fill a buffer that doesn't exist yet?");
    }

    uint8_t* vData;
    if (vRef->fData)
        vData = vRef->fData;
    else
        vData = group->GetVertBufferData(vRef->fIndex) + group->GetVertBufferStart(vRef->fIndex) * vRef->fVertexSize;
    
    vRef->PrepareForWrite();

    MTL::Buffer* vertexBuffer = vRef->GetBuffer();
    if(!vertexBuffer || vertexBuffer->length() < size) {
        //Plasma will present different length buffers at different times
        vertexBuffer = fDevice.fMetalDevice->newBuffer(vData, size, MTL::ResourceStorageModeManaged)->autorelease();
        if(vRef->Volatile()) {
            fDevice.GetCurrentCommandBuffer()->addCompletedHandler(^(MTL::CommandBuffer* buffer){
                //vRef->fVertexBuffer->setPurgeableState(MTL::PurgeableStateVolatile);
            });
        }
        vRef->SetBuffer(vertexBuffer);
    } else {
        memcpy(vertexBuffer->contents(),
               vData,
               size);
        vertexBuffer->didModifyRange(NS::Range(0, size));
    }

    vRef->fRefTime = fVtxRefTime;
    vRef->SetDirty(false);

    return false;
}

void plMetalPipeline::IHandleZMode(hsGMatState flags)
{
    //Metal is very particular that if there is no depth buffer we need to explictly disable z read and write
    if(fDevice.fCurrentDepthFormat == MTL::PixelFormatInvalid) {
        fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(fDevice.fNoZReadOrWriteStencilState);
        return;
    }
    
    MTL::DepthStencilState *newDepthState;
    switch (flags.fZFlags & hsGMatState::kZMask)
    {
        case hsGMatState::kZClearZ:
            //FIXME: Clear should actually clear the Z target
            newDepthState = fDevice.fNoZReadStencilState;
            break;
        case hsGMatState::kZNoZRead:
            newDepthState = fDevice.fNoZReadStencilState;
            break;
        case hsGMatState::kZNoZWrite:
            newDepthState = fDevice.fNoZWriteStencilState;
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZClearZ:
            newDepthState = fDevice.fNoZReadStencilState;
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite:
            newDepthState = fDevice.fNoZReadOrWriteStencilState;
            break;
        case 0:
            newDepthState = fDevice.fDefaultStencilState;
            break;
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite:
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead:
            hsAssert(false, "Illegal combination of Z Buffer modes (Clear but don't write)");
            break;
    }
    
    if(fCurrentDepthStencilState != newDepthState) {
        fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(newDepthState);
        fCurrentDepthStencilState = newDepthState;
    }

    if (flags.fZFlags & hsGMatState::kZIncLayer) {
        fDevice.CurrentRenderCommandEncoder()->setDepthBias(0.0, -2.0, -2.0);
    } else {
        fDevice.CurrentRenderCommandEncoder()->setDepthBias(0.0, 0.0, 0.0);
    }
}

void plMetalPipeline::IHandleBlendMode(hsGMatState flags)
{
    // No color, just writing out Z values.
    if (flags.fBlendFlags & hsGMatState::kBlendNoColor) {
        //printf("glBlendFunc(GL_ZERO, GL_ONE);\n");
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
                        //printf("glBlendFunc(GL_ONE, GL_SRC_ALPHA);\n");
                    } else {
                        //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);\n");
                    }
                } else {
                    if (flags.fBlendFlags & hsGMatState::kBlendAlphaPremultiplied) {
                        //printf("glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);\n");
                    } else {
                        //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);\n");
                    }
                }
                break;

            // Multiply the final color onto the frame buffer.
            case hsGMatState::kBlendMult:
                if (flags.fBlendFlags & hsGMatState::kBlendInvertFinalColor) {
                    //printf("glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);\n");
                } else {
                    //printf("glBlendFunc(GL_ZERO, GL_SRC_COLOR);\n");
                }
                break;

            // Add final color to FB.
            case hsGMatState::kBlendAdd:
                //printf("glBlendFunc(GL_ONE, GL_ONE);\n");
                break;

            // Multiply final color by FB color and add it into the FB.
            case hsGMatState::kBlendMADD:
                //printf("glBlendFunc(GL_DST_COLOR, GL_ONE);\n");
                break;

            // Final color times final alpha, added into the FB.
            case hsGMatState::kBlendAddColorTimesAlpha:
                if (flags.fBlendFlags & hsGMatState::kBlendInvertFinalAlpha) {
                    //printf("glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_ONE);\n");
                } else {
                    //printf("glBlendFunc(GL_SRC_ALPHA, GL_ONE);\n");
                }
                break;

            // Overwrite final color onto FB
            case 0:
                //printf("glBlendFunc(GL_ONE, GL_ZERO);\n");
                break;

            default:
                {
                    hsAssert(false, "Too many blend modes specified in material");
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
                }
                break;
        }
    }
}

void plMetalPipeline::ICalcLighting(plMetalMaterialShaderRef* mRef, const plLayerInterface* currLayer, const plSpan* currSpan)
{
    //plProfile_Inc(MatLightState);

    if (IsDebugFlagSet(plPipeDbg::kFlagAllBright))
    {
        fCurrentRenderPassUniforms->globalAmb = { 1.0, 1.0, 1.0, 1.0 };

        fCurrentRenderPassUniforms->ambientCol = { 1.0, 1.0, 1.0, 1.0 };
        fCurrentRenderPassUniforms->diffuseCol = { 1.0, 1.0, 1.0, 1.0 };
        fCurrentRenderPassUniforms->emissiveCol = { 1.0, 1.0, 1.0, 1.0 };
        fCurrentRenderPassUniforms->emissiveCol = { 1.0, 1.0, 1.0, 1.0 };
        fCurrentRenderPassUniforms->specularCol = { 1.0, 1.0, 1.0, 1.0 };

        fCurrentRenderPassUniforms->ambientSrc =  1.0;
        fCurrentRenderPassUniforms->diffuseSrc =  1.0;
        fCurrentRenderPassUniforms->emissiveSrc = 1.0;
        fCurrentRenderPassUniforms->specularSrc = 1.0;

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
                fCurrentRenderPassUniforms->globalAmb = { 1.0, 1.0, 1.0, 1.0 };
                fCurrentRenderPassUniforms->ambientCol = { 1.0, 1.0, 1.0, 1.0 };
            } else if (IsDebugFlagSet(plPipeDbg::kFlagNoPreShade)) {
                fCurrentRenderPassUniforms->globalAmb = { 0.0, 0.0, 0.0, 1.0 };
                fCurrentRenderPassUniforms->ambientCol = { 0.0, 0.0, 0.0, 1.0 };
            } else {
                hsColorRGBA amb = currLayer->GetPreshadeColor();
                fCurrentRenderPassUniforms->globalAmb = { static_cast<half>(amb.r), static_cast<half>(amb.g), static_cast<half>(amb.b), 1.0 };
                fCurrentRenderPassUniforms->ambientCol = { static_cast<half>(amb.r), static_cast<half>(amb.g), static_cast<half>(amb.b), 1.0 };
            }

            hsColorRGBA dif = currLayer->GetRuntimeColor();
            fCurrentRenderPassUniforms->diffuseCol = { static_cast<half>(dif.r), static_cast<half>(dif.g), static_cast<half>(dif.b), static_cast<half>(currLayer->GetOpacity()) };

            hsColorRGBA em = currLayer->GetAmbientColor();
            fCurrentRenderPassUniforms->emissiveCol = { static_cast<half>(em.r), static_cast<half>(em.g), static_cast<half>(em.b), 1.0 };

            // Set specular properties
            if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
                hsColorRGBA spec = currLayer->GetSpecularColor();
                fCurrentRenderPassUniforms->specularCol = { static_cast<half>(spec.r), static_cast<half>(spec.g), static_cast<half>(spec.b), 1.0 };
#if 0
                mat.Power = currLayer->GetSpecularPower();
#endif
            } else {
                fCurrentRenderPassUniforms->specularCol = { 0.0, 0.0, 0.0, 0.0 };
            }

            fCurrentRenderPassUniforms->diffuseSrc = 1.0;
            fCurrentRenderPassUniforms->emissiveSrc = 1.0;
            fCurrentRenderPassUniforms -> specularSrc = 1.0;

            if (state.fShadeFlags & hsGMatState::kShadeNoShade) {
                fCurrentRenderPassUniforms->ambientSrc = 1.0;
            } else {
                fCurrentRenderPassUniforms->ambientSrc = 0.0;
            }
            fCurrLightingMethod = plSpan::kLiteMaterial;

            break;
        }

        case plSpan::kLiteVtxPreshaded:  // Vtx preshaded
        {
            fCurrentRenderPassUniforms->globalAmb = { 0.0, 0.0, 0.0, 0.0 };
            fCurrentRenderPassUniforms->ambientCol = { 0.0, 0.0, 0.0, 0.0 };
            fCurrentRenderPassUniforms->diffuseCol = { 0.0, 0.0, 0.0, 0.0 };
            fCurrentRenderPassUniforms->emissiveCol = { 0.0, 0.0, 0.0, 0.0 };
            fCurrentRenderPassUniforms->specularCol = { 0.0, 0.0, 0.0, 0.0 };

            fCurrentRenderPassUniforms->diffuseSrc = 0.0;
            fCurrentRenderPassUniforms->ambientSrc = 1.0;
            fCurrentRenderPassUniforms->specularSrc = 1.0;

            if (state.fShadeFlags & hsGMatState::kShadeEmissive) {
                fCurrentRenderPassUniforms->emissiveSrc = 0.0;
            } else {
                fCurrentRenderPassUniforms->emissiveSrc = 1.0;
            }
            
            fCurrLightingMethod = plSpan::kLiteVtxPreshaded;
            break;
        }

        case plSpan::kLiteVtxNonPreshaded:      // Vtx non-preshaded
        {
            fCurrentRenderPassUniforms->ambientCol = { 0.0, 0.0, 0.0, 0.0 };
            fCurrentRenderPassUniforms->diffuseCol = { 0.0, 0.0, 0.0, 0.0 };

            hsColorRGBA em = currLayer->GetAmbientColor();
            fCurrentRenderPassUniforms->emissiveCol = { static_cast<half>(em.r), static_cast<half>(em.g), static_cast<half>(em.b), 1.0 };

            // Set specular properties
            if (state.fShadeFlags & hsGMatState::kShadeSpecular) {
                hsColorRGBA spec = currLayer->GetSpecularColor();
                fCurrentRenderPassUniforms->specularCol = { static_cast<half>(spec.r), static_cast<half>(spec.g), static_cast<half>(spec.b), 1.0 };
#if 0
                mat.Power = currLayer->GetSpecularPower();
#endif
            } else {
                fCurrentRenderPassUniforms->specularCol = { 0.0, 0.0, 0.0, 0.0 };
            }

            hsColorRGBA amb = currLayer->GetPreshadeColor();
            fCurrentRenderPassUniforms->globalAmb = { static_cast<half>(amb.r), static_cast<half>(amb.g), static_cast<half>(amb.b), static_cast<half>(amb.a) };

            fCurrentRenderPassUniforms->ambientSrc = 0.0;
            fCurrentRenderPassUniforms->diffuseSrc = 0.0;
            fCurrentRenderPassUniforms->emissiveSrc = 1.0;
            fCurrentRenderPassUniforms->specularSrc = 1.0;
            
            fCurrLightingMethod = plSpan::kLiteVtxNonPreshaded;
            break;
        }
    }
    // Piggy-back some temporary fog stuff on the lighting...
    const plFogEnvironment* fog = (currSpan ? (currSpan->fFogEnvironment ? currSpan->fFogEnvironment : &fView.GetDefaultFog()) : nullptr);
    
    if (currLayer)
    {
        if ((currLayer->GetShadeFlags() & hsGMatState::kShadeReallyNoFog) && !(fMatOverOff.fShadeFlags & hsGMatState::kShadeReallyNoFog))
            fog = nil;
    }
    
    uint8_t type = fog ? fog->GetType() : plFogEnvironment::kNoFog;
    hsColorRGBA color;

    switch (type) {
        case plFogEnvironment::kLinearFog:
        {
            float start, end;
            fog->GetPipelineParams(&start, &end, &color);

            fCurrentRenderPassUniforms->fogExponential = 0;
            fCurrentRenderPassUniforms->fogValues = {start, end};
            fCurrentRenderPassUniforms->fogColor = { static_cast<half>(color.r), static_cast<half>(color.g), static_cast<half>(color.b) };
            break;
        }
        case plFogEnvironment::kExpFog:
        case plFogEnvironment::kExp2Fog:
        {
            float density;
            float power = (type == plFogEnvironment::kExp2Fog) ? 2.0f : 1.0f;
            fog->GetPipelineParams(&density, &color);

            fCurrentRenderPassUniforms->fogExponential = 1;
            fCurrentRenderPassUniforms->fogValues = { power, density};
            fCurrentRenderPassUniforms->fogColor = { static_cast<half>(color.r), static_cast<half>(color.g), static_cast<half>(color.b) };
            break;
        }
        default:
            fCurrentRenderPassUniforms->fogExponential = 0;
            fCurrentRenderPassUniforms->fogValues = { 0.0, 0.0 };
            fCurrentRenderPassUniforms->fogColor = { 0.0, 0.0, 0.0 };
            break;
    }
    
    
    if( currLayer->GetBlendFlags() & (hsGMatState::kBlendAdd | hsGMatState::kBlendMADD | hsGMatState::kBlendAddColorTimesAlpha) ) {
        fCurrentRenderPassUniforms->fogColor = { 0.0, 0.0, 0.0 };
    }
}

void plMetalPipeline::ISelectLights(const plSpan* span, plMetalMaterialShaderRef* mRef, bool proj)
{
    const size_t numLights = 8;
    size_t i = 0;
    int32_t startScale;
    float threshhold;
    float overHold = 0.3;
    float scale;
    static std::vector<plLightInfo*>   onLights;
    onLights.clear();

    if  (!IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagNoApplyProjLights) && proj) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagOnlyApplyProjLights) && !proj))
    {
        std::vector<plLightInfo*>& spanLights = span->GetLightList(proj);

        for (i = 0; i < spanLights.size() && i < numLights; i++) {
            // If these are non-projected lights, go ahead and enable them.
            if( !proj )
            {
                IEnableLight(mRef, i, spanLights[i]);
            }
            onLights.emplace_back(spanLights[i]);
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
    
    // For the projected lights, don't enable, just remember who they are.
    if( proj )
    {
        fProjAll.clear();
        fProjEach.clear();
        for( i = 0; i < onLights.size(); i++ )
        {
            if( onLights[i]->OverAll() )
                fProjAll.emplace_back(onLights[i]);
            else
                fProjEach.emplace_back(onLights[i]);
        }
        onLights.clear();
    }

    for (; i < numLights; i++) {
        IDisableLight(mRef, i);
    }
}

void plMetalPipeline::IEnableLight(plMetalMaterialShaderRef* mRef, size_t i, plLightInfo* light)
{
    hsColorRGBA amb = light->GetAmbient();
    fCurrentRenderPassUniforms->lampSources[i].ambient = { static_cast<half>(amb.r), static_cast<half>(amb.g), static_cast<half>(amb.b), static_cast<half>(amb.a) };

    hsColorRGBA diff = light->GetDiffuse();
    fCurrentRenderPassUniforms->lampSources[i].diffuse = { static_cast<half>(diff.r), static_cast<half>(diff.g), static_cast<half>(diff.b), static_cast<half>(diff.a) };

    hsColorRGBA spec = light->GetSpecular();
    fCurrentRenderPassUniforms->lampSources[i].specular = { static_cast<half>(spec.r), static_cast<half>(spec.g), static_cast<half>(spec.b), static_cast<half>(spec.a) };

    plDirectionalLightInfo* dirLight = nullptr;
    plOmniLightInfo* omniLight = nullptr;
    plSpotLightInfo* spotLight = nullptr;

    if ((dirLight = plDirectionalLightInfo::ConvertNoRef(light)) != nullptr)
    {
        hsVector3 lightDir = dirLight->GetWorldDirection();
        fCurrentRenderPassUniforms->lampSources[i].position = { lightDir.fX, lightDir.fY, lightDir.fZ, 0.0 };
        fCurrentRenderPassUniforms->lampSources[i].direction = { lightDir.fX, lightDir.fY, lightDir.fZ };

        fCurrentRenderPassUniforms->lampSources[i].constAtten = 1.0f;
        fCurrentRenderPassUniforms->lampSources[i].linAtten = 0.0f;
        fCurrentRenderPassUniforms->lampSources[i].quadAtten = 0.0f;
    }
    else if ((omniLight = plOmniLightInfo::ConvertNoRef(light)) != nullptr)
    {
        hsPoint3 pos = omniLight->GetWorldPosition();
        fCurrentRenderPassUniforms->lampSources[i].position = { pos.fX, pos.fY, pos.fZ, 1.0 };

        // TODO: Maximum Range
        
        fCurrentRenderPassUniforms->lampSources[i].constAtten = omniLight->GetConstantAttenuation();
        fCurrentRenderPassUniforms->lampSources[i].linAtten = omniLight->GetLinearAttenuation();
        fCurrentRenderPassUniforms->lampSources[i].quadAtten = omniLight->GetQuadraticAttenuation();

        if (!omniLight->GetProjection() && (spotLight = plSpotLightInfo::ConvertNoRef(omniLight)) != nullptr) {
            hsVector3 lightDir = spotLight->GetWorldDirection();
            fCurrentRenderPassUniforms->lampSources[i].direction = { lightDir.fX, lightDir.fY, lightDir.fZ };

            float falloff = spotLight->GetFalloff();
            float theta = cosf(spotLight->GetSpotInner());
            float phi = cosf(spotLight->GetProjection() ? hsConstants::half_pi<float> : spotLight->GetSpotOuter());

            fCurrentRenderPassUniforms->lampSources[i].spotProps = { falloff, theta, phi };
        } else {
            fCurrentRenderPassUniforms->lampSources[i].spotProps = { 0.0, 0.0, 0.0 };
        }
    }
    else {
        IDisableLight(mRef, i);
    }
}

void plMetalPipeline::IDisableLight(plMetalMaterialShaderRef* mRef, size_t i)
{
    fCurrentRenderPassUniforms->lampSources[i].position = { 0.0f, 0.0f, 0.0f, 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].ambient = { 0.0f, 0.0f, 0.0f, 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].diffuse = { 0.0f, 0.0f, 0.0f, 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].specular = { 0.0f, 0.0f, 0.0f, 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].constAtten = { 1.0f };
    fCurrentRenderPassUniforms->lampSources[i].linAtten = { 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].quadAtten = { 0.0f };
    fCurrentRenderPassUniforms->lampSources[i].scale = { 0.0f };
}

void plMetalPipeline::IScaleLight(plMetalMaterialShaderRef* mRef, size_t i, float scale)
{
    scale = int(scale * 1.e1f) * 1.e-1f;
    fCurrentRenderPassUniforms->lampSources[i].scale = scale;
}

void plMetalPipeline::IDrawPlate(plPlate* plate)
{
    if(!plate->IsVisible()) {
        return;
    }
    hsGMaterial* material = plate->GetMaterial();
    
    plLayerInterface* lay = material->GetLayer(0);
    hsGMatState s;
    s.Composite(lay->GetState(), fMatOverOn, fMatOverOff);

    IHandleZMode(s);
    IHandleBlendMode(s);
    fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(fDevice.fNoZReadOrWriteStencilState);
    fCurrentDepthStencilState = fDevice.fNoZReadOrWriteStencilState;
    
    //column major layout
    simd_float4x4 projMat = matrix_identity_float4x4;
    //projMat.columns[2][3] = 1.0f;
    //projMat.columns[3][1] = -0.5f;
    projMat.columns[3][2] = 0.0f;
    projMat.columns[1][1] = 1.0f;

    /// Set up the transform directly
    fDevice.SetLocalToWorldMatrix(plate->GetTransform(), false);

    IPushPiggyBacks(material);

    // First, do we have a device ref at this index?
    plMetalMaterialShaderRef* mRef = (plMetalMaterialShaderRef*)material->GetDeviceRef();

    if (mRef == nullptr) {
        mRef = new plMetalMaterialShaderRef(material, this);
        material->SetDeviceRef(mRef);
    }

    if (!mRef->IsLinked()) {
        mRef->Link(&fMatRefList);
    }
    
    fDevice.SetLocalToWorldMatrix(plate->GetTransform());
    
    plMetalPlateManager *pm = (plMetalPlateManager *)fPlateMgr;
    
    if(fCurrentPipelineState != pm->fPlateRenderPipelineState) {
        fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(pm->fPlateRenderPipelineState);
        fCurrentPipelineState = pm->fPlateRenderPipelineState;
    }
    float alpha = material->GetLayer(0)->GetOpacity();
    fDevice.CurrentRenderCommandEncoder()->setFragmentBytes(&alpha, sizeof(float), 6);
    fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(pm->fDepthState);
    fDevice.CurrentRenderCommandEncoder()->setCullMode(MTL::CullModeNone);
    
    int uniformSize = sizeof(VertexUniforms);
    VertexUniforms uniforms;
    uniforms.projectionMatrix = projMat;
    matrix_float4x4 modelMatrix;
    uniforms.worldToCameraMatrix = modelMatrix;
    uniforms.uvTransforms[0].UVWSrc = 0;
    uniforms.numUVSrcs = 1;
    //uniforms.worldToLocalMatrix = fDevice.fMatrixW2L;
    
    //flip world to camera, it's upside down
    matrix_float4x4 flip = matrix_identity_float4x4;
    flip.columns[1][1] = -1.0f;
    
    
    //uniforms.worldToCameraMatrix =
    //uniforms.cameraToWorldMatrix = fDevice.fMatrixC2W;
    uniforms.localToWorldMatrix = matrix_multiply(flip, fDevice.fMatrixL2W);
    
    mRef->FastEncodeArguments(fDevice.CurrentRenderCommandEncoder(), &uniforms, 0);
    //FIXME: Hacking the old texture drawing into the plate path
    mRef->prepareTextures(fDevice.CurrentRenderCommandEncoder(), 0);
    
    fDevice.CurrentRenderCommandEncoder()->setVertexBytes(&uniforms, sizeof(VertexUniforms), BufferIndexState);
    
    pm->encodeVertexBuffer(fDevice.CurrentRenderCommandEncoder());
    
    IPopPiggyBacks();
}

//Push and pop light sources
//The DX version would just keep a giant pool of lights
//that could be claimed by different parts of the pipeline.
//In Metal, when a part of the pipeline wants to own lights
//we'll just let them push/pop the current state.
void plMetalPipeline::PushCurrentLightSources()
{
    plMetalShaderLightSource *lightSources = new plMetalShaderLightSource[8]();
    memcpy(lightSources, fCurrentRenderPassUniforms->lampSources, sizeof(plMetalShaderLightSource[8]));
    fLightSourceStack.emplace_back(lightSources);
}

void plMetalPipeline::PopCurrentLightSources()
{
    hsAssert(fLightSourceStack.size() > 0, "Asked to pop light sources but none on stack");
    plMetalShaderLightSource *lightSources = fLightSourceStack.back();
    fLightSourceStack.pop_back();
    memcpy(fCurrentRenderPassUniforms->lampSources, lightSources, sizeof(plMetalShaderLightSource[8]));
    delete lightSources;
}

// Special effects /////////////////////////////////////////////////////////////

// IPushOverBaseLayer /////////////////////////////////////////////////////////
// Sets fOverBaseLayer (if any) as a wrapper on top of input layer.
// This allows the OverBaseLayer to intercept and modify queries of
// the real current layer's properties (e.g. color or state).
// fOverBaseLayer is set to only get applied to the base layer during
// multitexturing.
// Must be matched with call to IPopOverBaseLayer.
plLayerInterface* plMetalPipeline::IPushOverBaseLayer(plLayerInterface* li)
{
    if( !li )
        return nil;

    fOverLayerStack.emplace_back(li);

    if( !fOverBaseLayer )
        return fOverBaseLayer = li;

    fForceMatHandle = true;
    fOverBaseLayer = fOverBaseLayer->Attach(li);
    fOverBaseLayer->Eval(fTime, fFrame, 0);
    return fOverBaseLayer;
}

// IPopOverBaseLayer /////////////////////////////////////////////////////////
// Removes fOverBaseLayer as wrapper on top of input layer.
// Should match calls to IPushOverBaseLayer.
plLayerInterface* plMetalPipeline::IPopOverBaseLayer(plLayerInterface* li)
{
    if( !li )
        return nil;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverBaseLayer = fOverBaseLayer->Detach(pop);

    return pop;
}

// IPushOverAllLayer ///////////////////////////////////////////////////
// Push fOverAllLayer (if any) as wrapper around the input layer.
// fOverAllLayer is set to be applied to each layer during multitexturing.
// Must be matched by call to IPopOverAllLayer
plLayerInterface* plMetalPipeline::IPushOverAllLayer(plLayerInterface* li)
{
    if( !li )
        return nil;

    fOverLayerStack.push_back(li);

    if( !fOverAllLayer )
    {
        fOverAllLayer = li;
        fOverAllLayer->Eval(fTime, fFrame, 0);
        return fOverAllLayer;
    }

    fForceMatHandle = true;
    fOverAllLayer = fOverAllLayer->Attach(li);
    fOverAllLayer->Eval(fTime, fFrame, 0);

    return fOverAllLayer;
}

// IPopOverAllLayer //////////////////////////////////////////////////
// Remove fOverAllLayer as wrapper on top of input layer.
// Should match calls to IPushOverAllLayer.
plLayerInterface* plMetalPipeline::IPopOverAllLayer(plLayerInterface* li)
{
    if( !li )
        return nil;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverAllLayer = fOverAllLayer->Detach(pop);

    return pop;
}

// IPushProjPiggyBack //////////////////////////////////////////////////
// Push a projected texture on as a piggy back.
void plMetalPipeline::IPushProjPiggyBack(plLayerInterface* li)
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.push_back(li);
    fActivePiggyBacks = fPiggyBackStack.size() - fMatPiggyBacks;
    fForceMatHandle = true;
}

// IPopProjPiggyBacks /////////////////////////////////////////////////
// Remove a projected texture from use as a piggy back.
void plMetalPipeline::IPopProjPiggyBacks()
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.resize(fMatPiggyBacks);
    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

// IPushPiggyBacks ////////////////////////////////////////////////////
// Push any piggy backs associated with a material, presumed to
// be a light map because that's all they are used for.
// Matched with IPopPiggyBacks
void plMetalPipeline::IPushPiggyBacks(hsGMaterial* mat)
{
    hsAssert(!fMatPiggyBacks, "Push/Pop Piggy mismatch");

    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    int i;
    for( i = 0; i < mat->GetNumPiggyBacks(); i++ )
    {
        if( !mat->GetPiggyBack(i) )
            continue;

        if ((mat->GetPiggyBack(i)->GetMiscFlags() & hsGMatState::kMiscLightMap)
            && IsDebugFlagSet(plPipeDbg::kFlagNoLightmaps))
            continue;

        fPiggyBackStack.push_back(mat->GetPiggyBack(i));
        fMatPiggyBacks++;
    }
    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

// IPopPiggyBacks ///////////////////////////////////////////////////////
// Pop any current piggy backs set from IPushPiggyBacks.
// Matches IPushPiggyBacks.
void plMetalPipeline::IPopPiggyBacks()
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.resize(fPiggyBackStack.size() - fMatPiggyBacks);
    fMatPiggyBacks = 0;

    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

// PiggyBacks - used in techniques like projective lighting.
// PiggyBacks are layers appended to each drawprimitive pass.
// For example, if a material has 3 layers which will be drawn
// in 2 passes,
//      pass0: layer0+layer1
//      pass1: layer2
// Then if a piggyback layer layerPB is active, the actual rendering would be
//      pass0: layer0+layer1+layerPB
//      pass1: layer2 + layerPB

// ISetNumActivePiggyBacks /////////////////////////////////////////////
// Calculate the number of active piggy backs.
int plMetalPipeline::ISetNumActivePiggyBacks()
{
    return fActivePiggyBacks = std::min(static_cast<size_t>(fMaxPiggyBacks), fPiggyBackStack.size());
}

struct plAVTexVert {
    float fPos[2];
    float fUv[2];
};

void plMetalPipeline::IPreprocessAvatarTextures()
{
    plProfile_Set(AvRTPoolUsed, fClothingOutfits.size());
    plProfile_Set(AvRTPoolCount, fAvRTPool.size());
    plProfile_Set(AvRTPoolRes, fAvRTWidth);
    plProfile_Set(AvRTShrinkTime, uint32_t(hsTimer::GetSysSeconds() - fAvRTShrinkValidSince));

    // Frees anyone used last frame that we don't need this frame
    IClearClothingOutfits(&fPrevClothingOutfits);

    if (fClothingOutfits.size() == 0)
        return;
    
    plMipmap *itemBufferTex = nullptr;

    for (size_t oIdx = 0; oIdx < fClothingOutfits.size(); oIdx++) {
        plClothingOutfit* co = fClothingOutfits[oIdx];
        if (co->fBase == nullptr || co->fBase->fBaseTexture == nullptr)
            continue;

        plRenderTarget* rt = plRenderTarget::ConvertNoRef(co->fTargetLayer->GetTexture());
        if (rt != nullptr && co->fDirtyItems.Empty())
            // we've still got our valid RT from last frame and we have nothing to do.
            continue;

        if (rt == nullptr) {
            rt = IGetNextAvRT();
            co->fTargetLayer->SetTexture(rt);
        }

        PushRenderTarget(rt);
        fDevice.CurrentRenderCommandEncoder()->setViewport({0, 0, static_cast<double>(rt->GetWidth()), static_cast<double>(rt->GetHeight()), 0.f, 1.f});
        
        static MTL::RenderPipelineState* baseAvatarRenderState = nullptr;
        static MTL::RenderPipelineState* avatarRenderState = nullptr;
        
        if (!baseAvatarRenderState) {
            //This is a bit of a hack, this really should be part of the plMetalDevice's function map.
            //But that hash map assumes that it follows the vertex arrangement of the models.
            //After a refactor, this function creation should go there.
            MTL::RenderPipelineDescriptor* descriptor = MTL::RenderPipelineDescriptor::alloc()->init()->autorelease();
            MTL::Library* library = fDevice.fMetalDevice->newDefaultLibrary()->autorelease();
            
            MTL::Function* vertFunction = library->newFunction(NS::MakeConstantString("PreprocessAvatarVertexShader"))->autorelease();
            MTL::Function* fragFunction = library->newFunction(NS::MakeConstantString("PreprocessAvatarFragmentShader"))->autorelease();
            
            descriptor->setVertexFunction(vertFunction);
            descriptor->setFragmentFunction(fragFunction);
            
            MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::vertexDescriptor();
            vertexDescriptor->attributes()->object(0)->setFormat(MTL::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(0)->setBufferIndex(0);
            vertexDescriptor->attributes()->object(0)->setOffset(0);
            vertexDescriptor->attributes()->object(1)->setFormat(MTL::VertexFormatFloat2);
            vertexDescriptor->attributes()->object(1)->setBufferIndex(0);
            vertexDescriptor->attributes()->object(1)->setOffset(sizeof(float) * 2);
            
            vertexDescriptor->layouts()->object(0)->setStride(sizeof(float) * 4);
            
            descriptor->setVertexDescriptor(vertexDescriptor);
            
            descriptor->colorAttachments()->object(0)->setBlendingEnabled(false);
            descriptor->colorAttachments()->object(0)->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
            NS::Error* error = nullptr;
            baseAvatarRenderState = fDevice.fMetalDevice->newRenderPipelineState(descriptor, &error);
            
            descriptor->colorAttachments()->object(0)->setBlendingEnabled(true);
            descriptor->colorAttachments()->object(0)->setSourceRGBBlendFactor(MTL::BlendFactorSourceAlpha);
            descriptor->colorAttachments()->object(0)->setDestinationRGBBlendFactor(MTL::BlendFactorOneMinusSourceAlpha);
            descriptor->colorAttachments()->object(0)->setSourceAlphaBlendFactor(MTL::BlendFactorZero);
            descriptor->colorAttachments()->object(0)->setDestinationAlphaBlendFactor(MTL::BlendFactorOne);
            avatarRenderState = fDevice.fMetalDevice->newRenderPipelineState(descriptor, &error);
        }

        float uOff = 0.5f / rt->GetWidth();
        float vOff = 0.5f / rt->GetHeight();
        
        plClothingLayout *layout = plClothingMgr::GetClothingMgr()->GetLayout(co->fBase->fLayoutName);

        for (plClothingItem *item : co->fItems)
        {
            
            for (size_t j = 0; j < item->fElements.size(); j++)
            {
                for (int k = 0; k < plClothingElement::kLayerMax; k++)
                {
                    if (item->fTextures[j][k] == nullptr)
                        continue;
                    
                    itemBufferTex = item->fTextures[j][k];
                    hsColorRGBA tint = co->GetItemTint(item, k);
                    if (k >= plClothingElement::kLayerSkinBlend1 && k <= plClothingElement::kLayerSkinLast)
                        tint.a = co->fSkinBlends[k - plClothingElement::kLayerSkinBlend1];
                    
                    if (k == plClothingElement::kLayerBase)
                    {
                        if(fCurrentPipelineState != baseAvatarRenderState) {
                            fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(baseAvatarRenderState);
                            fCurrentPipelineState = baseAvatarRenderState;
                        }
                    }
                    else
                    {
                        if(fCurrentPipelineState != avatarRenderState) {
                            fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(avatarRenderState);
                            fCurrentPipelineState = avatarRenderState;
                        }
                    }
                    fDevice.CurrentRenderCommandEncoder()->setFragmentBytes(&tint, sizeof(hsColorRGBA), 0);
                    
                    float screenW = (float)item->fElements[j]->fWidth / layout->fOrigWidth * 2.f;
                    float screenH = (float)item->fElements[j]->fHeight / layout->fOrigWidth * 2.f;
                    float screenX = (float)item->fElements[j]->fXPos / layout->fOrigWidth * 2.f - 1.f;
                    float screenY = (1.f - (float)item->fElements[j]->fYPos / layout->fOrigWidth) * 2.f - 1.f - screenH;
                    IDrawClothingQuad(screenX, screenY, screenW, screenH, uOff, vOff, itemBufferTex);
                }
            }
        }

        PopRenderTarget();
        co->fDirtyItems.Clear();
    }

    fView.fXformResetFlags = fView.kResetAll;

    fClothingOutfits.swap(fPrevClothingOutfits);
}

void plMetalPipeline::IDrawClothingQuad(float x, float y, float w, float h,
                                     float uOff, float vOff, plMipmap *tex)
{
    const uint32_t kVSize = sizeof(plAVTexVert);
    plMetalTextureRef* ref = (plMetalTextureRef*)tex->GetDeviceRef();
    if (!ref || ref->IsDirty())
    {
        CheckTextureRef(tex);
        ref = (plMetalTextureRef*)tex->GetDeviceRef();
    }
    if (!ref->fTexture)
    {
        IReloadTexture(tex, ref);
    }
    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(ref->fTexture, 0);

    plAVTexVert ptr[4];
    plAVTexVert vert;
    vert.fPos[0] = x;
    vert.fPos[1] = y;
    vert.fUv[0] = uOff;
    vert.fUv[1] = 1.f + vOff;

    // P0
    ptr[2] = vert;

    // P1
    ptr[0] = vert;
    ptr[0].fPos[0] += w;
    ptr[0].fUv[0] += 1.f;

    // P2
    ptr[1] = vert;
    ptr[1].fPos[0] += w;
    ptr[1].fUv[0] += 1.f;
    ptr[1].fPos[1] += h;
    ptr[1].fUv[1] -= 1.f;

    // P3
    ptr[3] = vert;
    ptr[3].fPos[1] += h;
    ptr[3].fUv[1] -= 1.f;
    
    fDevice.CurrentRenderCommandEncoder()->setVertexBytes(ptr, sizeof(ptr), 0);
    fDevice.CurrentRenderCommandEncoder()->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangleStrip, NS::UInteger(0), NS::UInteger(4));
}

void plMetalPipeline::FindFragFunction() {
    MTL::Library *library = fDevice.fMetalDevice->newDefaultLibrary();
    
    NS::Error *error = nullptr;
    
    MTL::FunctionConstantValues *functionContents = MTL::FunctionConstantValues::alloc()->init();
    short numUVs=1;
    functionContents->setConstantValue(&numUVs, MTL::DataTypeUShort, FunctionConstantNumUVs);
    functionContents->setConstantValue(&numUVs, MTL::DataTypeUShort, FunctionConstantNumLayers);
    
    MTL::Function *fragFunction = library->newFunction(
                                                       NS::String::string("pipelineFragmentShader", NS::ASCIIStringEncoding),
                                                       functionContents,
                                                       &error
                                                    );
    fFragFunction = fragFunction;
    
    library->release();
}

/*plPipeline* plPipelineCreate::ICreateMetalPipeline(hsWindowHndl disp, hsWindowHndl hWnd, const hsG3DDeviceModeRecord* devMode)
{
    plMetalPipeline* pipe = new plMetalPipeline(disp, hWnd, devMode);
    return pipe;
}*/

// IClearShadowSlaves ///////////////////////////////////////////////////////////////////////////
// At EndRender(), we need to clear our list of shadow slaves. They are only valid for one frame.
void plMetalPipeline::IClearShadowSlaves()
{
    int i;
    for( i = 0; i < fShadows.size(); i++ )
    {
        const plShadowCaster* caster = fShadows[i]->fCaster;
        caster->GetKey()->UnRefObject();
    }
    fShadows.clear();
}

// Create all our video memory consuming D3D objects.
bool plMetalPipeline::ICreateDynDeviceObjects()
{
    // Front/Back/Depth buffers
    //if( ICreateNormalSurfaces() )
    //    return true;

    // RenderTarget pools are shared for our shadow generation algorithm.
    // Different sizes for different resolutions.
    IMakeRenderTargetPools();

    // Create device-specific stuff
    fDebugTextMgr = new plDebugTextManager();
    if( fDebugTextMgr == nil )
        return true;

    // Vertex buffers, index buffers, textures, etc.
    LoadResources();

    return false;
}

// IReleaseDynDeviceObjects //////////////////////////////////////////////
// Make sure we aren't holding on to anything, and release all of
// the D3D resources that we normally hang on to forever. Meaning things
// that persist through unloading one age and loading the next.
void plMetalPipeline::IReleaseDynDeviceObjects()
{
    // We should do this earlier, but the textFont objects don't remove
    // themselves from their parent objects yet
    delete fDebugTextMgr;
    fDebugTextMgr = nil;

    while( fRenderTargetRefList )
    {
        plMetalRenderTargetRef* rtRef = fRenderTargetRefList;
        rtRef->Release();
        rtRef->Unlink();
    }

    // The shared dynamic vertex buffers used by things like objects skinned on CPU, or
    // particle systems.
    IReleaseDynamicBuffers();
    //IReleaseAvRTPool();
    IReleaseRenderTargetPools();

}

// IReleaseDynamicBuffers /////////////////////////////////////////////////
// Release everything we've created in POOL_DEFAULT.
// This is called on shutdown or when we lose the device. Search for D3DERR_DEVICELOST.
void plMetalPipeline::IReleaseDynamicBuffers()
{
    // PlateMgr has a POOL_DEFAULT vertex buffer for drawing quads.
    if (plMetalPlateManager* pm = static_cast<plMetalPlateManager*>(fPlateMgr))
        pm->IReleaseGeometry();
}

// IReleaseRenderTargetPools //////////////////////////////////////////////////
// Free up all resources assosiated with our pools of rendertargets of varying
// sizes. Primary user of these pools is the shadow generation.
void plMetalPipeline::IReleaseRenderTargetPools()
{
    int i;

    for( i = 0; i < fRenderTargetPool512.size(); i++ )
    {
        delete fRenderTargetPool512[i];
        fRenderTargetPool512[i] = nil;
    }
    fRenderTargetPool512.clear();

    for( i = 0; i < fRenderTargetPool256.size(); i++ )
    {
        delete fRenderTargetPool256[i];
        fRenderTargetPool256[i] = nil;
    }
    fRenderTargetPool256.clear();

    for( i = 0; i < fRenderTargetPool128.size(); i++ )
    {
        delete fRenderTargetPool128[i];
        fRenderTargetPool128[i] = nil;
    }
    fRenderTargetPool128.clear();

    for( i = 0; i < fRenderTargetPool64.size(); i++ )
    {
        delete fRenderTargetPool64[i];
        fRenderTargetPool64[i] = nil;
    }
    fRenderTargetPool64.clear();

    for( i = 0; i < fRenderTargetPool32.size(); i++ )
    {
        delete fRenderTargetPool32[i];
        fRenderTargetPool32[i] = nil;
    }
    fRenderTargetPool32.clear();

    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        fRenderTargetNext[i] = 0;
        //fBlurScratchRTs[i] = nil;
        //fBlurDestRTs[i] = nil;
    }

#ifdef MF_ENABLE_HACKOFF
    hackOffscreens.Reset();
#endif // MF_ENABLE_HACKOFF
}

///////////////////////////////////////////////////////////////////////////////
//// ShadowSection
//// Shadow specific internal functions
///////////////////////////////////////////////////////////////////////////////
// See plGLight/plShadowMaster.cpp for more notes.



float blurScale = -1.f;
static  const int kL2NumSamples = 3; // Log2(4)

// IPrepShadowCaster ////////////////////////////////////////////////////////////////////////
// Make sure all the geometry in this shadow caster is ready to be rendered.
// Keep in mind the single shadow caster may be multiple spans possibly in
// multiple drawables.
// The tricky part here is that we need to prep each drawable involved,
// but only prep it once. Say the caster is composed of:
// drawableA, span0
// drawableA, span1
// drawableB, span0
// Then we need to call plDrawable::PrepForRender() ONCE on drawableA,
// and once on drawableB. Further, we need to do any necessary CPU
// skinning with ISofwareVertexBlend(drawableA, visList={0,1}) and
// ISofwareVertexBlend(drawableB, visList={1}).
bool plMetalPipeline::IPrepShadowCaster(const plShadowCaster* caster)
{
    static hsBitVector done;
    done.Clear();
    const std::vector<plShadowCastSpan>& castSpans = caster->Spans();

    int i;
    for( i = 0; i < castSpans.size(); i++ )
    {
        if( !done.IsBitSet(i) )
        {
            // We haven't already done this castSpan

            plDrawableSpans* drawable = castSpans[i].fDraw;

            // Start a visList with this index.
            static std::vector<int16_t> visList;
            visList.clear();
            visList.push_back((int16_t)(castSpans[i].fIndex));
            
            // We're about to have done this castSpan.
            done.SetBit(i);

            // Look forward through castSpans for any other spans
            // with the same drawable, and add them to visList.
            // We'll handle all the spans from this drawable at once.
            int j;
            for( j = i+1; j < castSpans.size(); j++ )
            {
                if( !done.IsBitSet(j) && (castSpans[j].fDraw == drawable) )
                {
                    // Add to list
                    visList.push_back((int16_t)(castSpans[j].fIndex));

                    // We're about to have done this castSpan.
                    done.SetBit(j);
                }
            }
            // That's all, prep the drawable.
            drawable->PrepForRender( this );

            // Do any software skinning.
            if( !ISoftwareVertexBlend(drawable, visList) )
                return false;
        }
    }

    return true;
}

// IRenderShadowCaster ////////////////////////////////////////////////
// Render the shadow caster into the slave's render target, creating a shadow map.
bool plMetalPipeline::IRenderShadowCaster(plShadowSlave* slave)
{
    const plShadowCaster* caster = slave->fCaster;

    // Setup to render into the slave's render target.
    if( !IPushShadowCastState(slave) )
        return false;

    // Get the shadow caster ready to render.
    if( !IPrepShadowCaster(slave->fCaster) )
        return false;

    // for each shadowCaster.fSpans
    int iSpan;
    for( iSpan = 0; iSpan < caster->Spans().size(); iSpan++ )
    {
        plDrawableSpans* dr = caster->Spans()[iSpan].fDraw;
        const plSpan* sp = caster->Spans()[iSpan].fSpan;
        uint32_t spIdx = caster->Spans()[iSpan].fIndex;

        hsAssert(sp->fTypeMask & plSpan::kIcicleSpan, "Shadow casting from non-trimeshes not currently supported");

        // render shadowcaster.fSpans[i] to rendertarget
        if( !(sp->fProps & plSpan::kPropNoShadowCast) )
            IRenderShadowCasterSpan(slave, dr, *(const plIcicle*)sp);

        // Keep track of which shadow slaves this span was rendered into.
        // If self-shadowing is off, we use that to determine not to
        // project the shadow map onto its source geometry.
        sp->SetShadowBit(slave->fIndex); //index set in SubmitShadowSlave
    }

    // Debug only.
    if( blurScale >= 0.f )
        slave->fBlurScale = blurScale;

    // If this shadow requests being blurred, do it.
    //TODO: Shadow blurring
    //if( slave->fBlurScale > 0.f )
        //IBlurShadowMap(slave);

    // Finished up, restore previous state.
    IPopShadowCastState(slave);

#if MCN_BOUNDS_SPANS
    if (IsDebugFlagSet(plPipeDbg::kFlagShowShadowBounds))
    {
        /// Add a span to our boundsIce to show this
        IAddBoundsSpan(fBoundsSpans, &slave->fWorldBounds);
    }
#endif // MCN_BOUNDS_SPANS

    return true;
}

// We have a (possibly empty) list of shadows submitted for this frame.
// At BeginRender, we need to accomplish:
//  Find render targets for each shadow request of the requested size.
//  Render the associated spans into the render targets. Something like the following:
void plMetalPipeline::IPreprocessShadows()
{
    plProfile_BeginTiming(PrepShadows);

    // Mark our shared resources as free to be used.
    IResetRenderTargetPools();

    // Some board (possibly the Parhelia) freaked if anistropic filtering
    // was enabled when rendering to a render target. We never need it for
    // shadow maps, and it is slower, so we just kill it here.
    //ISetAnisotropy(false);

    // Generate a shadow map for each submitted shadow slave.
    // Shadow slave corresponds to one shadow caster paired
    // with one shadow light that affects it. So a single caster
    // may be in multiple slaves (from different lights), or a
    // single light may be in different slaves (affecting different
    // casters). The overall number is low in spite of the possible
    // permutation explosion, because a slave is only generated
    // for a caster being affected (in range etc.) by a light.
    int iSlave;
    for( iSlave = 0; iSlave < fShadows.size(); iSlave++ )
    {
        plShadowSlave* slave = fShadows[iSlave];
        
        // Any trouble, remove it from the list for this frame.
        if( !IRenderShadowCaster(slave) )
        {
            fShadows.erase(fShadows.begin() + iSlave);
            iSlave--;
            continue;
        }

    }

    // Restore
    //ISetAnisotropy(true);

    plProfile_EndTiming(PrepShadows);
}

// IPushShadowCastState ////////////////////////////////////////////////////////////////////////////////
// Push all the state necessary to start rendering this shadow map, but independent of the
// actual shadow caster to be rendered into the map.
bool plMetalPipeline::IPushShadowCastState(plShadowSlave* slave)
{
    plRenderTarget* renderTarg = IFindRenderTarget(slave->fWidth, slave->fHeight, slave->fView.GetOrthogonal());
    if( !renderTarg )
        return false;

    // Let the slave setup the transforms, viewport, etc. necessary to render it's shadow
    // map. This just goes into a plViewTransform, we translate that into D3D state ourselves below.
    if (!slave->SetupViewTransform(this))
        return false;
    
    // Set texture to U_LUT
    fCurrentRenderPassUniforms->specularSrc = 0.0;

    //if( !ref->fTexture )
    //{
    //    if( ref->fData )
    //        IReloadTexture( ref );
    //}
    //fDevice.SetRenderTarget(ref->fTexture);

    // Push the shadow slave's view transform as our current render state.
    fViewStack.push(fView);
    fView.SetMaxCullNodes(0);
    SetViewTransform(slave->fView);
    IProjectionMatrixToDevice();

    // Push the shadow map as the current render target
    PushRenderTarget(renderTarg);

    // We'll be rendering the light space distance to the span fragment into
    // alpha (color is white), so our camera space position, transformed into light space
    // and then converted to [0..255] via our ULut.
    
    //METAL NOTE: D3DTSS_TCI_CAMERASPACEPOSITION and D3DTTFF_COUNT3 are hardcoded into the shader

    // Set texture transform to slave's lut transform. See plShadowMaster::IComputeLUT().
    hsMatrix44 castLUT = slave->fCastLUT;
    if( slave->fFlags & plShadowSlave::kCastInCameraSpace )
    {
        hsMatrix44 c2w = GetCameraToWorld();

        castLUT = castLUT * c2w;
    }

    simd_float4x4 tXfm;
    hsMatrix2SIMD(castLUT, &tXfm);

    fCurrentRenderPassUniforms->uvTransforms[0].transform = tXfm;
    fCurrentRenderPassUniforms->uvTransforms[0].UVWSrc = plLayerInterface::kUVWPosition;

    /*DWORD clearColor = 0xff000000L;
//  const int l2NumSamples = kL2NumSamples; // HACKSAMPLE
    const int l2NumSamples = mfCurrentTest > 101 ? 3 : 2;
    DWORD intens;
    if( slave->fBlurScale > 0 )
    {
        const int kNumSamples = mfCurrentTest > 101 ? 8 : 4;
        int nPasses = (int)ceil(float(kNumSamples) / fMaxLayersAtOnce);
        int nSamplesPerPass = kNumSamples / nPasses;
        DWORD k = int(128.f / float(nSamplesPerPass));
        intens = (0xff << 24)
            | ((128 + k) << 16)
            | ((128 + k) << 8)
            | ((128 + k) << 0);
        clearColor = (0xff << 24)
            | ((128 - k) << 16)
            | ((128 - k) << 8)
            | ((128 - k) << 0);
    }
    else
        intens = 0xffffffff;*/

    // Note that we discard the shadow caster's alpha here, although we don't
    // need to. Even on a 2 texture stage system, we could include the diffuse
    // alpha and the texture alpha from the base texture. But we don't.

    // Set color to white. We could accomplish this easier by making the color
    // in our ULut white.
    /*fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, intens);

    fSettings.fVeryAnnoyingTextureInvalidFlag = true;
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);

    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    fLayerState[0].fBlendFlags = uint32_t(-1);

    // For stage 1 - disable
    fLastEndingStage = 1;
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    fLayerState[1].fBlendFlags = uint32_t(-1);*/

    //fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    //fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    //fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

    //fD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);

    slave->fPipeData = renderTarg;

    // Enable ZBuffering w/ write
    //fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
   // fLayerState[0].fZFlags &= ~hsGMatState::kZMask;

    // Clear the render target:
    // alpha to white ensures no shadow where there's no caster
    // color to black in case we ever get blurring going
    // Z to 1
    // Stencil ignored
    if( slave->ReverseZ() )
    {
        fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(fDevice.fReverseZStencilState);
        //fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        //fD3DDevice->Clear(0, nil, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 0.0f, 0L);
    }
    else
    {
        fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(fDevice.fDefaultStencilState);
    }

    // Bring the viewport in (AFTER THE CLEAR) to protect the alpha boundary.
    fView.GetViewTransform().SetViewPort(1, 1, (float)(slave->fWidth-2), (float)(slave->fHeight-2), false);
    fDevice.SetViewport();

    //inlEnsureLightingOff();

    return true;
}

// ISetupShadowLight //////////////////////////////////////////////////////////////////
// We use the shadow light to modulate the shadow effect in two ways while
// projecting the shadow map onto the scene.
// First, the intensity of the shadow follows the N dot L of the light on
// the surface being projected onto. So on a sphere, the darkening effect
// of the shadow will fall off as the normals go from pointing to the light to
// pointing 90 degrees off.
// Second, we attenuate the whole shadow effect through the lights diffuse color.
// We attenuate for different reasons, like the intensity of the light, or
// to fade out a shadow as it gets too far in the distance to matter.
void plMetalPipeline::ISetupShadowLight(plShadowSlave* slave)
{
    //FIXME: Do we need to clear the fCurrentRenderPassUniforms->lampSources array?
    //Feels like we could catch lights from a previous pass
    plMetalShaderLightSource lRef = fCurrentRenderPassUniforms->lampSources[0];
    memset(&lRef, 0, sizeof(lRef));

    lRef.diffuse.r
        = lRef.diffuse.g
        = lRef.diffuse.b
        = slave->fPower;

    slave->fSelfShadowOn = false;

    if( slave->Positional() )
    {
        hsPoint3 position = slave->fLightPos;
        lRef.position.x = position.fX;
        lRef.position.y = position.fY;
        lRef.position.z = position.fZ;

        //const float maxRange = 32767.f;
        //lRef->fD3DInfo.Range = maxRange;
        lRef.constAtten = 1.f;
        lRef.linAtten = 0;
        lRef.quadAtten = 0;

        //lRef->fD3DInfo.Type = D3DLIGHT_POINT;
        lRef.position.w = 1.0;
    }
    else
    {
        hsVector3 dir = slave->fLightDir;
        lRef.direction.x = dir.fX;
        lRef.direction.y = dir.fY;
        lRef.direction.z = dir.fZ;
        
        lRef.position.w = 0.0;
    }

    //fD3DDevice->SetLight( lRef->fD3DIndex, &lRef->fD3DInfo );
    fCurrentRenderPassUniforms->lampSources[0] = lRef;

    //Not sure hot to link lights in Metal. Do we even need to?
    //slave->fLightIndex = lRef->fD3DIndex;
}


// IFindRenderTarget //////////////////////////////////////////////////////////////////
// Find a matching render target from the pools. We prefer the requested size, but
// will look for a smaller size if there isn't one available.
// Param ortho indicates whether it will be used for orthogonal projection as opposed
// to perspective (directional light vs. point light), but is no longer used.
plRenderTarget* plMetalPipeline::IFindRenderTarget(uint32_t& width, uint32_t& height, bool ortho)
{
    std::vector<plRenderTarget*>* pool = nil;
    uint32_t* iNext = nil;
    // NOT CURRENTLY SUPPORTING NON-SQUARE SHADOWS. IF WE DO, CHANGE THIS.
    switch(height)
    {
    case 512:
        pool = &fRenderTargetPool512;
        iNext = &fRenderTargetNext[9];
        break;
    case 256:
        pool = &fRenderTargetPool256;
        iNext = &fRenderTargetNext[8];
        break;
    case 128:
        pool = &fRenderTargetPool128;
        iNext = &fRenderTargetNext[7];
        break;
    case 64:
        pool = &fRenderTargetPool64;
        iNext = &fRenderTargetNext[6];
        break;
    case 32:
        pool = &fRenderTargetPool32;
        iNext = &fRenderTargetNext[5];
        break;
    default:
        return nil;
    }
    plRenderTarget* rt = (*pool)[*iNext];
    if( !rt )
    {
        // We didn't find one, try again the next size down.
        if( height > 32 )
            return IFindRenderTarget(width >>= 1, height >>= 1, ortho);

        // We must be totally out. Oh well.
        return nil;
    }
    (*iNext)++;

    return rt;
}

//// SharedRenderTargetRef //////////////////////////////////////////////////////
// Same as MakeRenderTargetRef, except specialized for the shadow map generation.
// The shadow map pools of a given dimension (called RenderTargetPool) all share
// a single depth buffer of that size. This allows sharing on NVidia hardware
// that wants the depth buffer dimensions to match the color buffer size.
// It may be that NVidia hardware doesn't care any more. Contact Matthias
// about that.
hsGDeviceRef* plMetalPipeline::SharedRenderTargetRef(plRenderTarget* share, plRenderTarget *owner)
{
    plMetalRenderTargetRef*    ref = nil;
    MTL::Texture*     depthSurface = nil;
    MTL::Texture*       texture = nil;
    MTL::Texture*   cTexture = nil;
    int                     i;
    plCubicRenderTarget*    cubicRT;
    uint16_t                  width, height;

    // If we don't already have one to share from, start from scratch.
    if( !share )
        return MakeRenderTargetRef(owner);

    //hsAssert(!fManagedAlloced, "Allocating non-managed resource with managed resources alloc'd");

#ifdef HS_DEBUGGING
    // Check out the validity of the match. Debug only.
    hsAssert(!owner->GetParent() == !share->GetParent(), "Mismatch on shared render target");
    hsAssert(owner->GetWidth() == share->GetWidth(), "Mismatch on shared render target");
    hsAssert(owner->GetHeight() == share->GetHeight(), "Mismatch on shared render target");
    hsAssert(owner->GetZDepth() == share->GetZDepth(), "Mismatch on shared render target");
    hsAssert(owner->GetStencilDepth() == share->GetStencilDepth(), "Mismatch on shared render target");
#endif // HS_DEBUGGING

    /// Check--is this renderTarget really a child of a cubicRenderTarget?
    if( owner->GetParent() != nil )
    {
        /// This'll create the deviceRefs for all of its children as well
        SharedRenderTargetRef(share->GetParent(), owner->GetParent());
        return owner->GetDeviceRef();
    }

    if( owner->GetDeviceRef() != nil )
        ref = (plMetalRenderTargetRef *)owner->GetDeviceRef();

    // Look for a good format of matching color and depth size.
    //FIXME: we're hardcoded for a certain tier and we aren't trying to create matching render buffers for efficiency
    //if( !IFindRenderTargetInfo(owner, surfFormat, resType) )
    //{
    //    hsAssert( false, "Error getting renderTarget info" );
    //    return nil;
    //}


    /// Create the render target now
    // Start with the depth. We're just going to share the depth surface on the
    // input shareRef.
    plMetalRenderTargetRef* shareRef = (plMetalRenderTargetRef*)share->GetDeviceRef();
    hsAssert(shareRef, "Trying to share from a render target with no ref");
    depthSurface = shareRef->fDepthBuffer;

    //FIXME: Add the usage to these textures, they're only accessed by the GPU
    // Check for Cubic. This is unlikely, since this function is currently only
    // used for the shadow map pools.
    cubicRT = plCubicRenderTarget::ConvertNoRef( owner );
    if( cubicRT != nil )
    {
        /// And create the ref (it'll know how to set all the flags)
        if( ref != nil )
            ref->SetOwner(owner);
        else {
            ref = new plMetalRenderTargetRef();
            ref->SetOwner(owner);
        }
        
        MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::textureCubeDescriptor(MTL::PixelFormatRGBA8Uint, owner->GetWidth(), false);
        MTL::Texture* cubeTexture = fDevice.fMetalDevice->newTexture(textureDescriptor);

       // hsAssert(!fManagedAlloced, "Alloc default with managed alloc'd");
        if( cubeTexture )
        {

            /// Create a CUBIC texture
            for( i = 0; i < 6; i++ )
            {
                plRenderTarget          *face = cubicRT->GetFace( i );
                plMetalRenderTargetRef *fRef;

                if( face->GetDeviceRef() != nil )
                {
                    fRef = (plMetalRenderTargetRef *)face->GetDeviceRef();
                    fRef->SetOwner(face);
                    if( !fRef->IsLinked() )
                        fRef->Link( &fRenderTargetRefList );
                }
                else
                {
                    plMetalRenderTargetRef* targetRef = new plMetalRenderTargetRef();
                    targetRef->SetOwner(face);
                    face->SetDeviceRef( targetRef );
                    ( (plMetalRenderTargetRef *)face->GetDeviceRef())->Link( &fRenderTargetRefList );
                    // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
                    hsRefCnt_SafeUnRef( face->GetDeviceRef() );
                }
            }

            ref->fTexture = cubeTexture;
        }
        else
        {
            hsRefCnt_SafeUnRef(ref);
            ref = nil;
        }
    }
    // Is it a texture render target? Probably, since shadow maps are all we use this for.
    else if( owner->GetFlags() & plRenderTarget::kIsTexture || owner->GetFlags() & plRenderTarget::kIsOffscreen)
    {
        //DX seperated the onscreen and offscreen types. Metal doesn't care. All render targets are textures.
        /// Create a normal texture
        if( ref != nil )
            ref->SetOwner(owner);
        else {
            ref = new plMetalRenderTargetRef();
            ref->SetOwner(owner);
        }
        
        MTL::TextureDescriptor* textureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatBGRA8Unorm, owner->GetWidth(), owner->GetHeight(), false);
        textureDescriptor->setUsage(MTL::TextureUsageRenderTarget | MTL::TextureUsageShaderRead);
        MTL::Texture* texture = fDevice.fMetalDevice->newTexture(textureDescriptor);
        if( texture )
        {
            ref->fTexture = texture;
        }
        else
        {
            hsRefCnt_SafeUnRef(ref);
            ref = nil;
        }
        
        if (owner->GetZDepth() && (owner->GetFlags() & (plRenderTarget::kIsTexture | plRenderTarget::kIsOffscreen))) {
            MTL::TextureDescriptor *depthTextureDescriptor = MTL::TextureDescriptor::texture2DDescriptor(MTL::PixelFormatDepth32Float_Stencil8,
                                                                                                         owner->GetWidth(),
                                                                                                         owner->GetHeight(),
                                                                                                         false);
                                                                                                         
            if (fDevice.fMetalDevice->supportsFamily(MTL::GPUFamilyApple1)) {
                depthTextureDescriptor->setStorageMode(MTL::StorageModeMemoryless);
            }   else {
                depthTextureDescriptor->setStorageMode(MTL::StorageModePrivate);
            }
            depthTextureDescriptor->setUsage(MTL::TextureUsageRenderTarget);
            MTL::Texture *depthBuffer = fDevice.fMetalDevice->newTexture(depthTextureDescriptor);
            ref->fDepthBuffer = depthBuffer;
        }
    }

    if( owner->GetDeviceRef() != ref )
    {
        owner->SetDeviceRef( ref );
        // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
        hsRefCnt_SafeUnRef( ref );
        if( ref != nil && !ref->IsLinked() )
            ref->Link( &fRenderTargetRefList );
    }
    else
    {
        if( ref != nil && !ref->IsLinked() )
            ref->Link( &fRenderTargetRefList );
    }

    if( ref != nil )
    {
        ref->SetDirty( false );
    }

    return ref;
}

// IMakeRenderTargetPools /////////////////////////////////////////////////////////////
// These are actually only used as shadow map pools, but they could be used for other
// render targets.
// All these are created here in a single call because they go in POOL_DEFAULT, so they
// must be created before we start creating things in POOL_MANAGED.
void plMetalPipeline::IMakeRenderTargetPools()
{
    //FIXME: We should probably have a release function for the render target pools
    //IReleaseRenderTargetPools(); // Just to be sure.

    // Numbers of render targets to be created for each size.
    // These numbers were set with multi-player in mind, so should be reconsidered.
    // But do keep in mind that there are many things in production assets that cast
    // shadows besides the avatar.
    plConst(float)   kCount[kMaxRenderTargetNext] = {
        0, // 1x1
        0, // 2x2
        0, // 4x4
        0, // 8x8
        0, // 16x16
        32, // 32x32
        16, // 64x64
        8, // 128x128
        4, // 256x256
        0 // 512x512
    };
    int i;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        std::vector<plRenderTarget*>* pool = nil;
        switch( i )
        {
        default:
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
            break;

        case 5:
            pool = &fRenderTargetPool32;
            break;
        case 6:
            pool = &fRenderTargetPool64;
            break;
        case 7:
            pool = &fRenderTargetPool128;
            break;
        case 8:
            pool = &fRenderTargetPool256;
            break;
        case 9:
            pool = &fRenderTargetPool512;
            break;
        }
        if( pool )
        {
            pool->resize(kCount[i] + 1);
            (*pool)[0] = nil;
            (*pool)[(int)(kCount[i])] = nil;
            int j;
            for( j = 0; j < kCount[i]; j++ )
            {
                uint16_t flags = plRenderTarget::kIsTexture | plRenderTarget::kIsProjected;
                uint8_t bitDepth = 32;
                uint8_t zDepth = 24;
                uint8_t stencilDepth = 0;
                
                // If we ever allow non-square shadows, change this.
                int width = 1 << i;
                int height = width;

                plRenderTarget* rt = new plRenderTarget(flags, width, height, bitDepth, zDepth, stencilDepth);

                // If we've failed to create our render target ref, we're probably out of
                // video memory. We'll return nil, and this guy just doesn't get a shadow
                // until more video memory turns up (not likely).
                if( !SharedRenderTargetRef((*pool)[0], rt) )
                {
                    delete rt;
                    pool->resize(j+1);
                    (*pool)[j] = nil;
                    break;
                }
                (*pool)[j] = rt;
            }
        }
    }
}

// IPopShadowCastState ///////////////////////////////////////////////////
// Pop the state set to render this shadow caster, so we're ready to render
// a different shadow caster, or go on to our main render.
bool plMetalPipeline::IPopShadowCastState(plShadowSlave* slave)
{
    fView = fViewStack.top();
    fViewStack.pop();

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;
    
    fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(nullptr, 16);

    return true;
}

// IResetRenderTargetPools /////////////////////////////////////////////////////////////////
// No release of resources, this just resets for the start of a frame. So if a shadow
// slave gets a render target from a pool, once this is called (conceptually at the
// end of the frame), the slave no longer owns that render target.
void plMetalPipeline::IResetRenderTargetPools()
{
    int i;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        fRenderTargetNext[i] = 0;
        //fBlurScratchRTs[i] = nil;
        //fBlurDestRTs[i] = nil;
    }

    //fLights.fNextShadowLight = 0;
}

// IRenderShadowCasterSpan //////////////////////////////////////////////////////////////////////
// Render the span into a rendertarget of the correct size, generating
// a depth map from this light to that span.
void plMetalPipeline::IRenderShadowCasterSpan(plShadowSlave* slave, plDrawableSpans* drawable, const plIcicle& span)
{
    // Check that it's ready to render.
    plProfile_BeginTiming(CheckDyn);
    ICheckDynBuffers(drawable, drawable->GetBufferGroup(span.fGroupIdx), &span);
    plProfile_EndTiming(CheckDyn);

    plMetalVertexBufferRef*    vRef = (plMetalVertexBufferRef *)drawable->GetVertexRef(span.fGroupIdx, span.fVBufferIdx);
    plMetalIndexBufferRef* iRef = (plMetalIndexBufferRef *)drawable->GetIndexRef(span.fGroupIdx, span.fIBufferIdx);

    if( vRef->GetBuffer() == nil || iRef->GetBuffer() == nil )
    {
        hsAssert( false, "Trying to render a nil buffer pair!" );
        return;
    }

    /// Switch to the vertex buffer we want
    plMetalDevice::plMetalLinkedPipeline* linkedPipeline = plMetalRenderShadowCasterPipelineState(&fDevice, vRef).GetRenderPipelineState();
    if(fCurrentPipelineState != linkedPipeline->pipelineState) {
        fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
        fCurrentPipelineState = linkedPipeline->pipelineState;
    }
    
    if (fCurrentVertexBuffer != vRef->GetBuffer()) {
        fDevice.CurrentRenderCommandEncoder()->setVertexBuffer(vRef->GetBuffer(), 0, 0);
        fCurrentVertexBuffer = vRef->GetBuffer();
    }
    
    fCurrentVertexBuffer = vRef->GetBuffer();
    fDevice.fCurrentIndexBuffer = iRef->GetBuffer();
    fDevice.CurrentRenderCommandEncoder()->setCullMode(MTL::CullModeNone);

    uint32_t                  vStart = span.fVStartIdx;
    uint32_t                  vLength = span.fVLength;
    uint32_t                  iStart = span.fIPackedIdx;
    uint32_t                  iLength= span.fILength;

    plRenderTriListFunc render(&fDevice, 0, vStart, vLength, iStart, iLength);

    static hsMatrix44 emptyMatrix;
    hsMatrix44 m = emptyMatrix;

    ISetupTransforms(drawable, span, m);

    bool flip = slave->ReverseCull();
    ISetCullMode(flip);

    render.RenderPrims();
}


// IRenderShadowsOntoSpan /////////////////////////////////////////////////////////////////////
// After doing the usual render for a span (all passes), we call the following.
// If the span accepts shadows, this will loop over all the shadows active this
// frame, and apply the ones that intersect this spans bounds. See below for details.
void plMetalPipeline::IRenderShadowsOntoSpan(const plRenderPrimFunc& render, const plSpan* span, hsGMaterial* mat, plMetalVertexBufferRef *vRef)
{
    // We've already computed which shadows affect this span. That's recorded in slaveBits.
    const hsBitVector& slaveBits = span->GetShadowSlaves();

    bool first = true;

    for(size_t i = 0; i < fShadows.size(); i++ )
    {
        if( slaveBits.IsBitSet(fShadows[i]->fIndex) )
        {
            // This slave affects this span.
            if( first )
            {
                
                // On the first, we do all the setup that is independent of
                // the shadow slave, so state that needs to get set once before
                // projecting any number of shadow maps.
                ISetupShadowRcvTextureStages(mat);

                first = false;

            }

            // Now setup any state specific to this shadow slave.
            ISetupShadowSlaveTextures(fShadows[i]);
            
            // See ISetupShadowLight below for how the shadow light is used.
            // The shadow light isn't used in generating the shadow map, it's used
            // in projecting the shadow map onto the scene.
            ISetupShadowLight(fShadows[i]);
            
            struct plMetalMaterialPassDescription passDescription;
            memset(&passDescription, 0, sizeof(passDescription));
            passDescription.Populate(mat->GetLayer(0), 2);
            passDescription.numLayers = 3;
            if (mat->GetNumLayers()>1) {
                passDescription.Populate(mat->GetLayer(1), 2);
            }
            
            plMetalDevice::plMetalLinkedPipeline *linkedPipeline = plMetalRenderShadowPipelineState(&fDevice, vRef, passDescription).GetRenderPipelineState();
            if(fCurrentPipelineState != linkedPipeline->pipelineState) {
                fDevice.CurrentRenderCommandEncoder()->setRenderPipelineState(linkedPipeline->pipelineState);
                fCurrentPipelineState = linkedPipeline->pipelineState;
            }

            int selfShadowNow = span->IsShadowBitSet(fShadows[i]->fIndex);

            // We vary the shadow intensity when self shadowing (see below),
            // so we cache whether the shadow light is set for regular or
            // self shadowing intensity. If what we're doing now is different
            // than what we're currently set for, set it again.
            //if( selfShadowNow != fShadows[i]->fSelfShadowOn )
            //{
                plMetalShaderLightSource lRef = fCurrentRenderPassUniforms->lampSources[0];

                // We lower the power on self shadowing, because the artists like to
                // crank up the shadow strength to huge values to get a darker shadow
                // on the environment, which causes the shadow on the avatar to get
                // way too dark. Another way to look at it is when self shadowing,
                // the surface being projected onto is going to be very close to
                // the surface casting the shadow (because they are the same object).
                if( selfShadowNow )
                {
                    plConst(float) kMaxSelfPower = 0.3f;
                    float power = (float) fShadows[i]->fPower > kMaxSelfPower ? (float) kMaxSelfPower : ((float) fShadows[i]->fPower);
                    lRef.diffuse.r = lRef.diffuse.b = lRef.diffuse.g = power;
                }
                else
                {
                    lRef.diffuse.r = lRef.diffuse.b = lRef.diffuse.g = fShadows[i]->fPower;
                }
                lRef.scale = 1.0;
                fCurrentRenderPassUniforms->lampSources[0] = lRef;

                // record which our intensity is now set for.
                fShadows[i]->fSelfShadowOn = selfShadowNow;
            //}

            // Enable the light.
            //fD3DDevice->LightEnable(fShadows[i]->fLightIndex, true);*/

#ifndef PLASMA_EXTERNAL_RELEASE
            if (!IsDebugFlagSet(plPipeDbg::kFlagNoShadowApply))
#endif // PLASMA_EXTERNAL_RELEASE
                render.RenderPrims();
            
            // Disable it again.
            //fD3DDevice->LightEnable(fShadows[i]->fLightIndex, false);

        }
    }

}


// ISetupShadowRcvTextureStages ////////////////////////////////////////////
// Set the generic stage states. We'll fill in the specific textures
// for each slave later.
void plMetalPipeline::ISetupShadowRcvTextureStages(hsGMaterial* mat)
{
    //Do this first, this normally stomps all over our uniforms
    //FIXME: Way to encode layers without stomping all over uniforms?
    plMetalMaterialShaderRef* matShader = (plMetalMaterialShaderRef *)mat->GetDeviceRef();
    //matShader->encodeArguments(fDevice.CurrentRenderCommandEncoder(), fCurrentRenderPassUniforms, 0, 0, nullptr);
    
    // We're whacking about with renderstate independent of current material,
    // so make sure the next span processes it's material, even if it's the
    // same one.
    fForceMatHandle = true;

    // Set the D3D lighting/material model
    ISetShadowLightState(mat);

    // Zbuffering on read-only
    
    
    if(fCurrentDepthStencilState != fDevice.fNoZWriteStencilState) {
        fDevice.CurrentRenderCommandEncoder()->setDepthStencilState(fDevice.fNoZWriteStencilState);
        fCurrentDepthStencilState = fDevice.fNoZWriteStencilState;
    }
    
    int numUVSrcs = 2;
    
    int layerIndex = -1;
    // If mat's base layer is alpha'd, and we have > 3 TMU's factor
    // in the base layer's alpha.
    if( (fMaxLayersAtOnce > 3) && mat->GetLayer(0)->GetTexture() && (mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha) )
    {
        plLayerInterface* layer = mat->GetLayer(0);
        layerIndex = 0;
        
        

        // If the following conditions are met, it means that layer 1 is a better choice to
        // get the transparency from. The specific case we're looking for is vertex alpha
        // simulated by an invisible second layer alpha LUT (known as the alpha hack).
        if( (layer->GetMiscFlags() & hsGMatState::kMiscBindNext)
            && mat->GetLayer(1)
            && !(mat->GetLayer(1)->GetMiscFlags() & hsGMatState::kMiscNoShadowAlpha)
            && !(mat->GetLayer(1)->GetBlendFlags() & hsGMatState::kBlendNoTexAlpha)
           && mat->GetLayer(1)->GetTexture() ) {
                layer = mat->GetLayer(1);
            layerIndex = 1;
        }
        
        // Normal UVW source.
        uint32_t uvwSrc = layer->GetUVWSrc();
        
            // Normal UVW source.
        fCurrentRenderPassUniforms->uvTransforms[2].UVWSrc = uvwSrc;
        // MiscFlags to layer's misc flags
        matrix_float4x4 tXfm;
        hsMatrix2SIMD(layer->GetTransform(), &tXfm);
        fCurrentRenderPassUniforms->uvTransforms[2].transform = tXfm;
        
        numUVSrcs++;
    }
    
    fDevice.CurrentRenderCommandEncoder()->setFragmentBytes(&layerIndex, sizeof(int), FragmentShaderArgumentShadowAlphaSrc);
    
    fCurrentRenderPassUniforms->numUVSrcs = numUVSrcs;
}

// ISetShadowLightState //////////////////////////////////////////////////////////////////
// Set the D3D lighting/material model for projecting the shadow map onto this material.
void plMetalPipeline::ISetShadowLightState(hsGMaterial* mat)
{
    IDisableLightsForShadow();
    //inlEnsureLightingOn();

    fCurrLightingMethod = plSpan::kLiteShadow;

    if( mat && mat->GetNumLayers() && mat->GetLayer(0) )
        fCurrentRenderPassUniforms->diffuseCol.r = fCurrentRenderPassUniforms->diffuseCol.g = fCurrentRenderPassUniforms->diffuseCol.b = mat->GetLayer(0)->GetOpacity();
    else
        fCurrentRenderPassUniforms->diffuseCol.r = fCurrentRenderPassUniforms->diffuseCol.g = fCurrentRenderPassUniforms->diffuseCol.b = 1.f;
    fCurrentRenderPassUniforms->diffuseCol.a = 1.f;
    
    fCurrentRenderPassUniforms->diffuseSrc = 1.0;
    fCurrentRenderPassUniforms->emissiveSrc = 1.0;
    fCurrentRenderPassUniforms->emissiveCol = 0.0;
    fCurrentRenderPassUniforms->specularSrc = 0.0;
    fCurrentRenderPassUniforms->ambientSrc = 0.0;
    fCurrentRenderPassUniforms->globalAmb = 0.0;

    //fD3DDevice->SetMaterial(&d3dMat);
    //fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0 );*/
}

// IDisableLightsForShadow ///////////////////////////////////////////////////////////
// Disable any lights that are enabled. We'll only want the shadow light illuminating
// the surface.
void plMetalPipeline::IDisableLightsForShadow()
{
    int i;
    for( i = 0; i < 8; i++ )
    {
        IDisableLight(nullptr, i);
    }
}

// ISetupShadowSlaveTextures //////////////////////////////////////////////
// Set any state specific to this shadow slave for projecting the slave's
// shadow map onto the surface.
void plMetalPipeline::ISetupShadowSlaveTextures(plShadowSlave* slave)
{
    //D3DMATRIX tXfm;

    hsMatrix44 c2w = GetCameraToWorld();

    // Stage 0:
    // Set Stage 0's texture to the slave's rendertarget.
    // Set texture transform to slave's camera to texture transform
    plRenderTarget* renderTarg = (plRenderTarget*)slave->fPipeData;
    hsAssert(renderTarg, "Processing a slave that hasn't been rendered");
    if( !renderTarg )
        return;
    plMetalTextureRef* ref = (plMetalTextureRef*)renderTarg->GetDeviceRef();
    hsAssert(ref, "Shadow map ref should have been made when it was rendered");
    if( !ref )
        return;

    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fDevice.CurrentRenderCommandEncoder()->setFragmentTexture(ref->fTexture, 16);

    plMetalShadowCastFragmentShaderArgumentBuffer uniforms;
    uniforms.pointLightCast = slave->fView.GetOrthogonal() ? false : true;
    fDevice.CurrentRenderCommandEncoder()->setFragmentBytes(&uniforms, sizeof(plMetalShadowCastFragmentShaderArgumentBuffer), BufferIndexShadowCastFragArgBuffer);
    
    hsMatrix44 cameraToTexture = slave->fWorldToTexture * c2w;
    simd_float4x4 tXfm;
    hsMatrix2SIMD(cameraToTexture, &tXfm);
    
    fCurrentRenderPassUniforms->uvTransforms[0].UVWSrc = plLayerInterface::kUVWPosition;
    fCurrentRenderPassUniforms->uvTransforms[0].transform = tXfm;

    // Stage 1: the lut
    // Set the texture transform to slave's fRcvLUT
    hsMatrix44 cameraToLut = slave->fRcvLUT * c2w;
    hsMatrix2SIMD(cameraToLut, &tXfm);
    
    fCurrentRenderPassUniforms->uvTransforms[1].UVWSrc = plLayerInterface::kUVWPosition;
    fCurrentRenderPassUniforms->uvTransforms[1].transform = tXfm;

}

///////////////////////////////////////////////////////////////////////////////
//// View Stuff ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IIsViewLeftHanded ////////////////////////////////////////////////////////
//  Returns true if the combination of the local2world and world2camera
//  matrices is left-handed.

bool plMetalPipeline::IIsViewLeftHanded()
{
    return fView.GetViewTransform().GetOrthogonal() ^ ( fView.fLocalToWorldLeftHanded ^ fView.fWorldToCamLeftHanded ) ? true : false;
}

//// ISetCullMode /////////////////////////////////////////////////////////////
//  Tests and sets the current winding order cull mode (CW, CCW, or none).
// Will reverse the cull mode as necessary for left handed camera or local to world
// transforms.
void plMetalPipeline::ISetCullMode(bool flip)
{
    MTL::CullMode newCullMode = !IIsViewLeftHanded() ^ !flip ? MTL::CullModeFront : MTL::CullModeBack;
    fDevice.CurrentRenderCommandEncoder()->setCullMode(newCullMode);
    fCurrentCullMode = newCullMode;
}

plMetalDevice* plMetalPipeline::GetMetalDevice()
{
    return &fDevice;
}

//// Local Static Stuff ///////////////////////////////////////////////////////

//FIXME: CPU avatar stuff that should be evaluated once this moves onto the GPU.

template<typename T>
static inline void inlCopy(uint8_t*& src, uint8_t*& dst)
{
    T* src_ptr = reinterpret_cast<T*>(src);
    T* dst_ptr = reinterpret_cast<T*>(dst);
    *dst_ptr = *src_ptr;
    src += sizeof(T);
    dst += sizeof(T);
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

//// ISoftwareVertexBlend ///////////////////////////////////////////////////////
// Emulate matrix palette operations in software. The big difference between the hardware
// and software versions is we only want to lock the vertex buffer once and blend all the
// verts we're going to in software, so the vertex blend happens once for an entire drawable.
// In hardware, we want the opposite, to break it into managable chunks, manageable meaning
// few enough matrices to fit into hardware registers. So for hardware version, we set up
// our palette, draw a span or few, setup our matrix palette with new matrices, draw, repeat.
bool plMetalPipeline::ISoftwareVertexBlend(plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoSkinning))
        return true;

    if (drawable->GetSkinTime() == fRenderCnt)
        return true;

    const hsBitVector& blendBits = drawable->GetBlendingSpanVector();

    if (drawable->GetBlendingSpanVector().Empty()) {
        // This sucker doesn't have any skinning spans anyway. Just return
        drawable->SetSkinTime(fRenderCnt);
        return true;
    }

    plProfile_BeginTiming(Skin);

    // lock the data buffer

    // First, figure out which buffers we need to blend.
    const int kMaxBufferGroups = 20;
    const int kMaxVertexBuffers = 20;
    static char blendBuffers[kMaxBufferGroups][kMaxVertexBuffers];
    memset(blendBuffers, 0, kMaxBufferGroups * kMaxVertexBuffers * sizeof(**blendBuffers));

    hsAssert(kMaxBufferGroups >= drawable->GetNumBufferGroups(), "Bigger than we counted on num groups skin.");

    const std::vector<plSpan*>& spans = drawable->GetSpanArray();
    int i;
    for (i = 0; i < visList.size(); i++) {
        if (blendBits.IsBitSet(visList[i])) {
            const plVertexSpan &vSpan = *(plVertexSpan *)spans[visList[i]];
            hsAssert(kMaxVertexBuffers > vSpan.fVBufferIdx, "Bigger than we counted on num buffers skin.");

            blendBuffers[vSpan.fGroupIdx][vSpan.fVBufferIdx] = 1;
            drawable->SetBlendingSpanVectorBit(visList[i], false);
        }
    }

    // Now go through each of the group/buffer (= a real vertex buffer) pairs we found,
    // and blend into it. We'll lock the buffer once, and then for each span that
    // uses it, set the matrix palette and and then do the blend for that span.
    // When we've done all the spans for a group/buffer, we unlock it and move on.
    int j;
    for( i = 0; i < kMaxBufferGroups; i++ )
    {
        for( j = 0; j < kMaxVertexBuffers; j++ )
        {
            if( blendBuffers[i][j] )
            {
                // Found one. Do the lock.
                plMetalVertexBufferRef* vRef = (plMetalVertexBufferRef*)drawable->GetVertexRef(i, j);

                hsAssert(vRef->fData, "Going into skinning with no place to put results!");

                uint8_t* destPtr = vRef->fData;

                int k;
                for (k = 0; k < visList.size(); k++) {
                    const plIcicle& span = *(plIcicle*)spans[visList[k]];
                    if (span.fGroupIdx == i && span.fVBufferIdx == j) {
                        plProfile_Inc(NumSkin);

                        hsMatrix44* matrixPalette = drawable->GetMatrixPalette(span.fBaseMatrix);
                        matrixPalette[0] = span.fLocalToWorld;

                        uint8_t* ptr = vRef->fOwner->GetVertBufferData(vRef->fIndex);
                        ptr += span.fVStartIdx * vRef->fOwner->GetVertexSize();
                        IBlendVertBuffer( (plSpan*)&span,
                                                matrixPalette, span.fNumMatrices,
                                                ptr,
                                                vRef->fOwner->GetVertexFormat(),
                                                vRef->fOwner->GetVertexSize(),
                                                destPtr + span.fVStartIdx * vRef->fVertexSize,
                                                vRef->fVertexSize,
                                                span.fVLength,
                                                span.fLocalUVWChans );
                        vRef->SetDirty(true);
                    }
                }
                // Unlock and move on.
            }
        }
    }

    plProfile_EndTiming(Skin);

    if (drawable->GetBlendingSpanVector().Empty()) {
        // Only do this if we've blended ALL of the spans. Thus, this becomes a trivial
        // rejection for all the skinning flags being cleared
        drawable->SetSkinTime(fRenderCnt);
    }

    return true;
}


//// IBlendVertsIntoBuffer ////////////////////////////////////////////////////
//  Given a pointer into a buffer of verts that have blending data in the D3D
//  format, blends them into the destination buffer given without the blending
//  info.

void plMetalPipeline::IBlendVertBuffer(plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                             const uint8_t* src, uint8_t format, uint32_t srcStride,
                             uint8_t* dest, uint32_t destStride, uint32_t count,
                             uint16_t localUVWChans)
{
    float pt_buf[] = { 0.f, 0.f, 0.f, 1.f };
     float vec_buf[] = { 0.f, 0.f, 0.f, 0.f };
    hsPoint3*       pt = reinterpret_cast<hsPoint3*>(pt_buf);
    hsVector3*      vec = reinterpret_cast<hsVector3*>(vec_buf);

    uint32_t        indices;
    float           weights[4];

    // Dropped support for localUVWChans at templatization of code
    hsAssert(localUVWChans == 0, "support for skinned UVWs dropped. reimplement me?");
    const size_t uvChanSize = plGBufferGroup::CalcNumUVs(format) * sizeof(float) * 3;
    uint8_t numWeights = (format & plGBufferGroup::kSkinWeightMask) >> 4;

    for (uint32_t i = 0; i < count; ++i) {
        // Extract data
        src = inlExtract<hsPoint3>(src, pt);

        float weightSum = 0.f;
        for (uint8_t j = 0; j < numWeights; ++j) {
            src = inlExtract<float>(src, &weights[j]);
            weightSum += weights[j];
        }
        weights[numWeights] = 1.f - weightSum;

        if (format & plGBufferGroup::kSkinIndices)
            src = inlExtract<uint32_t>(src, &indices);
        else
            indices = 1 << 8;
        src = inlExtract<hsVector3>(src, vec);

        // Destination buffers (float4 for SSE alignment)
        simd_float4 destNorm_buf = (simd_float4){ 0.f, 0.f, 0.f, 0.f };
        simd_float4 destPt_buf = (simd_float4){ 0.f, 0.f, 0.f, 1.f };

        simd_float4x4 simdMatrix;
        
        // Blend
        for (uint32_t j = 0; j < numWeights + 1; ++j) {
            hsMatrix2SIMD(matrixPalette[indices & 0xFF], &simdMatrix);
            if (weights[j]) {
                //Note: This bit is different than GL/DirectX. It's using acclerate so this is also accelerated on ARM through NEON or maybe even the Neural Engine.
                destPt_buf += weights[j] * simd_mul(simdMatrix, *(simd_float4 *)pt_buf);
                destNorm_buf += weights[j] * simd_mul(simdMatrix, *(simd_float4 *)vec_buf);
            }
                //ISkinVertexSSE41(matrixPalette[indices & 0xFF], weights[j], pt_buf, destPt_buf, vec_buf, destNorm_buf);
            indices >>= 8;
        }
        // Probably don't really need to renormalize this. There errors are
        // going to be subtle and "smooth".
        /* hsFastMath::NormalizeAppr(destNorm); */

        // Slam data into position now
        dest = inlStuff<hsPoint3>(dest, reinterpret_cast<hsPoint3*>(&destPt_buf));
        dest = inlStuff<hsVector3>(dest, reinterpret_cast<hsVector3*>(&destNorm_buf));

        // Jump past colors and UVws
        dest += sizeof(uint32_t) * 2 + uvChanSize;
        src  += sizeof(uint32_t) * 2 + uvChanSize;
    }
}

//Resource checking

// CheckTextureRef //////////////////////////////////////////////////////
// Make sure the given layer's texture has background D3D resources allocated.
void plMetalPipeline::CheckTextureRef(plLayerInterface* layer)
{
    plBitmap* bitmap = layer->GetTexture();

    if (bitmap) {
        CheckTextureRef(bitmap);
    }
}

void plMetalPipeline::CheckTextureRef(plBitmap* bitmap)
{
    plMetalTextureRef* tRef = static_cast<plMetalTextureRef*>(bitmap->GetDeviceRef());
    
    if (!tRef) {
        tRef = static_cast<plMetalTextureRef*>(MakeTextureRef(bitmap));
    }
    
    // If it's dirty, refill it.
    if (tRef->IsDirty()) {
        IReloadTexture(bitmap, tRef);
    }
}

hsGDeviceRef    *plMetalPipeline::MakeTextureRef(plBitmap* bitmap)
{
    plMetalTextureRef* tRef = static_cast<plMetalTextureRef*>(bitmap->GetDeviceRef());

    if (!tRef) {
        tRef = new plMetalTextureRef();

        fDevice.SetupTextureRef(bitmap, tRef);
    }

    if (!tRef->IsLinked()) {
        tRef->Link(&fTextureRefList);
    }

    // Make sure it has all resources created.
    fDevice.CheckTexture(tRef);

    // If it's dirty, refill it.
    if (tRef->IsDirty()) {
        IReloadTexture( bitmap, tRef );
    }
    return tRef;
}

void    plMetalPipeline::IReloadTexture( plBitmap* bitmap, plMetalTextureRef *ref )
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

// CheckVertexBufferRef /////////////////////////////////////////////////////
// Make sure the buffer group has a valid buffer ref and that it is up to date.
void plMetalPipeline::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    // First, do we have a device ref at this index?
    plMetalVertexBufferRef* vRef = static_cast<plMetalVertexBufferRef*>(owner->GetVertexBufferRef(idx));

    // If not
    if (!vRef) {
        // Make the blank ref
        vRef = new plMetalVertexBufferRef();

        fDevice.SetupVertexBufferRef(owner, idx, vRef);
    }

    if (!vRef->IsLinked()) {
        vRef->Link(&fVtxBuffRefList);
    }

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
    }
    else
    {
        // Make sure we're going to be ready to fill it.
        if (!vRef->fData && (vRef->fFormat != owner->GetVertexFormat()))
        {
            vRef->fData = new uint8_t[vRef->fCount * vRef->fVertexSize];
            fDevice.FillVolatileVertexBufferRef(vRef, owner, idx);
        }
    }
}

// ISetupVertexBufferRef /////////////////////////////////////////////////////////
// Initialize input vertex buffer ref according to source.
void plMetalPipeline::ISetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, plMetalVertexBufferRef* vRef)
{

    uint8_t format = owner->GetVertexFormat();

    // All indexed skinning is currently done on CPU, so the source data
    // will have indices, but we strip them out for the D3D buffer.
    if( format & plGBufferGroup::kSkinIndices )
    {
        format &= ~(plGBufferGroup::kSkinWeightMask | plGBufferGroup::kSkinIndices);
        format |= plGBufferGroup::kSkinNoWeights;       // Should do nothing, but just in case...
        vRef->SetSkinned(true);
        vRef->SetVolatile(true);
    }

    uint32_t vertSize = IGetBufferFormatSize(format); // vertex stride
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

    owner->SetVertexBufferRef(idx, vRef);
    hsRefCnt_SafeUnRef(vRef);
}

// CheckIndexBufferRef /////////////////////////////////////////////////////
// Make sure the buffer group has an index buffer ref and that its data is current.
void plMetalPipeline::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    plMetalIndexBufferRef* iRef = static_cast<plMetalIndexBufferRef*>(owner->GetIndexBufferRef(idx));

    if (!iRef) {
        // Create one from scratch.
        iRef = new plMetalIndexBufferRef();

        fDevice.SetupIndexBufferRef(owner, idx, iRef);
    }

    if (!iRef->IsLinked()) {
        iRef->Link(&fIdxBuffRefList);
    }

    // Make sure it has all resources created.
    fDevice.CheckIndexBuffer(iRef);

    // If it's dirty, refill it.
    if (iRef->IsDirty()) {
        fDevice.FillIndexBufferRef(iRef, owner, idx);
    }
}

//// IGetBufferFormatSize /////////////////////////////////////////////////////
// Calculate the vertex stride from the given format.
uint32_t  plMetalPipeline::IGetBufferFormatSize( uint8_t format ) const
{
    uint32_t  size = sizeof( float ) * 6 + sizeof( uint32_t ) * 2; // Position and normal, and two packed colors

    
    switch( format & plGBufferGroup::kSkinWeightMask )
    {
        case plGBufferGroup::kSkinNoWeights:
            break;
        case plGBufferGroup::kSkin1Weight:
            size += sizeof(float);
            break;
        default:
            hsAssert( false, "Invalid skin weight value in IGetBufferFormatSize()" );
    }

    size += sizeof( float ) * 3 * plGBufferGroup::CalcNumUVs( format );

    return size;
}

void plMetalPipeline::ResetMetalStateTracking()
{
    fCurrentPipelineState = nullptr;
    fCurrentDepthStencilState = nullptr;
    fCurrentVertexBuffer = nullptr;
}
