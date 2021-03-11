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
#include "plVolumeIsect.h"
#include "hsBounds.h"
#include "hsFastMath.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plIntersect/plClosest.h"

static const float kDefLength = 5.f;

plSphereIsect::plSphereIsect()
    : fRadius(1.f)
{
    int i;
    for( i = 0; i < 3; i++ )
    {
        fMins[i] = -fRadius;
        fMaxs[i] =  fRadius;
    }
}

void plSphereIsect::SetCenter(const hsPoint3& c)
{
    fWorldCenter = fCenter = c;
    int i;
    for( i = 0; i < 3; i++ )
    {
        fMins[i] += c[i];
        fMaxs[i] += c[i];
    }
}

void plSphereIsect::SetRadius(float r)
{
    float del = r - fRadius;
    int i;
    for( i = 0; i < 3; i++ )
    {
        fMins[i] -= del;
        fMaxs[i] += del;
    }
    fRadius = r;
}

void plSphereIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    fWorldCenter = l2w * fCenter;
    fMaxs = fMins = fWorldCenter;
    int i;
    for( i = 0; i < 3; i++ )
    {
        fMins[i] -= fRadius;
        fMaxs[i] += fRadius;
    }
}

// Could use ClosestPoint to find the closest point on the bounds
// to our center, and do a distance test on that. Would be more
// accurate than this box test approx, but whatever.
plVolumeCullResult plSphereIsect::Test(const hsBounds3Ext& bnd) const
{
    const hsPoint3& maxs = bnd.GetMaxs();
    const hsPoint3& mins = bnd.GetMins();

    if( (maxs.fX < fMins.fX)
        ||
        (maxs.fY < fMins.fY)
        ||
        (maxs.fZ < fMins.fZ) )
            return kVolumeCulled;

    if( (mins.fX > fMaxs.fX)
        ||
        (mins.fY > fMaxs.fY)
        ||
        (mins.fZ > fMaxs.fZ) )
            return kVolumeCulled;

    if( (maxs.fX > fMaxs.fX)
        ||
        (maxs.fY > fMaxs.fY)
        ||
        (maxs.fZ > fMaxs.fZ) )
            return kVolumeSplit;

    if( (mins.fX < fMins.fX)
        ||
        (mins.fY < fMins.fY)
        ||
        (mins.fZ < fMins.fZ) )
            return kVolumeSplit;

    return kVolumeClear;
}

float plSphereIsect::Test(const hsPoint3& pos) const
{
    float dist = (pos - fWorldCenter).MagnitudeSquared();
    if( dist < fRadius*fRadius )
        return 0;
    dist = sqrt(dist);
    return dist - fRadius;
}

void plSphereIsect::Read(hsStream* s, hsResMgr* mgr)
{
    fCenter.Read(s);
    fWorldCenter.Read(s);
    fRadius = s->ReadLEFloat();
    fMins.Read(s);
    fMaxs.Read(s);
}

void plSphereIsect::Write(hsStream* s, hsResMgr* mgr)
{
    fCenter.Write(s);
    fWorldCenter.Write(s);
    s->WriteLEFloat(fRadius);
    fMins.Write(s);
    fMaxs.Write(s);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plConeIsect::plConeIsect()
    : fLength(kDefLength), fRadAngle(hsConstants::pi<float> * 0.25f), fCapped()
{
    ISetup();
}

void plConeIsect::SetAngle(float rads)
{
    fRadAngle = rads;
    ISetup();
}

void plConeIsect::ISetup()
{
    float sinAng, cosAng;
    hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

    const float kHither = 0.1f;
    fLightToNDC.Reset();
    fLightToNDC.fMap[0][0] =   cosAng / sinAng;
    fLightToNDC.fMap[1][1] =   cosAng / sinAng;
    fLightToNDC.fMap[2][2] =    -(fLength / (fLength - kHither));
    fLightToNDC.fMap[3][3] =    int32_t( 0 );
    fLightToNDC.fMap[3][2] =    int32_t( -1 );
    fLightToNDC.fMap[2][3] =    -(fLength * kHither / (fLength - kHither));
    fLightToNDC.NotIdentity();
}

plVolumeCullResult plConeIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult retVal = kVolumeClear;

    hsPoint2 depth;
    hsVector3 normDir = -fWorldNorm;
    bnd.TestPlane(normDir, depth);
    if( depth.fY < normDir.InnerProduct(fWorldTip) )
        return kVolumeCulled;

    int last = fCapped ? 5 : 4;
    int i;
    for( i = 0; i < last; i++ )
    {
        bnd.TestPlane(fNorms[i], depth);
        if( depth.fY + fDists[i] <= 0 )
            return kVolumeCulled;
        if( depth.fX + fDists[i] <= 0 )
            retVal = kVolumeSplit;
    }
    if( retVal == kVolumeSplit )
    {
        hsVector3 axis = normDir % hsVector3(&bnd.GetCenter(), &fWorldTip);
        hsFastMath::NormalizeAppr(axis);

        hsVector3 perp = axis % normDir;

        float sinAng, cosAng;
        hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

        hsVector3 tangent = normDir + sinAng * perp + (1-cosAng) * (axis % perp);

        hsVector3 normIn = tangent % axis;

        hsVector3 normIn2 = perp + sinAng * (perp % axis) + (1-cosAng) * (axis % (axis % perp));

        bnd.TestPlane(normIn, depth);
        float normInDotTip = normIn.InnerProduct(fWorldTip);
        if( depth.fY < normInDotTip )
            return kVolumeCulled;
    }

    return retVal;
}

float plConeIsect::Test(const hsPoint3& pos) const
{
    uint32_t clampFlags = fCapped ? plClosest::kClamp : plClosest::kClampLower;
    hsPoint3 cp;

    plClosest::PointOnLine(pos,
                  fWorldTip, fWorldNorm,
                  cp,
                  clampFlags);

    float radDist = (pos - cp).Magnitude();
    float axDist = fWorldNorm.InnerProduct(pos - fWorldTip) / fLength;
    if( axDist < 0 )
    {
        return radDist;
    }
    float sinAng, cosAng;

    hsFastMath::SinCosInRangeAppr(fRadAngle, sinAng, cosAng);

    float radius = axDist * sinAng / cosAng;

    radDist -= radius;
    axDist -= fLength;

    if( fCapped && (axDist > 0) )
    {
        return axDist > radDist ? axDist : radDist;
    }

    return radDist > 0 ? radDist : 0;
}

//#define MF_DEBUG_NORM
#ifdef MF_DEBUG_NORM
#define IDEBUG_NORMALIZE( a, b ) { float len = 1.f / a.Magnitude(); a *= len; b *= len; }
#else // MF_DEBUG_NORM
#define IDEBUG_NORMALIZE( a, b )
#endif // MF_DEBUG_NORM

void plConeIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    fWorldTip = l2w.GetTranslate();
    fWorldNorm.Set(l2w.fMap[0][2], l2w.fMap[1][2], l2w.fMap[2][2]);

    fWorldToNDC = fLightToNDC * w2l;
    int i;
    for( i = 0; i < 2; i++ )
    {
        fNorms[i].Set(fWorldToNDC.fMap[3][0] - fWorldToNDC.fMap[i][0], fWorldToNDC.fMap[3][1] - fWorldToNDC.fMap[i][1], fWorldToNDC.fMap[3][2] - fWorldToNDC.fMap[i][2]);
        fDists[i] = fWorldToNDC.fMap[3][3] - fWorldToNDC.fMap[i][3];
        
        IDEBUG_NORMALIZE( fNorms[i], fDists[i] );

        fNorms[i+2].Set(fWorldToNDC.fMap[3][0] + fWorldToNDC.fMap[i][0], fWorldToNDC.fMap[3][1] + fWorldToNDC.fMap[i][1], fWorldToNDC.fMap[3][2] + fWorldToNDC.fMap[i][2]);
        fDists[i+2] = fWorldToNDC.fMap[3][3] + fWorldToNDC.fMap[i][3];

        IDEBUG_NORMALIZE( fNorms[i+2], fDists[i+2] );
    }

    if( fCapped )
    {
        fNorms[4].Set(fWorldToNDC.fMap[3][0] - fWorldToNDC.fMap[2][0], fWorldToNDC.fMap[3][1] - fWorldToNDC.fMap[2][1], fWorldToNDC.fMap[3][2] - fWorldToNDC.fMap[2][2]);
        fDists[4] = fWorldToNDC.fMap[3][3] - fWorldToNDC.fMap[2][3];

        IDEBUG_NORMALIZE( fNorms[4], fDists[4] );
    }
}

void plConeIsect::SetLength(float d)
{
    if( d > 0 )
    {
        fCapped = true;
        fLength = d;
    }
    else
    {
        fCapped = false;
        fLength = kDefLength;
    }
    ISetup();
}

void plConeIsect::Read(hsStream* s, hsResMgr* mgr)
{
    fCapped = s->ReadBOOL();

    fRadAngle = s->ReadLEFloat();
    fLength = s->ReadLEFloat();

    fWorldTip.Read(s);
    fWorldNorm.Read(s);

    fWorldToNDC.Read(s);
    fLightToNDC.Read(s);

    int n = fCapped ? 5 : 4;
    int i;
    for(i = 0; i < n; i++ )
    {
        fNorms[i].Read(s);
        fDists[i] = s->ReadLEFloat();
    }
}

void plConeIsect::Write(hsStream* s, hsResMgr* mgr)
{
    s->WriteBOOL(fCapped);

    s->WriteLEFloat(fRadAngle);
    s->WriteLEFloat(fLength);

    fWorldTip.Write(s);
    fWorldNorm.Write(s);

    fWorldToNDC.Write(s);
    fLightToNDC.Write(s);

    int n = fCapped ? 5 : 4;
    int i;
    for(i = 0; i < n; i++ )
    {
        fNorms[i].Write(s);
        s->WriteLEFloat(fDists[i]);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void plCylinderIsect::ISetupCyl(const hsPoint3& wTop, const hsPoint3& wBot, float radius)
{
    fWorldNorm.Set(&wTop, &wBot);
    fLength = fWorldNorm.Magnitude();
    fMin = fWorldNorm.InnerProduct(wBot);
    fMax = fWorldNorm.InnerProduct(wTop);
    if( fMin > fMax )
    {
        float t = fMin;
        fMin = fMax;
        fMax = t;
    }
    fRadius = radius;
}

void plCylinderIsect::SetCylinder(const hsPoint3& lTop, const hsPoint3& lBot, float radius)
{
    fTop = lTop;
    fBot = lBot;
    fRadius = radius;

    ISetupCyl(fTop, fBot, fRadius);
}

void plCylinderIsect::SetCylinder(const hsPoint3& lBot, const hsVector3& axis, float radius)
{
    fBot = lBot;
    fTop = fBot;
    fTop += axis;

    ISetupCyl(fTop, fBot, radius);

}

void plCylinderIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    hsPoint3 wTop = l2w * fTop;
    hsPoint3 wBot = l2w * fBot;

    ISetupCyl(wTop, wBot, fRadius);
}

plVolumeCullResult plCylinderIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult radVal = kVolumeClear;

    // Central axis test
    hsPoint2 depth;
    bnd.TestPlane(fWorldNorm, depth);
    if( depth.fX > fMax )
        return kVolumeCulled;
    if( depth.fY < fMin )
        return kVolumeCulled;

    if( (depth.fX < fMin)
        ||(depth.fY > fMax) )
    {
        radVal = kVolumeSplit;
    }

    // Radial test
    plVolumeCullResult retVal = kVolumeCulled;

    // Find the closest point on/in the bounds to our central axis.
    // If that closest point is inside the cylinder, we have a hit.
    hsPoint3 corner;
    bnd.GetCorner(&corner);
    hsVector3 axes[3];
    bnd.GetAxes(axes+0, axes+1, axes+2);
    hsPoint3 cp = corner;

    float bndRadiusSq = bnd.GetRadius();
    bndRadiusSq *= bndRadiusSq;

    float radiusSq = fRadius*fRadius;

    float maxClearDistSq = fRadius - bnd.GetRadius();
    maxClearDistSq *= maxClearDistSq;

    int i;
    for( i = 0; i < 3; i++ )
    {
        hsPoint3 cp0;
        hsPoint3 currPt;
        plClosest::PointsOnLines(fWorldBot, fWorldNorm, 
                      cp, axes[i],
                      cp0, currPt,
                      plClosest::kClamp);
        float distSq = (cp0 - currPt).MagnitudeSquared();
        if( distSq < radiusSq )
        {
            if( distSq < maxClearDistSq )
            {
                return kVolumeClear == radVal ? kVolumeClear : kVolumeSplit;
            }
            retVal = kVolumeSplit;
        }
        cp = currPt;
    }

    return retVal;
}

float plCylinderIsect::Test(const hsPoint3& pos) const
{
    hsPoint3 cp;

    plClosest::PointOnLine(pos,
                  fWorldBot, fWorldNorm,
                  cp,
                  plClosest::kClamp);

    float radDist = (pos - cp).Magnitude() - fRadius;
    float axDist = fWorldNorm.InnerProduct(pos - fWorldBot) / fLength;

    if( axDist < 0 )
        axDist = -axDist;
    else
        axDist -= fLength;

    float dist = axDist > radDist ? axDist : radDist;
    
    return dist > 0 ? dist : 0;
}

void plCylinderIsect::Read(hsStream* s, hsResMgr* mgr)
{
    fTop.Read(s);
    fBot.Read(s);
    fRadius = s->ReadLEFloat();

    fWorldBot.Read(s);
    fWorldNorm.Read(s);
    fLength = s->ReadLEFloat();
    fMin = s->ReadLEFloat();
    fMax = s->ReadLEFloat();
}

void plCylinderIsect::Write(hsStream* s, hsResMgr* mgr)
{
    fTop.Write(s);
    fBot.Write(s);
    s->WriteLEFloat(fRadius);

    fWorldBot.Write(s);
    fWorldNorm.Write(s);
    s->WriteLEFloat(fLength);
    s->WriteLEFloat(fMin);
    s->WriteLEFloat(fMax);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void plParallelIsect::SetNumPlanes(size_t n)
{
    fPlanes.resize(n);
}

void plParallelIsect::SetPlane(size_t which, const hsPoint3& locPosOne, const hsPoint3& locPosTwo)
{
    fPlanes[which].fPosOne = locPosOne;
    fPlanes[which].fPosTwo = locPosTwo;
}

void plParallelIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    for (ParPlane& plane : fPlanes)
    {
        hsPoint3 wPosOne = l2w * plane.fPosOne;
        hsPoint3 wPosTwo = l2w * plane.fPosTwo;
        hsVector3 norm;
        norm.Set(&wPosOne, &wPosTwo);
        plane.fNorm = norm;
        float t0 = norm.InnerProduct(wPosOne);
        float t1 = norm.InnerProduct(wPosTwo);

        if( t0 > t1 )
        {
            plane.fMin = t1;
            plane.fMax = t0;
        }
        else
        {
            plane.fMin = t0;
            plane.fMax = t1;
        }
    }
}

plVolumeCullResult plParallelIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult retVal = kVolumeClear;
    for (const ParPlane& plane : fPlanes)
    {
        hsPoint2 depth;
        bnd.TestPlane(plane.fNorm, depth);
        if (depth.fY < plane.fMin)
            return kVolumeCulled;
        if (depth.fX > plane.fMax)
            return kVolumeCulled;
        if (depth.fX < plane.fMin)
            retVal = kVolumeSplit;
        if (depth.fY > plane.fMax)
            retVal = kVolumeSplit;
    }
    return retVal;
}

float plParallelIsect::Test(const hsPoint3& pos) const
{
    float maxDist = 0;
    for (const ParPlane& plane : fPlanes)
    {
        float dist = plane.fNorm.InnerProduct(pos);

        if (dist > plane.fMax)
        {
            dist -= plane.fMax;
            dist *= hsFastMath::InvSqrtAppr(plane.fNorm.MagnitudeSquared());
            if( dist > maxDist )
                maxDist = dist;
        }
        else if (dist < plane.fMin)
        {
            dist = plane.fMin - dist;
            dist *= hsFastMath::InvSqrtAppr(plane.fNorm.MagnitudeSquared());
            if( dist > maxDist )
                maxDist = dist;
        }

    }
    return maxDist;
}

void plParallelIsect::Read(hsStream* s, hsResMgr* mgr)
{
    uint16_t n = s->ReadLE16();

    fPlanes.resize(n);
    for (ParPlane& plane : fPlanes)
    {
        plane.fNorm.Read(s);
        plane.fMin = s->ReadLEFloat();
        plane.fMax = s->ReadLEFloat();

        plane.fPosOne.Read(s);
        plane.fPosTwo.Read(s);
    }
}

void plParallelIsect::Write(hsStream* s, hsResMgr* mgr)
{
    hsAssert(fPlanes.size() < std::numeric_limits<uint16_t>::max(), "Too many planes");
    s->WriteLE16((uint16_t)fPlanes.size());

    for (ParPlane& plane : fPlanes)
    {
        plane.fNorm.Write(s);
        s->WriteLEFloat(plane.fMin);
        s->WriteLEFloat(plane.fMax);

        plane.fPosOne.Write(s);
        plane.fPosTwo.Write(s);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void plConvexIsect::AddPlaneUnchecked(const hsVector3& n, float dist)
{
    SinglePlane plane;
    plane.fNorm = n;
    plane.fPos.Set(0,0,0);
    plane.fDist = dist;

    fPlanes.emplace_back(plane);
}

void plConvexIsect::AddPlane(const hsVector3& n, const hsPoint3& p)
{
    hsVector3 nNorm = n;
    hsFastMath::Normalize(nNorm);

    // First, make sure some idiot isn't adding the same plane in twice.
    // Also, look for the degenerate case of two parallel planes. In that
    // case, take the outer.
    for (SinglePlane& plane : fPlanes)
    {
        const float kCloseToOne = 1.f - 1.e-4f;
        if (plane.fNorm.InnerProduct(nNorm) >= kCloseToOne)
        {
            float dist = nNorm.InnerProduct(p);
            if (dist > plane.fDist)
            {
                plane.fDist = dist;
                plane.fPos = p;
            }
            return;
        }
    }
    SinglePlane plane;
    plane.fNorm = nNorm;
    plane.fPos = p;
    plane.fDist = nNorm.InnerProduct(p);

    fPlanes.emplace_back(plane);
}

void plConvexIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    for (SinglePlane& plane : fPlanes)
    {
        hsPoint3 wPos = l2w * plane.fPos;

        // Normal gets transpose of inverse.
        plane.fWorldNorm.fX = w2l.fMap[0][0] * plane.fNorm.fX
                                + w2l.fMap[1][0] * plane.fNorm.fY
                                + w2l.fMap[2][0] * plane.fNorm.fZ;

        plane.fWorldNorm.fY = w2l.fMap[0][1] * plane.fNorm.fX
                                + w2l.fMap[1][1] * plane.fNorm.fY
                                + w2l.fMap[2][1] * plane.fNorm.fZ;

        plane.fWorldNorm.fZ = w2l.fMap[0][2] * plane.fNorm.fX
                                + w2l.fMap[1][2] * plane.fNorm.fY
                                + w2l.fMap[2][2] * plane.fNorm.fZ;

        hsFastMath::NormalizeAppr(plane.fWorldNorm);

        plane.fWorldDist = plane.fWorldNorm.InnerProduct(wPos);
    }
}

plVolumeCullResult plConvexIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult retVal = kVolumeClear;
    for (const SinglePlane& plane : fPlanes)
    {
        hsPoint2 depth;
        bnd.TestPlane(plane.fWorldNorm, depth);

        if (depth.fX > plane.fWorldDist)
            return kVolumeCulled;

        if (depth.fY > plane.fWorldDist)
            retVal = kVolumeSplit;
    }
    return retVal;
}

float plConvexIsect::Test(const hsPoint3& pos) const
{
    float maxDist = 0;
    for (const SinglePlane& plane : fPlanes)
    {
        float dist = plane.fWorldNorm.InnerProduct(pos) - plane.fWorldDist;

        if( dist > maxDist )
            maxDist = dist;
    }
    return maxDist;
}

void plConvexIsect::Read(hsStream* s, hsResMgr* mgr)
{
    uint16_t n = s->ReadLE16();

    fPlanes.resize(n);
    for (SinglePlane& plane : fPlanes)
    {
        plane.fNorm.Read(s);
        plane.fPos.Read(s);
        plane.fDist = s->ReadLEFloat();

        plane.fWorldNorm.Read(s);
        plane.fWorldDist = s->ReadLEFloat();
    }
}

void plConvexIsect::Write(hsStream* s, hsResMgr* mgr)
{
    hsAssert(fPlanes.size() < std::numeric_limits<uint16_t>::max(), "Too many planes");
    s->WriteLE16((uint16_t)fPlanes.size());

    for (const SinglePlane& plane : fPlanes)
    {
        plane.fNorm.Write(s);
        plane.fPos.Write(s);
        s->WriteLEFloat(plane.fDist);

        plane.fWorldNorm.Write(s);
        s->WriteLEFloat(plane.fWorldDist);
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void plBoundsIsect::SetBounds(const hsBounds3Ext& bnd) 
{ 
    fLocalBounds = bnd; 
    fWorldBounds = bnd;
}

void plBoundsIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    fWorldBounds = fLocalBounds;
    fWorldBounds.Transform(&l2w);
}

plVolumeCullResult plBoundsIsect::Test(const hsBounds3Ext& bnd) const
{
    int retVal = fWorldBounds.TestBound(bnd);
    if( retVal < 0 )
        return kVolumeCulled;
    if( retVal > 0 )
        return kVolumeClear;

    retVal = bnd.TestBound(fWorldBounds);

    return retVal < 0 ? kVolumeCulled : kVolumeSplit;   
}

float plBoundsIsect::Test(const hsPoint3& pos) const
{
    hsAssert(false, "Unimplemented");
    return 0.f;
}

void plBoundsIsect::Read(hsStream* s, hsResMgr* mgr)
{
    fLocalBounds.Read(s);
    fWorldBounds.Read(s);
}

void plBoundsIsect::Write(hsStream* s, hsResMgr* mgr)
{
    fLocalBounds.Write(s);
    fWorldBounds.Write(s);
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plComplexIsect::~plComplexIsect()
{
    for (plVolumeIsect* volume : fVolumes)
        delete volume;
}

void plComplexIsect::AddVolume(plVolumeIsect* v)
{
    fVolumes.emplace_back(v);
}

void plComplexIsect::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l)
{
    for (plVolumeIsect* volume : fVolumes)
        volume->SetTransform(l2w, w2l);
}

void plComplexIsect::Read(hsStream* s, hsResMgr* mgr)
{
    uint16_t n = s->ReadLE16();
    fVolumes.resize(n);
    for (uint16_t i = 0; i < n; i++)
    {
        fVolumes[i] = plVolumeIsect::ConvertNoRef(mgr->ReadCreatable(s));
        hsAssert(fVolumes[i], "Failure reading in a sub-volume");
    }
}

void plComplexIsect::Write(hsStream* s, hsResMgr* mgr)
{
    hsAssert(fVolumes.size() < std::numeric_limits<uint16_t>::max(), "Too many volumes");
    s->WriteLE16((uint16_t)fVolumes.size());
    for (plVolumeIsect* volume : fVolumes)
        mgr->WriteCreatable(s, volume);
}



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plVolumeCullResult plUnionIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult retVal = kVolumeCulled;
    for (plVolumeIsect* volume : fVolumes)
    {
        plVolumeCullResult ret = volume->Test(bnd);

        switch( ret )
        {
        case kVolumeCulled:
            break;
        case kVolumeClear:
            return kVolumeClear;
        case kVolumeSplit:
            retVal = kVolumeSplit;
            break;
        };
    }
    return retVal;
}

float plUnionIsect::Test(const hsPoint3& pos) const
{
    float retVal = 1.e33f;
    for (plVolumeIsect* volume : fVolumes)
    {
        float ret = volume->Test(pos);
        if( ret <= 0 )
            return 0;
        if( ret < retVal )
            retVal = ret;
    }
    return retVal;
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

plVolumeCullResult plIntersectionIsect::Test(const hsBounds3Ext& bnd) const
{
    plVolumeCullResult retVal = kVolumeClear;
    for (plVolumeIsect* volume : fVolumes)
    {
        plVolumeCullResult ret = volume->Test(bnd);

        switch( ret )
        {
        case kVolumeCulled:
            return kVolumeCulled;
        case kVolumeClear:
            break;
        case kVolumeSplit:
            retVal = kVolumeSplit;
            break;
        };
    }
    return retVal;
}

float plIntersectionIsect::Test(const hsPoint3& pos) const
{
    float retVal = -1.f;
    for (plVolumeIsect* volume : fVolumes)
    {
        float ret = volume->Test(pos);
        if( ret > retVal )
            retVal = ret;
    }
    return retVal;
}

