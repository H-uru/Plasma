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
#ifndef _pl3DPipeline_inc_
#define _pl3DPipeline_inc_

#include <stack>

#include "plPipeline.h"
#include "plPipelineViewSettings.h"
#include "hsGDeviceRef.h"
#include "hsG3DDeviceSelector.h"

#include "plSurface/plLayerInterface.h"


class hsGMaterial;
class plLightInfo;
class plShadowSlave;
class plSpan;

#if defined(PLASMA_PIPELINE_DX)
#    include "DX/plDXDevice.h"
#    define DeviceType plDXDevice
#elif defined(PLASMA_PIPELINE_GL)
#    include "GL/plGLDevice.h"
#    define DeviceType plGLDevice
#else
#    error "plPipeline backend not specified"
#endif

class pl3DPipeline : public plPipeline
{
protected:
    DeviceType                          fDevice;

    plPipelineViewSettings              fView;
    std::stack<plPipelineViewSettings>  fViewStack;

    plPipelineTweakSettings             fTweaks;

    hsBitVector                         fDebugFlags;
    uint32_t                            fProperties;

    uint32_t                            fMaxLayersAtOnce;
    uint32_t                            fMaxPiggyBacks;
    uint32_t                            fMaxNumLights;
    uint32_t                            fMaxNumProjectors;

    hsGMatState                         fMatOverOn;
    hsGMatState                         fMatOverOff;
    hsTArray<hsGMaterial*>              fOverrideMat;
    hsGMaterial*                        fHoldMat;
    bool                                fForceMatHandle;

    hsTArray<plLayerInterface*>         fOverLayerStack;
    plLayerInterface*                   fOverBaseLayer;
    plLayerInterface*                   fOverAllLayer;
    hsTArray<plLayerInterface*>         fPiggyBackStack;
    int32_t                             fMatPiggyBacks;
    int32_t                             fActivePiggyBacks;

    DeviceType::VertexBufferRef*        fVtxBuffRefList;
    DeviceType::IndexBufferRef*         fIdxBuffRefList;

    hsGDeviceRef*                       fLayerRef[8];

    hsGMaterial*                        fCurrMaterial;

    plLayerInterface*                   fCurrLay;
    uint32_t                            fCurrLayerIdx;
    uint32_t                            fCurrNumLayers;
    uint32_t                            fCurrRenderLayer;
    uint32_t                            fCurrLightingMethod;    // Based on plSpan flags

    hsMatrix44                          fBumpDuMatrix;
    hsMatrix44                          fBumpDvMatrix;
    hsMatrix44                          fBumpDwMatrix;

    plLightInfo*                        fActiveLights;
    hsTArray<plLightInfo*>              fCharLights;
    hsTArray<plLightInfo*>              fVisLights;

    hsTArray<plShadowSlave*>            fShadows;

    hsTArray<plRenderTarget*>           fRenderTargets;
    plRenderTarget*                     fCurrRenderTarget;
    plRenderTarget*                     fCurrBaseRenderTarget;
    hsGDeviceRef*                       fCurrRenderTargetRef;

    uint32_t                            fOrigWidth;
    uint32_t                            fOrigHeight;
    uint32_t                            fColorDepth;

    uint32_t                            fInSceneDepth;
    double                              fTime;      // World time.
    uint32_t                            fFrame;     // inc'd every time the camera moves.
    uint32_t                            fRenderCnt; // inc'd every begin scene.

    bool                                fVSync;


public:
    pl3DPipeline(const hsG3DDeviceModeRecord* devModeRec);
    virtual ~pl3DPipeline();

    CLASSNAME_REGISTER(pl3DPipeline);
    GETINTERFACE_ANY(pl3DPipeline, plPipeline);

    size_t GetViewStackSize() const { return fViewStack.size(); }


    /*** VIRTUAL METHODS ***/
    //virtual bool PreRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;
    //virtual bool PrepForRender(plDrawable* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;


    /**
     * The normal way to render a subset of a drawable.
     *
     * This assumes that PreRender and PrepForRender have already been called.
     *
     * Note that PreRender and PrepForRender are called once per drawable per
     * render with a visList containing all of the spans which will be
     * rendered, but Render itself may be called with multiple visList subsets
     * which union to the visList passed into PreRender/PrepForRender. This
     * happens when drawing sorted spans, because some spans from drawable B
     * may be in the middle of the spans of drawable A, so the sequence would
     * be:
     *    PreRender(A, ATotalVisList);
     *    PreRender(B, BTotalVisList);
     *    PrepForRender(A, ATotalVisList);
     *    PrepForRender(B, BTotalVisList);
     *    Render(A, AFarHalfVisList);
     *    Render(B, BTotalVisList);
     *    Render(A, ANearHalfVisList);
     *
     * See plPageTreeMgr, which handles all this.
     */
    virtual void Render(plDrawable* d, const hsTArray<int16_t>& visList);


    /**
     * Convenience function for a drawable that needs to get drawn outside of
     * the normal scene graph render (i.e. something not managed by the
     * plPageTreeMgr).
     *
     * Not nearly as efficient, so only useful as a special case.
     */
    virtual void Draw(plDrawable* d);


    //virtual plTextFont* MakeTextFont(char* face, uint16_t size) = 0;
    //virtual void CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) = 0;
    //virtual void CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) = 0;
    //virtual bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) = 0;
    //virtual bool CloseAccess(plAccessSpan& acc) = 0;
    //virtual void CheckTextureRef(plLayerInterface* lay) = 0;

    virtual void SetDefaultFogEnviron(plFogEnvironment* fog) {
        fView.SetDefaultFog(*fog);
    }

    virtual const plFogEnvironment& GetDefaultFogEnviron() const {
        return fView.GetDefaultFog();
    }


    /**
     * Register a light with the pipeline.
     * Light becomes immediately ready to illuminate the scene.
     *
     * Note: This function does not set up the native light device ref,
     * so you must override this in your pipeline implementation!
     */
    virtual void RegisterLight(plLightInfo* light);


    /**
     * Remove a light from the pipeline's active light list.
     * Light will no longer illuminate the scene.
     */
    virtual void UnRegisterLight(plLightInfo* light);


    //virtual void PushRenderRequest(plRenderRequest* req);
    //virtual void PopRenderRequest(plRenderRequest* req) = 0;
    //virtual void ClearRenderTarget(plDrawable* d) = 0;
    //virtual void ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) = 0;

    virtual void SetClear(const hsColorRGBA* col=nullptr, const float* depth=nullptr) {
        fView.SetClear(col, depth);
    }

    virtual hsColorRGBA GetClearColor() const {
        return fView.GetClearColor();
    }

    virtual float GetClearDepth() const {
        return fView.GetClearDepth();
    }

    //virtual hsGDeviceRef* MakeRenderTargetRef(plRenderTarget* owner) = 0;

    /**
     * Begin rendering to the specified target.
     *
     * If target is null, that's the primary surface.
     */
    virtual void PushRenderTarget(plRenderTarget* target);

    /**
     * Resume rendering to the render target before the last PushRenderTarget,
     * making sure we aren't holding on to anything from the render target
     * getting popped.
     */
    virtual plRenderTarget* PopRenderTarget();

    //virtual bool BeginRender() = 0;
    //virtual bool EndRender() = 0;
    //virtual void RenderScreenElements() = 0;

    /**
     * Marks the beginning of a render with the given visibility manager.
     * In particular, we cache which lights the visMgr believes to be
     * currently active
     */
    virtual void BeginVisMgr(plVisMgr* visMgr);


    /** Marks the end of a render with the given visibility manager. */
    virtual void EndVisMgr(plVisMgr* visMgr);


    //virtual bool IsFullScreen() const = 0;

    virtual uint32_t Width() const {
        return GetViewTransform().GetViewPortWidth();
    }

    virtual uint32_t Height() const {
        return GetViewTransform().GetViewPortHeight();
    }

    virtual uint32_t ColorDepth() const {
        return fColorDepth;
    }

    //virtual void Resize(uint32_t width, uint32_t height) = 0;

    /**
     * Check if the world space bounds are visible within the current view
     * frustum.
     */
    virtual bool TestVisibleWorld(const hsBounds3Ext& wBnd) {
        return fView.TestVisibleWorld(wBnd);
    }


    /**
     * Check if the object space bounds are visible within the current view
     * frustum.
     */
    virtual bool TestVisibleWorld(const plSceneObject* sObj) {
        return fView.TestVisibleWorld(sObj);
    }


    /**
     * Contruct a list of the indices of leaf nodes in the given spacetree
     * which are currently visible according to the current cull tree.
     *
     * The cull tree factors in camera frustum and occluder polys, but _not_
     * the current visibility regions, plVisMgr.
     *
     * This is the normal path for visibility culling at a gross level (e.g.
     * which SceneNodes to bother with, which drawables within the SceneNode).
     * For finer objects, like the spans themselves, the culling is done via
     * fView.GetVisibleSpans, which also takes the plVisMgr into account.
     */
    virtual bool HarvestVisible(plSpaceTree* space, hsTArray<int16_t>& visList) {
        return fView.HarvestVisible(space, visList);
    }


    /**
     * Add the input polys into the list of polys from which to generate the
     * cull tree.
     */
    virtual bool SubmitOccluders(const hsTArray<const plCullPoly*>& polyList) {
        return fView.SubmitOccluders(polyList);
    }

    virtual void SetDebugFlag(uint32_t flag, bool on) {
        fDebugFlags.SetBit(flag, on);
    }

    virtual bool IsDebugFlagSet(uint32_t flag) const {
        return fDebugFlags.IsBitSet(flag);
    }

    virtual void SetMaxCullNodes(uint16_t n) {
        fView.SetMaxCullNodes(n);
    }

    virtual uint16_t GetMaxCullNodes() const {
        return fView.GetMaxCullNodes();
    }

    //virtual bool CheckResources() = 0;
    //virtual void LoadResources() = 0;

    virtual void SetProperty(uint32_t prop, bool on) {
        on ? fProperties |= prop : fProperties &= ~prop;
    }


    virtual bool GetProperty(uint32_t prop) const {
        return (fProperties & prop) ? true : false;
    }


    virtual uint32_t GetMaxLayersAtOnce() const {
        return fMaxLayersAtOnce;
    }


    virtual void SetDrawableTypeMask(uint32_t mask) {
        fView.SetDrawableTypeMask(mask);
    }


    virtual uint32_t GetDrawableTypeMask() const {
        return fView.GetDrawableTypeMask();
    }


    virtual void SetSubDrawableTypeMask(uint32_t mask) {
        fView.SetSubDrawableTypeMask(mask);
    }


    virtual uint32_t GetSubDrawableTypeMask() const {
        return fView.GetSubDrawableTypeMask();
    }


    virtual hsPoint3 GetViewPositionWorld() const {
        return GetViewTransform().GetPosition();
    }


    virtual hsVector3 GetViewAcrossWorld() const {
        return GetViewTransform().GetAcross();
    }


    virtual hsVector3 GetViewUpWorld() const {
        return GetViewTransform().GetUp();
    }


    virtual hsVector3 GetViewDirWorld() const {
        return GetViewTransform().GetDirection();
    }


    /** Get the current view direction, up, and direction x up. */
    virtual void GetViewAxesWorld(hsVector3 axes[3]) const {
        axes[0] = GetViewAcrossWorld();
        axes[1] = GetViewUpWorld();
        axes[2] = GetViewDirWorld();
    }


    /** Get the current FOV in degrees. */
    virtual void GetFOV(float& fovX, float& fovY) const {
        fovX = GetViewTransform().GetFovXDeg();
        fovY = GetViewTransform().GetFovYDeg();
    }


    /**
     * Set the current FOV in degrees.
     * Forces perspective rendering to be true.
     */
    virtual void SetFOV(float fovX, float fovY) {
        fView.SetFOV(fovX, fovY);
    }


    /** Get the orthogonal projection view size in world units (eg. feet). */
    virtual void GetSize(float& width, float& height) const {
        width = GetViewTransform().GetScreenWidth();
        height = GetViewTransform().GetScreenHeight();
    }


    /**
     * Set the orthogonal projection view size in world units (e.g. feet).
     * Forces projection to orthogonal if it wasn't.
     */
    virtual void SetSize(float width, float height) {
        fView.SetSize(width, height);
    }


    /** Get the current hither and yon. */
    virtual void GetDepth(float& hither, float& yon) const {
        GetViewTransform().GetDepth(hither, yon);
    }


    /** Set the current hither and yon. */
    virtual void SetDepth(float hither, float yon) {
        fView.SetDepth(hither, yon);
    }


    /**
     * If the board really doesn't support Z-biasing, we adjust the perspective
     * matrix in IGetCameraToNDC. The layer scale and translation are tailored
     * to the current hardware.
     */
    virtual void SetZBiasScale(float scale);

    virtual float GetZBiasScale() const;


    /** Return current World to Camera transform. */
    virtual const hsMatrix44& GetWorldToCamera() const {
        return fView.GetWorldToCamera();
    }


    /** Return current Camera to World transform. */
    virtual const hsMatrix44& GetCameraToWorld() const {
        return fView.GetCameraToWorld();
    }

    /** Immediate set of camera transform. */
    virtual void SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w);

    /**
     * Return current World to Local transform.
     * Note that this is only meaningful while an object is being rendered, so
     * this function is pretty worthless.
     */
    virtual const hsMatrix44& GetWorldToLocal() const {
        return fView.GetWorldToLocal();
    }


    /**
     * Return current Local to World transform.
     * Note that this is only meaningful while an object is being rendered, so
     * this function is pretty worthless.
     */
    virtual const hsMatrix44& GetLocalToWorld() const {
        return fView.GetLocalToWorld();
    }


    virtual const plViewTransform& GetViewTransform() const {
        return fView.GetConstViewTransform();
    }


    /**
     * Given a screen space pixel position, and a world space distance from
     * the camera, return a full world space position.
     * I.e. cast a ray through a screen pixel dist feet, and where is it.
     */
    virtual void ScreenToWorldPoint(int n, uint32_t stride, int32_t* scrX, int32_t* scrY, float dist, uint32_t strideOut, hsPoint3* worldOut);


    /**
     * Force a refresh of cached state when the projection matrix changes.
     */
    virtual void RefreshMatrices() {
        RefreshScreenMatrices();
    }

    /**
     * Force a refresh of cached state when the projection matrix changes.
     */
    virtual void RefreshScreenMatrices();


    /**
     * Push a material to be used instead of the material associated with
     * objects for rendering.
     * Must be matched with a PopOverrideMaterial.
     */
    virtual hsGMaterial* PushOverrideMaterial(hsGMaterial* mat);


    /**
     * Stop overriding with the current override material.
     * Must match a preceding PushOverrideMaterial.
     */
    virtual void PopOverrideMaterial(hsGMaterial* restore);


    /** Return the current override material, or nullptr if there isn't any. */
    virtual hsGMaterial* GetOverrideMaterial() const {
        return fOverrideMat.GetCount() ? fOverrideMat.Peek() : nullptr;
    }


    /**
     * Set up a layer wrapper to wrap around either all layers rendered with
     * or just the base layers.
     * Note that a single material has multiple base layers if it takes
     * multiple passes to render.
     *
     * Stays in effect until removed by RemoveLayerInterface.
     */
    virtual plLayerInterface* AppendLayerInterface(plLayerInterface* li, bool onAllLayers = false);


    /** Removes a layer wrapper installed by AppendLayerInterface. */
    virtual plLayerInterface* RemoveLayerInterface(plLayerInterface* li, bool onAllLayers = false);


    /**
     * Return the current bits set to be always on for the given category
     * (e.g. ZFlags).
     */
    virtual uint32_t GetMaterialOverrideOn(hsGMatState::StateIdx category) const {
        return fMatOverOn.Value(category);
    }


    /**
     * Return the current bits set to be always off for the given category
     * (e.g. ZFlags).
     */
    virtual uint32_t GetMaterialOverrideOff(hsGMatState::StateIdx category) const {
        return fMatOverOff.Value(category);
    }


    /**
     * Force material state bits on or off.
     *
     * If you use this, save the return value as input to
     * PopMaterialOverride, to restore previous values.
     */
    virtual hsGMatState PushMaterialOverride(const hsGMatState& state, bool on);


    /**
     * Force material state bits on or off.
     * This version just sets for one category (e.g. Z flags).
     *
     * If you use this, save the return value as input to
     * PopMaterialOverride, to restore previous values.
     */
    virtual hsGMatState PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on);


    /**
     * Restore the previous settings returned from the matching
     * PushMaterialOverride.
     */
    virtual void PopMaterialOverride(const hsGMatState& restore, bool on);


    /**
     * Return the current material state bits forced to on or off, depending
     * on input <on>.
     */
    virtual const hsGMatState& GetMaterialOverride(bool on) const {
        return on ? fMatOverOn : fMatOverOff;
    }


    /**
     * Puts the slave in a list valid for this frame only.
     * The list will be preprocessed at BeginRender.
     *
     * See IPreprocessShadows.
     */
    virtual void SubmitShadowSlave(plShadowSlave* slave);

    //virtual void SubmitClothingOutfit(plClothingOutfit* co) = 0;
    //virtual bool SetGamma(float eR, float eG, float eB) = 0;
    //virtual bool SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) = 0;
    //virtual bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) = 0;
    //virtual plMipmap* ExtractMipMap(plRenderTarget* targ) = 0;

    /** Return the current error string. */
    virtual const char* GetErrorString() {
        return fDevice.GetErrorString();
    }

    //virtual void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) = 0;
    //virtual int GetMaxAnisotropicSamples() = 0;
    //virtual int GetMaxAntiAlias(int Width, int Height, int ColorDepth) = 0;
    //virtual void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync = false  ) = 0;

    /** Push a piggy back onto the stack. */
    virtual plLayerInterface* PushPiggyBackLayer(plLayerInterface* li);


    /** Pull the piggy back out of the stack (if it's there). */
    virtual plLayerInterface* PopPiggyBackLayer(plLayerInterface* li);


    /**
     * Sets the current view transform.
     *
     * ViewTransform encapsulates everything about the current camera, viewport
     * and window necessary to render or convert from world space to pixel
     * space. Doesn't include the object dependent local to world transform.
     *
     * Set plViewTransform.h
     */
    virtual void SetViewTransform(const plViewTransform& trans);

    virtual void RenderSpans(plDrawableSpans* ice, const hsTArray<int16_t>& visList) = 0;


protected:
    /**
     * Gets (non-const) the current view transform.
     *
     * GetViewTransform is a virtual method that returns a const transform, so
     * we have this to get a non-const version.
     */
    plViewTransform& IGetViewTransform() { return fView.GetViewTransform(); }

    /**
     * Find all the visible spans in this drawable affected by this shadow map,
     * and attach it to them.
     */
    void IAttachSlaveToReceivers(int iSlave, plDrawableSpans* drawable, const hsTArray<int16_t>& visList);


    /**
     * For each active shadow map (in fShadows), attach it to all of the
     * visible spans in drawable that it affects. Shadows explicitly attached
     * via light groups are handled separately in ISetShadowFromGroup.
     */
    void IAttachShadowsToReceivers(plDrawableSpans* drawable, const hsTArray<int16_t>& visList);


    /** Only allow self shadowing if requested. */
    bool IAcceptsShadow(const plSpan* span, plShadowSlave* slave);


    /**
     * Want artists to be able to just disable shadows for spans where they'll
     * either look goofy, or won't contribute.
     *
     * Also, if we have less than 3 simultaneous textures, we want to skip
     * anything with an alpha'd base layer, unless it's been overriden.
     */
    bool IReceivesShadows(const plSpan* span, hsGMaterial* mat);


    /**
     * The light casting this shadow has been explicitly attached to this span,
     * so no need for checking bounds, but we do anyway because the artists
     * aren't very conservative along those lines. The light has a bitvector
     * indicating which of the current shadows are from it (there will be a
     * shadow map for each shadow-light/shadow-caster pair), so we look through
     * those shadow maps and if they are acceptable, attach them to the span.
     *
     * Note that a shadow slave corresponds to a shadow map.
     */
    void ISetShadowFromGroup(plDrawableSpans* drawable, const plSpan* span, plLightInfo* liInfo);


    /**
     * For every span in the list of visible span indices, find the list of
     * lights that currently affect the span with an estimate of the strength
     * of how much the light affects it. The strongest 8 lights will be used to
     * illuminate that span.
     *
     * For projective lights, there is no limit on how many are supported,
     * other than performance (usually fill rate limited).
     *
     * The permaLights and permaProjs are lights explicitly selected for a span
     * via the LightGroup component.
     *
     * For static objects and static lights, the lighting was done offline and
     * stored in the vertex diffuse color.
     *
     * So here we're only looking for:
     *  A) moving objects, which can't be staticly lit, so are affected by all
     *  runtime lights.
     *  B) moving lights, which can't staticly light, so affect all objects
     *  C) specular objects + specular lights, since specular can't be
     *  precomputed.
     */
    void ICheckLighting(plDrawableSpans* drawable, hsTArray<int16_t>& visList, plVisMgr* visMgr);


    /**
     * Get the camera to NDC transform.
     *
     * This may be adjusted to create a Z bias towards the camera for cases
     * where the D3D Z bias fails us.
     */
    hsMatrix44 IGetCameraToNDC();


    /**
     * Record and pass on to the device the current local to world transform
     * for the object about to be rendered.
     */
    void ISetLocalToWorld(const hsMatrix44& l2w, const hsMatrix44& w2l);


    /**
     * Refreshes all transforms. Useful after popping renderTargets :)
     */
    void ITransformsToDevice();

    /** Send the current camera to NDC transform to the device. */
    void IProjectionMatrixToDevice();

    /** Pass the current camera transform through to the device. */
    void IWorldToCameraToDevice();

    /** pass the current local to world tranform on to the device. */
    void ILocalToWorldToDevice();
};

#endif //_pl3DPipeline_inc_
