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

#include "../plSurface/plLayerInterface.h"
#include "hsMatrix44.h"
#include "plFogEnvironment.h"
#include "hsG3DDeviceSelector.h"
#include "hsGeometry3.h"
#include "hsTemplates.h"
#include "hsColorRGBA.h"
#include "hsGDeviceRef.h"
#include "hsPoint2.h"

class plAccessSpan;
class plAuxSpan;
class plVertexSpan;

#include "plPlates.h"	// Used to define plDXPlateManager


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
#define PROFILE_POOL_MEM(pool, size, add, id)		plDXPipeline::ProfilePoolMem(pool, size, add, id);
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

#define  ReleaseObject(x)	if(x){ plReleaseObject(x); x=NULL; }

typedef LPDIRECT3D9 (WINAPI * Direct3DCreateProc)( UINT sdkVersion );


//// Helper Classes ///////////////////////////////////////////////////////////

//// The RenderPrimFunc lets you have one function which does a lot of stuff
// around the actual call to render whatever type of primitives you have, instead
// of duplicating everything because the one line to render is different.
class plRenderPrimFunc
{
public:
	virtual hsBool RenderPrims() const = 0; // return true on error
};

//// DX-specific Plate Manager implementation
class plDXPlateManager : public plPlateManager
{
	friend class plDXPipeline;

	public:

		virtual ~plDXPlateManager();

	protected:

		const long	PLD3D_PLATEFVF;

		struct plPlateVertex
		{
			hsPoint3	fPoint;
			UInt32		fColor;
			hsPoint3	fUV;
		};

		IDirect3DDevice9		*fD3DDevice;
		IDirect3DVertexBuffer9	*fVertBuffer;

		plDXPlateManager( plDXPipeline *pipe, IDirect3DDevice9 *device );

		void ICreateGeometry(plDXPipeline* pipe);
		void IReleaseGeometry();

		virtual void	IDrawToDevice( plPipeline *pipe );
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
class plBinkPlayer;

class plDXPipeline : public plPipeline
{
protected:
	enum {
		kCapsNone				= 0x0,
		kCapsCompressTextures	= 0x1,
		kCapsMipmap				= 0x2,
		kCapsHWTransform		= 0x4,
		kCapsHWLighting			= 0x8,
		kCapsZBias				= 0x10,
		kCapsLinearFog			= 0x20,
		kCapsExpFog				= 0x40,
		kCapsExp2Fog			= 0x80,
		kCapsRangeFog			= 0x100,
		kCapsWBuffer			= 0x200,
		kCapsTexBoundToStage	= 0x400,
		kCapsDither				= 0x800,
		kCapsLODWatch			= 0x1000,
		kCapsFSAntiAlias		= 0x2000,
		kCapsLuminanceTextures	= 0x4000,
		kCapsDoesSmallTextures	= 0x8000,
		kCapsDoesWFog			= 0x10000,
		kCapsPixelFog			= 0x20000,
		kCapsHasBadYonStuff		= 0x40000,
		kCapsNoKindaSmallTexs	= 0x80000,

		kCapsCubicTextures		= 0x200000,
		kCapsCubicMipmap		= 0x400000
	};
	enum {
		kKNone					= 0x0,
		kKTNT					= 0x1
	};

	plDebugTextManager*			fDebugTextMgr;
	plDXPlateManager*			fPlateMgr;

	// The main D3D interfaces
    LPDIRECT3D9				fD3DObject;		// The main D3D object
    LPDIRECT3DDEVICE9		fD3DDevice;		// The D3D rendering device
	IDirect3DSurface9*		fD3DMainSurface;
	IDirect3DSurface9*		fD3DDepthSurface;
	IDirect3DSurface9*		fD3DBackBuff;

	IDirect3DSurface9*		fSharedDepthSurface[2];
	D3DFORMAT				fSharedDepthFormat[2];

	// Dynamic buffers
	UInt32					fVtxRefTime;
	UInt32					fNextDynVtx;
	UInt32					fDynVtxSize;
	IDirect3DVertexBuffer9*	fDynVtxBuff;
	hsBool					fManagedAlloced;
	hsBool					fAllocUnManaged;


	// States
	plDXGeneralSettings		fSettings;
	plDXTweakSettings		fTweaks;
	plDXStencilSettings		fStencil;
	hsBool					fDeviceLost;
	hsBool					fDevWasLost;

	hsTArray<const plCullPoly*>	fCullPolys;
	hsTArray<const plCullPoly*>	fCullHoles;
	plDrawableSpans*			fCullProxy;
	
	plDXVertexBufferRef*	fVtxBuffRefList;
	plDXIndexBufferRef*		fIdxBuffRefList;
	plDXTextureRef*			fTextureRefList;
	plTextFont*				fTextFontRefList;
	plDXRenderTargetRef*	fRenderTargetRefList;
	plDXVertexShader*		fVShaderRefList;
	plDXPixelShader*		fPShaderRefList;

	hsGMaterial*			fCurrMaterial;
	plLayerInterface*		fCurrLay;
	UInt32					fCurrLayerIdx, fCurrNumLayers, fCurrRenderLayer;
	UInt32					fCurrLightingMethod;	// Based on plSpan flags

	D3DCULL					fCurrCullMode;
	hsGMatState					fMatOverOn;
	hsGMatState					fMatOverOff;
	hsTArray<hsGMaterial*>		fOverrideMat;
	hsGMaterial*				fHoldMat;
	hsBool						fCurrD3DLiteState;

	hsMatrix44					fBumpDuMatrix;
	hsMatrix44					fBumpDvMatrix;
	hsMatrix44					fBumpDwMatrix;

	hsTArray<plLayerInterface*>			fOverLayerStack;
	plLayerInterface*					fOverBaseLayer;
	plLayerInterface*					fOverAllLayer;
	hsTArray<plLayerInterface*>			fPiggyBackStack;
	Int32								fMatPiggyBacks;
	Int32								fActivePiggyBacks;

	UINT					fCurrentAdapter;
	D3DEnum_DriverInfo*		fCurrentDriver;
	D3DEnum_DeviceInfo*		fCurrentDevice;
	D3DEnum_ModeInfo*		fCurrentMode;

	hsGDeviceRef*	fLayerRef[ 8 ];
	hsGMatState		fLayerState[ 8 ]; // base stage (0) state is held in base class
	hsGMatState		fOldLayerState[ 8 ];
	hsBool			fLayerTransform[ 8 ];
	float			fLayerLODBias[ 8 ];
	UInt32			fLayerUVWSrcs[ 8 ];
	UInt32			fLayerXformFlags[ 8 ];
	UInt32			fLastEndingStage;
	hsBool			fTexturing;
	hsBool			fForceMatHandle;

	UInt32			fInSceneDepth;
	UInt32			fTextUseTime;		// inc'd every frame - stat gather only
	static UInt32	fTexManaged;
	static UInt32	fTexUsed;
	static UInt32	fVtxManaged;
	static UInt32	fVtxUsed;
	UInt32			fEvictTime;
	UInt32			fManagedSeen;
	UInt32			fManagedCutoff;

	double			fTime;				// World time.
	UInt32			fFrame;				// inc'd every time the camera moves.
	UInt32			fRenderCnt;			// inc'd every begin scene.

	// View stuff
	plDXViewSettings			fView;

	hsBitVector		fDebugFlags;
	UInt32			fDebugSpanGraphY;

	// Fog
	plDXFogSettings	fCurrFog;

	// Light
	plDXLightSettings	fLights;

	// Shadows
	hsTArray<plShadowSlave*>		fShadows;
	hsTArray<plRenderTarget*>		fRenderTargetPool512;
	hsTArray<plRenderTarget*>		fRenderTargetPool256;
	hsTArray<plRenderTarget*>		fRenderTargetPool128;
	hsTArray<plRenderTarget*>		fRenderTargetPool64;
	hsTArray<plRenderTarget*>		fRenderTargetPool32;
	enum { kMaxRenderTargetNext = 10 };
	UInt32							fRenderTargetNext[kMaxRenderTargetNext];
	plDXTextureRef*					fULutTextureRef;
	plRenderTarget*					fBlurScratchRTs[kMaxRenderTargetNext];
	plRenderTarget*					fBlurDestRTs[kMaxRenderTargetNext];
	IDirect3DVertexBuffer9*			fBlurVBuffers[kMaxRenderTargetNext];
	UInt32							fBlurVSHandle;

	hsTArray<plClothingOutfit*>		fClothingOutfits;
	hsTArray<plClothingOutfit*>		fPrevClothingOutfits;

	// Debug stuff
	plDrawableSpans	*fBoundsSpans;
	hsGMaterial		*fBoundsMat;
	hsTArray<UInt32>	fBSpansToDelete;

	plStatusLogDrawer	*fLogDrawer;

	hsBool				fVSync;
	hsBool				fForceDeviceReset;

	void			IBeginAllocUnManaged();
	void			IEndAllocUnManaged();
	void			ICheckTextureUsage();
	void			ICheckVtxUsage();
	inline void		ICheckVBUsage(plDXVertexBufferRef* vRef);

	hsBool			IRefreshDynVertices(plGBufferGroup* group, plDXVertexBufferRef* vRef);
	hsBool			ICheckAuxBuffers(const plAuxSpan* span);
	hsBool			ICheckDynBuffers(plDrawableSpans* drawable, plGBufferGroup* group, const plSpan* span);
	void			ICheckStaticVertexBuffer(plDXVertexBufferRef* vRef, plGBufferGroup* owner, UInt32 idx);
	void			ICheckIndexBuffer(plDXIndexBufferRef* iRef);
	void			IFillStaticVertexBufferRef(plDXVertexBufferRef *ref, plGBufferGroup *group, UInt32 idx);
	void			IFillIndexBufferRef(plDXIndexBufferRef* iRef, plGBufferGroup* owner, UInt32 idx);
	void			ISetupVertexBufferRef(plGBufferGroup* owner, UInt32 idx, plDXVertexBufferRef* vRef);
	void			ISetupIndexBufferRef(plGBufferGroup* owner, UInt32 idx, plDXIndexBufferRef* iRef);
	void			ICreateDynamicBuffers();
	void			IReleaseDynamicBuffers();

	void			IAddBoundsSpan( plDrawableSpans *ice, const hsBounds3Ext *bounds, UInt32 bndColor = 0xffff0000 );
	void			IAddNormalsSpan( plDrawableSpans *ice, plIcicle *span, plDXVertexBufferRef *vRef, UInt32 bndColor );

	// Rendering
	hsBool		IFlipSurface();
	long		IGetBufferD3DFormat(UInt8 format) const;
	UInt32		IGetBufferFormatSize(UInt8 format) const;
	void		IGetVisibleSpans( plDrawableSpans* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr );
	void		IRenderSpans( plDrawableSpans *ice, const hsTArray<Int16>& visList );
	hsBool		ILoopOverLayers(const plRenderPrimFunc& render, hsGMaterial* material, const plSpan& span);
	void		IRenderBufferSpan( const plIcicle& span, 
									hsGDeviceRef *vb, hsGDeviceRef *ib, 
									hsGMaterial *material, 
									UInt32 vStart, UInt32 vLength, UInt32 iStart, UInt32 iLength );
	void		IRenderAuxSpan(const plSpan& span, const plAuxSpan* aux);
	void		IRenderAuxSpans(const plSpan& span);

	// Fog
	void		IGetVSFogSet(float* const set) const;
	void		ISetFogParameters(const plSpan* span, const plLayerInterface* baseLay);

	// Lighting
	hsGDeviceRef	*IMakeLightRef( plLightInfo *owner );
	void			IScaleD3DLight( plDXLightRef *ref, hsScalar scale);
	void			ICalcLighting( const plLayerInterface *currLayer, const plSpan *currSpan );
	void			IDisableSpanLights();
	void			IRestoreSpanLights();
	void			ISelectLights( plSpan *span, int numLights, hsBool proj );
	void			IEnableLights( plSpan *span );
	void			IMakeLightLists(plVisMgr* visMgr);
	void			ICheckLighting(plDrawableSpans* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr);
	inline void		inlEnsureLightingOff();
	inline void		inlEnsureLightingOn();
	void			IRenderProjection(const plRenderPrimFunc& render, plLightInfo* li);
	void			IRenderProjections(const plRenderPrimFunc& render);
	void			IRenderProjectionEach(const plRenderPrimFunc& render, hsGMaterial* material, int iPass, const plSpan& span);
	void			IRenderOverWire(const plRenderPrimFunc& render, hsGMaterial* material, const plSpan& span);

	hsBool					ISkipBumpMap(hsGMaterial* newMat, UInt32& layer, const plSpan* currSpan) const;
	void					ISetBumpMatrices(const plLayerInterface* layer, const plSpan* span);
	const hsMatrix44&		IGetBumpMatrix(UInt32 miscFlags) const;

	// Materials
	const hsGMatState&	ICompositeLayerState(int which, plLayerInterface* layer);
	Int32		IHandleMaterial(hsGMaterial* newMat, UInt32 which, const plSpan* currSpan);
	void		IHandleFirstTextureStage( plLayerInterface* layer );
	void		IHandleShadeMode();
	void		IHandleZMode();
	void		IHandleMiscMode();
	void		IHandleTextureStage(UInt32 stage, plLayerInterface* layer);
	void		IHandleFirstStageBlend();
	void		IHandleBumpEnv(int stage, UInt32 blendFlags);
	void		IHandleStageBlend(int stage);
	void		IHandleStageClamp(int stage);
	void		IHandleStageTransform(int stage, plLayerInterface* layer);
	void		IHandleTextureMode(plLayerInterface* layer);
	void		IUseTextureRef(int stage, hsGDeviceRef* dRef, plLayerInterface* layer);
	void		IStageStop(UInt32 stage);
	UInt32		ILayersAtOnce(hsGMaterial* mat, UInt32 which);
	hsBool		ICanEatLayer(plLayerInterface* lay);
	void		ISetLayer(UInt32 lay);
	void		IBottomLayer();

	// Push special effects
	plLayerInterface*	IPushOverBaseLayer(plLayerInterface* li);
	plLayerInterface*	IPopOverBaseLayer(plLayerInterface* li);
	plLayerInterface*	IPushOverAllLayer(plLayerInterface* li);
	plLayerInterface*	IPopOverAllLayer(plLayerInterface* li);

	int					ISetNumActivePiggyBacks();
	void				IPushPiggyBacks(hsGMaterial* mat);
	void				IPopPiggyBacks();
	void				IPushProjPiggyBack(plLayerInterface* li);
	void				IPopProjPiggyBacks();

	void				ISetPipeConsts(plShader* shader);
	HRESULT				ISetShaders(plShader* vShader, plShader* pShader);

	// Stenciling
	virtual hsBool			StencilEnable( hsBool enable );
	virtual void			StencilSetCompareFunc( UInt8 func, UInt32 refValue );
	virtual void			StencilSetMask( UInt32 mask, UInt32 writeMask );
	virtual void			StencilSetOps( UInt8 passOp, UInt8 failOp, UInt8 passButZFailOp );
	virtual hsBool			StencilGetCaps( plStencilCaps *caps );

	hsGDeviceRef	*MakeTextureRef( plLayerInterface* layer, plMipmap *b );
	void			IReloadTexture( plDXTextureRef *ref );
	void			IFillD3DTexture( plDXTextureRef *ref );
	void			IFillD3DCubeTexture( plDXCubeTextureRef *ref );
	void			IGetD3DTextureFormat( plBitmap *b, D3DFORMAT &formatType, UInt32& texSize );
	void			IFormatTextureData( UInt32 formatType, UInt32 numPix, hsRGBAColor32* const src, void *dst );
	void			*IGetPixelScratch( UInt32 size );
	hsGDeviceRef	*IMakeCubicTextureRef( plLayerInterface* layer, plCubicEnvironmap *cubic );
	hsBool			IProcessMipmapLevels( plMipmap *mipmap, UInt32 &numLevels,
												UInt32 *&levelSizes, UInt32 &totalSize, 
												UInt32 &numPixels, void *&textureData, hsBool noMip );
	IDirect3DTexture9		*IMakeD3DTexture( plDXTextureRef *ref, D3DFORMAT formatType );
	IDirect3DCubeTexture9	*IMakeD3DCubeTexture( plDXTextureRef *ref, D3DFORMAT formatType );

	// Visualization of active occluders
	void			IMakeOcclusionSnap();

	hsBool			IAvatarSort(plDrawableSpans* d, const hsTArray<Int16>& visList);
	void			IBlendVertsIntoBuffer( plSpan* span, 
											hsMatrix44* matrixPalette, int numMatrices,
											UInt8 *src, UInt8 format, UInt32 srcStride, 
											UInt8 *dest, UInt32 destStride, UInt32 count, UInt16 localUVWChans );
	hsBool			ISoftwareVertexBlend( plDrawableSpans* drawable, const hsTArray<Int16>& visList );


	void			ILinkDevRef( plDXDeviceRef *ref, plDXDeviceRef **refList );
	void			IUnlinkDevRef( plDXDeviceRef *ref );

	// Properties
	inline DWORD			inlGetD3DColor( const hsColorRGBA &c ) const;
	inline D3DCOLORVALUE	inlPlToD3DColor(const hsColorRGBA& c, float a) const;

	// Error handling
	void	IAddErrorMessage( char *errStr );
	void	ISetErrorMessage( char *errStr = nil );
	void	IGetD3DError();
	void	IShowErrorMessage( char *errStr = nil );
	hsBool	ICreateFail( char *errStr );

	// FPU mode check
	void	IFPUCheck();

	// Device initialization
	void	IInvalidateState();
	void	IInitDeviceState();
	void	IClearMembers();
	void	ISetCaps();
	void	IRestrictCaps( const hsG3DDeviceRecord& devRec );
	void	ISetGraphicsCapability(UInt32 v);

	hsBool	IFindDepthFormat(D3DPRESENT_PARAMETERS& params);
	hsBool	IFindCompressedFormats();
	hsBool	IFindLuminanceFormats();
	hsBool	ITextureFormatAllowed( D3DFORMAT format );

	void	ISetCurrentDriver( D3DEnum_DriverInfo *driv );
	void	ISetCurrentDevice( D3DEnum_DeviceInfo *dev );
	void	ISetCurrentMode( D3DEnum_ModeInfo *mode );

	hsBool		ICreateMaster();
	hsBool		ICreateDevice(hsBool windowed);
	hsBool		ICreateNormalSurfaces();

	hsBool		ICreateDeviceObjects();
	void		IReleaseDeviceObjects();

	hsBool		ICreateDynDeviceObjects();
	void		IReleaseDynDeviceObjects();
	void		IReleaseShaders();

	hsBool		IResetDevice();

	// View and clipping
	void		ISetViewport();
	void		IUpdateViewVectors() const;
	void		IRefreshCullTree();
	void		ISetAnisotropy(hsBool on);

	// Transforms
	D3DXMATRIX&		IMatrix44ToD3DMatrix( D3DXMATRIX& dst, const hsMatrix44& src );
	void			ITransformsToD3D();
	hsMatrix44		IGetCameraToNDC();
	void			IProjectionMatrixToD3D();
	void			IWorldToCameraToD3D();
	void			ILocalToWorldToD3D();
	void			ISavageYonHack();
	void			ISetLocalToWorld( const hsMatrix44& l2w, const hsMatrix44& w2l );
	void			ISetCullMode(hsBool flip=false);
	hsBool inline	IIsViewLeftHanded();
	hsBool			IGetClearViewPort(D3DRECT& r);
	plViewTransform& IGetViewTransform() { return fView.fTransform; }
	void			IUpdateViewFlags();
	void			ISetupTransforms(plDrawableSpans* drawable, const plSpan& span, hsMatrix44& lastL2W);

	// Plate management
	friend plDXPlateManager;
	friend plBinkPlayer;

	void		IDrawPlate( plPlate *plate );

	void	ISetRenderTarget( plRenderTarget *target );

	hsBool	IPrepRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat,
									D3DFORMAT &depthFormat, D3DRESOURCETYPE &resType );
	hsBool	IFindRenderTargetInfo( plRenderTarget *owner, D3DFORMAT &surfFormat, D3DRESOURCETYPE &resType );

	// From a D3DFORMAT enumeration, return the string literal for it
	static const char	*IGetDXFormatName( D3DFORMAT format );

	/////// Shadow internals
	// Generation
	void	IClearShadowSlaves();
	void	IPreprocessShadows();
	hsBool	IPrepShadowCaster(const plShadowCaster* caster);
	void	IRenderShadowCasterSpan(plShadowSlave* slave, plDrawableSpans* drawable, const plIcicle& span);
	void	ISetupShadowCastTextureStages(plShadowSlave* slave);
	hsBool	IRenderShadowCaster(plShadowSlave* slave);
	void	ISetupShadowLight(plShadowSlave* slave);
	plDXLightRef*	INextShadowLight(plShadowSlave* slave);

	hsBool	IPushShadowCastState(plShadowSlave* slave);
	hsBool	IPopShadowCastState(plShadowSlave* slave);

	plDXTextureRef*	IGetULutTextureRef();
	hsBool				ICreateBlurVBuffers();
	void				IReleaseBlurVBuffers();
	void				IMakeRenderTargetPools();
	void				IResetRenderTargetPools();
	plRenderTarget*		IFindRenderTarget(UInt32& w, UInt32& h, hsBool ortho);
	void				IReleaseRenderTargetPools();

	// Selection
	void	IAttachSlaveToReceivers(int iSlave, plDrawableSpans* drawable, const hsTArray<Int16>& visList);
	void	IAttachShadowsToReceivers(plDrawableSpans* drawable, const hsTArray<Int16>& visList);
	hsBool	IAcceptsShadow(const plSpan* span, plShadowSlave* slave);
	hsBool	IReceivesShadows(const plSpan* span, hsGMaterial* mat);
	void	ISetShadowFromGroup(plDrawableSpans* drawable, const plSpan* span, plLightInfo* liInfo);

	// Application	
	void	IRenderShadowsOntoSpan(const plRenderPrimFunc& render, const plSpan* span, hsGMaterial* mat);
	void	ISetupShadowRcvTextureStages(hsGMaterial* mat);
	void	ISetShadowLightState(hsGMaterial* mat);
	void	IDisableLightsForShadow();
	void	IEnableShadowLight(plShadowSlave* slave);
	void	ISetupShadowSlaveTextures(plShadowSlave* slave);

	// Postprocess (blurring)
	hsBool				ISetBlurQuadToRender(plRenderTarget* smap);
	void				IRenderBlurBackToShadowMap(plRenderTarget* smap, plRenderTarget* scratch, plRenderTarget* dst);
	void				IRenderBlurFromShadowMap(plRenderTarget* scratchRT, plRenderTarget* smap, hsScalar scale);
	void				IBlurSetRenderTarget(plRenderTarget* rt);
	int					IGetScratchRenderTarget(plRenderTarget* smap);
	void				IBlurShadowMap(plShadowSlave* slave);

	// Avatar Texture Rendering
	double						fAvRTShrinkValidSince;
	hsTArray<plRenderTarget*>	fAvRTPool;
	UInt16						fAvRTWidth;
	UInt32						fAvNextFreeRT;
	void					IFillAvRTPool();
	hsBool					IFillAvRTPool(UInt16 numRTs, UInt16 width); // Returns true if we successfully filled the pool. Otherwise cleans up.
	void					IReleaseAvRTPool();
	plRenderTarget*			IGetNextAvRT();
	void					IFreeAvRT(plRenderTarget* tex);
	void					IPreprocessAvatarTextures();
	void					IDrawClothingQuad(hsScalar x, hsScalar y, hsScalar w, hsScalar h, hsScalar uOff, hsScalar vOff, plMipmap *tex);
	void					IClearClothingOutfits(hsTArray<plClothingOutfit*>* outfits);

	void IPrintDeviceInitError();

	void IResetToDefaults(D3DPRESENT_PARAMETERS *params);

public:
	plDXPipeline( hsWinRef hWnd, const hsG3DDeviceModeRecord *devMode );
	virtual ~plDXPipeline();

	CLASSNAME_REGISTER( plDXPipeline );
	GETINTERFACE_ANY( plDXPipeline, plPipeline );

	virtual IDirect3DDevice9*			GetD3DDevice() const { return fD3DDevice; }

	// Typical 3D device
	virtual hsBool						PreRender(plDrawable* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr=nil);
	virtual hsBool						PrepForRender(plDrawable* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr=nil);
	virtual void						Render(plDrawable* d, const hsTArray<Int16>& visList);
	virtual void						Draw(plDrawable* d);
	
	virtual void						PushRenderRequest(plRenderRequest* req);
	virtual void						PopRenderRequest(plRenderRequest* req);

	void ResetDisplayDevice(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool VSync = false );

	virtual void						ClearRenderTarget( plDrawable* d );
	virtual void						ClearRenderTarget( const hsColorRGBA* col = nil, const hsScalar* depth = nil );
	virtual void						SetClear(const hsColorRGBA* col=nil, const hsScalar* depth=nil);
	virtual hsColorRGBA					GetClearColor() const;
	virtual hsScalar					GetClearDepth() const;
	virtual hsGDeviceRef*				MakeRenderTargetRef( plRenderTarget *owner );
	virtual hsGDeviceRef*				SharedRenderTargetRef(plRenderTarget* sharer, plRenderTarget *owner);
	virtual void						PushRenderTarget( plRenderTarget *target );
	virtual plRenderTarget*				PopRenderTarget();

	virtual hsBool						BeginRender();
	virtual hsBool						EndRender();
	virtual void						RenderScreenElements();

	virtual hsBool						BeginDrawable(plDrawable* d);
	virtual hsBool						EndDrawable(plDrawable* d);

	virtual void						BeginVisMgr(plVisMgr* visMgr);
	virtual void						EndVisMgr(plVisMgr* visMgr);

	virtual hsBool						IsFullScreen() const { return fSettings.fFullscreen; }
	virtual UInt32						Width() const { return fView.fTransform.GetViewPortWidth(); }
	virtual UInt32						Height() const { return fView.fTransform.GetViewPortHeight(); }
	virtual UInt32						ColorDepth() const { return fSettings.fColorDepth; }
	virtual void						Resize( UInt32 width, UInt32 height );

	// Culling. Might be used in Update before bothering to do any serious computation.
	virtual hsBool						TestVisibleWorld(const hsBounds3Ext& wBnd);
	virtual hsBool						TestVisibleWorld(const plSceneObject* sObj);
	virtual hsBool						HarvestVisible(plSpaceTree* space, hsTArray<Int16>& visList);
	virtual hsBool						SubmitOccluders(const hsTArray<const plCullPoly*>& polyList);

	// Debug flags
	virtual void						SetDebugFlag( UInt32 flag, hsBool on );
	virtual hsBool						IsDebugFlagSet( UInt32 flag ) const;

	// These are also only for debugging.
	virtual void						SetMaxCullNodes(UInt16 n) { fView.fCullMaxNodes = n; }
	virtual UInt16						GetMaxCullNodes() const { return fView.fCullMaxNodes; }

	virtual hsBool						CheckResources();
	virtual void						LoadResources();	// Tells us where it's a good time to load in unmanaged resources.

	// Properties
	virtual void						SetProperty( UInt32 prop, hsBool on ) { on ? fSettings.fProperties |= prop : fSettings.fProperties &= ~prop; }
	virtual hsBool						GetProperty( UInt32 prop ) const { return ( fSettings.fProperties & prop ) ? true : false; }

	virtual UInt32						GetMaxLayersAtOnce() const { return fSettings.fMaxLayersAtOnce; }

	// Drawable type mask
	virtual void						SetDrawableTypeMask( UInt32 mask ) { fView.fDrawableTypeMask = mask; }
	virtual UInt32						GetDrawableTypeMask() const { return fView.fDrawableTypeMask; }
	virtual void						SetSubDrawableTypeMask( UInt32 mask ) { fView.fSubDrawableTypeMask = mask; }
	virtual UInt32						GetSubDrawableTypeMask() const { return fView.fSubDrawableTypeMask; }

	// Create a debug text font object
	virtual plTextFont		*MakeTextFont( char *face, UInt16 size );

	// Create and/or Refresh geometry buffers
	virtual void			CheckVertexBufferRef(plGBufferGroup* owner, UInt32 idx);
	virtual void			CheckIndexBufferRef(plGBufferGroup* owner, UInt32 idx);

	virtual hsBool			OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, hsBool readOnly);
	virtual hsBool			CloseAccess(plAccessSpan& acc);

	virtual void			CheckTextureRef(plLayerInterface* lay);		
	static void				FreeManagedTexture(UInt32 sz) { hsAssert(fTexManaged >= sz, "Freeing mem we don't have"); fTexManaged -= sz; }
	static void				AllocManagedTexture(UInt32 sz) { fTexManaged += sz; }
	static void				FreeManagedVertex(UInt32 sz) { hsAssert(fVtxManaged >= sz, "Freeing mem we don't have"); fVtxManaged -= sz; }
	static void				AllocManagedVertex(UInt32 sz) { fVtxManaged += sz; }

#ifndef PLASMA_EXTERNAL_RELEASE
	static void ProfilePoolMem(D3DPOOL poolType, UInt32 size, hsBool add, char *id);
#endif // PLASMA_EXTERNAL_RELEASE

	//	From a D3DFORMAT enumeration, return the bit depth associated with it.
	static short	GetDXBitDepth( D3DFORMAT format );

	// Default fog settings
	virtual void						SetDefaultFogEnviron( plFogEnvironment *fog ) { fView.fDefaultFog = *fog; fCurrFog.fEnvPtr = nil; }
	virtual const plFogEnvironment		&GetDefaultFogEnviron() const { return fView.fDefaultFog; }

	// View state
	virtual hsPoint3					GetViewPositionWorld() const { return GetViewTransform().GetPosition(); }
	virtual hsVector3					GetViewAcrossWorld() const { return GetViewTransform().GetAcross(); }
	virtual hsVector3					GetViewUpWorld() const { return GetViewTransform().GetUp(); }
	virtual hsVector3					GetViewDirWorld() const { return GetViewTransform().GetDirection(); }
	virtual void						GetViewAxesWorld(hsVector3 axes[3] /* ac,up,at */ ) const;

	virtual void						GetFOV(hsScalar& fovX, hsScalar& fovY) const;
	virtual void						SetFOV(hsScalar fovX, hsScalar fovY);

	virtual void						GetSize(hsScalar& width, hsScalar& height) const;
	virtual void						SetSize(hsScalar width, hsScalar height);

	virtual void						GetDepth(hsScalar& hither, hsScalar& yon) const;
	virtual void						SetDepth(hsScalar hither, hsScalar yon);

	virtual hsScalar					GetZBiasScale() const;
	virtual void						SetZBiasScale(hsScalar scale);

	virtual const hsMatrix44&			GetWorldToCamera() const;
	virtual const hsMatrix44&			GetCameraToWorld() const;
	virtual void						SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w);

	virtual void						SetViewTransform(const plViewTransform& trans);
	virtual const plViewTransform&		GetViewTransform() const { return fView.fTransform; }

	virtual const hsMatrix44&			GetWorldToLocal() const;
	virtual const hsMatrix44&			GetLocalToWorld() const;

	virtual void						ScreenToWorldPoint( int n, UInt32 stride, Int32 *scrX, Int32 *scrY, 
													hsScalar dist, UInt32 strideOut, hsPoint3 *worldOut );
	
	virtual void						RefreshMatrices();
	virtual void						RefreshScreenMatrices();

	virtual void						RegisterLight(plLightInfo* light);
	virtual void						UnRegisterLight(plLightInfo* light);

	// Overrides, always push returns whatever is necessary to restore on pop.
	virtual hsGMaterial*				PushOverrideMaterial(hsGMaterial* mat);
	virtual void						PopOverrideMaterial(hsGMaterial* restore);
	virtual hsGMaterial*				GetOverrideMaterial() const;

	virtual plLayerInterface*			AppendLayerInterface(plLayerInterface* li, hsBool onAllLayers = false);
	virtual plLayerInterface*			RemoveLayerInterface(plLayerInterface* li, hsBool onAllLayers = false);

	virtual plLayerInterface*			PushPiggyBackLayer(plLayerInterface* li);
	virtual plLayerInterface*			PopPiggyBackLayer(plLayerInterface* li);

	virtual UInt32						GetMaterialOverrideOn(hsGMatState::StateIdx category) const;
	virtual UInt32						GetMaterialOverrideOff(hsGMatState::StateIdx category) const;

	virtual hsGMatState					PushMaterialOverride(const hsGMatState& state, hsBool on);
	virtual hsGMatState					PushMaterialOverride(hsGMatState::StateIdx cat, UInt32 which, hsBool on);
	virtual void						PopMaterialOverride(const hsGMatState& restore, hsBool on);
	virtual const hsGMatState&			GetMaterialOverride(hsBool on) const;

	virtual hsColorOverride				PushColorOverride(const hsColorOverride& over);
	virtual void						PopColorOverride(const hsColorOverride& restore);
	virtual const hsColorOverride&		GetColorOverride() const;

	virtual void						SubmitShadowSlave(plShadowSlave* slave);
	virtual void						SubmitClothingOutfit(plClothingOutfit* co);

	virtual hsBool						SetGamma(hsScalar eR, hsScalar eG, hsScalar eB);
	virtual hsBool						SetGamma(const UInt16* const tabR, const UInt16* const tabG, const UInt16* const tabB);

	virtual hsBool						CaptureScreen( plMipmap *dest, bool flipVertical = false, UInt16 desiredWidth = 0, UInt16 desiredHeight = 0 );
	virtual plMipmap*					ExtractMipMap(plRenderTarget* targ);

	/// Error handling
	virtual const char					*GetErrorString();

	hsBool								ManagedAlloced() const { return fManagedAlloced; }

	virtual void						GetSupportedColorDepths(hsTArray<int> &ColorDepths);
	virtual void						GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 );
	virtual int							GetMaxAnisotropicSamples();
	virtual int							GetMaxAntiAlias(int Width, int Height, int ColorDepth);

};


//// Direct3D Inlines //////////////////////////////////////////////////////
//	??.?? - Some mild optimizations PBG
//	MMW - take advantage of the 32 bit float representation on a PC

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

inline DWORD	plDXPipeline::inlGetD3DColor( const hsColorRGBA &col ) const
{
    DWORD	dr, dg, db, da;
    
	CONVERT_FLOAT_TO_BYTE_COLOR( col.r, dr );   
	CONVERT_FLOAT_TO_BYTE_COLOR( col.g, dg );
	CONVERT_FLOAT_TO_BYTE_COLOR( col.b, db );
	CONVERT_FLOAT_TO_BYTE_COLOR( col.a, da );
  
    return( ( da << 24 ) | ( dr << 16 ) | ( dg << 8 ) | db );
}


#endif // _plDX9Pipeline_h
