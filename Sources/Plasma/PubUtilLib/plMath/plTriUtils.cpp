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
#include "hsGeometry3.h"
#include <cmath>
#include "plTriUtils.h"

static constexpr float kAlmostZero = 1.e-5f;
static constexpr float kPastZero = -kAlmostZero;
static constexpr float kPastOne = 1.f + kAlmostZero;
static constexpr float kAlmostOne = 1.f - kAlmostZero;
static constexpr float kAlmostZeroSquared = kAlmostZero*kAlmostZero;

static inline hsVector3 Cross(const hsScalarTriple& p0, const hsScalarTriple& p1)
{
    return {p0.fY * p1.fZ - p0.fZ * p1.fY, 
    	       p0.fZ * p1.fX - p0.fX * p1.fZ, 
    	       p0.fX * p1.fY - p0.fY * p1.fX
    };
}


// There's actually a possibly faster way to do all this.
// The barycentric coordinate in 3-space is the same as the barycentric coordinate of the projection
// in 2-space, as long as the projection doesn't degenerate the triangle (i.e. project the tri onto
// a plane perpindicular to the tri). The tri can't be perpindicular to all three major axes, so by
// picking the right one (or just not picking the wrong one), the lengths of the cross products becomes
// just the z component (e.g. v0.x*v1.y - v0.y*v1.x), so all the square roots go away (not to mention all
// the vector math going from 3 component to 2).
plTriUtils::Bary plTriUtils::ComputeBarycentricProjection(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3&p, hsPoint3& out)
{
    hsVector3 v12(&p1, &p2);
    hsVector3 v02(&p0, &p2);
    hsVector3 norm = Cross(v12, v02);

    float invLenSq12 = norm.MagnitudeSquared();
    if( invLenSq12 < kAlmostZero )
        return kDegenerateTri; // degenerate triangle

    invLenSq12 = 1.f / invLenSq12;

    p += norm * (hsVector3(&p2, &p).InnerProduct(norm) * invLenSq12);

    hsVector3 vp2(&p, &p2);
    hsVector3 v0 = Cross(v12, vp2);
    hsVector3 v1 = Cross(vp2, v02);

    return IComputeBarycentric(norm, invLenSq12, v0, v1, out);
}

plTriUtils::Bary plTriUtils::ComputeBarycentric(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out)
{
    hsVector3 v12(&p1, &p2);
    hsVector3 v02(&p0, &p2);
    hsVector3 norm = Cross(v12, v02);
    float invLenSq12 = norm.MagnitudeSquared();
    if( invLenSq12 < kAlmostZero )
        return kDegenerateTri; // degenerate triangle

    invLenSq12 = 1.f / invLenSq12;

    hsVector3 vp2(&p, &p2);
    hsVector3 v0 = Cross(v12, vp2);
    hsVector3 v1 = Cross(vp2, v02);

    return IComputeBarycentric(norm, invLenSq12, v0, v1, out);
}


plTriUtils::Bary plTriUtils::IComputeBarycentric(const hsVector3& v12, float invLenSq12, const hsVector3& v0, const hsVector3& v1, hsPoint3& out)
{
    uint32_t  state = 0;

    float lenSq0 = v0.MagnitudeSquared();
    if( lenSq0 < kAlmostZeroSquared )
    {
        // On edge p1-p2;
        out[0] = 0;
        state |= kOnEdge12;
    }
    else
    {
        out[0] = lenSq0 * invLenSq12;
        out[0] = sqrt(out[0]);
        // 
        if( v0.InnerProduct(v12) < 0 )
        {
            out[0] = -out[0];
            state |= kOutsideTri;
        }
        else if( out[0] > kPastOne )
            state |= kOutsideTri;
        else if( out[0] > kAlmostOne )
            state |= kOnVertex0;
    }
    
    float lenSq1 = v1.MagnitudeSquared();
    if( lenSq1 < kAlmostZeroSquared )
    {
        // On edge p0-p2
        out[1] = 0;
        state |= kOnEdge02;
    }
    else
    {
        out[1] = lenSq1 * invLenSq12;
        out[1] = sqrt(out[1]);

        if( v1.InnerProduct(v12) < 0 )
        {
            out[1] = -out[1];
            state |= kOutsideTri;
        }
        else if( out[1] > kPastOne )
            state |= kOutsideTri;
        else if( out[1] > kAlmostOne )
            state |= kOnVertex1;
    }

    // Could make more robust against precision problems
    // by repeating above for out[2], then normalizing
    // so sum(out[i]) = 1.f
    out[2] = 1.f - out[0] - out[1];

    if( out[2] < kPastZero )
        state |= kOutsideTri;
    else if( out[2] < kAlmostZero )
        state |= kOnEdge01;
    else if( out[2] > kAlmostOne )
        state |= kOnVertex2;

    /*
    if( a,b,c outside range [0..1] )
        p is outside tri;
    else if( a,b,c == 1 )
        p is on vert;
    else if( a,b,c == 0 )
        p is on edge;
    */

    if( state & kOutsideTri )
        return kOutsideTri;

    if( state & kOnVertex )
        return Bary(state & kOnVertex);

    if( state & kOnEdge )
        return Bary(state & kOnEdge);

    return kInsideTri;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int plTriUtils::ISelectAxis(const hsVector3& norm)
{
    int retVal = -2;
    float maxDim = 0;
    int i;
    for( i = 0; i < 3; i++ )
    {
        if( norm[i] > maxDim )
        {
            maxDim = norm[i];
            retVal = i;
        }
        else if( -norm[i] > maxDim )
        {
            maxDim = -norm[i];
            retVal = i;
        }
    }
    return retVal;
}

bool plTriUtils::IFastBarycentric(int iAx, const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out)
{
    if( --iAx < 0 )
        iAx = 2;
    int jAx = iAx - 1;
    if( jAx < 0 )
        jAx = 2;

    hsVector3 v02(&p0, &p2);
    hsVector3 v12(&p1, &p2);

    float totArea = v02[iAx] * v12[jAx] - v02[jAx] * v12[iAx];
    hsAssert(totArea != 0, "Should have already filtered degerate tris and degenerate projection");
    
    float invTotArea = 1.f / totArea;

    hsVector3 vp2(&p, &p2);

    float aArea = vp2[iAx] * v12[jAx] - vp2[jAx] * v12[iAx];

    float bArea = v02[iAx] * vp2[jAx] - v02[jAx] * vp2[iAx];

    out[0] = aArea * invTotArea;
    out[1] = bArea * invTotArea;
    out[2] = 1.f - out[0] - out[1];

    return true;
}

bool plTriUtils::FastBarycentricProjection(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3&p, hsPoint3& out)
{
    hsVector3 v02(&p0, &p2);
    hsVector3 v12(&p1, &p2);

    hsVector3 norm = Cross(v12, v02);
    float invLenSq12 = norm.MagnitudeSquared();
    if( invLenSq12 < kAlmostZero )
        return false; // degenerate triangle

    invLenSq12 = 1.f / invLenSq12;

    hsVector3 del(&p0, &p);
    float delDotNormOverLenSq = del.InnerProduct(norm) * invLenSq12;

    p += norm * delDotNormOverLenSq;

    int iAx = ISelectAxis(norm);
    hsAssert(iAx >= 0, "Should have already picked out degenerate tris");

    return IFastBarycentric(iAx, p0, p1, p2, p, out);
}

bool plTriUtils::FastBarycentric(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsPoint3&p, hsPoint3& out)
{
    hsVector3 v02(&p0, &p2);
    hsVector3 v12(&p1, &p2);
    int iAx = ISelectAxis(Cross(v12, v02));
    if( iAx < 0 )
        return false;
    return IFastBarycentric(iAx, p0, p1, p2, p, out);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////
bool plTriUtils::ProjectOntoPlane(const hsVector3& norm, float dist, hsPoint3& p)
{
    float normMagSq = norm.MagnitudeSquared();
    if( normMagSq > kAlmostZero )
    {
        dist /= normMagSq;

        p += norm * dist;

        return true;
    }
    return false;
}

bool plTriUtils::ProjectOntoPlane(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, hsPoint3& p)
{
    hsVector3 v02(&p0, &p2);
    hsVector3 v12(&p1, &p2);

    hsVector3 norm = v12 % v02;

    float dist = norm.InnerProduct(p0 - p);

    return ProjectOntoPlane(norm, dist, p);
}

bool plTriUtils::ProjectOntoPlaneAlongVector(const hsVector3& norm, float dist, const hsVector3& vec, hsPoint3& p)
{
    float s = norm.InnerProduct(vec);
    const float kAlmostZero = 1.e-5f;
    if( (s > kAlmostZero)||(s < kPastZero) )
    {
        dist /= s;

        p += vec * dist;

        return true;
    }
    return false;
}

bool plTriUtils::ProjectOntoPlaneAlongVector(const hsPoint3& p0, const hsPoint3& p1, const hsPoint3& p2, const hsVector3& vec, hsPoint3& p)
{
    hsVector3 v02(&p0, &p2);
    hsVector3 v12(&p1, &p2);

    hsVector3 norm = v12 % v02;
    float dist = norm.InnerProduct(p0 - p);

    return ProjectOntoPlaneAlongVector(norm, dist, vec, p);
}
