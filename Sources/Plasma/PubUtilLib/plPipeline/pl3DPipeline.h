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
#include <string_theory/string>
#include <vector>

#include "plPipeline.h"
#include "plPipelineViewSettings.h"
#include "hsGDeviceRef.h"
#include "hsG3DDeviceSelector.h"
#include "hsGDeviceRef.h"
#include "plRenderTarget.h"
#include "plCubicRenderTarget.h"

#include "hsGMatState.inl"
#include "plPipeDebugFlags.h"
#include "plProfile.h"
#include "plTweak.h"
#include "hsTimer.h"

#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plAvatarClothing.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plGBufferGroup.h"
#include "plDrawable/plSpaceTree.h"
#include "plDrawable/plSpanTypes.h"
#include "plGImage/plMipmap.h"
#include "plGImage/plCubicEnvironmap.h"
#include "plGLight/plLightInfo.h"
#include "plGLight/plShadowSlave.h"
#include "plGLight/plShadowCaster.h"
#include "plIntersect/plVolumeIsect.h"
#include "plScene/plRenderRequest.h"
#include "plScene/plVisMgr.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"

class plDebugTextManager;
class plPlateManager;

plProfile_Extern(RenderScene);
plProfile_Extern(VisEval);
plProfile_Extern(VisSelect);
plProfile_Extern(FindSceneLights);
plProfile_Extern(FindLights);
plProfile_Extern(FindPerm);
plProfile_Extern(FindSpan);
plProfile_Extern(FindActiveLights);
plProfile_Extern(ApplyActiveLights);
plProfile_Extern(ApplyMoving);
plProfile_Extern(ApplyToSpec);
plProfile_Extern(ApplyToMoving);
plProfile_Extern(LightOn);
plProfile_Extern(LightVis);
plProfile_Extern(LightChar);
plProfile_Extern(LightActive);
plProfile_Extern(FindLightsFound);
plProfile_Extern(FindLightsPerm);
plProfile_Extern(AvatarSort);
plProfile_Extern(AvatarFaces);

static const float kPerspLayerScale  = 0.00001f;
static const float kPerspLayerScaleW = 0.001f;
static const float kPerspLayerTrans  = 0.00002f;

static const float kAvTexPoolShrinkThresh = 30.f; // seconds

//// Helper Classes ///////////////////////////////////////////////////////////

class plDisplayHelper
{
public:
    virtual plDisplayMode              DesktopDisplayMode() = 0;
    virtual std::vector<plDisplayMode> GetSupportedDisplayModes(hsDisplayHndl display, int ColorDepth = 32) const = 0;

    static plDisplayHelper* GetInstance() { return fCurrentDisplayHelper; }
    static void             SetInstance(plDisplayHelper* helper) { fCurrentDisplayHelper = helper; }

private:
    static plDisplayHelper* fCurrentDisplayHelper;
};

/**
 * The RenderPrimFunc lets you have one function which does a lot of stuff
 * around the actual call to render whatever type of primitives you have,
 * instead of duplicating everything because the one line to render is
 * different.
 *
 * These allow the same setup code path to be followed, no matter what the
 * primitive type (i.e. data-type/draw-call is going to happen once the render
 * state is set.
 * Originally useful to make one code path for trilists, tri-patches, and
 * rect-patches, but we've since dropped support for patches. We still use the
 * RenderNil function to allow the code to go through all the state setup
 * without knowing whether a render call is going to come out the other end.
 *
 * Would allow easy extension for supporting tristrips or pointsprites, but
 * we've never had a strong reason to use either.
 */
class plRenderPrimFunc
{
public:
    virtual bool RenderPrims() const = 0; // return true on error
};


class plRenderNilFunc : public plRenderPrimFunc
{
public:
    plRenderNilFunc() {}

    virtual bool RenderPrims() const { return false; }
};


template<class DeviceType>
class plRenderTriListFunc : public plRenderPrimFunc
{
protected:
    DeviceType* fDevice;
    int         fBaseVertexIndex;
    int         fVStart;
    int         fVLength;
    int         fIStart;
    int         fNumTris;

public:
    plRenderTriListFunc(DeviceType* device, int baseVertexIndex, int vStart, int vLength, int iStart, int iNumTris)
        : fDevice(device), fBaseVertexIndex(baseVertexIndex), fVStart(vStart),
          fVLength(vLength), fIStart(iStart), fNumTris(iNumTris) {}
};


//// Class Definition /////////////////////////////////////////////////////////

template<class DeviceType>
class pl3DPipeline : public plPipeline
{
protected:
    mutable DeviceType                      fDevice;

    plPipelineViewSettings                  fView;
    std::stack<plPipelineViewSettings>      fViewStack;

    plPipelineTweakSettings                 fTweaks;

    hsBitVector                             fDebugFlags;
    uint32_t                                fProperties;

    uint32_t                                fMaxLayersAtOnce;
    uint32_t                                fMaxPiggyBacks;
    uint32_t                                fMaxNumLights;
    uint32_t                                fMaxNumProjectors;

    hsGMatState                             fMatOverOn;
    hsGMatState                             fMatOverOff;
    std::vector<hsGMaterial*>               fOverrideMat;
    hsGMaterial*                            fHoldMat;
    bool                                    fForceMatHandle;

    std::vector<plLayerInterface*>          fOverLayerStack;
    plLayerInterface*                       fOverBaseLayer;
    plLayerInterface*                       fOverAllLayer;
    std::vector<plLayerInterface*>          fPiggyBackStack;
    int32_t                                 fMatPiggyBacks;
    int32_t                                 fActivePiggyBacks;

    typename DeviceType::VertexBufferRef*   fVtxBuffRefList;
    typename DeviceType::IndexBufferRef*    fIdxBuffRefList;
    typename DeviceType::TextureRef*        fTextureRefList;
    plTextFont*                             fTextFontRefList;

    hsGDeviceRef*                           fLayerRef[8];

    hsGMaterial*                            fCurrMaterial;

    plLayerInterface*                       fCurrLay;
    uint32_t                                fCurrLayerIdx;
    uint32_t                                fCurrNumLayers;
    uint32_t                                fCurrRenderLayer;
    uint32_t                                fCurrLightingMethod;    // Based on plSpan flags

    hsMatrix44                              fBumpDuMatrix;
    hsMatrix44                              fBumpDvMatrix;
    hsMatrix44                              fBumpDwMatrix;

    plLightInfo*                            fActiveLights;
    std::vector<plLightInfo*>               fCharLights;
    std::vector<plLightInfo*>               fVisLights;

    std::vector<plShadowSlave*>             fShadows;

    std::vector<plRenderTarget*>            fRenderTargets;
    plRenderTarget*                         fCurrRenderTarget;
    plRenderTarget*                         fCurrBaseRenderTarget;
    hsGDeviceRef*                           fCurrRenderTargetRef;

    uint32_t                                fOrigWidth;
    uint32_t                                fOrigHeight;
    uint32_t                                fColorDepth;

    uint32_t                                fInSceneDepth;
    double                                  fTime;      // World time.
    uint32_t                                fFrame;     // inc'd every time the camera moves.
    uint32_t                                fRenderCnt; // inc'd every begin scene.

    uint32_t                                fVtxRefTime;

    bool                                    fVSync;

    plPlateManager*                         fPlateMgr;
    plDebugTextManager*                     fDebugTextMgr;

    // Avatar Texture Rendering
    std::vector<plClothingOutfit*>          fClothingOutfits;
    std::vector<plClothingOutfit*>          fPrevClothingOutfits;
    double                                  fAvRTShrinkValidSince;
    std::vector<plRenderTarget*>            fAvRTPool;
    uint16_t                                fAvRTWidth;
    uint32_t                                fAvNextFreeRT;


public:
    pl3DPipeline(const hsG3DDeviceModeRecord* devModeRec);
    virtual ~pl3DPipeline();

    // NOTE: We are *deliberately* not including the Creatable Macros here,
    // since this is a templated class. It should be fine, since the pipeline
    // is registered as a non-creatable class.

    /*** VIRTUAL METHODS ***/
    //virtual bool PreRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;
    //virtual bool PrepForRender(plDrawable* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr=nullptr) = 0;


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
    void Render(plDrawable* d, const std::vector<int16_t>& visList) override;


    /**
     * Convenience function for a drawable that needs to get drawn outside of
     * the normal scene graph render (i.e. something not managed by the
     * plPageTreeMgr).
     *
     * Not nearly as efficient, so only useful as a special case.
     */
    void Draw(plDrawable* d) override;


    //virtual plTextFont* MakeTextFont(ST::string face, uint16_t size) = 0;

    /**
     * Make sure the buffer group has a vertex buffer ref and that its data is
     * current.
     */
    void CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx) override;

    /**
     * Make sure the buffer group has an index buffer ref and that its data is
     * current.
     */
    void CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx) override;

    //virtual bool OpenAccess(plAccessSpan& dst, plDrawableSpans* d, const plVertexSpan* span, bool readOnly) = 0;
    //virtual bool CloseAccess(plAccessSpan& acc) = 0;

    void CheckTextureRef(plLayerInterface* lay) override;

    void CheckTextureRef(plBitmap* bmp);

    void SetDefaultFogEnviron(plFogEnvironment* fog) override {
        fView.SetDefaultFog(*fog);
    }

    const plFogEnvironment& GetDefaultFogEnviron() const override {
        return fView.GetDefaultFog();
    }


    /**
     * Register a light with the pipeline.
     * Light becomes immediately ready to illuminate the scene.
     *
     * Note: This function does not set up the native light device ref,
     * so you must override this in your pipeline implementation!
     */
    void RegisterLight(plLightInfo* light) override;


    /**
     * Remove a light from the pipeline's active light list.
     * Light will no longer illuminate the scene.
     */
    void UnRegisterLight(plLightInfo* light) override;


    //virtual void PushRenderRequest(plRenderRequest* req);
    //virtual void PopRenderRequest(plRenderRequest* req) = 0;
    //virtual void ClearRenderTarget(plDrawable* d) = 0;
    //virtual void ClearRenderTarget(const hsColorRGBA* col = nullptr, const float* depth = nullptr) = 0;

    void SetClear(const hsColorRGBA* col=nullptr, const float* depth=nullptr) override {
        fView.SetClear(col, depth);
    }

    hsColorRGBA GetClearColor() const override {
        return fView.GetClearColor();
    }

    float GetClearDepth() const override {
        return fView.GetClearDepth();
    }

    //virtual hsGDeviceRef* MakeRenderTargetRef(plRenderTarget* owner) = 0;

    /**
     * Begin rendering to the specified target.
     *
     * If target is null, that's the primary surface.
     */
    void PushRenderTarget(plRenderTarget* target) override;

    /**
     * Resume rendering to the render target before the last PushRenderTarget,
     * making sure we aren't holding on to anything from the render target
     * getting popped.
     */
    plRenderTarget* PopRenderTarget() override;

    //virtual bool BeginRender() = 0;
    //virtual bool EndRender() = 0;
    //virtual void RenderScreenElements() = 0;

    /**
     * Marks the beginning of a render with the given visibility manager.
     * In particular, we cache which lights the visMgr believes to be
     * currently active
     */
    void BeginVisMgr(plVisMgr* visMgr) override;


    /** Marks the end of a render with the given visibility manager. */
    void EndVisMgr(plVisMgr* visMgr) override;


    //virtual bool IsFullScreen() const = 0;

    uint32_t Width() const override {
        return GetViewTransform().GetViewPortWidth();
    }

    uint32_t Height() const override {
        return GetViewTransform().GetViewPortHeight();
    }

    uint32_t ColorDepth() const override {
        return fColorDepth;
    }

    //virtual void Resize(uint32_t width, uint32_t height) = 0;

    /**
     * Check if the world space bounds are visible within the current view
     * frustum.
     */
    bool TestVisibleWorld(const hsBounds3Ext& wBnd) override {
        return fView.TestVisibleWorld(wBnd);
    }


    /**
     * Check if the object space bounds are visible within the current view
     * frustum.
     */
    bool TestVisibleWorld(const plSceneObject* sObj) override {
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
    bool HarvestVisible(plSpaceTree* space, std::vector<int16_t>& visList) override {
        return fView.HarvestVisible(space, visList);
    }


    /**
     * Add the input polys into the list of polys from which to generate the
     * cull tree.
     */
    bool SubmitOccluders(const std::vector<const plCullPoly*>& polyList) override {
        return fView.SubmitOccluders(polyList);
    }

    void SetDebugFlag(uint32_t flag, bool on) override {
        fDebugFlags.SetBit(flag, on);
    }

    bool IsDebugFlagSet(uint32_t flag) const override {
        return fDebugFlags.IsBitSet(flag);
    }

    void SetMaxCullNodes(uint16_t n) override {
        fView.SetMaxCullNodes(n);
    }

    uint16_t GetMaxCullNodes() const override {
        return fView.GetMaxCullNodes();
    }

    bool CheckResources() override;

    //virtual void LoadResources() = 0;

    void SetProperty(uint32_t prop, bool on) override {
        on ? fProperties |= prop : fProperties &= ~prop;
    }


    bool GetProperty(uint32_t prop) const override {
        return (fProperties & prop) ? true : false;
    }


    uint32_t GetMaxLayersAtOnce() const override {
        return fMaxLayersAtOnce;
    }


    void SetDrawableTypeMask(uint32_t mask) override {
        fView.SetDrawableTypeMask(mask);
    }


    uint32_t GetDrawableTypeMask() const override {
        return fView.GetDrawableTypeMask();
    }


    void SetSubDrawableTypeMask(uint32_t mask) override {
        fView.SetSubDrawableTypeMask(mask);
    }


    uint32_t GetSubDrawableTypeMask() const override {
        return fView.GetSubDrawableTypeMask();
    }


    hsPoint3 GetViewPositionWorld() const override {
        return GetViewTransform().GetPosition();
    }


    hsVector3 GetViewAcrossWorld() const override {
        return GetViewTransform().GetAcross();
    }


    hsVector3 GetViewUpWorld() const override {
        return GetViewTransform().GetUp();
    }


    hsVector3 GetViewDirWorld() const override {
        return GetViewTransform().GetDirection();
    }


    /** Get the current view direction, up, and direction x up. */
    void GetViewAxesWorld(hsVector3 axes[3]) const override {
        axes[0] = GetViewAcrossWorld();
        axes[1] = GetViewUpWorld();
        axes[2] = GetViewDirWorld();
    }


    /** Get the current FOV in degrees. */
    void GetFOV(float& fovX, float& fovY) const override {
        fovX = GetViewTransform().GetFovXDeg();
        fovY = GetViewTransform().GetFovYDeg();
    }


    /**
     * Set the current FOV in degrees.
     * Forces perspective rendering to be true.
     */
    void SetFOV(float fovX, float fovY) override {
        fView.SetFOV(fovX, fovY);
    }


    /** Get the orthogonal projection view size in world units (eg. feet). */
    void GetSize(float& width, float& height) const override {
        width = GetViewTransform().GetScreenWidth();
        height = GetViewTransform().GetScreenHeight();
    }


    /**
     * Set the orthogonal projection view size in world units (e.g. feet).
     * Forces projection to orthogonal if it wasn't.
     */
    void SetSize(float width, float height) override {
        fView.SetSize(width, height);
    }


    /** Get the current hither and yon. */
    void GetDepth(float& hither, float& yon) const override {
        GetViewTransform().GetDepth(hither, yon);
    }


    /** Set the current hither and yon. */
    void SetDepth(float hither, float yon) override {
        fView.SetDepth(hither, yon);
    }


    /**
     * If the board really doesn't support Z-biasing, we adjust the perspective
     * matrix in IGetCameraToNDC. The layer scale and translation are tailored
     * to the current hardware.
     */
    void SetZBiasScale(float scale) override;

    float GetZBiasScale() const override;


    /** Return current World to Camera transform. */
    const hsMatrix44& GetWorldToCamera() const override {
        return fView.GetWorldToCamera();
    }


    /** Return current Camera to World transform. */
    const hsMatrix44& GetCameraToWorld() const override {
        return fView.GetCameraToWorld();
    }

    /** Immediate set of camera transform. */
    void SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w) override;

    /**
     * Return current World to Local transform.
     * Note that this is only meaningful while an object is being rendered, so
     * this function is pretty worthless.
     */
    const hsMatrix44& GetWorldToLocal() const override {
        return fView.GetWorldToLocal();
    }


    /**
     * Return current Local to World transform.
     * Note that this is only meaningful while an object is being rendered, so
     * this function is pretty worthless.
     */
    const hsMatrix44& GetLocalToWorld() const override {
        return fView.GetLocalToWorld();
    }


    const plViewTransform& GetViewTransform() const override {
        return fView.GetConstViewTransform();
    }


    /**
     * Given a screen space pixel position, and a world space distance from
     * the camera, return a full world space position.
     * I.e. cast a ray through a screen pixel dist feet, and where is it.
     */
    void ScreenToWorldPoint(int n, uint32_t stride, int32_t* scrX, int32_t* scrY, float dist, uint32_t strideOut, hsPoint3* worldOut) override;


    /**
     * Force a refresh of cached state when the projection matrix changes.
     */
    void RefreshMatrices() override {
        RefreshScreenMatrices();
    }

    /**
     * Force a refresh of cached state when the projection matrix changes.
     */
    void RefreshScreenMatrices() override;


    /**
     * Push a material to be used instead of the material associated with
     * objects for rendering.
     * Must be matched with a PopOverrideMaterial.
     */
    hsGMaterial* PushOverrideMaterial(hsGMaterial* mat) override;


    /**
     * Stop overriding with the current override material.
     * Must match a preceding PushOverrideMaterial.
     */
    void PopOverrideMaterial(hsGMaterial* restore) override;


    /** Return the current override material, or nullptr if there isn't any. */
    hsGMaterial* GetOverrideMaterial() const override {
        return !fOverrideMat.empty() ? fOverrideMat.back() : nullptr;
    }


    /**
     * Set up a layer wrapper to wrap around either all layers rendered with
     * or just the base layers.
     * Note that a single material has multiple base layers if it takes
     * multiple passes to render.
     *
     * Stays in effect until removed by RemoveLayerInterface.
     */
    plLayerInterface* AppendLayerInterface(plLayerInterface* li, bool onAllLayers = false) override;


    /** Removes a layer wrapper installed by AppendLayerInterface. */
    plLayerInterface* RemoveLayerInterface(plLayerInterface* li, bool onAllLayers = false) override;

    /**
     * Return the current bits set to be always on for the given category
     * (e.g. ZFlags).
     */
    uint32_t GetMaterialOverrideOn(hsGMatState::StateIdx category) const override {
        return fMatOverOn.Value(category);
    }


    /**
     * Return the current bits set to be always off for the given category
     * (e.g. ZFlags).
     */
    uint32_t GetMaterialOverrideOff(hsGMatState::StateIdx category) const override {
        return fMatOverOff.Value(category);
    }


    /**
     * Force material state bits on or off.
     *
     * If you use this, save the return value as input to
     * PopMaterialOverride, to restore previous values.
     */
    hsGMatState PushMaterialOverride(const hsGMatState& state, bool on) override;


    /**
     * Force material state bits on or off.
     * This version just sets for one category (e.g. Z flags).
     *
     * If you use this, save the return value as input to
     * PopMaterialOverride, to restore previous values.
     */
    hsGMatState PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on) override;


    /**
     * Restore the previous settings returned from the matching
     * PushMaterialOverride.
     */
    void PopMaterialOverride(const hsGMatState& restore, bool on) override;


    /**
     * Return the current material state bits forced to on or off, depending
     * on input <on>.
     */
    const hsGMatState& GetMaterialOverride(bool on) const override {
        return on ? fMatOverOn : fMatOverOff;
    }


    /**
     * Puts the slave in a list valid for this frame only.
     * The list will be preprocessed at BeginRender.
     *
     * See IPreprocessShadows.
     */
    void SubmitShadowSlave(plShadowSlave* slave) override;

    void SubmitClothingOutfit(plClothingOutfit* co) override;

    //virtual bool SetGamma(float eR, float eG, float eB) = 0;
    //virtual bool SetGamma(const uint16_t* const tabR, const uint16_t* const tabG, const uint16_t* const tabB) = 0;
    //virtual bool CaptureScreen(plMipmap* dest, bool flipVertical = false, uint16_t desiredWidth = 0, uint16_t desiredHeight = 0) = 0;
    //virtual plMipmap* ExtractMipMap(plRenderTarget* targ) = 0;

    /** Return the current error string. */
    ST::string GetErrorString() override
    {
        return fDevice.GetErrorString();
    }

    //virtual void GetSupportedDisplayModes(std::vector<plDisplayMode> *res, int ColorDepth = 32 ) = 0;
    //virtual int GetMaxAnisotropicSamples() = 0;
    //virtual int GetMaxAntiAlias(int Width, int Height, int ColorDepth) = 0;
    //virtual void ResetDisplayDevice(int Width, int Height, int ColorDepth, bool Windowed, int NumAASamples, int MaxAnisotropicSamples, bool vSync = false  ) = 0;

    size_t GetViewStackSize() const override { return fViewStack.size(); }

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

    virtual void RenderSpans(plDrawableSpans* ice, const std::vector<int16_t>& visList) = 0;


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
    void IAttachSlaveToReceivers(size_t iSlave, plDrawableSpans* drawable, const std::vector<int16_t>& visList);


    /**
     * For each active shadow map (in fShadows), attach it to all of the
     * visible spans in drawable that it affects. Shadows explicitly attached
     * via light groups are handled separately in ISetShadowFromGroup.
     */
    void IAttachShadowsToReceivers(plDrawableSpans* drawable, const std::vector<int16_t>& visList);


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
     * At EndRender(), we need to clear our list of shadow slaves.
     * They are only valid for one frame.
     */
    void IClearShadowSlaves();

    void IClearClothingOutfits(std::vector<plClothingOutfit*>* outfits);
    void IFillAvRTPool();

    /**
     * Returns true if we successfully filled the pool. Otherwise cleans up.
     */
    bool IFillAvRTPool(uint16_t numRTs, uint16_t width);

    void IReleaseAvRTPool();
    plRenderTarget* IGetNextAvRT();
    void IFreeAvRT(plRenderTarget* tex);

    /**
     * Sets fOverBaseLayer (if any) as a wrapper on top of input layer.
     * This allows the OverBaseLayer to intercept and modify queries of
     * the real current layer's properties (e.g. color or state).
     * fOverBaseLayer is set to only get applied to the base layer during
     * multitexturing.
     *
     * Must be matched with call to IPopOverBaseLayer.
     */
    plLayerInterface* IPushOverBaseLayer(plLayerInterface* li);

    /**
     * Removes fOverBaseLayer as wrapper on top of input layer.
     *
     * Should match calls to IPushOverBaseLayer.
     */
    plLayerInterface* IPopOverBaseLayer(plLayerInterface* li);

    /**
     * Push fOverAllLayer (if any) as wrapper around the input layer.
     *
     * fOverAllLayer is set to be applied to each layer during multitexturing.
     *
     * Must be matched by call to IPopOverAllLayer
     */
    plLayerInterface* IPushOverAllLayer(plLayerInterface* li);

    /**
     * Remove fOverAllLayer as wrapper on top of input layer.
     *
     * Should match calls to IPushOverAllLayer.
     */
    plLayerInterface* IPopOverAllLayer(plLayerInterface* li);

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
    void ICheckLighting(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr);


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

    /**
     * Sorts the avatar geometry for display.
     *
     * We handle avatar sort differently from the rest of the face sort. The
     * reason is that within the single avatar index buffer, we want to only
     * sort the faces of spans requesting a sort, and sort them in place.
     *
     * Contrast that with the normal scene translucency sort. There, we sort
     * all the spans in a drawble, then we sort all the faces in that drawable,
     * then for each span in the sorted span list, we extract the faces for
     * that span appending onto the index buffer. This gives great efficiency
     * because only the visible faces are sorted and they wind up packed into
     * the front of the index buffer, which permits more batching. See
     * plDrawableSpans::SortVisibleSpans.
     *
     * For the avatar, it's generally the case that all the avatar is visible
     * or not, and there is only one material, so neither of those efficiencies
     * is helpful. Moreover, for the avatar the faces we want sorted are a tiny
     * subset of the avatar's faces. Moreover, and most importantly, for the
     * avatar, we want to preserve the order that spans are drawn, so, for
     * example, the opaque base head will always be drawn before the
     * translucent hair fringe, which will always be drawn before the pink
     * clear plastic baseball cap.
     */
    bool IAvatarSort(plDrawableSpans* d, const std::vector<int16_t>& visList);
};


template<class DeviceType>
pl3DPipeline<DeviceType>::pl3DPipeline(const hsG3DDeviceModeRecord* devModeRec)
:   fMaxLayersAtOnce(-1),
    fMaxPiggyBacks(),
    //fMaxNumLights(kD3DMaxTotalLights),
    //fMaxNumProjectors(kMaxProjectors),
    fOverBaseLayer(),
    fOverAllLayer(),
    fMatPiggyBacks(),
    fActivePiggyBacks(),
    fVtxBuffRefList(),
    fIdxBuffRefList(),
    fTextureRefList(),
    fTextFontRefList(),
    fCurrMaterial(),
    fCurrLay(),
    fCurrNumLayers(),
    fCurrRenderLayer(),
    fCurrLightingMethod(plSpan::kLiteMaterial),
    fActiveLights(),
    fCurrRenderTarget(),
    fCurrBaseRenderTarget(),
    fCurrRenderTargetRef(),
    fColorDepth(32),
    fInSceneDepth(),
    fTime(),
    fFrame(),
    fRenderCnt(),
    fVtxRefTime(),
    fVSync(),
    fPlateMgr(),
    fDebugTextMgr(),
    fAvRTShrinkValidSince(),
    fAvRTWidth(1024),
    fAvNextFreeRT()
{

    fOverLayerStack.clear();
    fPiggyBackStack.clear();

    fMatOverOn.Reset();
    fMatOverOff.Reset();

    for (int i = 0; i < 8; i++) {
        fLayerRef[i] = nullptr;
    }

    fTweaks.Reset();
    fView.Reset(this);
    fDebugFlags.Clear();


    // Get the requested mode and setup
    const hsG3DDeviceMode *devMode = devModeRec->GetMode();

    if (!fInitialPipeParams.Windowed) {
        fOrigWidth = devMode->GetWidth();
        fOrigHeight = devMode->GetHeight();
    } else {
        // windowed can run in any mode
        fOrigHeight = fInitialPipeParams.Height;
        fOrigWidth = fInitialPipeParams.Width;
    }

    IGetViewTransform().SetScreenSize(uint16_t(fOrigWidth), uint16_t(fOrigHeight));
    fColorDepth = devMode->GetColorDepth();

    fVSync = fInitialPipeParams.VSync;
}

template<class DeviceType>
pl3DPipeline<DeviceType>::~pl3DPipeline()
{
    fCurrLay = nullptr;
    hsAssert(fCurrMaterial == nullptr, "Current material not unrefed properly");

    // CullProxy is a debugging representation of our CullTree. See plCullTree.cpp, 
    // plScene/plOccluder.cpp and plScene/plOccluderProxy.cpp for more info
    if (fView.HasCullProxy())
        fView.GetCullProxy()->GetKey()->UnRefObject();

    // Tell the light infos to unlink themselves
    while (fActiveLights)
        UnRegisterLight(fActiveLights);

    IReleaseAvRTPool();
    IClearClothingOutfits(&fClothingOutfits);
    IClearClothingOutfits(&fPrevClothingOutfits);
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::Render(plDrawable* d, const std::vector<int16_t>& visList)
{
    // Reset here, since we can push/pop renderTargets after BeginRender() but
    // before this function, which necessitates this being called
    if (fView.fXformResetFlags != 0)
        ITransformsToDevice();

    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(d);

    if (ds)
        RenderSpans(ds, visList);
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::Draw(plDrawable* d)
{
    plDrawableSpans *ds = plDrawableSpans::ConvertNoRef(d);

    if (ds) {
        if (( ds->GetType() & fView.GetDrawableTypeMask()) == 0)
            return;

        static std::vector<int16_t> visList;

        PreRender(ds, visList);
        PrepForRender(ds, visList);
        Render(ds, visList);
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::CheckVertexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    // First, do we have a device ref at this index?
    typename DeviceType::VertexBufferRef* vRef = static_cast<typename DeviceType::VertexBufferRef*>(owner->GetVertexBufferRef(idx));

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
    }
    else
    {
        // Make sure we're going to be ready to fill it.
        if (!vRef->fData && (vRef->fFormat != owner->GetVertexFormat())) {
            vRef->fData = new uint8_t[vRef->fCount * vRef->fVertexSize];
            fDevice.FillVolatileVertexBufferRef(vRef, owner, idx);
        }
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::CheckIndexBufferRef(plGBufferGroup* owner, uint32_t idx)
{
    typename DeviceType::IndexBufferRef* iRef = static_cast<typename DeviceType::IndexBufferRef*>(owner->GetIndexBufferRef(idx));

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

template<class DeviceType>
void pl3DPipeline<DeviceType>::CheckTextureRef(plLayerInterface* layer)
{
    plBitmap* bitmap = layer->GetTexture();

    if (bitmap) {
        typename DeviceType::TextureRef* tRef = static_cast<typename DeviceType::TextureRef*>(bitmap->GetDeviceRef());

        if (!tRef) {
            tRef = new typename DeviceType::TextureRef();
            fDevice.SetupTextureRef(layer, bitmap, tRef);
        }

        if (!tRef->IsLinked())
            tRef->Link(&fTextureRefList);

        // Make sure it has all resources created.
        fDevice.CheckTexture(tRef);

        // If it's dirty, refill it.
        if (tRef->IsDirty()) {
            plMipmap* mip = plMipmap::ConvertNoRef(bitmap);
            if (mip) {
                fDevice.MakeTextureRef(tRef, layer, mip);
                return;
            }

            plCubicEnvironmap* cubic = plCubicEnvironmap::ConvertNoRef(bitmap);
            if (cubic) {
                fDevice.MakeCubicTextureRef(tRef, layer, cubic);
                return;
            }
        }
    }
}

template<class DeviceType>
void pl3DPipeline<DeviceType>::CheckTextureRef(plBitmap* bitmap)
{
    typename DeviceType::TextureRef* tRef = static_cast<typename DeviceType::TextureRef*>(bitmap->GetDeviceRef());

    if (!tRef) {
        tRef = new typename DeviceType::TextureRef();
        fDevice.SetupTextureRef(nullptr, bitmap, tRef);
    }

    if (!tRef->IsLinked())
        tRef->Link(&fTextureRefList);

    // Make sure it has all resources created.
    fDevice.CheckTexture(tRef);

    // If it's dirty, refill it.
    if (tRef->IsDirty()) {
        plMipmap* mip = plMipmap::ConvertNoRef(bitmap);
        if (mip) {
            fDevice.MakeTextureRef(tRef, nullptr, mip);
            return;
        }

        plCubicEnvironmap* cubic = plCubicEnvironmap::ConvertNoRef(bitmap);
        if (cubic) {
            fDevice.MakeCubicTextureRef(tRef, nullptr, cubic);
            return;
        }
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::RegisterLight(plLightInfo* liInfo)
{
    if (liInfo->IsLinked())
        return;

    liInfo->Link(&fActiveLights);

    // Override this method to set the light's native device ref!
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::UnRegisterLight(plLightInfo* liInfo)
{
    liInfo->SetDeviceRef(nullptr);
    liInfo->Unlink();
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::PushRenderTarget(plRenderTarget* target)
{
    fCurrRenderTarget = target;
    hsRefCnt_SafeAssign(fCurrRenderTargetRef, (target != nullptr) ? target->GetDeviceRef() : nullptr);

    while (target != nullptr) {
        fCurrBaseRenderTarget = target;
        target = target->GetParent();
    }

    fRenderTargets.push_back(fCurrRenderTarget);
    fDevice.SetRenderTarget(fCurrRenderTarget);
}


template<class DeviceType>
plRenderTarget* pl3DPipeline<DeviceType>::PopRenderTarget()
{
    plRenderTarget* old = fRenderTargets.back();
    fRenderTargets.pop_back();
    plRenderTarget* temp;
    size_t i = fRenderTargets.size();

    if (i == 0) {
        fCurrRenderTarget = nullptr;
        fCurrBaseRenderTarget = nullptr;
        hsRefCnt_SafeUnRef(fCurrRenderTargetRef);
        fCurrRenderTargetRef = nullptr;
    } else {
        fCurrRenderTarget = fRenderTargets[i - 1];
        temp = fCurrRenderTarget;

        while (temp != nullptr) {
            fCurrBaseRenderTarget = temp;
            temp = temp->GetParent();
        }

        hsRefCnt_SafeAssign(fCurrRenderTargetRef, (fCurrRenderTarget != nullptr) ? fCurrRenderTarget->GetDeviceRef() : nullptr);
    }

    fDevice.SetRenderTarget(fCurrRenderTarget);

    return old;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::BeginVisMgr(plVisMgr* visMgr)
{
    // Make Light Lists /////////////////////////////////////////////////////
    // Look through all the current lights, and fill out two lists.
    // Only active lights (not disabled, not exactly black, and not
    // ignored because of visibility regions by plVisMgr) will
    // be considered.
    // The first list is lights that will affect the avatar and similar
    // indeterminately mobile (physical) objects - fCharLights.
    // The second list is lights that aren't restricted by light include
    // lists.
    // These two abbreviated lists will be further refined for each object
    // and avatar to find the strongest 8 lights which affect that object.
    // A light with an include list, or LightGroup Component) has
    // been explicitly told which objects it affects, so they don't
    // need to be in the search lists.
    // These lists are only constructed once per render, but searched
    // multiple times

    plProfile_BeginTiming(FindSceneLights);
    fCharLights.clear();
    fVisLights.clear();
    if (visMgr) {
        const hsBitVector& visSet = visMgr->GetVisSet();
        const hsBitVector& visNot = visMgr->GetVisNot();
        plLightInfo* light;

        for (light = fActiveLights; light != nullptr; light = light->GetNext()) {
            plProfile_IncCount(LightActive, 1);
            if (!light->IsIdle() && !light->InVisNot(visNot) && light->InVisSet(visSet)) {
                plProfile_IncCount(LightOn, 1);
                if (light->GetProperty(plLightInfo::kLPHasIncludes)) {
                    if (light->GetProperty(plLightInfo::kLPIncludesChars))
                        fCharLights.emplace_back(light);
                } else {
                    fVisLights.emplace_back(light);
                    fCharLights.emplace_back(light);
                }
            }
        }
    } else {
        plLightInfo* light;
        for (light = fActiveLights; light != nullptr; light = light->GetNext()) {
            plProfile_IncCount(LightActive, 1);
            if (!light->IsIdle()) {
                plProfile_IncCount(LightOn, 1);
                if (light->GetProperty(plLightInfo::kLPHasIncludes)) {
                    if (light->GetProperty(plLightInfo::kLPIncludesChars))
                        fCharLights.emplace_back(light);
                } else {
                    fVisLights.emplace_back(light);
                    fCharLights.emplace_back(light);
                }
            }
        }
    }
    plProfile_IncCount(LightVis, fVisLights.size());
    plProfile_IncCount(LightChar, fCharLights.size());

    plProfile_EndTiming(FindSceneLights);
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::EndVisMgr(plVisMgr* visMgr)
{
    fCharLights.clear();
    fVisLights.clear();
}


template<class DeviceType>
bool pl3DPipeline<DeviceType>::CheckResources()
{
    if ((fClothingOutfits.size() <= 1 && fAvRTPool.size() > 1) ||
        (fAvRTPool.size() >= 16 && (fAvRTPool.size() / 2 >= fClothingOutfits.size())))
    {
        return (hsTimer::GetSysSeconds() - fAvRTShrinkValidSince > kAvTexPoolShrinkThresh);
    }

    fAvRTShrinkValidSince = hsTimer::GetSysSeconds();
    return (fAvRTPool.size() < fClothingOutfits.size());
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::SetZBiasScale(float scale)
{
    scale += 1.0f;
    fTweaks.fPerspLayerScale = fTweaks.fDefaultPerspLayerScale * scale;
    fTweaks.fPerspLayerTrans = kPerspLayerTrans * scale;
}


template<class DeviceType>
float pl3DPipeline<DeviceType>::GetZBiasScale() const
{
    return (fTweaks.fPerspLayerScale / fTweaks.fDefaultPerspLayerScale) - 1.0f;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::SetWorldToCamera(const hsMatrix44& w2c, const hsMatrix44& c2w)
{
    plViewTransform& view_xform = fView.GetViewTransform();

    view_xform.SetCameraTransform(w2c, c2w);

    fView.fCullTreeDirty = true;
    fView.fWorldToCamLeftHanded = fView.GetWorldToCamera().GetParity();

    IWorldToCameraToDevice();
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::ScreenToWorldPoint(int n, uint32_t stride, int32_t* scrX, int32_t* scrY, float dist, uint32_t strideOut, hsPoint3* worldOut)
{
    while (n--) {
        hsPoint3 scrP;
        scrP.Set(float(*scrX++), float(*scrY++), float(dist));
        *worldOut++ = GetViewTransform().ScreenToWorld(scrP);
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::RefreshScreenMatrices()
{
    fView.fCullTreeDirty = true;
    IProjectionMatrixToDevice();
}


template<class DeviceType>
hsGMaterial* pl3DPipeline<DeviceType>::PushOverrideMaterial(hsGMaterial* mat)
{
    hsGMaterial* ret = GetOverrideMaterial();
    hsRefCnt_SafeRef(mat);
    fOverrideMat.push_back(mat);
    fForceMatHandle = true;

    return ret;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::PopOverrideMaterial(hsGMaterial* restore)
{
    hsGMaterial *pop = fOverrideMat.back();
    fOverrideMat.pop_back();
    hsRefCnt_SafeUnRef(pop);

    if (fCurrMaterial == pop)
        fForceMatHandle = true;
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::AppendLayerInterface(plLayerInterface* li, bool onAllLayers)
{
    fForceMatHandle = true;
    if (onAllLayers)
        return fOverAllLayer = li->Attach(fOverAllLayer);
    else
        return fOverBaseLayer = li->Attach(fOverBaseLayer);
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::RemoveLayerInterface(plLayerInterface* li, bool onAllLayers)
{
    fForceMatHandle = true;

    if (onAllLayers) {
        if (!fOverAllLayer)
            return nullptr;
        return fOverAllLayer = fOverAllLayer->Remove(li);
    }

    if (!fOverBaseLayer)
        return nullptr;

    return fOverBaseLayer = fOverBaseLayer->Remove(li);
}


template<class DeviceType>
hsGMatState pl3DPipeline<DeviceType>::PushMaterialOverride(const hsGMatState& state, bool on)
{
    hsGMatState ret = GetMaterialOverride(on);
    if (on) {
        fMatOverOn |= state;
        fMatOverOff -= state;
    } else {
        fMatOverOff |= state;
        fMatOverOn -= state;
    }
    fForceMatHandle = true;
    return ret;
}


template<class DeviceType>
hsGMatState pl3DPipeline<DeviceType>::PushMaterialOverride(hsGMatState::StateIdx cat, uint32_t which, bool on)
{
    hsGMatState ret = GetMaterialOverride(on);
    if (on) {
        fMatOverOn[cat] |= which;
        fMatOverOff[cat] &= ~which;
    } else {
        fMatOverOn[cat] &= ~which;
        fMatOverOff[cat] |= which;
    }
    fForceMatHandle = true;
    return ret;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::PopMaterialOverride(const hsGMatState& restore, bool on)
{
    if (on) {
        fMatOverOn = restore;
        fMatOverOff.Clear(restore);
    } else {
        fMatOverOff = restore;
        fMatOverOn.Clear(restore);
    }
    fForceMatHandle = true;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::SubmitShadowSlave(plShadowSlave* slave)
{
    // Check that it's a valid slave.
    if (!(slave && slave->fCaster && slave->fCaster->GetKey()))
        return;

    // Ref the shadow caster so we're sure it will still be around when we go to
    // render it.
    slave->fCaster->GetKey()->RefObject();

    // Keep the shadow slaves in a priority sorted list. For performance reasons,
    // we may want only the strongest N or those of a minimum priority.
    size_t i;
    for (i = 0; i < fShadows.size(); i++) {
        if (slave->fPriority <= fShadows[i]->fPriority)
            break;
    }

    // Note that fIndex is no longer the index in the fShadows list, but
    // is still used as a unique identifier for this slave.
    slave->fIndex = fShadows.size();
    fShadows.insert(fShadows.begin() + i, slave);
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::SubmitClothingOutfit(plClothingOutfit* co)
{
    auto iter = std::find(fClothingOutfits.cbegin(), fClothingOutfits.cend(), co);

    if (iter == fClothingOutfits.cend()) {
        fClothingOutfits.emplace_back(co);
        auto prevIter = std::find(fPrevClothingOutfits.cbegin(), fPrevClothingOutfits.cend(), co);
        if (prevIter != fPrevClothingOutfits.cend())
            fPrevClothingOutfits.erase(prevIter);
        else
            co->GetKey()->RefObject();
    }
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::PushPiggyBackLayer(plLayerInterface* li)
{
    fPiggyBackStack.push_back(li);

    fActivePiggyBacks = std::min(static_cast<size_t>(fMaxPiggyBacks), fPiggyBackStack.size());

    fForceMatHandle = true;

    return li;
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::PopPiggyBackLayer(plLayerInterface* li)
{
    auto iter = std::find(fPiggyBackStack.cbegin(), fPiggyBackStack.cend(), li);
    if (iter == fPiggyBackStack.cend())
        return nullptr;

    fPiggyBackStack.erase(iter);

    fActivePiggyBacks = std::min(static_cast<size_t>(fMaxPiggyBacks), fPiggyBackStack.size());

    fForceMatHandle = true;

    return li;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::SetViewTransform(const plViewTransform& v)
{
    fView.SetViewTransform(v);

    if (!v.GetScreenWidth() || !v.GetScreenHeight())
        fView.GetViewTransform().SetScreenSize(uint16_t(fOrigWidth), uint16_t(fOrigHeight));

    fView.fCullTreeDirty = true;
    fView.fWorldToCamLeftHanded = fView.GetWorldToCamera().GetParity();

    IWorldToCameraToDevice();
}




/*** PROTECTED METHODS *******************************************************/

template<class DeviceType>
void pl3DPipeline<DeviceType>::IAttachSlaveToReceivers(size_t which, plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    plShadowSlave* slave = fShadows[which];

    // Whether the drawable is a character affects which lights/shadows affect it.
    bool isChar = drawable->GetNativeProperty(plDrawable::kPropCharacter);

    // If the shadow is part of a light group, it gets handled in ISetShadowFromGroup.
    // Unless the drawable is a character (something that moves around indeterminately,
    // like the avatar or a physical object), and the shadow affects all characters.
    if (slave->ObeysLightGroups() && !(slave->IncludesChars() && isChar))
        return;

    // Do a space tree harvest looking for spans that are visible and whose bounds
    // intercect the shadow volume.
    plSpaceTree* space = drawable->GetSpaceTree();

    static hsBitVector cache;
    cache.Clear();
    space->EnableLeaves(visList, cache);

    static std::vector<int16_t> hitList;
    hitList.clear();
    space->HarvestEnabledLeaves(slave->fIsect, cache, hitList);

    // For the visible spans that intercect the shadow volume, attach the shadow
    // to all appropriate for receiving this shadow map.
    for (int16_t idx : hitList) {
        const plSpan* span = drawable->GetSpan(idx);
        hsGMaterial* mat = drawable->GetMaterial(span->fMaterialIdx);

        // Check that the span isn't flagged as unshadowable, or has
        // a material that we can't shadow onto.
        if (!IReceivesShadows(span, mat))
            continue;

        // Check for self shadowing. If the shadow doesn't want self shadowing,
        // and the span is part of the shadow caster, then skip.
        if (!IAcceptsShadow(span, slave))
            continue;

        // Add it to this span's shadow list for this frame.
        span->AddShadowSlave(fShadows[which]->fIndex);
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IAttachShadowsToReceivers(plDrawableSpans* drawable, const std::vector<int16_t>& visList)
{
    for (size_t i = 0; i < fShadows.size(); i++)
        IAttachSlaveToReceivers(i, drawable, visList);
}


template<class DeviceType>
bool pl3DPipeline<DeviceType>::IAcceptsShadow(const plSpan* span, plShadowSlave* slave)
{
    // The span's shadow bits records which shadow maps that span was rendered
    // into.
    return slave->SelfShadow() || !span->IsShadowBitSet(slave->fIndex);
}


template<class DeviceType>
bool pl3DPipeline<DeviceType>::IReceivesShadows(const plSpan* span, hsGMaterial* mat)
{
    if (span->fProps & plSpan::kPropNoShadow)
        return false;

    if (span->fProps & plSpan::kPropForceShadow)
        return true;

    if (span->fProps & (plSpan::kPropSkipProjection | plSpan::kPropProjAsVtx))
        return false;

    if ((fMaxLayersAtOnce < 3) &&
        (mat->GetLayer(0)->GetTexture()) &&
        (mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendAlpha))
        return false;

    return true;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::ISetShadowFromGroup(plDrawableSpans* drawable, const plSpan* span, plLightInfo* liInfo)
{
    hsGMaterial* mat = drawable->GetMaterial(span->fMaterialIdx);

    // Check that this span/material combo can receive shadows at all.
    if (!IReceivesShadows(span, mat))
        return;

    const hsBitVector& slaveBits = liInfo->GetSlaveBits();

    for (plShadowSlave* shadow : fShadows) {
        if (slaveBits.IsBitSet(shadow->fIndex)) {
            // Check self shadowing.
            if (IAcceptsShadow(span, shadow)) {
                // Check for overlapping bounds.
                if (shadow->fIsect->Test(span->fWorldBounds) != kVolumeCulled)
                    span->AddShadowSlave(shadow->fIndex);
            }
        }
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IClearShadowSlaves()
{
    for (plShadowSlave* shadow : fShadows)
    {
        const plShadowCaster* caster = shadow->fCaster;
        caster->GetKey()->UnRefObject();
    }
    fShadows.clear();
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IClearClothingOutfits(std::vector<plClothingOutfit*>* outfits)
{
    while (!outfits->empty()) {
        plClothingOutfit *co = outfits->back();
        outfits->pop_back();
        IFreeAvRT((plRenderTarget*)co->fTargetLayer->GetTexture());
        co->fTargetLayer->SetTexture(nullptr);
        co->GetKey()->UnRefObject();
    }
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IFillAvRTPool()
{
    fAvNextFreeRT = 0;
    fAvRTShrinkValidSince = hsTimer::GetSysSeconds();
    size_t numRTs = 1;

    if (fClothingOutfits.size() > 1) {
        // Just jump to 8 for starters so we don't have to refresh for the 2nd, 4th, AND 8th player
        numRTs = 8;
        while (numRTs < fClothingOutfits.size())
            numRTs *= 2;
    }

    // I could see a 32MB video card going down to 64x64 RTs in extreme cases
    // (over 100 players onscreen at once), but really, if such hardware is ever trying to push
    // that, the low texture resolution is not going to be your major concern.
    for (fAvRTWidth = 1024 >> plMipmap::GetGlobalLevelChopCount(); fAvRTWidth >= 32; fAvRTWidth /= 2) {
        if (IFillAvRTPool(numRTs, fAvRTWidth))
            return;

        // Nope? Ok, lower the resolution and try again.
    }
}


template<class DeviceType>
bool pl3DPipeline<DeviceType>::IFillAvRTPool(uint16_t numRTs, uint16_t width)
{
    fAvRTPool.resize(numRTs);
    for (uint16_t i = 0; i < numRTs; i++)
    {
        uint16_t flags = plRenderTarget::kIsTexture | plRenderTarget::kIsProjected;
        uint8_t bitDepth = 32;
        uint8_t zDepth = 0;
        uint8_t stencilDepth = 0;
        fAvRTPool[i] = new plRenderTarget(flags, width, width, bitDepth, zDepth, stencilDepth);

        // If anyone fails, release everyone we've created.
        if (!MakeRenderTargetRef(fAvRTPool[i]))
        {
            for (uint16_t j = 0; j <= i; j++)
            {
                delete fAvRTPool[j];
            }
            return false;
        }
    }
    return true;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IReleaseAvRTPool()
{
    for (plClothingOutfit* outfit : fClothingOutfits)
        outfit->fTargetLayer->SetTexture(nullptr);

    for (plClothingOutfit* outfit : fPrevClothingOutfits)
        outfit->fTargetLayer->SetTexture(nullptr);

    for (plRenderTarget* avRT : fAvRTPool)
        delete(avRT);

    fAvRTPool.clear();
}


template<class DeviceType>
plRenderTarget *pl3DPipeline<DeviceType>::IGetNextAvRT()
{
    return fAvRTPool[fAvNextFreeRT++];
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IFreeAvRT(plRenderTarget* tex)
{
    auto iter = std::find(fAvRTPool.begin(), fAvRTPool.end(), tex);
    if (iter != fAvRTPool.end()) {
        hsAssert(iter - fAvRTPool.begin() < fAvNextFreeRT, "Freeing an avatar RT that's already free?");
        *iter = fAvRTPool[fAvNextFreeRT - 1];
        fAvRTPool[fAvNextFreeRT - 1] = tex;
        fAvNextFreeRT--;
    }
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::IPushOverBaseLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fOverLayerStack.push_back(li);

    if (!fOverBaseLayer)
        return fOverBaseLayer = li;

    fForceMatHandle = true;
    fOverBaseLayer = fOverBaseLayer->Attach(li);
    fOverBaseLayer->Eval(fTime, fFrame, 0);
    return fOverBaseLayer;
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::IPopOverBaseLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverBaseLayer = fOverBaseLayer->Detach(pop);

    return pop;
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::IPushOverAllLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fOverLayerStack.push_back(li);

    if (!fOverAllLayer) {
        fOverAllLayer = li;
        fOverAllLayer->Eval(fTime, fFrame, 0);
        return fOverAllLayer;
    }

    fForceMatHandle = true;
    fOverAllLayer = fOverAllLayer->Attach(li);
    fOverAllLayer->Eval(fTime, fFrame, 0);

    return fOverAllLayer;
}


template<class DeviceType>
plLayerInterface* pl3DPipeline<DeviceType>::IPopOverAllLayer(plLayerInterface* li)
{
    if (!li)
        return nullptr;

    fForceMatHandle = true;

    plLayerInterface* pop = fOverLayerStack.back();
    fOverLayerStack.pop_back();
    fOverAllLayer = fOverAllLayer->Detach(pop);

    return pop;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::ICheckLighting(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    if (fView.fRenderState & kRenderNoLights)
        return;

    if (visList.empty())
        return;

    plProfile_BeginTiming(FindLights);

    // First add in the explicit lights (from LightGroups).
    // Refresh the lights as they are added (actually a lazy eval).
    plProfile_BeginTiming(FindPerm);
    for (int16_t idx : visList) {
        drawable->GetSpan(idx)->ClearLights();

        if (IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights))
            continue;

        // Set the bits for the lights added from the permanent lists (during ClearLights()).
        const std::vector<plLightInfo*>& permaLights = drawable->GetSpan(idx)->fPermaLights;
        for (plLightInfo* permaLight : permaLights) {
            permaLight->Refresh();
            // If it casts a shadow, attach the shadow now.
            if (permaLight->GetProperty(plLightInfo::kLPShadowLightGroup) && !permaLight->IsIdle())
                ISetShadowFromGroup(drawable, drawable->GetSpan(idx), permaLight);
        }

        const std::vector<plLightInfo*>& permaProjs = drawable->GetSpan(idx)->fPermaProjs;
        for (plLightInfo* permaProj : permaProjs) {
            permaProj->Refresh();
            // If it casts a shadow, attach the shadow now.
            if (permaProj->GetProperty(plLightInfo::kLPShadowLightGroup) && !permaProj->IsIdle())
                ISetShadowFromGroup(drawable, drawable->GetSpan(idx), permaProj);
        }
    }
    plProfile_EndTiming(FindPerm);

    if (IsDebugFlagSet(plPipeDbg::kFlagNoRuntimeLights)) {
        plProfile_EndTiming(FindLights);
        return;
    }

    // Sort the incoming spans as either
    // A) moving - affected by all lights - moveList
    // B) specular - affected by specular lights - specList
    // C) visible - affected by moving lights - visList
    static std::vector<int16_t> tmpList;
    static std::vector<int16_t> moveList;
    static std::vector<int16_t> specList;

    moveList.clear();
    specList.clear();

    plProfile_BeginTiming(FindSpan);
    for (int16_t idx : visList) {
        const plSpan* span = drawable->GetSpan(idx);

        if (span->fProps & plSpan::kPropRunTimeLight) {
            moveList.emplace_back(idx);
            specList.emplace_back(idx);
        } else if (span->fProps & plSpan::kPropMatHasSpecular) {
            specList.emplace_back(idx);
        }
    }
    plProfile_EndTiming(FindSpan);

    // Make a list of lights that can potentially affect spans in this drawable
    // based on the drawables bounds and properties.
    // If the drawable has the PropCharacter property, it is affected by lights
    // in fCharLights, else only by the smaller list of fVisLights.

    plProfile_BeginTiming(FindActiveLights);
    static std::vector<plLightInfo*> lightList;
    lightList.clear();

    if (drawable->GetNativeProperty(plDrawable::kPropCharacter)) {
        for (plLightInfo* charLight : fCharLights) {
            if (charLight->AffectsBound(drawable->GetSpaceTree()->GetWorldBounds()))
                lightList.emplace_back(charLight);
        }
    } else {
        for (plLightInfo* visLight : fVisLights) {
            if (visLight->AffectsBound(drawable->GetSpaceTree()->GetWorldBounds()))
                lightList.emplace_back(visLight);
        }
    }
    plProfile_EndTiming(FindActiveLights);

    // Loop over the lights and for each light, extract a list of the spans that light
    // affects. Append the light to each spans list with a scalar strength of how strongly
    // the light affects it. Since the strength is based on the object's center position,
    // it's not very accurate, but good enough for selecting which lights to use.

    plProfile_BeginTiming(ApplyActiveLights);
    for (plLightInfo* light : lightList) {
        tmpList.clear();
        if (light->GetProperty(plLightInfo::kLPMovable)) {
            plProfile_BeginTiming(ApplyMoving);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                visList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList) {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection))) {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0) {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyMoving);
        } else if (light->GetProperty(plLightInfo::kLPHasSpecular)) {
            if (specList.empty())
                continue;

            plProfile_BeginTiming(ApplyToSpec);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                specList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList) {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection))) {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0) {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyToSpec);
        } else {
            if (moveList.empty())
                continue;

            plProfile_BeginTiming(ApplyToMoving);

            const std::vector<int16_t>& litList = light->GetAffected(drawable->GetSpaceTree(),
                moveList,
                tmpList,
                drawable->GetNativeProperty(plDrawable::kPropCharacter));

            // PUT OVERRIDE FOR KILLING PROJECTORS HERE!!!!
            bool proj = nullptr != light->GetProjection();
            if (fView.fRenderState & kRenderNoProjection)
                proj = false;

            for (int16_t litIdx : litList) {
                // Use the light IF light is enabled and
                //      1) light is movable
                //      2) span is movable, or
                //      3) Both the light and the span have specular
                const plSpan* span = drawable->GetSpan(litIdx);
                bool currProj = proj;

                if (span->fProps & plSpan::kPropProjAsVtx)
                    currProj = false;

                if (!(currProj && (span->fProps & plSpan::kPropSkipProjection))) {
                    float strength, scale;

                    light->GetStrengthAndScale(span->fWorldBounds, strength, scale);

                    // We can't pitch a light because it's "strength" is zero, because the strength is based
                    // on the center of the span and isn't conservative enough. We can pitch based on the
                    // scale though, since a light scaled down to zero will have no effect no where.
                    if (scale > 0) {
                        plProfile_Inc(FindLightsFound);
                        span->AddLight(light, strength, scale, currProj);
                    }
                }
            }
            plProfile_EndTiming(ApplyToMoving);
        }
    }
    plProfile_EndTiming(ApplyActiveLights);

    IAttachShadowsToReceivers(drawable, visList);

    plProfile_EndTiming(FindLights);
}


template<class DeviceType>
hsMatrix44 pl3DPipeline<DeviceType>::IGetCameraToNDC()
{
    hsMatrix44 cam2ndc = GetViewTransform().GetCameraToNDC();

    if (fView.IsPerspective()) {
        // Want to scale down W and offset in Z without
        // changing values of x/w, y/w. This is just
        // minimal math for
        // Mproj' * p = Mscaletrans * Mproj * p
        // where Mscaletrans =
        // [ s 0 0 0 ]
        // [ 0 s 0 0 ]
        // [ 0 0 s 0 ]
        // [ 0 0 t s ]
        // Resulting matrix Mproj' is not exactly "Fog Friendly",
        // but is close enough.
        // Resulting point is [sx, sy, sz + tw, sw] and after divide
        // is [x/w, y/w, z/w + t/s, 1/sw]

        float scale = 1.f - float(fCurrRenderLayer) * fTweaks.fPerspLayerScale;
        float zTrans = -scale * float(fCurrRenderLayer) * fTweaks.fPerspLayerTrans;

        cam2ndc.fMap[0][0] *= scale;
        cam2ndc.fMap[1][1] *= scale;

        cam2ndc.fMap[2][2] *= scale;
        cam2ndc.fMap[2][2] += zTrans * cam2ndc.fMap[3][2];
        cam2ndc.fMap[3][2] *= scale;
    } else {
        plConst(float) kZTrans = -1.e-4f;
        cam2ndc.fMap[2][3] += kZTrans * fCurrRenderLayer;
    }

    return cam2ndc;
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::ISetLocalToWorld(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    fView.SetLocalToWorld(l2w);
    fView.SetWorldToLocal(w2l);

    fView.fViewVectorsDirty = true;

    // We keep track of parity for winding order culling.
    fView.fLocalToWorldLeftHanded = fView.GetLocalToWorld().GetParity();

    ILocalToWorldToDevice();
}



template<class DeviceType>
void pl3DPipeline<DeviceType>::ITransformsToDevice()
{
    if (fView.fXformResetFlags & fView.kResetCamera)
        IWorldToCameraToDevice();

    if (fView.fXformResetFlags & fView.kResetL2W)
        ILocalToWorldToDevice();

    if (fView.fXformResetFlags & fView.kResetProjection)
        IProjectionMatrixToDevice();
}


template<class DeviceType>
void pl3DPipeline<DeviceType>::IProjectionMatrixToDevice()
{
    fDevice.SetProjectionMatrix(IGetCameraToNDC());
    fView.fXformResetFlags &= ~fView.kResetProjection;
}

template<class DeviceType>
void pl3DPipeline<DeviceType>::IWorldToCameraToDevice()
{
    fDevice.SetWorldToCameraMatrix(fView.GetWorldToCamera());
    fView.fXformResetFlags &= ~fView.kResetCamera;

    fFrame++;
}

template<class DeviceType>
void pl3DPipeline<DeviceType>::ILocalToWorldToDevice()
{
    fDevice.SetLocalToWorldMatrix(fView.GetLocalToWorld());
    fView.fXformResetFlags &= ~fView.kResetL2W;
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

template<class DeviceType>
bool pl3DPipeline<DeviceType>::IAvatarSort(plDrawableSpans* d, const std::vector<int16_t>& visList)
{
    plProfile_BeginTiming(AvatarSort);
    for (int16_t visIdx : visList)
    {
        hsAssert(d->GetSpan(visIdx)->fTypeMask & plSpan::kIcicleSpan, "Unknown type for sorting faces");

        plIcicle* span = (plIcicle*)d->GetSpan(visIdx);

        if (span->fProps & plSpan::kPartialSort) {
            hsAssert(d->GetBufferGroup(span->fGroupIdx)->AreIdxVolatile(), "Badly setup buffer group - set PartialSort too late?");

            const hsPoint3 viewPos = GetViewPositionWorld();

            plGBufferGroup* group = d->GetBufferGroup(span->fGroupIdx);

            typename DeviceType::VertexBufferRef* vRef = static_cast<typename DeviceType::VertexBufferRef*>(group->GetVertexBufferRef(span->fVBufferIdx));

            const uint8_t* vdata = vRef->fData;
            const uint32_t stride = vRef->fVertexSize;

            const int numTris = span->fILength/3;

            static std::vector<plSortFace> sortScratch;
            sortScratch.resize(numTris);

            plProfile_IncCount(AvatarFaces, numTris);

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

#endif //_pl3DPipeline_inc_
