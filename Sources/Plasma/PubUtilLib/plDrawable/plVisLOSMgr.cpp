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

#include "HeadSpin.h"
#include "hsBounds.h"
#include "hsFastMath.h"

#include "plVisLOSMgr.h"

#include "plSpaceTree.h"
#include "plDrawableSpans.h"
#include "plAccessGeometry.h"
#include "plAccessSpan.h"

#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayerInterface.h"

#include "plScene/plSceneNode.h"
#include "plScene/plPageTreeMgr.h"


#include "plPipeline.h"
#include "pnSceneObject/plSceneObject.h"

// Stuff for cursor los
#include "plInputCore/plInputDevice.h"
#include "plPipeline.h"


#include "plTweak.h"

#include <algorithm>
#include <functional>

plVisLOSMgr* plVisLOSMgr::Instance()
{
    static plVisLOSMgr inst;
    return &inst;
}

bool plVisLOSMgr::ICheckSpaceTreeRecur(plSpaceTree* space, int which, std::vector<plSpaceHit>& hits)
{
    const plSpaceTreeNode& node = space->GetNode(which);

    if( node.fFlags & plSpaceTreeNode::kDisabled )
        return false;

    float closest;
    // If it's a hit
    if( ICheckBound(node.fWorldBounds, closest) )
    {
        // If it's a leaf, 
        if( node.IsLeaf() )
        {
            // add it to the list with the closest intersection point,
            hits.emplace_back(which, closest);
            return true;
        }
        // else recurse on its children
        else
        {
            bool retVal = false;
            if( ICheckSpaceTreeRecur(space, node.GetChild(0), hits) )
                retVal = true;

            if( ICheckSpaceTreeRecur(space, node.GetChild(1), hits) )
                retVal = true;

            return retVal;
        }
    }
    return false;
}

bool plVisLOSMgr::ICheckSpaceTree(plSpaceTree* space, std::vector<plSpaceHit>& hits)
{
    hits.clear();

    if( space->IsEmpty() )
        return false;

    // Hierarchical search down the tree for bounds intersecting the current ray.
    bool retVal = ICheckSpaceTreeRecur(space, space->GetRoot(), hits);

    // Now sort them front to back.
    std::sort(hits.begin(), hits.end(), [](const plSpaceHit& lhs, const plSpaceHit& rhs) {
        return lhs.fClosest < rhs.fClosest;
    });

    return retVal;
}

bool plVisLOSMgr::ISetup(const hsPoint3& pStart, const hsPoint3& pEnd)
{
    fCurrFrom = pStart;
    fCurrTarg = pEnd;

    fMaxDist = hsVector3(&fCurrTarg, &fCurrFrom).Magnitude();

    const float kMinMaxDist(0);
    return fMaxDist > kMinMaxDist;
}

bool plVisLOSMgr::Check(const hsPoint3& pStart, const hsPoint3& pEnd, plVisHit& hit)
{
    if( !fPageMgr )
        return false;

    // Setup any internals, like fMaxDist
    if( !ISetup(pStart, pEnd) )
        return false;

    // Go through the nodes in the PageMgr and find the closest
    // point of intersection for each scene node. If none are before
    // pEnd, return false.
    // Node come out sorted by closest point, front to back
    static std::vector<plSpaceHit> hits;
    if( !ICheckSpaceTree(fPageMgr->GetSpaceTree(), hits) )
        return false;

    // In front to back order, check inside each node.
    // Our max distance can be changing as we do this, because a
    // face hit will limit how far we need to look. When we hit the
    // first node with a closest distance < fMaxDist, we're done.
    bool retVal = false;

    for (const plSpaceHit& spaceHit : hits)
    {
        if (spaceHit.fClosest > fMaxDist)
            break;

        if (ICheckSceneNode(fPageMgr->GetNodes()[spaceHit.fIdx], hit))
            retVal = true;
    }

    return retVal;
}

bool plVisLOSMgr::ICheckSceneNode(plSceneNode* node, plVisHit& hit)
{
    static std::vector<plSpaceHit> hits;
    if( !ICheckSpaceTree(node->GetSpaceTree(), hits) )
        return false;

    bool retVal = false;
    for (const plSpaceHit& spaceHit : hits)
    {
        if (spaceHit.fClosest > fMaxDist)
            break;

        if ((node->GetDrawPool()[spaceHit.fIdx]->GetRenderLevel().Major() > 0)
                && !node->GetDrawPool()[spaceHit.fIdx]->GetNativeProperty(plDrawable::kPropHasVisLOS))
            continue;

        if (ICheckDrawable(node->GetDrawPool()[spaceHit.fIdx], hit))
            retVal = true;
    }

    return retVal;
}


bool plVisLOSMgr::ICheckDrawable(plDrawable* d, plVisHit& hit)
{
    plDrawableSpans* ds = plDrawableSpans::ConvertNoRef(d);
    if( !ds )
        return false;

    static std::vector<plSpaceHit> hits;
    if( !ICheckSpaceTree(ds->GetSpaceTree(), hits) )
        return false;

    const bool isOpaque = ds->GetRenderLevel().Major() == plRenderLevel::kOpaqueMajorLevel;

    const std::vector<plSpan *>& spans = ds->GetSpanArray();

    bool retVal = false;
    for (const plSpaceHit& spaceHit : hits)
    {
        if (spaceHit.fClosest > fMaxDist)
            break;

        if (isOpaque || (spans[spaceHit.fIdx]->fProps & plSpan::kVisLOS))
        {
            if (ICheckSpan(ds, spaceHit.fIdx, hit))
                retVal = true;
        }
    }

    return retVal;
}

bool plVisLOSMgr::ICheckSpan(plDrawableSpans* dr, uint32_t spanIdx, plVisHit& hit)
{
    if( !(dr->GetSpan(spanIdx)->fTypeMask & plSpan::kIcicleSpan) )
        return false;

    plAccessSpan src;
    plAccessGeometry::Instance()->OpenRO(dr, spanIdx, src);

    const bool twoSided = !!(src.GetMaterial()->GetLayer(0)->GetMiscFlags() & hsGMatState::kMiscTwoSided);

    bool retVal = false;

    // We move into local space, look for hits, and convert the closest we find 
    // (if any) back into world space at the end.
    hsPoint3 currFrom = src.GetWorldToLocal() * fCurrFrom;
    hsPoint3 currTarg = src.GetWorldToLocal() * fCurrTarg;

    hsVector3 currDir(&currTarg, &currFrom);
    float maxDist = currDir.Magnitude();

    currDir /= maxDist;

    plAccTriIterator tri(&src.AccessTri());
    for( tri.Begin(); tri.More(); tri.Advance() )
    {
        // Project the current ray onto the tri plane
        hsVector3 norm = hsVector3(&tri.Position(1), &tri.Position(0)) % hsVector3(&tri.Position(2), &tri.Position(0));
        float dotNorm = norm.InnerProduct(currDir);

        const float kMinDotNorm = 1.e-3f;
        if( dotNorm >= -kMinDotNorm )
        {
            if( !twoSided )
                continue;
            if( dotNorm <= kMinDotNorm )
                continue;
        }
        float dist = hsVector3(&tri.Position(0), &currFrom).InnerProduct(norm);
        if( dist > 0 )
            continue;
        dist /= dotNorm;
        hsPoint3 projPt = currFrom;
        projPt += currDir * dist;

        // If the distance from source point to projected point is too long, skip
        if( dist > maxDist )
            continue;

        // Find the 3 cross products (v[i+1]-v[i]) X (proj - v[i]) dotted with current ray
        hsVector3 cross0 = hsVector3(&tri.Position(1), &tri.Position(0)) % hsVector3(&projPt, &tri.Position(0));
        float dot0 = cross0.InnerProduct(currDir);

        hsVector3 cross1 = hsVector3(&tri.Position(2), &tri.Position(1)) % hsVector3(&projPt, &tri.Position(1));
        float dot1 = cross1.InnerProduct(currDir);

        hsVector3 cross2 = hsVector3(&tri.Position(0), &tri.Position(2)) % hsVector3(&projPt, &tri.Position(2));
        float dot2 = cross2.InnerProduct(currDir);

        // If all 3 are negative, projPt is a hit
        // If all 3 are positive and we're two sided, projPt is a hit
        // We've already checked for back facing (when we checked for edge on in projection),
        // so we'll accept either case here.
        if( ((dot0 <= 0) && (dot1 <= 0) && (dot2 <= 0))
            ||((dot0 >= 0) && (dot1 >= 0) && (dot2 >= 0)) )
        {
            if( dist < maxDist )
            {
                maxDist = dist;
                hit.fPos = projPt;
                retVal = true;
            }
        }
    }
    plAccessGeometry::Instance()->Close(src);

    if( retVal )
    {
        hit.fPos = src.GetLocalToWorld() * hit.fPos;
        fCurrTarg = hit.fPos;
        fMaxDist = hsVector3(&fCurrTarg, &fCurrFrom).Magnitude();
    }

    return retVal;
}

bool plVisLOSMgr::ICheckBound(const hsBounds3Ext& bnd, float& closest)
{
    if( bnd.GetType() != kBoundsNormal )
        return false;

    if( bnd.IsInside(&fCurrFrom) || bnd.IsInside(&fCurrTarg) )
    {
        closest = 0;
        return true;
    }

    const int face[6][4] =
    {
        {0,1,3,2},
        {1,5,7,3},
        {2,3,7,6},
        {5,4,6,7},
        {0,4,5,1},
        {0,2,6,4}
    };

    hsPoint3 corn[8];
    bnd.GetCorners(corn);

    const hsPoint3& currFrom = fCurrFrom;
    const hsPoint3& currTarg = fCurrTarg;

    hsVector3 currDir(&currTarg, &currFrom);
    const float maxDistSq = currDir.MagnitudeSquared();

    currDir *= hsFastMath::InvSqrt(maxDistSq);

    int i;
    for( i = 0; i < 6; i++ )
    {
        const hsPoint3& p0 = corn[face[i][0]];
        const hsPoint3& p1 = corn[face[i][1]];
        const hsPoint3& p2 = corn[face[i][2]];
        const hsPoint3& p3 = corn[face[i][3]];

        // Project the current ray onto the tri plane
        hsVector3 norm = hsVector3(&p1, &p0) % hsVector3(&p2, &p0);
        float dotNorm = norm.InnerProduct(currDir);

        const float kMinDotNorm = 1.e-3f;
        if( dotNorm >= -kMinDotNorm )
        {
            continue;
        }
        float dist = hsVector3(&p0, &currFrom).InnerProduct(norm);
        if( dist >= 0 )
            continue;
        dist /= dotNorm;

        // If the distance from source point to projected point is too long, skip
        if( dist > fMaxDist )
            continue;

        hsPoint3 projPt = currFrom;
        projPt += currDir * dist;

        // Find the 3 cross products (v[i+1]-v[i]) X (proj - v[i]) dotted with current ray
        hsVector3 cross0 = hsVector3(&p1, &p0) % hsVector3(&projPt, &p0);
        float dot0 = cross0.InnerProduct(currDir);

        hsVector3 cross1 = hsVector3(&p2, &p1) % hsVector3(&projPt, &p1);
        float dot1 = cross1.InnerProduct(currDir);

        hsVector3 cross2 = hsVector3(&p3, &p2) % hsVector3(&projPt, &p2);
        float dot2 = cross2.InnerProduct(currDir);

        hsVector3 cross3 = hsVector3(&p0, &p3) % hsVector3(&projPt, &p3);
        float dot3 = cross3.InnerProduct(currDir);

        // If all 4 are negative, projPt is a hit
        if( (dot0 <= 0) && (dot1 <= 0) && (dot2 <= 0) && (dot3 <= 0) )
        {
            closest = dist;
            return true;
        }
    }
    return false;
}

bool plVisLOSMgr::CursorCheck(plVisHit& hit)
{
    int32_t sx= int32_t(plMouseDevice::Instance()->GetCursorX() * fPipe->Width());
    int32_t sy= int32_t(plMouseDevice::Instance()->GetCursorY() * fPipe->Height());

    hsPoint3 from = fPipe->GetViewPositionWorld();
    plConst(float) dist(1.e5f);

    hsPoint3 targ;
    fPipe->ScreenToWorldPoint(1, 0, &sx, &sy, dist, 0, &targ);
    return Check(from, targ, hit);
}

/////////////////////////////////////////////////////////////////

static plSceneObject* marker = nullptr;
static plPipeline* fPipe = nullptr;


void VisLOSHackBegin(plPipeline* p, plSceneObject* m)
{
    marker = m;
    fPipe = p;
}

void VisLOSHackPulse()
{
    if( !fPipe )
        return;

    plVisHit hit;
    if( plVisLOSMgr::Instance()->CursorCheck(hit) )
    {
        if( marker )
        {
            hsMatrix44 l2w = marker->GetLocalToWorld();
            l2w.fMap[0][3] = hit.fPos.fX;
            l2w.fMap[1][3] = hit.fPos.fY;
            l2w.fMap[2][3] = hit.fPos.fZ;
            l2w.NotIdentity();
            hsMatrix44 w2l;
            l2w.GetInverse(&w2l);
            marker->SetTransform(l2w, w2l);
        }
    }
}
