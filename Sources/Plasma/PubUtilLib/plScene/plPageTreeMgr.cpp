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

#include "plPageTreeMgr.h"

#include "HeadSpin.h"
#include "hsFastMath.h"
#include "plDrawable.h"
#include "plPipeline.h"
#include "plProfile.h"
#include "plTweak.h"

#include <algorithm>

#include "plCullPoly.h"
#include "plOccluder.h"
#include "plSceneNode.h"
#include "plVisMgr.h"

#include "plDrawable/plSpaceTreeMaker.h"
#include "plDrawable/plSpaceTree.h"
#include "plMath/hsRadixSort.h"

static std::vector<hsRadixSortElem> scratchList;

bool plPageTreeMgr::fDisableVisMgr = false;

plProfile_CreateTimer("Object Sort", "Draw", DrawObjSort);
plProfile_CreateCounter("Objects Sorted", "Draw", DrawObjSorted);
plProfile_CreateTimer("Occluder Sort", "Draw", DrawOccSort);
plProfile_CreateCounter("Occluders Used", "Draw", DrawOccUsed);
plProfile_CreateTimer("Occluder Build", "Draw", DrawOccBuild);
plProfile_CreateCounter("Occluder Polys Processed", "Draw", DrawOccPolyProc);
plProfile_CreateTimer("Occluder Poly Sort", "Draw", DrawOccPolySort);

plPageTreeMgr::plPageTreeMgr()
:   fSpaceTree()
{
    fVisMgr = plGlobalVisMgr::Instance();
}

plPageTreeMgr::~plPageTreeMgr()
{
    delete fSpaceTree;
}

void plPageTreeMgr::AddNode(plSceneNode* node)
{
    ITrashSpaceTree();

    node->Init();

    fNodes.push_back(node);
}

void plPageTreeMgr::RemoveNode(plSceneNode* node)
{
    ITrashSpaceTree();

    auto it = std::find(fNodes.begin(), fNodes.end(), node);
    if (it != fNodes.end())
        fNodes.erase(it);
}

void plPageTreeMgr::Reset()
{
    fNodes.clear();

    ITrashSpaceTree();
}

void plPageTreeMgr::ITrashSpaceTree()
{
    delete fSpaceTree;
    fSpaceTree = nullptr;
}

bool plPageTreeMgr::Harvest(plVolumeIsect* isect, std::vector<plDrawVisList>& levList)
{
    levList.clear();
    if( !(GetSpaceTree() || IBuildSpaceTree()) )
        return false;

    static std::vector<int16_t> list;

    GetSpaceTree()->HarvestLeaves(isect, list);

    for (int16_t idx : list)
    {
        fNodes[idx]->Harvest(isect, levList);
    }

    return !levList.empty();
}

#include "plProfile.h"
plProfile_CreateTimer("DrawableTime", "Draw", DrawableTime);
plProfile_Extern(RenderScene);

int plPageTreeMgr::Render(plPipeline* pipe)
{
    // If we don't have a space tree and can't make one, just bail
    if( !(GetSpaceTree() || IBuildSpaceTree()) )
        return 0;

    static std::vector<int16_t> list;
    list.clear();

    plProfile_BeginTiming(RenderScene);

    plVisMgr* visMgr = fDisableVisMgr ? nullptr : fVisMgr;

    if( visMgr )
    {
        plProfile_Extern(VisEval);
        plProfile_BeginTiming(VisEval);
        visMgr->Eval(pipe->GetViewPositionWorld());
        plProfile_EndTiming(VisEval);
    }

    pipe->BeginVisMgr(visMgr);

    IRefreshTree(pipe);

    IGetOcclusion(pipe, list);
    pipe->HarvestVisible(GetSpaceTree(), list);

    static std::vector<plDrawVisList> levList;
    levList.clear();
    for (int16_t idx : list)
    {
        fNodes[idx]->CollectForRender(pipe, levList, visMgr);
    }
    
    int numDrawn = IRenderVisList(pipe, levList);

    IResetOcclusion(pipe);

    pipe->EndVisMgr(visMgr);

    plProfile_EndTiming(RenderScene);

    return numDrawn;

}

size_t plPageTreeMgr::IRenderVisList(plPipeline* pipe, std::vector<plDrawVisList>& levList)
{
    // Sort levList into sortedDrawList, which is just a list
    // of drawable/visList pairs in ascending render priority order.
    // visLists are just lists of span indices, but only of the
    // spans which are visible (on screen and non-occluded and non-disabled).
    static std::vector<plDrawVisList> sortedDrawList;
    if( !ISortByLevel(pipe, levList, sortedDrawList) )
    {
        return 0;
    }

    size_t numDrawn = 0;

    plVisMgr* visMgr = fDisableVisMgr ? nullptr : fVisMgr;

    // Going through the list in order, if we hit a drawable which doesn't need
    // its spans sorted, we can just draw it.
    // If we hit a drawable which does need its spans sorted, we could just draw
    // it, but that precludes sorting spans between drawables (like the player avatar
    // sorting with normal scene objects). So when we hit a drawable which needs
    // span sorting, we sort its spans with the spans of the next N drawables in
    // the sorted list which have the same render priority and which also want their
    // spans sorted.
    for (size_t i = 0; i < sortedDrawList.size(); i++)
    {
        plDrawable* p = sortedDrawList[i].fDrawable;


        plProfile_BeginLap(DrawableTime, p->GetKey()->GetUoid().GetObjectName().c_str());
    
        if( sortedDrawList[i].fDrawable->GetNativeProperty(plDrawable::kPropSortSpans) )
        {
            // IPrepForRenderSortingSpans increments "i" to the next index to be drawn (-1 so the i++
            // at the top of the loop is correct.
            numDrawn += IPrepForRenderSortingSpans(pipe, sortedDrawList, i);
        }
        else
        {
            pipe->PrepForRender(sortedDrawList[i].fDrawable, sortedDrawList[i].fVisList, visMgr);

            pipe->Render(sortedDrawList[i].fDrawable, sortedDrawList[i].fVisList);

            numDrawn += sortedDrawList[i].fVisList.size();

        }

        plProfile_EndLap(DrawableTime, p->GetKey()->GetUoid().GetObjectName().c_str());
    }

    return numDrawn;
}

bool plPageTreeMgr::ISortByLevel(plPipeline* pipe, std::vector<plDrawVisList>& drawList, std::vector<plDrawVisList>& sortedDrawList)
{
    sortedDrawList.clear();

    if (drawList.empty())
        return false;

    scratchList.resize(drawList.size());

    hsRadixSort::Elem* listTrav = nullptr;
    for (size_t i = 0; i < drawList.size(); i++)
    {
        listTrav = &scratchList[i];
        listTrav->fBody = (intptr_t)&drawList[i];
        listTrav->fNext = listTrav+1;
        listTrav->fKey.fULong = drawList[i].fDrawable->GetRenderLevel().Level();
    }
    listTrav->fNext = nullptr;

    hsRadixSort rad;
    hsRadixSort::Elem* sortedList = rad.Sort(scratchList.data(), hsRadixSort::kUnsigned);

    listTrav = sortedList;

    while( listTrav )
    {
        plDrawVisList& drawVis = *(plDrawVisList*)listTrav->fBody;
        sortedDrawList.emplace_back(drawVis);
        
        listTrav = listTrav->fNext;
    }

    return true;
}

// Render from iDrawStart in drawVis list all drawables with the sort by spans property, well, sorting
// by spans.
// Returns the index of the last one drawn.
size_t plPageTreeMgr::IPrepForRenderSortingSpans(plPipeline* pipe, std::vector<plDrawVisList>& drawVis, size_t& iDrawStart)
{
    uint32_t renderLevel = drawVis[iDrawStart].fDrawable->GetRenderLevel().Level();

    static std::vector<plDrawVisList*> drawables;
    static std::vector<plDrawSpanPair> pairs;

    // Given the input drawVisList (list of drawable/visList pairs), we make two new
    // lists. The list "drawables" is just the excerpted sub-list from drawVis starting
    // from the input index and going through all compatible drawables (drawables which
    // are appropriate to sort (and hence intermix) with the first drawable in the list.
    // The second list is the drawableIndex/spanIndex pairs convenient for sorting (where
    // drawIndex indexes into drawables and spanIndex indexes into drawVis[iDraw].fVisList.
    // So pairs[i] resolves into 
    // drawables[pairs[i].fDrawable].fDrawable->GetSpan(pairs[i].fSpan)

    drawables.emplace_back(&drawVis[iDrawStart]);
    for (int16_t idx : drawVis[iDrawStart].fVisList)
        pairs.emplace_back(0, idx);

    size_t iDraw;
    for (iDraw = iDrawStart + 1;
            (iDraw < drawVis.size())
                && (drawVis[iDraw].fDrawable->GetRenderLevel().Level() == renderLevel)
                && drawVis[iDraw].fDrawable->GetNativeProperty(plDrawable::kPropSortSpans);
            iDraw++)
    {
        for (int16_t idx : drawVis[iDraw].fVisList)
            pairs.emplace_back((uint16_t)drawables.size(), idx);
        drawables.emplace_back(&drawVis[iDraw]);
    }

    // Now that we have them in a more convenient format, sort them and render.
    IRenderSortingSpans(pipe, drawables, pairs);

    size_t numDrawn = pairs.size();

    drawables.clear();
    pairs.clear();

    iDrawStart = iDraw - 1;

    return numDrawn;
}

bool plPageTreeMgr::IRenderSortingSpans(plPipeline* pipe, std::vector<plDrawVisList*>& drawList, std::vector<plDrawSpanPair>& pairs)
{

    if (pairs.empty())
        return false;

    hsPoint3 viewPos = pipe->GetViewPositionWorld();

    plProfile_BeginTiming(DrawObjSort);
    plProfile_IncCount(DrawObjSorted, pairs.size());

    hsRadixSort::Elem* listTrav;
    scratchList.resize(pairs.size());

    // First, sort on distance to the camera (squared).
    listTrav = nullptr;
    int iSort = 0;
    for (const plDrawSpanPair& pair : pairs)
    {
        plDrawable* drawable = drawList[pair.fDrawable]->fDrawable;

        listTrav = &scratchList[iSort++];
        listTrav->fBody = (intptr_t)&pair;
        listTrav->fNext = listTrav + 1;

        if( drawable->GetNativeProperty(plDrawable::kPropSortAsOne) )
        {
            const hsBounds3Ext& bnd = drawable->GetSpaceTree()->GetNode(drawable->GetSpaceTree()->GetRoot()).fWorldBounds;
            plConst(float) kDistFudge(1.e-1f);
            listTrav->fKey.fFloat = -(bnd.GetCenter() - viewPos).MagnitudeSquared() + float(pair.fSpan) * kDistFudge;
        }
        else
        {
            const hsBounds3Ext& bnd = drawable->GetSpaceTree()->GetNode(pair.fSpan).fWorldBounds;
            listTrav->fKey.fFloat = -(bnd.GetCenter() - viewPos).MagnitudeSquared();
        }
    }
    if( !listTrav )
    {
        plProfile_EndTiming(DrawObjSort);
        return false;
    }
    listTrav->fNext = nullptr;

    hsRadixSort rad;
    hsRadixSort::Elem* sortedList = rad.Sort(scratchList.data(), 0);

    plProfile_EndTiming(DrawObjSort);

    static std::vector<int16_t> visList;
    visList.clear();

    plVisMgr* visMgr = fDisableVisMgr ? nullptr : fVisMgr;

    // Call PrepForRender on each of these bad boys. We only want to call
    // PrepForRender once on each drawable, no matter how many times we're
    // going to pass it off to be rendered (like if we render span 0 from
    // drawable A, span 1 from drawable A, span 0 from drawable B, span 1 from Drawable A, we
    // don't want to PrepForRender twice or three times on drawable A).
    // So we're going to convert our sorted list back into a list of drawable/visList
    // pairs. We could have done this with our original drawable/visList, but we've
    // hopefully trimmed out some spans because of the fades. This drawable/visList
    // isn't appropriate for rendering (because it doesn't let us switch back and forth 
    // from a drawable, but it's right for the PrepForRenderCall (which does things like
    // face sorting).
    for (plDrawVisList* dvList : drawList)
        dvList->fVisList.clear();
    listTrav = sortedList;
    while( listTrav )
    {
        plDrawSpanPair& curPair = *(plDrawSpanPair*)listTrav->fBody;
        drawList[curPair.fDrawable]->fVisList.emplace_back(curPair.fSpan);
        listTrav = listTrav->fNext;
    }
    for (plDrawVisList* dvList : drawList)
    {
        pipe->PrepForRender(dvList->fDrawable, dvList->fVisList, visMgr);
    }

    // We'd like to call Render once on a drawable for each contiguous
    // set of spans (so we want to render span 0 and span 1 on a single Render
    // of drawable A in the above, then render drawable B, then back to A).
    // So we go through the sorted drawable/spanIndex pairs list, building
    // a visList for as long as the drawable remains the same. When it
    // changes, we render what we have so far, and start again with the
    // next drawable. Repeat until done.

#if 0
    listTrav = sortedList;
    plDrawSpanPair& curPair = *(plDrawSpanPair*)listTrav->fBody;
    int curDraw = curPair.fDrawable;
    visList.emplace_back(curPair.fSpan);
    listTrav = listTrav->fNext;

    while( listTrav )
    {
        curPair = *(plDrawSpanPair*)listTrav->fBody;
        if( curPair.fDrawable != curDraw )
        {
            pipe->Render(drawList[curDraw]->fDrawable, visList);
            curDraw = curPair.fDrawable;
            visList.clear();
            visList.emplace_back(curPair.fSpan);
        }
        else
        {
            visList.emplace_back(curPair.fSpan);
        }
        listTrav = listTrav->fNext;
    }
    pipe->Render(drawList[curDraw]->fDrawable, visList);
#else
    listTrav = sortedList;
    plDrawSpanPair& curPair = *(plDrawSpanPair*)listTrav->fBody;
    int curDraw = curPair.fDrawable;
    listTrav = listTrav->fNext;

    static std::vector<uint32_t> numDrawn;
    numDrawn.assign(drawList.size(), 0);

    visList.emplace_back(drawList[curDraw]->fVisList[numDrawn[curDraw]++]);

    while( listTrav )
    {
        curPair = *(plDrawSpanPair*)listTrav->fBody;
        if( curPair.fDrawable != curDraw )
        {
            pipe->Render(drawList[curDraw]->fDrawable, visList);
            curDraw = curPair.fDrawable;
            visList.clear();
        }
        visList.emplace_back(drawList[curDraw]->fVisList[numDrawn[curDraw]++]);
        listTrav = listTrav->fNext;
    }
    pipe->Render(drawList[curDraw]->fDrawable, visList);
#endif

    return true;
}

bool plPageTreeMgr::IBuildSpaceTree()
{
    if (fNodes.empty())
        return false;

    plSpaceTreeMaker maker;
    maker.Reset();
    for (plSceneNode* node : fNodes)
        maker.AddLeaf(node->GetSpaceTree()->GetWorldBounds(), node->GetSpaceTree()->IsEmpty());
    fSpaceTree = maker.MakeTree();

    return true;
}

bool plPageTreeMgr::IRefreshTree(plPipeline* pipe)
{
    hsAssert(fNodes.size() < std::numeric_limits<uint16_t>::max(), "Too many nodes");
    for (uint16_t i = 0; i < uint16_t(fNodes.size()); ++i) {
        plSceneNode* node = fNodes[i];
        if (node->GetSpaceTree()->IsDirty()) {
            node->GetSpaceTree()->Refresh();

            GetSpaceTree()->MoveLeaf(i, node->GetSpaceTree()->GetWorldBounds());

            if (!node->GetSpaceTree()->IsEmpty() && fSpaceTree->HasLeafFlag(i, plSpaceTreeNode::kDisabled))
                fSpaceTree->SetLeafFlag(i, plSpaceTreeNode::kDisabled, false);
        }
    }

    GetSpaceTree()->SetViewPos(pipe->GetViewPositionWorld());

    GetSpaceTree()->Refresh();

    return true;
}

void plPageTreeMgr::AddOccluderList(const std::vector<plOccluder*>& occList)
{
    size_t iStart = fOccluders.size();
    fOccluders.resize(iStart + occList.size());

    plVisMgr* visMgr = fDisableVisMgr ? nullptr : fVisMgr;

    if( visMgr )
    {
        const hsBitVector& visSet = visMgr->GetVisSet();
        const hsBitVector& visNot = visMgr->GetVisNot();
        for (plOccluder* occluder : occList)
        {
            if (occluder && !occluder->InVisNot(visNot) && occluder->InVisSet(visSet))
                fOccluders[iStart++] = occluder;
        }
    }
    else
    {
        for (plOccluder* occluder : occList)
        {
            if (occluder)
                fOccluders[iStart++] = occluder;
        }
    }
    fOccluders.resize(iStart);
}

void plPageTreeMgr::IAddCullPolyList(const std::vector<plCullPoly>& polyList)
{
    size_t iStart = fCullPolys.size();
    fCullPolys.resize(iStart + polyList.size());
    for (size_t i = 0; i < polyList.size(); i++)
    {
        fCullPolys[i + iStart] = &polyList[i];
    }
}

void plPageTreeMgr::ISortCullPolys(plPipeline* pipe)
{
    fSortedCullPolys.clear();
    if (fCullPolys.empty())
        return;

    constexpr size_t kMaxCullPolys = 300;
    size_t numSubmit = 0;

    hsPoint3 viewPos = pipe->GetViewPositionWorld();

    hsRadixSort::Elem* listTrav;
    scratchList.resize(fCullPolys.size());
    for (const plCullPoly* poly : fCullPolys)
    {
        bool backFace = poly->fNorm.InnerProduct(viewPos) + poly->fDist <= 0;
        if( backFace )
        {
            if (!poly->IsHole() && !poly->IsTwoSided())
                continue;
        }
        else
        {
            if (poly->IsHole())
                continue;
        }

        listTrav = &scratchList[numSubmit];
        listTrav->fBody = (intptr_t)poly;
        listTrav->fNext = listTrav + 1;
        listTrav->fKey.fFloat = (poly->GetCenter() - viewPos).MagnitudeSquared();

        numSubmit++;
    }
    if( !numSubmit )
        return;

    listTrav->fNext = nullptr;

    hsRadixSort rad;
    hsRadixSort::Elem* sortedList = rad.Sort(scratchList.data(), 0);
    listTrav = sortedList;

    if( numSubmit > kMaxCullPolys )
        numSubmit = kMaxCullPolys;

    fSortedCullPolys.resize(numSubmit);

    for (size_t i = 0; i < numSubmit; i++)
    {
        fSortedCullPolys[i] = (const plCullPoly*)listTrav->fBody;
        listTrav = listTrav->fNext;
    }
}

bool plPageTreeMgr::IGetCullPolys(plPipeline* pipe)
{
    if (fOccluders.empty())
        return false;

    plProfile_BeginTiming(DrawOccSort);

    hsRadixSort::Elem* listTrav = nullptr;
    scratchList.resize(fOccluders.size());

    hsPoint3 viewPos = pipe->GetViewPositionWorld();

    // cull test the occluders submitted
    uint32_t numSubmit = 0;
    for (const plOccluder* occluder : fOccluders)
    {
        if( pipe->TestVisibleWorld(occluder->GetWorldBounds()) )
        {
            float invDist = -hsFastMath::InvSqrtAppr((viewPos - occluder->GetWorldBounds().GetCenter()).MagnitudeSquared());
            listTrav = &scratchList[numSubmit++];
            listTrav->fBody = (intptr_t)occluder;
            listTrav->fNext = listTrav+1;
            listTrav->fKey.fFloat = occluder->GetPriority() * invDist;
        }
    }
    if( !listTrav )
    {
        plProfile_EndTiming(DrawOccSort);
        return false;
    }

    listTrav->fNext = nullptr;


    // Sort the occluders by priority
    hsRadixSort rad;
    hsRadixSort::Elem* sortedList = rad.Sort(scratchList.data(), 0);
    listTrav = sortedList;

    constexpr uint32_t kMaxOccluders = 1000;
    if (numSubmit > kMaxOccluders)
        numSubmit = kMaxOccluders;

    plProfile_IncCount(DrawOccUsed, numSubmit);

    // Take the polys from the first N of them
    for (uint32_t i = 0; i < numSubmit; i++)
    {
        plOccluder* occ = (plOccluder*)listTrav->fBody;
        IAddCullPolyList(occ->GetWorldPolyList());
        
        listTrav = listTrav->fNext;
    }

    plProfile_EndTiming(DrawOccSort);

    return !fCullPolys.empty();
}

bool plPageTreeMgr::IGetOcclusion(plPipeline* pipe, std::vector<int16_t>& list)
{
    plProfile_BeginTiming(DrawOccBuild);

    fCullPolys.clear();
    fOccluders.clear();
    for (plSceneNode* node : fNodes)
        node->SubmitOccluders(this);

    if( !IGetCullPolys(pipe) )
    {
        plProfile_EndTiming(DrawOccBuild);
        return false;
    }

    plProfile_IncCount(DrawOccPolyProc, fCullPolys.size());

    plProfile_BeginTiming(DrawOccPolySort);
    ISortCullPolys(pipe);
    plProfile_EndTiming(DrawOccPolySort);

    if (!fSortedCullPolys.empty())
        pipe->SubmitOccluders(fSortedCullPolys);

    plProfile_EndTiming(DrawOccBuild);

    return !fSortedCullPolys.empty();
}

void plPageTreeMgr::IResetOcclusion(plPipeline* pipe)
{
    fCullPolys.clear();
    fSortedCullPolys.clear();
}
