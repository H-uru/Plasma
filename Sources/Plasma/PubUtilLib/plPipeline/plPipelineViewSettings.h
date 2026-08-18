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

#ifndef _plPipelineViewSettings_inc_
#define _plPipelineViewSettings_inc_

#include <vector>

#include "HeadSpin.h"
#include "hsMatrix44.h"
#include "plFogEnvironment.h"
#include "hsGeometry3.h"
#include "hsColorRGBA.h"
#include "hsBitVector.h"
#include "hsPoint2.h"
#include "plCullTree.h"
#include "plViewTransform.h"

//// General Settings /////////////////////////////////////////////////////////

class plRenderRequest;
class plPipeline;
class plDrawableSpans;
class plVisMgr;
class plSceneObject;


// plPipelineViewSettings are just a convenience member struct to segregate the
// current view settings.
class plPipelineViewSettings
{
protected:
    plPipeline*                 fPipeline;

    uint32_t                    fDrawableTypeMask;
    uint32_t                    fSubDrawableTypeMask;

    plFogEnvironment            fDefaultFog;

    hsColorRGBA                 fClearColor;
    float                       fClearDepth;

    plViewTransform             fTransform;

    hsMatrix44                  fLocalToWorld;
    hsMatrix44                  fWorldToLocal;

    // Occluder & Culling stuff
    std::vector<const plCullPoly*> fCullPolys;
    std::vector<const plCullPoly*> fCullHoles;
    plCullTree                  fCullTree;
    plDrawableSpans*            fCullProxy;

    uint16_t                    fMaxCullNodes;

public:
    uint32_t                fRenderState;

    plRenderRequest*        fRenderRequest;

    bool                    fCullTreeDirty;

    enum XformResets
    {
        kResetProjection    = 0x01,
        kResetCamera        = 0x02,
        kResetL2W           = 0x04,

        kResetAll           = 0x07
    };

    uint8_t                 fXformResetFlags;
    bool                    fLocalToWorldLeftHanded;
    bool                    fWorldToCamLeftHanded;

    mutable bool            fViewVectorsDirty;

    const hsMatrix44&       GetLocalToWorld() const { return fLocalToWorld; }
    const hsMatrix44&       GetWorldToLocal() const { return fWorldToLocal; }

    void                    SetLocalToWorld(const hsMatrix44& l2w) { fLocalToWorld = l2w; }
    void                    SetWorldToLocal(const hsMatrix44& w2l) { fWorldToLocal = w2l; }

    const hsMatrix44&       GetWorldToCamera() const { return fTransform.GetWorldToCamera(); }
    const hsMatrix44&       GetCameraToWorld() const { return fTransform.GetCameraToWorld(); }
    bool                    IsPerspective() const { return fTransform.GetPerspective(); }

    const hsPoint3          GetViewPositionWorld() const { return fTransform.GetPosition(); }
    const hsVector3         GetViewAcrossWorld() const { return fTransform.GetAcross(); }
    const hsVector3         GetViewUpWorld() const { return fTransform.GetUp(); }
    const hsVector3         GetViewDirWorld() const { return fTransform.GetDirection(); }

    bool                    HasCullProxy() const { return fCullProxy != nullptr; }
    plDrawableSpans*        GetCullProxy() const { return fCullProxy; }

    uint16_t                GetMaxCullNodes() const { return fMaxCullNodes; }
    void                    SetMaxCullNodes(uint16_t max) { fMaxCullNodes = max; }

    const plFogEnvironment& GetDefaultFog() const { return fDefaultFog; }
    void                    SetDefaultFog(const plFogEnvironment& fog) { fDefaultFog = fog; }

    hsColorRGBA             GetClearColor() const { return fClearColor; }
    float                   GetClearDepth() const { return fClearDepth; }

    const plViewTransform&  GetConstViewTransform() const { return fTransform; }
    plViewTransform&        GetViewTransform() { return fTransform; }
    void                    SetViewTransform(const plViewTransform& vt) { fTransform = vt; }

    uint32_t                GetDrawableTypeMask() const { return fDrawableTypeMask; }
    void                    SetDrawableTypeMask(uint32_t mask) { fDrawableTypeMask = mask; }

    uint32_t                GetSubDrawableTypeMask() const { return fSubDrawableTypeMask; }
    void                    SetSubDrawableTypeMask(uint32_t mask) { fSubDrawableTypeMask = mask; }

    bool                    IsViewLeftHanded() const { return (GetConstViewTransform().GetOrthogonal() ^ (fLocalToWorldLeftHanded ^ fWorldToCamLeftHanded)) != 0; }

    /** Initialize the ViewSettings to default (normal/neutral) values. */
    void    Reset(plPipeline* pipeline);

    /** Set the current hither and yon. */
    void    SetDepth(float hither, float yon);

    /**
     * Set the current FOV in degrees. Forces perspective rendering to be
     * true.
     */
    void    SetFOV(float fovX, float fovY);

    /**
     * Set the orthogonal projection view size in world units (e.g. feet).
     * Forces projection to orthogonal if it wasn't.
     */
    void    SetSize(float width, float height);

    /** Set the color and depth clear values. */
    void    SetClear(const hsColorRGBA* col=nullptr, const float* depth=nullptr);


    /**
     * Takes a snapshot of the current occlusion BSP tree and renders it until
     * told to stop.
     * Debugging visualization tool only.
     */
    void    MakeOcclusionSnap();


    /**
     * The cull tree captures the view frustum and any occluders in the scene
     * into a single BSP tree.
     * It must be recomputed any time the camera moves.
     *
     * \sa plCullTree
     */
    void    RefreshCullTree();


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
     * GetVisibleSpans, which also takes the plVisMgr into account.
     */
    bool    HarvestVisible(plSpaceTree* space, std::vector<int16_t>& visList);


    /**
     * Given a drawable, returns a list of visible span indices. Disabled spans
     * will not show up in the list, behaving as if they were culled.
     *
     * \sa plCullTree
     * \sa plSpaceTree
     * \sa plVisMgr
     */
    void    GetVisibleSpans(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr);


    /**
     * Add the input polys into the list of polys from which to generate the
     * cull tree.
     */
    bool    SubmitOccluders(const std::vector<const plCullPoly*>& polyList);


    /**
     * Check if the world space bounds are visible within the current view
     * frustum.
     */
    bool    TestVisibleWorld(const hsBounds3Ext& wBnd);


    /**
     * Check if the object space bounds are visible within the current view
     * frustum.
     */
    bool    TestVisibleWorld(const plSceneObject* sObj);
};


//// Tweak Settings ///////////////////////////////////////////////////////////

class plPipelineTweakSettings
{
public:
    float fDefaultPerspLayerScale;
    float fPerspLayerScale;
    float fPerspLayerTrans;
    float fDefaultLODBias;
    float fFogExpApproxStart;
    float fFogExp2ApproxStart;
    float fFogEndBias;

    float fExp2FogKnee;
    float fExp2FogKneeVal;
    float fExpFogKnee;
    float fExpFogKneeVal;

    void Reset()
    {
        fDefaultPerspLayerScale = 0.00001f;
        fPerspLayerScale = 0.00001f;
        fPerspLayerTrans = 0.00002f;
        fDefaultLODBias = -0.25f;
        fFogExpApproxStart = 0.0f;
        fFogExp2ApproxStart = 0.0f;
        fFogEndBias = 0.0f;

        fExpFogKnee = fExp2FogKnee = 0.5f;
        fExpFogKneeVal = fExp2FogKneeVal = 0.15f;
    }
};

#endif //_plPipelineViewSettings_inc_
