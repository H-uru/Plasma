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
//  plDXPipeline Class Functions                                             //
//  plPipeline derivative for DirectX                                        //
//  Cyan, Inc.                                                               //
//                                                                           //
//// Version History //////////////////////////////////////////////////////////
//                                                                           //
//  2.23.2001 mcn - Created.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "plDXPipeline.h"

#include "HeadSpin.h"
#include "hsWindows.h"
#include <d3d9.h>
#include <DirectXMath.h>

#include "plDXBufferRefs.h"
#include "plDXEnumerate.h"
#include "plDXLightRef.h"
#include "plDXPixelShader.h"
#include "plDXRenderTargetRef.h"
#include "plDXTextFont.h"
#include "plDXTextureRef.h"
#include "plDXVertexShader.h"
#include "hsGDirect3D.h"

#include "hsFastMath.h"
#include "hsGMatState.inl"
#include "plPipeDebugFlags.h"
#include "plPipeResReq.h"
#include "plProfile.h"
#include "plQuality.h"
#include "hsResMgr.h"
#include "hsSIMD.h"
#include "hsTemplates.h"
#include "hsTimer.h"
#include "plTweak.h"

// Project local
#include "plPipeline/plCubicRenderTarget.h"
#include "plPipeline/plCullTree.h"
#include "plPipeline/plDebugText.h"
#include "plPipeline/plDynamicEnvMap.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "plPipeline/plFogEnvironment.h"
#include "plPipeline/plPipelineCreate.h"
#include "plPipeline/plRenderTarget.h"
#include "plPipeline/plStatusLogDrawer.h"
#include "plPipeline/hsWinRef.h"

#include "pnMessage/plClientMsg.h"
#include "pnMessage/plPipeResMakeMsg.h"
#include "pnNetCommon/plNetApp.h"   // for dbg logging
#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plAvatarClothing.h"
#include "plDrawable/plAccessSpan.h"
#include "plDrawable/plAuxSpan.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plGeometrySpan.h"
#include "plDrawable/plSpaceTree.h"
#include "plDrawable/plSpanTypes.h"
#include "plGImage/hsCodecManager.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGImage/plMipmap.h"
#include "plGLight/plLightInfo.h"
#include "plGLight/plShadowCaster.h"
#include "plGLight/plShadowSlave.h"
#include "plMessage/plDeviceRecreateMsg.h"
#include "plResMgr/plLocalization.h"
#include "plScene/plRenderRequest.h"
#include "plScene/plVisMgr.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerDepth.h" // mf horse - test hack, nuke this later
#include "plSurface/plLayerInterface.h"
#include "plSurface/plLayerShadowBase.h"
#include "plSurface/plShader.h"

#include "pfCamera/plCameraModifier.h"
#include "pfCamera/plVirtualCamNeu.h"

#include <algorithm>

//#define MF_TOSSER

int mfCurrentTest = 100;
//#define MF_ENABLE_HACKOFF
#ifdef MF_ENABLE_HACKOFF
//WHITE
static hsTArray<plRenderTarget*> hackOffscreens;
uint32_t doHackPlate = uint32_t(-1);
#endif // MF_ENABLE_HACKOFF

uint32_t  fDbgSetupInitFlags;     // HACK temp only

#ifdef HS_DEBUGGING
void plReleaseObject(IUnknown* x)
{
    if( x )
    {
        int refs = x->Release();
        if( refs )
            refs = 0;
    }
}
#else // HS_DEBUGGING
void plReleaseObject(IUnknown* x)
{
    if( x )
        x->Release();
}
#endif // HS_DEBUGGING

//// Local Static Stuff ///////////////////////////////////////////////////////

/// Macros for getting/setting data in a D3D vertex buffer
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
static inline const uint8_t* inlExtract<hsPoint3>(const uint8_t* src, hsPoint3* val)
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
static inline const uint8_t* inlExtract<hsVector3>(const uint8_t* src, hsVector3* val)
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

inline DWORD F2DW( FLOAT f ) 
{ 
    return *((DWORD*)&f); 
}

//// Macros for D3D error handling
#define INIT_ERROR_CHECK( cond, errMsg ) if( FAILED( fSettings.fDXError = cond ) ) { return ICreateFail( errMsg ); }    

#if 1       // DEBUG
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fSettings.fDXError = cond ) ) { IGetD3DError(); IShowErrorMessage(); }   
#define WEAK_ERROR_CHECK( cond )    STRONG_ERROR_CHECK( cond )
#else
#define STRONG_ERROR_CHECK( cond ) if( FAILED( fSettings.fDXError = cond ) ) { IGetD3DError(); }    
#define WEAK_ERROR_CHECK( cond )    cond
#endif

static D3DMATRIX d3dIdentityMatrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f };

static const enum _D3DTRANSFORMSTATETYPE    sTextureStages[ 8 ] =
{
    D3DTS_TEXTURE0, D3DTS_TEXTURE1, D3DTS_TEXTURE2, D3DTS_TEXTURE3, 
    D3DTS_TEXTURE4, D3DTS_TEXTURE5, D3DTS_TEXTURE6, D3DTS_TEXTURE7
};

static const float kAvTexPoolShrinkThresh = 30.f; // seconds

// This caps the number of D3D lights we use. We'll use up to the max allowed
// or this number, whichever is smaller. (This is to prevent us going haywire
// on trying to allocate an array for ALL of the lights in the Ref device.)
//#define kD3DMaxTotalLights        32
///HAAAAACK Let's be mean and limit the artists to only 4 run-time lights.... hehehehhehe (not my idea!!!)
const int kD3DMaxTotalLights = 8;
// The framerate is the limit on the number of projected lights an object can have.
const int kMaxProjectors = 100;

/// This controls whether we can draw bounds boxes around all the ice spans.
//#ifdef HS_DEBUGGING
#define MCN_BOUNDS_SPANS    1
//#endif

#define MF_BOUNDS_LEVEL_ICE 1
//#define HS_D3D_USE_SPECULAR

/// Define this to write out z-buffer debug info to plasmalog.txt
#ifdef HS_DEBUGGING
//#define DBG_WRITE_FORMATS
#endif

plProfile_CreateMemCounter("Pipeline Surfaces", "Memory", MemPipelineSurfaces);
plProfile_Extern(MemVertex);
plProfile_Extern(MemIndex);
plProfile_CreateCounter("Feed Triangles", "Draw", DrawFeedTriangles);
plProfile_CreateCounter("Polys", "General", DrawTriangles);
plProfile_CreateCounter("Draw Prim Static", "Draw", DrawPrimStatic);
plProfile_CreateMemCounter("Total Texture Size", "Draw", TotalTexSize);
plProfile_CreateCounter("Material Change", "Draw", MatChange);
plProfile_CreateCounter("Layer Change", "Draw", LayChange);

plProfile_CreateCounterNoReset("Reload", "PipeC", PipeReload);

plProfile_CreateTimer("PrepShadows", "PipeT", PrepShadows);
plProfile_CreateTimer("PrepDrawable", "PipeT", PrepDrawable);
plProfile_CreateTimer("  Skin", "PipeT", Skin);
plProfile_CreateTimer("  AvSort", "PipeT", AvatarSort);
plProfile_CreateTimer("     ClearLights", "PipeT", ClearLights);
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

plProfile_CreateMemCounter("DefMem", "PipeC", DefaultMem);
plProfile_CreateMemCounter("ManMem", "PipeC", ManagedMem);
plProfile_CreateMemCounterReset("CurrTex", "PipeC", CurrTex);
plProfile_CreateMemCounterReset("CurrVB", "PipeC", CurrVB);
plProfile_CreateMemCounterReset("fTexUsed", "PipeC", fTexUsed);
plProfile_CreateMemCounterReset("fTexManaged", "PipeC", fTexManaged);
plProfile_CreateMemCounterReset("fVtxUsed", "PipeC", fVtxUsed);
plProfile_CreateMemCounterReset("fVtxManaged", "PipeC", fVtxManaged);
plProfile_CreateCounter("Merge", "PipeC", SpanMerge);
plProfile_CreateCounter("TexNum", "PipeC", NumTex);
plProfile_CreateCounter("LiState", "PipeC", MatLightState);
plProfile_CreateCounter("NumSkin", "PipeC", NumSkin);
plProfile_CreateCounter("AvatarFaces", "PipeC", AvatarFaces);
plProfile_CreateCounter("VertexChange", "PipeC", VertexChange);
plProfile_CreateCounter("IndexChange", "PipeC", IndexChange);
plProfile_CreateCounter("DynVBuffs", "PipeC", DynVBuffs);
plProfile_CreateCounter("EmptyList", "PipeC", EmptyList);
plProfile_CreateCounter("AvRTPoolUsed", "PipeC", AvRTPoolUsed);
plProfile_CreateCounter("AvRTPoolCount", "PipeC", AvRTPoolCount);
plProfile_CreateCounter("AvRTPoolRes", "PipeC", AvRTPoolRes);
plProfile_CreateCounter("AvRTShrinkTime", "PipeC", AvRTShrinkTime);

static const float kPerspLayerScale = 0.00001f;
static const float kPerspLayerScaleW = 0.001f;
static const float kPerspLayerTrans = 0.00002f;

#ifndef PLASMA_EXTERNAL_RELEASE
/// Fun inlines for keeping track of surface creation/deletion memory
void D3DSURF_MEMNEW(IDirect3DSurface9* surf)
{
    if( surf ) 
    { 
        D3DSURFACE_DESC info; 
        surf->GetDesc( &info );
        PROFILE_POOL_MEM(D3DPOOL_DEFAULT, info.Width * info.Height * plDXPipeline::GetDXBitDepth(info.Format) / 8 + sizeof(IDirect3DSurface9), true, "D3DSurface");
        plProfile_NewMem(MemPipelineSurfaces, info.Width * info.Height * plDXPipeline::GetDXBitDepth(info.Format) / 8 + sizeof(IDirect3DSurface9)); 
    }
}

void D3DSURF_MEMNEW(IDirect3DTexture9* tex)
{
    if( tex )
    {
        IDirect3DSurface9* surf;
        tex->GetSurfaceLevel(0, &surf);
        if( surf )
        {
            D3DSURF_MEMNEW(surf);
            surf->Release();
        }
    }
}

void D3DSURF_MEMNEW(IDirect3DCubeTexture9* cTex)
{
    if( cTex )
    {
        IDirect3DSurface9* surf;
        cTex->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &surf);
        if( surf )
        {
            D3DSURF_MEMNEW(surf);
            D3DSURF_MEMNEW(surf);
            D3DSURF_MEMNEW(surf);
            D3DSURF_MEMNEW(surf);
            D3DSURF_MEMNEW(surf);
            D3DSURF_MEMNEW(surf);
            surf->Release();
        }
    }   
}

void D3DSURF_MEMDEL(IDirect3DSurface9* surf)
{
    if( surf ) 
    { 
        D3DSURFACE_DESC info; 
        surf->GetDesc( &info );
        PROFILE_POOL_MEM(D3DPOOL_DEFAULT, info.Width * info.Height * plDXPipeline::GetDXBitDepth(info.Format) / 8 + sizeof(IDirect3DSurface9), false, "D3DSurface");
        plProfile_DelMem(MemPipelineSurfaces, info.Width * info.Height * plDXPipeline::GetDXBitDepth(info.Format) / 8 + sizeof(IDirect3DSurface9)); 
    }
}

void D3DSURF_MEMDEL(IDirect3DTexture9* tex)
{
    if( tex )
    {
        IDirect3DSurface9* surf;
        tex->GetSurfaceLevel(0, &surf);
        if( surf )
        {
            D3DSURF_MEMDEL(surf);
            surf->Release();
        }
    }
}

void D3DSURF_MEMDEL(IDirect3DCubeTexture9* cTex)
{
    if( cTex )
    {
        IDirect3DSurface9* surf;
        cTex->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, 0, &surf);
        if( surf )
        {
            D3DSURF_MEMDEL(surf);
            D3DSURF_MEMDEL(surf);
            D3DSURF_MEMDEL(surf);
            D3DSURF_MEMDEL(surf);
            D3DSURF_MEMDEL(surf);
            D3DSURF_MEMDEL(surf);
            surf->Release();
        }
    }   
}
#else
void D3DSURF_MEMNEW(IDirect3DSurface9* surf) {}
void D3DSURF_MEMNEW(IDirect3DTexture9* tex) {}
void D3DSURF_MEMNEW(IDirect3DCubeTexture9* cTex) {}
void D3DSURF_MEMDEL(IDirect3DSurface9* surf) {}
void D3DSURF_MEMDEL(IDirect3DTexture9* tex) {}
void D3DSURF_MEMDEL(IDirect3DCubeTexture9* cTex) {}
#endif // PLASMA_EXTERNAL_RELEASE

#ifndef PLASMA_EXTERNAL_RELEASE
void plDXPipeline::ProfilePoolMem(D3DPOOL poolType, uint32_t size, bool add, const char *id)
{
    switch( poolType )
    {
    case D3DPOOL_MANAGED:
        if (add)
        {
            plProfile_NewMem(ManagedMem, size);
            //plStatusLog::AddLineSF("pipeline.log", 0xffff0000, "Adding   MANAGED mem. Size: {10d}, Total: {10d} ID: {}",
            //                    size, gProfileVarManagedMem.GetValue(), id);
        }
        else
        {
            plProfile_DelMem(ManagedMem, size);
            //plStatusLog::AddLineSF("pipeline.log", 0xffff0000, "Deleting MANAGED mem. Size: {10d}, Total: {10d} ID: {}",
            //                    size, gProfileVarManagedMem.GetValue(), id);
        }
        break;
    default:
        if (add)
        {
            plProfile_NewMem(DefaultMem, size);
            //plStatusLog::AddLineSF("pipeline.log", 0xffff0000, "Adding   DEFAULT mem. Size: {10d}, Total: {10d} ID: {}",
            //                    size, gProfileVarDefaultMem.GetValue(), id);
        }
        else
        {
            plProfile_DelMem(DefaultMem, size);
            //plStatusLog::AddLineSF("pipeline.log", 0xffff0000, "Deleting DEFAULT mem. Size: {10d}, Total: {10d} ID: {}",
            //                    size, gProfileVarDefaultMem.GetValue(), id);
        }
        break;
    }
}
#endif // PLASMA_EXTERNAL_RELEASE

/////////////////////////////////////////////////////////////////////////////////////////
// Implementations of RenderPrims types.
// Currently support render tri list
// These allow the same setup code path to be followed, no matter what the primitive type
// (i.e. data-type/draw-call is going to happen once the render state is set.
// Originally useful to make one code path for trilists, tri-patches, and rect-patches, but
// we've since dropped support for patches. We still use the RenderNil function to allow the
// code to go through all the state setup without knowing whether a render call is going to
// come out the other end.
// Would allow easy extension for supporting tristrips or pointsprites, but we've never had
// a strong reason to use either.
// First, Declarations.

// Adding a nil RenderPrim for turning off drawing
class plRenderNilFunc : public plRenderPrimFunc
{
public:
    plRenderNilFunc() {}

    bool RenderPrims() const override { return false; }
};
static plRenderNilFunc sRenderNil;

class plRenderTriListFunc : public plRenderPrimFunc
{
protected:
    LPDIRECT3DDEVICE9   fD3DDevice;
    int                 fBaseVertexIndex;
    int                 fVStart;
    int                 fVLength;
    int                 fIStart;
    int                 fNumTris;
public:
    plRenderTriListFunc(LPDIRECT3DDEVICE9 d3dDevice, int baseVertexIndex,
                        int vStart, int vLength, int iStart, int iNumTris)
        : fD3DDevice(d3dDevice), fBaseVertexIndex(baseVertexIndex), fVStart(vStart), fVLength(vLength), fIStart(iStart), fNumTris(iNumTris) {}

    bool RenderPrims() const override;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Implementations

bool plRenderTriListFunc::RenderPrims() const
{
    plProfile_IncCount(DrawFeedTriangles, fNumTris);
    plProfile_IncCount(DrawTriangles, fNumTris);
    plProfile_Inc(DrawPrimStatic);

    return FAILED( fD3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, fBaseVertexIndex, fVStart, fVLength, fIStart, fNumTris ) );
}   

//// Constructor & Destructor /////////////////////////////////////////////////

uint32_t plDXPipeline::fTexUsed(0);
uint32_t plDXPipeline::fTexManaged(0);
uint32_t plDXPipeline::fVtxUsed(0);
uint32_t plDXPipeline::fVtxManaged(0);

plDXPipeline::plDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devModeRec )
:   pl3DPipeline(devModeRec),
    fManagedAlloced(false),
    fAllocUnManaged(false)
{
    hsAssert(D3DTSS_TCI_PASSTHRU == plLayerInterface::kUVWPassThru, "D3D Enum has changed. Notify graphics department.");
    hsAssert(D3DTSS_TCI_CAMERASPACENORMAL == plLayerInterface::kUVWNormal, "D3D Enum has changed. Notify graphics department.");
    hsAssert(D3DTSS_TCI_CAMERASPACEPOSITION == plLayerInterface::kUVWPosition, "D3D Enum has changed. Notify graphics department.");
    hsAssert(D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR == plLayerInterface::kUVWReflect, "D3D Enum has changed. Notify graphics department.");

    // Initialize everything to NULL.
    IClearMembers();

    // Get the requested mode and setup
    const hsG3DDeviceRecord *devRec = devModeRec->GetDevice();
    const hsG3DDeviceMode *devMode = devModeRec->GetMode();

    /// Init our screen mode
    fDevice.fHWnd = hWnd;
    fDevice.fPipeline = this;

    if( devRec->GetAASetting() == 0 )
        fSettings.fNumAASamples = 0;
    else
        fSettings.fNumAASamples = devMode->GetFSAAType( devRec->GetAASetting() - 1 );

    hsGDirect3DTnLEnumerate& d3dEnum = hsGDirect3D::EnumerateTnL();
    if( d3dEnum.GetEnumeErrorStr()[ 0 ] )
    {
        IShowErrorMessage( (char *)d3dEnum.GetEnumeErrorStr() );
        return;
    }

    if( d3dEnum.SelectFromDevMode(devRec, devMode) )
    {
        IShowErrorMessage( (char *)d3dEnum.GetEnumeErrorStr() );
        return;
    }

    // Record the requested mode/setup.
    ISetCurrentDriver( d3dEnum.GetCurrentDriver() );
    ISetCurrentDevice( d3dEnum.GetCurrentDevice() );
    D3DEnum_ModeInfo *pModeInfo = d3dEnum.GetCurrentMode();
    pModeInfo->fWindowed = fInitialPipeParams.Windowed;     // set windowed mode from ini file
    ISetCurrentMode( d3dEnum.GetCurrentMode() );

    fSettings.fFullscreen = !fCurrentMode->fWindowed;

    fSettings.fNumAASamples = fInitialPipeParams.AntiAliasingAmount;

    // ISetCaps just records the card capabilities that were passed in.
    ISetCaps();
    // IRestrictCaps looks over those explicit caps and makes some decisions on 
    // what the card can really do.
    IRestrictCaps( *devRec );

    fSettings.fMaxAnisotropicSamples = fInitialPipeParams.AnisotropicLevel;
    if(fSettings.fMaxAnisotropicSamples > fCurrentDevice->fDDCaps.MaxAnisotropy)
        fSettings.fMaxAnisotropicSamples = (uint8_t)fCurrentDevice->fDDCaps.MaxAnisotropy;

    plConst(uint32_t) kDefaultDynVtxSize(32000 * 44);
    plConst(uint32_t) kDefaultDynIdxSize(0 * plGBufferGroup::kMaxNumIndicesPerBuffer * 2);
    fDynVtxSize = kDefaultDynVtxSize;
    fVtxRefTime = 0;

    // Go create surfaces and DX-dependent objects
    if( ICreateDeviceObjects() )
    {
        IShowErrorMessage( "Cannot create Direct3D device" );
        return;
    }

    // We don't need the TnL enumeration for the lifetime of the game, so say goodbye!
    hsGDirect3D::ReleaseTnLEnum();
}

// Cleanup - Most happens in IReleaseDeviceObject().
plDXPipeline::~plDXPipeline()
{
    fCurrLay = nullptr;
    hsAssert(fCurrMaterial == nullptr, "Current material not unrefed properly");

    // CullProxy is a debugging representation of our CullTree. See plCullTree.cpp, 
    // plScene/plOccluder.cpp and plScene/plOccluderProxy.cpp for more info
    if( fView.HasCullProxy() )
        fView.GetCullProxy()->GetKey()->UnRefObject();
    delete fCurrentDriver;
    delete fCurrentDevice;
    delete fCurrentMode;

    IReleaseDeviceObjects();
    IClearClothingOutfits(&fClothingOutfits);
    IClearClothingOutfits(&fPrevClothingOutfits);
}

//// IClearMembers ////////////////////////////////////////////////////////////
// Initialize everything to a nil state.
// This does not initialize to a working state, but to a state that can be
// built from. For example, the fD3DObject pointer is set to nil so that it's safe
// to delete or set to a valid pointer. It must be set to a valid pointer
// before the pipeline can be used for much.
// After the core initialization is done (in ICreateDeviceObjects)
// render state will be initialized in IInitDeviceState.

void    plDXPipeline::IClearMembers()
{
    /// Clear some stuff
    fTextureRefList = nullptr;
    fTextFontRefList = nullptr;
    fRenderTargetRefList = nullptr;
    fVShaderRefList = nullptr;
    fPShaderRefList = nullptr;
#if MCN_BOUNDS_SPANS
    fBoundsMat = nullptr;
    fBoundsSpans = nullptr;
#endif
    fPlateMgr = nullptr;
    fLogDrawer = nullptr;
    fDebugTextMgr = nullptr;

    fTexturing = false;
    fLastEndingStage = -1;

    fSettings.Reset();
    fStencil.Reset();
    fLights.Reset(this);
    fCurrFog.Reset();
    fDeviceLost = false;
    fDevWasLost = false;

    fSettings.fCurrFVFFormat = 0;
    fDynVtxBuff = nullptr;
    fNextDynVtx = 0;

    int i;

    IResetRenderTargetPools();
    fULutTextureRef = nullptr;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
        fBlurVBuffers[i] = nullptr;
    fBlurVSHandle = 0;

    fSharedDepthSurface[0] = nullptr;
    fSharedDepthFormat[0] = D3DFMT_UNKNOWN;
    fSharedDepthSurface[1] = nullptr;
    fSharedDepthFormat[1] = D3DFMT_UNKNOWN;

    fCurrentMode = nullptr;
    fCurrentDriver = nullptr;
    fCurrentDevice = nullptr;

    for( i = 0; i < 8; i++ )
    {
        fLayerState[i].Reset();
        fOldLayerState[i].Reset();
    }

    fMaxNumLights = kD3DMaxTotalLights;
    fMaxNumProjectors = kMaxProjectors;

    fRenderCnt = 0;

    fForceMatHandle = true;
    fAvRTShrinkValidSince = 0;
    fAvRTWidth = 1024;
    fAvNextFreeRT = 0;
}

//// plDXGeneralSettings::Reset //////////////////////////////////////////////
// Catch all struct of general settings plus pointers to current d3d objects.

void    plDXGeneralSettings::Reset()
{
    fCurrVertexBuffRef = nullptr;
    fCurrIndexBuffRef = nullptr;
    fFullscreen = false;
    fD3DCaps = 0;
    fBoardKluge = 0;
    fStageEnd = 0;
    fBoundsDrawLevel = -1;

    fClearColor = 0;

    fNoGammaCorrect = false;
    fMaxUVWSrc = 8;
    fCantProj = false;
    fBadManaged = false;
    fShareDepth = false;
    fCurrAnisotropy = false;
    fIsIntel = false;

    fDXError = D3D_OK;
    memset( fErrorStr, 0, sizeof( fErrorStr ) );

    fCurrFVFFormat = 0;
    fCurrVertexShader = nullptr;
    fCurrPixelShader = nullptr;

    fVeryAnnoyingTextureInvalidFlag = false;
}

//// IInitDeviceState /////////////////////////////////////////////////////////
// Initialize the device to a known state. This also syncs it up with our internal state
// as recorded in the fLayerStates. 
// Some of these states reflect the caps of the device, but for the most part, the 
// important thing here is NOT what state we're in coming out of this function, but
// that we are in a known state, and that the known state is recorded in fLayerStates.
void    plDXPipeline::IInitDeviceState()
{
    fLayerState[0].Reset();
    fDevice.fCurrCullMode = D3DCULL_CW;

    /// Set D3D states
    fCurrFog.Reset();
    ISetFogParameters(nullptr, nullptr);

    fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_LESSEQUAL );
    fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    fD3DDevice->SetRenderState( D3DRS_ZENABLE,      D3DZB_TRUE );
    fD3DDevice->SetRenderState( D3DRS_CLIPPING,     TRUE ); 
    fD3DDevice->SetRenderState( D3DRS_CULLMODE,     fDevice.fCurrCullMode );
    ISetCullMode();

    fD3DDevice->SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE );
    fD3DDevice->SetRenderState( D3DRS_ALPHAFUNC,        D3DCMP_GREATEREQUAL );
    fD3DDevice->SetRenderState( D3DRS_ALPHAREF,         0x00000001 );

    fD3DDevice->SetRenderState( D3DRS_MULTISAMPLEANTIALIAS, ( fSettings.fD3DCaps & kCapsFSAntiAlias ) ? TRUE : FALSE );
    fD3DDevice->SetRenderState( D3DRS_ANTIALIASEDLINEENABLE,        FALSE );

    fD3DDevice->SetRenderState( D3DRS_DITHERENABLE,     FALSE );
    fD3DDevice->SetRenderState( D3DRS_SPECULARENABLE,   FALSE );
    fD3DDevice->SetRenderState( D3DRS_LIGHTING,         FALSE );    
    fCurrD3DLiteState = false;
    fD3DDevice->SetRenderState( D3DRS_TEXTUREFACTOR,    0x0 );
    fD3DDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
    fD3DDevice->SetTransform( D3DTS_TEXTURE0,           &d3dIdentityMatrix );
    fD3DDevice->SetTransform( D3DTS_WORLD,              &d3dIdentityMatrix );

    /// NEW: to compensate for scaling transformations that might screw up our nicely
    /// normalized normals. Note: nVidia says this is as fast or faster than with
    /// this disabled, but who knows what it'll do on other cards...
    fD3DDevice->SetRenderState( D3DRS_NORMALIZENORMALS, TRUE );
    fD3DDevice->SetRenderState( D3DRS_LOCALVIEWER, TRUE );

    uint32_t totalMem = fD3DDevice->GetAvailableTextureMem();
    plProfile_Set(TotalTexSize, totalMem);

    // Initialization for all 8 stages (even though we only use a few of them).
    int i;
    for( i = 0; i < 8; i++ )
    {
        fLayerLODBias[ i ] = fTweaks.fDefaultLODBias;
        fLayerTransform[ i ] = false;
        fLayerRef[i] = nullptr;
        fLayerUVWSrcs[ i ] = i;
        fLayerState[ i ].Reset();

        fD3DDevice->SetTexture(i, nullptr);
        fD3DDevice->SetTextureStageState( i, D3DTSS_TEXCOORDINDEX, i );
        fD3DDevice->SetSamplerState( i, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP  );
        fD3DDevice->SetSamplerState( i, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP  );
        fD3DDevice->SetSamplerState( i, D3DSAMP_MIPMAPLODBIAS, *(DWORD *)( &fLayerLODBias[ i ] ) );

        if( fSettings.fMaxAnisotropicSamples > 0 && !IsDebugFlagSet(plPipeDbg::kFlagNoAnisotropy))
        {
            fD3DDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAXANISOTROPY, (DWORD)fSettings.fMaxAnisotropicSamples );
            fSettings.fCurrAnisotropy = true;
        }
        else
        {
            fD3DDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
            fSettings.fCurrAnisotropy = false;
        }
        fD3DDevice->SetSamplerState( i, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR );

        fD3DDevice->SetTransform( sTextureStages[ i ], &d3dIdentityMatrix );
        fLayerXformFlags[ i ] = D3DTTFF_COUNT2;
        fD3DDevice->SetTextureStageState( i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2 );
    }

    // Initialize our bump mapping matrices.
    for( i = 0; i < 4; i++ )
    {
        int j;
        for( j = 0; j < 4; j++ )
        {
            fBumpDuMatrix.fMap[i][j] = 0;
            fBumpDvMatrix.fMap[i][j] = 0;
            fBumpDwMatrix.fMap[i][j] = 0;
            
        }
    }
    fBumpDuMatrix.NotIdentity();
    fBumpDvMatrix.NotIdentity();
    fBumpDwMatrix.NotIdentity();
    
    PushMaterialOverride( hsGMatState::kShade, hsGMatState::kShadeSpecularHighlight, false );

    fLights.Reset(this);

    // Tell the light infos to unlink themselves
    while (fActiveLights)
        UnRegisterLight(fActiveLights);

    return;
}

//// ISetCaps /////////////////////////////////////////////////////////////////
// We've recorded the capabilities of the current device in fCurrentDevice (traditionally in the setup program),
// now translate that into our own caps flags.
void    plDXPipeline::ISetCaps()
{
    fSettings.fD3DCaps = kCapsNone;

    // Set relevant caps (ones we can do something about).
    if (fCurrentDevice->fDDCaps.RasterCaps & D3DPRASTERCAPS_DEPTHBIAS)
        fSettings.fD3DCaps |= kCapsZBias;
    if (fCurrentDevice->fDDCaps.RasterCaps & D3DPRASTERCAPS_FOGRANGE)
        fSettings.fD3DCaps |= kCapsRangeFog;
    if (fCurrentDevice->fDDCaps.RasterCaps & D3DPRASTERCAPS_FOGTABLE)
        fSettings.fD3DCaps |= kCapsLinearFog | kCapsExpFog | kCapsExp2Fog | kCapsPixelFog;
    else
        fSettings.fD3DCaps |= kCapsLinearFog;
    if (fCurrentDevice->fDDCaps.TextureFilterCaps & D3DPTFILTERCAPS_MIPFLINEAR)
        fSettings.fD3DCaps |= kCapsMipmap;
    if (fCurrentDevice->fDDCaps.TextureCaps & D3DPTEXTURECAPS_MIPCUBEMAP)
        fSettings.fD3DCaps |= kCapsCubicMipmap;
    if (fSettings.fNumAASamples > 0)
        fSettings.fD3DCaps |= kCapsFSAntiAlias;
    if (fCurrentDevice->fDDCaps.RasterCaps & D3DPRASTERCAPS_WFOG)
        fSettings.fD3DCaps |= kCapsDoesWFog;
    if (fCurrentDevice->fDDCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP)
        fSettings.fD3DCaps |= kCapsCubicTextures;

    // Unconditional Non-Power of Two Textures
    // To make life easy for us, we can have non POT textures or we can't
    if (!(fCurrentDevice->fDDCaps.TextureCaps & D3DPTEXTURECAPS_POW2 &&
          fCurrentDevice->fDDCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL))
          fSettings.fD3DCaps |= kCapsNpotTextures;

    /// New 1.5.2000 - cull out mixed vertex processing
    if (fCurrentDevice->fDDCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT &&
        fCurrentMode->fDDBehavior == D3DCREATE_HARDWARE_VERTEXPROCESSING)
        fSettings.fD3DCaps |= kCapsHWTransform;


    // Currently always want d3d to transform
    fSettings.fD3DCaps |= kCapsHWTransform;

    /// Always assume we can do small textures (IRestrictCaps will turn this off
    /// if necessary)
    fSettings.fD3DCaps |= kCapsDoesSmallTextures;

    /// Look for supported texture formats
    if ( IFindCompressedFormats() )
        fSettings.fD3DCaps |= kCapsCompressTextures;
    if ( IFindLuminanceFormats() )
        fSettings.fD3DCaps |= kCapsLuminanceTextures;

    /// Max # of hardware lights
    fMaxNumLights = fCurrentDevice->fDDCaps.MaxActiveLights;
    if ( fMaxNumLights > kD3DMaxTotalLights )
        fMaxNumLights = kD3DMaxTotalLights;

    // Intel Extreme chips report 0 lights, meaning T&L is done
    // in software, so you can have as many lights as you want.
    // We only need 8, so set that here. Also turn off shadows,
    // since the extreme can't really afford them, and record
    // the fact this is the extreme for other driver problem
    // workarounds.
    if ( !fMaxNumLights )
    {
        fMaxNumLights = kD3DMaxTotalLights;
        fSettings.fIsIntel = true;
        plShadowCaster::SetCanShadowCast(false);
    }

    /// Max # of textures at once
    fMaxLayersAtOnce = fCurrentDevice->fDDCaps.MaxSimultaneousTextures;
    if ( fCurrentDevice->fDDCaps.DevCaps & D3DDEVCAPS_SEPARATETEXTUREMEMORIES )
        fMaxLayersAtOnce = 1;
    // Alloc half our simultaneous textures to piggybacks.
    // Won't hurt us unless we try to many things at once.
    fMaxPiggyBacks = fMaxLayersAtOnce >> 1; 

    // Less than 4 layers at once means we have to fallback on uv bumpmapping
    if (fMaxLayersAtOnce < 4)
        SetDebugFlag(plPipeDbg::kFlagBumpUV, true);

    fSettings.fMaxAnisotropicSamples = (uint8_t)(fCurrentDevice->fDDCaps.MaxAnisotropy);

    fSettings.fNoGammaCorrect = !(fCurrentDevice->fDDCaps.Caps2 & D3DCAPS2_FULLSCREENGAMMA);

    if (!(fCurrentDevice->fDDCaps.TextureCaps & D3DPTEXTURECAPS_PROJECTED))
        plDynamicCamMap::SetCapable(false);

    ISetGraphicsCapability(fCurrentDevice->fDDCaps.PixelShaderVersion);
}

// ISetGraphicsCapability ///////////////////////////////////////////////////////
// Tell our global quality settings what we can do. We'll use this to only load
// versions we can render. So if we can render it, we load it and skip its low quality substitute,
// if we can't render it, we skip it and load its low quality substitute. 
// Naturally, this must happen before we do any loading.
void plDXPipeline::ISetGraphicsCapability(uint32_t v)
{
    int pixelMajor = D3DSHADER_VERSION_MAJOR(v);
    int pixelMinor = D3DSHADER_VERSION_MINOR(v);

    switch (pixelMajor)
    {
    case 1:
        if (pixelMinor >= 4)
            plQuality::SetCapability(plQuality::kPS_1_4);
        else if (pixelMinor > 0)
            plQuality::SetCapability(plQuality::kPS_1_1);
        break;
    case 2:
        plQuality::SetCapability(plQuality::kPS_2);
        break;
    case 3:
        plQuality::SetCapability(plQuality::kPS_3);
        break;
    default:
        // Hopefully this is always FALSE. If not, may gawd have mercy upon your soul.
        if (pixelMajor == 0)
            plQuality::SetCapability(plQuality::kMinimum);
        else
            plQuality::SetCapability(plQuality::kPS_3);
        break;
    }
}

//// IRestrictCaps ////////////////////////////////////////////////////////////
// ISetCaps() sets our native caps based on the D3D caps bits D3D returns.
// IRestrictCaps looks at our hsG3DDeviceSelector flags and translates those
// into our runtime native caps.
// The DeviceSelector flags aren't set by what the board claims, but rather
// we try to identify the board and set them according to previous knowledge.
// For example, the ATI7500 will only use uvw coordinates 0 or 1. There's
// no d3d cap to reflect this, and it really should support [0..7], but 
// there's no way to force it to be d3d compliant. So when we see we have
// an ATI7500, we set the cap kCapsMaxUVWSrc2.
// See hsG3DDeviceSelector.cpp for details and implementation.
void    plDXPipeline::IRestrictCaps( const hsG3DDeviceRecord& devRec )
{
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsMipmap ) )
        fSettings.fD3DCaps &= ~kCapsMipmap;
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsCubicMipmap ) )
        fSettings.fD3DCaps &= ~kCapsCubicMipmap;
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsZBias ) )
        fSettings.fD3DCaps &= ~kCapsZBias;
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsFogExp ) )
        fSettings.fD3DCaps &= ~kCapsExpFog;
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsCubicTextures ) )
        fSettings.fD3DCaps &= ~kCapsCubicTextures;

    if( devRec.GetCap(hsG3DDeviceSelector::kCapsCantShadow) )
        plShadowCaster::SetCanShadowCast(false);

    if( devRec.GetCap(hsG3DDeviceSelector::kCapsCantProj) )
        fSettings.fCantProj = true;
    if( devRec.GetCap(hsG3DDeviceSelector::kCapsBadManaged) )
        fSettings.fBadManaged = true;
    if( devRec.GetCap(hsG3DDeviceSelector::kCapsShareDepth) )
        fSettings.fShareDepth = true;

    /// Added 9.6.2000 mcn - shouldn't they be here anyway?
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsFogExp2 ) )
        fSettings.fD3DCaps &= ~kCapsExp2Fog;
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsDoesSmallTextures ) )
        fSettings.fD3DCaps &= ~kCapsDoesSmallTextures;

    /// 9.22.2000 mcn - dFlag for bad (savage4) yon fix
    if( devRec.GetCap( hsG3DDeviceSelector::kCapsBadYonStuff ) )
        fSettings.fD3DCaps |= kCapsHasBadYonStuff;

    /// Note: the following SHOULD be here, but we later detect for texture
    /// formats and reset this flag. It should only be set if it is set already,
    /// but that means ensuring it's set beforehand, which it might not be.
    if( !devRec.GetCap( hsG3DDeviceSelector::kCapsCompressTextures ) )
        fSettings.fD3DCaps &= ~kCapsCompressTextures;

    /// Set up tweaks
    SetZBiasScale( (float)devRec.GetZBiasRating() );
    fTweaks.fDefaultLODBias = (float)-( 0.25 + (float)devRec.GetLODBiasRating() );
    devRec.GetFogApproxStarts( fTweaks.fFogExpApproxStart, fTweaks.fFogExp2ApproxStart );
    fTweaks.fFogEndBias = (float)devRec.GetFogEndBias();

    // Fog knee stuff
    devRec.GetFogKneeParams( hsG3DDeviceRecord::kFogExp, fTweaks.fExpFogKnee, fTweaks.fExpFogKneeVal );
    devRec.GetFogKneeParams( hsG3DDeviceRecord::kFogExp2, fTweaks.fExp2FogKnee, fTweaks.fExp2FogKneeVal );

    // Max # of layers
    uint32_t max = devRec.GetLayersAtOnce();
    if( max > 0 && max < fMaxLayersAtOnce )
        fMaxLayersAtOnce = max;

    /// Debug flag to force high-level cards down to GeForce 2 caps
    if( fDbgSetupInitFlags & 0x00000004 )
    {
        fSettings.fD3DCaps &= ~kCapsFSAntiAlias;
        if( fMaxLayersAtOnce > 2 )
            fMaxLayersAtOnce = 2;
        fSettings.fMaxAnisotropicSamples = 0;

        plQuality::SetCapability(plQuality::kMinimum);
    }

    /// Set up the z-bias scale values
    fTweaks.fDefaultPerspLayerScale = kPerspLayerScale;


    // Less than 4 layers at once means we have to fallback on uv bumpmapping
    if( fMaxLayersAtOnce < 4 )
        SetDebugFlag(plPipeDbg::kFlagBumpUV, true);

    if( ( fSettings.fD3DCaps & kCapsHWTransform ) && ( fCurrentMode->fDDBehavior == D3DCREATE_SOFTWARE_VERTEXPROCESSING ) )
        fSettings.fD3DCaps &= ~kCapsHWTransform;

    if( devRec.GetCap(hsG3DDeviceSelector::kCapsMaxUVWSrc2) )
        fSettings.fMaxUVWSrc = 2;

    /// Anisotropy stuff
    //if( devRec.GetMaxAnisotropicSamples() < fSettings.fMaxAnisotropicSamples )
    //  fSettings.fMaxAnisotropicSamples = devRec.GetMaxAnisotropicSamples();
    if( devRec.GetCap(hsG3DDeviceSelector::kCapsNoAniso) || (fSettings.fMaxAnisotropicSamples <= 1) )
        fSettings.fMaxAnisotropicSamples = 0;
}

// Create all our video memory consuming D3D objects.
bool plDXPipeline::ICreateDynDeviceObjects()
{
    // Front/Back/Depth buffers
    if( ICreateNormalSurfaces() )
        return true;

    // RenderTarget pools are shared for our shadow generation algorithm.
    // Different sizes for different resolutions.
    IMakeRenderTargetPools();

    // Create device-specific stuff
    fDebugTextMgr = new plDebugTextManager();
    if (fDebugTextMgr == nullptr)
        return true;

    // Vertex buffers, index buffers, textures, etc.
    LoadResources();

    return false;
}
//// ICreateDeviceObjects /////////////////////////////////////////////////////
//  Create all of our steady state D3D objects. More D3D objects will be created
// and destroyed as ages are loaded and unloaded, but these are the things that
// only go away when we lose the device.

bool  plDXPipeline::ICreateDeviceObjects()
{
    // The D3D device
    if( ICreateDevice(!fSettings.fFullscreen) )
        return true;

    // Most everything else D3D
    if( ICreateDynDeviceObjects() )
        return true;

    // PlateMgr is largely for debugging and performance stats,
    // but also gets used for some things like the cursor and 
    // linking fade to/from black.
    fPlateMgr = new plDXPlateManager( this, fD3DDevice );
    if (fPlateMgr == nullptr || !fPlateMgr->IsValid())
        return true;

    // We've got everything created now, initialize to a known state.
    IInitDeviceState();
    if (FAILED(fD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, fSettings.fClearColor, 1.0f, 0L)))
        return true;

    // You may be wondering what this is. It's a workaround for a GeForce2 driver bug, where
    // clears to the Zbuffer (but not color) are getting partially ignored. Don't even ask.
    // So this is just to try and get the board used to the kind of foolishness it can expect
    // from here out.
    if (FAILED(fD3DDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, fSettings.fClearColor, 1.0f, 0L)))
        return true;
    if (FAILED(fD3DDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, fSettings.fClearColor, 1.0f, 0L)))
        return true;
    if (FAILED(fD3DDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, fSettings.fClearColor, 1.0f, 0L)))
        return true;

    /// Log renderer
    fLogDrawer = new plStatusLogDrawer( this );
    plStatusLogMgr::GetInstance().SetDrawer( fLogDrawer );

    /// Ok, we're done now
#if MCN_BOUNDS_SPANS
    fBoundsSpans = new plDrawableSpans();
    hsgResMgr::ResMgr()->NewKey( "BoundsSpans", fBoundsSpans, plLocation::kGlobalFixedLoc );
    fBoundsSpans->SetNativeProperty( plDrawable::kPropVolatile, true );
    fBoundsMat = new hsGMaterial();
    hsgResMgr::ResMgr()->NewKey( "BoundsMaterial", fBoundsMat, plLocation::kGlobalFixedLoc );
    plLayer *lay = fBoundsMat->MakeBaseLayer();
    lay->SetMiscFlags( hsGMatState::kMiscWireFrame | hsGMatState::kMiscTwoSided );
    lay->SetShadeFlags( lay->GetShadeFlags() | hsGMatState::kShadeWhite );

    // Set up a ref to these. Since we don't have a key, we use the
    // generic RefObject() (and matching UnRefObject() when we're done).
    // If we had a key, we would use myKey->AddViaNotify(otherKey) and myKey->Release(otherKey).
    fBoundsMat->GetKey()->RefObject();
    fBoundsSpans->GetKey()->RefObject();
#endif

    return false;
}

//// ISetCurrentDriver ////////////////////////////////////////////////////////
// Copy over the driver info.
void    plDXPipeline::ISetCurrentDriver( D3DEnum_DriverInfo *driv )
{
    if (fCurrentDriver != nullptr)
        delete fCurrentDriver;

    fCurrentDriver = new D3DEnum_DriverInfo;

    fCurrentDriver->fGuid = driv->fGuid;
    hsStrncpy( fCurrentDriver->fStrDesc, driv->fStrDesc, 40 );
    hsStrncpy( fCurrentDriver->fStrName, driv->fStrName, 40 );

    fCurrentDriver->fDesktopMode = driv->fDesktopMode;
    fCurrentDriver->fAdapterInfo = driv->fAdapterInfo;

    fCurrentDriver->fCurrentMode = nullptr;
    fCurrentDriver->fCurrentDevice = nullptr;

    /// Go looking for an adapter to match this one
    IDirect3D9* d3d = hsGDirect3D::GetDirect3D();
    UINT    iAdapter;

    for( fCurrentAdapter = 0, iAdapter = 0; iAdapter < d3d->GetAdapterCount(); iAdapter++ )
    {
        D3DADAPTER_IDENTIFIER9      adapterInfo;
        d3d->GetAdapterIdentifier( iAdapter, 0, &adapterInfo );

        if( adapterInfo.DeviceIdentifier == fCurrentDriver->fAdapterInfo.DeviceIdentifier )
        {
            fCurrentAdapter = iAdapter;
            break;
        }   
    }
}

//// ISetCurrentDevice ////////////////////////////////////////////////////////
// Copy over the device info.
void    plDXPipeline::ISetCurrentDevice( D3DEnum_DeviceInfo *dev )
{
    if (fCurrentDevice != nullptr)
        delete fCurrentDevice;
    fCurrentDevice = new D3DEnum_DeviceInfo;

    hsStrncpy( fCurrentDevice->fStrName, dev->fStrName, 40 );

    fCurrentDevice->fDDCaps = dev->fDDCaps;
    fCurrentDevice->fDDType = dev->fDDType;
    fCurrentDevice->fIsHardware = dev->fIsHardware;
    fCurrentDevice->fCanWindow = dev->fCanWindow;
//  fCurrentDevice->fCanAntialias = dev->fCanAntialias;
    fCurrentDevice->fCompatibleWithDesktop = dev->fCompatibleWithDesktop;

    // copy over supported device modes
    D3DEnum_ModeInfo currMode;

    for(int i = 0; i < dev->fModes.Count(); i++)
    {
        // filter unusable modes
        if(dev->fModes[i].fDDmode.Width < MIN_WIDTH || dev->fModes[i].fDDmode.Height < MIN_HEIGHT)
            continue;

        currMode.fBitDepth = dev->fModes[i].fBitDepth;
        currMode.fCanRenderToCubic = dev->fModes[i].fCanRenderToCubic;
        currMode.fDDBehavior = dev->fModes[i].fDDBehavior;
        currMode.fDepthFormats = dev->fModes[i].fDepthFormats;
        currMode.fFSAATypes = dev->fModes[i].fFSAATypes;
        memcpy(&currMode.fDDmode, &dev->fModes[i].fDDmode, sizeof(D3DDISPLAYMODE));
        strcpy(currMode.fStrDesc, dev->fModes[i].fStrDesc);
        currMode.fWindowed = dev->fModes[i].fWindowed;

        fCurrentDevice->fModes.Push(currMode);
    }
}

//// ISetCurrentMode //////////////////////////////////////////////////////////
// Copy over the mode info.
void    plDXPipeline::ISetCurrentMode( D3DEnum_ModeInfo *mode )
{
    if (fCurrentMode != nullptr)
        delete fCurrentMode;
    fCurrentMode = new D3DEnum_ModeInfo;

    *fCurrentMode = *mode;
}

//// IFindCompressedFormats ///////////////////////////////////////////////////
//
//  New DX Way: Check to see if each format is valid.

bool  plDXPipeline::IFindCompressedFormats()
{
    D3DFORMAT   toCheckFor[] = {D3DFMT_DXT1, 
                                //D3DFMT_DXT2, 
                                //D3DFMT_DXT3, 
                                //D3DFMT_DXT4, 
                                D3DFMT_DXT5, 
                                D3DFMT_UNKNOWN };
    short       i = 0;


    for( i = 0; toCheckFor[ i ] != D3DFMT_UNKNOWN; i++ )
    {
        if( FAILED( hsGDirect3D::GetDirect3D()->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType,
                                                                   fCurrentMode->fDDmode.Format,
                                                                   0, D3DRTYPE_TEXTURE, toCheckFor[ i ] ) ) )
            return false;
    }

    /// Got here, must have found them all
    return true;
}

//// IFindLuminanceFormats ////////////////////////////////////////////////////
//
//  New DX Way: Check to see if each format we want is valid

bool  plDXPipeline::IFindLuminanceFormats()
{
    D3DFORMAT   toCheckFor[] = { D3DFMT_L8, D3DFMT_A8L8, D3DFMT_UNKNOWN };
    short       i = 0;


    for( i = 0; toCheckFor[ i ] != D3DFMT_UNKNOWN; i++ )
    {
        if (FAILED(hsGDirect3D::GetDirect3D()->CheckDeviceFormat(fCurrentAdapter, fCurrentDevice->fDDType,
                                                                 fCurrentMode->fDDmode.Format,
                                                                 0, D3DRTYPE_TEXTURE, toCheckFor[ i ] ) ) )
            return false;
    }

    /// Got here, must have found them all
    return true;
}

//// ITextureFormatAllowed ////////////////////////////////////////////////////
//
//  Returns true if the given format is supported on the current device and
//  mode, false if it isn't.

bool      plDXPipeline::ITextureFormatAllowed( D3DFORMAT format )
{
    if (FAILED( hsGDirect3D::GetDirect3D()->CheckDeviceFormat(fCurrentAdapter, fCurrentDevice->fDDType,
                                                              fCurrentMode->fDDmode.Format,
                                                              0, D3DRTYPE_TEXTURE, format ) ) )
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//// Device Creation //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// ICreateDevice ////////////////////////////////////////////////////
//
//  Creates the device. Surfaces, buffers, etc. created separately (in case of lost device).
// See ICreateDeviceObjects.

bool plDXPipeline::ICreateDevice(bool windowed)
{
    /// First, create the D3D Device object
    D3DPRESENT_PARAMETERS       params;
    D3DDISPLAYMODE              dispMode;
    int                         i;
#ifdef DBG_WRITE_FORMATS
    char                        msg[ 256 ];
#endif // DBG_WRITE_FORMATS

    IDirect3D9* d3d = hsGDirect3D::GetDirect3D();
    if (!d3d)
        return ICreateFail("Failed to get Direct3D Object");

    INIT_ERROR_CHECK( d3d->GetAdapterDisplayMode( fCurrentAdapter, &dispMode ),
        "Cannot get desktop display mode" );

    // save desktop properties
    fDesktopParams.Width = dispMode.Width;
    fDesktopParams.Height = dispMode.Height;
    fDesktopParams.ColorDepth = GetDXBitDepth( dispMode.Format );


    if( windowed )
    {
        // Reset fColor, since we're getting the desktop bitdepth
        fColorDepth = GetDXBitDepth( dispMode.Format );
        if(fOrigWidth > fDesktopParams.Width || fOrigHeight > fDesktopParams.Height)
        {
            fOrigWidth = fDesktopParams.Width;
            fOrigHeight = fDesktopParams.Height;
            IGetViewTransform().SetScreenSize(fDesktopParams.Width, fDesktopParams.Height);
        }
    }

    memset( &params, 0, sizeof( params ) );
    params.Windowed = TRUE; // NOTE: fullscreen is faked by changing the desktop resolution
    params.Flags = 0;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
    params.BackBufferCount = 1;
    params.BackBufferWidth = GetViewTransform().GetScreenWidth();
    params.BackBufferHeight = GetViewTransform().GetScreenHeight();
    params.EnableAutoDepthStencil = TRUE;

    // NOTE: This was changed 5.29.2001 mcn to avoid the nasty flashing bug on nVidia's 12.60 beta drivers
// SWAPEFFECT must be _DISCARD when using antialiasing, so we'll just go with _DISCARD for the time being. mf
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
    params.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

#ifdef DBG_WRITE_FORMATS
    for( i = 0; i < fCurrentMode->fDepthFormats.GetCount(); i++ )
    {
        sprintf( msg, "-- Valid depth buffer format: %s", IGetDXFormatName( fCurrentMode->fDepthFormats[ i ] ) );
        hsDebugMessage( msg, 0 );
    }
#endif

    // Attempt to find the closest AA setting we can
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    for( i = fSettings.fNumAASamples; i >= 2; i-- )
    {
        if( fCurrentMode->fFSAATypes.Find( (D3DMULTISAMPLE_TYPE)i ) != fCurrentMode->fFSAATypes.kMissingIndex )
        {
            params.MultiSampleType = (D3DMULTISAMPLE_TYPE)i;
            break;
        }
    }

    if( !IFindDepthFormat(params) )
    {
        // If we haven't found a depth format, turn off multisampling and try it again.
        params.MultiSampleType = D3DMULTISAMPLE_NONE;
        if( !IFindDepthFormat(params) )
            // Okay, we're screwed here, we might as well bail.
            return ICreateFail( "Can't find a Depth Buffer format" );
    }

    /// TEMP HACK--if we're running 16-bit z-buffer or below, use our z-bias (go figure, it works better
    /// in 16-bit, worse in 24 and 32)
    if( params.AutoDepthStencilFormat == D3DFMT_D15S1 || 
        params.AutoDepthStencilFormat == D3DFMT_D16 ||
        params.AutoDepthStencilFormat == D3DFMT_D16_LOCKABLE )
        fSettings.fD3DCaps &= ~kCapsZBias;

#ifdef DBG_WRITE_FORMATS
    sprintf( msg, "-- Requesting depth buffer format: %s", IGetDXFormatName( params.AutoDepthStencilFormat ) );
    hsDebugMessage( msg, 0 );
#endif


    params.BackBufferFormat = dispMode.Format;
#ifdef DBG_WRITE_FORMATS
    sprintf( msg, "-- Requesting back buffer format: %s", IGetDXFormatName( params.BackBufferFormat ) );
    hsDebugMessage( msg, 0 );
#endif

    params.hDeviceWindow = fDevice.fHWnd;

    // Enable this to switch to a pure device. 
//  fCurrentMode->fDDBehavior |= D3DCREATE_PUREDEVICE;
//  fCurrentMode->fDDBehavior |= D3DCREATE_DISABLE_DRIVER_MANAGEMENT;

#ifndef PLASMA_EXTERNAL_RELEASE
    UINT adapter;
    for (adapter = 0; adapter < d3d->GetAdapterCount(); adapter++)
    {
        D3DADAPTER_IDENTIFIER9 id;
        d3d->GetAdapterIdentifier(adapter, 0, &id);

        // We should be matching against "NVIDIA NVPerfHUD", but the space
        // in the description seems to be bogus. This seems to be a fair
        // alternative
        if (strstr(id.Description, "NVPerfHUD"))
        {
            // This won't actually use the REF device, but we ask for
            // it as part of the handshake to let NVPerfHUD know we give
            // it permission to analyze us.
            fCurrentAdapter = adapter;
            fCurrentDevice->fDDType= D3DDEVTYPE_REF;
            SetDebugFlag(plPipeDbg::kFlagNVPerfHUD, true);
            break;
        }
    }
#endif // PLASMA_EXTERNAL_RELEASE

    INIT_ERROR_CHECK( d3d->CreateDevice( fCurrentAdapter, fCurrentDevice->fDDType,
                                         fDevice.fHWnd, fCurrentMode->fDDBehavior,
                                         &params, &fD3DDevice ),
                        "Cannot create primary display surface via CreateDevice()" );

    fDevice.fD3DDevice = fD3DDevice;
    fSettings.fPresentParams = params;

    return false;
}

// IFindDepthFormat //////////////////////////////////////////////////////////////
// Look through available depth formats for the closest to what we want that
// will work.
bool plDXPipeline::IFindDepthFormat(D3DPRESENT_PARAMETERS& params)
{
    IDirect3D9* d3d = hsGDirect3D::GetDirect3D();

    // Okay, we're not using the stencil buffer right now, and it's bringing out
    // some painful driver bugs on the GeForce2. So rather than go out of our way
    // looking for trouble, we're going to look for a depth buffer with NO STENCIL.
    int i;
    for( i = fCurrentMode->fDepthFormats.GetCount() - 1; i >= 0; i-- )
    {
        D3DFORMAT fmt = fCurrentMode->fDepthFormats[ i ];
        if( (fmt == D3DFMT_D32)
            ||(fmt == D3DFMT_D24X8)
            ||(fmt == D3DFMT_D16) )
        {
            HRESULT hr = d3d->CheckDeviceMultiSampleType(fCurrentAdapter,
                                                         fCurrentDevice->fDDType,
                                                         fmt,
                                                         fCurrentMode->fWindowed ? TRUE : FALSE,
                                                         params.MultiSampleType, nullptr);
            if( !FAILED(hr) )
            {
                params.AutoDepthStencilFormat = fmt;
                fStencil.fDepth = 0;
                break;
            }
        }
    }
    if( i < 0 )
    {
        for( i = fCurrentMode->fDepthFormats.GetCount() - 1; i >= 0; i-- )
        {
            D3DFORMAT fmt = fCurrentMode->fDepthFormats[ i ];
            if( fmt == D3DFMT_D15S1 || fmt == D3DFMT_D24X4S4 || fmt == D3DFMT_D24S8 )
            {
                HRESULT hr = d3d->CheckDeviceMultiSampleType(fCurrentAdapter,
                                                             fCurrentDevice->fDDType,
                                                             fmt,
                                                             fCurrentMode->fWindowed ? TRUE : FALSE,
                                                             params.MultiSampleType, nullptr);
                if( !FAILED(hr) )
                {
                    params.AutoDepthStencilFormat = fmt;
                    if( fmt == D3DFMT_D15S1 )
                        fStencil.fDepth = 1;
                    else if( fmt == D3DFMT_D24X4S4 )
                        fStencil.fDepth = 4;
                    else
                        fStencil.fDepth = 8;
                    break;
                }
            }
        }
    }
    return i >= 0;
}

// ICreateNormalSurfaces //////////////////////////////////////////////////////
// Create the primary color and depth buffers.
//
bool plDXPipeline::ICreateNormalSurfaces()
{
    /// Now get the backbuffer surface pointer
    INIT_ERROR_CHECK( fD3DDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &fDevice.fD3DBackBuff ), 
                        "Cannot get primary surface's back buffer" );

    /// And finally, get the main D3D surfaces (for restoring after rendertargets )
    INIT_ERROR_CHECK( fD3DDevice->GetRenderTarget( 0, &fDevice.fD3DMainSurface ), "Cannot capture primary surface" );
    INIT_ERROR_CHECK( fD3DDevice->GetDepthStencilSurface( &fDevice.fD3DDepthSurface ), "Cannot capture primary depth surface" );

    fDevice.fCurrD3DMainSurface = fDevice.fD3DMainSurface;
    fDevice.fCurrD3DDepthSurface = fDevice.fD3DDepthSurface;

    D3DSURF_MEMNEW( fDevice.fD3DMainSurface );
    D3DSURF_MEMNEW( fDevice.fD3DDepthSurface );
    D3DSURF_MEMNEW( fDevice.fD3DBackBuff );

    D3DSURFACE_DESC info; 
    fDevice.fD3DMainSurface->GetDesc( &info );
    fDevice.fD3DDepthSurface->GetDesc( &info );
    fDevice.fD3DBackBuff->GetDesc( &info );

    return false;
}

// IReleaseRenderTargetPools //////////////////////////////////////////////////
// Free up all resources assosiated with our pools of rendertargets of varying
// sizes. Primary user of these pools is the shadow generation.
void plDXPipeline::IReleaseRenderTargetPools()
{
    int i;

    for( i = 0; i < fRenderTargetPool512.GetCount(); i++ )
    {
        delete fRenderTargetPool512[i];
        fRenderTargetPool512[i] = nullptr;
    }
    fRenderTargetPool512.SetCount(0);

    for( i = 0; i < fRenderTargetPool256.GetCount(); i++ )
    {
        delete fRenderTargetPool256[i];
        fRenderTargetPool256[i] = nullptr;
    }
    fRenderTargetPool256.SetCount(0);

    for( i = 0; i < fRenderTargetPool128.GetCount(); i++ )
    {
        delete fRenderTargetPool128[i];
        fRenderTargetPool128[i] = nullptr;
    }
    fRenderTargetPool128.SetCount(0);

    for( i = 0; i < fRenderTargetPool64.GetCount(); i++ )
    {
        delete fRenderTargetPool64[i];
        fRenderTargetPool64[i] = nullptr;
    }
    fRenderTargetPool64.SetCount(0);

    for( i = 0; i < fRenderTargetPool32.GetCount(); i++ )
    {
        delete fRenderTargetPool32[i];
        fRenderTargetPool32[i] = nullptr;
    }
    fRenderTargetPool32.SetCount(0);

    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        fRenderTargetNext[i] = 0;
        fBlurScratchRTs[i] = nullptr;
        fBlurDestRTs[i] = nullptr;
    }

#ifdef MF_ENABLE_HACKOFF
    hackOffscreens.Reset();
#endif // MF_ENABLE_HACKOFF
}

// IReleaseDynDeviceObjects //////////////////////////////////////////////
// Make sure we aren't holding on to anything, and release all of
// the D3D resources that we normally hang on to forever. Meaning things
// that persist through unloading one age and loading the next.
void plDXPipeline::IReleaseDynDeviceObjects()
{
    // We should do this earlier, but the textFont objects don't remove
    // themselves from their parent objects yet
    delete fDebugTextMgr;
    fDebugTextMgr = nullptr;

    if( fD3DDevice )
    {
        fD3DDevice->SetStreamSource(0, nullptr, 0, 0);
        fD3DDevice->SetIndices(nullptr);
    }

    /// Delete actual d3d objects
    hsRefCnt_SafeUnRef( fSettings.fCurrVertexBuffRef );
    fSettings.fCurrVertexBuffRef = nullptr;
    hsRefCnt_SafeUnRef( fSettings.fCurrIndexBuffRef );
    fSettings.fCurrIndexBuffRef = nullptr;

    while( fTextFontRefList )
        delete fTextFontRefList;

    while( fRenderTargetRefList )
    {
        plDXRenderTargetRef* rtRef = fRenderTargetRefList;
        rtRef->Release();
        rtRef->Unlink();
    }

    // The shared dynamic vertex buffers used by things like objects skinned on CPU, or
    // particle systems.
    IReleaseDynamicBuffers();
    IReleaseAvRTPool();
    IReleaseRenderTargetPools();

    if( fSharedDepthSurface[0] )
    {
        D3DSURF_MEMDEL(fSharedDepthSurface[0]);
        ReleaseObject(fSharedDepthSurface[0]);
        fSharedDepthFormat[0] = D3DFMT_UNKNOWN;
    }
    if( fSharedDepthSurface[1] )
    {
        D3DSURF_MEMDEL(fSharedDepthSurface[1]);
        ReleaseObject(fSharedDepthSurface[1]);
        fSharedDepthFormat[1] = D3DFMT_UNKNOWN;
    }

    D3DSURF_MEMDEL( fDevice.fD3DMainSurface );
    D3DSURF_MEMDEL( fDevice.fD3DDepthSurface );
    D3DSURF_MEMDEL( fDevice.fD3DBackBuff );

    ReleaseObject( fDevice.fD3DBackBuff );
    ReleaseObject( fDevice.fD3DDepthSurface );
    ReleaseObject( fDevice.fD3DMainSurface );

}

// IReleaseShaders ///////////////////////////////////////////////////////////////
// Delete our vertex and pixel shaders. Releasing the plasma ref will release the
// D3D handle. 
void plDXPipeline::IReleaseShaders()
{
    while( fVShaderRefList )
    {
        plDXVertexShader* ref = fVShaderRefList;
        ref->Release();
        ref->Unlink();
    }

    while( fPShaderRefList )
    {
        plDXPixelShader* ref = fPShaderRefList;
        ref->Release();
        ref->Unlink();
    }
}

//// IReleaseDeviceObjects ///////////////////////////////////////////////////////
// Release everything we've created. This is the main cleanup function.
void    plDXPipeline::IReleaseDeviceObjects()
{
    plDXDeviceRef   *ref;

    /// Delete d3d-dependent objects
#if MCN_BOUNDS_SPANS
    if( fBoundsSpans )
        fBoundsSpans->GetKey()->UnRefObject();
    fBoundsSpans = nullptr;
    if( fBoundsMat )
        fBoundsMat->GetKey()->UnRefObject();
    fBoundsMat = nullptr;
#endif

    plStatusLogMgr::GetInstance().SetDrawer(nullptr);
    delete fLogDrawer;
    fLogDrawer = nullptr;

    IGetPixelScratch( 0 );  

    int i;
    for( i = 0; i < 8; i++ )
    {
        if( fLayerRef[i] )
        {
            hsRefCnt_SafeUnRef(fLayerRef[i]);
            fLayerRef[i] = nullptr;
        }
    }

#ifdef MF_ENABLE_HACKOFF
    //WHITE
    hackOffscreens.SetCount(0);
#endif // MF_ENABLE_HACKOFF

    if( fULutTextureRef )
        delete [] fULutTextureRef->fData;
    hsRefCnt_SafeUnRef(fULutTextureRef);
    fULutTextureRef = nullptr;

    while( fVtxBuffRefList )
    {
        ref = fVtxBuffRefList;
        ref->Release();
        ref->Unlink();
    }
    while( fIdxBuffRefList )
    {
        ref = fIdxBuffRefList;
        ref->Release();
        ref->Unlink();
    }
    while( fTextureRefList )
    {
        ref = fTextureRefList;
        ref->Release();
        ref->Unlink();
    }

    IReleaseShaders();

    fLights.Release();

    IReleaseDynDeviceObjects();

    delete fPlateMgr;
    fPlateMgr = nullptr;

    if (fD3DDevice != nullptr)
    {
        LONG ret;
        while( ret = fD3DDevice->Release() )
        {
            hsStatusMessageF("%d - Error releasing device", ret);
        }
        fD3DDevice = nullptr;
    }

    fManagedAlloced = false;
    fAllocUnManaged = false;
}

// IReleaseDynamicBuffers /////////////////////////////////////////////////
// Release everything we've created in POOL_DEFAULT.
// This is called on shutdown or when we lose the device. Search for D3DERR_DEVICELOST.
void plDXPipeline::IReleaseDynamicBuffers()
{
    // Workaround for ATI driver bug.
    if( fSettings.fBadManaged )
    {
        plDXTextureRef* tRef = fTextureRefList;
        while( tRef )
        {
            tRef->Release();
            tRef = tRef->GetNext();
        }
    }
    plDXVertexBufferRef* vbRef = fVtxBuffRefList;
    while( vbRef )
    {
        if( vbRef->Volatile() && vbRef->fD3DBuffer )
        {
            vbRef->fD3DBuffer->Release();
            vbRef->fD3DBuffer = nullptr;

            // Actually, if it's volatile, it's sharing the global dynamic vertex buff, so we're already
            // accounting for the memory when we clear the global buffer.
            //PROFILE_POOL_MEM(D3DPOOL_DEFAULT, vbRef->fCount * vbRef->fVertexSize, false, "VtxBuff");
        }
        // 9600 THRASH
        else if( fSettings.fBadManaged )
        {
            vbRef->Release();
        }
        vbRef = vbRef->GetNext();
    }
    plDXIndexBufferRef* iRef = fIdxBuffRefList;
    while( iRef )
    {
        // If it's volatile, we have to release it.
        // If it's not, we want to release it so
        // we can make it volatile (D3DPOOL_DEFAULT)
        if (iRef->fD3DBuffer)
        {
            iRef->fD3DBuffer->Release();
            iRef->fD3DBuffer = nullptr;
            PROFILE_POOL_MEM(iRef->fPoolType, iRef->fCount * sizeof(uint16_t), false, "IndexBuff");
        }
        iRef = iRef->GetNext();
    }
    if (fDynVtxBuff)
    {
        ReleaseObject(fDynVtxBuff);
        PROFILE_POOL_MEM(D3DPOOL_DEFAULT, fDynVtxSize, false, "DynVtxBuff");
        fDynVtxBuff = nullptr;
    }

    fNextDynVtx = 0;

    fVtxRefTime++;

    // PlateMgr has a POOL_DEFAULT vertex buffer for drawing quads.
    if( fPlateMgr )
        fPlateMgr->IReleaseGeometry();

    // Also has POOL_DEFAULT vertex buffer.
    plDXTextFont::ReleaseShared(fD3DDevice);

    IReleaseBlurVBuffers();
}

// ICreateDynamicBuffers /////////////////////////////////////////////////////
// Create the things we need in POOL_DEFAULT. We clump them into this function,
// because they must be created before anything in POOL_MANAGED.
// So we create these global POOL_DEFAULT objects here, then send out a message
// to the objects in the scene to create anything they need in POOL_DEFAULT,
// then go on to create things on POOL_MANAGED.
// Set LoadResources().
void plDXPipeline::ICreateDynamicBuffers()
{
    ICreateBlurVBuffers();

    plDXTextFont::CreateShared(fD3DDevice);

    if( fPlateMgr )
        fPlateMgr->ICreateGeometry(this);

    fNextDynVtx = 0;

    fVtxRefTime++;

    DWORD usage = D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC;
    hsAssert(!fManagedAlloced, "Alloc default with managed alloc'd");
    D3DPOOL poolType = D3DPOOL_DEFAULT;
    if( fDynVtxSize )
    {
        PROFILE_POOL_MEM(poolType, fDynVtxSize, true, "DynVtxBuff");
        if( FAILED( fD3DDevice->CreateVertexBuffer( fDynVtxSize,
                                                    usage, 
                                                    0,
                                                    poolType, 
                                                    &fDynVtxBuff, nullptr)))
        {
            hsAssert(false, "Don't know what to do here.");
        }
    }
}


void plDXPipeline::IPrintDeviceInitError()
{
    char str[256];
    char err[16];
    switch(plLocalization::GetLanguage())
    {
        case plLocalization::kFrench:   strcpy(err, "Erreur"); strcpy(str, "Erreur d'initialisation de votre carte graphique. Les valeurs par défaut de ses paramètres ont été rétablis. ");    break;
        case plLocalization::kGerman:   strcpy(err, "Fehler");  strcpy(str, "Bei der Initialisierung Ihrer Grafikkarte ist ein Fehler aufgetreten. Standardeinstellungen werden wiederhergestellt."); break;
        case plLocalization::kSpanish:  strcpy(err, "Error"); strcpy(str, "Ocurrió un error al inicializar tu tarjeta de vídeo. Hemos restaurado los ajustes por defecto. "); break;
        case plLocalization::kItalian:  strcpy(err, "Errore");  strcpy(str, "Errore di inizializzazione della scheda video. Sono state ripristinate le impostazioni predefinite."); break;
        default:                        strcpy(err, "Error"); strcpy(str, "There was an error initializing your video card. We have reset it to its Default settings."); break;
    }
    hsMessageBox(str, err, hsMessageBoxNormal, hsMessageBoxIconError);
}

// Reset device creation parameters to default and write to ini file
void plDXPipeline::IResetToDefaults(D3DPRESENT_PARAMETERS *params)
{
    // this will reset device parameters to default and make sure all other necessary parameters are updated
    params->BackBufferWidth = fDefaultPipeParams.Width;
    params->BackBufferHeight = fDefaultPipeParams.Height;
    fOrigWidth = fDefaultPipeParams.Width;
    fOrigHeight = fDefaultPipeParams.Height;
    IGetViewTransform().SetScreenSize(fDefaultPipeParams.Width, fDefaultPipeParams.Height);
    params->BackBufferFormat = D3DFMT_X8R8G8B8;
    fColorDepth = fDefaultPipeParams.ColorDepth;

    int i;
    hsTArray<D3DEnum_ModeInfo> *modes = &fCurrentDevice->fModes;
    for( i = 0; i < modes->Count(); i++ )
    {
        D3DEnum_ModeInfo *mode = &(*modes)[i];
        if(mode->fDDmode.Width == params->BackBufferWidth &&
            mode->fDDmode.Height == params->BackBufferHeight &&
            mode->fBitDepth == 32 )
        {
            ISetCurrentMode(&(*modes)[i]);
            break;
        }
    }
    params->Windowed = fDefaultPipeParams.Windowed;
    fSettings.fFullscreen = !fDefaultPipeParams.Windowed;
    fCurrentMode->fWindowed = fDefaultPipeParams.Windowed;

     // Attempt to find the closest AA setting we can
    params->MultiSampleType = D3DMULTISAMPLE_NONE;
    fSettings.fNumAASamples = 0;
    for( int i = fDefaultPipeParams.AntiAliasingAmount; i >= 2; i-- )
    {
        if( fCurrentMode->fFSAATypes.Find( (D3DMULTISAMPLE_TYPE)i ) != fCurrentMode->fFSAATypes.kMissingIndex )
        {
            fSettings.fNumAASamples = i;
            params->MultiSampleType = (D3DMULTISAMPLE_TYPE)i;
            break;
        }
    }
    fSettings.fMaxAnisotropicSamples = fDefaultPipeParams.AnisotropicLevel;

    fVSync = false;

    params->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

    plShadowCaster::EnableShadowCast(fDefaultPipeParams.Shadows ? true : false);
    plQuality::SetQuality(fDefaultPipeParams.VideoQuality);
    plQuality::SetCapability(fDefaultPipeParams.VideoQuality);
    plDynamicCamMap::SetEnabled(fDefaultPipeParams.PlanarReflections ? true : false);
    plBitmap::SetGlobalLevelChopCount(2 - fDefaultPipeParams.TextureQuality);

    // adjust camera properties
    plVirtualCam1::Refresh();

    // fire off a message to the client so we can write defaults to the ini file, and adjust the window size
    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
    plClientMsg* clientMsg = new plClientMsg(plClientMsg::kSetGraphicsDefaults);
    clientMsg->Send(clientKey);

}

// IResetDevice
// reset the device to its operational state.
// returns true if not ready yet, false if the reset was successful.
// All this is generally in response to a fullscreen alt-tab.
bool plDXPipeline::IResetDevice()
{
    bool fakeDevLost(false);
    if( fakeDevLost )
        fDeviceLost = true;

    if( fDeviceLost )
    {
        IClearShadowSlaves();

        Sleep(100);
        HRESULT coopLev = fD3DDevice->TestCooperativeLevel();
        if( coopLev == D3DERR_DEVICELOST )
        {
            // Nothing to do yet.
            return true;
        }
        if( fakeDevLost )
            coopLev = D3DERR_DEVICENOTRESET;
        if( coopLev == D3DERR_DEVICENOTRESET || fForceDeviceReset)
        {
            plStatusLog::AddLineS("pipeline.log", 0xffff0000, "Resetting Device");
            IReleaseDynDeviceObjects();
            if( !IFindDepthFormat(fSettings.fPresentParams) )
            {
                // If we haven't found a depth format, turn off multisampling and try it again.
                fSettings.fPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
                IFindDepthFormat(fSettings.fPresentParams);
            }
            HRESULT hr = fD3DDevice->Reset(&fSettings.fPresentParams);
            // The device is inited the first time on the client loader thread, but this is the main thread
            // we expect to get one failure... So let's try recreating the device on the main thread.
            if (FAILED(hr)) {
                IReleaseDeviceObjects();
                for (int i = 0; true; ++i) {
                    if (!ICreateDeviceObjects())
                        break;
                    ::Sleep(250);
                    // Old magic number from reset land
                    if (i == 25) {
                        IPrintDeviceInitError();
                        IResetToDefaults(&fSettings.fPresentParams);
                    }
                }
            }
            fSettings.fCurrFVFFormat = 0;
            fSettings.fCurrVertexShader = nullptr;
            fManagedAlloced = false;
            ICreateDynDeviceObjects();
            IInitDeviceState();

            /// Broadcast a message letting everyone know that we were recreated and that
            /// all device-specific stuff needs to be recreated
            plDeviceRecreateMsg* clean = new plDeviceRecreateMsg(this);
            clean->Send();
        }
        fDevWasLost = true;
        fDeviceLost = false;

        // We return true here, even though we've successfully recreated, to take
        // another spin around the update loop and give everyone a chance to
        // get back in sync.
        return true;
    }
    return false;
}

void plDXPipeline::ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync /* = false */)
{
    if( fSettings.fPresentParams.BackBufferWidth == Width &&
        fSettings.fPresentParams.BackBufferHeight == Height &&
        (fSettings.fPresentParams.Windowed ? 1 : fColorDepth == ColorDepth) && // if we're windowed dont check color depth we just use the desktop colordepth
        ((fSettings.fPresentParams.Windowed && Windowed)  || (!fSettings.fPresentParams.Windowed && !Windowed)) &&
        fSettings.fNumAASamples == NumAASamples &&
        fSettings.fMaxAnisotropicSamples == MaxAnisotropicSamples &&
        fVSync == VSync 
    )
    {
        return;     // nothing has changed
    }

    fVSync = VSync;
    int i = 0;
    hsTArray<D3DEnum_ModeInfo> *modes = &fCurrentDevice->fModes;
    // check for supported resolution if we're not going to windowed mode
    if(!Windowed)
    {
        for( i = 0; i < modes->Count(); i++ )
        {
            D3DEnum_ModeInfo *mode = &(*modes)[i];
            if(mode->fDDmode.Width == Width &&
                mode->fDDmode.Height == Height &&
                mode->fBitDepth == ColorDepth )
            {
                ISetCurrentMode(&(*modes)[i]);
                break;
            }
        }
    }
    if(i != modes->Count())
    {
        // Set Resolution
        fOrigWidth = Width;
        fOrigHeight = Height;
        IGetViewTransform().SetScreenSize(Width, Height);
        fSettings.fPresentParams.BackBufferWidth = Width;
        fSettings.fPresentParams.BackBufferHeight = Height;
        fColorDepth = ColorDepth;
        fSettings.fPresentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
    }

    // set windowed/fullscreen mode
    fCurrentMode->fWindowed = Windowed;
    fSettings.fPresentParams.Windowed = TRUE;
    fSettings.fFullscreen = !Windowed;

    // set Antialiasing
    fSettings.fNumAASamples = 0;
    // Attempt to find the closest AA setting we can
    fSettings.fPresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
    for( i = NumAASamples; i >= 2; i-- )
    {
        if( fCurrentMode->fFSAATypes.Find( (D3DMULTISAMPLE_TYPE)i ) != fCurrentMode->fFSAATypes.kMissingIndex )
        {
            fSettings.fNumAASamples = i;
            fSettings.fPresentParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)i;
            break;
        }
    }
    if( fSettings.fNumAASamples > 0 )
        fSettings.fD3DCaps |= kCapsFSAntiAlias;
    else
        fSettings.fD3DCaps &= ~kCapsFSAntiAlias;

    // Set Anisotropic filtering
    fSettings.fMaxAnisotropicSamples = MaxAnisotropicSamples;
    ISetAnisotropy(MaxAnisotropicSamples > 0);
    if(Windowed)
    {
        fSettings.fPresentParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    }
    else
    {
        fSettings.fPresentParams.PresentationInterval = ( fVSync ? D3DPRESENT_INTERVAL_DEFAULT : D3DPRESENT_INTERVAL_IMMEDIATE );
    }

    // Force a device reset
    fDeviceLost = true;
    fForceDeviceReset = true;

    plVirtualCam1::Refresh();
    IResetDevice();


    return;
}

void plDXPipeline::GetSupportedColorDepths(hsTArray<int> &ColorDepths)
{
    int i, j;
    // iterate through display modes
    for( i = 0; i < fCurrentDevice->fModes.Count(); i++ )
    {
        // Check to see if color depth has been added already
        for( j = 0; j < ColorDepths.Count(); j++ )
        {
            if( fCurrentDevice->fModes[i].fBitDepth == ColorDepths[i] )
                break;
        }
        if(j == ColorDepths.Count())
        {
            //add it
            ColorDepths.Push( fCurrentDevice->fModes[i].fBitDepth );
        }
    }
}

void plDXPipeline::GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth  )
{
    int i, j;
    std::vector<plDisplayMode> supported;
    // loop through display modes
    for( i = 0; i < fCurrentDevice->fModes.Count(); i++ )
    {
        if( fCurrentDevice->fModes[i].fBitDepth == ColorDepth )
        {
            // check for duplicate mode
            for( j = 0; j < supported.size(); j++ )
            {
                if(supported[j].Width == fCurrentDevice->fModes[i].fDDmode.Width && supported[j].Height == fCurrentDevice->fModes[i].fDDmode.Height)
                    break;
            }
            if(j == supported.size())
            {
                // new mode, add it
                plDisplayMode mode;
                mode.Width = fCurrentDevice->fModes[i].fDDmode.Width;
                mode.Height = fCurrentDevice->fModes[i].fDDmode.Height;
                mode.ColorDepth = ColorDepth;
                supported.push_back(mode);
            }
        }
    }

    *res = supported;
}

// Get max anitialias for the specified displaymode
int plDXPipeline::GetMaxAntiAlias(int Width, int Height, int ColorDepth)
{
    int max = 0;
    D3DEnum_ModeInfo *pCurrMode = nullptr;
    hsTArray<D3DEnum_ModeInfo> *modes = &fCurrentDevice->fModes;
    for(int i = 0; i < modes->Count(); i++ )
    {
        D3DEnum_ModeInfo *mode = &(*modes)[i];
        if( mode->fDDmode.Width == Width &&
            mode->fDDmode.Height == Height &&
            mode->fBitDepth == ColorDepth )
        {
            pCurrMode = mode;
        }
    }
    if(pCurrMode)
    {
        for(int i = 0; i < pCurrMode->fFSAATypes.Count(); i++)
        {
            if(pCurrMode->fFSAATypes[i] > max)
                max = pCurrMode->fFSAATypes[i];
        }
    }
    return max;
}

int plDXPipeline::GetMaxAnisotropicSamples()
{
    return fCurrentDevice ? fCurrentDevice->fDDCaps.MaxAnisotropy : 0;
}

//// Resize ///////////////////////////////////////////////////////////////////
// Resize is fairly obsolete, having been replaced by IResetDevice, which is
// automatically called if needed on BeginRender.
// This Resize function used to serve as both to Resize the primary buffers and
// to restore after losing the device (alt-tab). It didn't actually do either
// very well, so I'm not sure why I haven't deleted it.
void    plDXPipeline::Resize( uint32_t width, uint32_t height )
{
    hsMatrix44  w2c, c2w, proj;

    
    HRESULT coopLev = fD3DDevice->TestCooperativeLevel();
    if( coopLev == D3DERR_DEVICELOST )
    {
        /// Direct3D is reporting that we lost the device but are unable to reset
        /// it yet, so ignore.
        hsStatusMessage( "Received Resize() request at an invalid time. Ignoring...\n" );
        return;
    }
    if( !width && !height )
    {
        if( D3D_OK == coopLev )
            return;

        IReleaseDynDeviceObjects();
        HRESULT hr = fD3DDevice->Reset(&fSettings.fPresentParams);
        fManagedAlloced = false;
        if( !FAILED(hr) )
        {
            ICreateDynDeviceObjects();
            IInitDeviceState();
            return;
        }
    }

    // Store some states that we *want* to restore back...
    plViewTransform resetTransform = GetViewTransform();

    /// HACK: Don't recreate if we're windowed, bad things happen
    /// Comment out this if if you want to test the crashing thing in windowed alt-tabbing
#if 0
    if( ( width == 0 || height == 0 ) && !fSettings.fFullscreen )
        return;
#endif

    // Destroy old
    IReleaseDeviceObjects();

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

    // Recreate
    if( hsGDirect3D::GetDirect3D(true) )
    {
        IShowErrorMessage( "Cannot create D3D master object" );
        return;
    }

    // Go recreate surfaces and DX-dependent objects
    if( ICreateDeviceObjects() )
    {
        IShowErrorMessage( "Cannot create Direct3D device" );
        return;
    }

    // Restore states
    SetViewTransform(resetTransform);
    IProjectionMatrixToDevice();

    /// Broadcast a message letting everyone know that we were recreated and that
    /// all device-specific stuff needs to be recreated
    plDeviceRecreateMsg* clean = new plDeviceRecreateMsg(this);
    clean->Send();
}


///////////////////////////////////////////////////////////////////////////////
//// Debug Text ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// MakeTextFont /////////////////////////////////////////////////////////////

plTextFont  *plDXPipeline::MakeTextFont( char *face, uint16_t size )
{
    plTextFont  *font;


    font = new plDXTextFont( this, fD3DDevice );
    if (font == nullptr)
        return nullptr;
    font->Create( face, size );
    font->Link( &fTextFontRefList );

    return font;
}


///////////////////////////////////////////////////////////////////////////////
//// Drawable Stuff ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// Draw /////////////////////////////////////////////////////////////////////

// PreRender //////////////////////////////////////////////////////////////////
// Most of this is debugging stuff, drawing the bounds, drawing the normals, etc.
// The functional part is in IGetVisibleSpans, which creates a list of the visible (non-culled)
// span indices within this drawable.
// This is called once per render, and generally well before rendering begins (as part of the 
// cull phase).
bool  plDXPipeline::PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(drawable);
    if( !ds )
        return false;
    if( ( ds->GetType() & fView.GetDrawableTypeMask() ) == 0 )
        return false;

    fView.GetVisibleSpans( ds, visList, visMgr );

#if MCN_BOUNDS_SPANS
    if( ( drawable != fBoundsSpans ) && IsDebugFlagSet(plPipeDbg::kFlagShowAllBounds) )
    {
        const hsTArray<plSpan *>    &spans = ds->GetSpanArray();
        for (int16_t idx : visList)
        {
            /// Add a span to our boundsIce to show this
            IAddBoundsSpan(fBoundsSpans, &spans[idx]->fWorldBounds);
        }
    }
    else if( ( drawable != fBoundsSpans ) && IsDebugFlagSet(plPipeDbg::kFlagShowNormals) )
    {
        const hsTArray<plSpan *>    &spans = ds->GetSpanArray();
        for (int16_t idx : visList)
        {
            /// Add a span to our boundsIce to show this
            plIcicle    *span = (plIcicle *)spans[idx];
            if( span->fTypeMask & plSpan::kIcicleSpan )
            {
                IAddNormalsSpan( fBoundsSpans, span, (plDXVertexBufferRef *)ds->GetVertexRef( span->fGroupIdx, span->fVBufferIdx ), 0xff0000ff );
            }
        }
    }
#endif
#if MF_BOUNDS_LEVEL_ICE
    if( (fSettings.fBoundsDrawLevel >= 0) && ( drawable != fBoundsSpans ) )
    {
        std::vector<int16_t> bndList;
        drawable->GetSpaceTree()->HarvestLevel(fSettings.fBoundsDrawLevel, bndList);
        for (int16_t bnd : bndList)
        {
            const hsBounds3Ext& nodeBounds = drawable->GetSpaceTree()->GetNode(bnd).GetWorldBounds();
            IAddBoundsSpan( fBoundsSpans, &nodeBounds, 0xff000000 | (0xf << ((fSettings.fBoundsDrawLevel % 6) << 2)) );
        }
    }
#endif // MF_BOUNDS_LEVEL_ICE


    return !visList.empty();
}

struct plSortFace
{
    uint16_t      fIdx[3];
    float    fDist;
};

struct plCompSortFace
{
    bool operator()( const plSortFace& lhs, const plSortFace& rhs) const
    {
        return lhs.fDist > rhs.fDist;
    }
};

// IAvatarSort /////////////////////////////////////////////////////////////////////////
// We handle avatar sort differently from the rest of the face sort. The reason is that
// within the single avatar index buffer, we want to only sort the faces of spans requesting
// a sort, and sort them in place.
// Contrast that with the normal scene translucency sort. There, we sort all the spans in a drawble,
// then we sort all the faces in that drawable, then for each span in the sorted span list, we extract
// the faces for that span appending onto the index buffer. This gives great efficiency because
// only the visible faces are sorted and they wind up packed into the front of the index buffer, which
// permits more batching. See plDrawableSpans::SortVisibleSpans.
// For the avatar, it's generally the case that all the avatar is visible or not, and there is only
// one material, so neither of those efficiencies is helpful. Moreover, for the avatar the faces we
// want sorted are a tiny subset of the avatar's faces. Moreover, and most importantly, for the avatar, we
// want to preserve the order that spans are drawn, so, for example, the opaque base head will always be
// drawn before the translucent hair fringe, which will always be drawn before the pink clear plastic baseball cap.
bool plDXPipeline::IAvatarSort(plDrawableSpans* d, const std::vector<int16_t>& visList)
{
    plProfile_BeginTiming(AvatarSort);
    for (int16_t visIdx : visList)
    {
        hsAssert(d->GetSpan(visIdx)->fTypeMask & plSpan::kIcicleSpan, "Unknown type for sorting faces");

        plIcicle* span = (plIcicle*)d->GetSpan(visIdx);

        if( span->fProps & plSpan::kPartialSort )
        {
            hsAssert(d->GetBufferGroup(span->fGroupIdx)->AreIdxVolatile(), "Badly setup buffer group - set PartialSort too late?");

            const hsPoint3 viewPos = GetViewPositionWorld();

            plGBufferGroup* group = d->GetBufferGroup(span->fGroupIdx);

            plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx); 

            const uint8_t* vdata = vRef->fData;
            const uint32_t stride = vRef->fVertexSize;

            const int numTris = span->fILength/3;
            
            static std::vector<plSortFace> sortScratch;
            sortScratch.resize(numTris);

            plProfile_IncCount(AvatarFaces, numTris);

            // 
            // Have three very similar sorts here, differing only on where the "position" of
            // each triangle is defined, either as the center of the triangle, the nearest
            // point on the triangle, or the farthest point on the triangle.
            // Having tried all three on the avatar (the only thing this sort is used on),
            // the best results surprisingly came from using the center of the triangle.
            uint16_t* indices = group->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;
            int j;
            for( j = 0; j < numTris; j++ )
            {
#if 1 // TRICENTER
                uint16_t idx = *indices++;
                sortScratch[j].fIdx[0] = idx;
                hsPoint3 pos = *(hsPoint3*)(vdata + idx * stride);

                idx = *indices++;
                sortScratch[j].fIdx[1] = idx;
                pos += *(hsPoint3*)(vdata + idx * stride);

                idx = *indices++;
                sortScratch[j].fIdx[2] = idx;
                pos += *(hsPoint3*)(vdata + idx * stride);

                pos *= 0.3333f;

                sortScratch[j].fDist = hsVector3(&pos, &viewPos).MagnitudeSquared();
#elif 0 // NEAREST
                uint16_t idx = *indices++;
                sortScratch[j].fIdx[0] = idx;
                hsPoint3 pos = *(hsPoint3*)(vdata + idx * stride);
                float dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                float minDist = dist;

                idx = *indices++;
                sortScratch[j].fIdx[1] = idx;
                pos = *(hsPoint3*)(vdata + idx * stride);
                dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                if( dist < minDist )
                    minDist = dist;

                idx = *indices++;
                sortScratch[j].fIdx[2] = idx;
                pos = *(hsPoint3*)(vdata + idx * stride);
                dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                if( dist < minDist )
                    minDist = dist;

                sortScratch[j].fDist = minDist;
#elif 1 // FURTHEST
                uint16_t idx = *indices++;
                sortScratch[j].fIdx[0] = idx;
                hsPoint3 pos = *(hsPoint3*)(vdata + idx * stride);
                float dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                float maxDist = dist;

                idx = *indices++;
                sortScratch[j].fIdx[1] = idx;
                pos = *(hsPoint3*)(vdata + idx * stride);
                dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                if( dist > maxDist )
                    maxDist = dist;

                idx = *indices++;
                sortScratch[j].fIdx[2] = idx;
                pos = *(hsPoint3*)(vdata + idx * stride);
                dist = hsVector3(&pos, &viewPos).MagnitudeSquared();
                if( dist > maxDist )
                    maxDist = dist;

                sortScratch[j].fDist = maxDist;
#endif // SORTTYPES
            }

            std::sort(sortScratch.begin(), sortScratch.end(), plCompSortFace());

            indices = group->GetIndexBufferData(span->fIBufferIdx) + span->fIStartIdx;
            for (const plSortFace& iter : sortScratch)
            {
                *indices++ = iter.fIdx[0];
                *indices++ = iter.fIdx[1];
                *indices++ = iter.fIdx[2];
            }

            group->DirtyIndexBuffer(span->fIBufferIdx);
        }
    }
    plProfile_EndTiming(AvatarSort);
    return true;
}

// PrepForRender //////////////////////////////////////////////////////////////////
// Make sure the given drawable and each of the spans to be drawn (as noted in the
// indices in visList) is ready to be rendered.
// This means:
// a) select which lights will be used for each span
// b) do any necessary sorting (if required, spans are already in sorted order in visList,
//      so this only means face sorting).
// c) do any necessary software skinning.
// This is called once per render, and before any rendering actually starts. See plPageTreeMgr.cpp.
// So any preperation needs to last until rendering actually begins. So cached information, like
// which lights a span will use, needs to be stored on the span.
bool plDXPipeline::PrepForRender(plDrawable* d, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    plProfile_BeginTiming(PrepDrawable);

    plDrawableSpans *drawable = plDrawableSpans::ConvertNoRef(d);
    if( !drawable )
    {
        plProfile_EndTiming(PrepDrawable);
        return false;
    }

    // Find our lights
    ICheckLighting(drawable, visList, visMgr);

    // Sort our faces
    if( drawable->GetNativeProperty(plDrawable::kPropSortFaces) )
    {
        drawable->SortVisibleSpans(visList, this);
    }

    // Prep for render. This is gives the drawable a chance to
    // do any last minute updates for its buffers, including
    // generating particle tri lists.
    drawable->PrepForRender( this );

    // Any skinning necessary
    if( !ISoftwareVertexBlend(drawable, visList) )
    {
        plProfile_EndTiming(PrepDrawable);
        return false;
    }
    // Avatar face sorting happens after the software skin.
    if( drawable->GetNativeProperty(plDrawable::kPropPartialSort) )
    {
        IAvatarSort(drawable, visList);
    }

    plProfile_EndTiming(PrepDrawable);

    return true;
}

// ISetupTransforms //////////////////////////////////////////////////////////////////////////////////
// Set the D3D world transform according to the input span.
// Engine currently supports HW vertex blending with 2 matrices,
// else a single Local To World.
// If software skinning is being used, the WORLD matrix will be identity,
// because the full local to world is folded into the skinned vertices.
void plDXPipeline::ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W)
{
    if( span.fNumMatrices )
    {
        if( span.fNumMatrices <= 2 )
        {
            ISetLocalToWorld( span.fLocalToWorld, span.fWorldToLocal );
            lastL2W = span.fLocalToWorld;
        }
        else
        {
            lastL2W.Reset();
            ISetLocalToWorld( lastL2W, lastL2W );
            fView.fLocalToWorldLeftHanded = span.fLocalToWorld.GetParity();
        }
    }
    else
    if( lastL2W != span.fLocalToWorld )
    {
        ISetLocalToWorld( span.fLocalToWorld, span.fWorldToLocal );
        lastL2W = span.fLocalToWorld;
    }
    else
    {
        fView.fLocalToWorldLeftHanded = lastL2W.GetParity();
    }

    if( span.fNumMatrices == 2 )
    {
        D3DMATRIX  mat;
        IMatrix44ToD3DMatrix(mat, drawable->GetPaletteMatrix(span.fBaseMatrix+1));
        fD3DDevice->SetTransform(D3DTS_WORLDMATRIX(1), &mat);
        fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_1WEIGHTS);
    }
    else
    {
        fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    }
}

// IRefreshDynVertices ////////////////////////////////////////////////////////////////////////
// All dynamic vertices share a single dynamic vertex buffer. They are cycled through
// that buffer using the NOOVERWRITE/DISCARD paradigm. Since the vertices sharing that
// buffer may be of different formats, care is taken to always start a group of vertices
// a the next available position in the buffer aligned with that vertex size.
// Only software skinned objects, dynamic decals, and particle systems currently use the 
// dynamic vertex buffer.
bool plDXPipeline::IRefreshDynVertices(plGBufferGroup* group, plDXVertexBufferRef* vRef)
{
    // First, pad out our next slot to be on a vertex boundary (for this vertex size).
    fNextDynVtx = ((fNextDynVtx + vRef->fVertexSize-1) / vRef->fVertexSize) * vRef->fVertexSize;

    int32_t size = (group->GetVertBufferEnd(vRef->fIndex) - group->GetVertBufferStart(vRef->fIndex)) * vRef->fVertexSize;
    if( !size )
        return false; // No error, just nothing to do.

    hsAssert(size > 0, "Bad start and end counts in a group");

    // If we DON'T have room in our dynamic buffer
    if( fNextDynVtx + size > fDynVtxSize )
    {
        plProfile_IncCount(DynVBuffs, 1);

        // Advance the timestamp, because we're about to reuse the buffer
        fVtxRefTime++;

        // Reset next available spot index to zero
        fNextDynVtx = 0;

    }
    // Point our ref at the next available spot
    int32_t newStart = fNextDynVtx / vRef->fVertexSize;

    vRef->fOffset = newStart - group->GetVertBufferStart(vRef->fIndex);

    // Lock the buffer
    // If index is zero, lock with discard, else with overwrite.
    DWORD lockFlag = fNextDynVtx ? D3DLOCK_NOOVERWRITE : D3DLOCK_DISCARD;
    uint8_t* destPtr = nullptr;
    if( FAILED( fDynVtxBuff->Lock( fNextDynVtx, 
                                size, 
                                (void **)&destPtr, 
                                lockFlag) ) )
    {
        hsAssert( false, "Cannot lock vertex buffer for writing" );
        return true;
    }

    uint8_t* vData;
    if( vRef->fData )
    {
        vData = vRef->fData;
    }
    else
    {
        vData = group->GetVertBufferData(vRef->fIndex) + group->GetVertBufferStart(vRef->fIndex) * vRef->fVertexSize;
    }
    memcpy(destPtr, vData, size);

    // Unlock the buffer
    fDynVtxBuff->Unlock();

    // Advance next available spot index
    fNextDynVtx += size;

    // Set the timestamp
    vRef->fRefTime = fVtxRefTime;
    vRef->SetDirty(false);

    if( !vRef->fD3DBuffer )
    {
        vRef->fD3DBuffer = fDynVtxBuff;
        fDynVtxBuff->AddRef();
    }
    hsAssert(vRef->fD3DBuffer == fDynVtxBuff, "Holding on to an old dynamic buffer?");

//  vRef->SetRebuiltSinceUsed(true);

    return false;
}

// ICheckAuxBuffers ///////////////////////////////////////////////////////////////////////
// The AuxBuffers are associated with drawables for things to be drawn right after that
// drawable's contents. In particular, see the plDynaDecal, which includes things like
// water ripples, bullet hits, and footprints.
// This function just makes sure they are ready to be rendered, called right before
// the rendering.
bool plDXPipeline::ICheckAuxBuffers(const plAuxSpan* span)
{
    plGBufferGroup* group = span->fGroup;

    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx); 
    if( !vRef )
        return true;

    plDXIndexBufferRef* iRef = (plDXIndexBufferRef*)group->GetIndexBufferRef(span->fIBufferIdx);
    if( !iRef )
        return true;

    // If our vertex buffer ref is volatile and the timestamp is off
    // then it needs to be refilled
    if( vRef->Expired(fVtxRefTime) )
    {
        IRefreshDynVertices(group, vRef);
    }
    if( vRef->fOffset != iRef->fOffset )
    {
        iRef->fOffset = vRef->fOffset;

        iRef->SetRebuiltSinceUsed(true);
    }

    return false; // No error
}

// ICheckDynBuffers ////////////////////////////////////////////////////////////////////////////////////////
// Make sure the buffers underlying this span are ready to be rendered. Meaning that the underlying
// D3D buffers are in sync with the plasma buffers.
bool plDXPipeline::ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* spanBase)
{
    if( !(spanBase->fTypeMask & plSpan::kVertexSpan) )
        return false;
    // If we arent' an trilist, we're toast.
    if( !(spanBase->fTypeMask & plSpan::kIcicleSpan) )
        return false;

    plIcicle* span = (plIcicle*)spanBase;

    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)group->GetVertexBufferRef(span->fVBufferIdx); 
    if( !vRef )
        return true;

    plDXIndexBufferRef* iRef = (plDXIndexBufferRef*)group->GetIndexBufferRef(span->fIBufferIdx);
    if( !iRef )
        return true;

    // If our vertex buffer ref is volatile and the timestamp is off
    // then it needs to be refilled
    if( vRef->Expired(fVtxRefTime) )
    {
        IRefreshDynVertices(group, vRef);
    }
    if( vRef->fOffset != iRef->fOffset )
    {
        iRef->fOffset = vRef->fOffset;

        iRef->SetRebuiltSinceUsed(true);
    }
    if( iRef->IsDirty()  )
    {
        IFillIndexBufferRef(iRef, group, span->fIBufferIdx);
        iRef->SetRebuiltSinceUsed(true);
    }

    return false; // No error
}

//// RenderSpans /////////////////////////////////////////////////////////////
// Renders an array of spans obtained from a plDrawableSpans object
// The incoming visList gives the indices of the spans which are visible and should
// be drawn now, and gives them in sorted order.
void    plDXPipeline::RenderSpans(plDrawableSpans *drawable, const std::vector<int16_t>& visList)
{
    plProfile_BeginTiming(RenderSpan);

    hsMatrix44      lastL2W;
    bool            drewPatch = false;
    hsGMaterial     *material;

    const hsTArray<plSpan *>&       spans = drawable->GetSpanArray();

    plProfile_IncCount(EmptyList, visList.empty() ? 1 : 0);

    /// Set this (*before* we do our TestVisibleWorld stuff...)
    lastL2W.Reset();
    ISetLocalToWorld( lastL2W, lastL2W );   // This is necessary; otherwise, we have to test for
                                            // the first transform set, since this'll be identity
                                            // but the actual device transform won't be (unless
                                            // we do this)


    /// Loop through our spans, combining them when possible
    for (size_t i = 0; i < visList.size(); )
    {
        material = GetOverrideMaterial() ? GetOverrideMaterial() : drawable->GetMaterial( spans[ visList[ i ] ]->fMaterialIdx );

        /// It's an icicle--do our icicle merge loop
        plIcicle tempIce(*( (plIcicle *)spans[ visList[ i ] ] ));

        // Start at i + 1, look for as many spans as we can add to tempIce
        size_t j;
        for (j = i + 1; j < visList.size(); j++)
        {
            if( GetOverrideMaterial() )
                tempIce.fMaterialIdx = spans[visList[j]]->fMaterialIdx;

            plProfile_BeginTiming(MergeCheck);
            if( !spans[ visList[ j ] ]->CanMergeInto( &tempIce ) )
            {
                plProfile_EndTiming(MergeCheck);
                break;
            }
            plProfile_EndTiming(MergeCheck);
            plProfile_Inc(SpanMerge);

            plProfile_BeginTiming(MergeSpan);
            spans[ visList[ j ] ]->MergeInto( &tempIce );
            plProfile_EndTiming(MergeSpan);
        }

        if (material != nullptr)
        {
            // What do we change?

            plProfile_BeginTiming(SpanTransforms);
            ISetupTransforms(drawable, tempIce, lastL2W);
            plProfile_EndTiming(SpanTransforms);

            // Turn on this spans lights and turn off the rest.
            IEnableLights( &tempIce );

            // Check that the underlying buffers are ready to go.
            plProfile_BeginTiming(CheckDyn);
            ICheckDynBuffers(drawable, drawable->GetBufferGroup(tempIce.fGroupIdx), &tempIce);
            plProfile_EndTiming(CheckDyn);

            plProfile_BeginTiming(CheckStat);
            CheckVertexBufferRef(drawable->GetBufferGroup(tempIce.fGroupIdx), tempIce.fVBufferIdx);
            CheckIndexBufferRef(drawable->GetBufferGroup(tempIce.fGroupIdx), tempIce.fIBufferIdx);
            plProfile_EndTiming(CheckStat);

            // Draw this span now
            IRenderBufferSpan( tempIce,
                                drawable->GetVertexRef( tempIce.fGroupIdx, tempIce.fVBufferIdx ),
                                drawable->GetIndexRef( tempIce.fGroupIdx, tempIce.fIBufferIdx ),
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

//// IAddBoundsSpan ///////////////////////////////////////////////////////////
//  Creates a new span for the given drawable to represent the specified
//  world bounds.
// Debugging only.

void    plDXPipeline::IAddBoundsSpan( plDrawableSpans *ice, const hsBounds3Ext *bounds, uint32_t bndColor )
{
#if MCN_BOUNDS_SPANS
    static std::vector<plGeometrySpan *> spanArray;
    static hsMatrix44       identMatrix;
    static hsPoint3     c[ 8 ], n[ 8 ];
    static int          nPts[ 8 ][ 3 ] = { { -1, -1, -1 }, { 1, -1, -1 }, { -1, 1, -1 }, { 1, 1, -1 },
                                        { -1, -1, 1 }, { 1, -1, 1 }, { -1, 1, 1 }, { 1, 1, 1 } };
    int             i;


    if (spanArray.empty())
    {
        spanArray = { new plGeometrySpan };
        identMatrix.Reset();

        // Make normals
        for( i = 0; i < 8; i++ )
        {
            n[ i ].fX = (float)nPts[ i ][ 0 ];
            n[ i ].fY = (float)nPts[ i ][ 1 ];
            n[ i ].fZ = (float)nPts[ i ][ 2 ];
        }
    }
    else
        spanArray[ 0 ] = new plGeometrySpan();

    plGeometrySpan* newSpan = spanArray[0];

    newSpan->BeginCreate( fBoundsMat, identMatrix, 0 );

    // Make corners
    c[1] = c[2] = c[4] = *bounds->GetCorner(&c[0]);
    hsVector3 axes[3];
    bounds->GetAxes(axes+0, axes+1, axes+2);
    c[1] += axes[0];
    c[2] += axes[1];
    c[4] += axes[2];

    c[3] = c[1];
    c[3] += axes[1];

    c[5] = c[1];
    c[5] += axes[2];

    c[6] = c[2];
    c[6] += axes[2];

    c[7] = c[6];
    c[7] += axes[0];

    for( i = 0; i < 8; i++ )
        newSpan->AddVertex( &c[ i ], &n[ i ], bndColor );

    newSpan->AddTriIndices( 0, 1, 2 );
    newSpan->AddTriIndices( 2, 1, 3 );

    newSpan->AddTriIndices( 6, 3, 7 );
    newSpan->AddTriIndices( 7, 1, 5 );
    newSpan->AddTriIndices( 5, 0, 4 );
    newSpan->AddTriIndices( 4, 2, 6 );

    newSpan->EndCreate();

    fBSpansToDelete.Append( ice->AppendDISpans( spanArray ) );

#endif
}

//// IAddNormalsSpan //////////////////////////////////////////////////////////
//  Creates a new span for the given drawable to represent the specified
//  world bounds.
// Debugging only.

void    plDXPipeline::IAddNormalsSpan( plDrawableSpans *ice, plIcicle *span, plDXVertexBufferRef *vRef, uint32_t bndColor )
{
#if MCN_BOUNDS_SPANS
    static std::vector<plGeometrySpan *> spanArray;
    static hsMatrix44       identMatrix;
    static hsPoint3     point, off, blank;
    hsVector3   b2;
    uint16_t      v1, v2, v3;
    int             i;


    if (spanArray.empty())
    {
        spanArray = { new plGeometrySpan };
        identMatrix.Reset();
    }
    else
        spanArray[ 0 ] = new plGeometrySpan();

    plGeometrySpan* newSpan = spanArray[0];

    newSpan->BeginCreate( fBoundsMat, span->fLocalToWorld, 0 );

    for( i = 0; i < span->fVLength; i++ )
    {
        point = vRef->fOwner->Position( span->fVBufferIdx, span->fCellIdx, span->fCellOffset + i );
        b2 = vRef->fOwner->Normal( span->fVBufferIdx, span->fCellIdx, span->fCellOffset + i );
        off.Set( point.fX + b2.fX, point.fY + b2.fY, point.fZ + b2.fZ );
        v1 = newSpan->AddVertex( &point, &blank, bndColor );
        v2 = newSpan->AddVertex( &off, &blank, bndColor );
        v3 = newSpan->AddVertex( &point, &blank, bndColor );
        newSpan->AddTriIndices( v1, v2, v3 );
    }

    newSpan->EndCreate();

    fBSpansToDelete.Append( ice->AppendDISpans( spanArray ) );

#endif
}

//// BeginRender //////////////////////////////////////////////////////////////
// Specifies the beginning of the render frame.
// If this succeeds (returns false) it must be matched with a call to EndRender.
// Normally, the main client loop will wrap the entire scene render (including
// any offscreen rendering) in a BeginRender/EndRender pair. There is no need
// for further calls for sub-renders.
bool plDXPipeline::BeginRender()
{
    // Do we have some restoration work ahead of us?
    // Checks for Device Lost condition
    if( IResetDevice() )
        return true;

    // We were lost, but now we're found! Spread the good uint16_t brother!
    if( fDevWasLost )
    {
        /// Broadcast a message letting everyone know that we were recreated and that
        /// all device-specific stuff needs to be recreated
//      plDeviceRecreateMsg* clean = new plDeviceRecreateMsg(this);
//      clean->Send();

        fDevWasLost = false;
    }

    if (IsDebugFlagSet(plPipeDbg::kFlagReload))
    {
        IReleaseShaders();
        fD3DDevice->EvictManagedResources();
        SetDebugFlag(plPipeDbg::kFlagReload, false);
    }

    // offset transform
    RefreshScreenMatrices();

    // If this is the primary BeginRender, make sure we're really ready.
    if( !fInSceneDepth++ )
    {
        // Superfluous setting of Z state.
        fD3DDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

        /// If we have a renderTarget active, use its viewport
        fDevice.SetViewport();

        // Tell D3D we're ready to start rendering.
        if( FAILED(fD3DDevice->BeginScene()) )
        {
            fDeviceLost = true;
        }

        // Reset all our buffer/image usage counters
        fNextDynVtx = 0;
        fVtxRefTime++;

        fTexUsed = 0;
        fVtxUsed = 0;

        // Render any shadow maps that have been submitted for this frame.
        IPreprocessShadows();
        IPreprocessAvatarTextures();
    }
    fRenderCnt++;

    // Would probably rather this be an input.
    fTime = hsTimer::GetSysSeconds();

    return false;
}


//// RenderScreenElements /////////////////////////////////////////////////////
//  Renders all the screen elements, such as debug text and plates. Also puts
//  up all the info about vertex buffers and such. Should be called right 
//  before EndRender(), but only on the main surface (not on renderTargets,
//  for example).

void    plDXPipeline::RenderScreenElements()
{
    bool        reset = false;


#if MCN_BOUNDS_SPANS
    if( fBoundsSpans && fBSpansToDelete.GetCount() > 0 )
    {
        Draw( fBoundsSpans );
        
        int     i;
        for( i = 0; i < fBSpansToDelete.GetCount(); i++ )
            fBoundsSpans->RemoveDISpans( fBSpansToDelete[ i ] );

        fBSpansToDelete.Reset();
    }
#endif
    if( fView.HasCullProxy())
        Draw( fView.GetCullProxy() );

#ifdef MF_ENABLE_HACKOFF
    //WHITE
    static plPlate* hackPlate = nullptr;
    if( doHackPlate < hackOffscreens.GetCount() )
    {
        if( !hackPlate )
        {
            fPlateMgr->CreatePlate(&hackPlate, 0.5f, 0.5f, 1.0f, 1.0f);
            hackPlate->CreateBlankMaterial(32, 32, false);
        }
    }
    if( hackPlate )
    {
        if( doHackPlate < hackOffscreens.GetCount() )
        {
            hsGMaterial* hackMat = hackPlate->GetMaterial();
            plLayer* lay = plLayer::ConvertNoRef(hackMat->GetLayer(0));
            if( lay )
                lay->SetTexture(hackOffscreens[doHackPlate]);
            hackPlate->SetVisible( true );
        }
        else
        {
            hackPlate->SetVisible( false );
        }
    }
#endif // MF_ENABLE_HACKOFF

    hsGMatState tHack = PushMaterialOverride(hsGMatState::kMisc, hsGMatState::kMiscWireFrame, false);
    hsGMatState ambHack = PushMaterialOverride(hsGMatState::kShade, hsGMatState::kShadeWhite, true);
    
    plProfile_BeginTiming(PlateMgr);
    /// Plates
    if( fPlateMgr )
    {
        fPlateMgr->DrawToDevice( this );
        reset = true;
    }
    plProfile_EndTiming(PlateMgr);

    PopMaterialOverride(ambHack, true);
    PopMaterialOverride(tHack, false);

    plProfile_BeginTiming(DebugText);
    /// Debug text
    if( fDebugTextMgr && plDebugText::Instance().IsEnabled() )
    {
        fDebugTextMgr->DrawToDevice( this );

        reset = true;
    }
    plProfile_EndTiming(DebugText);

    plProfile_BeginTiming(Reset);
    if( reset )
    {
        // Reset these since the drawing might have trashed them
        hsRefCnt_SafeUnRef( fSettings.fCurrVertexBuffRef );
        hsRefCnt_SafeUnRef( fSettings.fCurrIndexBuffRef );
        fSettings.fCurrVertexBuffRef = nullptr;
        fSettings.fCurrIndexBuffRef = nullptr;

        fView.fXformResetFlags = fView.kResetAll;       // Text destroys view transforms
        hsRefCnt_SafeUnRef( fLayerRef[ 0 ] );
        fLayerRef[0] = nullptr;       // Text destroys stage 0 texture
    }
    plProfile_EndTiming(Reset);
}

//// EndRender ////////////////////////////////////////////////////////////////
// Tell D3D we're through rendering for this frame, and flip the back buffer to front.
// Also includes a bit of making sure we're not holding onto anything that might
// get deleted before the next render.
bool plDXPipeline::EndRender()
{
#ifdef MF_ENABLE_HACKOFF
    hackOffscreens.SetCount(0);
#endif // MF_ENABLE_HACKOFF

    IBottomLayer();

    bool retVal = false;
    /// Actually end the scene
    if( !--fInSceneDepth )
    {
        WEAK_ERROR_CHECK( fD3DDevice->EndScene() );
        retVal = IFlipSurface();

        IClearShadowSlaves();
    }

    // Do this last, after we've drawn everything
    // Just letting go of things we're done with for the frame.
    fForceMatHandle = true;
    hsRefCnt_SafeUnRef( fCurrMaterial );
    fCurrMaterial = nullptr;

    int i;
    for( i = 0; i < 8; i++ )
    {
        if( fLayerRef[i] )
        {
            hsRefCnt_SafeUnRef(fLayerRef[i]);
            fLayerRef[i] = nullptr;
        }
    }

    return retVal;
}

// SetGamma ////////////////////////////////////////////////////////////
// Create and set a gamma table based on the input exponent values for
// R, G, and B. Can also set explicit table using the other SetGamma().
bool plDXPipeline::SetGamma(float eR, float eG, float eB)
{
    if( fSettings.fNoGammaCorrect )
        return false;

    D3DGAMMARAMP ramp;

    ramp.red[0] = ramp.green[0] = ramp.blue[0] = 0L;

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
        ramp.red[i] = uint16_t(gamm);

        gamm = pow(orig, eG);
        gamm *= float(uint16_t(-1));
        ramp.green[i] = uint16_t(gamm);

        gamm = pow(orig, eB);
        gamm *= float(uint16_t(-1));
        ramp.blue[i] = uint16_t(gamm);
    }

    fD3DDevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &ramp);

    return true;
}

// SetGamma
// Copy the input gamma tables and pass them to the hardware.
bool plDXPipeline::SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB)
{
    if( fSettings.fNoGammaCorrect )
        return false;

    D3DGAMMARAMP ramp;
    memcpy(ramp.red, tabR, 256 * sizeof(WORD));
    memcpy(ramp.green, tabG, 256 * sizeof(WORD));
    memcpy(ramp.blue, tabB, 256 * sizeof(WORD));

    fD3DDevice->SetGammaRamp(0, D3DSGR_NO_CALIBRATION, &ramp);

    return true;
}


//// IFlipSurface /////////////////////////////////////////////////////////////
// Initiate moving the back buffer contents to the front buffer. Will detect
// and set the device lost condition when it occurs.
bool  plDXPipeline::IFlipSurface()
{
    /// Works now for both fullscreen and windowed modes
    HRESULT hr = D3D_OK;
    if (fCurrRenderTarget == nullptr)
    {
        hr = fD3DDevice->Present(nullptr, nullptr, fDevice.fHWnd, nullptr);
    }

    if( FAILED(hr) )
    {
        fDeviceLost = true;
    }
    return fDeviceLost;
}

// ExtractMipMap
// This code works and is fairly fast for creating a new mipmap
// as a copy of the data in an offscreen render target. It's not
// currently used, because of driver bugs found in rendering to
// offscreen render targets.
plMipmap* plDXPipeline::ExtractMipMap(plRenderTarget* targ)
{
    if( plCubicRenderTarget::ConvertNoRef(targ) )
        return nullptr;

    if( targ->GetPixelSize() != 32 )
    {
        hsAssert(false, "Only RGBA8888 currently implemented");
        return nullptr;
    }

    plDXRenderTargetRef* ref = (plDXRenderTargetRef*)targ->GetDeviceRef();
    if( !ref )
        return nullptr;

    IDirect3DSurface9* surf = ref->GetColorSurface();
    if( !surf )
        return nullptr;

    D3DLOCKED_RECT rect;
    if (FAILED(surf->LockRect(&rect, nullptr, D3DLOCK_READONLY)))
    {
        return nullptr;
    }

    const int width = targ->GetWidth();
    const int height = targ->GetHeight();

    plMipmap* mipMap = new plMipmap(width, height, plMipmap::kARGB32Config, 1);

    uint8_t* ptr = (uint8_t*)(rect.pBits);
    const int pitch = rect.Pitch;

    const uint32_t blackOpaque = 0xff000000;
    int y;
    for( y = 0; y < height; y++ )
    {
        uint32_t* destPtr = mipMap->GetAddr32(0, y);
        uint32_t* srcPtr = (uint32_t*)ptr;
        int x;
        for( x = 0; x < width; x++ )
        {
            destPtr[x] = srcPtr[x] | blackOpaque;
        }
        ptr += pitch;
    }

    surf->UnlockRect();

    return mipMap;
}

//// CaptureScreen ////////////////////////////////////////////////////////////
// Copy the current contents of the front buffer to the destination mipmap, with optional
// rescaling. Note that the mipmap function which does this rescaling is of low quality
// (pyramid filter even though it claims a box filter) and low performance (slow).
// If it mattered, it would take about an hour to have a higher performance, higher quality,
// more robust rescale function.
// This function is fairly straightforward, the complexity only comes from making sure
// all pixels in dest get written to, even though the client window may be partially 
// offscreen. If the client window is partially offscreen, there will be no values
// for the "offscreen pixels" to copy to dest, so opaque black is used.
bool  plDXPipeline::CaptureScreen( plMipmap *dest, bool flipVertical, uint16_t desiredWidth, uint16_t desiredHeight )
{
    uint32_t            y, *destPtr, *srcPtr, width, height, bigWidth, bigHeight;
    IDirect3DSurface9   *surface;
    D3DLOCKED_RECT      rect;
    RECT                rToLock;


    width = GetViewTransform().GetViewPortWidth();
    height = GetViewTransform().GetViewPortHeight();

    int left = 0;
    int right = width;
    int top = 0;
    int bottom = height;

    if( fSettings.fFullscreen )
    {
        if (FAILED(fD3DDevice->CreateOffscreenPlainSurface(width, height, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, nullptr)))
            return false;

        rToLock.left = GetViewTransform().GetViewPortLeft();
        rToLock.top = GetViewTransform().GetViewPortTop();
        rToLock.right = GetViewTransform().GetViewPortRight();
        rToLock.bottom = GetViewTransform().GetViewPortBottom();
    }
    else
    {
        bigWidth = GetSystemMetrics( SM_CXSCREEN );
        bigHeight = GetSystemMetrics( SM_CYSCREEN );

        if (FAILED(fD3DDevice->CreateOffscreenPlainSurface(bigWidth, bigHeight, D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &surface, nullptr)))
            return false;

        GetClientRect( fDevice.fHWnd, &rToLock );
        MapWindowPoints(fDevice.fHWnd, nullptr, (POINT *)&rToLock, 2);

        if( rToLock.right > bigWidth )
        {
            right -= (rToLock.right - bigWidth);
            rToLock.right = bigWidth;
        }
        if( rToLock.bottom > bigHeight )
        {
            bottom -= (rToLock.bottom - bigHeight);
            rToLock.bottom = bigHeight;
        }
        if( rToLock.top < 0 )
        {
            top -= rToLock.top;
            rToLock.top = 0;
        }
        if( rToLock.left < 0 )
        {
            left -= rToLock.left;
            rToLock.left = 0;
        }
    }

    UINT swapChain = 0;
    if( FAILED( fD3DDevice->GetFrontBufferData(swapChain, surface) ) )
    {
        ReleaseObject( surface );
        return false;
    }

    if( FAILED( surface->LockRect( &rect, &rToLock, D3DLOCK_READONLY ) ) )
    {
        ReleaseObject( surface );
        return false;
    }

    if( dest->GetWidth() != width || dest->GetHeight() != height ||
        dest->GetPixelSize() != 32 )
    {
        dest->Reset();
        dest->Create( width, height, plMipmap::kARGB32Config, 1 );
    }

    const uint32_t blackOpaque = 0xff000000;
    /// Copy over
    for( y = 0; y < top; y++ )
    {
        if (flipVertical)
            destPtr = dest->GetAddr32( 0, height - 1 - y );
        else
            destPtr = dest->GetAddr32( 0, y );

        int x;
        for( x = 0; x < width; x++ )
        {
            *destPtr++ = blackOpaque;
        }
    }
    for( y = top; y < bottom; y++ )
    {
        srcPtr = (uint32_t *)( (uint8_t *)rect.pBits + rect.Pitch * y );
        if (flipVertical)
            destPtr = dest->GetAddr32( 0, height - 1 - y );
        else
            destPtr = dest->GetAddr32( 0, y );

        int x;
        for( x = 0; x < left; x++ )
            *destPtr++ = blackOpaque;

        memcpy( destPtr, srcPtr, (right - left) * sizeof( uint32_t ) );
        destPtr += (right - left);

        for( x = right; x < width; x++ )
            *destPtr++ = blackOpaque;
    }
    for( y = bottom; y < height; y++ )
    {
        if (flipVertical)
            destPtr = dest->GetAddr32( 0, height - 1 - y );
        else
            destPtr = dest->GetAddr32( 0, y );

        int x;
        for( x = 0; x < width; x++ )
        {
            *destPtr++ = blackOpaque;
        }
    }

    surface->UnlockRect();
    ReleaseObject( surface );

    if( desiredWidth != 0 && desiredHeight != 0 )
    {
        // Rescale to the right size
        dest->ResizeNicely( desiredWidth, desiredHeight, plMipmap::kDefaultFilter );
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
//// Render Targets ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// MakeRenderTargetRef //////////////////////////////////////////////////////
// Create the a Plasma render target ref, filling in the underlying D3D resources
// (e.g. color/depth buffers).
// Note that for ATI boards, we create a single depth surface for them to share.
// That can actually be 2 depth surfaces, if some color surfaces are 16 bit and
// others are 24/32 bit, since the ATI's want to match color depth with depth depth.
hsGDeviceRef    *plDXPipeline::MakeRenderTargetRef( plRenderTarget *owner )
{
    plDXRenderTargetRef *ref = nullptr;
    IDirect3DSurface9       *surface = nullptr, *depthSurface = nullptr;
    IDirect3DTexture9       *texture = nullptr;
    IDirect3DCubeTexture9   *cTexture = nullptr;
    D3DFORMAT               surfFormat = D3DFMT_UNKNOWN, depthFormat = D3DFMT_UNKNOWN;
    D3DRESOURCETYPE         resType;
    plCubicRenderTarget     *cubicRT;

    hsAssert(!fManagedAlloced, "Allocating non-managed resource with managed resources alloc'd");

    // If we have Shader Model 3 and support non-POT textures, let's make reflections the pipe size
    plDynamicCamMap* camMap = plDynamicCamMap::ConvertNoRef(owner);
    if (camMap)
    {
        if ((plQuality::GetCapability() > plQuality::kPS_2) && fSettings.fD3DCaps & kCapsNpotTextures)
            camMap->ResizeViewport(IGetViewTransform());
    }

    /// Check--is this renderTarget really a child of a cubicRenderTarget?
    if( owner->GetParent() )
    {
        /// This'll create the deviceRefs for all of its children as well
        MakeRenderTargetRef( owner->GetParent() );
        return owner->GetDeviceRef();
    }

    // If we already have a rendertargetref, we just need it filled out with D3D resources.
    if( owner->GetDeviceRef() )
        ref = (plDXRenderTargetRef *)owner->GetDeviceRef();

    // Look for supported format. Note that the surfFormat and depthFormat are
    // passed in by ref, so they may be different after this function call (if
    // an exact match isn't supported, but something similar is).
    if( !IPrepRenderTargetInfo( owner, surfFormat, depthFormat, resType ) )
    {
        hsAssert( false, "Error getting renderTarget info" );
        return nullptr;
    }

    /// Create the render target now
    // Start with the depth surface.
    // Note that we only ever give a cubic rendertarget a single shared depth buffer, 
    // since we only render one face at a time. If we were rendering part of face X, then part
    // of face Y, then more of face X, then they would all need their own depth buffers.
    if( owner->GetZDepth() && (owner->GetFlags() & ( plRenderTarget::kIsTexture | plRenderTarget::kIsOffscreen )) )
    {
        // 9600 THRASH
        if( !fSettings.fShareDepth )
        {
            /// Create the depthbuffer
            if( FAILED( fD3DDevice->CreateDepthStencilSurface(
                                owner->GetWidth(), owner->GetHeight(), depthFormat, 
                                D3DMULTISAMPLE_NONE, 0, FALSE,
                                &depthSurface, nullptr)))
            {
                return nullptr;
            }

            // See plDXRenderTargetRef::Release()
            //D3DSURF_MEMNEW(depthSurface);
        }
        else
        {
            const int iZ = owner->GetZDepth() / 24;
            if( !fSharedDepthSurface[iZ] )
            {
                plConst(DWORD) kSharedWidth(800);
                plConst(DWORD) kSharedHeight(600);
                if( FAILED( fD3DDevice->CreateDepthStencilSurface(
                                    kSharedWidth, kSharedHeight, depthFormat, 
                                    D3DMULTISAMPLE_NONE, 0, FALSE,
                                    &fSharedDepthSurface[iZ], nullptr)))
                {
                    return nullptr;
                }
                // See plDXRenderTargetRef::Release()
                //D3DSURF_MEMNEW(fSharedDepthSurface[iZ]);
                fSharedDepthFormat[iZ] = depthFormat;
            }
            hsAssert(depthFormat == fSharedDepthFormat[iZ], "Mismatch on render target types");
            fSharedDepthSurface[iZ]->AddRef();
            depthSurface = fSharedDepthSurface[iZ];
        }
    }

    // See if it's a cubic render target. 
    // Primary consumer here is the vertex/pixel shader water.
    cubicRT = plCubicRenderTarget::ConvertNoRef( owner );
    if( cubicRT )
    {
        /// And create the ref (it'll know how to set all the flags)
        if( ref )
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        if( !FAILED( fD3DDevice->CreateCubeTexture( owner->GetWidth(), 1, D3DUSAGE_RENDERTARGET, surfFormat,
                                                    D3DPOOL_DEFAULT, (IDirect3DCubeTexture9 **)&cTexture, nullptr)))
        {
            /// Create a CUBIC texture
            for( int i = 0; i < 6; i++ )
            {
                plRenderTarget          *face = cubicRT->GetFace( i );
                plDXRenderTargetRef *fRef;

                if (face->GetDeviceRef() != nullptr)
                {
                    fRef = (plDXRenderTargetRef *)face->GetDeviceRef();
                    fRef->Set( surfFormat, 0, face );
                    if( !fRef->IsLinked() )
                        fRef->Link( &fRenderTargetRefList );
                }
                else
                {
                    face->SetDeviceRef( new plDXRenderTargetRef( surfFormat, 0, face, false ) );
                    ( (plDXRenderTargetRef *)face->GetDeviceRef())->Link( &fRenderTargetRefList );
                    // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
                    hsRefCnt_SafeUnRef( face->GetDeviceRef() );
                }
            }

            D3DSURF_MEMNEW(cTexture);

            ref->SetTexture( cTexture, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }
    }
    // Not a cubic, is it a texture render target? These are currently used
    // primarily for shadow map generation.
    else if( owner->GetFlags() & plRenderTarget::kIsTexture )
    {
        /// Create a normal texture
        if( ref )
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        if( !FAILED( fD3DDevice->CreateTexture( owner->GetWidth(), owner->GetHeight(), 1, D3DUSAGE_RENDERTARGET, surfFormat,
                                                D3DPOOL_DEFAULT, (IDirect3DTexture9 **)&texture, nullptr)))
        {
            D3DSURF_MEMNEW(texture);

            ref->SetTexture( texture, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }
    }
    // Not a texture either, must be a plain offscreen.
    // Note that the plain offscreen code path works and was used until recently,
    // until it turned up that some hardware had bugs on rendering to
    // an offscreen. 
    // Some GeForce1's had lighting anomolies, although my GeForce1 DDR didn't.
    // Some ATI's showed a momemtary glitch of corrupted rendering on the frame
    // when rendering both to the primary and an offscreen (again, not mine).
    // So the Offscreen isn't currently used for anything.
    else if( owner->GetFlags() & plRenderTarget::kIsOffscreen )
    {
        /// Create a blank surface
        if( ref )
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        // Specify true for lockable, otherwise I'm not sure what we'd do with it. I guess we
        // could copyrect to another surface, presumably a texture. But right now the only
        // thing we use this for is to render a snapshot and copy it to sysmem, which implies
        // lockable.
        if( !FAILED( fD3DDevice->CreateRenderTarget( owner->GetWidth(), owner->GetHeight(), surfFormat,
                            D3DMULTISAMPLE_NONE, 0,
                            TRUE, &surface, nullptr)))
        {
            D3DSURF_MEMNEW(surface);

            ref->SetTexture( surface, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }

    }

    // Keep it in a linked list for ready destruction.
    if( owner->GetDeviceRef() != ref )
    {
        owner->SetDeviceRef( ref );
        // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
        hsRefCnt_SafeUnRef( ref );
        if (ref != nullptr && !ref->IsLinked())
            ref->Link( &fRenderTargetRefList );
    }
    else
    {
        if (ref != nullptr && !ref->IsLinked())
            ref->Link( &fRenderTargetRefList );
    }

    // Mark as dirty.
    if (ref != nullptr)
    {
        ref->SetDirty( false );
    }

    return ref;
}

//// SharedRenderTargetRef //////////////////////////////////////////////////////
// Same as MakeRenderTargetRef, except specialized for the shadow map generation.
// The shadow map pools of a given dimension (called RenderTargetPool) all share
// a single depth buffer of that size. This allows sharing on NVidia hardware
// that wants the depth buffer dimensions to match the color buffer size.
// It may be that NVidia hardware doesn't care any more. Contact Matthias 
// about that.
hsGDeviceRef* plDXPipeline::SharedRenderTargetRef(plRenderTarget* share, plRenderTarget *owner)
{
    plDXRenderTargetRef*    ref = nullptr;
    IDirect3DSurface9*      surface = nullptr;
    IDirect3DSurface9*      depthSurface = nullptr;
    IDirect3DTexture9*      texture = nullptr;
    IDirect3DCubeTexture9*  cTexture = nullptr;
    D3DFORMAT               surfFormat = D3DFMT_UNKNOWN, depthFormat = D3DFMT_UNKNOWN;
    D3DRESOURCETYPE         resType;
    int                     i;
    plCubicRenderTarget*    cubicRT;
    uint16_t                  width, height;

    // If we don't already have one to share from, start from scratch.
    if( !share )
        return MakeRenderTargetRef(owner);

    hsAssert(!fManagedAlloced, "Allocating non-managed resource with managed resources alloc'd");

#ifdef HS_DEBUGGING
    // Check out the validity of the match. Debug only.
    hsAssert(!owner->GetParent() == !share->GetParent(), "Mismatch on shared render target");
    hsAssert(owner->GetWidth() == share->GetWidth(), "Mismatch on shared render target");
    hsAssert(owner->GetHeight() == share->GetHeight(), "Mismatch on shared render target");
    hsAssert(owner->GetZDepth() == share->GetZDepth(), "Mismatch on shared render target");
    hsAssert(owner->GetStencilDepth() == share->GetStencilDepth(), "Mismatch on shared render target");
#endif // HS_DEBUGGING

    /// Check--is this renderTarget really a child of a cubicRenderTarget?
    if (owner->GetParent() != nullptr)
    {
        /// This'll create the deviceRefs for all of its children as well
        SharedRenderTargetRef(share->GetParent(), owner->GetParent());
        return owner->GetDeviceRef();
    }

    if (owner->GetDeviceRef() != nullptr)
        ref = (plDXRenderTargetRef *)owner->GetDeviceRef();

    // Look for a good format of matching color and depth size. 
    if( !IFindRenderTargetInfo(owner, surfFormat, resType) )
    {
        hsAssert( false, "Error getting renderTarget info" );
        return nullptr;
    }


    /// Create the render target now
    // Start with the depth. We're just going to share the depth surface on the
    // input shareRef.
    plDXRenderTargetRef* shareRef = (plDXRenderTargetRef*)share->GetDeviceRef();
    hsAssert(shareRef, "Trying to share from a render target with no ref");
    if( shareRef->fD3DDepthSurface )
        shareRef->fD3DDepthSurface->AddRef();
    depthSurface = shareRef->fD3DDepthSurface;

    // Check for Cubic. This is unlikely, since this function is currently only
    // used for the shadow map pools.
    cubicRT = plCubicRenderTarget::ConvertNoRef( owner );
    if (cubicRT != nullptr)
    {
        /// And create the ref (it'll know how to set all the flags)
        if (ref != nullptr)
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        hsAssert(!fManagedAlloced, "Alloc default with managed alloc'd");
        if( !FAILED( fD3DDevice->CreateCubeTexture( owner->GetWidth(), 1, D3DUSAGE_RENDERTARGET, surfFormat,
                                                    D3DPOOL_DEFAULT, (IDirect3DCubeTexture9 **)&cTexture, nullptr)))
        {

            /// Create a CUBIC texture
            for( i = 0; i < 6; i++ )
            {
                plRenderTarget          *face = cubicRT->GetFace( i );
                plDXRenderTargetRef *fRef;

                if (face->GetDeviceRef() != nullptr)
                {
                    fRef = (plDXRenderTargetRef *)face->GetDeviceRef();
                    fRef->Set( surfFormat, 0, face );
                    if( !fRef->IsLinked() )
                        fRef->Link( &fRenderTargetRefList );
                }
                else
                {
                    face->SetDeviceRef( new plDXRenderTargetRef( surfFormat, 0, face, false ) );
                    ( (plDXRenderTargetRef *)face->GetDeviceRef())->Link( &fRenderTargetRefList );
                    // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
                    hsRefCnt_SafeUnRef( face->GetDeviceRef() );
                }
            }
        
            D3DSURF_MEMNEW(cTexture);

            ref->SetTexture( cTexture, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }
    }
    // Is it a texture render target? Probably, since shadow maps are all we use this for.
    else if( owner->GetFlags() & plRenderTarget::kIsTexture )
    {
        /// Create a normal texture
        if (ref != nullptr)
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        hsAssert(!fManagedAlloced, "Alloc default with managed alloc'd");
        if( !FAILED( fD3DDevice->CreateTexture( owner->GetWidth(), owner->GetHeight(), 1, D3DUSAGE_RENDERTARGET, surfFormat, 
                                                D3DPOOL_DEFAULT, (IDirect3DTexture9 **)&texture, nullptr)))
        {
            D3DSURF_MEMNEW(texture);

            ref->SetTexture( texture, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }
    }
    // Pretty sure this code path has never been followed.
    else if( owner->GetFlags() & plRenderTarget::kIsOffscreen )
    {
        /// Create a blank surface
        if (ref != nullptr)
            ref->Set( surfFormat, 0, owner );
        else
            ref = new plDXRenderTargetRef( surfFormat, 0, owner );

        width = owner->GetWidth();
        height = owner->GetHeight();

        if( !FAILED( fD3DDevice->CreateRenderTarget( width, height, surfFormat, 
                            D3DMULTISAMPLE_NONE, 0,
                            FALSE, &surface, nullptr)))
        {
            D3DSURF_MEMNEW(surface);

            ref->SetTexture( surface, depthSurface );
        }
        else
        {
            ReleaseObject(depthSurface);
            hsRefCnt_SafeUnRef(ref);
            ref = nullptr;
        }

    }

    if( owner->GetDeviceRef() != ref )
    {
        owner->SetDeviceRef( ref );
        // Unref now, since for now ONLY the RT owns the ref, not us (not until we use it, at least)
        hsRefCnt_SafeUnRef( ref );
        if (ref != nullptr && !ref->IsLinked())
            ref->Link( &fRenderTargetRefList );
    }
    else
    {
        if (ref != nullptr && !ref->IsLinked())
            ref->Link( &fRenderTargetRefList );
    }

    if (ref != nullptr)
    {
        ref->SetDirty( false );
    }

    return ref;
}

//// IPrepRenderTargetInfo ////////////////////////////////////////////////////
//  Shared processing of render target creation parameters. Also does the 
//  dirty work of finding a good surface format to use.
bool  plDXPipeline::IPrepRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat,
                                              D3DFORMAT &depthFormat, D3DRESOURCETYPE &resType )
{
    int         i, j;
    uint16_t      flags, width, height;
    int8_t        bitDepth, zDepth, stencilDepth, stencilIndex;
    D3DFORMAT   depthFormats[] = { D3DFMT_D24X8, D3DFMT_D24X4S4, D3DFMT_D24S8 };
    IDirect3D9* d3d = hsGDirect3D::GetDirect3D();


    flags = owner->GetFlags();
    width = owner->GetWidth();
    height = owner->GetHeight();
    bitDepth = owner->GetPixelSize();
    zDepth = owner->GetZDepth();
    stencilDepth = owner->GetStencilDepth();

    if( flags != 0 )
    {
        if( flags & plRenderTarget::kIsTexture )
        {
            /// Do an extra check for width and height here
            if (!(fSettings.fD3DCaps & kCapsNpotTextures))
            {
                for( i = width >> 1, j = 0; i != 0; i >>= 1, j++ );
                if( width != ( 1 << j ) )
                    return false;

                for( i = height >> 1, j = 0; i != 0; i >>= 1, j++ );
                if( height!= ( 1 << j ) )
                    return false;
            }

            resType = D3DRTYPE_TEXTURE;
        }
        else
            resType = D3DRTYPE_SURFACE;

        if( bitDepth == 16 )
            surfFormat = D3DFMT_A4R4G4B4;
        else if( bitDepth == 32 )
            surfFormat = D3DFMT_A8R8G8B8;

        /// Get the backbuffer format (if one is requested)
        if( zDepth )
        {
            if( zDepth == 16 && stencilDepth == 0 )
                depthFormat = D3DFMT_D16;
            else if( zDepth == 24 )
            {
                if( stencilDepth == 0 ) stencilIndex = 0;
                else if( stencilDepth <= 4 ) stencilIndex = 1;
                else if( stencilDepth <= 8 ) stencilIndex = 2;
                else
                    stencilIndex = 2;

                depthFormat = depthFormats[ stencilIndex ];
            }
            else if( zDepth == 32 && stencilDepth == 0 )
                depthFormat = D3DFMT_D32;
            else if( zDepth == 15 && stencilDepth == 1 )
                depthFormat = D3DFMT_D15S1;

            if( surfFormat == D3DFMT_UNKNOWN || depthFormat == D3DFMT_UNKNOWN )
            {
                return false;
            }
        }
        else
        {
            depthFormat = D3DFMT_UNKNOWN;
        }

        /// Check the device format
        if( FAILED( fSettings.fDXError = d3d->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                 D3DUSAGE_RENDERTARGET, resType, surfFormat ) ) )
        {
            if( bitDepth == 16 )
            {
                bitDepth = 32;
                surfFormat = D3DFMT_A8R8G8B8;
            }
            else if( bitDepth == 32 )
            {
                bitDepth = 16;
                surfFormat = D3DFMT_A4R4G4B4;
            }
            if( FAILED( fSettings.fDXError = d3d->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                     D3DUSAGE_RENDERTARGET, resType, surfFormat ) ) )
            {
                IGetD3DError();
                return false;
            }
        }

        if( zDepth )
        {
            while( FAILED( fSettings.fDXError = d3d->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                        D3DUSAGE_DEPTHSTENCIL, D3DRTYPE_SURFACE, depthFormat ) ) )
            {
                if( stencilIndex < sizeof( depthFormats ) / sizeof( depthFormats[ 0 ] ) - 1 )
                {
                    stencilIndex++;
                    depthFormat = depthFormats[ stencilIndex ];
                }
                else
                {
                    IGetD3DError();
                    return false;
                }
            }

            if( FAILED( fSettings.fDXError = d3d->CheckDepthStencilMatch( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                          surfFormat, depthFormat ) ) )
            {
                IGetD3DError();
                return false;
            }
        }
    }

    return true;
}

//// IFindRenderTargetInfo ////////////////////////////////////////////////////
//  Shared processing of render target creation parameters. Also does the 
//  dirty work of finding a good surface format to use.
// Doesn't bother checking depth buffer, since this is only used for a render target
// that's going to share a depth buffer that's already been created.
bool  plDXPipeline::IFindRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat, D3DRESOURCETYPE &resType )
{
    uint16_t      flags, width, height;
    int8_t        bitDepth;


    flags = owner->GetFlags();
    width = owner->GetWidth();
    height = owner->GetHeight();
    bitDepth = owner->GetPixelSize();

    IDirect3D9* d3d = hsGDirect3D::GetDirect3D();
    if( flags != 0 )
    {
        if( flags & plRenderTarget::kIsTexture )
        {
            resType = D3DRTYPE_TEXTURE;
        }
        else
            resType = D3DRTYPE_SURFACE;

        if( bitDepth == 16 )
            surfFormat = D3DFMT_A4R4G4B4;
        else if( bitDepth == 32 )
            surfFormat = D3DFMT_A8R8G8B8;

        if( surfFormat == D3DFMT_UNKNOWN )
        {
            return false;
        }

        /// Check the device format
        if( FAILED( fSettings.fDXError = d3d->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                 D3DUSAGE_RENDERTARGET, resType, surfFormat ) ) )
        {
            if( bitDepth == 16 )
            {
                bitDepth = 32;
                surfFormat = D3DFMT_A8R8G8B8;
            }
            else if( bitDepth == 32 )
            {
                bitDepth = 16;
                surfFormat = D3DFMT_A4R4G4B4;
            }
            if( FAILED( fSettings.fDXError = d3d->CheckDeviceFormat( fCurrentAdapter, fCurrentDevice->fDDType, fCurrentMode->fDDmode.Format,
                                                                     D3DUSAGE_RENDERTARGET, resType, surfFormat ) ) )
            {
                IGetD3DError();
                return false;
            }
        }
    }

    return true;
}

// PushRenderRequest ///////////////////////////////////////////////
// We're moving from our current render (probably to primary) onto 
// another specialized render request. This may be to the primary (if req->GetRenderTarget() is nil)
// or to a texture. This function saves enough state to resume rendering on PopRenderRequest.
// The render request may just be a new camera position.
void plDXPipeline::PushRenderRequest(plRenderRequest* req)
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

    if( req->GetFogStart() < 0 )
    {
        fView.SetDefaultFog(defFog);
    }
    else
    {
        defFog.Set(req->GetYon() * (1.f - req->GetFogStart()), req->GetYon(), 1.f, &req->GetClearColor());
        fView.SetDefaultFog(defFog);
        fCurrFog.fEnvPtr = nullptr;
    }

    if( req->GetOverrideMat() )
        PushOverrideMaterial(req->GetOverrideMat());

    // Set from our saved ones...
    fView.SetWorldToLocal(w2l);
    fView.SetLocalToWorld(l2w);

    RefreshMatrices();

    if (req->GetIgnoreOccluders())
        fView.SetMaxCullNodes(0);

    fView.fCullTreeDirty = true;
}

// PopRenderRequest //////////////////////////////////////////////////
// Restore state to resume rendering as before the preceding PushRenderRequest.
void plDXPipeline::PopRenderRequest(plRenderRequest* req)
{
    if( req->GetOverrideMat() )
        PopOverrideMaterial(nullptr);

    hsRefCnt_SafeUnRef(fView.fRenderRequest);
    fView = fViewStack.top();
    fViewStack.pop();

    // Force the next thing drawn to update the fog settings.
    fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    fCurrFog.fEnvPtr = nullptr;

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;
}


// ISetAnisotropy ///////////////////////////////////////////////////////////
// Set the current anisotropic filtering settings to D3D
void plDXPipeline::ISetAnisotropy(bool on)
{
    if( (fSettings.fMaxAnisotropicSamples <= 0) || IsDebugFlagSet(plPipeDbg::kFlagNoAnisotropy) )
        on = false;

    if( on == fSettings.fCurrAnisotropy )
        return;

    if( on )
    {
        int i;
        for( i = 0; i < 8; i++ )
        {   
            // GeForce cards have decided that they no longer handle anisotropic as a mag filter.
            // We could detect caps... but I don't think we'd notice if we just made the mag
            // filter always be linear.
            fD3DDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAXANISOTROPY, (DWORD)fSettings.fMaxAnisotropicSamples );
        }
        fSettings.fCurrAnisotropy = true;
    }
    else
    {
        int i;
        for( i = 0; i < 8; i++ )
        {
            fD3DDevice->SetSamplerState( i, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
            fD3DDevice->SetSamplerState( i, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );
        }
        fSettings.fCurrAnisotropy = false;
    }
}

//// ClearRenderTarget ////////////////////////////////////////////////////////
// Clear the current color and depth buffers. If a drawable is passed in, then
// the color buffer will be cleared by rendering that drawable.
// The depth buffer is always cleared  with a clear call.
// Clearing of depth and/or color may be turned off by setting the kRenderClearDepth 
// and kRenderClearColor bits in fView.fRenderState to false.
void plDXPipeline::ClearRenderTarget( plDrawable* d )
{
    plDrawableSpans* src = plDrawableSpans::ConvertNoRef(d);

    if( !src )
    {
        ClearRenderTarget();
        return;
    }
    // First clear the depth buffer as normal.
    if( fView.fRenderState & kRenderClearDepth )
    {
        D3DRECT r;
        bool useRect = IGetClearViewPort(r);

        if( useRect )
        {
            WEAK_ERROR_CHECK( fD3DDevice->Clear( 1, &r, D3DCLEAR_ZBUFFER, 0, fView.GetClearDepth(), 0L ) );
        }
        else
        {
            WEAK_ERROR_CHECK(fD3DDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER, 0, fView.GetClearDepth(), 0L));
// debug, clears to red         WEAK_ERROR_CHECK(fD3DDevice->Clear(0, nullptr, D3DCLEAR_ZBUFFER | D3DCLEAR_TARGET, 0xffff0000, fView.fClearDepth, 0L));
        }
    }

    uint32_t s = fView.fRenderState;
    uint32_t dtm = fView.GetDrawableTypeMask();
    uint32_t sdtm = fView.GetSubDrawableTypeMask();
    
    fView.SetDrawableTypeMask(plDrawable::kNormal);
    fView.SetSubDrawableTypeMask(uint32_t(-1));

    Draw(d);

    fView.SetSubDrawableTypeMask(sdtm);
    fView.SetDrawableTypeMask(dtm);
    fView.fRenderState = s;

}

// IGetClearViewPort //////////////////////////////////////////////
// Sets the input rect to the current viewport. Returns true if
// that is a subset of the current render target, else false.
bool plDXPipeline::IGetClearViewPort(D3DRECT& r)
{
    r.x1 = GetViewTransform().GetViewPortLeft();
    r.y1 = GetViewTransform().GetViewPortTop();
    r.x2 = GetViewTransform().GetViewPortRight();
    r.y2 = GetViewTransform().GetViewPortBottom();

    bool useRect = false;
    if (fCurrRenderTarget != nullptr)
    {
        useRect = ( (r.x1 != 0) || (r.y1 != 0) || (r.x2 != fCurrRenderTarget->GetWidth()) || (r.y2 != fCurrRenderTarget->GetHeight()) );

    }
    else
    {
        useRect = ( (r.x1 != 0) || (r.y1 != 0) || (r.x2 != fOrigWidth) || (r.y2 != fOrigHeight) );
    }

    return useRect;
}

// ClearRenderTarget //////////////////////////////////////////////////////////////////////////////
// Flat fill the current render target with the specified color and depth values.
void    plDXPipeline::ClearRenderTarget( const hsColorRGBA *col, const float* depth )
{
    if( fView.fRenderState & (kRenderClearColor | kRenderClearDepth) )
    {
        DWORD clearColor = inlGetD3DColor(col ? *col : GetClearColor());
        float clearDepth = depth ? *depth : fView.GetClearDepth();

        DWORD   dwFlags = 0;//fStencil.fDepth > 0 ? D3DCLEAR_STENCIL : 0;
        if( fView.fRenderState & kRenderClearColor )
            dwFlags |= D3DCLEAR_TARGET;
        if( fView.fRenderState & kRenderClearDepth )
            dwFlags |= D3DCLEAR_ZBUFFER;

        D3DRECT r;
        bool useRect = IGetClearViewPort(r);
        if( useRect )
        {
            WEAK_ERROR_CHECK( fD3DDevice->Clear( 1, &r, dwFlags, clearColor, clearDepth, 0L ) );
        }
        else
        {
            WEAK_ERROR_CHECK(fD3DDevice->Clear(0, nullptr, dwFlags, clearColor, clearDepth, 0L));
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
//// Fog //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// The current fog system sucks. It was never meant to get used this way, but
// the production artists started using it with debug commands that were around,
// and before they could be stopped it was too late.
// The good news is that there's a lot that could be done with fog here that
// would be greatly appreciated.

// IGetVSFogSet ///////////////////////////////////////////////////////////////
// Translate the current fog settings into a linear fog that the current
// vertex shaders can use.
void plDXPipeline::IGetVSFogSet(float* const set) const
{
    set[2] = 0.f;
    set[3] = 1.f;
    if( fCurrFog.fEnvPtr )
    {
        hsColorRGBA colorTrash;
        float start;
        float end;
        fCurrFog.fEnvPtr->GetPipelineParams(&start, &end, &colorTrash);
        if( end > start )
        {
            set[0] = -end;
            set[1] = 1.f / (start - end);
        }
        else
        {
            set[0] = 1.f;
            set[1] = 0.f;
        }
    }
    else
    {
        set[0] = 1.f;
        set[1] = 0.f;
    }
}   

//// ISetFogParameters ////////////////////////////////////////////////////////
// So looking at this function, one might guess that fog parameters were settable
// individually for different objects, and that was the original intent, with transitions
// as something like the avatar moved from one fog region to another.
// Never happened.
// So the current state is that there is one set of fog parameters per age, and things
// are either fogged, or not fogged.
// This is complicated by the DX vertex/pixel shaders only supporting per-vertex fog,
// so the same plasma fog settings may turn into differing D3D fog state.
void plDXPipeline::ISetFogParameters(const plSpan* span, const plLayerInterface* baseLay)
{
#ifndef PLASMA_EXTERNAL_RELEASE
    if (IsDebugFlagSet(plPipeDbg::kFlagNoFog))
    {
        fCurrFog.fEnvPtr = nullptr;
        fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
        return;
    }
#endif // PLASMA_EXTERNAL_RELEASE

    const plFogEnvironment* fog = (span ? (span->fFogEnvironment ? span->fFogEnvironment : &fView.GetDefaultFog()) : nullptr);

    uint8_t isVertex = 0;
    uint8_t isShader = false;
    if (baseLay)
    {
        if ((baseLay->GetShadeFlags() & hsGMatState::kShadeReallyNoFog) && !(fMatOverOff.fShadeFlags & hsGMatState::kShadeReallyNoFog))
            fog = nullptr;
        if (baseLay->GetVertexShader())
            isShader = true;
    }
    if (fMatOverOn.fShadeFlags & hsGMatState::kShadeReallyNoFog)
        fog = nullptr;

    bool forceLoad = false;
    D3DRENDERSTATETYPE  d3dFogType = D3DRS_FOGTABLEMODE;        // Use VERTEXMODE for vertex fog

    if (!(fSettings.fD3DCaps & kCapsPixelFog) || isShader)
    {
        d3dFogType = D3DRS_FOGVERTEXMODE;
        isVertex = true;
    }

    // Quick check
    if ((fCurrFog.fEnvPtr == fog) && (fCurrFog.fIsVertex == isVertex) && (fCurrFog.fIsShader == isShader))
        return;

    uint8_t type = (fog == nullptr) ? plFogEnvironment::kNoFog : fog->GetType();

    if (type == plFogEnvironment::kNoFog)
    {
        /// No fog, just disable
        fD3DDevice->SetRenderState( D3DRS_FOGENABLE, FALSE );
        fCurrFog.fEnvPtr = nullptr;
        return;
    }
    else if( fCurrFog.fEnvPtr != fog )
    {
        fD3DDevice->SetRenderState( D3DRS_FOGENABLE, TRUE );
        forceLoad = true;
        fCurrFog.fEnvPtr = fog;
    }

    if( isShader )
        type = plFogEnvironment::kLinearFog;

    if( fCurrFog.fIsShader != isShader )
        forceLoad = true;

    if( fCurrFog.fIsVertex != isVertex )
        forceLoad = true;

    fCurrFog.fIsShader = isShader;
    fCurrFog.fIsVertex = isVertex;

    float    startOrDensity, end;
    hsColorRGBA color;

    /// Get params
    if( type == plFogEnvironment::kLinearFog )
    {
        fog->GetPipelineParams( &startOrDensity, &end, &color );

        if (startOrDensity == end)
        {
            // This should be legal, but some cards don't like it. Just disable. Same thing.
            fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
            return;
        }
    }
    else
        fog->GetPipelineParams( &startOrDensity, &color );

    if( isShader )
    {
        // None of this is technically necessary, but it's to work around
        // a known goofiness in the NVidia drivers. Actually, I don't think
        // having to set the tablemode fog to linear in addition to setting
        // the vertexmode is even a "known" issue. But turns out to be 
        // necessary on GeForceFX latest drivers.
        startOrDensity = 1.f;
        end = 0.f;

        // Setting FOGTABLEMODE to none seems to work on both ATI and NVidia,
        // but I haven't tried it on the GeForceFX yet.
        //      if( fCurrFog.fMode != D3DFOG_LINEAR )
        //          fD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
        fD3DDevice->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_NONE);
    }

    /// Set color
    if( !( fCurrFog.fColor == color ) || forceLoad )
    {
        fCurrFog.fColor = color;
        fCurrFog.fHexColor = inlGetD3DColor( color );
        fD3DDevice->SetRenderState( D3DRS_FOGCOLOR, fCurrFog.fHexColor );
    }

    D3DFOGMODE          modes[ 4 ] = { D3DFOG_LINEAR, D3DFOG_EXP, D3DFOG_EXP2, D3DFOG_NONE };

    /// Set type
    if( fCurrFog.fMode != modes[type] || forceLoad )
    {
        fCurrFog.fMode = modes[type];

        if( fCurrFog.fMode == D3DFOG_LINEAR )
        {
            fCurrFog.fStart = startOrDensity;
            fCurrFog.fEnd = end;

            fD3DDevice->SetRenderState( d3dFogType, fCurrFog.fMode );
            fD3DDevice->SetRenderState( D3DRS_FOGSTART, *(DWORD *)( &fCurrFog.fStart ) );
            fD3DDevice->SetRenderState( D3DRS_FOGEND, *(DWORD *)( &fCurrFog.fEnd ) );
        }
        else
        {
            fCurrFog.fDensity = startOrDensity;

            fD3DDevice->SetRenderState( d3dFogType, fCurrFog.fMode );
            fD3DDevice->SetRenderState( D3DRS_FOGDENSITY, *(DWORD *)( &fCurrFog.fDensity ) );
        }
    }
    else
    {
        // Type is the same, but are the params?
        if( fCurrFog.fMode == D3DFOG_LINEAR )
        {
            if( fCurrFog.fStart != startOrDensity )
            {
                fCurrFog.fStart = startOrDensity;
                fD3DDevice->SetRenderState( D3DRS_FOGSTART, *(DWORD *)( &fCurrFog.fStart ) );
            }

            if( fCurrFog.fEnd != end )
            {
                fCurrFog.fEnd = end;
                fD3DDevice->SetRenderState( D3DRS_FOGEND, *(DWORD *)( &fCurrFog.fEnd ) );
            }
        }
        else
        {
            if( fCurrFog.fDensity != startOrDensity )
            {
                fCurrFog.fDensity = startOrDensity;
                fD3DDevice->SetRenderState( D3DRS_FOGDENSITY, *(DWORD *)( &fCurrFog.fDensity ) );
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
//// Stenciling ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// I know that none of this stencil code has ever been used in production.
// To my knowledge, none of this stencil code was ever even tested.
// It may save you some time as a starting point, but don't trust it.

//// StencilEnable ////////////////////////////////////////////////////////////

bool  plDXPipeline::StencilEnable( bool enable )
{
    if( fStencil.fEnabled == enable )
        return true;

    if( enable && fStencil.fDepth == 0 )
        return false;           // Can't enable stenciling when we don't support it!

    fD3DDevice->SetRenderState( D3DRS_STENCILENABLE, enable ? TRUE : FALSE );

    return true;
}

//// StencilSetCompareFunc ////////////////////////////////////////////////////

void    plDXPipeline::StencilSetCompareFunc( uint8_t func, uint32_t refValue )
{
    D3DCMPFUNC  newFunc;


    switch( func )
    {
        case plStencilCaps::kCmpNever:              newFunc = D3DCMP_NEVER; break;
        case plStencilCaps::kCmpLessThan:           newFunc = D3DCMP_LESS; break;
        case plStencilCaps::kCmpEqual:              newFunc = D3DCMP_EQUAL; break;
        case plStencilCaps::kCmpLessThanOrEqual:    newFunc = D3DCMP_LESSEQUAL; break;
        case plStencilCaps::kCmpGreaterThan:        newFunc = D3DCMP_GREATER; break;
        case plStencilCaps::kCmpNotEqual:           newFunc = D3DCMP_NOTEQUAL; break;
        case plStencilCaps::kCmpGreaterThanOrEqual: newFunc = D3DCMP_GREATEREQUAL; break;
        case plStencilCaps::kCmpAlways:             newFunc = D3DCMP_ALWAYS; break;
        default: hsAssert( false, "Invalid compare function to StencilSetCompareFunc()" ); return;
    }

    if( fStencil.fCmpFunc != newFunc )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILFUNC, newFunc );
        fStencil.fCmpFunc = newFunc;
    }

    if( fStencil.fRefValue != refValue )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILREF, refValue );
        fStencil.fRefValue = refValue;
    }
}

//// StencilSetMask ///////////////////////////////////////////////////////////

void    plDXPipeline::StencilSetMask( uint32_t mask, uint32_t writeMask )
{
    if( fStencil.fMask != mask )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILMASK, mask );
        fStencil.fMask = mask;
    }

    if( fStencil.fWriteMask != writeMask )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILWRITEMASK, writeMask );
        fStencil.fWriteMask = writeMask;
    }
}

//// StencilSetOps ////////////////////////////////////////////////////////////

void    plDXPipeline::StencilSetOps( uint8_t passOp, uint8_t failOp, uint8_t passButZFailOp )
{
    D3DSTENCILOP        op;


    /// Pass op
    switch( passOp )
    {
        case plStencilCaps::kOpKeep:        op = D3DSTENCILOP_KEEP; break;
        case plStencilCaps::kOpSetToZero:   op = D3DSTENCILOP_ZERO; break;
        case plStencilCaps::kOpReplace:     op = D3DSTENCILOP_REPLACE; break;
        case plStencilCaps::kOpIncClamp:    op = D3DSTENCILOP_INCRSAT; break;
        case plStencilCaps::kOpDecClamp:    op = D3DSTENCILOP_DECRSAT; break;
        case plStencilCaps::kOpInvert:      op = D3DSTENCILOP_INVERT; break;
        case plStencilCaps::kOpIncWrap:     op = D3DSTENCILOP_INCR; break;
        case plStencilCaps::kOpDecWrap:     op = D3DSTENCILOP_DECR; break;
        default: hsAssert( false, "Invalid op to StencilSetOps()" ); return;
    }

    if( fStencil.fPassOp != op )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILPASS, op );
        fStencil.fPassOp = op;
    }

    /// Fail op
    switch( failOp )
    {
        case plStencilCaps::kOpKeep:        op = D3DSTENCILOP_KEEP; break;
        case plStencilCaps::kOpSetToZero:   op = D3DSTENCILOP_ZERO; break;
        case plStencilCaps::kOpReplace:     op = D3DSTENCILOP_REPLACE; break;
        case plStencilCaps::kOpIncClamp:    op = D3DSTENCILOP_INCRSAT; break;
        case plStencilCaps::kOpDecClamp:    op = D3DSTENCILOP_DECRSAT; break;
        case plStencilCaps::kOpInvert:      op = D3DSTENCILOP_INVERT; break;
        case plStencilCaps::kOpIncWrap:     op = D3DSTENCILOP_INCR; break;
        case plStencilCaps::kOpDecWrap:     op = D3DSTENCILOP_DECR; break;
        default: hsAssert( false, "Invalid op to StencilSetOps()" ); return;
    }

    if( fStencil.fFailOp != op )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILFAIL, op );
        fStencil.fFailOp = op;
    }

    /// Pass-but-z-fail op
    switch( passButZFailOp )
    {
        case plStencilCaps::kOpKeep:        op = D3DSTENCILOP_KEEP; break;
        case plStencilCaps::kOpSetToZero:   op = D3DSTENCILOP_ZERO; break;
        case plStencilCaps::kOpReplace:     op = D3DSTENCILOP_REPLACE; break;
        case plStencilCaps::kOpIncClamp:    op = D3DSTENCILOP_INCRSAT; break;
        case plStencilCaps::kOpDecClamp:    op = D3DSTENCILOP_DECRSAT; break;
        case plStencilCaps::kOpInvert:      op = D3DSTENCILOP_INVERT; break;
        case plStencilCaps::kOpIncWrap:     op = D3DSTENCILOP_INCR; break;
        case plStencilCaps::kOpDecWrap:     op = D3DSTENCILOP_DECR; break;
        default: hsAssert( false, "Invalid op to StencilSetOps()" ); return;
    }

    if( fStencil.fPassButZFailOp != op )
    {
        fD3DDevice->SetRenderState( D3DRS_STENCILZFAIL, op );
        fStencil.fPassButZFailOp = op;
    }
}

//// StencilGetCaps ///////////////////////////////////////////////////////////

bool  plDXPipeline::StencilGetCaps( plStencilCaps *caps )
{
    hsAssert(caps != nullptr, "Invalid pointer to StencilGetCaps()");

    int     i;

    
    /// Find supported depths
    caps->fSupportedDepths = 0;
    for( i = 0; i < fCurrentMode->fDepthFormats.GetCount(); i++ )
    {
        switch( fCurrentMode->fDepthFormats[ i ] )
        {
            case D3DFMT_D15S1:      caps->fSupportedDepths |= plStencilCaps::kDepth1Bit; break;
            case D3DFMT_D24X4S4:    caps->fSupportedDepths |= plStencilCaps::kDepth4Bits; break;
            case D3DFMT_D24S8:      caps->fSupportedDepths |= plStencilCaps::kDepth8Bits; break;
        }
    }

    if( caps->fSupportedDepths == 0 )
    {
        caps->fIsSupported = false;
        return false;
    }

    /// Get supported ops
    caps->fSupportedOps = 0;

    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_DECR )
        caps->fSupportedOps |= plStencilCaps::kOpDecWrap;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_DECRSAT )
        caps->fSupportedOps |= plStencilCaps::kOpDecClamp;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_INCR )
        caps->fSupportedOps |= plStencilCaps::kOpIncWrap;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_INCRSAT )
        caps->fSupportedOps |= plStencilCaps::kOpIncClamp;

    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_INVERT )
        caps->fSupportedOps |= plStencilCaps::kOpInvert;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_KEEP )
        caps->fSupportedOps |= plStencilCaps::kOpKeep;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_REPLACE )
        caps->fSupportedOps |= plStencilCaps::kOpReplace;
    if( fCurrentDevice->fDDCaps.StencilCaps & D3DSTENCILCAPS_ZERO )
        caps->fSupportedOps |= plStencilCaps::kOpSetToZero;

    return true;
}


///////////////////////////////////////////////////////////////////////////////
//// Lighting /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IMakeLightRef ////////////////////////////////////////////////////////////
// Create a plasma device ref for a light. Includes reserving a D3D light
// index for the light. Ref is kept in a linked list for ready disposal
// as well as attached to the light.
hsGDeviceRef    *plDXPipeline::IMakeLightRef( plLightInfo *owner )
{
    plDXLightRef    *lRef = new plDXLightRef();

    /// Assign stuff and update
    lRef->fD3DIndex = fLights.ReserveD3DIndex();
    lRef->fOwner = owner;
    owner->SetDeviceRef( lRef );
    // Unref now, since for now ONLY the BG owns the ref, not us (not until we use it, at least)
    hsRefCnt_SafeUnRef( lRef );

    lRef->Link( &fLights.fRefList );

    lRef->UpdateD3DInfo( fD3DDevice, &fLights );

    // Neutralize it until we need it.
    fD3DDevice->LightEnable(lRef->fD3DIndex, false);

    return lRef;
}

//// RegisterLight ////////////////////////////////////////////////////////////
// Register a light with the pipeline. Light become immediately
// ready to illuminate the scene.
void plDXPipeline::RegisterLight(plLightInfo* liInfo)
{
    pl3DPipeline::RegisterLight(liInfo);
    liInfo->SetDeviceRef( IMakeLightRef( liInfo ) );
    fLights.fTime++;
}

//// UnRegisterLight //////////////////////////////////////////////////////////
// Remove a light from the pipeline's active light list. Light will
// no longer illuminate the scene.
void plDXPipeline::UnRegisterLight(plLightInfo* liInfo)
{
    pl3DPipeline::UnRegisterLight(liInfo);

    fLights.fTime++;
}

//// IEnableLights ////////////////////////////////////////////////////////////
//  Does the lighting enable pass. Given a span with lights to use, builds
//  a bit vector representing the lights to use, then uses that to mask off
//  which lights actually need to be enabled/disabled.
// Constructs 2 lists on the span, one for normal lights, and one for projective lights.

void    plDXPipeline::IEnableLights( plSpan *span )
{
    plProfile_BeginTiming(SelectLights);
    ISelectLights( span, fMaxNumLights, false );
    plProfile_EndTiming(SelectLights);
    if( !(fView.fRenderState & kRenderNoProjection) )
    {
        plProfile_BeginTiming(SelectProj);
        ISelectLights( span, fMaxNumProjectors, true );
        plProfile_EndTiming(SelectProj);
    }
}

// ISelectLights ///////////////////////////////////////////////////////////////
// Find the strongest numLights lights to illuminate the span with.
// Weaker lights are faded out in effect so they won't pop when the
// strongest N changes membership.
void    plDXPipeline::ISelectLights( plSpan *span, int numLights, bool proj )
{
    int                 i, startScale;
    static hsBitVector  newFlags;   
    static hsTArray<plLightInfo*>   onLights;
    plDXLightRef        *ref;
    float               threshhold, overHold = 0.3f, scale;

    /// Build new flags

    /// Step 1: Find the n strongest lights
    newFlags.Clear();
    onLights.SetCount(0);

    if  (!IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagNoApplyProjLights) && proj) &&
        !(IsDebugFlagSet(plPipeDbg::kFlagOnlyApplyProjLights) && !proj))
    {
        hsTArray<plLightInfo*>& spanLights = span->GetLightList(proj);

        for( i = 0; i < spanLights.GetCount() && i < numLights; i++ )
        {
            ref = (plDXLightRef *)spanLights[i]->GetDeviceRef();

            if( ref->IsDirty() )
            {
                if( ref->fD3DIndex == 0 )
                    ref->fD3DIndex = fLights.ReserveD3DIndex();
                ref->UpdateD3DInfo( fD3DDevice, &fLights );
                ref->SetDirty( false );
            }

            newFlags.SetBit( ref->fD3DIndex );
            onLights.Append(spanLights[i]);
        }
        startScale = i;

        /// Attempt #2: Take some of the n strongest lights (below a given threshhold) and
        /// fade them out to nothing as they get closer to the bottom. This way, they fade
        /// out of existence instead of pop out.

        if( i < spanLights.GetCount() - 1 && i > 0 )
        {
            threshhold = span->GetLightStrength( i, proj );
            i--;
            overHold = threshhold * 1.5f;
            if( overHold > span->GetLightStrength( 0, proj ) )
                overHold = span->GetLightStrength( 0, proj );

            for( ; i > 0 && span->GetLightStrength( i, proj ) < overHold; i-- )
            {
                scale = ( overHold - span->GetLightStrength( i, proj ) ) / ( overHold - threshhold );

                ref = (plDXLightRef *)spanLights[i]->GetDeviceRef();

                IScaleD3DLight( ref, (1 - scale) * span->GetLightScale(i, proj) );
            }
            startScale = i + 1;
        }

        /// Make sure those lights that aren't scaled....aren't
        for( i = 0; i < startScale; i++ )
        {
            ref = (plDXLightRef *)spanLights[i]->GetDeviceRef();
            IScaleD3DLight(ref, span->GetLightScale(i, proj) );
        }

    }

    // If these are non-projected lights, go ahead and enable them.
    // For the projected lights, don't enable, just remember who they are.
    if( !proj )
    {
        // A little change here. Some boards get sticky about exactly
        // how many lights you have enabled, whether you are currently
        // rendering or not. So if we go through enabling the lights
        // we want and disabling the ones we don't, then even though
        // at the end of the loop, less than MaxNumLights are enabled,
        // we can still wind up screwed.
        // Think about if we have 8 lights enabled, and they all happen
        // to be at the end of fLights. Now we want to enable a different
        // 8 lights, which happen to be at the beginning of the list.
        // So we loop through and enable the lights we want, and then later
        // in the loop disable the lights we don't want. Problem is that
        // when we were enabling the ones we want we went over our 8 light
        // limit, and some boards (ATI) react by ignoring the enable request.
        // So then we disable the other lights at the end of the loop, but
        // it's too late because our enable requests at the beginning of the
        // loop were ignored.
        // Solution is to go through the list twice, first disabling, then
        // enabling. mf
        hsBitVector newOff = fLights.fEnabledFlags - newFlags;
        hsBitIterator iterOff(newOff);
        for( iterOff.Begin(); !iterOff.End(); iterOff.Advance() )
            fD3DDevice->LightEnable(iterOff.Current(), false);

        hsBitVector newOn = newFlags - fLights.fEnabledFlags;
        hsBitIterator iterOn(newOn);
        for( iterOn.Begin(); !iterOn.End(); iterOn.Advance() )
            fD3DDevice->LightEnable(iterOn.Current(), true);
        fLights.fEnabledFlags = newFlags;
    }
    else
    {
        fLights.fProjAll.clear();
        fLights.fProjEach.clear();
        for( i = 0; i < onLights.GetCount(); i++ )
        {
            if( onLights[i]->OverAll() )
                fLights.fProjAll.emplace_back(onLights[i]);
            else
                fLights.fProjEach.emplace_back(onLights[i]);
        }
        onLights.SetCount(0);
    }
}

// IDisableSpanLights /////////////////////////////////////////////////////
// Disable all the enabled lights, remembering which they are for
// quick reenabling.
void plDXPipeline::IDisableSpanLights()
{
    int i;
    for( i = 0; i < fLights.fLastIndex + 1; i++ )
    {
        if( fLights.fEnabledFlags.IsBitSet(i) )
        {
            fD3DDevice->LightEnable(i, false);
            fLights.fHoldFlags.SetBit(i);
        }
    }
    fLights.fEnabledFlags.Clear();
}

// IRestoreSpanLights //////////////////////////////////////////////////////
// Re-enable all the lights disabled by the matching IDisableSpanLights.
void plDXPipeline::IRestoreSpanLights()
{
    int i;
    for( i = 0; i < fLights.fLastIndex + 1; i++ )
    {
        if( fLights.fHoldFlags.IsBitSet(i) )
        {
            fD3DDevice->LightEnable(i, true);
            fLights.fEnabledFlags.SetBit(i);
        }
    }
    fLights.fHoldFlags.Clear();
}

//// IScaleD3DLight ///////////////////////////////////////////////////////////
// Scale the D3D light by the given scale factor, used for fading lights
// in and out by importance.
void    plDXPipeline::IScaleD3DLight( plDXLightRef *ref, float scale )
{
    scale = int(scale * 1.e1f) * 1.e-1f;
    if( ref->fScale != scale )
    {
        D3DLIGHT9       light = ref->fD3DInfo;


        light.Diffuse.r *= scale;
        light.Diffuse.g *= scale;
        light.Diffuse.b *= scale;

        light.Ambient.r *= scale;
        light.Ambient.g *= scale;
        light.Ambient.b *= scale;

        light.Specular.r *= scale;
        light.Specular.g *= scale;
        light.Specular.b *= scale;

        fD3DDevice->SetLight( ref->fD3DIndex, &light );
        ref->fScale = scale;
    }
}

// inlPlToDWORDColor /////////////////////////////////////////////////
// Convert a plasma floating point color to a D3D DWORD color
static inline DWORD inlPlToDWORDColor(const hsColorRGBA& c)
{
    return (DWORD(c.a * 255.99f) << 24)
        | (DWORD(c.r * 255.99f) << 16)
        | (DWORD(c.g * 255.99f) << 8)
        | (DWORD(c.b * 255.99f) << 0);
}

// inlPlToD3DColor ////////////////////////////////////////////////////
// Convert a plasma floating point color to a D3D floating point color.
inline D3DCOLORVALUE plDXPipeline::inlPlToD3DColor(const hsColorRGBA& c, float a) const
{ 
    D3DCOLORVALUE ret; 
    ret.r = c.r; 
    ret.g = c.g; 
    ret.b = c.b; 
    ret.a = a; 
    return ret; 
}

// inlEnsureLightingOn ////////////////////////////////////////////////
// Turn D3D lighting on if it isn't already.
inline void plDXPipeline::inlEnsureLightingOn()
{
    if( !fCurrD3DLiteState )
    {
        fD3DDevice->SetRenderState( D3DRS_LIGHTING, TRUE );
        fCurrD3DLiteState = true;
    }
}

// inlEnsureLightingOff ///////////////////////////////////////////////
// Turn D3D lighting off if it isn't already.
inline void plDXPipeline::inlEnsureLightingOff()
{
    if( fCurrD3DLiteState )
    { 
        fD3DDevice->SetRenderState( D3DRS_LIGHTING, FALSE );
        fCurrD3DLiteState = false;
    }
}

// ColorMul ///////////////////////////////////////////////////////////
// Multiply a D3D floating point color by a plasma floating point color,
// returning the result as a D3D floating point color.
static inline D3DCOLORVALUE ColorMul(const D3DCOLORVALUE& c0, const hsColorRGBA& c1)
{
    D3DCOLORVALUE ret;
    ret.r = c0.r * c1.r;
    ret.g = c0.g * c1.g;
    ret.b = c0.b * c1.b;
    ret.a = c0.a * c1.a;

    return ret;
}

//// ICalcLighting ////////////////////////////////////////////////////////////
// Kind of misnamed. Sets the D3D material lighting model based on what we're
// currently doing.
void    plDXPipeline::ICalcLighting( const plLayerInterface *currLayer, const plSpan *currSpan )
{
    D3DMATERIAL9    mat;
    static float diffScale = 1.f;
    static float ambScale = 1.f;
    uint32_t          props;


    plProfile_Inc(MatLightState);

            /// New (temporary) lighting method:
            /// The vertices now include the following:
            ///     diffuse = maxVertexColor * matDiffuse + matAmbient
            ///     specular = ( maxLighting + maxIllum ) * matDiffuse + matAmbient
            /// And we want the lighting set up like:
            ///     L = I*v1 + v2 + (sigma)(light stuff * v3 + 0)
            /// Where I = 0 for now (will be the environmental light constant eventually),
            /// v1 is the diffuse vertex color and v2 is the specular vertex color.
            /// So it basically translates into:
            ///     D3D ambient color = diffuse vertex color
            ///     D3D ambient constant = environmental light constant (0 for now)
            ///     D3D emissive color = specular vertex color
            ///     D3D diffuse color = diffuse vertex color

    /// We now provide three lighting equations at the pipeline's disposal:
    ///     Material: (the one we all know and love)
    ///             MATd * VTXd + MATa + <sigma of lighting w/ MATd>
    ///     Vtx preshaded: (particle systems)
    ///             MATa * VTXd + 0 + <sigma of lighting w/ VTXd>
    ///     Vtx non-preshaded:
    ///             white * VTXd + MATa + <sigma of lighting w/ VTXd>
    /// We also have a few more for shadows and such, which are handled individually
    
    memset( &mat, 0, sizeof( mat ) );

    /// Normal rendering--select the right lighting equation
    if (IsDebugFlagSet(plPipeDbg::kFlagAllBright))
    {
        inlEnsureLightingOn();
        mat.Diffuse.r = mat.Diffuse.g = mat.Diffuse.b = mat.Diffuse.a = 1.f;
        mat.Ambient.r = mat.Ambient.g = mat.Ambient.b = mat.Ambient.a = 1.f;
        mat.Emissive.r = mat.Emissive.g = mat.Emissive.b = mat.Emissive.a = 1.f;
        fD3DDevice->SetMaterial( &mat );
        fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );
        return;
    }
    
    props = (currSpan != nullptr) ? (currSpan->fProps & plSpan::kLiteMask) : plSpan::kLiteMaterial;
    
    if( fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpChans )
    {
        props = plSpan::kLiteMaterial;
        fLayerState[0].fShadeFlags |= hsGMatState::kShadeNoShade | hsGMatState::kShadeWhite;
    }
    /// Select one of our three lighting methods
    switch( props )
    {
    case plSpan::kLiteMaterial:     // Material shading
        
        ///     Material: (the one we all know and love)
        ///             MATd * VTXd + MATa + <sigma of lighting w/ MATd>
        
        inlEnsureLightingOn();
        
        // D3D ambient - give it our material static diffuse, since it will be multiplied by the vertex color
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeWhite )
        {
            mat.Ambient.r = mat.Ambient.g = mat.Ambient.b = diffScale;
            mat.Ambient.a = 1.f;
            
        }
        else if (IsDebugFlagSet(plPipeDbg::kFlagNoPreShade))
        {
            mat.Ambient.r = mat.Ambient.g = mat.Ambient.b = 0;
            mat.Ambient.a = 1.f;
        }
        else
            mat.Ambient = inlPlToD3DColor(currLayer->GetPreshadeColor() * diffScale, 1.f);
        
        // D3D diffuse - give it our runtime material diffuse
        mat.Diffuse = inlPlToD3DColor(currLayer->GetRuntimeColor() * diffScale, currLayer->GetOpacity());
        
        // D3D emissive - give it our material ambient
        mat.Emissive = inlPlToD3DColor(currLayer->GetAmbientColor() * ambScale, 1.f);
        
        // Set specular properties
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeSpecular )
        {
            mat.Specular = inlPlToD3DColor( currLayer->GetSpecularColor(), 1.f);
            mat.Power = currLayer->GetSpecularPower();
        }
        
        fD3DDevice->SetMaterial( &mat );
        fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
        
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeWhite )
            fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff );
        else
            fD3DDevice->SetRenderState( D3DRS_AMBIENT, inlGetD3DColor( *(hsColorRGBA*)&mat.Ambient ) );
        
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeNoShade )
            fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
        else
            fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
        
        fCurrLightingMethod = plSpan::kLiteMaterial;
        break;
        
    case plSpan::kLiteVtxPreshaded:  // Vtx preshaded
        //              MATa * VTXd  + 0     + <sigma of lighting w/ VTXd>
        // Mapping to:  GLa  * AMSrc + EMSrc + <.....................DMSrc> 
        
#if 0 // PARTICLESHADE
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeEmissive )
        {
            inlEnsureLightingOff();
        }
        else
        {
            inlEnsureLightingOn();
            
            // Set a black material (we ONLY care about vertex color when doing particles, 
            //                       er I mean, vtxPreshaded)
            fD3DDevice->SetMaterial( &mat );
            
            fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
            fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
            fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0 );
            
            fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
            fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
        }
#else // PARTICLESHADE
        inlEnsureLightingOn();
        
        // MATa * white + 0 + <sigma of lighting with VTXd>
        

        fD3DDevice->SetMaterial( &mat );
        
        fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
        fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0 );
        
        fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );

        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeEmissive )
            fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 );
        else
            fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );

#endif // PARTICLESHADE
        
        fCurrLightingMethod = plSpan::kLiteVtxPreshaded;
        break;
        
        
    case plSpan::kLiteVtxNonPreshaded:      // Vtx non-preshaded
        //              white * VTXd + MATa  + <sigma of lighting w/ VTXd>
        // Mapping to:  GLa  * AMSrc + EMSrc + <.....................DMSrc>
        
        inlEnsureLightingOn();
        
        // D3D emissive - give it our material ambient
        mat.Emissive = inlPlToD3DColor(currLayer->GetAmbientColor() * ambScale, 1.f);
        
        // Set specular properties
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeSpecular )
        {
            mat.Specular = inlPlToD3DColor( currLayer->GetSpecularColor(), 1.f);
            mat.Power = currLayer->GetSpecularPower();
        }
        fD3DDevice->SetMaterial( &mat );
        
        // Lightmaps want WHITE here, otherwise we want BLACK
        DWORD preShadeStrength;
        preShadeStrength = inlPlToDWORDColor(currLayer->GetPreshadeColor());
        fD3DDevice->SetRenderState(D3DRS_AMBIENT, preShadeStrength);
        
        fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 );
        fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1 );
        fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );
        fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
        
        fCurrLightingMethod = plSpan::kLiteVtxNonPreshaded;
        break;
        
    default:
        hsAssert( false, "Bad lighting type" );
        break;
    }

}

///////////////////////////////////////////////////////////////////////////////
//// plDXLightSettings Functions /////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//// Reset ////////////////////////////////////////////////////////////////////
//  Sets member variables to initial states. 

void    plDXLightSettings::Reset( plDXPipeline *pipe )
{
    Release();

    fUsedFlags.Clear();
    fEnabledFlags.Clear();
    fHoldFlags.Clear();
    fProjEach.clear();
    fProjAll.clear();
    fShadowLights.clear();
    fNextIndex = 1;     /// Light 0 is reserved
    fLastIndex = 1;
    fTime = 0;
    fRefList = nullptr;
    fPipeline = pipe;
}

//// Release //////////////////////////////////////////////////////////////////
//  Releases/deletes anything associated with these settings.
// This includes unregistering all lights.
void    plDXLightSettings::Release()
{
    plDXLightRef    *ref;

    fProjEach.clear();
    fProjAll.clear();

    while( fRefList )
    {
        ref = fRefList;
        ref->Release();
        ref->Unlink();
    }

    for (plDXLightRef*& shadowLight : fShadowLights.pool())
    {
        hsRefCnt_SafeUnRef(shadowLight);
        shadowLight = nullptr;
    }
    fShadowLights.release_and_clear();
}

//// ReserveD3DIndex //////////////////////////////////////////////////////////
//  Reserve a D3D light index.

uint32_t  plDXLightSettings::ReserveD3DIndex()
{
    for( ; fNextIndex < (uint32_t)-1; fNextIndex++ )
    {
        if( !fUsedFlags.IsBitSet( fNextIndex ) )
            break;
    }

    fUsedFlags.SetBit( fNextIndex );
    fEnabledFlags.ClearBit( fNextIndex );   // Ensure it's cleared
    fHoldFlags.ClearBit( fNextIndex );
    if( fNextIndex > fLastIndex )
        fLastIndex = fNextIndex;

    return fNextIndex;
}

//// ReleaseD3DIndex //////////////////////////////////////////////////////////
//  Release a reserved D3D light index to be reused.

void    plDXLightSettings::ReleaseD3DIndex( uint32_t idx )
{
    fUsedFlags.SetBit( idx, false );
    if( fNextIndex > idx )
        fNextIndex = idx;       // Forces search to start here next time

    // Dec down fLastIndex
    while( fLastIndex > 0 && !fUsedFlags.IsBitSet( fLastIndex ) )
        fLastIndex--;

    if( fNextIndex > fLastIndex )
        fNextIndex = fLastIndex;
}


///////////////////////////////////////////////////////////////////////////////
//// Materials ////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// ISetLayer ////////////////////////////////////////////////////////////////
// Sets whether we're rendering a base layer or upper layer. Upper layer has
// a Z bias to avoid Z fighting.
void    plDXPipeline::ISetLayer( uint32_t lay )
{
    if( lay )
    {
        if( fCurrRenderLayer != lay )
        {
            fCurrRenderLayer = lay;

            plCONST(int) kBiasMult = 8;
            if( !( fSettings.fD3DCaps & kCapsZBias ) )
                IProjectionMatrixToDevice();
            else
                fD3DDevice->SetRenderState( D3DRS_DEPTHBIAS, kBiasMult * fCurrRenderLayer );
        }
    }
    else
        IBottomLayer();
}

//// IBottomLayer /////////////////////////////////////////////////////////////
// Turn off any Z bias.
void    plDXPipeline::IBottomLayer()
{
    if( fCurrRenderLayer != 0 )
    {
        fCurrRenderLayer = 0;
        if( !( fSettings.fD3DCaps & kCapsZBias ) )
            IProjectionMatrixToDevice();
        else
            fD3DDevice->SetRenderState( D3DRS_DEPTHBIAS, 0 );
    }
}

// Special effects /////////////////////////////////////////////////////////////

// IPushOverBaseLayer /////////////////////////////////////////////////////////
// Sets fOverBaseLayer (if any) as a wrapper on top of input layer.
// This allows the OverBaseLayer to intercept and modify queries of
// the real current layer's properties (e.g. color or state).
// fOverBaseLayer is set to only get applied to the base layer during
// multitexturing.
// Must be matched with call to IPopOverBaseLayer.
plLayerInterface* plDXPipeline::IPushOverBaseLayer(plLayerInterface* li)
{
    if( !li )
        return nullptr;

    fOverLayerStack.Push(li);

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
plLayerInterface* plDXPipeline::IPopOverBaseLayer(plLayerInterface* li)
{
    if( !li )
        return nullptr;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.Pop();
    fOverBaseLayer = fOverBaseLayer->Detach(pop);

    return pop;
}

// IPushOverAllLayer ///////////////////////////////////////////////////
// Push fOverAllLayer (if any) as wrapper around the input layer.
// fOverAllLayer is set to be applied to each layer during multitexturing.
// Must be matched by call to IPopOverAllLayer
plLayerInterface* plDXPipeline::IPushOverAllLayer(plLayerInterface* li)
{
    if( !li )
        return nullptr;

    fOverLayerStack.Push(li);

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
plLayerInterface* plDXPipeline::IPopOverAllLayer(plLayerInterface* li)
{
    if( !li )
        return nullptr;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.Pop();
    fOverAllLayer = fOverAllLayer->Detach(pop);

    return pop;
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
int plDXPipeline::ISetNumActivePiggyBacks()
{
    return fActivePiggyBacks = std::min(static_cast<int>(fMaxPiggyBacks), fPiggyBackStack.GetCount());
}

// IPushProjPiggyBack //////////////////////////////////////////////////
// Push a projected texture on as a piggy back.
void plDXPipeline::IPushProjPiggyBack(plLayerInterface* li)
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.Push(li);
    fActivePiggyBacks = fPiggyBackStack.GetCount() - fMatPiggyBacks;
    fForceMatHandle = true;
}

// IPopProjPiggyBacks /////////////////////////////////////////////////
// Remove a projected texture from use as a piggy back.
void plDXPipeline::IPopProjPiggyBacks()
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.SetCount(fMatPiggyBacks);
    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

// IPushPiggyBacks ////////////////////////////////////////////////////
// Push any piggy backs associated with a material, presumed to
// be a light map because that's all they are used for.
// Matched with IPopPiggyBacks
void plDXPipeline::IPushPiggyBacks(hsGMaterial* mat)
{
    hsAssert(!fMatPiggyBacks, "Push/Pop Piggy mismatch");

    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    for (size_t i = 0; i < mat->GetNumPiggyBacks(); i++)
    {
        if( !mat->GetPiggyBack(i) )
            continue;

        if ((mat->GetPiggyBack(i)->GetMiscFlags() & hsGMatState::kMiscLightMap)
            && IsDebugFlagSet(plPipeDbg::kFlagNoLightmaps))
            continue;

        fPiggyBackStack.Push(mat->GetPiggyBack(i));
        fMatPiggyBacks++;
    }
    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

// IPopPiggyBacks ///////////////////////////////////////////////////////
// Pop any current piggy backs set from IPushPiggyBacks.
// Matches IPushPiggyBacks.
void plDXPipeline::IPopPiggyBacks()
{
    if( fView.fRenderState & plPipeline::kRenderNoPiggyBacks )
        return;

    fPiggyBackStack.SetCount(fPiggyBackStack.GetCount() - fMatPiggyBacks);
    fMatPiggyBacks = 0;

    ISetNumActivePiggyBacks();
    fForceMatHandle = true;
}

//// IHandleMaterial //////////////////////////////////////////////////////////
//  Takes the starting "layer" and uses as many layers as possible in the given
//  material and sets up the device to draw with it. Returns the first layer
//  index not yet used. (I.e. if we ate layers 0 and 1, it'll return 2). 
// A return value of -1 means don't bother rendering.

int32_t   plDXPipeline::IHandleMaterial( hsGMaterial *newMat, uint32_t layer, const plSpan *currSpan )
{
    // No material means no draw.
    if( !newMat && newMat->GetLayer(layer) )
        return -1;

    // If this is a bump mapping pass but the object isn't currently runtime lit, just skip.
    // Note that <layer> may change here, if we're skipping past the bump layers but there
    // are more layers (passes) to do after that.
    if( ISkipBumpMap(newMat, layer, currSpan) )
    {
        return -1;
    }

    // Workaround for the ATI Radeon 7500's inability to use uvw coordinates above 1.
    // If we have a layer trying to use uvw 2 or higher, skip it and any layers bound to
    // it.
    while( (layer < newMat->GetNumLayers()) 
        && newMat->GetLayer(layer) 
        && ((newMat->GetLayer(layer)->GetUVWSrc() & 0xf) > fSettings.fMaxUVWSrc) )
    {
        if( newMat->GetLayer(layer)->GetMiscFlags() & hsGMatState::kMiscBindNext )
            layer++;
        layer++;
    }
    if( layer >= newMat->GetNumLayers() )
        return -1;

    // If nothing has changed, we don't need to recompute and set state.
    if( !fForceMatHandle && (newMat == fCurrMaterial && layer == fCurrLayerIdx) )
    {
        // Before returning, check if we have to redo our lighting
        uint32_t lightType = (currSpan != nullptr) ? (currSpan->fProps & plSpan::kLiteMask) : plSpan::kLiteMaterial;
        if( lightType != fCurrLightingMethod )
            ICalcLighting( fCurrLay, currSpan );    
        
        if( fLayerState[0].fMiscFlags & (hsGMatState::kMiscBumpDu|hsGMatState::kMiscBumpDw) )
            ISetBumpMatrices(fCurrLay, currSpan);

        return layer + fCurrNumLayers;
    }

    fForceMatHandle = false;

    fCurrLayerIdx = layer;
//  fCurrNumLayers = newMat->GetNumLayers();

    if (newMat != fCurrMaterial)
        plProfile_Inc(MatChange);
    plProfile_Inc(LayChange);

    /// Test for fail states
    if (IsDebugFlagSet(plPipeDbg::kFlagNoDecals) && (newMat->GetCompositeFlags() & hsGMaterial::kCompDecal))
    {
        return -1;
    }

    /// Workaround for a D3D limitation--you're not allowed to render with a texture that you're
    /// rendering INTO. Hence we can't have self-reflecting cubicRenderTargets (damn)
    if (fCurrBaseRenderTarget != nullptr &&
        newMat->GetLayer( layer )->GetTexture() == plBitmap::ConvertNoRef( fCurrBaseRenderTarget ) )
    {
        return -1;
    }

    /// Figure out our current states
    // Start with the base layer.
    plLayerInterface    *currLay = IPushOverBaseLayer(newMat->GetLayer(layer));
    
    if (IsDebugFlagSet(plPipeDbg::kFlagBumpW) && (currLay->GetMiscFlags() & hsGMatState::kMiscBumpDu) )
        currLay = newMat->GetLayer(fCurrLayerIdx = ++layer);

    currLay = IPushOverAllLayer(currLay);

    /// Save stuff for next time around
    ICompositeLayerState(0, currLay);
    hsRefCnt_SafeAssign( fCurrMaterial, newMat );
    fCurrLayerIdx = layer;
    fCurrLay = currLay;

    if (IsDebugFlagSet(plPipeDbg::kFlagDisableSpecular))
        fLayerState[0].fShadeFlags &= ~hsGMatState::kShadeSpecular;

    // ZIncLayer requests Z bias for upper layers.
    if( fLayerState[0].fZFlags & hsGMatState::kZIncLayer )
        ISetLayer( 1 );
    else
        IBottomLayer(); 
        
    /// A few debugging things
    if (IsDebugFlagSet(plPipeDbg::kFlagNoAlphaBlending))
        fLayerState[0].fBlendFlags &= ~hsGMatState::kBlendMask;

    if ((IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpChans) )
    {
        switch( fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpChans )
        {
        case hsGMatState::kMiscBumpDu:
            break;
        case hsGMatState::kMiscBumpDv:
            if( !(fCurrMaterial->GetLayer(layer-2)->GetBlendFlags() & hsGMatState::kBlendAdd) )
            {
                fLayerState[0].fBlendFlags &= ~hsGMatState::kBlendMask;
                fLayerState[0].fBlendFlags |= hsGMatState::kBlendMADD;
            }
            break;
        case hsGMatState::kMiscBumpDw:
            if( !(fCurrMaterial->GetLayer(layer-1)->GetBlendFlags() & hsGMatState::kBlendAdd) )
            {
                fLayerState[0].fBlendFlags &= ~hsGMatState::kBlendMask;
                fLayerState[0].fBlendFlags |= hsGMatState::kBlendMADD;
            }
            break;
        default:
            break;
        }
    }

    /// Get the # of layers we can draw in this pass into fCurrNumLayers
    int oldNumLayers = fCurrNumLayers;
    ILayersAtOnce( newMat, layer );
    if( oldNumLayers != fCurrNumLayers )
    {
        // This hack is necessary to cover a hack necessary to cover a "limitation" in the GeForce2 drivers.
        // Basically, we have to handle NoTexAlpha/Color differently if it's stage 1 than other stages,
        // so even though the BlendFlags haven't changed, the calls to D3D are different. Another
        // way to handle this would be to have a different handler based on whether we are 2 TMU limited
        // or not, but whatever.
        if( fLayerState[1].fBlendFlags & (hsGMatState::kBlendNoTexAlpha | hsGMatState::kBlendNoTexColor) )
            fLayerState[1].fBlendFlags = uint32_t(-1);
    }

    // Placed here, since it's material-dependent (or more accurately, current-layer-dependent)
    ICalcLighting( currLay, currSpan ); 

    // If we're bump mapping, compute the texture transforms.
    if( fLayerState[0].fMiscFlags & (hsGMatState::kMiscBumpDu|hsGMatState::kMiscBumpDw) )
        ISetBumpMatrices(currLay, currSpan);

    /// Transfer states to D3D now
    IHandleFirstTextureStage( currLay );

    currLay = IPopOverAllLayer(currLay);
    currLay = IPopOverBaseLayer(currLay);
    fCurrLay = currLay;

    int nextLayer = fCurrLayerIdx + fCurrNumLayers;
    if (IsDebugFlagSet(plPipeDbg::kFlagBumpW) && (fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpDw) )
    {
        // Bump mapping approximation using only the W (normal direction) component of lighting.
        plLayerInterface* layPtr = IPushOverAllLayer(newMat->GetLayer(fCurrLayerIdx + 2));
        if( !layPtr )
            return -1;
        ICompositeLayerState(1, layPtr);
        IHandleTextureStage( 1, layPtr );
        layPtr = IPopOverAllLayer(layPtr);
        nextLayer = fCurrLayerIdx + 3;
    }
    else if (IsDebugFlagSet(plPipeDbg::kFlagBumpUV) && (fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpDu) )
    {
        // Bump mapping approximation using only the UV (surface tangent directions) component of lighting.
        plLayerInterface* layPtr = IPushOverAllLayer(newMat->GetLayer(fCurrLayerIdx + 3));
        if( !layPtr )
            return -1;
        ICompositeLayerState(1, layPtr);
        IHandleTextureStage( 1, layPtr );
        layPtr = IPopOverAllLayer(layPtr);
        nextLayer = fCurrLayerIdx + 2;
    }
    else
    {
        // Normal multi texturing.
        /// Loop through all multitexturing layers
        int i;
        if( fView.fRenderState & plPipeline::kRenderBaseLayerOnly )
            nextLayer = newMat->GetNumLayers();

        for( i = 1; i < fCurrNumLayers; i++ )
        {
            plLayerInterface* layPtr = newMat->GetLayer( fCurrLayerIdx + i );
            if( !layPtr )
                return -1;

            // Can't render into a render target using same rendertarget as a texture.
            if( fCurrBaseRenderTarget 
                && 
                layPtr->GetTexture() == (plBitmap*)(fCurrBaseRenderTarget) )
            {
                // Oops, just bail
                return -1;
            }

            layPtr = IPushOverAllLayer(layPtr);
            ICompositeLayerState(i, layPtr);
            IHandleTextureStage( i, layPtr );
            layPtr = IPopOverAllLayer(layPtr);
        }

    }

    // More cleanup for the DX9.0c 2 texture limitation. See ILayersAtOnce()
    if (fMaxLayersAtOnce == 2)
    {
        if ((fLayerState[0].fBlendFlags & hsGMatState::kBlendAdd)
            && (newMat->GetNumLayers() > fCurrLayerIdx + 1)
            && (newMat->GetLayer(fCurrLayerIdx + 1)->GetUVWSrc() & plLayerInterface::kUVWPosition))
        {
            // If we're doing additive blending and the next layer is based on position,
            // it's probably a distance fade. We'd rather have our diffuse color.
            // ILayersAtOnce will already have told us we can't use it this pass.
            // Skip it so it won't draw on its own next pass.
            nextLayer++;
        }
    }

    int numActivePiggyBacks = 0;
    if( !(fLayerState[0].fMiscFlags & hsGMatState::kMiscBumpChans) && !(fLayerState[0].fShadeFlags & hsGMatState::kShadeEmissive) )
    {
        /// Tack lightmap onto last stage if we have one
        numActivePiggyBacks = fActivePiggyBacks;
        if( numActivePiggyBacks > fMaxLayersAtOnce - fCurrNumLayers )
            numActivePiggyBacks = fMaxLayersAtOnce - fCurrNumLayers;
        if( numActivePiggyBacks )
        {
            int i;
            for( i = 0; i < numActivePiggyBacks; i++ )
            {
                // Note that we take piggybacks off the end of fPiggyBackStack.
                plLayerInterface* layPtr = IPushOverAllLayer( fPiggyBackStack[fPiggyBackStack.GetCount()-1-i] );
                if( !layPtr )
                    return -1;
                ICompositeLayerState(fCurrNumLayers+i, layPtr);
                IHandleTextureStage( fCurrNumLayers+i, layPtr );
                layPtr = IPopOverAllLayer(layPtr);
            }

            // If we've got a piggyback, plus two layers that must be drawn together, but
            // only two TMU's to work with, we're screwed. Someone has got to get skipped and
            // hope no one notices. Typically, the first (base) layer has the color info,
            // and the second the opacity. So we'll try using the projection to brighten
            // the color, ignoring the opacity. 
//          if( ((fCurrNumLayers + numActivePiggyBacks) == fSettings.fMaxLayersAtOnce)
//                  && (fLayerState[0].fMiscFlags & hsGMatState::kMiscBindNext) )
            if( (fLayerState[0].fMiscFlags & hsGMatState::kMiscBindNext)
                && (fCurrNumLayers < 2) )
                nextLayer++;
        }
    }

    // Declare we won't be using any more texture stages.
    IStageStop( fCurrNumLayers + numActivePiggyBacks );

    return nextLayer;
}

// ICompositeLayerState /////////////////////////////////////////////////////////////////
// Set the current Plasma state based on the input layer state and the material overrides.
// fMatOverOn overrides to set a state bit whether it is set in the layer or not.
// fMatOverOff overrides to clear a state bit whether it is set in the layer or not.
const hsGMatState& plDXPipeline::ICompositeLayerState(int which, plLayerInterface* layer)
{
    fOldLayerState[which] = fLayerState[which];
    fLayerState[which].Composite(layer->GetState(), fMatOverOn, fMatOverOff);
    if( fOldLayerState[which].fBlendFlags == uint32_t(-1) )
        fOldLayerState[which].fBlendFlags = ~fLayerState[which].fBlendFlags;

    return fLayerState[which];
}

//// IHandleFirstTextureStage /////////////////////////////////////////////////
// Convert internal material state to D3D state for the base layer.
void    plDXPipeline::IHandleFirstTextureStage( plLayerInterface *layer )
{
    IHandleTextureMode(layer);
    IHandleShadeMode();
    if( fLayerState[0].Differs( fLayerState[0].fZFlags, fOldLayerState[0].fZFlags, hsGMatState::kZMask ) )
        IHandleZMode();
    IHandleMiscMode();

    IHandleTextureStage( 0, layer );
}

//// IHandleShadeMode /////////////////////////////////////////////////////////
// Convert shade state into D3D settings.
void    plDXPipeline::IHandleShadeMode()
{
    if( fLayerState[0].Differs( fLayerState[0].fShadeFlags, fOldLayerState[0].fShadeFlags, hsGMatState::kShadeSpecular ) )
    {
        if( fLayerState[0].fShadeFlags & hsGMatState::kShadeSpecular )
            fD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, TRUE );
        else
            fD3DDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    }
}

//// IHandleZMode /////////////////////////////////////////////////////////////
// Convert Z state into D3D settings.
void    plDXPipeline::IHandleZMode()
{
    switch( fLayerState[0].fZFlags & hsGMatState::kZMask )
    {
        case hsGMatState::kZClearZ:
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_ALWAYS );
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
            break;
        case hsGMatState::kZNoZRead:
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_ALWAYS );
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
            break;
        case hsGMatState::kZNoZWrite:
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_LESSEQUAL );
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZClearZ:
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_ALWAYS );
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
            break;
        case hsGMatState::kZNoZRead | hsGMatState::kZNoZWrite:
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_ALWAYS );
            break;
        case 0:
            fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_LESSEQUAL );
            fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );
            break;

        // illegal combinations
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite:
        case hsGMatState::kZClearZ | hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead:
            hsAssert(false, "Illegal combination of Z Buffer modes (Clear but don't write)");
            break;
    }
}

//// IHandleMiscMode //////////////////////////////////////////////////////////
// Convert Misc state into D3D settings.
void    plDXPipeline::IHandleMiscMode()
{
    if( fLayerState[0].Differs(fLayerState[0].fMiscFlags, fOldLayerState[0].fMiscFlags, hsGMatState::kMiscWireFrame) )
    {
        if( fLayerState[0].fMiscFlags & hsGMatState::kMiscWireFrame )
            fD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
        else
            fD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
    }
}

//// IHandleTextureStage //////////////////////////////////////////////////////
// Issue D3D calls to enable rendering the given layer at the given texture stage.
void    plDXPipeline::IHandleTextureStage( uint32_t stage, plLayerInterface *layer )
{
    hsGDeviceRef        *ref = nullptr;
    plBitmap            *texture;

    // Blend mode
    const hsGMatState& layState = fLayerState[stage];
    if( fLayerState[ stage ].fBlendFlags ^ fOldLayerState[stage].fBlendFlags )
        IHandleStageBlend(stage);

    // Texture wrap/clamp mode
    if( fLayerState[ stage ].fClampFlags ^ fOldLayerState[stage].fClampFlags )
        IHandleStageClamp(stage);

    // UVW transform
    IHandleStageTransform( stage, layer );

    // Create the D3D texture (if necessary) and set it to the device.
    if (texture = layer->GetTexture(); texture != nullptr)
    {
        ref = texture->GetDeviceRef();
        if (ref == nullptr || ref->IsDirty())
        {
            // Normal textures
            plMipmap            *mip;
            plCubicEnvironmap   *cubic;

            if (mip = plMipmap::ConvertNoRef(texture); mip != nullptr)
                ref = MakeTextureRef( layer, mip );

            // Cubic environment maps
            else if (cubic = plCubicEnvironmap::ConvertNoRef(texture); cubic != nullptr)
                ref = IMakeCubicTextureRef( layer, cubic );
        }
    }

    if (ref != nullptr)
        IUseTextureRef(stage, ref, layer);
    else
    {
        fD3DDevice->SetTexture(stage, nullptr);
        hsRefCnt_SafeUnRef( fLayerRef[ stage ] );
        fLayerRef[stage] = nullptr;
    }
}

// CheckTextureRef //////////////////////////////////////////////////////
// Make sure the given layer's texture has background D3D resources allocated.
void plDXPipeline::CheckTextureRef(plLayerInterface* layer)
{
    plBitmap* bitmap = layer->GetTexture();
    if( bitmap )
    {
        hsGDeviceRef* ref = bitmap->GetDeviceRef();

        if( !ref )
        {
            plMipmap* mip = plMipmap::ConvertNoRef(bitmap);
            if( mip )
            {
                MakeTextureRef(layer, mip);
                return;
            }

            plCubicEnvironmap* cubic = plCubicEnvironmap::ConvertNoRef(bitmap);
            if( cubic )
            {
                IMakeCubicTextureRef(layer, cubic);
                return;
            }
        }
    }
}

// IHandleBumpEnv //////////////////////////////////////////////////////////////
// D3D settings for BUMPENVMAPLUMINANCE.
// This has never been used in production assets, because I never got 
// a good effect out of it, and BUMPENVMAPLUMINANCE isn't universally
// supported in hardware.
void plDXPipeline::IHandleBumpEnv(int stage, uint32_t blendFlags)
{
    DWORD current = stage ? D3DTA_CURRENT : D3DTA_DIFFUSE;
    uint32_t colorSrc = blendFlags & hsGMatState::kBlendInvertColor ? D3DTA_TEXTURE | D3DTA_COMPLEMENT : D3DTA_TEXTURE;

    fD3DDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_BUMPENVMAPLUMINANCE);
    fD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, colorSrc); 
    fD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, current); 

    fD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
    fD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

    const hsMatrix44& envXfm = fCurrLay->GetBumpEnvMatrix();
    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVMAT00, F2DW(envXfm.fMap[0][0]));
    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVMAT01, F2DW(envXfm.fMap[1][0]));
    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVMAT10, F2DW(envXfm.fMap[0][1]));
    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVMAT11, F2DW(envXfm.fMap[1][1]));

    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVLSCALE, F2DW(envXfm.fMap[2][2]));
    fD3DDevice->SetTextureStageState(stage, D3DTSS_BUMPENVLOFFSET, F2DW(envXfm.fMap[2][3]));
}

//// IHandleStageBlend ////////////////////////////////////////////////////////
// Translate current blend state for this stage into D3D settings.
void    plDXPipeline::IHandleStageBlend(int stage)
{
    const uint32_t blendFlags = fLayerState[stage].fBlendFlags;
    // If it's the base layer, handle that differently, because it's not really
    // texture stage settings, but frame buffer blend settings.
    if( stage == 0 )
    {
        IHandleFirstStageBlend();
        return;
    }

    uint32_t colorSrc = D3DTA_TEXTURE;
    if( blendFlags & hsGMatState::kBlendInvertColor )
        colorSrc |= D3DTA_COMPLEMENT ;
    // kBlendEnvBumpNext not really used.
    if( blendFlags & hsGMatState::kBlendEnvBumpNext )
    {
        IHandleBumpEnv(stage, blendFlags);
    }
    else switch( blendFlags & hsGMatState::kBlendMask )
    {
        // Alpha blending. Complicated by the ability to ignore either
        // color or alpha for any given texture. The lower end GeForces
        // don't orthogonally support settings, especially when the final
        // (3rd) stage is the diffuse color/alpha modulate and the board
        // really only wants to support 2 stages.
        // So we couldn't just translate our internal plasma stage states
        // into D3D states, we had to do some rearranging.
        // Note that by the time we get here, we _know_ that this isn't the
        // base layer (stage 0), because that's handled elsewhere.
        case hsGMatState::kBlendAlpha:
            // If the current number of layers is 2, then we've already handled the
            // base layer, so this must be layer 1 and the final layer.
            // If the base layer has NoTexColor or this layer has NoTexColor, we need
            // to do some rearranging.
            if( (fCurrNumLayers == 2)
                &&((blendFlags | fLayerState[0].fBlendFlags) & hsGMatState::kBlendNoTexColor) )
            {
                // If this layer AND base layer are NoTexColor, then we just want the diffuse color.
                if( (blendFlags & hsGMatState::kBlendNoTexColor)
                    &&(fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexColor) )
                {
                    // select diffuse color
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE ); 
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
                }
                // If the base layer has NoTexColor but this layer doesn't, then we
                // want the output to be this texture color times diffuse (ignoring base texture color).
                else if( fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexColor )
                {
                    // diffuse is arg2, modulate
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
                }
                // If base layer doesn't have NoTexColor, but this layer does, then
                // we want the output to be diffuse times base texture, which is in current.
                else if( blendFlags & hsGMatState::kBlendNoTexColor )
                {
                    // diffuse is arg1, modulate
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_MODULATE );
                }

            }
            // If we get here and this layer has NoTexColor, then we MUST be on a layer
            // above 1, which means we're on an advanced enough board to handle this orthogonally,
            // i.e. one with more than 2 texture stages.
            else if( blendFlags & hsGMatState::kBlendNoTexColor )
            {
                fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
                fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
            }
            // Finally, no NoTexColor in sight, just set it.
            else
            {
                fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
                fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
                fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   
                                                blendFlags & hsGMatState::kBlendInvertAlpha 
                                                    ? D3DTOP_MODULATEINVALPHA_ADDCOLOR
                                                    : D3DTOP_BLENDTEXTUREALPHA );
            }
            // The same ordeal for alpha, and the ability to ignore the alpha on any texture.
            // Note the additional logic for how to combine the alphas of multiple textures
            // into a final FB alpha.
            // This is orthogonal to using the alpha to combine colors of two different textures.
            // The default behavior is to use the upper texture alpha to blend the upper layer color
            // with the lower texture color, but retain the lower texture alpha (modulated by diffuse)
            // for the frame buffer alpha.
            switch( blendFlags & ( hsGMatState::kBlendAlphaAdd | hsGMatState::kBlendAlphaMult ) )
            {
                default:
                case 0:
                    // Using alpha to blend textures, but this layer's alpha doesn't affect final FB
                    // alpha.
                    // Two layer setup with one or the other (or both) ignoring alpha.
                    if( (fCurrNumLayers == 2)
                        &&((blendFlags | fLayerState[0].fBlendFlags) & hsGMatState::kBlendNoTexAlpha) )
                    {
                        // Both ignoring alpha, use diffuse.
                        if( (blendFlags & hsGMatState::kBlendNoTexAlpha)
                            &&(fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexAlpha) )
                        {
                            // select diffuse alpha
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ); 
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 );
                        }
                        // Base ignoring alpha, use diffuse times this texure alpha.
                        else if( fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexAlpha )
                        {
                            // diffuse is arg2, modulate
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG1, 
                                                            blendFlags & hsGMatState::kBlendInvertAlpha 
                                                                ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                                                                : D3DTA_TEXTURE);
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ); 
                        }
                        // This ignoring alpha, use diffuse times base alpha (in current).
                        else if( blendFlags & hsGMatState::kBlendNoTexAlpha )
                        {
                            // diffuse is arg1, modulate
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
                            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
                        }
                    }
                    // Ignoring alpha or not, with more than 2 texture stages, 
                    // Either way, we'll ignore this texture's alpha, because it's an upper layer
                    // and has already been used (if it's going to get used) to blend this texture's
                    // color with the lower layers.
                    else
                    {
                        fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
                        fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
                    }
                    break;
                    // Alpha coming out of this stage is lower stage alpha plus this texture alpha.
                case hsGMatState::kBlendAlphaAdd:
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_ADD );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG1, 
                                                    blendFlags & hsGMatState::kBlendInvertAlpha 
                                                        ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                                                        : D3DTA_TEXTURE);
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
                    break;
                    // Alpha coming out of this stage is lower stage alpha times this texture alpha.
                case hsGMatState::kBlendAlphaMult:
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG1, 
                                                    blendFlags & hsGMatState::kBlendInvertAlpha 
                                                        ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                                                        : D3DTA_TEXTURE);
                    fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
                    break;
            }
            break;

            // Add texture colors, pass through current alpha.
        case hsGMatState::kBlendAdd:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_ADD );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            break;

            // Multiply texture colors, pass through current alpha
        case hsGMatState::kBlendMult:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_MODULATE );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            
            if (fMaxLayersAtOnce == 2 && stage == 1)
            {
                // On these boards, the only way we can do 2 textures plus diffuse is to
                // multiply it in during stage 0, but that only gives the same result
                // when doing a mult blend, which we won't know when setting up stage 0.
                // Now that we know, adjust stage 0 settings.
                fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
                fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
            }
            break;

            // Dot3 texture colors, pass through current alpha.
        case hsGMatState::kBlendDot3:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_DOTPRODUCT3 );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            break;

            // Add signed texture colors, pass through current alpha.
        case hsGMatState::kBlendAddSigned:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            break;

            // Add signed * 2 texture colors, pass through current alpha.
        case hsGMatState::kBlendAddSigned2X:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED2X );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            break;

            // kBlendAddColorTimesAlpha is only supported for the base layer.
        case hsGMatState::kBlendAddColorTimesAlpha:
            hsAssert(false, "Blend mode unsupported on upper layers");
            break;

            // No blend, select this texture color and pass through current alpha
        case 0:
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG1, colorSrc );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLORARG2, D3DTA_CURRENT ); 
            fD3DDevice->SetTextureStageState( stage, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );

            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
            fD3DDevice->SetTextureStageState( stage, D3DTSS_ALPHAARG2, D3DTA_CURRENT ); 
            break;
    }
}

//// IHandleFirstStageBlend ///////////////////////////////////////////////////
// Set frame buffer blend mode for blending the base layer
// For the case of rendering to a texture with alpha, the alpha written to
// the render target will be computed exactly as the color (limitation of D3D).
void    plDXPipeline::IHandleFirstStageBlend()
{
    // No color, just writing out Z values.
    if( fLayerState[0].fBlendFlags & hsGMatState::kBlendNoColor )
    {
        fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
        fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
        fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
        fLayerState[0].fBlendFlags |= 0x80000000;
    }
    else
    {
        switch( fLayerState[0].fBlendFlags & hsGMatState::kBlendMask )
        {
            // Detail is just a special case of alpha, handled in construction of the texture
            // mip chain by making higher levels of the chain more transparent.
            case hsGMatState::kBlendDetail:
            case hsGMatState::kBlendAlpha:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                if( fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertFinalAlpha )
                {
                    if( fLayerState[0].fBlendFlags & hsGMatState::kBlendAlphaPremultiplied )
                    {
                        fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
                    }
                    else
                    {
                        fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_INVSRCALPHA );
                    }
                    fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCALPHA );
                }
                else
                {
                    if( fLayerState[0].fBlendFlags & hsGMatState::kBlendAlphaPremultiplied )
                    {
                        fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
                    }
                    else
                    {
                        fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
                    }
                    fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
                }
                break;
            // Multiply the final color onto the frame buffer.
            case hsGMatState::kBlendMult:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                if( fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertFinalColor )
                {
                    fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
                    fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR );
                }
                else
                {
                    fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
                    fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR );
                }
                break;

            // Add final color to FB.
            case hsGMatState::kBlendAdd:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
                fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

                break;

            // Multiply final color by FB color and add it into the FB.
            case hsGMatState::kBlendMADD:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_DESTCOLOR );
                fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

                break;

            // Final color times final alpha, added into the FB.
            case hsGMatState::kBlendAddColorTimesAlpha:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                if( fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertFinalAlpha )
                    fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_INVSRCALPHA );
                else
                    fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
                fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );

                break;

            // Overwrite final color onto FB
            case 0:
                fD3DDevice->SetRenderState( D3DRS_ALPHABLENDENABLE,   TRUE );
                fD3DDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ONE );
                fD3DDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );

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
    // Blend ops, not currently used in production.
    if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, (hsGMatState::kBlendSubtract | hsGMatState::kBlendRevSubtract) ) )
    {
        if( fLayerState[0].fBlendFlags & hsGMatState::kBlendSubtract )
            fD3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_SUBTRACT );
        else if( fLayerState[0].fBlendFlags & hsGMatState::kBlendRevSubtract )
            fD3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_REVSUBTRACT );
        else
            fD3DDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

    }

    // AlphaTestHigh is used for reducing sort artifacts on textures that are mostly opaque or transparent, but
    // have regions of translucency in transition. Like a texture for a bush billboard. It lets there be some
    // transparency falloff, but quit drawing before it gets so transparent that draw order problems (halos)
    // become apparent.
    if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, hsGMatState::kBlendAlphaTestHigh) )
    {
        plConst(uint32_t) kHighAlphaTest(0x40);
        if( fLayerState[0].fBlendFlags & hsGMatState::kBlendAlphaTestHigh )
            fD3DDevice->SetRenderState(D3DRS_ALPHAREF, kHighAlphaTest);
        else
            fD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000001);
    }
    // Set the alpha test function, turn on for alpha blending, else off.
    if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, hsGMatState::kBlendAlpha | hsGMatState::kBlendTest | hsGMatState::kBlendAlphaAlways | hsGMatState::kBlendAddColorTimesAlpha) )
    {
        if( (fLayerState[0].fBlendFlags & (hsGMatState::kBlendAlpha | hsGMatState::kBlendTest | hsGMatState::kBlendAddColorTimesAlpha))
                && !(fLayerState[0].fBlendFlags & hsGMatState::kBlendAlphaAlways) )
            fD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER );
        else
            fD3DDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_ALWAYS );
    }
    // Adjust the fog color based on the blend mode. Setting fog color to black for additive modes is
    // an exact solution, setting it to white for multipication is as close of an approximation to correct
    // as you're going to get with DX.
    if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, hsGMatState::kBlendAdd | hsGMatState::kBlendMult | hsGMatState::kBlendMADD | hsGMatState::kBlendAddColorTimesAlpha ) )
    {
        if( fLayerState[0].fBlendFlags & (hsGMatState::kBlendAdd | hsGMatState::kBlendMADD | hsGMatState::kBlendAddColorTimesAlpha) )
            fD3DDevice->SetRenderState( D3DRS_FOGCOLOR, 0 );
        else if( fLayerState[0].fBlendFlags & hsGMatState::kBlendMult )
            fD3DDevice->SetRenderState( D3DRS_FOGCOLOR, 0xffffffff );
        else
            fD3DDevice->SetRenderState( D3DRS_FOGCOLOR, fCurrFog.fHexColor );
    }
}

//// IHandleTextureMode ///////////////////////////////////////////////////////
// Handle the texture stage state for the base layer.
void    plDXPipeline::IHandleTextureMode(plLayerInterface* layer)
{
    plBitmap *bitmap = layer->GetTexture();
    if( bitmap )
    {
        // EnvBumpNext not used in production.
        if( fLayerState[0].fBlendFlags & hsGMatState::kBlendEnvBumpNext )
        {
            IHandleBumpEnv(0, fLayerState[0].fBlendFlags);
        }
        // If the texture stage settings have changed. Note that this
        // is a bad test, we should just be doing something like keeping
        // an array of D3D TextureStageStates as we set them and checking against
        // that directly rather than trying to infer from higher level state
        // whether we need to make the D3D call.
        else if( fSettings.fVeryAnnoyingTextureInvalidFlag 
            || !fTexturing  
            || ( fLayerState[ 0 ].fBlendFlags ^ fOldLayerState[0].fBlendFlags ) 
            || ( fCurrNumLayers + fActivePiggyBacks != fLastEndingStage )
                )
        {
            // If we're only doing one layer, just modulate texture color by diffuse and we're done.
            if( ( fCurrNumLayers + fActivePiggyBacks ) <= 1 )
            {
                // See IHandleStageBlend for notes on NoTexColor.
                if( fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexColor )
                    fD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 );
                else
                    fD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_MODULATE );   
                fD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 
                    fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertColor 
                        ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                        : D3DTA_TEXTURE);
                fD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );

            }
            else
            {
                // See the check in IHandleStageBlend for fSettings.fMaxLayersAtOnce == 2.
                // It depends on these settings and adjusts what it needs.

                // Multitexturing, select texture color to make its way upstream on stages.
                fD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
                fD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, 
                    fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertColor 
                        ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                        : D3DTA_TEXTURE);

                // If our NoTexColor setting has changed, for a refresh of blend state on the next stage
                // since it's affected by our NoTexColor state.
                if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, hsGMatState::kBlendNoTexColor) )
                    fLayerState[1].fBlendFlags = uint32_t(-1);
            }

            // Alpha Arg1 is texture alpha (possibly complemented), and Arg2 is diffuse (possibly complemented).
            // If we want to ignore vertex alpha, select arg1
            // If we want to ignore texture alpha, select arg2
            // Otherwise (and normally) multiply the two.
            fD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,
                fLayerState[0].fBlendFlags & hsGMatState::kBlendNoVtxAlpha
                    ? D3DTOP_SELECTARG1
                    :   fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexAlpha
                        ? D3DTOP_SELECTARG2
                        : D3DTOP_MODULATE );
            fD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, 
                fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertAlpha 
                    ? D3DTA_TEXTURE | D3DTA_COMPLEMENT 
                    : D3DTA_TEXTURE);
            fD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE | 
                        ( fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertVtxAlpha 
                            ? D3DTA_COMPLEMENT 
                            : 0 ) ); 

            fTexturing = true;
        }
    }
    // Here we've no texture for the base layer, but we have more than layer.
    // Select diffuse color and alpha, and pretend we have a texture but we're ignoring its
    // color and alpha.
    else if( fCurrNumLayers + fActivePiggyBacks > 1 )
    {
        fLayerState[0].fBlendFlags |= hsGMatState::kBlendNoTexColor | hsGMatState::kBlendNoTexAlpha;
        fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
        fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
        fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
        if( fLayerState[0].Differs( fLayerState[0].fBlendFlags, fOldLayerState[0].fBlendFlags, (hsGMatState::kBlendNoTexColor|hsGMatState::kBlendNoTexAlpha)) )
            fLayerState[1].fBlendFlags = uint32_t(-1);
        fTexturing = false;
    }
    // Finally, a color only (non-textured) pass. Just select diffuse.
    else
    {
        if( fTexturing || fSettings.fVeryAnnoyingTextureInvalidFlag )
        {
            fD3DDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
            fD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );
            fD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
            fD3DDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE |
                        ( fLayerState[0].fBlendFlags & hsGMatState::kBlendInvertVtxAlpha ? D3DTA_COMPLEMENT : 0 ) ); 

            fTexturing = false;
        }
    }

    fSettings.fVeryAnnoyingTextureInvalidFlag = false;
}

//// IHandleStageClamp ////////////////////////////////////////////////////////
// Translate our current wrap/clamp mode to D3D calls.
void    plDXPipeline::IHandleStageClamp(int stage)
{
    const uint32_t flags = fLayerState[stage].fClampFlags;
    switch( flags )
    {
        case 0:
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP  );
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP  );
            break;
        case hsGMatState::kClampTextureU:
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP  );
            break;
        case hsGMatState::kClampTextureV:
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP  );
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
            break;
        case hsGMatState::kClampTexture:
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
            fD3DDevice->SetSamplerState( stage, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );
            break;
    }
}

void plDXPipeline::ISetBumpMatrices(const plLayerInterface* layer, const plSpan* span)
{
//#define BUMP_COMPARE_MATH
#ifdef BUMP_COMPARE_MATH
    // This section is just debugging, to compute the matrices that will be set.
    static hsMatrix44 preMDu;
    static hsMatrix44 preMDv;
    static hsMatrix44 preMDw;
    static int preMInit = false;
    if( !preMInit )
    {
        hsMatrix44 rotAndCollapseToX;
        int i, j;
        for( i = 0; i < 4; i++ )
        {
            for( j = 0; j < 4; j++ )
            {
                rotAndCollapseToX.fMap[i][j] = 0;
            }
        }
        rotAndCollapseToX.fMap[0][2] = 1.f;
        rotAndCollapseToX.fMap[3][3] = 1.f;
        rotAndCollapseToX.NotIdentity();

        hsMatrix44 offset;
        offset.Reset();
        offset.fMap[0][0] = 0.5f;
        offset.fMap[0][3] = 0.5f;
        offset.NotIdentity();

        preMDu = offset * rotAndCollapseToX;

        offset.fMap[1][3] = 0.5f;

        preMDv = offset * rotAndCollapseToX;

        offset.fMap[1][3] = 1.f;

        preMDw = offset * rotAndCollapseToX;

        preMInit = true;
    }

    hsMatrix44 localToLight = span->GetLight(0, false)->GetWorldToLight() * span->fLocalToWorld;
    localToLight.fMap[0][3] = localToLight.fMap[1][3] = localToLight.fMap[2][3] = 0;
    
    fBumpDuMatrix = preMDu * localToLight;
    fBumpDvMatrix = preMDv * localToLight;

    hsMatrix44 c2w = fView.fCameraToWorld;
    hsMatrix44 cameraToLight = span->GetLight(0, false)->GetWorldToLight() * c2w;
    cameraToLight.fMap[0][3] = cameraToLight.fMap[1][3] = cameraToLight.fMap[2][3] = 0;
    fBumpDwMatrix = preMDw * cameraToLight;

    // HACK PART - FOR COMPARISON
    hsMatrix44 bDu = fBumpDuMatrix;
    hsMatrix44 bDv = fBumpDvMatrix;
    hsMatrix44 bDw = fBumpDwMatrix;
    static hsMatrix44 zeroMatrix;
    fBumpDuMatrix = zeroMatrix;
    fBumpDvMatrix = zeroMatrix;
    fBumpDwMatrix = zeroMatrix;
    // HACK PART - FOR COMPARISON

#endif // BUMP_COMPARE_MATH

    // Here's the math
    // The incoming uv coordinate is either:
    //  kMiscBumpDu - dPos/dU (in other words, the direction in space from this vertex where U increases and V remains constant) in local space.
    //  kMiscBumpDv - dPos/dV (in other words, the direction in space from this vertex where V increases and U remains constant) in local space.
    //  kMiscBumpDw - the normal in camera space.
    //
    // In each case, we need to transform the vector (uvw coord) into light space, and dot it with the light direction.
    // Well, in light space, the light direction is always (0,0,1).
    // So really, we just transform the vector into light space, and the z component is what we want.
    // Then, for each of these, we take that z value (the dot product) and put it into a color channel.
    // R = dPos/dU dot liDir
    // G = dPos/dV dot liDir
    // B = dPos/dW dot liDir
    //
    // That's what we want, here's how we get it.
    // Here, Li(vec) means the vector in light space, Loc(vec) is local space, Tan(vec) is tangent space
    //
    // Li(uvw) = local2Light * Loc(uvw) (uvw comes in in local space, ie input uvw == Loc(uvw)
    // Then we want to:
    //      a) Rotate the Z component to be along X (U) axis
    //      b) Zero out the new Y and Z
    //      c) Scale and offset our new X (the old Z) so -1 => 0, 1 => 1 (scale by 0.5, add 0.5).
    // The following matrix does all this (it's just a concatenation of the above 3 simple matrices).
    //  M = |0 0 0.5 0.5|
    //      |0 0 0   0  |
    //      |0 0 0   0  |
    //      |0 0 0   1  |
    //
    // Our lookup texture that these transformed coords will read into has three horizontal bands,
    // the bottom 3rd is a ramp along U of 0->red
    // middle 3rd is a ramp along U of 0->green
    // last third (highest V) is a ramp along U of 0->blue.
    // So we can do the conversion from our dot to a color with an appropriate V offset in the above M.
    //
    // dPos/dU and dPos/dV are both input in local space, so the transform to get them into light space is
    // the same for each, and that's obviously WorldToLight * LocalToWorld.
    // That's a little inconvenient and inefficient. It's inconvenient, because for an omni light, we
    // can easily fake a light direction (span position - light position), but the full matrix is kind
    // of arbitrary. We could fake it, but instead we move on. It's inefficient because, looking at the 
    // form of matrix M, we know we'll be throwing away a lot of it anyway. So we work through the matrix
    // math and find that we're going to wind up with:
    //
    //      M1 =    |   M[0][2] * loc2li[2][0]  M[0][2] * loc2li[2][1]  M[0][2] * loc2li[2][2]  0.5 |
    //              |                   0                   0                       0           0   |
    //              |                   0                   0                       0           0   |
    //              |                   0                   0                       0           1   |
    //
    // So all we really need is loc2li[2] (row 2). A little more matrix math gives us:
    //
    //      loc2li[2] = (w2li[2] dot loc2wT[0], w2li[2] dot loc2wT[1], w2li[2] dot loc2wT[2]) (where loc2wT is Transpose(loc2w)
    //
    // And hey, that's just dependent on the light's direction w2li[2]. The same thing works out for dPos/dW, except
    // substitue cam2w for loc2w (since input is in camera space instead of world space).
    //
    // And that's about it. We don't actually have to multiply all those matrices at run-time, because
    // we know what the answer will be anyway. We just construct the matrices, making sure we set the
    // appropriate translate for V to get each into the right color channel. The hardware does the three
    // uv transforms and lookups, sums the results, and the output is:
    // (dPos/dU dot liDir, dPos/dV dot liDir, dPos/dW dot liDir), which also happens to be the light direction
    // transformed into tangent space. We dot that with our bump map (which has the normals in tangent space),
    // and we've got per-pixel shading for this light direction.


    hsPoint3 spanPos = span->fWorldBounds.GetCenter();
    hsVector3 liDir(0,0,0);
    int i;
    const hsTArray<plLightInfo*>& spanLights = span->GetLightList(false);
    float maxStrength = 0;
    for( i = 0; i < spanLights.GetCount(); i++ )
    {
        float liWgt = span->GetLightStrength(i, false);
        // A light strength of 2.f means it's from a light group, and we haven't actually calculated
        // the strength. So calculate it now.
        if( liWgt == 2.f )
        {
            float scale;
            spanLights[i]->GetStrengthAndScale(span->fWorldBounds, liWgt, scale);
        }
        if( liWgt > maxStrength )
            maxStrength = liWgt;
        liDir += spanLights[i]->GetNegativeWorldDirection(spanPos) * liWgt;
    }
    hsFastMath::NormalizeAppr(liDir);

    static float kUVWScale = 1.f;
    float uvwScale = kUVWScale;
    if( fLayerState[0].fBlendFlags & hsGMatState::kBlendAdd )
    {
        hsPoint3 viewPos = GetViewPositionWorld();
        hsVector3 cam2span(&viewPos, &spanPos);
        hsFastMath::NormalizeAppr(cam2span);
        liDir += cam2span;
        hsFastMath::NormalizeAppr(liDir);
        static float kSpecularMax = 0.1f;
        static float kSpecularMaxUV = 0.5f;
        if (IsDebugFlagSet(plPipeDbg::kFlagBumpUV))
            uvwScale *= kSpecularMaxUV;
        else
            uvwScale *= kSpecularMax;
    }

    switch( fCurrMaterial->GetLayer(fCurrLayerIdx)->GetMiscFlags() & hsGMatState::kMiscBumpChans )
    {
    case hsGMatState::kMiscBumpDu:
        uvwScale *= fCurrMaterial->GetLayer(fCurrLayerIdx+3)->GetRuntimeColor().r;
        break;
    case hsGMatState::kMiscBumpDv: // This currently should never happen
        uvwScale *= fCurrMaterial->GetLayer(fCurrLayerIdx+1)->GetRuntimeColor().r;
        break;
    case hsGMatState::kMiscBumpDw:
        uvwScale *= fCurrMaterial->GetLayer(fCurrLayerIdx+2)->GetRuntimeColor().r;
        break;
    }
    maxStrength *= 20.f;
    if( maxStrength > 1.f )
        maxStrength = 1.f;
    liDir *= uvwScale * maxStrength;

    const float kUVWOffset = 0.5f;

    float kOffsetToRed;
    float kOffsetToGreen;
    float kOffsetToBlue;

    if (IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || IsDebugFlagSet(plPipeDbg::kFlagBumpW))
    {
        kOffsetToRed = 0.2f;
        kOffsetToGreen = 0.6f;
        kOffsetToBlue = 1.f;
    }
    else
    {
        kOffsetToRed = 0.f;
        kOffsetToGreen = 0.4f;
        kOffsetToBlue = 0.8f;
    }

    const hsMatrix44& l2w = span->fLocalToWorld;

    fBumpDvMatrix.fMap[0][0] = fBumpDuMatrix.fMap[0][0] = (liDir.fX * l2w.fMap[0][0] + liDir.fY * l2w.fMap[1][0] + liDir.fZ * l2w.fMap[2][0]);
    fBumpDvMatrix.fMap[0][1] = fBumpDuMatrix.fMap[0][1] = (liDir.fX * l2w.fMap[0][1] + liDir.fY * l2w.fMap[1][1] + liDir.fZ * l2w.fMap[2][1]);
    fBumpDvMatrix.fMap[0][2] = fBumpDuMatrix.fMap[0][2] = (liDir.fX * l2w.fMap[0][2] + liDir.fY * l2w.fMap[1][2] + liDir.fZ * l2w.fMap[2][2]);

    fBumpDvMatrix.fMap[0][3] = fBumpDuMatrix.fMap[0][3] = kUVWOffset;

    fBumpDuMatrix.fMap[1][3] = kOffsetToRed;
    fBumpDvMatrix.fMap[1][3] = kOffsetToGreen;

#ifndef BUMP_COMPARE_MATH
    hsMatrix44 c2w = fView.GetCameraToWorld();
#endif // BUMP_COMPARE_MATH

    // The bump textures created so far have very strong blue components, which make anything
    // bump mapped glow. The ideal fix would be to have the artists adjust the blue component
    // to a better (lower) value, so there would be a little extra illumination where the bump
    // is straight out into the normal direction, to complement the lateral illumination. 
    // Attempts so far have been unsuccessful in getting them to get a better understanding
    // of bump maps, so I've just zeroed out the contribution in the normal direction.
    plConst(int) kBumpUVOnly(true);
    if( !kBumpUVOnly )
    {
        fBumpDwMatrix.fMap[0][0] = (liDir.fX * c2w.fMap[0][0] + liDir.fY * c2w.fMap[1][0] + liDir.fZ * c2w.fMap[2][0]);
        fBumpDwMatrix.fMap[0][1] = (liDir.fX * c2w.fMap[0][1] + liDir.fY * c2w.fMap[1][1] + liDir.fZ * c2w.fMap[2][1]);
        fBumpDwMatrix.fMap[0][2] = (liDir.fX * c2w.fMap[0][2] + liDir.fY * c2w.fMap[1][2] + liDir.fZ * c2w.fMap[2][2]);
    }
    else
    {
        fBumpDwMatrix.fMap[0][0] = 0;
        fBumpDwMatrix.fMap[0][1] = 0;
        fBumpDwMatrix.fMap[0][2] = 0;
    }

    fBumpDwMatrix.fMap[0][3] = kUVWOffset;
    fBumpDwMatrix.fMap[1][3] = kOffsetToBlue;
}

// IGetBumpMatrix ///////////////////////////////////////////////////////
// Return the correct uvw transform for the bump map channel implied
// in the miscFlags. The matrices have been previously set in ISetBumpMatrices.
const hsMatrix44& plDXPipeline::IGetBumpMatrix(uint32_t miscFlags) const
{
    switch( miscFlags & hsGMatState::kMiscBumpChans )
    {
    case hsGMatState::kMiscBumpDu:
        return fBumpDuMatrix;
    case hsGMatState::kMiscBumpDv:
        return fBumpDvMatrix;
    case hsGMatState::kMiscBumpDw:
    default:
        return fBumpDwMatrix;
    }
}

// ISkipBumpMap /////////////////////////////////////////////////////////////////////////
// Determine whether to skip bumpmapping on this object/material/layer combination.
// We skip if the span isn't illuminated by any lights, or bump mapping is disabled.
// If skipping, we advance <layer> past the bump layers. 
// If there are no more layers after that, we return true (to abort further rendering of currSpan),
// else false to continue rendering.
bool plDXPipeline::ISkipBumpMap(hsGMaterial* newMat, uint32_t& layer, const plSpan* currSpan) const
{
    if( newMat && currSpan )
    {
        if (newMat->GetLayer(layer) 
            &&(newMat->GetLayer(layer)->GetMiscFlags() & hsGMatState::kMiscBumpChans) 
            &&(!currSpan->GetNumLights(false) || IsDebugFlagSet(plPipeDbg::kFlagNoBump)) )
        {
            layer += 4;
            if( layer >= newMat->GetNumLayers() )
                return true;
        }
    }
    return false;
}

//// IHandleStageTransform ////////////////////////////////////////////////////
// Compute and set the UVW transform to D3D.
// This only gets interesting if the transform is dependent on on the current camera transform,
// as is the case with Reflection, Projection, or bump mapping.
void    plDXPipeline::IHandleStageTransform( int stage, plLayerInterface *layer )
{
    if( 1 
        || !(layer->GetTransform().fFlags & hsMatrix44::kIsIdent) 
        || (fLayerState[stage].fMiscFlags & (hsGMatState::kMiscUseReflectionXform|hsGMatState::kMiscUseRefractionXform|hsGMatState::kMiscProjection|hsGMatState::kMiscBumpChans)) )
    {
        D3DMATRIX tXfm;

        if( fLayerState[stage].fMiscFlags & (hsGMatState::kMiscUseReflectionXform | hsGMatState::kMiscUseRefractionXform) )
        {
            // Reflection - this is just the camera to world, with translation removed,
            // and rotated to match cube map conventions.
            hsMatrix44 c2env = fView.GetCameraToWorld();
            c2env = fView.GetCameraToWorld();

            c2env.fMap[0][3] 
                = c2env.fMap[1][3]
                = c2env.fMap[2][3]
                = 0.f;


            if( fLayerState[stage].fMiscFlags & hsGMatState::kMiscUseReflectionXform )
            {

                // This is just a rotation about X of Pi/2 (y = z, z = -y), 
                // followed by flipping Z to reflect back towards us (z = -z).
                float t = c2env.fMap[1][0];
                c2env.fMap[1][0] = c2env.fMap[2][0];
                c2env.fMap[2][0] = t;

                t = c2env.fMap[1][1];
                c2env.fMap[1][1] = c2env.fMap[2][1];
                c2env.fMap[2][1] = t;

                t = c2env.fMap[1][2];
                c2env.fMap[1][2] = c2env.fMap[2][2];
                c2env.fMap[2][2] = t;
            }
            else // must be kMiscUseRefractionXform
            {

                // Okay, I know this refraction isn't any where near
                // right, so don't sit down and try to figure out the
                // math and hook it to the refractive index.
                // It's just a hack that will fool anyone that isn't
                // really paying attention.

                // This is just a rotation about X of Pi/2 (y = z, z = -y), 
                // followed by NOT flipping Z to reflect back towards us (z = -z).
                // In other words, same as reflection, but then c2env = c2env * scaleMatNegateZ.
                float t = c2env.fMap[1][0];
                c2env.fMap[1][0] = c2env.fMap[2][0];
                c2env.fMap[2][0] = t;

                t = c2env.fMap[1][1];
                c2env.fMap[1][1] = c2env.fMap[2][1];
                c2env.fMap[2][1] = t;

                t = c2env.fMap[1][2];
                c2env.fMap[1][2] = c2env.fMap[2][2];
                c2env.fMap[2][2] = t;

                c2env.fMap[0][2] = -c2env.fMap[0][2];
                c2env.fMap[1][2] = -c2env.fMap[1][2];
                c2env.fMap[2][2] = -c2env.fMap[2][2];

#if 0
                const float kFishEyeScale = 0.5f;
                // You can adjust the fish-eye-ness of this by scaling
                // X and Y as well. Eventually, you wind up with the same
                // as c2env * scaleMatXYAndNegateZ, but this is shorter.
                // kFishEyeScale gets pretty fish-eye at about 0.5, and
                // like you're looking through the wrong end of a telescope
                // at about 1.5. 
                // Ideally kFishEyeScale would be a parameter of the layer.
                c2env.fMap[0][0] *= kFishEyeScale;
                c2env.fMap[1][0] *= kFishEyeScale;
                c2env.fMap[2][0] *= kFishEyeScale;
                
                c2env.fMap[0][1] *= kFishEyeScale;
                c2env.fMap[1][1] *= kFishEyeScale;
                c2env.fMap[2][1] *= kFishEyeScale;
#endif
            }

            IMatrix44ToD3DMatrix( tXfm, c2env );
        }
        // cam2Screen will also have the kMiscPerspProjection flag set, so this needs
        // to go before the regular kMiscProjection check.
        else if (fLayerState[stage].fMiscFlags & hsGMatState::kMiscCam2Screen )
        {
            // Still needs a bit of cleaning...
            static hsVector3 camScale(0.5f, -0.5f, 1.f);
            static hsVector3 camTrans(0.5f, 0.5f, 0.f);
            hsMatrix44 p2s;
            p2s.MakeScaleMat(&camScale);
            p2s.fMap[0][3] += camTrans.fX;
            p2s.fMap[1][3] += camTrans.fY;

            // The scale and trans move us from NDC to Screen space. We need to swap
            // the Z and W coordinates so that the texture projection will divide by W
            // and give us projected 2D coordinates.
            float temp = p2s.fMap[2][2];
            p2s.fMap[2][2] = p2s.fMap[3][2];
            p2s.fMap[3][2] = temp;

            temp = p2s.fMap[2][3];
            p2s.fMap[2][3] = p2s.fMap[3][3];
            p2s.fMap[3][3] = temp;

            IMatrix44ToD3DMatrix(tXfm, p2s * IGetCameraToNDC());
        }
        else if( fLayerState[stage].fMiscFlags & hsGMatState::kMiscProjection )
        {
            // For projection, the worldToLight transform is in the layer transform,
            // so we append the cameraToWorld, getting cameraToLight
            hsMatrix44 c2w = fView.GetCameraToWorld();
            if( !(layer->GetUVWSrc() & plLayerInterface::kUVWPosition) )
            {
                c2w.fMap[0][3] = 0;
                c2w.fMap[1][3] = 0;
                c2w.fMap[2][3] = 0;
            }

            // We've already stuffed the worldToLight transform into the layer.
            hsMatrix44 c2l = layer->GetTransform() * c2w;

            IMatrix44ToD3DMatrix(tXfm, c2l);
        }
        else if( fLayerState[stage].fMiscFlags & hsGMatState::kMiscBumpChans )
        {
            // Bump matrices are already set, just get the right one and stuff it in.
            hsMatrix44 m = IGetBumpMatrix(fLayerState[stage].fMiscFlags);

            IMatrix44ToD3DMatrix(tXfm, m);
        }
        else
        {
            // Just put take the layer transform and stuff it in.
            IMatrix44ToD3DMatrix( tXfm, layer->GetTransform() );
        }

        fD3DDevice->SetTransform( sTextureStages[ stage ], &tXfm );
        fLayerTransform[ stage ] = true;
    }
    else if( fLayerTransform[ stage ] )
    {
        // We'd like to just turn it off, but the Voodoo board freaks if the
        // texture coordinates are 3-tuple for no apparent reason.
        fD3DDevice->SetTransform( sTextureStages[ stage ], &d3dIdentityMatrix );
        fLayerTransform[ stage ] = false;
    }

    // If there's an lod bias associated with the layer, set it here.
    // There usually isn't.
    float newBias = fLayerState[stage].fZFlags & hsGMatState::kZLODBias ? layer->GetLODBias() : fTweaks.fDefaultLODBias;
    if( newBias != fLayerLODBias[ stage ] )
    {
        fLayerLODBias[ stage ] = newBias;
        fD3DDevice->SetSamplerState( stage, D3DSAMP_MIPMAPLODBIAS, *(DWORD*)(&fLayerLODBias[ stage ]) );
    }
}

//// IUseTextureRef ///////////////////////////////////////////////////////////
// Set the texturing flags and texture.
void    plDXPipeline::IUseTextureRef( int stage, hsGDeviceRef *dRef, plLayerInterface* layer )
{
    plDXTextureRef *ref = (plDXTextureRef *)dRef;
    uint32_t          xformFlags;

    uint32_t uvwSrc = layer->GetUVWSrc();

    // DX pixel shaders require the TEXCOORDINDEX to be equal to the stage,
    // even though its ignored.
    if( layer->GetPixelShader() && (stage != uvwSrc) )
        uvwSrc = stage;

    // Update our UVW source
    if( fLayerUVWSrcs[ stage ] != uvwSrc )
    {
        fD3DDevice->SetTextureStageState( stage, D3DTSS_TEXCOORDINDEX, uvwSrc );
        fLayerUVWSrcs[ stage ] = uvwSrc;
    }

    if (!layer->GetVertexShader() && !layer->GetPixelShader())
    {
        /// Set the transform flags
        /// Note: the perspective projection flag must be taken from the layer, since it's layer-specific.
        /// Storing it on the texture ref is bad, because the texture ref can be shared among layers whose
        /// projection flags might not match. This should probably be cleaned up, but for now this fixes the
        /// problem.
        if( ref->GetFlags() & plDXTextureRef::kCubicMap )
            xformFlags = D3DTTFF_COUNT3;
        else if( layer->GetMiscFlags() & hsGMatState::kMiscPerspProjection )
            xformFlags = D3DTTFF_COUNT3 | D3DTTFF_PROJECTED;
        else
            xformFlags = D3DTTFF_COUNT2;

        if( xformFlags != fLayerXformFlags[ stage ] )
        {
            fLayerXformFlags[ stage ] = xformFlags;
            fD3DDevice->SetTextureStageState( stage, D3DTSS_TEXTURETRANSFORMFLAGS, xformFlags );
        }
    }

    // Update our current ref
    if( !ref->fD3DTexture )
    {
        if( ref->fData )
            IReloadTexture( ref );
    }
    else if( dRef == fLayerRef[ stage ] )
    {
        return;
    }
    hsRefCnt_SafeAssign( fLayerRef[ stage ], dRef );

    /// Actually make it active!
    fD3DDevice->SetTexture( stage, ref->fD3DTexture );
}

//// IStageStop ///////////////////////////////////////////////////////////////
// Tell the hardware we won't be using any more stages.
// This is more complicated than it sounds. Cases:
// a) single texture stage, we're done (because we've already set
//      texture times diffuse), so just disable stage 1.
// b) we have 2 stages active.
//      b.0) we're skipping texture color on one of those 2 stages. In that
//              case, we've already modulated in our diffuse, so just
//              disable stage 2.
//      b.1) we're using texture color from both stages 0 and 1, and still need
//              to modulate in diffuse. So set stage 2 to modulate in diffuse,
//              and disable stage 3.
// c) we have 3 or more stages active. Append a modulation by diffuse
// Note that this only applies to color, because diffuse alpha is always modulated
// in from the start.
void    plDXPipeline::IStageStop( uint32_t stage )
{
    int disableStage = stage;

    // Note: even if we don't have a texture, we handle it similar to if we had one,
    // so the only special case we need here is if we only had one stage to set up -mcn
    if( ( stage <= 1 ) )
    {
        fD3DDevice->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
        fD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        fLayerState[ stage ].fBlendFlags = uint32_t(-1);
        disableStage = stage;
    }
    else if( stage == 2 )
    {
        // The fMaxLayersAtOnce == 2 check is for the DX9.0c 2 texture limitation.
        // See ILayersAtOnce()
        if ((fLayerState[0].fBlendFlags & hsGMatState::kBlendNoTexColor)
            || (fLayerState[1].fBlendFlags & hsGMatState::kBlendNoTexColor)
            || fMaxLayersAtOnce == 2)
        {
            fD3DDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
            disableStage = 2;
        }
        else
        {
            fD3DDevice->SetTextureStageState(2, D3DTSS_COLOROP,   D3DTOP_MODULATE);
            fD3DDevice->SetTextureStageState(2, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
            fD3DDevice->SetTextureStageState(2, D3DTSS_COLORARG2, D3DTA_CURRENT);
            
            fD3DDevice->SetTextureStageState(3, D3DTSS_COLOROP, D3DTOP_DISABLE);
            disableStage = 3;
        }

        fD3DDevice->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        
        fLayerState[2].fBlendFlags = uint32_t(-1);
        fLayerState[3].fBlendFlags = uint32_t(-1);
    }
    else
    {
        // This is directly contrary to the DX documentation, but in line with
        // the code generated by MFCTex (which works). The docs say:
        //  "Alpha operations cannot be disabled when color operations are enabled. 
        //      Setting the alpha operation to D3DTOP_DISABLE when color blending 
        //      is enabled causes undefined behavior."
        // But not disabling the earliest possible alpha stage causes the driver
        // to choke.


        fD3DDevice->SetTextureStageState(stage, D3DTSS_COLOROP,   D3DTOP_MODULATE);
        fD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        fD3DDevice->SetTextureStageState(stage, D3DTSS_COLORARG2, D3DTA_CURRENT);

        fD3DDevice->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        fLayerState[stage].fBlendFlags = uint32_t(-1);

        fD3DDevice->SetTextureStageState(stage+1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        fLayerState[stage+1].fBlendFlags = uint32_t(-1);

        disableStage = stage+1;
    }

    fLastEndingStage = stage;

    if( fSettings.fIsIntel )
    {
        int maxUVW = 0;
        int k;
        for( k = 0; k < fCurrNumLayers; k++ )
        {
            if( (fCurrMaterial->GetLayer(k + fCurrLayerIdx)->GetUVWSrc() & 0xf) > maxUVW )
                maxUVW = fCurrMaterial->GetLayer(k + fCurrLayerIdx)->GetUVWSrc() & 0xf;
        }
        for( k = disableStage; k <= maxUVW; k++ )
        {
            fD3DDevice->SetTextureStageState(k, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);
            fD3DDevice->SetTextureStageState(k, D3DTSS_COLORARG2, D3DTA_CURRENT);
        }
        fD3DDevice->SetTextureStageState(k, D3DTSS_COLOROP, D3DTOP_DISABLE);
    }
}

// IInvalidateState /////////////////////////////////////////////////////////////
// Documentation is unclear on what state persists or becomes invalid on switching
// a render target or finishing a frame. I put into this function things that show
// up as suspect, whether they "ought" to be here or not.
void plDXPipeline::IInvalidateState()
{
    fLastEndingStage = 0;
    fTexturing = false;
    int i;
    for( i = 0; i < 8; i++ )
    {
        hsRefCnt_SafeUnRef( fLayerRef[ i ] );
        fLayerRef[i] = nullptr;
        fD3DDevice->SetTexture(i, nullptr);
    }

    fLayerState[ 0 ].fZFlags = 0;
    fD3DDevice->SetRenderState( D3DRS_ZFUNC,        D3DCMP_LESSEQUAL );
    fD3DDevice->SetRenderState( D3DRS_ZWRITEENABLE, TRUE );

    // This is a workaround for the latest ATI drivers (6.14.10.6422).
    // They seem to be caching something on lights (possibly only specular
    // lights, but I haven't been able to prove it) the first time they
    // are used in a render, and then not letting go when the camera
    // is moved for another render in the same frame (same BeginScene/EndScene pair).
    // The effect is very incorrect lighting. Moreover, if the multiple renders
    // per frame are infrequent (e.g. refreshing an environment map every few
    // seconds), you'll get flashes after the double render frames.
    // Workaround is to Disable all lights at render target switch, although
    // a more correct workaround might be to disable all lights at camera move.
    // All of this is strictly conjecture, so I'm going with what works.
    // Note also that I'm only disabling lights that are currently enabled
    // at the time of the render target switch. Since this is dealing with
    // a driver bug, it might be safer to disable them all, but timings
    // show that looping through all the lights in a scene like Teledahn exterior,
    // with hundreds of active lights, incurs a measurable expense (some milliseconds),
    // whereas disabling only the active lights fixes the known problem but costs
    // zero.
    plProfile_BeginTiming(ClearLights);
    
    hsBitIterator iterOff(fLights.fEnabledFlags);
    for( iterOff.Begin(); !iterOff.End(); iterOff.Advance() )
        fD3DDevice->LightEnable(iterOff.Current(), false);
    fLights.fEnabledFlags.Clear();
    fLights.fHoldFlags.Clear();

    plProfile_EndTiming(ClearLights);

    // This is very annoying. Set fTexturing to false doesn't work if the next layer
    // we draw doesn't have a texture. So we have to set this flag instead to force
    // a state update. I have an idea about how to do all of this a lot better, but
    // it's not time to do it...not yet at least.... --mcn
    fSettings.fVeryAnnoyingTextureInvalidFlag = true;
}

//// ILayersAtOnce ////////////////////////////////////////////////////////////
// Compute how many of the upcoming layers we can render in a single pass on the
// current hardware.
uint32_t  plDXPipeline::ILayersAtOnce( hsGMaterial *mat, uint32_t which )
{
    fCurrNumLayers = 1;

    if( fView.fRenderState & plPipeline::kRenderBaseLayerOnly )
        return fCurrNumLayers;

    plLayerInterface *lay = mat->GetLayer( which );

    if (IsDebugFlagSet(plPipeDbg::kFlagNoMultitexture))
        return fCurrNumLayers;

    if ((IsDebugFlagSet(plPipeDbg::kFlagBumpUV) || IsDebugFlagSet(plPipeDbg::kFlagBumpW)) && (lay->GetMiscFlags() & hsGMatState::kMiscBumpChans) )
        return fCurrNumLayers = 2;

    if( (lay->GetBlendFlags() & hsGMatState::kBlendNoColor)
        ||(lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner)
        )
        return fCurrNumLayers;

    // New DX9.0c limitation for cards that can only do 2 textures per pass.
    // We used to be able to set stage 0 and 1 to textures, and set stage 2 to the
    // diffuse color. With DX9.0c we just get two texture stages. Period. 
    // Either we give up a texture or the diffuse color.
    if (fMaxLayersAtOnce == 2)
    {
        if ((mat->GetNumLayers() > which + 1)
            && !(mat->GetLayer(which + 1)->GetBlendFlags() & hsGMatState::kBlendNoTexColor))
        {
            // If we're just using the texture for alpha, we can multiply
            // the diffuse color in stage 1. Otherwise, save it for the next pass.
            return fCurrNumLayers;
        }
    }

    int i;
    int maxLayersAtOnce = fMaxLayersAtOnce;

    // Now Reserve space for piggy backs, and see if there are 
    // are any more layers we can pick up.
    // 
    maxLayersAtOnce = fMaxLayersAtOnce - fActivePiggyBacks;
    if( which + maxLayersAtOnce > mat->GetNumLayers() )
        maxLayersAtOnce = mat->GetNumLayers() - which;

    for( i = fCurrNumLayers; i < maxLayersAtOnce; i++ )
    {
        plLayerInterface *lay = mat->GetLayer(which + i);
        if( (lay->GetUVWSrc() & 0xf) > fSettings.fMaxUVWSrc )
            break;
        if( (lay->GetMiscFlags() & hsGMatState::kMiscBindNext)
                &&(i+1 >= maxLayersAtOnce) )
            break;
        if( lay->GetMiscFlags() & hsGMatState::kMiscRestartPassHere )
            break;
        if( !(mat->GetLayer(which+i-1)->GetMiscFlags() & hsGMatState::kMiscBindNext)
                && !ICanEatLayer(lay) )
            break;
        fCurrNumLayers++;
    }
    return fCurrNumLayers;
}

//// ICanEatLayer /////////////////////////////////////////////////////////////
// Determine if this layer can be an upper layer, or if it needs
// to be the base on another pass.
bool  plDXPipeline::ICanEatLayer( plLayerInterface* lay )
{
    if( !lay->GetTexture() )
        return false;


    if( (lay->GetBlendFlags() & hsGMatState::kBlendNoColor)
        ||(lay->GetBlendFlags() & hsGMatState::kBlendAddColorTimesAlpha) // has to be base layer
        ||(lay->GetMiscFlags() & hsGMatState::kMiscTroubledLoner) )
        return false;

    if( (lay->GetBlendFlags() & hsGMatState::kBlendAlpha )
        &&(lay->GetAmbientColor().a < 1.f) )
        return false;

    if( !(lay->GetZFlags() & hsGMatState::kZNoZWrite) )
        return false;

    return true;
}

///////////////////////////////////////////////////////////////////////////////
//// Textures /////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IReloadTexture ///////////////////////////////////////////////////////////
// Fills in D3D texture resource, creating it if necessary.
void    plDXPipeline::IReloadTexture( plDXTextureRef *ref )
{
    if( ref->GetFlags() & plDXTextureRef::kCubicMap )
    {
        if (ref->fD3DTexture == nullptr)
            ref->fD3DTexture = IMakeD3DCubeTexture( ref, ref->fFormatType );

        if (ref->fD3DTexture != nullptr)
            IFillD3DCubeTexture( (plDXCubeTextureRef *)ref );
    }
    else
    {
        if (ref->fD3DTexture == nullptr)
            ref->fD3DTexture = IMakeD3DTexture( ref, ref->fFormatType );

        if (ref->fD3DTexture != nullptr)
            IFillD3DTexture( ref );
    }
}

static uint32_t IGetD3DTextureUsage(const plDXTextureRef* ref)
{
    uint32_t usage = 0;
    if (ref->GetFlags() & plDXTextureRef::kAutoGenMipmap)
        usage |= D3DUSAGE_AUTOGENMIPMAP;
    return usage;
}

//// IMakeD3DTexture //////////////////////////////////////////////////////////
//  Makes a DX Texture object based on the ref given.

IDirect3DTexture9   *plDXPipeline::IMakeD3DTexture( plDXTextureRef *ref, D3DFORMAT formatType )
{
    D3DPOOL poolType = D3DPOOL_MANAGED;
    IDirect3DTexture9   *texPtr;
    fManagedAlloced = true;
    if( FAILED( fSettings.fDXError = fD3DDevice->CreateTexture( ref->fMaxWidth, ref->fMaxHeight, 
                                          ref->fMMLvs,
                                          IGetD3DTextureUsage(ref),
                                          formatType,
                                          poolType,
                                          &texPtr, nullptr)))
    {
        IGetD3DError();
        plStatusLog::AddLineSF( "pipeline.log", 0xffff0000, "Unable to create texture ({}) Owner: {} "
                                            "Size: {} x {} NumLvls: {} Flags: {x}",
                                            fSettings.fErrorStr, ref->fOwner ? ref->fOwner->GetKey() ? ref->fOwner->GetKey()->GetUoid().GetObjectName() : ST_LITERAL("") : ST_LITERAL(""),
                                            ref->fMaxWidth, ref->fMaxHeight, ref->fMMLvs, ref->GetFlags() );
        return nullptr;
    }
    PROFILE_POOL_MEM(poolType, ref->fDataSize, true, (ref->fOwner ? ref->fOwner->GetKey() ? ref->fOwner->GetKey()->GetUoid().GetObjectName().c_str() : "(UnknownTexture)" : "(UnknownTexture)"));
    fTexManaged += ref->fDataSize;

    return texPtr;
}

//// IFillD3DTexture //////////////////////////////////////////////////////////
// Copies the data from the ref into the D3D texture, filling in all
// mip levels. 
void    plDXPipeline::IFillD3DTexture( plDXTextureRef *ref )
{
    int         i;
    uint8_t       *pTexDat = (uint8_t *)ref->fData;


    if (pTexDat == nullptr)
    {
        plStatusLog::AddLineSF( "pipeline.log", 0xffff0000, "Unable to fill texture ref (data is nil) Owner: {}",
                                            ref->fOwner ? ref->fOwner->GetKey() ? ref->fOwner->GetKey()->GetUoid().GetObjectName() : ST_LITERAL("") : ST_LITERAL("") );
        return;
    }

    IDirect3DTexture9 *lpDst = (IDirect3DTexture9 *)ref->fD3DTexture;

    for( i = 0; i < ref->fMMLvs; i++ )
    {
        D3DLOCKED_RECT      lockInfo;

        fSettings.fDXError = lpDst->LockRect(i, &lockInfo, nullptr, 0);
        if (FAILED(fSettings.fDXError))
        {
            IGetD3DError();
            plStatusLog::AddLineSF( "pipeline.log", 0xffff0000, "Unable to lock texture level {} for filling ({}) Owner: {} "
                                                "Size: {} x {} NumLvls: {} Flags: {x}",
                                                i, fSettings.fErrorStr, ref->fOwner ? ref->fOwner->GetKey() ? ref->fOwner->GetKey()->GetUoid().GetObjectName() : ST_LITERAL("") : ST_LITERAL(""),
                                                ref->fMaxWidth, ref->fMaxHeight, ref->fMMLvs, ref->GetFlags() );
            return;
        }

        memcpy( (char *)lockInfo.pBits, pTexDat, ref->fLevelSizes[ i ] );
        pTexDat += ref->fLevelSizes[ i ];
        lpDst->UnlockRect( i );
    }       
}

//// IMakeD3DCubeTexture //////////////////////////////////////////////////////
//  Makes a DX Cubic Texture object based on the ref given.

IDirect3DCubeTexture9   *plDXPipeline::IMakeD3DCubeTexture( plDXTextureRef *ref, D3DFORMAT formatType )
{
    D3DPOOL                 poolType = D3DPOOL_MANAGED;
    IDirect3DCubeTexture9   *texPtr = nullptr;
    fManagedAlloced = true;
    WEAK_ERROR_CHECK(fD3DDevice->CreateCubeTexture( ref->fMaxWidth, ref->fMMLvs, 0, formatType, poolType, &texPtr, nullptr));
    PROFILE_POOL_MEM(poolType, ref->fDataSize, true, (ref->fOwner ? ref->fOwner->GetKey() ? ref->fOwner->GetKey()->GetUoid().GetObjectName().c_str() : "(UnknownTexture)" : "(UnknownTexture)"));
    fTexManaged += ref->fDataSize;
    return texPtr;
}

//// IFillD3DCubeTexture //////////////////////////////////////////////////////
// Fill in all faces of the D3D cube map from the input reference.
void    plDXPipeline::IFillD3DCubeTexture( plDXCubeTextureRef *ref )
{
    int                 i, f;
    D3DCUBEMAP_FACES    faces[ 6 ] = {  D3DCUBEMAP_FACE_NEGATIVE_X,     // Left
                                        D3DCUBEMAP_FACE_POSITIVE_X,     // Right
                                        D3DCUBEMAP_FACE_POSITIVE_Z,     // Front
                                        D3DCUBEMAP_FACE_NEGATIVE_Z,     // Back
                                        D3DCUBEMAP_FACE_POSITIVE_Y,     // Top
                                        D3DCUBEMAP_FACE_NEGATIVE_Y };   // Bottom
    
    for( f = 0; f < 6; f++ )
    {
        uint8_t                   *pTexDat = ( f == 0 ) ? (uint8_t *)ref->fData : (uint8_t *)ref->fFaceData[ f - 1 ];
        IDirect3DCubeTexture9   *lpDst = (IDirect3DCubeTexture9 *)ref->fD3DTexture;

        for( i = 0; i < ref->fMMLvs; i++ )
        {
            D3DLOCKED_RECT      lockInfo;

            lpDst->LockRect(faces[f], i, &lockInfo, nullptr, 0);
            memcpy( (char *)lockInfo.pBits, pTexDat, ref->fLevelSizes[ i ] );
            pTexDat += ref->fLevelSizes[ i ];
            lpDst->UnlockRect( faces[ f ], i );
        }       
    }
}

//// MakeTextureRef ///////////////////////////////////////////////////////////
//  Creates a hsGDeviceRef for a texture.
// May have to decompress the texture if the hardware doesn't support compressed textures (unlikely).
hsGDeviceRef    *plDXPipeline::MakeTextureRef( plLayerInterface* layer, plMipmap *b )
{
    plMipmap    *original = b, *colorized = nullptr;

    // If the hardware doesn't support Luminance maps, we'll just treat as ARGB.
    if( !( fSettings.fD3DCaps & kCapsLuminanceTextures ) )
        b->SetFlags( b->GetFlags() & ~plMipmap::kIntensityMap );

    /// Colorize if we're supposed to (8.21.2000 mcn)
    // Debugging only.
    if (IsDebugFlagSet(plPipeDbg::kFlagColorizeMipmaps))
    {
        b = original->Clone();
        if (b != nullptr)
            b->Colorize();
        else
            b = original;
    }

    if( !( fSettings.fD3DCaps & kCapsCompressTextures ) && b->IsCompressed() )
        b = hsCodecManager::Instance().CreateUncompressedMipmap( b, hsCodecManager::k16BitDepth );

    /// Set up some stuff
    uint32_t      mmlvs      = 1;
    D3DFORMAT     formatType = D3DFMT_UNKNOWN;    // D3D Format
    uint32_t      formatSize = 0;
    uint32_t      totalSize = 0;
    uint32_t*     levelSizes = nullptr;
    uint32_t      numPix = 0;
    uint32_t      externData = false;
    void          *tData;
    bool          noMip = !(fSettings.fD3DCaps & kCapsMipmap);


    /// Convert the bitmap over
    // Select a target format
    IGetD3DTextureFormat( b, formatType, formatSize );

    // Process the texture data into a format that can be directly copied to the D3D texture.
    // externData returned as true means that tData just points directly into the mipmap's fImage,
    // so don't delete it when deleting the texture device ref. externData false means this is
    // a reformatted copy, so the ref owns it.
    externData = IProcessMipmapLevels( b, mmlvs, levelSizes, totalSize, numPix, tData, noMip );

    // If the texture has a device ref, just re-purpose it, else make one and initialize it.
    plDXTextureRef *ref = (plDXTextureRef *)b->GetDeviceRef();
    if( !ref )
    {
        ref = new plDXTextureRef( formatType, 
                                          mmlvs, b->GetWidth(), b->GetHeight(), 
                                          numPix, totalSize, totalSize, levelSizes,
                                          tData, externData );
        ref->fOwner = original;
        ref->Link( &fTextureRefList );
        original->SetDeviceRef( ref );
        // Note: this is because SetDeviceRef() will ref it, and at this point,
        // only the bitmap should own the ref, not us. We ref/unref it on Use()
        hsRefCnt_SafeUnRef( ref );  
    }
    else
        ref->Set( formatType, mmlvs, b->GetWidth(), b->GetHeight(), 
                  numPix, totalSize, totalSize, levelSizes, tData, externData );

    // Keep the refs in a linked list for easy disposal.
    if( !ref->IsLinked() )
    {
        // Re-linking
        ref->Link( &fTextureRefList );
    }

    // NOTE: This is just a hint, so setting it on a device with no support for it
    //       or mipmaps in general won't do any damage.
    if (original->GetFlags() & plMipmap::kAutoGenMipmap)
        ref->SetFlags(ref->GetFlags() | plDXTextureRef::kAutoGenMipmap);

    /// Copy the data into the ref
    IReloadTexture( ref );

    ref->fData = nullptr;
    ref->SetDirty( false );

    // Set any implied flags.
    if (layer)
    {
        if( layer->GetMiscFlags() & hsGMatState::kMiscPerspProjection )
            ref->SetFlags(ref->GetFlags() | plDXTextureRef::kPerspProjection);
        else if( layer->GetMiscFlags() & hsGMatState::kMiscOrthoProjection )
            ref->SetFlags(ref->GetFlags() | plDXTextureRef::kOrthoProjection);

        if( layer->GetMiscFlags() & hsGMatState::kMiscBumpDw )
            ref->SetFlags(ref->GetFlags() | plDXTextureRef::kUVWNormal);
    }

    if( b != original )
        delete b;       // Delete if we created a new (temporary) one

    // Turn this on to delete the plasma system memory copy once we have a D3D managed version.
    // Currently disabled, because there are still mipmaps that are read from after their managed
    // versions are created, but aren't flagged DontThrowAwayImage or kUserOwnesBitmap.
    if( !( original->GetFlags() & ( plMipmap::kUserOwnsBitmap | plMipmap::kDontThrowAwayImage ) )
        && !GetProperty( kPropDontDeleteTextures ) )
    {
#ifdef MF_TOSSER
        original->Reset();
#endif // MF_TOSSER
    }

    return ref;
}

//// IMakeCubicTextureRef /////////////////////////////////////////////////////
// Same as MakeTextureRef, except done for the six faces of a cube map.
hsGDeviceRef    *plDXPipeline::IMakeCubicTextureRef( plLayerInterface* layer, plCubicEnvironmap *cubic )
{
    plDXCubeTextureRef  *ref;
    plMipmap            *faces[ 6 ];
    int                 i;
    D3DFORMAT           formatType = D3DFMT_UNKNOWN;
    uint32_t            formatSize = 0;
    uint32_t            numLevels = 1;
    uint32_t            totalSize = 0;
    uint32_t            *levelSizes = nullptr;
    uint32_t            numPixels = 0;
    uint32_t            externData;
    void                *textureData[ 6 ];

    if (cubic == nullptr || !(fSettings.fD3DCaps & kCapsCubicTextures))
        return nullptr;


    bool noMip = !(fSettings.fD3DCaps & kCapsMipmap) || !(fSettings.fD3DCaps & kCapsCubicMipmap);

    /// Get the mips
    if( !( fSettings.fD3DCaps & kCapsCompressTextures ) )
    {
        for( i = 0; i < 6; i++ )
        {
            faces[ i ] = cubic->GetFace( i );
            if( faces[ i ]->IsCompressed() )
                faces[ i ] = hsCodecManager::Instance().CreateUncompressedMipmap( faces[ i ], hsCodecManager::k16BitDepth );
        }
    }
    else
    {
        for( i = 0; i < 6; i++ )
            faces[ i ] = cubic->GetFace( i );
    }

    /// Create the ref
    // Get format
    IGetD3DTextureFormat( faces[0], formatType, formatSize );

    // Process the data.
    if( faces[0]->IsCompressed() || ( faces[0]->GetPixelSize() < 32 ) )
    {
        /// For this, we just take the image data pointers directly, so only call IProcess once
        externData = IProcessMipmapLevels( faces[ 0 ], numLevels, levelSizes, totalSize, numPixels, textureData[ 0 ], noMip );
        for( i = 1; i < 6; i++ )
            textureData[ i ] = faces[ i ]->GetImage();
    }
    else
    {
        for( i = 0; i < 6; i++ )
        {
            /// Some of this will be redundant, but oh well
            externData = IProcessMipmapLevels( faces[ i ], numLevels, levelSizes, totalSize, numPixels, textureData[ i ], noMip );
        }
    }

    ref = (plDXCubeTextureRef *)cubic->GetDeviceRef();
    if( !ref )
    {
        ref = new plDXCubeTextureRef( formatType, 
                                          numLevels, faces[ 0 ]->GetWidth(), faces[ 0 ]->GetHeight(), 
                                          numPixels, totalSize, totalSize * 6, levelSizes,
                                          textureData[ 0 ], externData );
        ref->fOwner = cubic;
        ref->Link( &fTextureRefList );  // So we don't ref later on down
        for( i = 0; i < 5; i++ )
            ref->fFaceData[ i ] = textureData[ i + 1 ];

        cubic->SetDeviceRef( ref );
        // Note: this is because SetDeviceRef() will ref it, and at this point,
        // only the bitmap should own the ref, not us. We ref/unref it on Use()
        hsRefCnt_SafeUnRef( ref );
    }
    else
    {
        ref->Set( formatType, numLevels, faces[ 0 ]->GetWidth(), faces[ 0 ]->GetHeight(), 
                  numPixels, totalSize, totalSize * 6, levelSizes, textureData[ 0 ], externData );

        for( i = 0; i < 5; i++ )
            ref->fFaceData[ i ] = textureData[ i + 1 ];
    }
    ref->SetFlags( ref->GetFlags() | plDXTextureRef::kCubicMap );

    // Put in linked list for easy disposal.
    if( !ref->IsLinked() )
    {
        // Re-linking
        ref->Link( &fTextureRefList );
    }

    /// Copy the data into the ref
    IReloadTexture( ref );
    ref->SetDirty( false );

    /// Cleanup
    for( i = 0; i < 6; i++ )
    {
        if( faces[ i ] != cubic->GetFace( i ) )
            delete faces[ i ];
        if( !( cubic->GetFace(i)->GetFlags() & (plMipmap::kUserOwnsBitmap | plMipmap::kDontThrowAwayImage) ) && !GetProperty( kPropDontDeleteTextures ) )
        {
            // Turn this on to delete the plasma system memory copy once we have a D3D managed version.
            // Currently disabled, because there are still mipmaps that are read from after their managed
            // versions are created, but aren't flagged DontThrowAwayImage or kUserOwnesBitmap.
//          cubic->GetFace(i)->Reset();
        }
    }

    return ref;
}

//// IProcessMipmapLevels /////////////////////////////////////////////////////
// Compute proper values for the arguments passed in.
// Return true if the data returned points directly into the mipmap data,
// return false if textureData is a reformatted copy of the mipmap's data.
bool  plDXPipeline::IProcessMipmapLevels( plMipmap *mipmap, uint32_t &numLevels,
                                            uint32_t *&levelSizes, uint32_t &totalSize, 
                                            uint32_t &numPixels, void *&textureData, bool noMip )
{
    bool        externData = false;
    D3DFORMAT   formatType = D3DFMT_UNKNOWN;    // D3D Format
    uint32_t    formatSize;

    
    IGetD3DTextureFormat( mipmap, formatType, formatSize );

    // Compressed or 16 bit, we can use directly.
    if( mipmap->IsCompressed() || ( mipmap->GetPixelSize() < 32 ) )
    {
        numPixels = 0;
        if( noMip )
        {
            numLevels = 1;
            levelSizes = nullptr;
            totalSize = mipmap->GetLevelSize(0);
        }
        else
        {
            uint32_t          sizeMask = 0x03;

            int maxLevel = mipmap->GetNumLevels() - 1;

            /// 9.7.2000 - Also do this test if the card doesn't support
            /// itty bitty textures
            if( mipmap->IsCompressed() || !( fSettings.fD3DCaps & kCapsDoesSmallTextures ) )
            {
                mipmap->SetCurrLevel( maxLevel );
                while (maxLevel && (mipmap->GetCurrWidth() | mipmap->GetCurrHeight()) & sizeMask)
                {
                    maxLevel--;
                    mipmap->SetCurrLevel( maxLevel );
                }
            }

            mipmap->SetCurrLevel( 0 );
            totalSize = 0;
            numLevels = maxLevel + 1;
            levelSizes = new uint32_t[ numLevels ];
            int i;
            for( i = 0; i < numLevels; i++ )
            {
                levelSizes[ i ] = mipmap->GetLevelSize( i );
                totalSize += mipmap->GetLevelSize( i );
            }
        }

        textureData = mipmap->GetImage();
        externData = true;
    }
    else
    {
        // 32 bit uncompressed data. In general, we reformat to 16 bit if we're running
        // 16 bit, or if 32 bit leave it at 32. All subject to what the hardware can do
        // and what the texture is for. See IGetD3DTextureFormat.
        formatSize >>= 3;

        if( !noMip )
        {
            numPixels = mipmap->GetTotalSize() * 8 / mipmap->GetPixelSize();
            numLevels = mipmap->GetNumLevels();

            levelSizes = new uint32_t[ numLevels ];

            int     i;
            uint32_t w, h;
            for( i = 0; i < numLevels; i++ )
            {
                mipmap->GetLevelPtr( i, &w, &h );
                levelSizes[ i ] = w * h * formatSize;
            }
        }
        else
        {
            numPixels = mipmap->GetWidth() * mipmap->GetHeight();
            numLevels = 1;
            levelSizes = nullptr;
        }
        totalSize = numPixels * formatSize;

        // Shared scratch space to reformat a texture before it's copied into
        // the D3D surface.
        textureData = IGetPixelScratch( totalSize );

        // Convert it to the requested format.
        IFormatTextureData( formatType, numPixels, (hsRGBAColor32 *)mipmap->GetImage(), textureData );
    }

    return externData;
}

//// IGetPixelScratch /////////////////////////////////////////////////////////
// Return scratch space at least of at least size bytes, to reformat a mipmap into.
void    *plDXPipeline::IGetPixelScratch( uint32_t size )
{
    static char     *sPtr = nullptr;
    static uint32_t   sSize = 0;

    if( size > sSize )
    {
        if (sPtr != nullptr)
            delete [] sPtr;
        
        if( size > 0 )
            sPtr = new char[ sSize = size ];
        else
            sPtr = nullptr;
    }
    else if( size == 0 )
    {
        if (sPtr != nullptr)
            delete [] sPtr;

        sPtr = nullptr;
        sSize = 0;
    }

    return sPtr;
}

//// IGetD3DTextureFormat /////////////////////////////////////////////////////
//  Given a bitmap, finds the matching D3D format.

void    plDXPipeline::IGetD3DTextureFormat( plBitmap *b, D3DFORMAT &formatType, uint32_t& texSize )
{
    hsAssert( b, "Nil input to GetTextureFormat()" );

    bool prefer32bit = 0 != (b->GetFlags() & plBitmap::kForce32Bit);

    if( b->IsCompressed() )
    {
        hsAssert( plMipmap::kDirectXCompression == b->fCompressionType, "Unsupported compression format" );
        texSize = 0;
        switch( b->fDirectXInfo.fCompressionType )
        {
            case plMipmap::DirectXInfo::kDXT1:
                formatType = D3DFMT_DXT1;
                break;
//          case plMipmap::DirectXInfo::kDXT2:
//              formatType = D3DFMT_DXT2;
//              break;
//          case plMipmap::DirectXInfo::kDXT3:
//              formatType = D3DFMT_DXT3;
//              break;
//          case plMipmap::DirectXInfo::kDXT4:
//              formatType = D3DFMT_DXT4;
//              break;
            case plMipmap::DirectXInfo::kDXT5:
                formatType = D3DFMT_DXT5;
                break;
            default:
                hsAssert(false, "Unknown DirectX compression format");
        }
    }
    else if( b->GetFlags() & plMipmap::kBumpEnvMap )
    {
        texSize = 16;
        if( b->GetFlags() & plMipmap::kAlphaChannelFlag )
            formatType = D3DFMT_L6V5U5;
        else
            formatType = D3DFMT_V8U8;
    }
    else if( b->GetPixelSize() == 16 )
    {
        texSize = 16;
        if( b->GetFlags() & plMipmap::kIntensityMap )
        {
            if( b->GetFlags() & plMipmap::kAlphaChannelFlag )
                formatType = D3DFMT_A8L8;
            else
                formatType = D3DFMT_L8;
        }
        else
        {
            if( b->GetFlags() & plMipmap::kAlphaChannelFlag )
                formatType = D3DFMT_A4R4G4B4;
            else
                formatType = D3DFMT_A1R5G5B5;
        }
    }
    else if( b->GetFlags() & plMipmap::kIntensityMap )
    {
        if( b->GetFlags() & plMipmap::kAlphaChannelFlag )
        {
            if( ITextureFormatAllowed( D3DFMT_A8L8 ) )
            {
                formatType = D3DFMT_A8L8;
                texSize = 16;
            }
            else if( !prefer32bit && ( fColorDepth == 16 ) && ITextureFormatAllowed( D3DFMT_A4R4G4B4 ) )
            {
                formatType = D3DFMT_A4R4G4B4;
                texSize = 16;
            }
            else if( ITextureFormatAllowed( D3DFMT_A8R8G8B8 ) )
            {
                formatType = D3DFMT_A8R8G8B8;
                texSize = 32;
            }
            else if( ITextureFormatAllowed( D3DFMT_A4R4G4B4 ) )
            {
                formatType = D3DFMT_A4R4G4B4;
                texSize = 16;
            }
        }
        else
        {
            if( ITextureFormatAllowed( D3DFMT_L8 ) )
            {
                formatType = D3DFMT_L8;
                texSize = 8;
            }
            else if( !prefer32bit && ( fColorDepth == 16 ) && ITextureFormatAllowed( D3DFMT_A1R5G5B5 ) )
            {
                formatType = D3DFMT_A1R5G5B5;
                texSize = 16;
            }
            else if( ITextureFormatAllowed( D3DFMT_A8R8G8B8 ) )
            {
                formatType = D3DFMT_A8R8G8B8;
                texSize = 32;
            }
            else if( ITextureFormatAllowed( D3DFMT_A1R5G5B5 ) )
            {
                formatType = D3DFMT_A1R5G5B5;
                texSize = 16;
            }
        }
    }
    else
    {
        if( b->GetFlags() & plMipmap::kAlphaChannelFlag )
        {
            if( !prefer32bit && ( fColorDepth == 16 ) && ITextureFormatAllowed( D3DFMT_A4R4G4B4 ) )
            {
                formatType = D3DFMT_A4R4G4B4;
                texSize = 16;
            }
            else if( ITextureFormatAllowed( D3DFMT_A8R8G8B8 ) )
            {
                formatType = D3DFMT_A8R8G8B8;
                texSize = 32;
            }
            else if( ITextureFormatAllowed( D3DFMT_A4R4G4B4 ) )
            {
                formatType = D3DFMT_A4R4G4B4;
                texSize = 16;
            }       
        }
        else
        {
            if( !prefer32bit && ( fColorDepth == 16 ) && ITextureFormatAllowed( D3DFMT_A1R5G5B5 ) )
            {
                formatType = D3DFMT_A1R5G5B5;
                texSize = 16;
            }
            else if( ITextureFormatAllowed( D3DFMT_A8R8G8B8 ) )
            {
                formatType = D3DFMT_A8R8G8B8;
                texSize = 32;
            }
            else if( ITextureFormatAllowed( D3DFMT_A1R5G5B5 ) )
            {
                formatType = D3DFMT_A1R5G5B5;
                texSize = 16;
            }
        }
    }

    hsAssert( formatType, "failing to find format type" );
}

//// IFormatTextureData ///////////////////////////////////////////////////////
// Convert the input 32 bit uncompressed RGBA data into the requested format.
void    plDXPipeline::IFormatTextureData( uint32_t formatType, uint32_t numPix, hsRGBAColor32* const src, void *dst )
{
    switch( formatType )
    {
        case D3DFMT_L6V5U5:
            {
                uint16_t *pixels = (uint16_t *)dst;
                hsRGBAColor32* p = src;
                hsRGBAColor32* end = src + numPix;

                while( p < end )
                {
                    *pixels = ((p->a << 8) & 0xfc00) 
                        | ((p->g << 2) & 0x03e0)
                        | ((p->r >> 3) & 0x001f);
#ifdef HS_DEBUGGING
                    if( *pixels & 0xfc00 )
                        pixels++;
                    else if( *pixels & 0x03e0 )
                        pixels++;
                    else if( *pixels & 0x001f )
                        pixels++;
                    else
#endif // HS_DEBUGGING
                    pixels++;
                    p++;
                }
            }
            break;

        case D3DFMT_V8U8:
            {
                uint16_t *pixels = (uint16_t *)dst;
                hsRGBAColor32* p = src;
                hsRGBAColor32* end = src + numPix;

                while( p < end )
                {
                    *pixels = (p->g << 8)
                        | (p->r << 0);
                    pixels++;
                    p++;
                }
            }
            break;

        case D3DFMT_A8L8:
            {
                uint16_t *pixels = (uint16_t *)dst;
                int i;
                hsRGBAColor32* const p = src;
                
                for(i =0; i < numPix; i++)
                    pixels[i]= ((p[i].a & 0xff) << 8) | (p[i].r & 0xff);
            }
            break;

        case D3DFMT_A4R4G4B4:
            {
                uint16_t *pixels = (uint16_t *)dst;
                int i;
                hsRGBAColor32* const p = src;

                for(i =0; i < numPix; i++)
                {
                    pixels[i]= (((p[i].r>>4) & 0xf) << 8) 
                                | (((p[i].g >> 4) & 0xf) << 4) 
                                | (((p[i].b >> 4) & 0xf) )
                                | (((p[i].a >> 4) & 0xf) << 12);
                }
            }
            break;

        case D3DFMT_A1R5G5B5:
            {
                uint16_t *pixels = (uint16_t *)dst;
                int i;
                hsRGBAColor32* const p = src;

                for(i =0; i < numPix; i++)
                {
                    pixels[i]= (((p[i].r>>3) & 0x1f) << 10) | 
                                (((p[i].g >> 3) & 0x1f) << 5) |
                                ((p[i].b >> 3) & 0x1f) | ((p[i].a == 0) ? 0 : 0x8000);
                }
            }
            break;

        case D3DFMT_L8:
            {
                uint8_t *pixels = (uint8_t *)dst;
                int i;
                hsRGBAColor32* const p = src;

                for(i =0; i < numPix; i++)
                    pixels[i]= p[i].r;
            }
            break;

        case D3DFMT_A8R8G8B8:
            {
                uint32_t *pixels = (uint32_t *)dst;
                int i;
                hsRGBAColor32* const p = src;

                for(i =0; i < numPix; i++)
                    pixels[i]= ( ( p[i].a << 24 ) | ( p[i].r << 16 ) | ( p[i].g << 8 ) | p[i].b );
            }
            break;

        default:
            hsAssert(false, "Unknown texture format selected");
            break;
    }
}


///////////////////////////////////////////////////////////////////////////////
//// View Stuff ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IIsViewLeftHanded ////////////////////////////////////////////////////////
//  Returns true if the combination of the local2world and world2camera
//  matrices is left-handed.

bool  plDXPipeline::IIsViewLeftHanded()
{
    return fView.GetViewTransform().GetOrthogonal() ^ ( fView.fLocalToWorldLeftHanded ^ fView.fWorldToCamLeftHanded ) ? true : false;
}


///////////////////////////////////////////////////////////////////////////////
//// Transforms ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// IMatrix44ToD3DMatrix /////////////////////////////////////////////////////
// Make a D3DXMATRIX matching the input plasma matrix. Mostly a transpose.
D3DMATRIX&     plDXPipeline::IMatrix44ToD3DMatrix( D3DMATRIX& dst, const hsMatrix44& src )
{
    if (src.fFlags & hsMatrix44::kIsIdent) {
        dst = d3dIdentityMatrix;
    } else {
        dst.m[0][0] = src.fMap[0][0];
        dst.m[1][0] = src.fMap[0][1];
        dst.m[2][0] = src.fMap[0][2];
        dst.m[3][0] = src.fMap[0][3];

        dst.m[0][1] = src.fMap[1][0];
        dst.m[1][1] = src.fMap[1][1];
        dst.m[2][1] = src.fMap[1][2];
        dst.m[3][1] = src.fMap[1][3];

        dst.m[0][2] = src.fMap[2][0];
        dst.m[1][2] = src.fMap[2][1];
        dst.m[2][2] = src.fMap[2][2];
        dst.m[3][2] = src.fMap[2][3];

        dst.m[0][3] = src.fMap[3][0];
        dst.m[1][3] = src.fMap[3][1];
        dst.m[2][3] = src.fMap[3][2];
        dst.m[3][3] = src.fMap[3][3];
    }

    return dst;
}


//// ISetCullMode /////////////////////////////////////////////////////////////
//  Tests and sets the current winding order cull mode (CW, CCW, or none).
// Will reverse the cull mode as necessary for left handed camera or local to world
// transforms.
void    plDXPipeline::ISetCullMode(bool flip)
{
    D3DCULL newCull = D3DCULL_NONE;

    if( !(fLayerState[0].fMiscFlags & hsGMatState::kMiscTwoSided) )
        newCull = !IIsViewLeftHanded() ^ !flip ? D3DCULL_CW : D3DCULL_CCW;

    if( newCull != fDevice.fCurrCullMode )
    {
        fDevice.fCurrCullMode = newCull;
        fD3DDevice->SetRenderState( D3DRS_CULLMODE, fDevice.fCurrCullMode );
    }
}

// ISetupVertexBufferRef /////////////////////////////////////////////////////////
// Initialize input vertex buffer ref according to source.
void plDXPipeline::ISetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, plDXVertexBufferRef* vRef)
{
    // Initialize to nullptr, in case something goes wrong.
    vRef->fD3DBuffer = nullptr;

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

    vRef->fDevice = fD3DDevice;

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

// ICheckStaticVertexBuffer ///////////////////////////////////////////////////////////////////////
// Ensure a static vertex buffer has any D3D resources necessary for rendering created and filled
// with proper vertex data.
void plDXPipeline::ICheckStaticVertexBuffer(plDXVertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx)
{
    hsAssert(!vRef->Volatile(), "Creating a managed vertex buffer for a volatile buffer ref");

    if( !vRef->fD3DBuffer )
    {
        // Okay, haven't done this one.

        DWORD fvfFormat = IGetBufferD3DFormat(vRef->fFormat);

    
        D3DPOOL poolType = D3DPOOL_MANAGED;
//      DWORD usage = D3DUSAGE_WRITEONLY;
        DWORD usage = 0;
        const int numVerts = vRef->fCount;
        const int vertSize = vRef->fVertexSize;
        fManagedAlloced = true;
        if( FAILED( fD3DDevice->CreateVertexBuffer( numVerts * vertSize,
                                                    usage, 
                                                    fvfFormat,
                                                    poolType, 
                                                    &vRef->fD3DBuffer, nullptr)))
        {
            hsAssert( false, "CreateVertexBuffer() call failed!" );
            vRef->fD3DBuffer = nullptr;
            return;
        }
        PROFILE_POOL_MEM(poolType, numVerts * vertSize, true, "VtxBuff");

        // Record that we've allocated this into managed memory, in case we're
        // fighting that NVidia driver bug. Search for OSVERSION for mor info.
        AllocManagedVertex(numVerts * vertSize);

        // Fill in the vertex data.
        IFillStaticVertexBufferRef(vRef, owner, idx);

        // This is currently a no op, but this would let the buffer know it can
        // unload the system memory copy, since we have a managed version now.
        owner->PurgeVertBuffer(idx);
    }
}

// IFillStaticVertexBufferRef //////////////////////////////////////////////////
// BufferRef is set up, just copy the data in.
// This is uglied up hugely by the insane non-interleaved data case with cells
// and whatever else.
void plDXPipeline::IFillStaticVertexBufferRef(plDXVertexBufferRef *ref, plGBufferGroup *group, uint32_t idx)
{
    IDirect3DVertexBuffer9* vertexBuff = ref->fD3DBuffer;

    if( !vertexBuff )
    {
        // We most likely already warned about this earlier, best to just quietly return now
        return;
    }

    const uint32_t vertSize = ref->fVertexSize;
    const uint32_t vertStart = group->GetVertBufferStart(idx) * vertSize;
    const uint32_t size = group->GetVertBufferEnd(idx) * vertSize - vertStart;
    if( !size )
        return;

    /// Lock the buffer
    uint8_t* ptr;
    if( FAILED( vertexBuff->Lock( vertStart, size, (void **)&ptr, group->AreVertsVolatile() ? D3DLOCK_DISCARD : 0 ) ) )
    {
        hsAssert( false, "Failed to lock vertex buffer for writing" );
    }

    if( ref->fData )
    {
        memcpy(ptr, ref->fData + vertStart, size);
    }
    else
    {
        hsAssert(0 == vertStart, "Offsets on non-interleaved data not supported");
        hsAssert(group->GetVertBufferCount(idx) * vertSize == size, "Trailing dead space on non-interleaved data not supported");

        const uint32_t vertSmallSize = group->GetVertexLiteStride() - sizeof( hsPoint3 ) * 2;
        uint8_t* srcVPtr = group->GetVertBufferData(idx);
        plGBufferColor* const srcCPtr = group->GetColorBufferData( idx );

        const int numCells = group->GetNumCells(idx);
        int i;
        for( i = 0; i < numCells; i++ )
        {
            plGBufferCell   *cell = group->GetCell( idx, i );

            if( cell->fColorStart == (uint32_t)-1 )
            {
                /// Interleaved, do straight copy
                memcpy( ptr, srcVPtr + cell->fVtxStart, cell->fLength * vertSize );
                ptr += cell->fLength * vertSize;
            }
            else
            {
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
    }

    /// Unlock and clean up
    vertexBuff->Unlock();
    ref->SetRebuiltSinceUsed(true);
    ref->SetDirty(false);
}

void plDXPipeline::IFillVolatileVertexBufferRef(plDXVertexBufferRef* ref, plGBufferGroup* group, uint32_t idx)
{
    uint8_t* dst = ref->fData;
    uint8_t* src = group->GetVertBufferData(idx);

    size_t uvChanSize = plGBufferGroup::CalcNumUVs(group->GetVertexFormat()) * sizeof(float) * 3;
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

// OpenAccess ////////////////////////////////////////////////////////////////////////////////////////
// Lock the managed buffer and setup the accessSpan to point into the buffers data.
bool plDXPipeline::OpenAccess(plAccessSpan& dst, plDrawableSpans* drawable, const plVertexSpan* span, bool readOnly)
{
    plGBufferGroup* grp = drawable->GetBufferGroup(span->fGroupIdx);
    hsAssert(!grp->AreVertsVolatile(), "Don't ask for D3DBuffer data on a volatile buffer");

    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)grp->GetVertexBufferRef(span->fVBufferIdx);
    if( !vRef )
    {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    IDirect3DVertexBuffer9* vertexBuff = vRef->fD3DBuffer;
    if( !vertexBuff )
    {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    const uint32_t stride = vRef->fVertexSize;
    const uint32_t vertStart = span->fVStartIdx * stride;
    const uint32_t size = span->fVLength * stride;

    if( !size )
    {
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    DWORD lockFlags = readOnly ? D3DLOCK_READONLY : 0;

    uint8_t* ptr;
    if( FAILED( vertexBuff->Lock(vertStart, size, (void **)&ptr, lockFlags) ) )
    {
        hsAssert( false, "Failed to lock vertex buffer for writing" );
        dst.SetType(plAccessSpan::kUndefined);
        return false;
    }

    plAccessVtxSpan& acc = dst.AccessVtx();

    acc.SetVertCount((uint16_t)(span->fVLength));

    int32_t offset = (-(int32_t)(span->fVStartIdx)) * ((int32_t)stride);

    acc.PositionStream(ptr, (uint16_t)stride, offset);
    ptr += sizeof(hsPoint3);

    int numWgts = grp->GetNumWeights();
    if( numWgts )
    {
        acc.SetNumWeights(numWgts);
        acc.WeightStream(ptr, (uint16_t)stride, offset);
        ptr += numWgts * sizeof(float);
        if( grp->GetVertexFormat() & plGBufferGroup::kSkinIndices )
        {
            acc.WgtIndexStream(ptr, (uint16_t)stride, offset);
            ptr += sizeof(uint32_t);
        }
        else
        {
            acc.WgtIndexStream(nullptr, 0, offset);
        }
    }
    else
    {
        acc.SetNumWeights(0);
    }

    acc.NormalStream(ptr, (uint16_t)stride, offset);
    ptr += sizeof(hsVector3);

    acc.DiffuseStream(ptr, (uint16_t)stride, offset);
    ptr += sizeof(uint32_t);

    acc.SpecularStream(ptr, (uint16_t)stride, offset);
    ptr += sizeof(uint32_t);

    acc.UVWStream(ptr, (uint16_t)stride, offset);

    acc.SetNumUVWs(grp->GetNumUVs());

    acc.SetVtxDeviceRef(vRef);

    return true;
}

// CloseAccess /////////////////////////////////////////////////////////////////////
// Unlock the buffer, invalidating the accessSpan.
bool plDXPipeline::CloseAccess(plAccessSpan& dst)
{
    if( !dst.HasAccessVtx() )
        return false;

    plAccessVtxSpan& acc = dst.AccessVtx();

    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)acc.GetVtxDeviceRef();
    if( !vRef )
        return false;

    IDirect3DVertexBuffer9* vertexBuff = vRef->fD3DBuffer;
    if( !vertexBuff )
        return false;

    vertexBuff->Unlock();

    return true;
}

// CheckVertexBufferRef /////////////////////////////////////////////////////
// Make sure the buffer group has a valid buffer ref and that it is up to date.
void plDXPipeline::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    // First, do we have a device ref at this index?
    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)owner->GetVertexBufferRef(idx);
    // If not
    if( !vRef )
    {
        // Make the blank ref
        vRef = new plDXVertexBufferRef;

        ISetupVertexBufferRef(owner, idx, vRef);

    }
    if( !vRef->IsLinked() )
        vRef->Link( &fVtxBuffRefList );

    // One way or another, we now have a vbufferref[idx] in owner.
    // Now, does it need to be (re)filled?
    // If the owner is volatile, then we hold off. It might not
    // be visible, and we might need to refill it again if we
    // have an overrun of our dynamic D3D buffer.
    if( !vRef->Volatile() )
    {
        if( fAllocUnManaged )
            return;

        // If it's a static buffer, allocate a D3D vertex buffer for it. Otherwise, it'll
        // be sharing the global D3D dynamic buffer, and marked as volatile.
        ICheckStaticVertexBuffer(vRef, owner, idx);

        // Might want to remove this assert, and replace it with a dirty check if
        // we have static buffers that change very seldom rather than never.
        hsAssert(!vRef->IsDirty(), "Non-volatile vertex buffers should never get dirty");
    }
    else
    {
        // Make sure we're going to be ready to fill it.

        if( !vRef->fData && (vRef->fFormat != owner->GetVertexFormat()) )
        {
            vRef->fData = new uint8_t[vRef->fCount * vRef->fVertexSize];
            IFillVolatileVertexBufferRef(vRef, owner, idx);
        }
    }
}

// CheckIndexBufferRef /////////////////////////////////////////////////////
// Make sure the buffer group has an index buffer ref and that its data is current.
void plDXPipeline::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    plDXIndexBufferRef* iRef = (plDXIndexBufferRef*)owner->GetIndexBufferRef(idx);
    if( !iRef )
    {
        // Create one from scratch.

        iRef = new plDXIndexBufferRef;

        ISetupIndexBufferRef(owner, idx, iRef);

    }
    if( !iRef->IsLinked() )
        iRef->Link(&fIdxBuffRefList);

    // Make sure it has all D3D resources created.
    ICheckIndexBuffer(iRef);

    // If it's dirty, refill it.
    if( iRef->IsDirty()  )
        IFillIndexBufferRef(iRef, owner, idx);
}

// IFillIndexBufferRef ////////////////////////////////////////////////////////////
// Refresh the D3D index buffer from the plasma index buffer.
void plDXPipeline::IFillIndexBufferRef(plDXIndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx)
{
    uint32_t startIdx = owner->GetIndexBufferStart(idx);
    uint32_t size = (owner->GetIndexBufferEnd(idx) - startIdx) * sizeof(uint16_t);
    if( !size )
        return;

    DWORD lockFlags = iRef->Volatile() ? D3DLOCK_DISCARD : 0;
    uint16_t* destPtr = nullptr;
    if( FAILED( iRef->fD3DBuffer->Lock(startIdx * sizeof(uint16_t), size, (void **)&destPtr, lockFlags) ) )
    {
        hsAssert( false, "Cannot lock index buffer for writing" );
        return;
    }

    memcpy( destPtr, owner->GetIndexBufferData(idx) + startIdx, size );
    
    iRef->fD3DBuffer->Unlock(); 

    iRef->SetDirty( false );

}

// ICheckIndexBuffer ////////////////////////////////////////////////////////
// Make sure index buffer ref has any D3D resources it needs.
void plDXPipeline::ICheckIndexBuffer(plDXIndexBufferRef* iRef)
{
    if( !iRef->fD3DBuffer && iRef->fCount )
    {
        D3DPOOL poolType = fAllocUnManaged ? D3DPOOL_DEFAULT : D3DPOOL_MANAGED;
        DWORD usage = D3DUSAGE_WRITEONLY;
        iRef->SetVolatile(false);
        if( FAILED( fD3DDevice->CreateIndexBuffer( sizeof( uint16_t ) * iRef->fCount,
                                                    usage, 
                                                    D3DFMT_INDEX16, 
                                                    poolType, 
                                                    &iRef->fD3DBuffer, nullptr)))
        {
            hsAssert( false, "CreateIndexBuffer() call failed!" );
            iRef->fD3DBuffer = nullptr;
            return;
        }
        PROFILE_POOL_MEM(poolType, sizeof(uint16_t) * iRef->fCount, true, "IndexBuff");

        iRef->fPoolType = poolType;
        iRef->SetDirty(true);
        iRef->SetRebuiltSinceUsed(true);
    }
}

// ISetupIndexBufferRef ////////////////////////////////////////////////////////////////
// Initialize the index buffer ref, but don't create anything for it.
void plDXPipeline::ISetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, plDXIndexBufferRef* iRef)
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

//// ISoftwareVertexBlend ///////////////////////////////////////////////////////
// Emulate matrix palette operations in software. The big difference between the hardware
// and software versions is we only want to lock the vertex buffer once and blend all the
// verts we're going to in software, so the vertex blend happens once for an entire drawable.
// In hardware, we want the opposite, to break it into managable chunks, manageable meaning
// few enough matrices to fit into hardware registers. So for hardware version, we set up
// our palette, draw a span or few, setup our matrix palette with new matrices, draw, repeat.
bool      plDXPipeline::ISoftwareVertexBlend(plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoSkinning))
        return true;

    if( drawable->GetSkinTime() == fRenderCnt )
        return true;

    const hsBitVector   &blendBits = drawable->GetBlendingSpanVector();

    if( drawable->GetBlendingSpanVector().Empty() )
    {
        // This sucker doesn't have any skinning spans anyway. Just return
        drawable->SetSkinTime( fRenderCnt );
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

    const hsTArray<plSpan *>& spans = drawable->GetSpanArray();
    for (int16_t idx : visList)
    {
        if (blendBits.IsBitSet(idx))
        {
            const plVertexSpan &vSpan = *(plVertexSpan *)spans[idx];
            hsAssert(kMaxVertexBuffers > vSpan.fVBufferIdx, "Bigger than we counted on num buffers skin.");

            blendBuffers[vSpan.fGroupIdx][vSpan.fVBufferIdx] = 1;
            drawable->SetBlendingSpanVectorBit(idx, false);
        }
    }

    // Now go through each of the group/buffer (= a real vertex buffer) pairs we found,
    // and blend into it. We'll lock the buffer once, and then for each span that
    // uses it, set the matrix palette and and then do the blend for that span.
    // When we've done all the spans for a group/buffer, we unlock it and move on.
    int j;
    for (int i = 0; i < kMaxBufferGroups; i++)
    {
        for( j = 0; j < kMaxVertexBuffers; j++ )
        {
            if( blendBuffers[i][j] )
            {
                // Found one. Do the lock.
                plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)drawable->GetVertexRef(i, j);

                hsAssert(vRef->fData, "Going into skinning with no place to put results!");

                uint8_t*  destPtr = vRef->fData;

                for (int16_t idx : visList)
                {
                    const plIcicle& span = *(plIcicle*)spans[idx];
                    if( (span.fGroupIdx == i)&&(span.fVBufferIdx == j) )
                    {
                        plProfile_Inc(NumSkin);

                        hsMatrix44* matrixPalette = drawable->GetMatrixPalette(span.fBaseMatrix);
                        matrixPalette[0] = span.fLocalToWorld;

                        uint8_t* ptr = vRef->fOwner->GetVertBufferData(vRef->fIndex);
                        ptr += span.fVStartIdx * vRef->fOwner->GetVertexSize();
                        IBlendVertsIntoBuffer( (plSpan*)&span,
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

    if( drawable->GetBlendingSpanVector().Empty() )
    {
        // Only do this if we've blended ALL of the spans. Thus, this becomes a trivial 
        // rejection for all the skinning flags being cleared
        drawable->SetSkinTime(fRenderCnt);
    }

    return true;
}

// IBeginAllocUnmanaged ///////////////////////////////////////////////////////////////////
// Before allocating anything into POOL_DEFAULT, we must evict managed memory.
// See LoadResources.
void plDXPipeline::IBeginAllocUnManaged()
{
    // Flush out all managed resources to make room for unmanaged resources.
    fD3DDevice->EvictManagedResources();

    fManagedAlloced = false;
    fAllocUnManaged = true; // we're currently only allocating POOL_DEFAULT
}

// IEndAllocUnManged.
// Before allocating anything into POOL_DEFAULT, we must evict managed memory.
// See LoadResources.
void plDXPipeline::IEndAllocUnManaged()
{
    fAllocUnManaged = false;

    // Flush the (should be empty) resource manager to reset its internal allocation pool.
    fD3DDevice->EvictManagedResources();
}

bool plDXPipeline::CheckResources()
{
    if ((fClothingOutfits.GetCount() <= 1 && fAvRTPool.GetCount() > 1) ||
        (fAvRTPool.GetCount() >= 16 && (fAvRTPool.GetCount() / 2 >= fClothingOutfits.GetCount())))
    {
        return (hsTimer::GetSysSeconds() - fAvRTShrinkValidSince > kAvTexPoolShrinkThresh);
    }

    fAvRTShrinkValidSince = hsTimer::GetSysSeconds();
    return (fAvRTPool.GetCount() < fClothingOutfits.GetCount());
}

// LoadResources ///////////////////////////////////////////////////////////////////////
// Basically, we have to load anything that goes into POOL_DEFAULT before
// anything into POOL_MANAGED, or the memory manager gets confused.
// More precisely, we have to evict everything from POOL_MANAGED before we
// can allocate anything into POOL_DEFAULT.
// So, this function frees up everything in POOL_DEFAULT, evicts managed memory,
// calls out for anything needing to be created POOL_DEFAULT to do so,
// Then we're free to load into POOL_MANAGED on demand.
// This is typically called at the beginning of the first render after loading
// a new age.
void plDXPipeline::LoadResources()
{
    hsStatusMessageF("Begin Device Reload t=%f",hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("Begin Device Reload");

    // Just to be safe.
    IInitDeviceState(); // 9700 THRASH

    // Evict mananged memory.
    IBeginAllocUnManaged();

    // Release everything we have in POOL_DEFAULT.
    IReleaseDynamicBuffers();
    IReleaseAvRTPool();

    // Create all RenderTargets
    plPipeRTMakeMsg* rtMake = new plPipeRTMakeMsg(this);
    rtMake->Send();

    // Create all our shadow render targets and pipeline specific POOL_DEFAULT vertex buffers.
    // This includes our single dynamic vertex buffer that we cycle through for software
    // skinned, particle systems, etc.
    ICreateDynamicBuffers();

    // Create all POOL_DEFAULT (sorted) index buffers in the scene.
    plPipeGeoMakeMsg* defMake = new plPipeGeoMakeMsg(this, true);
    defMake->Send();

    // This can be a bit of a mem hog and will use more mem if available, so keep it last in the
    // POOL_DEFAULT allocs.
    IFillAvRTPool();

    // We should have everything POOL_DEFAULT we need now.
    IEndAllocUnManaged();

    // Force a create of all our static D3D vertex buffers.
#define MF_PRELOAD_MANAGEDBUFFERS
#ifdef MF_PRELOAD_MANAGEDBUFFERS
    plPipeGeoMakeMsg* manMake = new plPipeGeoMakeMsg(this, false);
    manMake->Send();
#endif // MF_PRELOAD_MANAGEDBUFFERS

    // Forcing a preload of textures turned out to not be so great,
    // since there are typically so many in an age, it swamped out
    // VM.
#ifdef MF_TOSSER
#define MF_PRELOAD_TEXTURES
#endif // MF_TOSSER
#ifdef MF_PRELOAD_TEXTURES
    plPipeTexMakeMsg* texMake = new plPipeTexMakeMsg(this);
    texMake->Send();
#endif // MF_PRELOAD_TEXTURES

    fD3DDevice->EvictManagedResources();

    // Okay, we've done it, clear the request.
    plPipeResReq::Clear();

    plProfile_IncCount(PipeReload, 1);

    hsStatusMessageF("End Device Reload t=%f",hsTimer::GetSeconds());
    plNetClientApp::StaticDebugMsg("End Device Reload");
}

// inlTESTPOINT /////////////////////////////////////////
// Update mins and maxs if destP is outside.
inline void inlTESTPOINT(const hsPoint3& destP,
                         float& minX, float& minY, float& minZ,
                         float& maxX, float& maxY, float& maxZ)
{
    if( destP.fX < minX )
        minX = destP.fX;
    else if( destP.fX > maxX )
        maxX = destP.fX;

    if( destP.fY < minY )
        minY = destP.fY;
    else if( destP.fY > maxY )
        maxY = destP.fY;

    if( destP.fZ < minZ )
        minZ = destP.fZ;
    else if( destP.fZ > maxZ )
        maxZ = destP.fZ;
}

//// IBlendVertsIntoBuffer ////////////////////////////////////////////////////
//  Given a pointer into a buffer of verts that have blending data in the D3D
//  format, blends them into the destination buffer given without the blending
//  info.

static inline void ISkinVertexFPU(const hsMatrix44& xfm, float wgt,
                                  const float* pt_src, float* pt_dst,
                                  const float* vec_src, float* vec_dst)
{
    const float& m00 = xfm.fMap[0][0];
    const float& m01 = xfm.fMap[0][1];
    const float& m02 = xfm.fMap[0][2];
    const float& m03 = xfm.fMap[0][3];
    const float& m10 = xfm.fMap[1][0];
    const float& m11 = xfm.fMap[1][1];
    const float& m12 = xfm.fMap[1][2];
    const float& m13 = xfm.fMap[1][3];
    const float& m20 = xfm.fMap[2][0];
    const float& m21 = xfm.fMap[2][1];
    const float& m22 = xfm.fMap[2][2];
    const float& m23 = xfm.fMap[2][3];

    // position
    {
        const float& srcX = pt_src[0];
        const float& srcY = pt_src[1];
        const float& srcZ = pt_src[2];

        pt_dst[0] += (srcX * m00 + srcY * m01 + srcZ * m02 + m03) * wgt;
        pt_dst[1] += (srcX * m10 + srcY * m11 + srcZ * m12 + m13) * wgt;
        pt_dst[2] += (srcX * m20 + srcY * m21 + srcZ * m22 + m23) * wgt;
    }

    // normal
    {
        const float& srcX = vec_src[0];
        const float& srcY = vec_src[1];
        const float& srcZ = vec_src[2];

        vec_dst[0] += (srcX * m00 + srcY * m01 + srcZ * m02) * wgt;
        vec_dst[1] += (srcX * m10 + srcY * m11 + srcZ * m12) * wgt;
        vec_dst[1] += (srcX * m20 + srcY * m21 + srcZ * m22) * wgt;
    }
}

#ifdef HAVE_SSE3
static inline void ISkinDpSSE3(const float* src, float* dst, const __m128& mc0,
                               const __m128& mc1, const __m128& mc2, const __m128& mwt)
{
    __m128 msr = _mm_load_ps(src);
    __m128 _x  = _mm_mul_ps(_mm_mul_ps(mc0, msr), mwt);
    __m128 _y  = _mm_mul_ps(_mm_mul_ps(mc1, msr), mwt);
    __m128 _z  = _mm_mul_ps(_mm_mul_ps(mc2, msr), mwt);

    __m128 hbuf1 = _mm_hadd_ps(_x, _y);
    __m128 hbuf2 = _mm_hadd_ps(_z, _z);
    hbuf1 = _mm_hadd_ps(hbuf1, hbuf2);
    __m128 _dst = _mm_load_ps(dst);
    _dst = _mm_add_ps(_dst, hbuf1);
    _mm_store_ps(dst, _dst);
}
#endif // HAVE_SSE3

static inline void ISkinVertexSSE3(const hsMatrix44& xfm, float wgt,
                                   const float* pt_src, float* pt_dst,
                                   const float* vec_src, float* vec_dst)
{
#ifdef HAVE_SSE3
    __m128 mc0 = _mm_load_ps(xfm.fMap[0]);
    __m128 mc1 = _mm_load_ps(xfm.fMap[1]);
    __m128 mc2 = _mm_load_ps(xfm.fMap[2]);
    __m128 mwt = _mm_set_ps1(wgt);

    ISkinDpSSE3(pt_src, pt_dst, mc0, mc1, mc2, mwt);
    ISkinDpSSE3(vec_src, vec_dst, mc0, mc1, mc2, mwt);
#endif // HAVE_SSE3
}

#ifdef HAVE_SSE41
static inline void ISkinDpSSE41(const float* src, float* dst, const __m128& mc0,
                                const __m128& mc1, const __m128& mc2, const __m128& mwt)
{
    enum { DP_F4_X = 0xF1, DP_F4_Y = 0xF2, DP_F4_Z = 0xF4 };

    __m128 msr = _mm_load_ps(src);
    __m128 _r =        _mm_dp_ps(msr, mc0, DP_F4_X);
    _r = _mm_or_ps(_r, _mm_dp_ps(msr, mc1, DP_F4_Y));
    _r = _mm_or_ps(_r, _mm_dp_ps(msr, mc2, DP_F4_Z));

    __m128 _dst = _mm_load_ps(dst);
    _dst = _mm_add_ps(_dst, _mm_mul_ps(_r, mwt));
    _mm_store_ps(dst, _dst);
}
#endif // HAVE_SSE41

static inline void ISkinVertexSSE41(const hsMatrix44& xfm, float wgt,
                                    const float* pt_src, float* pt_dst,
                                    const float* vec_src, float* vec_dst)
{
#ifdef HAVE_SSE41
    __m128 mc0 = _mm_load_ps(xfm.fMap[0]);
    __m128 mc1 = _mm_load_ps(xfm.fMap[1]);
    __m128 mc2 = _mm_load_ps(xfm.fMap[2]);
    __m128 mwt = _mm_set_ps1(wgt);

    ISkinDpSSE41(pt_src, pt_dst, mc0, mc1, mc2, mwt);
    ISkinDpSSE41(vec_src, vec_dst, mc0, mc1, mc2, mwt);
#endif // HAVE_SSE41
}

typedef void(*skin_vert_ptr)(const hsMatrix44&, float, const float*, float*, const float*, float*);

template<skin_vert_ptr T>
static void IBlendVertBuffer(plSpan* span, hsMatrix44* matrixPalette, int numMatrices,
                             const uint8_t* src, uint8_t format, uint32_t srcStride,
                             uint8_t* dest, uint32_t destStride, uint32_t count,
                             uint16_t localUVWChans)
{
    ALIGN(16) float pt_buf[] = { 0.f, 0.f, 0.f, 1.f };
    ALIGN(16) float vec_buf[] = { 0.f, 0.f, 0.f, 0.f };
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
        ALIGN(16) float destNorm_buf[] = { 0.f, 0.f, 0.f, 0.f };
        ALIGN(16) float destPt_buf[] = { 0.f, 0.f, 0.f, 1.f };

        // Blend
        for (uint32_t j = 0; j < numWeights + 1; ++j) {
            if (weights[j])
                T(matrixPalette[indices & 0xFF], weights[j], pt_buf, destPt_buf, vec_buf, destNorm_buf);
            indices >>= 8;
        }
        // Probably don't really need to renormalize this. There errors are
        // going to be subtle and "smooth".
        /* hsFastMath::NormalizeAppr(destNorm); */

        // Slam data into position now
        dest = inlStuff<hsPoint3>(dest, reinterpret_cast<hsPoint3*>(destPt_buf));
        dest = inlStuff<hsVector3>(dest, reinterpret_cast<hsVector3*>(destNorm_buf));

        // Jump past colors and UVws
        dest += sizeof(uint32_t) * 2 + uvChanSize;
        src  += sizeof(uint32_t) * 2 + uvChanSize;
    }
}

// CPU-optimized functions requiring dispatch
hsCpuFunctionDispatcher<plDXPipeline::blend_vert_buffer_ptr> plDXPipeline::blend_vert_buffer {
    &IBlendVertBuffer<ISkinVertexFPU>,
    nullptr,                                // SSE1
    nullptr,                                // SSE2
    &IBlendVertBuffer<ISkinVertexSSE3>,
    nullptr,                                // SSSE3
    &IBlendVertBuffer<ISkinVertexSSE41>
};

// ISetPipeConsts //////////////////////////////////////////////////////////////////
// A shader can request that the pipeline fill in certain constants that are indeterminate
// until the pipeline is about to render the object the shader is applied to. For example,
// the object's local to world. A single shader may be used on multiple objects with 
// multiple local to world transforms. This ensures the pipeline will shove the proper
// local to world into the shader immediately before the render.
// See plShader.h for the list of available pipe constants.
// Note that the lighting pipe constants are NOT implemented.
void plDXPipeline::ISetPipeConsts(plShader* shader)
{
    size_t n = shader->GetNumPipeConsts();
    for (size_t i = 0; i < n; i++)
    {
        const plPipeConst& pc = shader->GetPipeConst(i);
        switch( pc.fType )
        {
        case plPipeConst::kFogSet:
            {
                float set[4];
                IGetVSFogSet(set);
                shader->SetFloat4(pc.fReg, set);
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
// be nullptr, in which case the fixed function pipeline is indicated.
// Any Pipe Constants the non-FFP shader wants will be set here.
// Lastly, all constants will be set (as a block) for any non-FFP vertex or pixel shader.
HRESULT plDXPipeline::ISetShaders(plShader* vShader, plShader* pShader)
{
    IDirect3DVertexShader9 *vsHandle = nullptr;
    if( vShader )
    {
        hsAssert(vShader->IsVertexShader(), "Wrong type shader as vertex shader");
        ISetPipeConsts(vShader);

        plDXVertexShader* vRef = (plDXVertexShader*)vShader->GetDeviceRef();
        if( !vRef )
        {
            vRef = new plDXVertexShader(vShader);
            hsRefCnt_SafeUnRef(vRef);
        }
        if( !vRef->IsLinked() )
            vRef->Link(&fVShaderRefList);
        vsHandle = vRef->GetShader(this);

        // This is truly obnoxious, but D3D insists that, while using the progammable pipeline,
        // all stages be set up like this, not just the ones we're going to use. We have to
        // do this if we have either a vertex or a pixel shader. See below. Whatever. mf
        int i;
        for( i = 0; i < 8; i++ )
        {
            fD3DDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, fLayerUVWSrcs[i] = i);
            fD3DDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, fLayerXformFlags[i] = 0);
        }
    }

    IDirect3DPixelShader9 *psHandle = nullptr;
    if( pShader )
    {
        hsAssert(pShader->IsPixelShader(), "Wrong type shader as pixel shader");

        ISetPipeConsts(pShader);

        plDXPixelShader* pRef = (plDXPixelShader*)pShader->GetDeviceRef();
        if( !pRef )
        {
            pRef = new plDXPixelShader(pShader);
            hsRefCnt_SafeUnRef(pRef);
        }
        if( !pRef->IsLinked() )
            pRef->Link(&fPShaderRefList);
        psHandle = pRef->GetShader(this);

        if( !vShader )
        {
            int i;
            for( i = 0; i < 8; i++ )
            {
                fD3DDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, fLayerUVWSrcs[i] = i);
                fD3DDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, fLayerXformFlags[i] = 0);
            }
        }
    }

    if( vsHandle != fSettings.fCurrVertexShader )
    {
        HRESULT hr = fD3DDevice->SetVertexShader(fSettings.fCurrVertexShader = vsHandle);
        hsAssert(!FAILED(hr), "Error setting vertex shader");
    }

    if( psHandle != fSettings.fCurrPixelShader )
    {
        HRESULT hr = fD3DDevice->SetPixelShader(fSettings.fCurrPixelShader = psHandle);
        hsAssert(!FAILED(hr), "Error setting pixel shader");
    }

    // Handle cull mode here, because current cullmode is dependent on
    // the handedness of the LocalToCamera AND whether we are twosided.
    ISetCullMode();

    return S_OK;
}

// IRenderAuxSpan //////////////////////////////////////////////////////////
// Aux spans (auxilliary) are geometry rendered immediately after, and therefore dependent, on
// other normal geometry. They don't have SceneObjects, Drawables, DrawInterfaces or
// any of that, and therefore don't correspond to any object in the scene.
// They are dynamic procedural decals. See plDynaDecal.cpp and plDynaDecalMgr.cpp.
// This is wrapped by IRenderAuxSpans, which makes sure state is restored to resume
// normal rendering after the AuxSpan is rendered.
void plDXPipeline::IRenderAuxSpan(const plSpan& span, const plAuxSpan* aux)
{
    // Make sure the underlying resources are created and filled in with current data.
    CheckVertexBufferRef(aux->fGroup, aux->fVBufferIdx);
    CheckIndexBufferRef(aux->fGroup, aux->fIBufferIdx);
    ICheckAuxBuffers(aux);

    // Set to render from the aux spans buffers.
    plDXVertexBufferRef* vRef = (plDXVertexBufferRef*)aux->fGroup->GetVertexBufferRef(aux->fVBufferIdx); 

    if( !vRef )
        return;

    plDXIndexBufferRef* iRef = (plDXIndexBufferRef*)aux->fGroup->GetIndexBufferRef(aux->fIBufferIdx);

    if( !iRef )
        return;

    HRESULT     r;

    r = fD3DDevice->SetStreamSource( 0, vRef->fD3DBuffer, 0, vRef->fVertexSize );
    hsAssert( r == D3D_OK, "Error trying to set the stream source!" );
    plProfile_Inc(VertexChange);

    fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = IGetBufferD3DFormat(vRef->fFormat));
    
    r = fD3DDevice->SetIndices( iRef->fD3DBuffer );
    hsAssert( r == D3D_OK, "Error trying to set the indices!" );

    plRenderTriListFunc render(fD3DDevice, iRef->fOffset, aux->fVStartIdx, aux->fVLength, aux->fIStartIdx, aux->fILength/3);

    // Now just loop through the aux material, rendering in as many passes as it takes.
    hsGMaterial* material = aux->fMaterial;
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
    }

}

// IRenderAuxSpans ////////////////////////////////////////////////////////////////////////////
// Save and restore render state around calls to IRenderAuxSpan. This lets
// a list of aux spans get rendered with only one save/restore state.
void plDXPipeline::IRenderAuxSpans(const plSpan& span)
{
    if (IsDebugFlagSet(plPipeDbg::kFlagNoAuxSpans))
        return;

    plDXVertexBufferRef* oldVRef = fSettings.fCurrVertexBuffRef;
    plDXIndexBufferRef* oldIRef = fSettings.fCurrIndexBuffRef;

    ISetLocalToWorld(hsMatrix44::IdentityMatrix(), hsMatrix44::IdentityMatrix());

    int i;
    for( i = 0; i < span.GetNumAuxSpans(); i++ )
        IRenderAuxSpan(span, span.GetAuxSpan(i));

    ISetLocalToWorld(span.fLocalToWorld, span.fWorldToLocal);

    HRESULT     r;

    r = fD3DDevice->SetStreamSource( 0, oldVRef->fD3DBuffer, 0, oldVRef->fVertexSize );
    hsAssert( r == D3D_OK, "Error trying to set the stream source!" );
    plProfile_Inc(VertexChange);

    r = fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = IGetBufferD3DFormat(oldVRef->fFormat));

    r = fD3DDevice->SetIndices( oldIRef->fD3DBuffer );
    hsAssert( r == D3D_OK, "Error trying to set the indices!" );

}

//// IRenderBufferSpan ////////////////////////////////////////////////////////
// Sets up the vertex and index buffers for a span, and then
// renders it in as many passes as it takes in ILoopOverLayers.
void    plDXPipeline::IRenderBufferSpan( const plIcicle& span,
                                         hsGDeviceRef *vb, hsGDeviceRef *ib, 
                                         hsGMaterial *material, uint32_t vStart, uint32_t vLength, 
                                         uint32_t iStart, uint32_t iLength )
{
    plProfile_BeginTiming(RenderBuff);

    plDXVertexBufferRef *vRef = (plDXVertexBufferRef *)vb;
    plDXIndexBufferRef      *iRef = (plDXIndexBufferRef *)ib;

    HRESULT     r;

    if (vRef->fD3DBuffer == nullptr || iRef->fD3DBuffer == nullptr)
    {
        plProfile_EndTiming(RenderBuff);
        hsAssert( false, "Trying to render a nil buffer pair!" );
        return;
    }

    /// Switch to the vertex buffer we want
    if( fSettings.fCurrVertexBuffRef != vRef )
    {
        hsRefCnt_SafeAssign( fSettings.fCurrVertexBuffRef, vRef );
        hsAssert(vRef->fD3DBuffer != nullptr, "Trying to render a buffer pair without a vertex buffer!");
        vRef->SetRebuiltSinceUsed(true);
    }

    if( vRef->RebuiltSinceUsed() )
    {
        r = fD3DDevice->SetStreamSource( 0, vRef->fD3DBuffer, 0, vRef->fVertexSize );
        hsAssert( r == D3D_OK, "Error trying to set the stream source!" );
        plProfile_Inc(VertexChange);

        DWORD fvf = IGetBufferD3DFormat(vRef->fFormat);
        if (fSettings.fCurrFVFFormat != fvf)
            fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = fvf);

        vRef->SetRebuiltSinceUsed(false);
    }

    // Note: both these stats are the same, since we don't do any culling or clipping on the tris
    if( fSettings.fCurrIndexBuffRef != iRef )
    {
        hsRefCnt_SafeAssign( fSettings.fCurrIndexBuffRef, iRef );
        hsAssert(iRef->fD3DBuffer != nullptr, "Trying to render with a nil index buffer");
        iRef->SetRebuiltSinceUsed(true);
    }

    if( iRef->RebuiltSinceUsed() )
    {
        r = fD3DDevice->SetIndices( iRef->fD3DBuffer );
        hsAssert( r == D3D_OK, "Error trying to set the indices!" );
        plProfile_Inc(IndexChange);
        iRef->SetRebuiltSinceUsed(false);
    }

    plRenderTriListFunc render(fD3DDevice, iRef->fOffset, vStart, vLength, iStart, iLength/3);

    plProfile_EndTiming(RenderBuff);
    ILoopOverLayers(render, material, span);
}

// ILoopOverLayers /////////////////////////////////////////////////////////////////////////////////
// Render the input span with the input material in as many passes as it takes.
// Also handles rendering projected lights, either onto each pass or 
// once onto the FB after all the passes, as appropriate.
bool plDXPipeline::ILoopOverLayers(const plRenderPrimFunc& inRender, hsGMaterial* material, const plSpan& span)
{
    plProfile_BeginTiming(RenderPrim);

    const plRenderPrimFunc& render = IsDebugFlagSet(plPipeDbg::kFlagNoRender) ? (const plRenderPrimFunc&)sRenderNil : inRender;

    if( GetOverrideMaterial() )
        material = GetOverrideMaterial();

    IPushPiggyBacks(material);

    bool normalLightsDisabled = false;

    // Loop across all the layers we need to draw
    int j;
    for( j = 0; j < material->GetNumLayers(); )
    {
        int iCurrMat = j;
        j = IHandleMaterial( material, iCurrMat, &span );
        if (j == -1)
            break;

        if( (fLayerState[0].fBlendFlags & hsGMatState::kBlendAlpha)
                &&(material->GetLayer(iCurrMat)->GetOpacity() <= 0) 
                &&(fCurrLightingMethod != plSpan::kLiteVtxPreshaded) ) // This opt isn't good for particles, since their
                                                                        // material opacity is undefined/unused... -mcn
            continue;

        plProfile_BeginTiming(SpanFog);
        ISetFogParameters(&span, material->GetLayer(iCurrMat));
        plProfile_EndTiming(SpanFog);

        ISetShaders(material->GetLayer(iCurrMat)->GetVertexShader(), material->GetLayer(iCurrMat)->GetPixelShader());

        if( normalLightsDisabled )
            IRestoreSpanLights();

#ifdef HS_DEBUGGING
        DWORD nPass;
        fSettings.fDXError = fD3DDevice->ValidateDevice(&nPass);
        if( fSettings.fDXError != D3D_OK )
            IGetD3DError();
#endif // HS_DEBUGGING

        // Do the regular draw.
        render.RenderPrims();

        // Take care of projections that get applied to each pass.
        if (!fLights.fProjEach.empty() && !(fView.fRenderState & kRenderNoProjection))
        {
            // Disable all the normal lights.
            IDisableSpanLights();
            normalLightsDisabled = true;

            IRenderProjectionEach(render, material, iCurrMat, span);

        }
        if (IsDebugFlagSet(plPipeDbg::kFlagNoUpperLayers))
            j = material->GetNumLayers();
    }
    IPopPiggyBacks();

    // If we disabled lighting, re-enable it.
    if( normalLightsDisabled )
        IRestoreSpanLights();

    // Render any aux spans associated.
    if( span.GetNumAuxSpans() )
        IRenderAuxSpans(span);

    // Only render projections and shadows if we successfully rendered the span.
    // j == -1 means we aborted render.
    if( j >= 0 )
    {
        // Projections that get applied to the frame buffer (after all passes).
        if (!fLights.fProjAll.empty() && !(fView.fRenderState & kRenderNoProjection))
            IRenderProjections(render);

        // Handle render of shadows onto geometry.
        if( fShadows.GetCount() )
            IRenderShadowsOntoSpan(render, &span, material);
    }

    // Debug only
    if (IsDebugFlagSet(plPipeDbg::kFlagOverlayWire))
    {
        IRenderOverWire(render, material, span);
    }
    plProfile_EndTiming(RenderPrim);

    return false;
}

// IRenderOverWire ///////////////////////////////////////////////////////////////////////////////
// Debug only, renders wireframe on top of normal render.
void plDXPipeline::IRenderOverWire(const plRenderPrimFunc& render, hsGMaterial* material, const plSpan& span)
{
    uint32_t state = fView.fRenderState;
    fView.fRenderState |= plPipeline::kRenderBaseLayerOnly;
    static plLayerDepth depth;
    depth.SetMiscFlags(depth.GetMiscFlags() | hsGMatState::kMiscWireFrame | hsGMatState::kMiscTwoSided);
    depth.SetZFlags((depth.GetZFlags() & ~hsGMatState::kZNoZRead) | hsGMatState::kZIncLayer);

    AppendLayerInterface(&depth, false);

    if( IHandleMaterial( material, 0, &span ) >= 0 )
    {
        ISetShaders(nullptr, nullptr);
        render.RenderPrims();
    }

    RemoveLayerInterface(&depth, false) ;
    
    fView.fRenderState = state;
}

// IRenderProjectionEach ///////////////////////////////////////////////////////////////////////////////////////
// Render any lights that are to be projected onto each pass of the object.
void plDXPipeline::IRenderProjectionEach(const plRenderPrimFunc& render, hsGMaterial* material, int iPass, const plSpan& span)
{
    // If this is a bump map pass, forget it, we've already "done" per-pixel lighting.
    if( fLayerState[iPass].fMiscFlags & (hsGMatState::kMiscBumpLayer | hsGMatState::kMiscBumpChans) )
        return;

    // Push the LayerShadowBase override. This sets the blend
    // to framebuffer as Add/ZNoWrite and AmbientColor = 0.
    static plLayerLightBase layLightBase;

    int iNextPass = iPass + fCurrNumLayers;

    // For each projector:
    for (plLightInfo* li : fLights.fProjEach)
    {
        // Push it's projected texture as a piggyback.

        // Lower end boards are iffy on when they'll project correctly.
        if( fSettings.fCantProj && !li->GetProperty(plLightInfo::kLPForceProj) )
            continue;

        plLayerInterface* proj = li->GetProjection();
        hsAssert(proj, "A projector with no texture to project?");
        IPushProjPiggyBack(proj);

        // Enable the projecting light only.
        plDXLightRef* ref = (plDXLightRef *)li->GetDeviceRef();
        fD3DDevice->LightEnable( ref->fD3DIndex, true );

        AppendLayerInterface(&layLightBase, false);

        // Render until it's done.
        int iRePass = iPass;
        while( iRePass < iNextPass )
        {
            iRePass = IHandleMaterial( material, iRePass, &span );
            ISetShaders(nullptr, nullptr);

            // Do the render with projection.
            render.RenderPrims();
        }

        RemoveLayerInterface(&layLightBase, false);

        // Disable the projecting light
        fD3DDevice->LightEnable(ref->fD3DIndex, false);

        // Pop it's projected texture off piggyback
        IPopProjPiggyBacks();

    }

}

// IRenderProjections ///////////////////////////////////////////////////////////
// Render any projected lights that want to be rendered a single time after
// all passes on the object are complete.
void plDXPipeline::IRenderProjections(const plRenderPrimFunc& render)
{
    IDisableSpanLights();
    for (plLightInfo* li : fLights.fProjAll)
    {
        if( fSettings.fCantProj && !li->GetProperty(plLightInfo::kLPForceProj) )
            continue;
        
        IRenderProjection(render, li);
    }
    IRestoreSpanLights();
}

// IRenderProjection //////////////////////////////////////////////////////////////
// Render this light's projection onto the frame buffer.
void plDXPipeline::IRenderProjection(const plRenderPrimFunc& render, plLightInfo* li)
{
    plDXLightRef* ref = (plDXLightRef *)li->GetDeviceRef();
    fD3DDevice->LightEnable(ref->fD3DIndex, true);

    plLayerInterface* proj = li->GetProjection();

    static D3DMATERIAL9 mat;
    mat.Diffuse.r = mat.Diffuse.g = mat.Diffuse.b = mat.Diffuse.a = 1.f;

    fD3DDevice->SetMaterial( &mat );
    fD3DDevice->SetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL );
    fD3DDevice->SetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL );
    fD3DDevice->SetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL );

    fD3DDevice->SetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL );

    fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0xffffffff ); //@@@

    // Set the FB blend mode, texture, all that.
    ICompositeLayerState(0, proj);
    // We should have put ZNoZWrite on during export, but we didn't.
    fLayerState[0].fZFlags = hsGMatState::kZNoZWrite;
    fCurrNumLayers = 1;
    IHandleFirstTextureStage(proj);

    if( proj->GetBlendFlags() & hsGMatState::kBlendInvertFinalColor )
    {
        fD3DDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE | D3DTA_COMPLEMENT);
    }

    // Seal it up
    fLastEndingStage = 1;
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    fLayerState[1].fBlendFlags = uint32_t(-1);

#ifdef HS_DEBUGGING
    DWORD nPass;
    fSettings.fDXError = fD3DDevice->ValidateDevice(&nPass);
    if( fSettings.fDXError != D3D_OK )
        IGetD3DError();
#endif // HS_DEBUGGING

    // Okay, render it already.

    render.RenderPrims();

    fD3DDevice->LightEnable(ref->fD3DIndex, false);
}

//// IGetBufferD3DFormat //////////////////////////////////////////////////////
// Convert the dumbest vertex format on the planet (ours) into an FVF code.
// Note the assumption of position, normal, diffuse, and specular.
// We no longer use FVF codes, just shader handles.
long    plDXPipeline::IGetBufferD3DFormat( uint8_t format ) const
{
    long    fmt, i;
    
    
    switch( format & plGBufferGroup::kSkinWeightMask )
    {
        case plGBufferGroup::kSkinNoWeights: 
            fmt = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL; 
            break;
        case plGBufferGroup::kSkin1Weight: 
            fmt = D3DFVF_XYZB1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL; 
            break;
        case plGBufferGroup::kSkin2Weights: 
            fmt = D3DFVF_XYZB2 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL; 
            break;
        case plGBufferGroup::kSkin3Weights: 
            fmt = D3DFVF_XYZB3 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL; 
            break;
        default:
            hsAssert( false, "Bad skin weight value in IGetBufferD3DFormat()" );
    }
    if( format & plGBufferGroup::kSkinIndices )
    {
        hsAssert(false, "Indexed skinning not supported");
        fmt |= D3DFVF_LASTBETA_UBYTE4;
    }

    switch( plGBufferGroup::CalcNumUVs( format ) )
    {
        case 0: fmt |= D3DFVF_TEX0; break;
        case 1: fmt |= D3DFVF_TEX1; break;
        case 2: fmt |= D3DFVF_TEX2; break;
        case 3: fmt |= D3DFVF_TEX3; break;
        case 4: fmt |= D3DFVF_TEX4; break;
        case 5: fmt |= D3DFVF_TEX5; break;
        case 6: fmt |= D3DFVF_TEX6; break;
        case 7: fmt |= D3DFVF_TEX7; break;
        case 8: fmt |= D3DFVF_TEX8; break;
    }

    for( i = 0; i < plGBufferGroup::CalcNumUVs( format ); i++ )
        fmt |= D3DFVF_TEXCOORDSIZE3( i );

    return fmt;
}

//// IGetBufferFormatSize /////////////////////////////////////////////////////
// Calculate the vertex stride from the given format.
uint32_t  plDXPipeline::IGetBufferFormatSize( uint8_t format ) const
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

///////////////////////////////////////////////////////////////////////////////
//// Plate and PlateManager Functions /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// None of this plate code is mine, so your guess is as good as mine. 
// I'll throw in comments where I happen to know what it's doing, but a lot
// of this is just ugly.
// The plates are mostly used for debugging/performance tools, but they do
// unfortunately get used for some production things like the cursor.
// By the way, a Plate is just a screen aligned textured quad that is rendered
// on top of the normal scene. mf

// ICreateGeometry /////////////////////////////////////////////////////////
// Make a quad suitable for rendering as a tristrip.
void plDXPlateManager::ICreateGeometry(plDXPipeline* pipe)
{
    uint32_t fvfFormat = PLD3D_PLATEFVF;
    D3DPOOL poolType = D3DPOOL_DEFAULT;
    hsAssert(!pipe->ManagedAlloced(), "Alloc default with managed alloc'd");
    if( FAILED( fD3DDevice->CreateVertexBuffer( 4 * sizeof( plPlateVertex ),
                                                D3DUSAGE_WRITEONLY, 
                                                fvfFormat,
                                                poolType, &fVertBuffer, nullptr)))
    {
        hsAssert( false, "CreateVertexBuffer() call failed!" );
        fCreatedSucessfully = false;
        return;
    }
    PROFILE_POOL_MEM(poolType, 4 * sizeof(plPlateVertex), true, "PlateMgrVtxBuff");

    /// Lock the buffer
    plPlateVertex *ptr;
    if( FAILED( fVertBuffer->Lock( 0, 0, (void **)&ptr, D3DLOCK_NOSYSLOCK ) ) )
    {
        hsAssert( false, "Failed to lock vertex buffer for writing" );
        fCreatedSucessfully = false;
        return;
    }
    
    /// Set 'em up
    ptr[ 0 ].fPoint.Set( -0.5f, -0.5f, 0.0f );
    ptr[ 0 ].fColor = 0xffffffff;
    ptr[ 0 ].fUV.Set( 0.0f, 0.0f, 0.0f );

    ptr[ 1 ].fPoint.Set( -0.5f, 0.5f, 0.0f );
    ptr[ 1 ].fColor = 0xffffffff;
    ptr[ 1 ].fUV.Set( 0.0f, 1.0f, 0.0f );

    ptr[ 2 ].fPoint.Set( 0.5f, -0.5f, 0.0f );
    ptr[ 2 ].fColor = 0xffffffff;
    ptr[ 2 ].fUV.Set( 1.0f, 0.0f, 0.0f );

    ptr[ 3 ].fPoint.Set( 0.5f, 0.5f, 0.0f );
    ptr[ 3 ].fColor = 0xffffffff;
    ptr[ 3 ].fUV.Set( 1.0f, 1.0f, 0.0f );

    /// Unlock and we're done!
    fVertBuffer->Unlock();
    fCreatedSucessfully = true;

}

// IReleaseGeometry ////////////////////////////////////////////////////////////
// Let go of any D3D resources created for this.
void plDXPlateManager::IReleaseGeometry()
{
    if (fVertBuffer)
    {
        ReleaseObject(fVertBuffer);
        PROFILE_POOL_MEM(D3DPOOL_DEFAULT, 4 * sizeof(plPlateVertex), false, "PlateMgrVtxBuff");
        fVertBuffer = nullptr;
    }
}

//// Constructor & Destructor /////////////////////////////////////////////////

plDXPlateManager::plDXPlateManager( plDXPipeline *pipe, IDirect3DDevice9 *device ) : plPlateManager( pipe ),
            PLD3D_PLATEFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE3(0) ),
            fD3DDevice(device),
            fVertBuffer()
{   
}

plDXPlateManager::~plDXPlateManager()
{
    IReleaseGeometry();
}

//// IDrawPlate ///////////////////////////////////////////////////////////////
// Render all currently enabled plates to the screen.
void    plDXPlateManager::IDrawToDevice( plPipeline *pipe )
{
    plDXPipeline    *dxPipe = (plDXPipeline *)pipe;
    plPlate         *plate;
    uint32_t        scrnWidthDiv2 = fOwner->Width() >> 1;
    uint32_t        scrnHeightDiv2 = fOwner->Height() >> 1;
    D3DCULL         oldCullMode;
    
    if( !fVertBuffer )
        return;
    
    // Make sure skinning is disabled.
    fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    fD3DDevice->SetVertexShader(dxPipe->fSettings.fCurrVertexShader = nullptr);
    fD3DDevice->SetFVF(dxPipe->fSettings.fCurrFVFFormat = PLD3D_PLATEFVF);
    fD3DDevice->SetStreamSource( 0, fVertBuffer, 0, sizeof( plPlateVertex ) );  
    plProfile_Inc(VertexChange);

    // To get plates properly pixel-aligned, we need to compensate for D3D9's weird half-pixel
    // offset (see http://drilian.com/2008/11/25/understanding-half-pixel-and-half-texel-offsets/
    // or http://msdn.microsoft.com/en-us/library/bb219690(VS.85).aspx).
    auto viewMat = DirectX::XMMatrixTranslation(-0.5f/scrnWidthDiv2, -0.5f/scrnHeightDiv2, 0.0f);
    fD3DDevice->SetTransform( D3DTS_VIEW, (D3DMATRIX*)&viewMat );
    oldCullMode = dxPipe->fDevice.fCurrCullMode;

    for (plate = fPlates; plate != nullptr; plate = plate->GetNext())
    {
        if( plate->IsVisible() )
        {
            dxPipe->IDrawPlate( plate );

            const char *title = plate->GetTitle();
            if( plDebugText::Instance().IsEnabled() && title[ 0 ] != 0 )
            {
                hsPoint3 pt;
                pt.Set( 0, -0.5, 0 );
                pt = plate->GetTransform() * pt;
                pt.fX = pt.fX * scrnWidthDiv2 + scrnWidthDiv2;
                pt.fY = pt.fY * scrnHeightDiv2 + scrnHeightDiv2;
                pt.fX -= plDebugText::Instance().CalcStringWidth( title ) >> 1;
                plDebugText::Instance().DrawString( (uint16_t)pt.fX, (uint16_t)pt.fY + 1, title, 255, 255, 255, 255, plDebugText::kStyleBold ); 
            }

            if( plate->GetFlags() & plPlate::kFlagIsAGraph )
            {
                plGraphPlate    *graph = (plGraphPlate *)plate;
                hsPoint3        pt, pt2;
                int             i;

                if( graph->GetLabelText( 0 )[ 0 ] != 0 )
                {
                    /// Draw key
                    const char *str;

                    pt.Set( -0.5, -0.5, 0 );
                    pt = plate->GetTransform() * pt;
                    pt.fX = pt.fX * scrnWidthDiv2 + scrnWidthDiv2;
                    pt.fY = pt.fY * scrnHeightDiv2 + scrnHeightDiv2;
                    pt.fY += plDebugText::Instance().GetFontHeight();

                    uint32_t numLabels = graph->GetNumLabels();
                    if (numLabels > graph->GetNumColors())
                        numLabels = graph->GetNumColors();

                    for( i = 0; i < numLabels; i++ )
                    {
                        str = graph->GetLabelText( i );
                        if( str[ 0 ] == 0 )
                            break;

                        pt2 = pt;
                        pt2.fX -= plDebugText::Instance().CalcStringWidth( str );
                        plDebugText::Instance().DrawString( (uint16_t)pt2.fX, (uint16_t)pt2.fY, str, 
                                                            graph->GetDataColor( i ), plDebugText::kStyleBold ); 
                        pt.fY += plDebugText::Instance().GetFontHeight();
                    }
                }
            }
        }
    }

    dxPipe->fDevice.fCurrCullMode = ( dxPipe->fLayerState[0].fMiscFlags & hsGMatState::kMiscTwoSided ) ? D3DCULL_NONE : oldCullMode;
    fD3DDevice->SetRenderState( D3DRS_CULLMODE, dxPipe->fDevice.fCurrCullMode );
}

// IDrawPlate ///////////////////////////////////////////////////////////////////////
// Render this plate, in as many passes as it takes.
void    plDXPipeline::IDrawPlate( plPlate *plate )
{
    int         i;
    hsGMaterial *material = plate->GetMaterial();
    D3DMATRIX  mat;


    /// Set up the D3D transform directly
    IMatrix44ToD3DMatrix( mat, plate->GetTransform() );
    fD3DDevice->SetTransform( D3DTS_WORLD, &mat );
    mat = d3dIdentityMatrix;
    mat.m[1][1] = -1.0f;
    mat.m[2][2] = 2.0f;
    mat.m[2][3] = 1.0f;
    mat.m[3][2] = -2.0f;
    mat.m[3][3] = 0.0f;

    IPushPiggyBacks(material);

    /// Draw the vertex buffer once for each material pass
    for( i = 0; i < material->GetNumLayers(); )
    {
        // Stat gather adjust: since IHandleMaterial will count this in the stat gather,
        // artificially decrement here so that the plates don't skew the stat gathering
        // Taking this out. If the plates are causing more material changes, they should
        // show up in the stats. mf


        i = IHandleMaterial(material, i, nullptr);
        ISetShaders(nullptr, nullptr);

        // To override the transform done by the z-bias
        fD3DDevice->SetTransform( D3DTS_PROJECTION, &mat );
        // And this to override cullmode set based on material 2-sidedness.
        fD3DDevice->SetRenderState( D3DRS_CULLMODE, fDevice.fCurrCullMode = D3DCULL_CW );

        WEAK_ERROR_CHECK( fD3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 ) );
    }

    IPopPiggyBacks();
}


///////////////////////////////////////////////////////////////////////////////
//// Error Message Stuff //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// IAddErrorMessage ////////////////////////////////////////////////////
// Append the error string to the current error string.
void    plDXPipeline::IAddErrorMessage( char *errStr )
{
    static char str[ 512 ];
    if( errStr && strlen( errStr ) + strlen( fSettings.fErrorStr ) < sizeof( fSettings.fErrorStr ) - 4 )
    {
        strcpy( str, fSettings.fErrorStr );
        sprintf( fSettings.fErrorStr, "%s\n(%s)", errStr, str );
        plStatusLog::AddLineS("pipeline.log", fSettings.fErrorStr);
    }
}

// ISetErrorMessage //////////////////////////////////////////////////////////
// Clear the current error string to the input string.
void    plDXPipeline::ISetErrorMessage( char *errStr )
{
    if( errStr )
    {
        strcpy( fSettings.fErrorStr, errStr );
        plStatusLog::AddLineS("pipeline.log", fSettings.fErrorStr);
    }
    else
        fSettings.fErrorStr[ 0 ] = 0;
}

// IGetD3DError /////////////////////////////////////////////////////////////////
// Convert the last D3D error code to a string (probably "Conflicting Render State").
void    plDXPipeline::IGetD3DError()
{
    sprintf( fSettings.fErrorStr, "D3DError : %s", fSettings.fDXError.ToString().c_str() );
}

// IShowErrorMessage /////////////////////////////////////////////////////////////
// Append the string to the running error string.
void    plDXPipeline::IShowErrorMessage( char *errStr )
{
    if (errStr != nullptr)
        IAddErrorMessage( errStr );

//  hsAssert( false, fSettings.fErrorStr );
}

// ICreateFail ////////////////////////////////////////////////////////////////////
// Called on unrecoverable error during device creation. Frees up anything 
// allocated so far, sets the error string, and returns true.
bool  plDXPipeline::ICreateFail( char *errStr )
{
    // Don't overwrite any error string we already had
    if( fSettings.fErrorStr[ 0 ] == 0 )
        IGetD3DError();

    if( errStr && *errStr )
    {
        IAddErrorMessage( errStr );
    }
    else if( !*fSettings.fErrorStr )
        IAddErrorMessage( "unknown" );

    IReleaseDeviceObjects();
    return true;
}

// GetErrorString ///////////////////////////////////////////////////////////////////////////
// Return the current error string.
const char  *plDXPipeline::GetErrorString()
{
    if( fSettings.fErrorStr[ 0 ] == 0 )
        return nullptr;

    return fSettings.fErrorStr;
}


///////////////////////////////////////////////////////////////////////////////
//// Miscellaneous Utility Functions //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// GetDXBitDepth //////////////////////////////////////////////////////////
//
//  From a D3DFORMAT enumeration, return the bit depth associated with it.

short   plDXPipeline::GetDXBitDepth( D3DFORMAT format )
{
    if( format == D3DFMT_UNKNOWN )
        return 0;
    else if( format == D3DFMT_R8G8B8 )
        return 24;
    else if( format == D3DFMT_A8R8G8B8 )
        return 32;
    else if( format == D3DFMT_X8R8G8B8 )
        return 32;
    else if( format == D3DFMT_R5G6B5 )
        return 16;
    else if( format == D3DFMT_X1R5G5B5 )
        return 16;
    else if( format == D3DFMT_A1R5G5B5 )
        return 16;
    else if( format == D3DFMT_A4R4G4B4 )
        return 16;
    else if( format == D3DFMT_R3G3B2 )
        return 8;
    else if( format == D3DFMT_A8 )
        return 8;
    else if( format == D3DFMT_A8R3G3B2 )
        return 16;
    else if( format == D3DFMT_X4R4G4B4 )
        return 16;
    else if( format == D3DFMT_A8P8 )
        return 16;
    else if( format == D3DFMT_P8 )
        return 8;
    else if( format == D3DFMT_L8 )
        return 8;
    else if( format == D3DFMT_A8L8 )
        return 16;
    else if( format == D3DFMT_A4L4 )
        return 8;
    else if( format == D3DFMT_V8U8 )
        return 16;
    else if( format == D3DFMT_L6V5U5 )
        return 16;
    else if( format == D3DFMT_X8L8V8U8 )
        return 32;
    else if( format == D3DFMT_Q8W8V8U8 )
        return 32;
    else if( format == D3DFMT_V16U16 )
        return 32;
//  else if( format == D3DFMT_W11V11U10 )
//      return 32;
    /* /// These formats really don't have bit depths associated with them
    D3DFMT_UYVY
    D3DFMT_YUY2     
    D3DFMT_DXT1     
    D3DFMT_DXT2     
    D3DFMT_DXT3     
    D3DFMT_DXT4     
    D3DFMT_DXT5     
    D3DFMT_VERTEXDATA  
    */
    else if( format == D3DFMT_D16_LOCKABLE )
        return 16;
    else if( format == D3DFMT_D32 )
        return 32;
    else if( format == D3DFMT_D15S1 )
        return 16;
    else if( format == D3DFMT_D24S8 )
        return 32;
    else if( format == D3DFMT_D16 )
        return 16;
    else if( format == D3DFMT_D24X8 )
        return 32;
    else if( format == D3DFMT_D24X4S4 )
        return 32;
    else if( format == D3DFMT_INDEX16 )
        return 16;
    else if( format == D3DFMT_INDEX32 )
        return 32;
    
    // Unsupported translation format--return 0
    return 0;
}

//// IGetDXFormatName ////////////////////////////////////////////////////////
//
//  From a D3DFORMAT enumeration, return the string for it.

const char  *plDXPipeline::IGetDXFormatName( D3DFORMAT format )
{
    switch( format )
    {
        case D3DFMT_UNKNOWN: return "D3DFMT_UNKNOWN"; 
        case D3DFMT_R8G8B8: return "D3DFMT_R8G8B8";
        case D3DFMT_A8R8G8B8: return "D3DFMT_A8R8G8B8";
        case D3DFMT_X8R8G8B8: return "D3DFMT_X8R8G8B8";
        case D3DFMT_R5G6B5: return "D3DFMT_R5G6B5";
        case D3DFMT_X1R5G5B5: return "D3DFMT_X1R5G5B5";
        case D3DFMT_A1R5G5B5: return "D3DFMT_A1R5G5B5";
        case D3DFMT_A4R4G4B4: return "D3DFMT_A4R4G4B4";
        case D3DFMT_R3G3B2: return "D3DFMT_R3G3B2";
        case D3DFMT_A8: return "D3DFMT_A8";
        case D3DFMT_A8R3G3B2: return "D3DFMT_A8R3G3B2";
        case D3DFMT_X4R4G4B4: return "D3DFMT_X4R4G4B4";
        case D3DFMT_A8P8: return "D3DFMT_A8P8";
        case D3DFMT_P8: return "D3DFMT_P8";
        case D3DFMT_L8: return "D3DFMT_L8";
        case D3DFMT_A8L8: return "D3DFMT_A8L8";
        case D3DFMT_A4L4: return "D3DFMT_A4L4";
        case D3DFMT_V8U8: return "D3DFMT_V8U8";
        case D3DFMT_L6V5U5: return "D3DFMT_L6V5U5";
        case D3DFMT_X8L8V8U8: return "D3DFMT_X8L8V8U8";
        case D3DFMT_Q8W8V8U8: return "D3DFMT_Q8W8V8U8";
        case D3DFMT_V16U16: return "D3DFMT_V16U16";
        //case D3DFMT_W11V11U10: return "D3DFMT_W11V11U10";
        case D3DFMT_UYVY: return "D3DFMT_UYVY";
        case D3DFMT_YUY2: return "D3DFMT_YUY2";
        case D3DFMT_DXT1: return "D3DFMT_DXT1";    
//      case D3DFMT_DXT2: return "D3DFMT_DXT2";    
//      case D3DFMT_DXT3: return "D3DFMT_DXT3";    
//      case D3DFMT_DXT4: return "D3DFMT_DXT4";    
        case D3DFMT_DXT5: return "D3DFMT_DXT5";
        case D3DFMT_VERTEXDATA: return "D3DFMT_VERTEXDATA";
        case D3DFMT_D16_LOCKABLE: return "D3DFMT_D16_LOCKABLE";
        case D3DFMT_D32: return "D3DFMT_D32";
        case D3DFMT_D15S1: return "D3DFMT_D15S1";
        case D3DFMT_D24S8: return "D3DFMT_D24S8";
        case D3DFMT_D16: return "D3DFMT_D16";
        case D3DFMT_D24X8: return "D3DFMT_D24X8";
        case D3DFMT_D24X4S4: return "D3DFMT_D24X4S4";
        case D3DFMT_INDEX16: return "D3DFMT_INDEX16";
        case D3DFMT_INDEX32: return "D3DFMT_INDEX32";
        default: return "Bad format";   
    }
}

///////////////////////////////////////////////////////////////////////////////
//// ShadowSection
//// Shadow specific internal functions
///////////////////////////////////////////////////////////////////////////////
// See plGLight/plShadowMaster.cpp for more notes.



float blurScale = -1.f;
static  const int kL2NumSamples = 3; // Log2(4)

// IBlurShadowMap //////////////////////////////////////////////////////////////////
// For a shadow map, we've got a specific (non-general) blurring in mind.
// This could be used as a loose model for more general blurring, but you
// wouldn't want to run a generic texture or render target through this.
// Specifically, we assume:
//  Input: 
//      An RGBA rendertarget with an alpha we want to preserve, and color
//          going from black (unused) to white (written).
//      A blur factor
//  Output:
//      The rendertarget with alpha preserved, and the color channel blurred
//          appropriately.
//  We'll want to minimize our render target changes, so
//      we clear our scratch render target to black/white (color/alpha), then
//      render additively the color of our input with a zero alpha. The scratch
//      accumulates the color sum, but the alpha starts and stays saturated to 100%.
//      Then we modulate that back into the input, so the alpha is unchanged, the
//      color (within the white region) falls off at the edges. The color outside the
//      white region is black and stays black, but we don't care because we'll be ignoring
//      that anyway.
//  Notice that this depends on the input, each pixel having been all black or all "white".
// Also depends on "white" having 1/N premodulated in, where N is the number of samples.
//      That's why we can just sum up the colors, without needing to do a divide. Otherwise
//      we'd saturate at 255 during the sum, and the divide would be pointless.
// One other thing we're counting on here, is that we've just been rendering to an
//      offscreen, we're done, and we're about to pop our rendertarget, which is going
//      to reset a lot of render state that we would otherwise be responsible for here.
// We're hoping that this blur function (if efficient enough) can get called enough times
//      per frame to warrant the sins described above.
void plDXPipeline::IBlurShadowMap(plShadowSlave* slave)
{
    plRenderTarget* smap = (plRenderTarget*)slave->fPipeData;
    float scale = slave->fBlurScale;

    // Find a scratch rendertarget which matches the input.
    int which = IGetScratchRenderTarget(smap);
    plRenderTarget* scratchRT = fBlurScratchRTs[which];
    if( !scratchRT )
        return;
    plRenderTarget* destRT = fBlurDestRTs[which];
    if( !destRT )
        return;

    // Set up to render into it.
    IBlurSetRenderTarget(scratchRT);

    // Clear it appropriately
    fD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000L, 1.0f, 0L);

    // Setup our quad for rendering
    ISetBlurQuadToRender(smap);

    // Render the input image into the scratch image, creating the blur.
    IRenderBlurFromShadowMap(scratchRT, smap, scale);

    // Set the rendertarget back to src
    // Setup renderstate to render it back modulating.
    // Render the scratch back into the input.
    IRenderBlurBackToShadowMap(smap, scratchRT, destRT);

    // dst is now now slave's rendertarget and smap is the new scratch dst
    // for this size.
    slave->fPipeData = (void*)destRT;
    fBlurDestRTs[which] = smap;
}

// IGetScratchRenderTarget ////////////////////////////////////////////
// Look for a render target for as scratch space for blurring the input render target.
// Note that the whole blur process requires 3 render targets, the source,
// an intermediate, and the destination (which gets swapped with the source).
// But that's only an extra 2 render targets for all shadow maps of a given
// size.
// Note also that the intermediate is one size smaller than the source,
// to get better blurring through bilerp magnification.
int plDXPipeline::IGetScratchRenderTarget(plRenderTarget* smap)
{
    int which = -1;
    switch(smap->GetHeight())
    {
    case 512:
        which = 9;
        break;
    case 256:
        which = 8;
        break;
    case 128:
        which = 7;
        break;
    case 64:
        which = 6;
        break;
    case 32:
        which = 5;
        break;
    default:
        return false;
    }
    if( !fBlurScratchRTs[which] )
    {
        // We may or may not get back the size we requested here, but if we didn't,
        // we aren't going to later, so we might as well stuff the smaller render target
        // into the bigger slot. Bad thing is that we might want a smaller render target
        // later, and we won't know to look in the bigger slot for it, so we could wind
        // up using say two 128x128's (one in the 256 slot, one in the 128 slot). 
        // This intermediate is one power of 2 smaller than the source.
        uint32_t width = smap->GetWidth();
        uint32_t height = smap->GetHeight();
        if( width > 32 )
        {
            width >>= 1;
            height >>= 1;
        }
        fBlurScratchRTs[which] = IFindRenderTarget(width, height, smap->GetFlags() & plRenderTarget::kIsOrtho);
    }
    if( !fBlurDestRTs[which] )
    {
        // Destination is same size as source.
        uint32_t width = smap->GetWidth();
        uint32_t height = smap->GetHeight();
        fBlurDestRTs[which] = IFindRenderTarget(width, height, smap->GetFlags() & plRenderTarget::kIsOrtho);
    }
#ifdef MF_ENABLE_HACKOFF
    if( hackOffscreens.kMissingIndex == hackOffscreens.Find(fBlurScratchRTs[which]) )
        hackOffscreens.Append(fBlurScratchRTs[which]);
    if( hackOffscreens.kMissingIndex == hackOffscreens.Find(fBlurDestRTs[which]) )
        hackOffscreens.Append(fBlurDestRTs[which]);
#endif // MF_ENABLE_HACKOFF
    return which;
}

// IBlurSetRenderTarget /////////////////////////////////////////////////////////////////////
// Set the input render target up to be rendered into. This abbreviated version
// of PushRenderTarget is possible because of the special case of the state coming
// in, and that we know we're going to immediately pop the previous render target
// when we're done.
void plDXPipeline::IBlurSetRenderTarget(plRenderTarget* rt)
{
    plDXRenderTargetRef* ref = (plDXRenderTargetRef *)rt->GetDeviceRef();
    // Set the rendertarget
    IDirect3DSurface9* main = ref->GetColorSurface();
    IDirect3DSurface9* depth = ref->fD3DDepthSurface;

    fDevice.fCurrD3DMainSurface = main;
    fDevice.fCurrD3DDepthSurface = depth;
    fD3DDevice->SetRenderTarget(0, main);
    fD3DDevice->SetDepthStencilSurface(depth);

    // Now set the correct viewport
    D3DVIEWPORT9 vp = { 0,
                        0,
                        rt->GetWidth(),
                        rt->GetHeight(),
                        0.f, 1.f };

    
    WEAK_ERROR_CHECK( fD3DDevice->SetViewport( &vp ) );
}


// IRenderBlurFromShadowMap ////////////////////////////////////////////////////////////////////////////////
// Render a shadow map into a scratch render target multiple times offset slightly to create a blur
// in the color, preserving alpha exactly. It's just rendering a single quad with slight offsets
// in the UVW transform.
void plDXPipeline::IRenderBlurFromShadowMap(plRenderTarget* scratchRT, plRenderTarget* smap, float scale)
{
    // Quad is set up in camera space.
    fD3DDevice->SetTransform(D3DTS_VIEW, &d3dIdentityMatrix);
    fD3DDevice->SetTransform(D3DTS_WORLD, &d3dIdentityMatrix);
    fD3DDevice->SetTransform(D3DTS_PROJECTION, &d3dIdentityMatrix);

    // Figure out how many passes we'll need.
//  const int kNumSamples = 1 << kL2NumSamples; // HACKSAMPLE
    const int kNumSamples = mfCurrentTest > 101 ? 8 : 4;
    int nPasses = (int)ceil(float(kNumSamples) / fMaxLayersAtOnce);
    int nSamplesPerPass = kNumSamples / nPasses;

    // Attenuate by number of passes, to average as we sum.
    DWORD atten = 255 / nPasses;
    plConst(float) kAtten(1.f);
    atten = DWORD(atten * kAtten);
    atten = (atten << 24)
        | (atten << 16)
        | (atten << 8)
        | (atten << 0);

    // Disable skinning
    fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    //
    //  AlphaEnable = true
    //  AlphaTest OFF
    fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
    fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
    fD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);

    //  ZBUFFER disabled
    fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    fLayerState[0].fZFlags &= ~hsGMatState::kZMask;
    fLayerState[0].fZFlags |= hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead;
    //
    //  Cullmode is NONE
    fDevice.fCurrCullMode = D3DCULL_NONE; 
    fD3DDevice->SetRenderState( D3DRS_CULLMODE, fDevice.fCurrCullMode );

    plDXTextureRef* ref = (plDXTextureRef*)smap->GetDeviceRef();
    hsAssert(ref, "Shadow map ref should have been made when it was rendered");
    if( !ref )
        return;

    // TFactor contains the attenuation
    fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, atten);

    // Set the N texture stages all to use the same
    // src rendertarget texture.
    // Blend modes are:
    //  Stage0:
    //      Color
    //      Arg1 = texture
    //      Op = selectArg1
    //      Alpha
    //      Arg1 = TFACTOR = white
    //      Op = selectArg1
    //  Stage[1..n-1]
    //      Color
    //      Arg1 = texture
    //      Arg2 = current
    //      Op = AddSigned
    //      Alpha
    //      Arg1 = texture
    //      Arg2 = current
    //      Op = SelectArg2
    //  StageN
    //      Color/Alpha
    //      Op = disable
    //
    // Frame buffer blend is
    //      SRCBLEND = ONE
    //      DSTBLEND = ONE
    //  All texture stages are clamped
    //
    // Set stage0, then loop over the rest
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fLayerState[0].fClampFlags = hsGMatState::kClampTexture;

    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1);

    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR); 
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);
    fLayerState[0].fBlendFlags = uint32_t(-1);

    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fD3DDevice->SetTexture( 0, ref->fD3DTexture );

    if( D3DTTFF_COUNT2 != fLayerXformFlags[0] )
    {
        fLayerXformFlags[0] = D3DTTFF_COUNT2;
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
    }
    fD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
    fLayerUVWSrcs[0] = 0;

    int i;
    for( i = 1; i < nSamplesPerPass; i++ )
    {
        fD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        fD3DDevice->SetSamplerState(i, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
        fLayerState[i].fClampFlags = hsGMatState::kClampTexture;

        fD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        fD3DDevice->SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
        fD3DDevice->SetTextureStageState(i, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED);

        fD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
        fD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
        fD3DDevice->SetTextureStageState(i, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
        fLayerState[i].fBlendFlags = uint32_t(-1);

        hsRefCnt_SafeAssign( fLayerRef[i], ref );
        fD3DDevice->SetTexture( i, ref->fD3DTexture );

        if( D3DTTFF_COUNT2 != fLayerXformFlags[i] )
        {
            fLayerXformFlags[i] = D3DTTFF_COUNT2;
            fD3DDevice->SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT2);
        }
        fD3DDevice->SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, 0);
        fLayerUVWSrcs[i] = 0;
    }
    fD3DDevice->SetTextureStageState(nSamplesPerPass, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(nSamplesPerPass, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    // N offsets are { (-1,-1), (1, -1), (1, 1), (-1, 1) } * offsetScale / size, with
    // useful offsetScales probably going from 0.5 to 1.5, but we'll just have
    // to experiment and see. Larger values likely to require more than the current
    // 4 samples
    struct offsetStruct
    {
        float   fU;
        float   fV;
    };
    offsetStruct offsetScale = { scale / scratchRT->GetWidth(), scale / scratchRT->GetHeight() };
    static offsetStruct offsets[8] = {
        {-1.f,  -1.f},
        {1.f,   -1.f},
        {1.f,   1.f},
        {-1.f,  1.f},
        {0.f,   -0.5f},
        {0.f,   0.5f},
        {-0.5f, 0.f},
        {0.5f,  0.f}
    };

    int iSample = 0;
    // For each pass, 
    for( i = 0; i < nPasses; i++ )
    {
        // Set the N texture stage uv transforms to the
        // next N offsets.
        int j;
        for( j = 0; j < nSamplesPerPass; j++ )
        {
            D3DMATRIX offXfm = d3dIdentityMatrix;
            offXfm.m[2][0] = offsets[iSample].fU * offsetScale.fU;
            offXfm.m[2][1] = offsets[iSample].fV * offsetScale.fV;
            fD3DDevice->SetTransform(sTextureStages[j], &offXfm);
            fLayerTransform[j] = true;
            
            iSample++;
        }

        // Render our quad
        fD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

//      fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0L);
//      fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//      fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TFACTOR);
//      fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED);
//      fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);

    }
}

// IRenderBlurBackToShadowMap /////////////////////////////////////////////////////////////////////
// Render our intermediate blurred map back into a useable shadow map.
void plDXPipeline::IRenderBlurBackToShadowMap(plRenderTarget* smap, plRenderTarget* scratch, plRenderTarget* dst)
{
    // Set the rendertarget
    IBlurSetRenderTarget(dst);

    // Clear it appropriately. This might not be necessary, since we're just going to overwrite.
    fD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET, 0xff000000L, 1.0f, 0L);

    // Scratch has an all white alpha, and the blurred color from smap. But the color
    // is a signed biased color. We need to remap [128..255] from scratch into [0..255]
    // on dst. Plus, we need to copy the alpha as is from smap into dst.
    // So, scratch is texture0, smap is texture1. TFACTOR is 0.
    // Color is ADDSIGNED2X(TFACTOR, texture0).
    // Alpha is SELECTARG1(texture1, current).
    // Then FB blend is just opaque copy.

    // Set Stage0 texture transform
    // Clamp still on (from RBFSM)
    D3DMATRIX offXfm = d3dIdentityMatrix;
    fD3DDevice->SetTransform(sTextureStages[0], &offXfm);
    fD3DDevice->SetTransform(sTextureStages[1], &offXfm);
    fLayerTransform[0] = false;
    fLayerTransform[1] = false;

    plDXTextureRef* ref = (plDXTextureRef*)scratch->GetDeviceRef();
    hsAssert(ref, "Blur scratch map ref should have been made when it was rendered");
    if( !ref )
        return;
    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fD3DDevice->SetTexture( 0, ref->fD3DTexture );

    ref = (plDXTextureRef*)smap->GetDeviceRef();
    hsAssert(ref, "Blur src map ref should have been made when it was rendered");
    if( !ref )
        return;
    hsRefCnt_SafeAssign( fLayerRef[1], ref );
    fD3DDevice->SetTexture( 1, ref->fD3DTexture );

    // Stage0:
    //      Color
    //      Arg1 = TFACTOR = black
    //      Arg2 = texture
    //      Op = ADDSIGNED2X
    //      Alpha
    //      Arg1 = texture
    //      Op = selectArg1
    //  Texture = scratch
    // Stage1:
    //      Color
    //      Arg1 = texture
    //      Arg2 = current
    //      Op = selectArg2
    //      Alpha
    //      Arg1 = texture
    //      Op = selectArg1
    //  Texture = smap
    // FB blend 
    //      SRCBLEND = ONE
    //      DSTBLEND = ZERO

    fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0L);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_ADDSIGNED2X);

    // This alpha will be ignored, because in the next stage we select texture alpha.
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

    fLayerState[0].fBlendFlags = uint32_t(-1);

    fD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_SELECTARG2);

    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

    fLayerState[1].fBlendFlags = uint32_t(-1);

    fD3DDevice->SetTextureStageState(2, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(2, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

    fLastEndingStage = 2;

    fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

    // Our quad should still be setup to go.
    fD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

}

struct plShadowVertStruct
{
    float       fPos[3];
    float       fUv[2];
};

// IReleaseBlurVBuffers //////////////////////////////////////////////////////////
// Free up our blur quad vertex buffers. Note these are in POOL_DEFAULT
void plDXPipeline::IReleaseBlurVBuffers()
{
    const uint32_t kVSize = sizeof(plShadowVertStruct);
    int i;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        if (fBlurVBuffers[i])
        {
            ReleaseObject(fBlurVBuffers[i]);
            PROFILE_POOL_MEM(D3DPOOL_DEFAULT, 4 * kVSize, false, "BlurVtxBuff");
            fBlurVBuffers[i] = nullptr;
        }
    }
}

// ICreateBlurVBuffers //////////////////////////////////////////////////////////////////
// We need a quad for each size of shadow map, because there's a slight dependency
// of UVW coordinates on size of render target. Sucks but it's true.
bool plDXPipeline::ICreateBlurVBuffers()
{
    // vertex size is 4 verts, with 4 floats each for position, and 2 floats each for uv.
    const uint32_t kVSize = sizeof(plShadowVertStruct);
    const uint32_t kVFormat = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) ;

    int i;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        int width = 0;
        int height = 0;
        int which = -1;
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
            width = height = 1 << i;
            which = i;
            break;
        case 6:
            width = height = 1 << i;
            which = i;
            break;
        case 7:
            width = height = 1 << i;
            which = i;
            break;
        case 8:
            width = height = 1 << i;
            which = i;
            break;
        case 9:
            width = height = 1 << i;
            which = i;
            break;
        }
        if( which < 0 )
            continue;

        // positions are { (-0.5,-0.5,0,1), (w-0.5,-0.5,0,1), (w-0.5,h-0.5,0,1), (-0.5,h-0.5,0,1) }
        // UVs are { (0,0), (1,0), (1,1), (0,1) }
        // So we won't have to bother with indices, we'll put them in as
        // p1, p2, p0, p3 and render tristrip


        // Create the buffer.
        IDirect3DVertexBuffer9* vBuffer = nullptr;

        uint32_t fvfFormat = kVFormat;
        hsAssert(!ManagedAlloced(), "Alloc default with managed alloc'd");
        if( FAILED( fD3DDevice->CreateVertexBuffer( 4 * kVSize,
                                                    D3DUSAGE_WRITEONLY, 
                                                    fvfFormat,
                                                    D3DPOOL_DEFAULT, 
                                                    &vBuffer, nullptr)))
        {
            hsAssert( false, "CreateVertexBuffer() call failed!" );
            return false;
        }
        plShadowVertStruct* ptr = nullptr;

        /// Lock the buffer and fill it in.
        if( FAILED( vBuffer->Lock( 0, 0, (void **)&ptr, 0 ) ) )
        {
            hsAssert( false, "Failed to lock vertex buffer for writing" );
            vBuffer->Release();
            return false;
        }
        PROFILE_POOL_MEM(D3DPOOL_DEFAULT, 4 * kVSize, true, "BlurVtxBuff");

        plShadowVertStruct vert;
        vert.fPos[0] = -1.f;
        vert.fPos[1] = -1.f;
        vert.fPos[2] = 0.5f;

        vert.fUv[0] = 0.5f / width;
        vert.fUv[1] = 1.f + 0.5f / height;

        // P0
        ptr[2] = vert;

        // P1
        ptr[0] = vert;
        ptr[0].fPos[0] += 2.f;
        ptr[0].fUv[0] += 1.f;

        // P2
        ptr[1] = vert;
        ptr[1].fPos[0] += 2.f;
        ptr[1].fUv[0] += 1.f;
        ptr[1].fPos[1] += 2.f;
        ptr[1].fUv[1] -= 1.f;

        // P3
        ptr[3] = vert;
        ptr[3].fPos[1] += 2.f;
        ptr[3].fUv[1] -= 1.f;

        vBuffer->Unlock();

        fBlurVBuffers[which] = vBuffer;
    }
    return true;
}

// ISetBlurQuadToRender ////////////////////////////////////////////////////
// Select the appropriate blur quad (based on size of shadow map) and set it up to render.
bool plDXPipeline::ISetBlurQuadToRender(plRenderTarget* smap)
{
    const uint32_t kVSize = sizeof(plShadowVertStruct);
    const uint32_t kVFormat = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0) ;

    // Each vb will be rendertarget size specific, so select one based on input rendertarget
    int which = -1;
    switch(smap->GetHeight())
    {
    case 512:
        which = 9;
        break;
    case 256:
        which = 8;
        break;
    case 128:
        which = 7;
        break;
    case 64:
        which = 6;
        break;
    case 32:
        which = 5;
        break;
    default:
        return false;
    }

    // If we haven't created (or have lost) our d3d resources, make them
    IDirect3DVertexBuffer9* vBuffer = fBlurVBuffers[which];
    if( !vBuffer )
    {
        ICreateBlurVBuffers();
        vBuffer = fBlurVBuffers[which];
        hsAssert(vBuffer, "AllocBlurVBuffers failed");
    }

    HRESULT r = fD3DDevice->SetVertexShader(fSettings.fCurrVertexShader = nullptr);
    fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = kVFormat);
    hsAssert( r == D3D_OK, "Error trying to set the vertex shader!" );

    hsRefCnt_SafeUnRef(fSettings.fCurrVertexBuffRef);
    fSettings.fCurrVertexBuffRef = nullptr;

    r = fD3DDevice->SetStreamSource(0, vBuffer, 0, kVSize);
    plProfile_Inc(VertexChange);

    // No SetIndices, we'll do a direct DrawPrimitive (not DrawIndexedPrimitive)

    // No transforms, we're supplying screen ready verts.

    return true;
}

// IRenderShadowCasterSpan //////////////////////////////////////////////////////////////////////
// Render the span into a rendertarget of the correct size, generating
// a depth map from this light to that span.
void plDXPipeline::IRenderShadowCasterSpan(plShadowSlave* slave, plDrawableSpans* drawable, const plIcicle& span)
{
    // Check that it's ready to render.
    plProfile_BeginTiming(CheckDyn);
    ICheckDynBuffers(drawable, drawable->GetBufferGroup(span.fGroupIdx), &span);
    plProfile_EndTiming(CheckDyn);

    plDXVertexBufferRef*    vRef = (plDXVertexBufferRef *)drawable->GetVertexRef(span.fGroupIdx, span.fVBufferIdx);
    plDXIndexBufferRef* iRef = (plDXIndexBufferRef *)drawable->GetIndexRef(span.fGroupIdx, span.fIBufferIdx);

    HRESULT     r;

    if (vRef->fD3DBuffer == nullptr || iRef->fD3DBuffer == nullptr)
    {
        hsAssert( false, "Trying to render a nil buffer pair!" );
        return;
    }

    /// Switch to the vertex buffer we want
    if( fSettings.fCurrVertexBuffRef != vRef )
    {
        hsRefCnt_SafeAssign( fSettings.fCurrVertexBuffRef, vRef );
        hsAssert(vRef->fD3DBuffer != nullptr, "Trying to render a buffer pair without a vertex buffer!");
        vRef->SetRebuiltSinceUsed(true);
    }

    if( vRef->RebuiltSinceUsed() )
    {
        r = fD3DDevice->SetStreamSource( 0, vRef->fD3DBuffer, 0, vRef->fVertexSize );
        hsAssert( r == D3D_OK, "Error trying to set the stream source!" );
        plProfile_Inc(VertexChange);

        fSettings.fCurrFVFFormat = IGetBufferD3DFormat(vRef->fFormat);
        r = fD3DDevice->SetVertexShader(fSettings.fCurrVertexShader = nullptr);
        fD3DDevice->SetFVF(fSettings.fCurrFVFFormat);
        hsAssert( r == D3D_OK, "Error trying to set the vertex shader!" );

        vRef->SetRebuiltSinceUsed(false);

    }

    if( fSettings.fCurrIndexBuffRef != iRef )
    {
        hsRefCnt_SafeAssign( fSettings.fCurrIndexBuffRef, iRef );
        hsAssert(iRef->fD3DBuffer != nullptr, "Trying to render with a nil index buffer");
        iRef->SetRebuiltSinceUsed(true);
    }

    if( iRef->RebuiltSinceUsed() )
    {
        r = fD3DDevice->SetIndices( iRef->fD3DBuffer );
        hsAssert( r == D3D_OK, "Error trying to set the indices!" );
        plProfile_Inc(IndexChange);
        iRef->SetRebuiltSinceUsed(false);
    }

    uint32_t                  vStart = span.fVStartIdx;
    uint32_t                  vLength = span.fVLength;
    uint32_t                  iStart = span.fIPackedIdx;
    uint32_t                  iLength= span.fILength;

    plRenderTriListFunc render(fD3DDevice, iRef->fOffset, vStart, vLength, iStart, iLength/3);

    static hsMatrix44 emptyMatrix;
    hsMatrix44 m = emptyMatrix;

    ISetupTransforms(drawable, span, m);

    bool flip = slave->ReverseCull();
    ISetCullMode(flip);

    render.RenderPrims();
}

// IGetULutTextureRef ///////////////////////////////////////////////////////////
// The ULut just translates a U coordinate in range [0..1] into 
// color and alpha of U * 255.9f. We just have the one we keep
// lying around.
plDXTextureRef* plDXPipeline::IGetULutTextureRef()
{
    const int width = 256;
    const int height = 1;
    if( !fULutTextureRef )
    {
        uint32_t* tData = new uint32_t[width * height];

        uint32_t* pData = tData;
        int j;
        for( j = 0; j < height; j++ )
        {
            int i;
            for( i = 0; i < width; i++ )
            {
                *pData = (i << 24)
                    | (i << 16)
                    | (i << 8)
                    | (i << 0);
                pData++;
            }
        }

        plDXTextureRef* ref = new plDXTextureRef( D3DFMT_A8R8G8B8, 
                                              1, // Num mip levels
                                              width, height, // width by height
                                              width * height, // numpix
                                              width*height*sizeof(uint32_t), // totalsize
                                              width*height*sizeof(uint32_t),
                                              nullptr, // levels data
                                              tData, 
                                              false // externData
                                              );
        ref->Link(&fTextureRefList);

        fULutTextureRef = ref;
    }
    return fULutTextureRef;
}

// IFindRenderTarget //////////////////////////////////////////////////////////////////
// Find a matching render target from the pools. We prefer the requested size, but
// will look for a smaller size if there isn't one available.
// Param ortho indicates whether it will be used for orthogonal projection as opposed
// to perspective (directional light vs. point light), but is no longer used.
plRenderTarget* plDXPipeline::IFindRenderTarget(uint32_t& width, uint32_t& height, bool ortho)
{
    hsTArray<plRenderTarget*>* pool = nullptr;
    uint32_t* iNext = nullptr;
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
        return nullptr;
    }
    plRenderTarget* rt = (*pool)[*iNext];
    if( !rt )
    {
        // We didn't find one, try again the next size down.
        if( height > 32 )
            return IFindRenderTarget(width >>= 1, height >>= 1, ortho);

        // We must be totally out. Oh well.
        return nullptr;
    }
    (*iNext)++;

    return rt;
}

// IPushShadowCastState ////////////////////////////////////////////////////////////////////////////////
// Push all the state necessary to start rendering this shadow map, but independent of the
// actual shadow caster to be rendered into the map.
bool plDXPipeline::IPushShadowCastState(plShadowSlave* slave)
{
    plRenderTarget* renderTarg = IFindRenderTarget(slave->fWidth, slave->fHeight, slave->fView.GetOrthogonal());
    if( !renderTarg )
        return false;

    // Let the slave setup the transforms, viewport, etc. necessary to render it's shadow
    // map. This just goes into a plViewTransform, we translate that into D3D state ourselves below.
    if (!slave->SetupViewTransform(this))
        return false;

    // Turn off fogging and specular.
    fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    fCurrFog.fEnvPtr = nullptr;
    fD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    fLayerState[0].fShadeFlags &= ~hsGMatState::kShadeSpecular;

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

    // For stage 0:
    // Set uvw src
    if( fLayerUVWSrcs[0] != D3DTSS_TCI_CAMERASPACEPOSITION )
    {
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        fLayerUVWSrcs[0] = D3DTSS_TCI_CAMERASPACEPOSITION;
    }
    uint32_t xformFlags = D3DTTFF_COUNT3;

    if( xformFlags != fLayerXformFlags[0] )
    {
        fLayerXformFlags[0] = xformFlags;
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, xformFlags);
    }

    // Set texture transform to slave's lut transform. See plShadowMaster::IComputeLUT().
    hsMatrix44 castLUT = slave->fCastLUT;
    if( slave->fFlags & plShadowSlave::kCastInCameraSpace )
    {
        hsMatrix44 c2w = GetCameraToWorld();

        castLUT = castLUT * c2w;
    }

    D3DMATRIX tXfm;
    IMatrix44ToD3DMatrix(tXfm, castLUT);

    fD3DDevice->SetTransform( sTextureStages[0], &tXfm );
    fLayerTransform[0] = true;

    // Set texture to clamp
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fLayerState[0].fClampFlags = hsGMatState::kClampTexture;

    DWORD clearColor = 0xff000000L;
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
        intens = 0xffffffff;

    // Note that we discard the shadow caster's alpha here, although we don't
    // need to. Even on a 2 texture stage system, we could include the diffuse
    // alpha and the texture alpha from the base texture. But we don't.

    // Set color to white. We could accomplish this easier by making the color
    // in our ULut white.
    fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, intens);

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
    fLayerState[1].fBlendFlags = uint32_t(-1);

    // Set texture to U_LUT
    plDXTextureRef* ref = IGetULutTextureRef();

    if( !ref->fD3DTexture )
    {
        if( ref->fData )
            IReloadTexture( ref );
    }

    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fD3DDevice->SetTexture( 0, ref->fD3DTexture );

    fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ONE);
    fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);

    fD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS);

    slave->fPipeData = renderTarg;

    // Enable ZBuffering w/ write
    fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    fLayerState[0].fZFlags &= ~hsGMatState::kZMask;

    // Clear the render target:
    // alpha to white ensures no shadow where there's no caster
    // color to black in case we ever get blurring going
    // Z to 1
    // Stencil ignored
    if( slave->ReverseZ() )
    {
        fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_GREATEREQUAL);
        fD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 0.0f, 0L);
    }
    else
    {
        fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
        fD3DDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clearColor, 1.0f, 0L);
    }

    // Bring the viewport in (AFTER THE CLEAR) to protect the alpha boundary.
    fView.GetViewTransform().SetViewPort(1, 1, (float)(slave->fWidth-2), (float)(slave->fHeight-2), false);
    fDevice.SetViewport();

    inlEnsureLightingOff();

    // See ISetupShadowLight below for how the shadow light is used.
    // The shadow light isn't used in generating the shadow map, it's used
    // in projecting the shadow map onto the scene.
    ISetupShadowLight(slave);

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
void plDXPipeline::ISetupShadowLight(plShadowSlave* slave)
{
    plDXLightRef* lRef = INextShadowLight(slave);

    lRef->fD3DInfo.Diffuse.r 
        = lRef->fD3DInfo.Diffuse.g 
        = lRef->fD3DInfo.Diffuse.b
        = slave->fPower;

    slave->fSelfShadowOn = false;

    if( slave->Positional() )
    {
        hsPoint3 position = slave->fLightPos;
        lRef->fD3DInfo.Position.x = position.fX;
        lRef->fD3DInfo.Position.y = position.fY;
        lRef->fD3DInfo.Position.z = position.fZ;

        const float maxRange = 32767.f;
        lRef->fD3DInfo.Range = maxRange;
        lRef->fD3DInfo.Attenuation0 = 1.f;
        lRef->fD3DInfo.Attenuation1 = 0;
        lRef->fD3DInfo.Attenuation2 = 0;

        lRef->fD3DInfo.Type = D3DLIGHT_POINT;
    }
    else
    {
        hsVector3 dir = slave->fLightDir;
        lRef->fD3DInfo.Direction.x = dir.fX;
        lRef->fD3DInfo.Direction.y = dir.fY;
        lRef->fD3DInfo.Direction.z = dir.fZ;

        lRef->fD3DInfo.Type = D3DLIGHT_DIRECTIONAL;
    }

    fD3DDevice->SetLight( lRef->fD3DIndex, &lRef->fD3DInfo );

    slave->fLightIndex = lRef->fD3DIndex;
}

// INextShadowLight /////////////////////////////////////////////////////
// Get a scratch light for this shadow slave and assign it. The slave
// only keeps it for this render frame.
plDXLightRef* plDXPipeline::INextShadowLight(plShadowSlave* slave)
{
    plDXLightRef* lightRef = fLights.fShadowLights.next([this] {
        plDXLightRef* lRef = new plDXLightRef();

        /// Assign stuff and update
        lRef->fD3DIndex = fLights.ReserveD3DIndex();
        lRef->fOwner = nullptr;
        lRef->fD3DDevice = fD3DDevice;

        lRef->Link( &fLights.fRefList );

        // Neutralize it until we need it.
        fD3DDevice->LightEnable(lRef->fD3DIndex, false);

        // Some things never change.
        memset(&lRef->fD3DInfo, 0, sizeof(lRef->fD3DInfo));
        lRef->fD3DInfo.Ambient.r = lRef->fD3DInfo.Ambient.g = lRef->fD3DInfo.Ambient.b = 0;
        lRef->fD3DInfo.Specular.r = lRef->fD3DInfo.Specular.g = lRef->fD3DInfo.Specular.b = 0;

        return lRef;
    });
    slave->fLightRefIdx = fLights.fShadowLights.size() - 1;

    return lightRef;
}

// IPopShadowCastState ///////////////////////////////////////////////////
// Pop the state set to render this shadow caster, so we're ready to render
// a different shadow caster, or go on to our main render.
bool plDXPipeline::IPopShadowCastState(plShadowSlave* slave)
{
    fView = fViewStack.top();
    fViewStack.pop();

    PopRenderTarget();
    fView.fXformResetFlags = fView.kResetProjection | fView.kResetCamera;

    return true;
}

// IMakeRenderTargetPools /////////////////////////////////////////////////////////////
// These are actually only used as shadow map pools, but they could be used for other
// render targets.
// All these are created here in a single call because they go in POOL_DEFAULT, so they
// must be created before we start creating things in POOL_MANAGED.
void plDXPipeline::IMakeRenderTargetPools()
{
    hsAssert(!fManagedAlloced, "Allocating rendertargets with managed resources alloced");
    IReleaseRenderTargetPools(); // Just to be sure.

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
        hsTArray<plRenderTarget*>* pool = nullptr;
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
            pool->SetCount((int)(kCount[i]+1));
            (*pool)[0] = nullptr;
            (*pool)[(int)(kCount[i])] = nullptr;
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
                // video memory. We'll return nullptr, and this guy just doesn't get a shadow
                // until more video memory turns up (not likely).
                if( !SharedRenderTargetRef((*pool)[0], rt) )
                {
                    delete rt;
                    pool->SetCount(j+1);
                    (*pool)[j] = nullptr;
                    break;
                }
                (*pool)[j] = rt;
            }
        }
    }
}

// IResetRenderTargetPools /////////////////////////////////////////////////////////////////
// No release of resources, this just resets for the start of a frame. So if a shadow
// slave gets a render target from a pool, once this is called (conceptually at the
// end of the frame), the slave no longer owns that render target.
void plDXPipeline::IResetRenderTargetPools()
{
    int i;
    for( i = 0; i < kMaxRenderTargetNext; i++ )
    {
        fRenderTargetNext[i] = 0;
        fBlurScratchRTs[i] = nullptr;
        fBlurDestRTs[i] = nullptr;
    }

    fLights.fShadowLights.clear();
}

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
bool plDXPipeline::IPrepShadowCaster(const plShadowCaster* caster)
{
    static hsBitVector done;
    done.Clear();
    const std::vector<plShadowCastSpan>& castSpans = caster->Spans();

    for (size_t i = 0; i < castSpans.size(); i++)
    {
        if( !done.IsBitSet(i) )
        {
            // We haven't already done this castSpan

            plDrawableSpans* drawable = castSpans[i].fDraw;

            // Start a visList with this index.
            static std::vector<int16_t> visList;
            visList.clear();
            visList.emplace_back((int16_t)(castSpans[i].fIndex));
            
            // We're about to have done this castSpan.
            done.SetBit(i);

            // Look forward through castSpans for any other spans
            // with the same drawable, and add them to visList.
            // We'll handle all the spans from this drawable at once.
            for (size_t j = i + 1; j < castSpans.size(); j++)
            {
                if( !done.IsBitSet(j) && (castSpans[j].fDraw == drawable) )
                {
                    // Add to list
                    visList.emplace_back((int16_t)(castSpans[j].fIndex));

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
bool plDXPipeline::IRenderShadowCaster(plShadowSlave* slave)
{
    const plShadowCaster* caster = slave->fCaster;

    // Setup to render into the slave's render target.
    if( !IPushShadowCastState(slave) )
        return false;

    // Get the shadow caster ready to render.
    if( !IPrepShadowCaster(slave->fCaster) )
        return false;

    // for each shadowCaster.fSpans
    for (const plShadowCastSpan& castSpan : caster->Spans())
    {
        plDrawableSpans* dr = castSpan.fDraw;
        const plSpan* sp = castSpan.fSpan;
        uint32_t spIdx = castSpan.fIndex;

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
    if( slave->fBlurScale > 0.f )
        IBlurShadowMap(slave);

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
void plDXPipeline::IPreprocessShadows()
{
    plProfile_BeginTiming(PrepShadows);

    // Mark our shared resources as free to be used.
    IResetRenderTargetPools();

    // Some board (possibly the Parhelia) freaked if anistropic filtering
    // was enabled when rendering to a render target. We never need it for
    // shadow maps, and it is slower, so we just kill it here.
    ISetAnisotropy(false);

    // Generate a shadow map for each submitted shadow slave.
    // Shadow slave corresponds to one shadow caster paired
    // with one shadow light that affects it. So a single caster
    // may be in multiple slaves (from different lights), or a
    // single light may be in different slaves (affecting different
    // casters). The overall number is low in spite of the possible
    // permutation explosion, because a slave is only generated
    // for a caster being affected (in range etc.) by a light.
    int iSlave;
    for( iSlave = 0; iSlave < fShadows.GetCount(); iSlave++ )
    {
        plShadowSlave* slave = fShadows[iSlave];
        
        // Any trouble, remove it from the list for this frame.
        if( !IRenderShadowCaster(slave) )
        {
            fShadows.Remove(iSlave);
            iSlave--;
            continue;
        }

    }

    // Restore
    ISetAnisotropy(true);

    plProfile_EndTiming(PrepShadows);
}

// IClearShadowSlaves ///////////////////////////////////////////////////////////////////////////
// At EndRender(), we need to clear our list of shadow slaves. They are only valid for one frame.
void plDXPipeline::IClearShadowSlaves()
{
    int i;
    for( i = 0; i < fShadows.GetCount(); i++ )
    {
        const plShadowCaster* caster = fShadows[i]->fCaster;
        caster->GetKey()->UnRefObject();
    }
    fShadows.SetCount(0);
}


// IRenderShadowsOntoSpan /////////////////////////////////////////////////////////////////////
// After doing the usual render for a span (all passes), we call the following.
// If the span accepts shadows, this will loop over all the shadows active this
// frame, and apply the ones that intersect this spans bounds. See below for details.
void plDXPipeline::IRenderShadowsOntoSpan(const plRenderPrimFunc& render, const plSpan* span, hsGMaterial* mat)
{
    // We've already computed which shadows affect this span. That's recorded in slaveBits.
    const hsBitVector& slaveBits = span->GetShadowSlaves();

    bool first = true;

    int i;
    for( i = 0; i < fShadows.GetCount(); i++ )
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

            int selfShadowNow = span->IsShadowBitSet(fShadows[i]->fIndex);

            // We vary the shadow intensity when self shadowing (see below),
            // so we cache whether the shadow light is set for regular or
            // self shadowing intensity. If what we're doing now is different
            // than what we're currently set for, set it again.
            if( selfShadowNow != fShadows[i]->fSelfShadowOn )
            {
                plDXLightRef* lRef = fLights.fShadowLights[fShadows[i]->fLightRefIdx];

                // We lower the power on self shadowing, because the artists like to
                // crank up the shadow strength to huge values to get a darker shadow
                // on the environment, which causes the shadow on the avatar to get 
                // way too dark. Another way to look at it is when self shadowing,
                // the surface being projected onto is going to be very close to
                // the surface casting the shadow (because they are the same object).
                if( selfShadowNow )
                {
                    plConst(float) kMaxSelfPower = 0.3f;
                    float power = fShadows[i]->fPower > kMaxSelfPower ? (float)kMaxSelfPower : fShadows[i]->fPower;
                    lRef->fD3DInfo.Diffuse.r 
                        = lRef->fD3DInfo.Diffuse.g 
                        = lRef->fD3DInfo.Diffuse.b
                        = power;
                }
                else
                {
                    lRef->fD3DInfo.Diffuse.r 
                        = lRef->fD3DInfo.Diffuse.g 
                        = lRef->fD3DInfo.Diffuse.b
                        = fShadows[i]->fPower;
                }
                fD3DDevice->SetLight(lRef->fD3DIndex, &lRef->fD3DInfo);

                // record which our intensity is now set for.
                fShadows[i]->fSelfShadowOn = selfShadowNow;
            }

            // Enable the light.
            fD3DDevice->LightEnable(fShadows[i]->fLightIndex, true);
            
#ifdef HS_DEBUGGING
            DWORD nPass;
            fSettings.fDXError = fD3DDevice->ValidateDevice(&nPass);
            if( fSettings.fDXError != D3D_OK )
                IGetD3DError();
#endif // HS_DEBUGGING

#ifndef PLASMA_EXTERNAL_RELEASE
            if (!IsDebugFlagSet(plPipeDbg::kFlagNoShadowApply))
#endif // PLASMA_EXTERNAL_RELEASE
                render.RenderPrims();
            
            // Disable it again.
            fD3DDevice->LightEnable(fShadows[i]->fLightIndex, false);

        }
    }

}

// ISetupShadowRcvTextureStages ////////////////////////////////////////////
// Set the generic stage states. We'll fill in the specific textures
// for each slave later.
void plDXPipeline::ISetupShadowRcvTextureStages(hsGMaterial* mat)
{
    // Setup for nil shaders to get us back to fixed function pipeline.
    ISetShaders(nullptr, nullptr);

    // We're whacking about with renderstate independent of current material,
    // so make sure the next span processes it's material, even if it's the
    // same one.
    fForceMatHandle = true;

    // Set the D3D lighting/material model
    ISetShadowLightState(mat);

    // Zbuffering on read-only
    fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
    fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    fLayerState[0].fZFlags &= ~hsGMatState::kZMask;
    fLayerState[0].fZFlags |= hsGMatState::kZNoZWrite;

    // Stage 0:
    // Texture is slave specific
    // Texture transform is slave specific
    // ColorArg1 = texture
    // ColorArg2 = diffuse
    // ColorOp = modulate
    // AlphaArg1 = texture
    // AlphaOp = SelectArg1
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);

    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1);

    if( fLayerUVWSrcs[0] != D3DTSS_TCI_CAMERASPACEPOSITION )
    {
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        fLayerUVWSrcs[0] = D3DTSS_TCI_CAMERASPACEPOSITION;
    }

    // Set texture to clamp
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fLayerState[0].fClampFlags = hsGMatState::kClampTexture;

    // Stage 1:
    // Set texture to ULut
    // Texture transform is slave specific
    // *** With the optional texture blurring, the state here becomes
    // *** partially slave dependent. Specifically, if we've done a blur,
    // *** then we want to modulate the lut color value by current (which is
    // *** the blurred color), else just select the lut. So we'll just move
    // *** the ColorOp down to the slave specific section.
    // %%% Okay, get this. The GeForce2 won't take a SelectArg1 on Stage1 if
    // %%% we're also trying to use Stage2 to modulate in the diffuse. But
    // %%% it WILL let us do a modulate on Stage1. So we're going to make sure
    // %%% that our shadowmap texture is white, then we can just modulate them
    // %%% with no effect. If we're blurring, we already wanted to modulate, so
    // %%% no change there. This means we can set the ColorOp now, rather than
    // %%% having to wait for the Slave specific section later.
    // ColorArg1 = 1 - ULut
    // ColorArg2 = Current
    // ColorOp = Modulate
    // AlphaArg1 = ULut
    // AlphaArg2 = Current
    // AlphaOp = Subtract
    plDXTextureRef* ref = IGetULutTextureRef();
    if( !ref->fD3DTexture )
    {
        if( ref->fData )
            IReloadTexture(ref);
    }
    hsRefCnt_SafeAssign(fLayerRef[1], ref);
    fD3DDevice->SetTexture(1, ref->fD3DTexture);

    // The following commented out block is kind of cool, because it
    // bases the darkness of the shadow on the distance between the 
    // shadow caster and the point receiving the shadow. So, for example,
    // the hand's shadow would get darker as it reaches for the lever.
    // Unfortunately, it doesn't guarantee that the shadow will completely
    // attenuate out at the fAttenDist (in fact, it pretty much guarantees
    // that it won't), so shadows will pop in and out. So instead, we'll
    // base the color on the distance from the start of the slave. The
    // difference is subtle, and usually unnoticable, and we get no popping.
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);

    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SUBTRACT);
    fLayerState[1].fBlendFlags = uint32_t(-1);

    if( fLayerUVWSrcs[1] != D3DTSS_TCI_CAMERASPACEPOSITION )
    {
        fD3DDevice->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEPOSITION);
        fLayerUVWSrcs[1] = D3DTSS_TCI_CAMERASPACEPOSITION;
    }
    if( D3DTTFF_COUNT3 != fLayerXformFlags[1] )
    {
        fLayerXformFlags[1] = D3DTTFF_COUNT3;
        fD3DDevice->SetTextureStageState(1, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_COUNT3);
    }

    // Set texture to clamp
    fD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fD3DDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fLayerState[1].fClampFlags = hsGMatState::kClampTexture;

    int iNextStage = 2;

    // If mat's base layer is alpha'd, and we have > 3 TMU's factor
    // in the base layer's alpha.   
    if( (fMaxLayersAtOnce > 3) && mat->GetLayer(0)->GetTexture() && (mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha) )
    {
        plLayerInterface* layer = mat->GetLayer(0);

        // If the following conditions are met, it means that layer 1 is a better choice to
        // get the transparency from. The specific case we're looking for is vertex alpha
        // simulated by an invisible second layer alpha LUT (known as the alpha hack).
        if( (layer->GetMiscFlags() & hsGMatState::kMiscBindNext) 
            && mat->GetLayer(1) 
            && !(mat->GetLayer(1)->GetMiscFlags() & hsGMatState::kMiscNoShadowAlpha)
            && !(mat->GetLayer(1)->GetBlendFlags() & hsGMatState::kBlendNoTexAlpha) 
            && mat->GetLayer(1)->GetTexture() )
                layer = mat->GetLayer(1);

        // Take the texture alpha and modulate the color so far with it. In
        // the final shadow map, black will have no effect, white will be maximal
        // darkening.
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_ALPHAREPLICATE);
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLORARG2, D3DTA_CURRENT);
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLOROP,   D3DTOP_MODULATE);

        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_ALPHAARG2, D3DTA_CURRENT); 
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);

        // Blend flags to layer blend (alpha +- complement)
        fLayerState[iNextStage].fBlendFlags = uint32_t(-1);

        // Clamp to whatever the texture wants.
        if( fLayerState[iNextStage].fClampFlags ^ layer->GetClampFlags() )
        {
            fLayerState[iNextStage].fClampFlags = layer->GetClampFlags();
            IHandleStageClamp(iNextStage);
        }

        // Shade to 0
        fLayerState[iNextStage].fShadeFlags = 0;

        // ZFlags to ZNoZWrite
        fLayerState[iNextStage].fZFlags = hsGMatState::kZNoZWrite;

        // MiscFlags to layer's misc flags
        fLayerState[iNextStage].fMiscFlags = layer->GetMiscFlags();

        // Set up whatever UVW transform the layer normally uses.
        IHandleStageTransform(iNextStage, layer);
    
        // Normal UVW source.
        uint32_t uvwSrc = layer->GetUVWSrc();

        if( fLayerUVWSrcs[ iNextStage ] != uvwSrc )
        {
            fD3DDevice->SetTextureStageState( iNextStage, D3DTSS_TEXCOORDINDEX, uvwSrc );
            fLayerUVWSrcs[ iNextStage ] = uvwSrc;
        }

        uint32_t xformFlags;
        if( layer->GetMiscFlags() & hsGMatState::kMiscPerspProjection )
            xformFlags = D3DTTFF_COUNT3 | D3DTTFF_PROJECTED;
        else if( uvwSrc & (plLayerInterface::kUVWNormal | plLayerInterface::kUVWPosition | plLayerInterface::kUVWReflect) )
            xformFlags = D3DTTFF_COUNT3;
        else
            xformFlags = D3DTTFF_COUNT2;

        if( xformFlags != fLayerXformFlags[iNextStage] )
        {
            fLayerXformFlags[iNextStage] = xformFlags;
            fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_TEXTURETRANSFORMFLAGS, xformFlags);
        }

        // This ref should be pretty safe to use, because we just rendered it.
        ref = (plDXTextureRef*)layer->GetTexture()->GetDeviceRef();

        hsRefCnt_SafeAssign( fLayerRef[iNextStage], ref );
        fD3DDevice->SetTexture( iNextStage, ref->fD3DTexture );

        iNextStage++;

        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLORARG1, D3DTA_DIFFUSE | D3DTA_ALPHAREPLICATE);
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLORARG2, D3DTA_CURRENT);
        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLOROP,   D3DTOP_MODULATE);

        fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);

        fLayerState[iNextStage].fBlendFlags = uint32_t(-1);

        iNextStage++;
    }

    fLayerState[iNextStage].fBlendFlags = uint32_t(-1);

    // And seal it up
    fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(iNextStage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    fLayerState[iNextStage].fBlendFlags = uint32_t(-1);

    fLastEndingStage = 0;

    // Now set the frame buffer blend
    // Remember that white darkens and black is no effect.
    // Form is Src * SrcBlend + Dst * DstBlend
    // We want inverse Src * Dst, so
    // Src * ZERO + Dst * InvSrc
    fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
    fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO);
    fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCCOLOR);

    fLayerState[0].fBlendFlags = uint32_t(-1);

    // Turn on alpha test. Alpha of zero means the shadow map depth
    // is greater or equal to the surface depth, i.e. the surface
    // is between the shadow caster and the light and doesn't receive
    // shadow.
    fD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL); 
    fD3DDevice->SetRenderState(D3DRS_ALPHAREF, 0x00000001);
    fLayerState[0].fBlendFlags |= hsGMatState::kBlendTest;

    fD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    fLayerState[0].fShadeFlags &= ~hsGMatState::kShadeSpecular; 

    // Set fog color to black
    // We should automatically reset it, because our blend mode is -1'd.
    fD3DDevice->SetRenderState(D3DRS_FOGCOLOR, 0);

#ifdef HS_DEBUGGING
    DWORD nPass;
    fSettings.fDXError = fD3DDevice->ValidateDevice(&nPass);
    if( fSettings.fDXError != D3D_OK )
        IGetD3DError();
#endif // HS_DEBUGGING
}

// ISetupShadowSlaveTextures //////////////////////////////////////////////
// Set any state specific to this shadow slave for projecting the slave's
// shadow map onto the surface.
void plDXPipeline::ISetupShadowSlaveTextures(plShadowSlave* slave)
{
    D3DMATRIX tXfm;

    hsMatrix44 c2w = GetCameraToWorld();

    // Stage 0:
    // Set Stage 0's texture to the slave's rendertarget.
    // Set texture transform to slave's camera to texture transform
    plRenderTarget* renderTarg = (plRenderTarget*)slave->fPipeData;
    hsAssert(renderTarg, "Processing a slave that hasn't been rendered");
    if( !renderTarg )
        return;
    plDXTextureRef* ref = (plDXTextureRef*)renderTarg->GetDeviceRef();
    hsAssert(ref, "Shadow map ref should have been made when it was rendered");
    if( !ref )
        return;

    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fD3DDevice->SetTexture( 0, ref->fD3DTexture );
    
    hsMatrix44 cameraToTexture = slave->fWorldToTexture * c2w;
    IMatrix44ToD3DMatrix(tXfm, cameraToTexture);

    fD3DDevice->SetTransform( sTextureStages[0], &tXfm );
    fLayerTransform[0] = true;

    // Directional lights (ortho projection) just use COUNT2, point lights use COUNT3|PROJECTED.
    uint32_t xformFlags = slave->fView.GetOrthogonal() ? D3DTTFF_COUNT2 : D3DTTFF_COUNT3 | D3DTTFF_PROJECTED;

    if( xformFlags != fLayerXformFlags[0] )
    {
        fLayerXformFlags[0] = xformFlags;
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, xformFlags);
    }

    // Stage 1: the lut
    // Set the texture transform to slave's fRcvLUT
    hsMatrix44 cameraToLut = slave->fRcvLUT * c2w;
    IMatrix44ToD3DMatrix(tXfm, cameraToLut);

    fD3DDevice->SetTransform( sTextureStages[1], &tXfm );
    fLayerTransform[1] = true;

}

// ISetShadowLightState //////////////////////////////////////////////////////////////////
// Set the D3D lighting/material model for projecting the shadow map onto this material.
void plDXPipeline::ISetShadowLightState(hsGMaterial* mat)
{
    IDisableLightsForShadow();
    inlEnsureLightingOn();

    fCurrLightingMethod = plSpan::kLiteShadow;

    static D3DMATERIAL9 d3dMat;
    if( mat && mat->GetNumLayers() && mat->GetLayer(0) )
        d3dMat.Diffuse.r = d3dMat.Diffuse.g = d3dMat.Diffuse.b = mat->GetLayer(0)->GetOpacity();
    else
        d3dMat.Diffuse.r = d3dMat.Diffuse.g = d3dMat.Diffuse.b = 1.f;
    d3dMat.Diffuse.a = 1.f;

    fD3DDevice->SetMaterial(&d3dMat);
    fD3DDevice->SetRenderState( D3DRS_AMBIENT, 0 );
}

// IDisableLightsForShadow ///////////////////////////////////////////////////////////
// Disable any lights that are enabled. We'll only want the shadow light illuminating
// the surface.
void plDXPipeline::IDisableLightsForShadow()
{
    int i;
    for( i = 0; i < fLights.fLastIndex + 1; i++ )
    {
        if( fLights.fEnabledFlags.IsBitSet(i) )
        {
            fD3DDevice->LightEnable(i, false);
        }
    }
    fLights.fEnabledFlags.Clear();
}

// IEnableShadowLight ///////////////////////////////////////////////
// Enable this shadow slave's light.
// NOT USED.
void plDXPipeline::IEnableShadowLight(plShadowSlave* slave)
{
    fD3DDevice->LightEnable(slave->fLightIndex, true);
}

void plDXPipeline::SubmitClothingOutfit(plClothingOutfit* co)
{
    if (fClothingOutfits.Find(co) == fClothingOutfits.kMissingIndex)
    {
        fClothingOutfits.Append(co);
        if (!fPrevClothingOutfits.RemoveItem(co))
            co->GetKey()->RefObject();
    }
}

void plDXPipeline::IClearClothingOutfits(hsTArray<plClothingOutfit*>* outfits)
{
    int i;
    for (i = outfits->GetCount() - 1; i >= 0; i--)
    {
        plClothingOutfit *co = outfits->Get(i);
        outfits->Remove(i);
        IFreeAvRT((plRenderTarget*)co->fTargetLayer->GetTexture());
        co->fTargetLayer->SetTexture(nullptr);
        co->GetKey()->UnRefObject();
    }
}

void plDXPipeline::IFillAvRTPool()
{
    fAvNextFreeRT = 0;
    fAvRTShrinkValidSince = hsTimer::GetSysSeconds();
    int numRTs = 1;
    if (fClothingOutfits.GetCount() > 1)
    {
        // Just jump to 8 for starters so we don't have to refresh for the 2nd, 4th, AND 8th player
        numRTs = 8;
        while (numRTs < fClothingOutfits.GetCount())
            numRTs *= 2;
    }

    // I could see a 32MB video card going down to 64x64 RTs in extreme cases
    // (over 100 players onscreen at once), but really, if such hardware is ever trying to push 
    // that, the low texture resolution is not going to be your major concern.
    for (fAvRTWidth = 1024 >> plMipmap::GetGlobalLevelChopCount(); fAvRTWidth >= 32; fAvRTWidth /= 2)
    {
        if (IFillAvRTPool(numRTs, fAvRTWidth))
            return;

        // Nope? Ok, lower the resolution and try again.
    }
}

bool plDXPipeline::IFillAvRTPool(uint16_t numRTs, uint16_t width)
{
    fAvRTPool.SetCount(numRTs);
    int i;
    for (i = 0; i < numRTs; i++)
    {
        uint16_t flags = plRenderTarget::kIsTexture | plRenderTarget::kIsProjected;
        uint8_t bitDepth = 32;
        uint8_t zDepth = 0;
        uint8_t stencilDepth = 0;
        fAvRTPool[i] = new plRenderTarget(flags, width, width, bitDepth, zDepth, stencilDepth);

        // If anyone fails, release everyone we've created.
        if (!MakeRenderTargetRef(fAvRTPool[i]))
        {
            int j;
            for (j = 0; j <= i; j++)
            {
                delete fAvRTPool[j];
            }
            return false;
        }
    }
    return true;
}

void plDXPipeline::IReleaseAvRTPool()
{
    int i;
    for (i = 0; i < fClothingOutfits.GetCount(); i++)
    {
        fClothingOutfits[i]->fTargetLayer->SetTexture(nullptr);
    }
    for (i = 0; i < fPrevClothingOutfits.GetCount(); i++)
    {
        fPrevClothingOutfits[i]->fTargetLayer->SetTexture(nullptr);
    }   
    for (i = 0; i < fAvRTPool.GetCount(); i++)
    {
        delete(fAvRTPool[i]);
    }
    fAvRTPool.Reset();
}

plRenderTarget *plDXPipeline::IGetNextAvRT()
{
    return fAvRTPool[fAvNextFreeRT++];
}

void plDXPipeline::IFreeAvRT(plRenderTarget* tex)
{
    uint32_t index = fAvRTPool.Find(tex);
    if (index != fAvRTPool.kMissingIndex)
    {
        hsAssert(index < fAvNextFreeRT, "Freeing an avatar RT that's already free?");
        fAvRTPool[index] = fAvRTPool[fAvNextFreeRT - 1];
        fAvRTPool[fAvNextFreeRT - 1] = tex;
        fAvNextFreeRT--;
    }
}

struct plAVTexVert
{
    float fPos[3];
    float fUv[2];
};

void plDXPipeline::IPreprocessAvatarTextures()
{   
    plProfile_Set(AvRTPoolUsed, fClothingOutfits.GetCount());
    plProfile_Set(AvRTPoolCount, fAvRTPool.GetCount());
    plProfile_Set(AvRTPoolRes, fAvRTWidth);
    plProfile_Set(AvRTShrinkTime, uint32_t(hsTimer::GetSysSeconds() - fAvRTShrinkValidSince));

    IClearClothingOutfits(&fPrevClothingOutfits); // Frees anyone used last frame that we don't need this frame

    const uint32_t kVFormat = D3DFVF_XYZ | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0);

    if (fClothingOutfits.GetCount() == 0)
        return;

    plMipmap *itemBufferTex = nullptr;

    fForceMatHandle = true;
    ISetShaders(nullptr, nullptr); // Has a side effect of futzing with our cull settings...

    // Even though we're going to use DrawPrimitiveUP, we explicitly set the current VB ref to nullptr,
    // otherwise we might try and use the same VB ref later, think it hasn't changed, and
    // not update our FVF. 
    hsRefCnt_SafeUnRef(fSettings.fCurrVertexBuffRef);
    fSettings.fCurrVertexBuffRef = nullptr;
    fD3DDevice->SetStreamSource(0, nullptr, 0, 0);
    fD3DDevice->SetFVF(fSettings.fCurrFVFFormat = kVFormat);
    fD3DDevice->SetTransform(D3DTS_VIEW, &d3dIdentityMatrix);
    fD3DDevice->SetTransform(D3DTS_WORLD, &d3dIdentityMatrix);
    fD3DDevice->SetTransform(D3DTS_PROJECTION, &d3dIdentityMatrix);
    fD3DDevice->SetRenderState(D3DRS_CULLMODE, fDevice.fCurrCullMode = D3DCULL_NONE);
    fD3DDevice->SetRenderState(D3DRS_VERTEXBLEND, D3DVBF_DISABLE);
    fD3DDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
    fD3DDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
    fLayerState[0].fZFlags &= ~hsGMatState::kZMask;
    fLayerState[0].fZFlags |= hsGMatState::kZNoZWrite | hsGMatState::kZNoZRead;
    if (fLayerUVWSrcs[0] != 0)
    {
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
        fLayerUVWSrcs[0] = 0;
    }
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
    fD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
    fLayerState[0].fClampFlags = hsGMatState::kClampTexture;
    if (D3DTTFF_DISABLE != fLayerXformFlags[0])
    {
        fLayerXformFlags[0] = D3DTTFF_DISABLE;
        fD3DDevice->SetTextureStageState(0, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
    }
    fD3DDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
    fLayerState[0].fShadeFlags &= ~hsGMatState::kShadeSpecular; 
    fD3DDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
    fCurrFog.fEnvPtr = nullptr;
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
    fD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR);
    fD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
    fD3DDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_ALWAYS); 
    fLayerState[0].fBlendFlags = uint32_t(-1);
    fD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
    fD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
    fLayerState[1].fBlendFlags = uint32_t(-1);
    inlEnsureLightingOff();

    int oIdx;
    for (oIdx = 0; oIdx < fClothingOutfits.GetCount(); oIdx++)
    {   
        plClothingOutfit *co = fClothingOutfits[oIdx];
        if (co->fBase == nullptr || co->fBase->fBaseTexture == nullptr)
            continue;

        plRenderTarget *rt = plRenderTarget::ConvertNoRef(co->fTargetLayer->GetTexture());
        if (rt != nullptr && co->fDirtyItems.Empty())
        {
            // we've still got our valid RT from last frame and we have nothing to do.
            continue;
        }

        if (rt == nullptr)
        {
            rt = IGetNextAvRT();
            co->fTargetLayer->SetTexture(rt);
        }

        PushRenderTarget(rt);
        D3DVIEWPORT9 vp = {0, 0, rt->GetWidth(), rt->GetHeight(), 0.f, 1.f};
        WEAK_ERROR_CHECK(fD3DDevice->SetViewport(&vp));

        float uOff = 0.5f / rt->GetWidth();
        float vOff = 0.5f / rt->GetHeight();

        // Copy over the base
        fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        fD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
        fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, 0xffffffff);
        fLayerState[0].fBlendFlags = uint32_t(-1);
        IDrawClothingQuad(-1.f, -1.f, 2.f, 2.f, uOff, vOff, co->fBase->fBaseTexture);
        plClothingLayout *layout = plClothingMgr::GetClothingMgr()->GetLayout(co->fBase->fLayoutName);

        for (plClothingItem *item : co->fItems)
        {
            //if (!co->fDirtyItems.IsBitSet(item->fTileset))
            //  continue; // Not dirty, don't update

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
                        fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
                        fD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
                    }
                    else
                    {
                        fD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
                        fD3DDevice->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
                        fD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
                        fD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE);
                    }
                    fD3DDevice->SetRenderState(D3DRS_TEXTUREFACTOR, tint.ToARGB32());
                    fLayerState[0].fBlendFlags = uint32_t(-1);
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
    // Nothing else sets this render state, so let's just set it back to the default to be safe
    fD3DDevice->SetRenderState(D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_RED | D3DCOLORWRITEENABLE_GREEN | D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA);
    fView.fXformResetFlags = fView.kResetAll;

    fClothingOutfits.Swap(fPrevClothingOutfits);
}

void plDXPipeline::IDrawClothingQuad(float x, float y, float w, float h, 
                                     float uOff, float vOff, plMipmap *tex)
{
    const uint32_t kVSize = sizeof(plAVTexVert);
    plDXTextureRef* ref = (plDXTextureRef*)tex->GetDeviceRef();
    if (!ref || ref->IsDirty())
    {
        MakeTextureRef(nullptr, tex);
        ref = (plDXTextureRef*)tex->GetDeviceRef();
    }
    if (!ref->fD3DTexture)
    {
        if (ref->fData)
            IReloadTexture(ref);
    }
    hsRefCnt_SafeAssign( fLayerRef[0], ref );
    fD3DDevice->SetTexture(0, ref->fD3DTexture);

    plAVTexVert ptr[4];
    plAVTexVert vert;
    vert.fPos[0] = x;
    vert.fPos[1] = y;
    vert.fPos[2] = 0.5f;
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

#ifdef HS_DEBUGGING
    DWORD nPass;
    fSettings.fDXError = fD3DDevice->ValidateDevice(&nPass);
    if( fSettings.fDXError != D3D_OK )
        IGetD3DError();
#endif // HS_DEBUGGING
    fD3DDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, ptr, kVSize);
}

///////////////////////////////////////////////////////////////////////////////
// Test hackery as R&D for water
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// End Test hackery as R&D for water
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//// Functions from Other Classes That Need to Be Here to Compile Right ///////
///////////////////////////////////////////////////////////////////////////////

plPipeline  *plPipelineCreate::ICreateDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode )
{
    plDXPipeline    *pipe = new plDXPipeline( hWnd, devMode );

    // Taken out 8.1.2001 mcn - If we have an error, still return so the client can grab the string
//  if (pipe->GetErrorString() != nullptr)
//  {
//      delete pipe;
//      pipe = nullptr;
//  }

    return pipe;
}
