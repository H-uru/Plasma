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

#ifndef plPipeline_inc
#define plPipeline_inc

#include "../pnFactory/plCreatable.h"
#include "hsGMatState.h"
#include "hsTemplates.h"
#include "hsStlUtils.h"

#define MIN_WIDTH 640
#define MIN_HEIGHT 480

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 600
#define DEFAULT_WINDOWED false
#define DEFAULT_COLORDEPTH 32
#define DEFAULT_ANTIALIASAMOUNT 0
#define DEFAULT_ANISOLEVEL 0
#define DEFAULT_TEXTUREQUALITY 2
#define DEFAULT_VIDEOQUALITY 2
#define DEFAULT_SHADOWS 0
#define DEFAULT_PLANARREFLECTIONS 0


struct hsPoint3;
struct hsVector3;
struct hsMatrix44;
struct hsColorRGBA;
class hsBounds3Ext;
class hsGMaterial;
class plDrawPrim;
class plRenderTarget;
class hsG3DDevice;
class hsGView3;
class plDrawable;
class plGBufferGroup;
class plLayerInterface;
class plSpaceTree;
class plCullPoly;
class plRenderRequest;
class plShadowSlave;
class plAccessSpan;
class plTextFont;
class plVertexSpan;
class plDrawableSpans;
class plSceneObject;
class plClothingOutfit;
class hsGDeviceRef;
class plFogEnvironment;
class plLightInfo;
class plMipmap;
class plVisMgr;

class plViewTransform;


struct PipelineParams
{
	PipelineParams():
	Width(DEFAULT_WIDTH),
	Height(DEFAULT_HEIGHT),
	Windowed(DEFAULT_WINDOWED),
	ColorDepth(DEFAULT_COLORDEPTH),
	AntiAliasingAmount(DEFAULT_ANTIALIASAMOUNT),
	AnisotropicLevel(DEFAULT_ANISOLEVEL),
	TextureQuality(DEFAULT_TEXTUREQUALITY),
	VideoQuality(DEFAULT_VIDEOQUALITY),
	Shadows(DEFAULT_SHADOWS),
	PlanarReflections(DEFAULT_PLANARREFLECTIONS),
#ifndef PLASMA_EXTERNAL_RELEASE
	ForceSecondMonitor(false),
#endif // PLASMA_EXTERNAL_RELEASE
	VSync(false)
	{
	}

	int Width;
	int Height;
	hsBool Windowed;
	int ColorDepth;
	int AntiAliasingAmount;
	int AnisotropicLevel;
	int TextureQuality;
	int VideoQuality;
	int Shadows;
	int PlanarReflections;
	hsBool VSync;
#ifndef PLASMA_EXTERNAL_RELEASE
	hsBool ForceSecondMonitor;
#endif // PLASMA_EXTERNAL_RELEASE
};

class plDisplayMode
{
public:
	int Width;
	int Height;
	int ColorDepth;
};

class plPipeline : public plCreatable
{
public:

	CLASSNAME_REGISTER( plPipeline );
	GETINTERFACE_ANY( plPipeline, plCreatable );

	// Typical 3D device
	//
	// PreRender - fill out the visList with which spans from drawable will be drawn.
	// visList is write only. On output, visList is UNSORTED visible spans.
	// Called once per scene render (maybe multiple times per frame).
	// Returns true if rendering should proceed.
	virtual hsBool						PreRender(plDrawable* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr=nil) = 0;
	// PrepForRender - perform any processing on the drawable data nessecary before rendering.
	// visList is read only. On input, visList is SORTED visible spans, and is ALL spans which will be drawn this render.
	// Called once per scene render. 
	// Returns true if rendering should proceed.
	virtual hsBool						PrepForRender(plDrawable* drawable, hsTArray<Int16>& visList, plVisMgr* visMgr=nil) = 0;
	// Render - draw the drawable to the current render target.
	// visList is read only. On input, visList is SORTED visible spans. May not be the complete list of visible spans
	// for this drawable.
	// Called multiple times per scene render e.g.:
	//		Render(drawable0, visList0) // visList0 contains furthest spans in drawable0
	//		Render(drawable1, visList1) // visList1 contains spans from drawable1 between drawable0's visList0 and visList2
	//		Render(drawable0, visList2) // visList2 contains closest spans in drawable0.
	virtual void						Render(plDrawable* d, const hsTArray<Int16>& visList) = 0;
	// Draw - Convenience wrapper for standalone renders. Calls PreRender, PrepForRender, Render. Currently for internal
	// use only, but may prove useful for procedurals (render to texture).
	virtual void						Draw(plDrawable* d) = 0;
	
	// Device-specific ref creation. Includes buffers and fonts
	virtual plTextFont					*MakeTextFont( char *face, UInt16 size ) = 0;

	// Create and/or Refresh geometry buffers
	virtual void			CheckVertexBufferRef(plGBufferGroup* owner, UInt32 idx) = 0;
	virtual void			CheckIndexBufferRef(plGBufferGroup* owner, UInt32 idx) = 0;

	virtual hsBool			OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, hsBool readOnly) = 0;
	virtual hsBool			CloseAccess(plAccessSpan& acc) = 0;

	virtual void			CheckTextureRef(plLayerInterface* lay) = 0;		

	// Default fog settings
	virtual void						SetDefaultFogEnviron( plFogEnvironment *fog ) = 0;
	virtual const plFogEnvironment		&GetDefaultFogEnviron( void ) const = 0;

	virtual void						RegisterLight(plLightInfo* light) = 0;
	virtual void						UnRegisterLight(plLightInfo* light) = 0;

	enum RenderTargetFlags
	{
		kRTMainScreen		= 0x0000,
		kRTOffscreen		= 0x0001,
		kRTTexture			= 0x0002,
		kRTPerspProjected	= 0x0004,
		kRTOrthoProjected	= 0x0008,
		kRTProjected		= kRTPerspProjected | kRTOrthoProjected
	};
	enum RenderStateSettings 
	{
		kRenderNormal			= 0x0,
		kRenderProjection		= 0x1,
		kRenderShadow			= 0x2,
		kRenderBaseLayerOnly	= 0x4,
		kRenderNoPiggyBacks		= 0x8,
		kRenderClearColor		= 0x10,
		kRenderClearDepth		= 0x20,
		kRenderOrthogonal		= 0x40,
		kRenderNoProjection		= 0x80,
		kRenderNoLights			= 0x100,
		kRenderShadowErase		= 0x200
	};

	virtual void						PushRenderRequest(plRenderRequest* req) = 0;
	virtual void						PopRenderRequest(plRenderRequest* req) = 0;

	virtual void						ClearRenderTarget( plDrawable* d ) = 0; // nil d reverts to ClearRenderTarget(nil, nil).
	virtual void						ClearRenderTarget(const hsColorRGBA* col = nil, const hsScalar* depth = nil) = 0; // col/depth are overrides for current default.
	virtual void						SetClear(const hsColorRGBA* col=nil, const hsScalar* depth=nil) = 0; // sets the default clear for current render target.
	virtual hsColorRGBA					GetClearColor() const = 0;
	virtual hsScalar					GetClearDepth() const = 0;
	virtual hsGDeviceRef				*MakeRenderTargetRef( plRenderTarget *owner ) = 0;
	virtual void						PushRenderTarget( plRenderTarget *target ) = 0;
	virtual plRenderTarget				*PopRenderTarget( void ) = 0;

	virtual hsBool						BeginRender() = 0;
	virtual hsBool						EndRender() = 0;
	virtual void						RenderScreenElements( void ) = 0;

	virtual hsBool						BeginDrawable(plDrawable* d) = 0;
	virtual hsBool						EndDrawable(plDrawable* d) = 0;

	virtual void						BeginVisMgr(plVisMgr* visMgr) = 0;
	virtual void						EndVisMgr(plVisMgr* visMgr) = 0;

	virtual hsBool						IsFullScreen() const = 0;
	virtual UInt32						Width() const = 0;
	virtual UInt32						Height() const = 0;
	virtual UInt32						ColorDepth() const = 0;
	virtual void						Resize( UInt32 width, UInt32 height ) = 0;

	// Culling. Might be used in Update before bothering to do any serious computation.
	virtual hsBool						TestVisibleWorld(const hsBounds3Ext& wBnd) = 0;
	virtual hsBool						TestVisibleWorld(const plSceneObject* sObj) = 0;
	virtual hsBool						HarvestVisible(plSpaceTree* space, hsTArray<Int16>& visList) = 0;
	virtual hsBool						SubmitOccluders(const hsTArray<const plCullPoly*>& polyList) = 0;
	
	virtual void						SetDebugFlag( UInt32 flag, hsBool on ) = 0;
	virtual hsBool						IsDebugFlagSet( UInt32 flag ) const = 0;
	virtual void						SetMaxCullNodes(UInt16 n) = 0; // Debug/analysis only
	virtual UInt16						GetMaxCullNodes() const = 0; // Debug/analysis only

	// Properties
	enum Properties
	{
		kPropDontDeleteTextures		= 0x00000001			// Keeps the pipeline from deleting textures on 
															// MakeTextureRef, regardless of the kUserOwnsBitmap flag
	};

	virtual hsBool						CheckResources() = 0; // Do we need to call LoadResources?
	virtual void						LoadResources() = 0;

	virtual void						SetProperty( UInt32 prop, hsBool on ) = 0;
	virtual hsBool						GetProperty( UInt32 prop ) const = 0;
	virtual UInt32						GetMaxLayersAtOnce() const = 0;

	// Drawable type mask
	virtual void						SetDrawableTypeMask( UInt32 mask ) = 0;
	virtual UInt32						GetDrawableTypeMask( void ) const = 0;
	virtual void						SetSubDrawableTypeMask( UInt32 mask ) = 0;
	virtual UInt32						GetSubDrawableTypeMask( void ) const = 0;

	// View state
	virtual hsPoint3					GetViewPositionWorld() const = 0;
	virtual hsVector3					GetViewAcrossWorld() const = 0;
	virtual hsVector3					GetViewUpWorld() const = 0;
	virtual hsVector3					GetViewDirWorld() const = 0;
	virtual void						GetViewAxesWorld(hsVector3 axes[3] /* ac,up,at */ ) const = 0;

	virtual void						GetFOV(hsScalar& fovX, hsScalar& fovY) const = 0;
	virtual void						SetFOV(hsScalar fovX, hsScalar fovY) = 0;

	virtual void						GetSize(hsScalar& width, hsScalar& height) const = 0;
	virtual void						SetSize(hsScalar width, hsScalar height) = 0;

	virtual void						GetDepth(hsScalar& hither, hsScalar& yon) const = 0;
	virtual void						SetDepth(hsScalar hither, hsScalar yon) = 0;

	virtual void						SetZBiasScale( hsScalar scale ) = 0;
	virtual hsScalar					GetZBiasScale( void ) const = 0;

	virtual const hsMatrix44&			GetWorldToCamera() const = 0;
	virtual const hsMatrix44&			GetCameraToWorld() const = 0;
	virtual void						SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w) = 0;

	virtual const hsMatrix44&			GetWorldToLocal() const = 0;
	virtual const hsMatrix44&			GetLocalToWorld() const = 0;

	virtual const plViewTransform&		GetViewTransform() const = 0;

	virtual void						ScreenToWorldPoint( int n, UInt32 stride, Int32 *scrX, Int32 *scrY, 
													hsScalar dist, UInt32 strideOut, hsPoint3 *worldOut ) = 0;

	virtual void						RefreshMatrices( void ) = 0;
	virtual void						RefreshScreenMatrices( void ) = 0;

	// Overrides, always push returns whatever is necessary to restore on pop.
	virtual hsGMaterial*				PushOverrideMaterial(hsGMaterial* mat) = 0;
	virtual void						PopOverrideMaterial(hsGMaterial* restore) = 0;
	virtual hsGMaterial*				GetOverrideMaterial() const = 0;

	virtual plLayerInterface*			AppendLayerInterface(plLayerInterface* li, hsBool onAllLayers = false) = 0;
	virtual plLayerInterface*			RemoveLayerInterface(plLayerInterface* li, hsBool onAllLayers = false) = 0;

	virtual UInt32						GetMaterialOverrideOn(hsGMatState::StateIdx category) const = 0;
	virtual UInt32						GetMaterialOverrideOff(hsGMatState::StateIdx category) const = 0;

	virtual hsGMatState					PushMaterialOverride(const hsGMatState& state, hsBool on) = 0;
	virtual hsGMatState					PushMaterialOverride(hsGMatState::StateIdx cat, UInt32 which, hsBool on) = 0;
	virtual void						PopMaterialOverride(const hsGMatState& restore, hsBool on) = 0;
	virtual const hsGMatState&			GetMaterialOverride(hsBool on) const = 0;

	virtual hsColorOverride				PushColorOverride(const hsColorOverride& over) = 0;
	virtual void						PopColorOverride(const hsColorOverride& restore) = 0;
	virtual const hsColorOverride&		GetColorOverride() const = 0;

	virtual void						SubmitShadowSlave(plShadowSlave* slave) = 0;
	virtual void						SubmitClothingOutfit(plClothingOutfit* co) = 0;

	// These all return true if the gamma was successfully set.
	virtual hsBool						SetGamma(hsScalar eR, hsScalar eG, hsScalar eB) = 0;
	virtual hsBool						SetGamma(const UInt16* const tabR, const UInt16* const tabG, const UInt16* const tabB) = 0; // len table = 256.
	virtual hsBool						SetGamma(hsScalar e) { return SetGamma(e, e, e); }
	virtual hsBool						SetGamma(const UInt16* const table) { return SetGamma(table, table, table); } 

	// flipVertical is for the AVI writer, which wants it's frames upside down
	virtual hsBool						CaptureScreen( plMipmap *dest, bool flipVertical = false, UInt16 desiredWidth = 0, UInt16 desiredHeight = 0 ) = 0;

	// Returns an un-named (GetKey()==nil) mipmap same dimensions as targ. You are responsible for deleting said mipMap.
	virtual plMipmap*					ExtractMipMap(plRenderTarget* targ) = 0;

	/// Error handling
	virtual const char					*GetErrorString( void ) = 0;

	// info about current rendering device
	virtual void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) = 0;
	virtual int GetMaxAnisotropicSamples() = 0;
	virtual int GetMaxAntiAlias(int Width, int Height, int ColorDepth) = 0;
	int GetDesktopWidth() { return fDesktopParams.Width; }
	int GetDesktopHeight() { return fDesktopParams.Height; }
	int GetDesktopColorDepth() { return fDesktopParams.ColorDepth; }
	PipelineParams *GetDefaultParams() { return &fDefaultPipeParams; }

	virtual void ResetDisplayDevice(int Width, int Height, int ColorDepth, hsBool Windowed, int NumAASamples, int MaxAnisotropicSamples, hsBool vSync = false  ) = 0;
	static PipelineParams fDefaultPipeParams;
	static PipelineParams fInitialPipeParams;
	plDisplayMode fDesktopParams;
};

#endif // plPipeline_inc
