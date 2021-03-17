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

#include "plPipelineViewSettings.h"

#include "pl3DPipeline.h"
#include "plPipeDebugFlags.h"
#include "plProfile.h"
#include "hsResMgr.h"

#include "pnSceneObject/plDrawInterface.h"
#include "pnSceneObject/plSceneObject.h"

#include "plDrawable/plDrawableGenerator.h"
#include "plDrawable/plDrawableSpans.h"
#include "plDrawable/plSpaceTree.h"
#include "plDrawable/plSpanTypes.h"
#include "plScene/plVisMgr.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"
#include "plSurface/plLayerInterface.h"

plProfile_CreateTimer("Harvest", "Draw", Harvest);
plProfile_Extern(DrawOccBuild);

plProfile_CreateCounter("OccPoly", "PipeC", OccPolyUsed);
plProfile_CreateCounter("OccNode", "PipeC", OccNodeUsed);


void plPipelineViewSettings::Reset(pl3DPipeline* pipeline)
{
    // The "owner" pipeline
    fPipeline = pipeline;

    fCullProxy = nullptr;

    // Normal render, on clear, clear the color buffer and depth buffer.
    fRenderState = plPipeline::kRenderNormal | plPipeline::kRenderClearColor | plPipeline::kRenderClearDepth;

    fRenderRequest = nullptr;

    fDrawableTypeMask = plDrawable::kNormal;
    fSubDrawableTypeMask = plDrawable::kSubNormal;

    // Clear color to black, depth to yon.
    fClearColor.Set(0.f, 0.f, 0.f, 0.f);
    fClearDepth = 1.f;
    fDefaultFog.Clear();

    // Want to limit the number of nodes in the cull tree. After adding so many nodes,
    // the benefits (#objects culled) falls off, but the cost (evaluating objects against
    // node planes) keeps rising.
    const uint16_t kCullMaxNodes = 250;
    fCullTree.Reset();
    fCullTreeDirty = true;
    fMaxCullNodes = kCullMaxNodes;

    // Object Local to world transform and its inverse.
    fLocalToWorld.Reset();
    fWorldToLocal.Reset();

    // see Core/plViewTransform.h
    fTransform.Reset();

    fTransform.SetScreenSize(800, 600);

    // Keep track of handedness of local to world and camera transform for winding.
    fLocalToWorldLeftHanded = false;
    fWorldToCamLeftHanded = false;
}

void plPipelineViewSettings::SetDepth(float hither, float yon) {
    fTransform.SetDepth(hither, yon);
}


void plPipelineViewSettings::SetFOV(float fovX, float fovY) {
    fTransform.SetFovDeg(fovX, fovY);
    fTransform.SetPerspective(true);
}


void plPipelineViewSettings::SetSize(float width, float height) {
    fTransform.SetWidth(width);
    fTransform.SetHeight(height);
    fTransform.SetOrthogonal(true);
}


void plPipelineViewSettings::SetClear(const hsColorRGBA* col, const float* depth)
{
    if (col)
        fClearColor = *col;

    if (depth)
        fClearDepth = *depth;
}


void plPipelineViewSettings::MakeOcclusionSnap()
{
    hsTArray<hsPoint3>& pos         = fCullTree.GetCaptureVerts();
    hsTArray<hsVector3>& norm       = fCullTree.GetCaptureNorms();
    hsTArray<hsColorRGBA>& color    = fCullTree.GetCaptureColors();
    hsTArray<uint16_t>& tris        = fCullTree.GetCaptureTris();

    if (tris.GetCount())
    {
        hsMatrix44 ident;
        ident.Reset();

        hsGMaterial* mat = new hsGMaterial();
        hsgResMgr::ResMgr()->NewKey("OcclusionSnapMat", mat, plLocation::kGlobalFixedLoc);
        plLayer *lay = mat->MakeBaseLayer();
        lay->SetZFlags(hsGMatState::kZNoZWrite);
        lay->SetPreshadeColor(hsColorRGBA().Set(1.f, 0.5f, 0.5f, 1.f));
        lay->SetRuntimeColor(hsColorRGBA().Set(1.f, 0.5f, 0.5f, 1.f));
        lay->SetAmbientColor(hsColorRGBA().Set(0,0,0,1.f));
        lay->SetOpacity(0.5f);
        lay->SetBlendFlags(lay->GetBlendFlags() | hsGMatState::kBlendAlpha);

        fCullProxy = plDrawableGenerator::GenerateDrawable(pos.GetCount(),
                                            pos.AcquireArray(),
                                            norm.AcquireArray(),
                                            nullptr,
                                            0,
                                            color.AcquireArray(),
                                            true,
                                            nullptr,
                                            tris.GetCount(),
                                            tris.AcquireArray(),
                                            mat,
                                            ident,
                                            true,
                                            nullptr,
                                            nullptr);

        if (fCullProxy)
        {
            fCullProxy->GetKey()->RefObject();
            fCullProxy->SetType(plDrawable::kOccSnapProxy);

            fDrawableTypeMask |= plDrawable::kOccSnapProxy;

            fCullProxy->PrepForRender(fPipeline);
        }
    }
    fCullTree.ReleaseCapture();
}


void plPipelineViewSettings::RefreshCullTree()
{
    if (fCullTreeDirty)
    {
        plProfile_BeginTiming(DrawOccBuild);

        fCullTree.Reset();

        fCullTree.SetViewPos(GetViewPositionWorld());

        if (fCullProxy && !fPipeline->IsDebugFlagSet(plPipeDbg::kFlagOcclusionSnap))
        {
            fCullProxy->GetKey()->UnRefObject();
            fCullProxy = nullptr;

            fDrawableTypeMask &= ~plDrawable::kOccSnapProxy;
        }

        bool doCullSnap = fPipeline->IsDebugFlagSet(plPipeDbg::kFlagOcclusionSnap) && !fCullProxy && !fPipeline->GetViewStackSize();
        if( doCullSnap )
        {
            fCullTree.BeginCapturePolys();
            fCullTree.SetVisualizationYon(fTransform.GetYon());
        }
        fCullTree.InitFrustum(fTransform.GetWorldToNDC());
        fCullTreeDirty = false;

        if (fMaxCullNodes)
        {
            int i;
            for (i = 0; i < fCullPolys.GetCount(); i++)
            {
                fCullTree.AddPoly(*fCullPolys[i]);
                if (fCullTree.GetNumNodes() >= fMaxCullNodes)
                    break;
            }
            fCullPolys.SetCount(0);
            plProfile_Set(OccPolyUsed, i);

            for (i = 0; i < fCullHoles.GetCount(); i++)
            {
                fCullTree.AddPoly(*fCullHoles[i]);
            }
            fCullHoles.SetCount(0);
            plProfile_Set(OccNodeUsed, fCullTree.GetNumNodes());
        }

        if (doCullSnap)
        {
            fCullTree.EndCapturePolys();
            MakeOcclusionSnap();
        }

        plProfile_EndTiming(DrawOccBuild);
    }
}


bool plPipelineViewSettings::HarvestVisible(plSpaceTree* space, std::vector<int16_t>& visList)
{
    if (!space)
        return false;

    space->SetViewPos(GetViewPositionWorld());

    space->Refresh();

    if (fCullTreeDirty)
        RefreshCullTree();

    plProfile_BeginTiming(Harvest);
    fCullTree.Harvest(space, visList);
    plProfile_EndTiming(Harvest);

    return !visList.empty();
}


void plPipelineViewSettings::GetVisibleSpans(plDrawableSpans* drawable, std::vector<int16_t>& visList, plVisMgr* visMgr)
{
    static std::vector<int16_t> tmpVis;
    tmpVis.clear();
    visList.clear();

    drawable->GetSpaceTree()->SetViewPos(GetViewPositionWorld());

    drawable->GetSpaceTree()->Refresh();

    if (fCullTreeDirty)
        RefreshCullTree();

    const float viewDist = GetViewDirWorld().InnerProduct(GetViewPositionWorld());

    const std::vector<plSpan *>& spans = drawable->GetSpanArray();

    plProfile_BeginTiming(Harvest);
    if (visMgr)
    {
        drawable->SetVisSet(visMgr);
        fCullTree.Harvest(drawable->GetSpaceTree(), tmpVis);
        drawable->SetVisSet(nullptr);
    }
    else
    {
        fCullTree.Harvest(drawable->GetSpaceTree(), tmpVis);
    }

    // This is a big waste of time, As a desparate "optimization" pass, the artists
    // insist on going through and marking objects to fade or pop out of rendering
    // past a certain distance. This breaks the batching and requires more CPU to
    // check the objects by distance. Since there is no pattern to the distance at
    // which objects will be told not to draw, there's no way to make this hierarchical,
    // which is what it would take to make it a performance win. So they succeed in
    // reducing the poly count, but generally the frame rate goes _down_ as well.
    // Unfortunately, this technique actually does work in a few key areas, so
    // I haven't been able to purge it.
    if (fPipeline->IsDebugFlagSet(plPipeDbg::kFlagSkipVisDist))
    {
        for (int16_t vis : tmpVis)
        {
            if (spans[vis]->fSubType & fSubDrawableTypeMask)
            {
                visList.emplace_back(vis);
            }
        }
    }
    else
    {
        for (int16_t vis : tmpVis)
        {
            if (spans[vis]->fSubType & fSubDrawableTypeMask)
            {
                // We'll check here for spans we can discard because they've completely distance faded out.
                // Note this is based on view direction distance (because the fade is), rather than the
                // preferrable distance to camera we sort by.
                float minDist, maxDist;
                if (drawable->GetSubVisDists(vis, minDist, maxDist))
                {
                    const hsBounds3Ext& bnd = drawable->GetSpaceTree()->GetNode(vis).fWorldBounds;
                    hsPoint2 depth;
                    bnd.TestPlane(GetViewDirWorld(), depth);

                    if ((0 < minDist + viewDist - depth.fY) ||(0 > maxDist + viewDist - depth.fX))
                        continue;
                }

                visList.emplace_back(vis);
            }
        }
    }
    plProfile_EndTiming(Harvest);
}


bool plPipelineViewSettings::SubmitOccluders(const std::vector<const plCullPoly*>& polyList)
{
    fCullPolys.SetCount(0);
    fCullHoles.SetCount(0);
    for (const plCullPoly* poly : polyList)
    {
        if (poly->IsHole())
            fCullHoles.Append(poly);
        else
            fCullPolys.Append(poly);
    }
    fCullTreeDirty = true;

    return true;
}


bool plPipelineViewSettings::TestVisibleWorld(const hsBounds3Ext& wBnd)
{
    if (fCullTreeDirty)
        RefreshCullTree();

    if (wBnd.GetType() == kBoundsNormal)
        return fCullTree.BoundsVisible(wBnd);
    else
        return false;
}

bool plPipelineViewSettings::TestVisibleWorld(const plSceneObject* sObj)
{
    const plDrawInterface* di = sObj->GetDrawInterface();
    if (!di)
        return false;

    const size_t numDraw = di->GetNumDrawables();
    for (size_t i = 0; i < numDraw; i++)
    {
        plDrawableSpans* dr = plDrawableSpans::ConvertNoRef(di->GetDrawable(i));
        if (!dr)
            continue;

        plDISpanIndex& diIndex = dr->GetDISpans(di->GetDrawableMeshIndex(i));
        if (diIndex.IsMatrixOnly())
            continue;

        const int numSpan = diIndex.GetCount();
        int j;
        for (j = 0; j < numSpan; j++)
        {
            const plSpan* span = dr->GetSpan(diIndex[j]);

            if (span->fProps & plSpan::kPropNoDraw)
                continue;

            if (!span->GetVisSet().Overlap(plGlobalVisMgr::Instance()->GetVisSet())
                || span->GetVisSet().Overlap(plGlobalVisMgr::Instance()->GetVisNot()))

                continue;

            if (!TestVisibleWorld(span->fWorldBounds))
                continue;

            return true;
        }
    }
    return false;
}