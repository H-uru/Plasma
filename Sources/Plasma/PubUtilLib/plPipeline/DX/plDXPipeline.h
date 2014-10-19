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

#include "plPipeline.h"
#include "plDXSettings.h"

#include "plSurface/plLayerInterface.h"
#include "hsMatrix44.h"
#include "plPipeline/plFogEnvironment.h"
#include "plPipeline/hsG3DDeviceSelector.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "hsColorRGBA.h"
#include "plPipeline/hsGDeviceRef.h"
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

struct D3DXMATRIX;

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

#define  ReleaseObject(x)   if(x){ plReleaseObject(x); x=NULL; }

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

        virtual void    IDrawToDevice( plPipeline *pipe );
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

class plDXPipeline : public plPipeline
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

    plDebugTextManager*         fDebugTextMgr;
    plDXPlateManager*           fPlateMgr;

    // The main D3D interfaces
    LPDIRECT3DDEVICE9       fD3DDevice;     // The D3D rendering device
    IDirect3DSurface9*      fD3DMainSurface;
    IDirect3DSurface9*      fD3DDepthSurface;
    IDirect3DSurface9*      fD3DBackBuff;

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
    plDXTweakSettings       fTweaks;
    plDXStencilSettings     fStencil;
    bool                    fDeviceLost;
    bool                    fDevWasLost;

    hsTArray<const plCullPoly*> fCullPolys;
    hsTArray<const plCullPoly*> fCullHoles;
    plDrawableSpans*            fCullProxy;
    
    plDXVertexBufferRef*    fVtxBuffRefList;
    plDXIndexBufferRef*     fIdxBuffRefList;
    plDXTextureRef*         fTextureRefList;
    plTextFont*             fTextFontRefList;
    plDXRenderTargetRef*    fRenderTargetRefList;
    plDXVertexShader*       fVShaderRefList;
    plDXPixelShader*        fPShaderRefList;

    hsGMaterial*            fCurrMaterial;
    plLayerInterface*       fCurrLay;
    uint32_t                  fCurrLayerIdx, fCurrNumLayers, fCurrRenderLayer;
    uint32_t                  fCurrLightingMethod;    // Based on plSpan flags

    D3DCULL                 fCurrCullMode;
    hsGMatState                 fMatOverOn;
    hsGMatState                 fMatOverOff;
    hsTArray<hsGMaterial*>      fOverrideMat;
    hsGMaterial*                fHoldMat;
    bool                        fCurrD3DLiteState;

    hsMatrix44                  fBumpDuMatrix;
    hsMatrix44                  fBumpDvMatrix;
    hsMatrix44                  fBumpDwMatrix;

    hsTArray<plLayerInterface*>         fOverLayerStack;
    plLayerInterface*                   fOverBaseLayer;
    plLayerInterface*                   fOverAllLayer;
    hsTArray<plLayerInterface*>         fPiggyBackStack;
    int32_t                               fMatPiggyBacks;
    int32_t                               fActivePiggyBacks;

    UINT                    fCurrentAdapter;
    D3DEnum_DriverInfo*     fCurrentDriver;
    D3DEnum_DeviceInfo*     fCurrentDevice;
    D3DEnum_ModeInfo*       fCurrentMode;

    hsGDeviceRef*   fLayerRef[ 8 ];
    hsGMatState     fLayerState[ 8 ]; // base stage (0) state is held in base class
    hsGMatState     fOldLayerState[ 8 ];
    bool            fLayerTransform[ 8 ];
    float           fLayerLODBias[ 8 ];
    uint32_t        fLayerUVWSrcs[ 8 ];
    uint32_t        fLayerXformFlags[ 8 ];
    uint32_t        fLastEndingStage;
    bool            fTexturing;
    bool            fForceMatHandle;

    uint32_t          fInSceneDepth;
    uint32_t          fTextUseTime;       // inc'd every frame - stat gather only
    static uint32_t   fTexManaged;
    static uint32_t   fTexUsed;
    static uint32_t   fVtxManaged;
    static uint32_t   fVtxUsed;
    uint32_t          fEvictTime;
    uint32_t          fManagedSeen;
    uint32_t          fManagedCutoff;

    double            fTime;              // World time.
    uint32_t          fFrame;             // inc'd every time the camera moves.
    uint32_t          fRenderCnt;         // inc'd every begin scene.

    // View stuff
    plDXViewSettings            fView;

    hsBitVector     fDebugFlags;
    uint32_t          fDebugSpanGraphY;

    // Fog
    plDXFogSettings fCurrFog;

    // Light
    plDXLightSettings   fLights;

    // Shadows
    hsTArray<plShadowSlave*>        fShadows;
    hsTArray<plRenderTarget*>       fRenderTargetPool512;
    hsTArray<plRenderTarget*>       fRenderTargetPool256;
    hsTArray<plRenderTarget*>       fRenderTargetPool128;
    hsTArray<plRenderTarget*>       fRenderTargetPool64;
    hsTArray<plRenderTarget*>       fRenderTargetPool32;
    enum { kMaxRenderTargetNext = 10 };
    uint32_t                          fRenderTargetNext[kMaxRenderTargetNext];
    plDXTextureRef*                 fULutTextureRef;
    plRenderTarget*                 fBlurScratchRTs[kMaxRenderTargetNext];
    plRenderTarget*                 fBlurDestRTs[kMaxRenderTargetNext];
    IDirect3DVertexBuffer9*         fBlurVBuffers[kMaxRenderTargetNext];
    uint32_t                          fBlurVSHandle;

    hsTArray<plClothingOutfit*>     fClothingOutfits;
    hsTArray<plClothingOutfit*>     fPrevClothingOutfits;

    // Debug stuff
    plDrawableSpans *fBoundsSpans;
    hsGMaterial     *fBoundsMat;
    hsTArray<uint32_t>    fBSpansToDelete;

    plStatusLogDrawer   *fLogDrawer;

    bool            fVSync;
    bool            fForceDeviceReset;

    void            IBeginAllocUnManaged();
    void            IEndAllocUnManaged();
    void            ICheckTextureUsage();
    void            ICheckVtxUsage();
    inline void     ICheckVBUsage(plDXVertexBufferRef* vRef);

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
    void        IGetVisibleSpans( plDrawableSpans* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr );
    void        IRenderSpans( plDrawableSpans *ice, const hsTArray<int16_t>& visList );
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
    void            IMakeLightLists(plVisMgr* visMgr);
    void            ICheckLighting(plDrawableSpans* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr);
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

    bool            IAvatarSort(plDrawableSpans* d, const hsTArray<int16_t>& visList);
    void            IBlendVertsIntoBuffer( plSpan* span, 
                                            hsMatrix44* matrixPalette, int numMatrices,
                                            const uint8_t *src, uint8_t format, uint32_t srcStride, 
                                            uint8_t *dest, uint32_t destStride, uint32_t count, uint16_t localUVWChans )
                                                { blend_vert_buffer.call(span, matrixPalette, numMatrices, src, format, srcStride, dest, destStride, count, localUVWChans); };
    bool            ISoftwareVertexBlend( plDrawableSpans* drawable, const hsTArray<int16_t>& visList );


    void            ILinkDevRef( plDXDeviceRef *ref, plDXDeviceRef **refList );
    void            IUnlinkDevRef( plDXDeviceRef *ref );

    // Properties
    inline DWORD            inlGetD3DColor( const hsColorRGBA &c ) const;
    inline D3DCOLORVALUE    inlPlToD3DColor(const hsColorRGBA& c, float a) const;

    // Error handling
    void    IAddErrorMessage( char *errStr );
    void    ISetErrorMessage( char *errStr = nil );
    void    IGetD3DError();
    void    IShowErrorMessage( char *errStr = nil );
    bool    ICreateFail( char *errStr );

    // FPU mode check
    void    IFPUCheck();

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
    void        ISetViewport();
    void        IUpdateViewVectors() const;
    void        IRefreshCullTree();
    void        ISetAnisotropy(bool on);

    // Transforms
    D3DXMATRIX&     IMatrix44ToD3DMatrix( D3DXMATRIX& dst, const hsMatrix44& src );
    void            ITransformsToD3D();
    hsMatrix44      IGetCameraToNDC();
    void            IProjectionMatrixToD3D();
    void            IWorldToCameraToD3D();
    void            ILocalToWorldToD3D();
    void            ISavageYonHack();
    void            ISetLocalToWorld( const hsMatrix44& l2w, const hsMatrix44& w2l );
    void            ISetCullMode(bool flip=false);
    bool inline   IIsViewLeftHanded();
    bool            IGetClearViewPort(D3DRECT& r);
    plViewTransform& IGetViewTransform() { return fView.fTransform; }
    void            IUpdateViewFlags();
    void            ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W);

    // Plate management
    friend class plDXPlateManager;

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

    // Selection
    void    IAttachSlaveToReceivers(int iSlave, plDrawableSpans* drawable, const hsTArray<int16_t>& visList);
    void    IAttachShadowsToReceivers(plDrawableSpans* drawable, const hsTArray<int16_t>& visList);
    bool    IAcceptsShadow(const plSpan* span, plShadowSlave* slave);
    bool    IReceivesShadows(const plSpan* span, hsGMaterial* mat);
    void    ISetShadowFromGroup(plDrawableSpans* drawable, const plSpan* span, plLightInfo* liInfo);

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
    double                      fAvRTShrinkValidSince;
    hsTArray<plRenderTarget*>   fAvRTPool;
    uint16_t                      fAvRTWidth;
    uint32_t                      fAvNextFreeRT;
    void                    IFillAvRTPool();
    bool                    IFillAvRTPool(uint16_t numRTs, uint16_t width); // Returns true if we successfully filled the pool. Otherwise cleans up.
    void                    IReleaseAvRTPool();
    plRenderTarget*         IGetNextAvRT();
    void                    IFreeAvRT(plRenderTarget* tex);
    void                    IPreprocessAvatarTextures();
    void                    IDrawClothingQuad(float x, float y, float w, float h, float uOff, float vOff, plMipmap *tex);
    void                    IClearClothingOutfits(hsTArray<plClothingOutfit*>* outfits);

    void IPrintDeviceInitError();

    void IResetToDefaults(D3DPRESENT_PARAMETERS *params);

public:
    plDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode );
    virtual ~plDXPipeline();

    CLASSNAME_REGISTER( plDXPipeline );
    GETINTERFACE_ANY( plDXPipeline, plPipeline );

    virtual IDirect3DDevice9*           GetD3DDevice() const { return fD3DDevice; }

    // Typical 3D device
    virtual bool                        PreRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nil);
    virtual bool                        PrepForRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nil);
    virtual void                        Render(plDrawable* d, const hsTArray<int16_t>& visList);
    virtual void                        Draw(plDrawable* d);
    
    virtual void                        PushRenderRequest(plRenderRequest* req);
    virtual void                        PopRenderRequest(plRenderRequest* req);

    void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool VSync = false );

    virtual void                        ClearRenderTarget( plDrawable* d );
    virtual void                        ClearRenderTarget( const hsColorRGBA* col = nil, const float* depth = nil );
    virtual void                        SetClear(const hsColorRGBA* col=nil, const float* depth=nil);
    virtual hsColorRGBA                 GetClearColor() const;
    virtual float                    GetClearDepth() const;
    virtual hsGDeviceRef*               MakeRenderTargetRef( plRenderTarget *owner );
    virtual hsGDeviceRef*               SharedRenderTargetRef(plRenderTarget* sharer, plRenderTarget *owner);
    virtual void                        PushRenderTarget( plRenderTarget *target );
    virtual plRenderTarget*             PopRenderTarget();

    virtual bool                        BeginRender();
    virtual bool                        EndRender();
    virtual void                        RenderScreenElements();

    virtual bool                        BeginDrawable(plDrawable* d);
    virtual bool                        EndDrawable(plDrawable* d);

    virtual void                        BeginVisMgr(plVisMgr* visMgr);
    virtual void                        EndVisMgr(plVisMgr* visMgr);

    virtual bool                        IsFullScreen() const { return fSettings.fFullscreen; }
    virtual uint32_t                      Width() const { return fView.fTransform.GetViewPortWidth(); }
    virtual uint32_t                      Height() const { return fView.fTransform.GetViewPortHeight(); }
    virtual uint32_t                      ColorDepth() const { return fSettings.fColorDepth; }
    virtual void                        Resize( uint32_t width, uint32_t height );

    // Culling. Might be used in Update before bothering to do any serious computation.
    virtual bool                        TestVisibleWorld(const hsBounds3Ext& wBnd);
    virtual bool                        TestVisibleWorld(const plSceneObject* sObj);
    virtual bool                        HarvestVisible(plSpaceTree* space, hsTArray<int16_t>& visList);
    virtual bool                        SubmitOccluders(const hsTArray<const plCullPoly*>& polyList);

    // Debug flags
    virtual void                        SetDebugFlag( uint32_t flag, bool on );
    virtual bool                        IsDebugFlagSet( uint32_t flag ) const;

    // These are also only for debugging.
    virtual void                        SetMaxCullNodes(uint16_t n) { fView.fCullMaxNodes = n; }
    virtual uint16_t                      GetMaxCullNodes() const { return fView.fCullMaxNodes; }

    virtual bool                        CheckResources();
    virtual void                        LoadResources();    // Tells us where it's a good time to load in unmanaged resources.

    // Properties
    virtual void                        SetProperty( uint32_t prop, bool on ) { on ? fSettings.fProperties |= prop : fSettings.fProperties &= ~prop; }
    virtual bool                        GetProperty( uint32_t prop ) const { return ( fSettings.fProperties & prop ) ? true : false; }

    virtual uint32_t                      GetMaxLayersAtOnce() const { return fSettings.fMaxLayersAtOnce; }

    // Drawable type mask
    virtual void                        SetDrawableTypeMask( uint32_t mask ) { fView.fDrawableTypeMask = mask; }
    virtual uint32_t                      GetDrawableTypeMask() const { return fView.fDrawableTypeMask; }
    virtual void                        SetSubDrawableTypeMask( uint32_t mask ) { fView.fSubDrawableTypeMask = mask; }
    virtual uint32_t                      GetSubDrawableTypeMask() const { return fView.fSubDrawableTypeMask; }

    // Create a debug text font object
    virtual plTextFont      *MakeTextFont( char *face, uint16_t size );

    // Create and/or Refresh geometry buffers
    virtual void            CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx);
    virtual void            CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx);

    virtual bool            OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly);
    virtual bool            CloseAccess(plAccessSpan& acc);

    virtual void            CheckTextureRef(plLayerInterface* lay);     
    static void             FreeManagedTexture(uint32_t sz) { hsAssert(fTexManaged >= sz, "Freeing mem we don't have"); fTexManaged -= sz; }
    static void             AllocManagedTexture(uint32_t sz) { fTexManaged += sz; }
    static void             FreeManagedVertex(uint32_t sz) { hsAssert(fVtxManaged >= sz, "Freeing mem we don't have"); fVtxManaged -= sz; }
    static void             AllocManagedVertex(uint32_t sz) { fVtxManaged += sz; }

#ifndef PLASMA_EXTERNAL_RELEASE
    static void ProfilePoolMem(D3DPOOL poolType, uint32_t size, bool add, const char *id);
#endif // PLASMA_EXTERNAL_RELEASE

    //  From a D3DFORMAT enumeration, return the bit depth associated with it.
    static short    GetDXBitDepth( D3DFORMAT format );

    // Default fog settings
    virtual void                        SetDefaultFogEnviron( plFogEnvironment *fog ) { fView.fDefaultFog = *fog; fCurrFog.fEnvPtr = nil; }
    virtual const plFogEnvironment      &GetDefaultFogEnviron() const { return fView.fDefaultFog; }

    // View state
    virtual hsPoint3                    GetViewPositionWorld() const { return GetViewTransform().GetPosition(); }
    virtual hsVector3                   GetViewAcrossWorld() const { return GetViewTransform().GetAcross(); }
    virtual hsVector3                   GetViewUpWorld() const { return GetViewTransform().GetUp(); }
    virtual hsVector3                   GetViewDirWorld() const { return GetViewTransform().GetDirection(); }
    virtual void                        GetViewAxesWorld(hsVector3 axes[3] /* ac,up,at */ ) const;

    virtual void                        GetFOV(float& fovX, float& fovY) const;
    virtual void                        SetFOV(float fovX, float fovY);

    virtual void                        GetSize(float& width, float& height) const;
    virtual void                        SetSize(float width, float height);

    virtual void                        GetDepth(float& hither, float& yon) const;
    virtual void                        SetDepth(float hither, float yon);

    virtual float                       GetZBiasScale() const;
    virtual void                        SetZBiasScale(float scale);

    virtual const hsMatrix44&           GetWorldToCamera() const;
    virtual const hsMatrix44&           GetCameraToWorld() const;
    virtual void                        SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w);

    virtual void                        SetViewTransform(const plViewTransform& trans);
    virtual const plViewTransform&      GetViewTransform() const { return fView.fTransform; }

    virtual const hsMatrix44&           GetWorldToLocal() const;
    virtual const hsMatrix44&           GetLocalToWorld() const;

    virtual void                        ScreenToWorldPoint( int n, uint32_t stride, int32_t *scrX, int32_t *scrY, 
                                                    float dist, uint32_t strideOut, hsPoint3 *worldOut );
    
    virtual void                        RefreshMatrices();
    virtual void                        RefreshScreenMatrices();

    virtual void                        RegisterLight(plLightInfo* light);
    virtual void                        UnRegisterLight(plLightInfo* light);

    // Overrides, always push returns whatever is necessary to restore on pop.
    virtual hsGMaterial*                PushOverrideMaterial(hsGMaterial* mat);
    virtual void                        PopOverrideMaterial(hsGMaterial* restore);
    virtual hsGMaterial*                GetOverrideMaterial() const;

    virtual plLayerInterface*           AppendLayerInterface(plLayerInterface* li, bool onAllLayers = false);
    virtual plLayerInterface*           RemoveLayerInterface(plLayerInterface* li, bool onAllLayers = false);

    virtual plLayerInterface*           PushPiggyBackLayer(plLayerInterface* li);
    virtual plLayerInterface*           PopPiggyBackLayer(plLayerInterface* li);

    virtual uint32_t                      GetMaterialOverrideOn(hsGMatState::StateIdx category) const;
    virtual uint32_t                      GetMaterialOverrideOff(hsGMatState::StateIdx category) const;

    virtual hsGMatState                 PushMaterialOverride(const hsGMatState& state, bool on);
    virtual hsGMatState                 PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on);
    virtual void                        PopMaterialOverride(const hsGMatState& restore, bool on);
    virtual const hsGMatState&          GetMaterialOverride(bool on) const;

    virtual hsColorOverride             PushColorOverride(const hsColorOverride& over);
    virtual void                        PopColorOverride(const hsColorOverride& restore);
    virtual const hsColorOverride&      GetColorOverride() const;

    virtual void                        SubmitShadowSlave(plShadowSlave* slave);
    virtual void                        SubmitClothingOutfit(plClothingOutfit* co);

    virtual bool                        SetGamma(float eR, float eG, float eB);
    virtual bool                        SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB);

    virtual bool                        CaptureScreen( plMipmap *dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0 );
    virtual plMipmap*                   ExtractMipMap(plRenderTarget* targ);

    /// Error handling
    virtual const char                  *GetErrorString();

    bool                                ManagedAlloced() const { return fManagedAlloced; }

    virtual void                        GetSupportedColorDepths(hsTArray<int> &ColorDepths);
    virtual void                        GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 );
    virtual int                         GetMaxAnisotropicSamples();
    virtual int                         GetMaxAntiAlias(int Width, int Height, int ColorDepth);


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
