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
#ifndef _plDX9Pipeline_h
#define _plDX9Pipeline_h

#include <vector>

#include "plPipeline/pl3DPipeline.h"
#include "plDXSettings.h"
#include "plDXDevice.h"

#include "plSurface/plLayerInterface.h"
#include "hsMatrix44.h"
#include "plPipeline/plFogEnvironment.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"
#include "hsGDeviceRef.h"
#include "hsPoint2.h"

class plAccessSpan;
class plAuxSpan;
class plVertexSpan;

#include "plPipeline/plPlates.h"   // Used to define plDXPlateManager


//// Defines and Konstants and Other Nifty Stuff //////////////////////////////

class plDXLightRef;
class plDXVertexBufferRef;
class plDXIndexBufferRef;
class plDXTextureRef;
class plDXCubeTextureRef;
class plDXVertexShader;
class plDXPixelShader;

class plShader;

class plVisMgr;

//#define HS_D3D_USE_SPECULAR

class hsGMaterial;
class plMipmap;

class plLightInfo;
class plCullTree;
class plShadowSlave;
class plShadowCaster;

struct _D3DMATRIX;
typedef _D3DMATRIX D3DMATRIX;

#ifdef HS_DEBUGGING
#define HS_CHECK_RELEASE
#endif

#ifndef PLASMA_EXTERNAL_RELEASE
#define PROFILE_POOL_MEM(pool, size, add, id)       plDXPipeline::ProfilePoolMem(pool, size, add, id);
#else
#define PROFILE_POOL_MEM(pool, size, add, id)
#endif // PLASMA_EXTERNAL_RELEASE

extern void D3DSURF_MEMNEW(IDirect3DSurface9* surf);
extern void D3DSURF_MEMNEW(IDirect3DTexture9* tex);
extern void D3DSURF_MEMNEW(IDirect3DCubeTexture9* cTex);
extern void D3DSURF_MEMDEL(IDirect3DSurface9* surf);
extern void D3DSURF_MEMDEL(IDirect3DTexture9* tex);
extern void D3DSURF_MEMDEL(IDirect3DCubeTexture9* cTex);


extern void plReleaseObject(IUnknown* x);

#define  ReleaseObject(x)   if (x) { plReleaseObject(x); x = nullptr; }

typedef LPDIRECT3D9 (WINAPI * Direct3DCreateProc)( UINT sdkVersion );


//// Helper Classes ///////////////////////////////////////////////////////////

//// The RenderPrimFunc lets you have one function which does a lot of stuff
// around the actual call to render whatever type of primitives you have, instead
// of duplicating everything because the one line to render is different.
class plRenderPrimFunc
{
public:
    virtual bool RenderPrims() const = 0; // return true on error
};

//// DX-specific Plate Manager implementation
class plDXPlateManager : public plPlateManager
{
    friend class plDXPipeline;

    public:

        virtual ~plDXPlateManager();

    protected:

        const long  PLD3D_PLATEFVF;

        struct plPlateVertex
        {
            hsPoint3    fPoint;
            uint32_t      fColor;
            hsPoint3    fUV;
        };

        IDirect3DDevice9        *fD3DDevice;
        IDirect3DVertexBuffer9  *fVertBuffer;

        plDXPlateManager( plDXPipeline *pipe, IDirect3DDevice9 *device );

        void ICreateGeometry(plDXPipeline* pipe);
        void IReleaseGeometry();

        void    IDrawToDevice(plPipeline *pipe) override;
};

//// Class Definition /////////////////////////////////////////////////////////

class plDebugTextManager;
struct D3DEnum_DriverInfo;
struct D3DEnum_DeviceInfo;
struct D3DEnum_ModeInfo;
class plGeometrySpan;
class plDrawableSpans;
class plSpan;
class plIcicle;
class hsG3DDeviceModeRecord;
class plDXDeviceRef;
class plParticleSpan;
class plCubicEnvironmap;
class plDXRenderTargetRef;
class plStatusLogDrawer;

class plDXPipeline : public pl3DPipeline<plDXDevice>
{
protected:
    enum {
        kCapsNone               = 0x00000000,
        kCapsCompressTextures   = 0x00000001,
        kCapsMipmap             = 0x00000002,
        kCapsHWTransform        = 0x00000004,
        kCapsHWLighting         = 0x00000008,
        kCapsZBias              = 0x00000010,
        kCapsLinearFog          = 0x00000020,
        kCapsExpFog             = 0x00000040,
        kCapsExp2Fog            = 0x00000080,
        kCapsRangeFog           = 0x00000100,
        kCapsTexBoundToStage    = 0x00000200,
        kCapsLODWatch           = 0x00000400,
        kCapsFSAntiAlias        = 0x00000800,
        kCapsLuminanceTextures  = 0x00001000,
        kCapsDoesSmallTextures  = 0x00002000,
        kCapsDoesWFog           = 0x00004000,
        kCapsPixelFog           = 0x00008000,
        kCapsHasBadYonStuff     = 0x00010000,
        kCapsNpotTextures       = 0x00020000,
        kCapsCubicTextures      = 0x00040000,
        kCapsCubicMipmap        = 0x00080000
    };
    enum {
        kKNone                  = 0x0,
        kKTNT                   = 0x1
    };

    // The main D3D interfaces
    LPDIRECT3DDEVICE9       fD3DDevice;     // The D3D rendering device

    IDirect3DSurface9*      fSharedDepthSurface[2];
    D3DFORMAT               fSharedDepthFormat[2];

    // Dynamic buffers
    uint32_t                fVtxRefTime;
    uint32_t                fNextDynVtx;
    uint32_t                fDynVtxSize;
    IDirect3DVertexBuffer9* fDynVtxBuff;
    bool                    fManagedAlloced;
    bool                    fAllocUnManaged;


    // States
    plDXGeneralSettings     fSettings;
    plDXStencilSettings     fStencil;
    bool                    fDeviceLost;
    bool                    fDevWasLost;

    plDXTextureRef*         fTextureRefList;
    plTextFont*             fTextFontRefList;
    plDXRenderTargetRef*    fRenderTargetRefList;
    plDXVertexShader*       fVShaderRefList;
    plDXPixelShader*        fPShaderRefList;

    bool                        fCurrD3DLiteState;

    UINT                    fCurrentAdapter;
    D3DEnum_DriverInfo*     fCurrentDriver;
    D3DEnum_DeviceInfo*     fCurrentDevice;
    D3DEnum_ModeInfo*       fCurrentMode;

    hsGMatState     fLayerState[ 8 ]; // base stage (0) state is held in base class
    hsGMatState     fOldLayerState[ 8 ];
    bool            fLayerTransform[ 8 ];
    float           fLayerLODBias[ 8 ];
    uint32_t        fLayerUVWSrcs[ 8 ];
    uint32_t        fLayerXformFlags[ 8 ];
    uint32_t        fLastEndingStage;
    bool            fTexturing;

    static uint32_t   fTexManaged;
    static uint32_t   fTexUsed;
    static uint32_t   fVtxManaged;
    static uint32_t   fVtxUsed;

    uint32_t          fDebugSpanGraphY;

    // Fog
    plDXFogSettings fCurrFog;

    // Light
    plDXLightSettings   fLights;

    // Shadows
    std::vector<plRenderTarget*>    fRenderTargetPool512;
    std::vector<plRenderTarget*>    fRenderTargetPool256;
    std::vector<plRenderTarget*>    fRenderTargetPool128;
    std::vector<plRenderTarget*>    fRenderTargetPool64;
    std::vector<plRenderTarget*>    fRenderTargetPool32;
    enum { kMaxRenderTargetNext = 10 };
    uint32_t                          fRenderTargetNext[kMaxRenderTargetNext];
    plDXTextureRef*                 fULutTextureRef;
    plRenderTarget*                 fBlurScratchRTs[kMaxRenderTargetNext];
    plRenderTarget*                 fBlurDestRTs[kMaxRenderTargetNext];
    IDirect3DVertexBuffer9*         fBlurVBuffers[kMaxRenderTargetNext];
    uint32_t                          fBlurVSHandle;

    // Debug stuff
    plDrawableSpans *fBoundsSpans;
    hsGMaterial     *fBoundsMat;
    std::vector<uint32_t> fBSpansToDelete;

    plStatusLogDrawer   *fLogDrawer;

    bool            fForceDeviceReset;

    void            IBeginAllocUnManaged();
    void            IEndAllocUnManaged();

    bool            IRefreshDynVertices(plGBufferGroup* group, plDXVertexBufferRef* vRef);
    bool            ICheckAuxBuffers(const plAuxSpan* span);
    bool            ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* span);
    void            ICheckStaticVertexBuffer(plDXVertexBufferRef* vRef, plGBufferGroup* owner, uint32_t idx);
    void            ICheckIndexBuffer(plDXIndexBufferRef* iRef);
    void            IFillStaticVertexBufferRef(plDXVertexBufferRef *ref, plGBufferGroup *group, uint32_t idx);
    void            IFillVolatileVertexBufferRef(plDXVertexBufferRef* ref, plGBufferGroup* group, uint32_t idx);
    void            IFillIndexBufferRef(plDXIndexBufferRef* iRef, plGBufferGroup* owner, uint32_t idx);
    void            ISetupVertexBufferRef(plGBufferGroup* owner, uint32_t idx, plDXVertexBufferRef* vRef);
    void            ISetupIndexBufferRef(plGBufferGroup* owner, uint32_t idx, plDXIndexBufferRef* iRef);
    void            ICreateDynamicBuffers();
    void            IReleaseDynamicBuffers();

    void            IAddBoundsSpan( plDrawableSpans *ice, const hsBounds3Ext *bounds, uint32_t bndColor = 0xffff0000 );
    void            IAddNormalsSpan( plDrawableSpans *ice, plIcicle *span, plDXVertexBufferRef *vRef, uint32_t bndColor );

    // Rendering
    bool        IFlipSurface();
    long        IGetBufferD3DFormat(uint8_t format) const;
    uint32_t    IGetBufferFormatSize(uint8_t format) const;
    void        IGetVisibleSpans(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr);
    bool        ILoopOverLayers(const plRenderPrimFunc& render, hsGMaterial* material, const plSpan& span);
    void        IRenderBufferSpan( const plIcicle& span, 
                                    hsGDeviceRef *vb, hsGDeviceRef *ib, 
                                    hsGMaterial *material, 
                                    uint32_t vStart, uint32_t vLength, uint32_t iStart, uint32_t iLength );
    void        IRenderAuxSpan(const plSpan& span, const plAuxSpan* aux);
    void        IRenderAuxSpans(const plSpan& span);

    // Fog
    void        IGetVSFogSet(float* const set) const;
    void        ISetFogParameters(const plSpan* span, const plLayerInterface* baseLay);

    // Lighting
    hsGDeviceRef    *IMakeLightRef( plLightInfo *owner );
    void            IScaleD3DLight( plDXLightRef *ref, float scale);
    void            ICalcLighting( const plLayerInterface *currLayer, const plSpan *currSpan );
    void            IDisableSpanLights();
    void            IRestoreSpanLights();
    void            ISelectLights( plSpan *span, int numLights, bool proj );
    void            IEnableLights( plSpan *span );
    inline void     inlEnsureLightingOff();
    inline void     inlEnsureLightingOn();
    void            IRenderProjection(const plRenderPrimFunc& render, plLightInfo* li);
    void            IRenderProjections(const plRenderPrimFunc& render);
    void            IRenderProjectionEach(const plRenderPrimFunc& render, hsGMaterial* material, int iPass, const plSpan& span);
    void            IRenderOverWire(const plRenderPrimFunc& render, hsGMaterial* material, const plSpan& span);

    bool                    ISkipBumpMap(hsGMaterial* newMat, uint32_t& layer, const plSpan* currSpan) const;
    void                    ISetBumpMatrices(const plLayerInterface* layer, const plSpan* span);
    const hsMatrix44&       IGetBumpMatrix(uint32_t miscFlags) const;

    // Materials
    const hsGMatState&  ICompositeLayerState(int which, plLayerInterface* layer);
    int32_t       IHandleMaterial(hsGMaterial* newMat, uint32_t which, const plSpan* currSpan);
    void        IHandleFirstTextureStage( plLayerInterface* layer );
    void        IHandleShadeMode();
    void        IHandleZMode();
    void        IHandleMiscMode();
    void        IHandleTextureStage(uint32_t stage, plLayerInterface* layer);
    void        IHandleFirstStageBlend();
    void        IHandleBumpEnv(int stage, uint32_t blendFlags);
    void        IHandleStageBlend(int stage);
    void        IHandleStageClamp(int stage);
    void        IHandleStageTransform(int stage, plLayerInterface* layer);
    void        IHandleTextureMode(plLayerInterface* layer);
    void        IUseTextureRef(int stage, hsGDeviceRef* dRef, plLayerInterface* layer);
    void        IStageStop(uint32_t stage);
    uint32_t    ILayersAtOnce(hsGMaterial* mat, uint32_t which);
    bool        ICanEatLayer(plLayerInterface* lay);
    void        ISetLayer(uint32_t lay);
    void        IBottomLayer();

    // Push special effects
    plLayerInterface*   IPushOverBaseLayer(plLayerInterface* li);
    plLayerInterface*   IPopOverBaseLayer(plLayerInterface* li);
    plLayerInterface*   IPushOverAllLayer(plLayerInterface* li);
    plLayerInterface*   IPopOverAllLayer(plLayerInterface* li);

    int                 ISetNumActivePiggyBacks();
    void                IPushPiggyBacks(hsGMaterial* mat);
    void                IPopPiggyBacks();
    void                IPushProjPiggyBack(plLayerInterface* li);
    void                IPopProjPiggyBacks();

    void                ISetPipeConsts(plShader* shader);
    HRESULT             ISetShaders(plShader* vShader, plShader* pShader);

    // Stenciling
    virtual bool            StencilEnable( bool enable );
    virtual void            StencilSetCompareFunc( uint8_t func, uint32_t refValue );
    virtual void            StencilSetMask( uint32_t mask, uint32_t writeMask );
    virtual void            StencilSetOps( uint8_t passOp, uint8_t failOp, uint8_t passButZFailOp );
    virtual bool            StencilGetCaps( plStencilCaps *caps );

    hsGDeviceRef    *MakeTextureRef( plLayerInterface* layer, plMipmap *b );
    void            IReloadTexture( plDXTextureRef *ref );
    void            IFillD3DTexture( plDXTextureRef *ref );
    void            IFillD3DCubeTexture( plDXCubeTextureRef *ref );
    void            IGetD3DTextureFormat( plBitmap *b, D3DFORMAT &formatType, uint32_t& texSize );
    void            IFormatTextureData( uint32_t formatType, uint32_t numPix, hsRGBAColor32* const src, void *dst );
    void            *IGetPixelScratch( uint32_t size );
    hsGDeviceRef    *IMakeCubicTextureRef( plLayerInterface* layer, plCubicEnvironmap *cubic );
    bool            IProcessMipmapLevels( plMipmap *mipmap, uint32_t &numLevels,
                                                uint32_t *&levelSizes, uint32_t &totalSize, 
                                                uint32_t &numPixels, void *&textureData, bool noMip );
    IDirect3DTexture9       *IMakeD3DTexture( plDXTextureRef *ref, D3DFORMAT formatType );
    IDirect3DCubeTexture9   *IMakeD3DCubeTexture( plDXTextureRef *ref, D3DFORMAT formatType );

    // Visualization of active occluders
    void            IMakeOcclusionSnap();

    bool            IAvatarSort(plDrawableSpans* d, const std::vector<int16_t>& visList);
    void            IBlendVertsIntoBuffer( plSpan* span, 
                                            hsMatrix44* matrixPalette, int numMatrices,
                                            const uint8_t *src, uint8_t format, uint32_t srcStride, 
                                            uint8_t *dest, uint32_t destStride, uint32_t count, uint16_t localUVWChans )
                                                { blend_vert_buffer.call(span, matrixPalette, numMatrices, src, format, srcStride, dest, destStride, count, localUVWChans); };
    bool            ISoftwareVertexBlend(plDrawableSpans* drawable, const std::vector<int16_t>& visList);


    void            ILinkDevRef( plDXDeviceRef *ref, plDXDeviceRef **refList );
    void            IUnlinkDevRef( plDXDeviceRef *ref );

    // Properties
    inline DWORD            inlGetD3DColor( const hsColorRGBA &c ) const;
    inline D3DCOLORVALUE    inlPlToD3DColor(const hsColorRGBA& c, float a) const;

    // Error handling
    void    IAddErrorMessage( char *errStr );
    void    ISetErrorMessage(char *errStr = nullptr);
    void    IGetD3DError();
    void    IShowErrorMessage(char *errStr = nullptr);
    bool    ICreateFail( char *errStr );

    // Device initialization
    void    IInvalidateState();
    void    IInitDeviceState();
    void    IClearMembers();
    void    ISetCaps();
    void    IRestrictCaps( const hsG3DDeviceRecord& devRec );
    void    ISetGraphicsCapability(uint32_t v);

    bool    IFindDepthFormat(D3DPRESENT_PARAMETERS& params);
    bool    IFindCompressedFormats();
    bool    IFindLuminanceFormats();
    bool    ITextureFormatAllowed( D3DFORMAT format );

    void    ISetCurrentDriver( D3DEnum_DriverInfo *driv );
    void    ISetCurrentDevice( D3DEnum_DeviceInfo *dev );
    void    ISetCurrentMode( D3DEnum_ModeInfo *mode );

    bool        ICreateDevice(bool windowed);
    bool        ICreateNormalSurfaces();

    bool        ICreateDeviceObjects();
    void        IReleaseDeviceObjects();

    bool        ICreateDynDeviceObjects();
    void        IReleaseDynDeviceObjects();
    void        IReleaseShaders();

    bool        IResetDevice();

    // View and clipping
    void        IUpdateViewVectors() const;
    void        ISetAnisotropy(bool on);

    // Transforms
    D3DMATRIX&     IMatrix44ToD3DMatrix( D3DMATRIX& dst, const hsMatrix44& src );
    void            ISetCullMode(bool flip=false);
    bool inline   IIsViewLeftHanded();
    bool            IGetClearViewPort(D3DRECT& r);
    void            ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W);

    // Plate management
    friend class plDXPlateManager;
    friend class plDXDevice;

    void        IDrawPlate( plPlate *plate );

    void    ISetRenderTarget( plRenderTarget *target );

    bool    IPrepRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat,
                                    D3DFORMAT &depthFormat, D3DRESOURCETYPE &resType );
    bool    IFindRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat, D3DRESOURCETYPE &resType );

    // From a D3DFORMAT enumeration, return the string literal for it
    static const char   *IGetDXFormatName( D3DFORMAT format );

    /////// Shadow internals
    // Generation
    void    IClearShadowSlaves();
    void    IPreprocessShadows();
    bool    IPrepShadowCaster(const plShadowCaster* caster);
    void    IRenderShadowCasterSpan(plShadowSlave* slave, plDrawableSpans* drawable, const plIcicle& span);
    void    ISetupShadowCastTextureStages(plShadowSlave* slave);
    bool    IRenderShadowCaster(plShadowSlave* slave);
    void    ISetupShadowLight(plShadowSlave* slave);
    plDXLightRef*   INextShadowLight(plShadowSlave* slave);

    bool    IPushShadowCastState(plShadowSlave* slave);
    bool    IPopShadowCastState(plShadowSlave* slave);

    plDXTextureRef* IGetULutTextureRef();
    bool                ICreateBlurVBuffers();
    void                IReleaseBlurVBuffers();
    void                IMakeRenderTargetPools();
    void                IResetRenderTargetPools();
    plRenderTarget*     IFindRenderTarget(uint32_t& w, uint32_t& h, bool ortho);
    void                IReleaseRenderTargetPools();

    // Application  
    void    IRenderShadowsOntoSpan(const plRenderPrimFunc& render, const plSpan* span, hsGMaterial* mat);
    void    ISetupShadowRcvTextureStages(hsGMaterial* mat);
    void    ISetShadowLightState(hsGMaterial* mat);
    void    IDisableLightsForShadow();
    void    IEnableShadowLight(plShadowSlave* slave);
    void    ISetupShadowSlaveTextures(plShadowSlave* slave);

    // Postprocess (blurring)
    bool                ISetBlurQuadToRender(plRenderTarget* smap);
    void                IRenderBlurBackToShadowMap(plRenderTarget* smap, plRenderTarget* scratch, plRenderTarget* dst);
    void                IRenderBlurFromShadowMap(plRenderTarget* scratchRT, plRenderTarget* smap, float scale);
    void                IBlurSetRenderTarget(plRenderTarget* rt);
    int                 IGetScratchRenderTarget(plRenderTarget* smap);
    void                IBlurShadowMap(plShadowSlave* slave);

    // Avatar Texture Rendering
    void                    IPreprocessAvatarTextures();
    void                    IDrawClothingQuad(float x, float y, float w, float h, float uOff, float vOff, plMipmap *tex);

    void IPrintDeviceInitError();

    void IResetToDefaults(D3DPRESENT_PARAMETERS *params);

public:
    plDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode );
    virtual ~plDXPipeline();

    CLASSNAME_REGISTER( plDXPipeline );
    GETINTERFACE_ANY( plDXPipeline, plPipeline);

    virtual IDirect3DDevice9*           GetD3DDevice() const { return fD3DDevice; }

    // Typical 3D device
    bool                        PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;
    bool                        PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) override;

    void                        PushRenderRequest(plRenderRequest* req) override;
    void                        PopRenderRequest(plRenderRequest* req) override;

    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync = false) override;

    void                        ClearRenderTarget(plDrawable* d) override;
    void                        ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) override;
    hsGDeviceRef*               MakeRenderTargetRef(plRenderTarget *owner) override;
    virtual hsGDeviceRef*       SharedRenderTargetRef(plRenderTarget* sharer, plRenderTarget *owner);

    bool                        BeginRender() override;
    bool                        EndRender() override;
    void                        RenderScreenElements() override;

    bool                        IsFullScreen() const override { return fSettings.fFullscreen; }
    void                        Resize(uint32_t width, uint32_t height) override;

    void                        LoadResources() override;    // Tells us where it's a good time to load in unmanaged resources.

    // Create a debug text font object
    plTextFont      *MakeTextFont(char *face, uint16_t size) override;

    // Create and/or Refresh geometry buffers
    void            CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) override;
    void            CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) override;

    bool            OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) override;
    bool            CloseAccess(plAccessSpan& acc) override;

    void            CheckTextureRef(plLayerInterface* lay) override;
    static void     FreeManagedTexture(uint32_t sz) { hsAssert(fTexManaged >= sz, "Freeing mem we don't have"); fTexManaged -= sz; }
    static void     AllocManagedTexture(uint32_t sz) { fTexManaged += sz; }
    static void     FreeManagedVertex(uint32_t sz) { hsAssert(fVtxManaged >= sz, "Freeing mem we don't have"); fVtxManaged -= sz; }
    static void     AllocManagedVertex(uint32_t sz) { fVtxManaged += sz; }

#ifndef PLASMA_EXTERNAL_RELEASE
    static void ProfilePoolMem(D3DPOOL poolType, uint32_t size, bool add, const char *id);
#endif // PLASMA_EXTERNAL_RELEASE

    //  From a D3DFORMAT enumeration, return the bit depth associated with it.
    static short    GetDXBitDepth( D3DFORMAT format );

    // Default fog settings
    void            SetDefaultFogEnviron(plFogEnvironment *fog) override { fView.SetDefaultFog(*fog); fCurrFog.fEnvPtr = nullptr; }



    // Overriden (Un)Register Light methods
    void            RegisterLight(plLightInfo* light) override;
    void            UnRegisterLight(plLightInfo* light) override;

    bool            SetGamma(float eR, float eG, float eB) override;
    bool            SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) override;

    bool            CaptureScreen(plMipmap *dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) override;
    plMipmap*       ExtractMipMap(plRenderTarget* targ) override;

    /// Error handling
    const char      *GetErrorString() override;

    bool            ManagedAlloced() const { return fManagedAlloced; }

    virtual void    GetSupportedColorDepths(std::vector<int> &ColorDepths);
    void            GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32) override;
    int             GetMaxAnisotropicSamples() override;
    int             GetMaxAntiAlias(int Width, int Height, int ColorDepth) override;

    void RenderSpans(plDrawableSpans *ice, const std::vector<int16_t>& visList) override;

    //  CPU-optimized functions
protected:
    typedef void(*blend_vert_buffer_ptr)(plSpan*, hsMatrix44*, int, const uint8_t *,
                                         uint8_t , uint32_t, uint8_t *, uint32_t,
                                         uint32_t, uint16_t);
    static hsCpuFunctionDispatcher<blend_vert_buffer_ptr> blend_vert_buffer;
};


//// Direct3D Inlines //////////////////////////////////////////////////////
//  ??.?? - Some mild optimizations PBG
//  MMW - take advantage of the 32 bit float representation on a PC

#define CONVERT_FLOAT_TO_BYTE_COLOR( f, dest ) \
    { \
        LONG const floatBitsOne = 0x3f800000; \
        LONG const floatBits = *( (LONG const *)( &f ) ); \
        if( floatBits <= 0 ) dest = 0; \
        else if( floatBits >= floatBitsOne ) dest = 255; \
        else \
        { \
            LONG const times256 = floatBits + ( 8 << 23 ); \
            dest = (DWORD)( *( (float const *)( &times256 ) ) ); \
        } \
    }

inline DWORD    plDXPipeline::inlGetD3DColor( const hsColorRGBA &col ) const
{
    DWORD   dr, dg, db, da;
    
    CONVERT_FLOAT_TO_BYTE_COLOR( col.r, dr );   
    CONVERT_FLOAT_TO_BYTE_COLOR( col.g, dg );
    CONVERT_FLOAT_TO_BYTE_COLOR( col.b, db );
    CONVERT_FLOAT_TO_BYTE_COLOR( col.a, da );
  
    return( ( da << 24 ) | ( dr << 16 ) | ( dg << 8 ) | db );
}


#endif // _plDX9Pipeline_h
