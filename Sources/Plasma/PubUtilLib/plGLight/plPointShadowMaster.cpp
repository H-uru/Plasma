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

#include "plPointShadowMaster.h"
#include "plShadowSlave.h"
#include "plShadowCaster.h"

#include "plIntersect/plVolumeIsect.h"
#include "plMessage/plShadowCastMsg.h"


#include "plLightInfo.h"

#include "hsMatrix44.h"
#include "hsBounds.h"
#include "hsFastMath.h"

static const float kMinMinZ = 1.f; // totally random arbitrary number (has to be > 0).

static inline void QuickNorm( hsVector3& a, float& b ) 
{ 
    float len = hsFastMath::InvSqrtAppr((a).MagnitudeSquared()); 
    a *= len; 
    b *= len; 
}

static inline hsVector3 CrossProd(const hsVector3& a, const hsPoint3& b)
{
    return hsVector3(a.fY*b.fZ - a.fZ*b.fY, a.fZ*b.fX - a.fX*b.fZ, a.fX*b.fY - a.fY*b.fX);
}

static inline void InverseOfPureRotTran(const hsMatrix44& src, hsMatrix44& inv)
{
    inv = src;

    // We know this is a pure rotation and translation matrix, so
    // we won't have to do a full inverse. Okay kids, don't try this
    // at home.
    inv.fMap[0][1] = src.fMap[1][0];
    inv.fMap[0][2] = src.fMap[2][0];

    inv.fMap[1][0] = src.fMap[0][1];
    inv.fMap[1][2] = src.fMap[2][1];

    inv.fMap[2][0] = src.fMap[0][2];
    inv.fMap[2][1] = src.fMap[1][2];

    hsPoint3 newTran(-src.fMap[0][3], -src.fMap[1][3], -src.fMap[2][3]);
    inv.fMap[0][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[0][0]);
    inv.fMap[1][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[1][0]);
    inv.fMap[2][3] = newTran.InnerProduct((hsVector3*)&inv.fMap[2][0]);
}

////////////////////////////////////////////////////////////////////////////////////
// Point first
////////////////////////////////////////////////////////////////////////////////////

plPointShadowMaster::plPointShadowMaster()
{
}

plPointShadowMaster::~plPointShadowMaster()
{
}

std::unique_ptr<plShadowSlave> plPointShadowMaster::INewSlave(const plShadowCaster* caster)
{
    return std::make_unique<plPointShadowSlave>();
}

void plPointShadowMaster::IBeginRender()
{
    plShadowMaster::IBeginRender();

    fIsectPool.clear();
}

void plPointShadowMaster::IComputeWorldToLight(const hsBounds3Ext& bnd, plShadowSlave* slave) const
{
    const float kMinMag = 0.5f;
    hsPoint3 from = fLightInfo->GetLightToWorld().GetTranslate();
    hsPoint3 at = bnd.GetCenter();

    hsVector3 atToFrom(&from, &at);
    float distSq = atToFrom.MagnitudeSquared();
    atToFrom *= hsFastMath::InvSqrtAppr(distSq);

    hsPoint2 depth;
    bnd.TestPlane(atToFrom, depth);

    float fromDepth = atToFrom.InnerProduct(from);

    float dist = fromDepth - depth.fY;

    static float kMinDist = 3.f;
    if( dist < kMinDist )
    {
        atToFrom *= kMinDist - dist;
        from += atToFrom;
    }

    hsVector3 up(0.f, 0.f, 1.f);
    if( CrossProd(up, (at - from)).MagnitudeSquared() < kMinMag )
        up.Set(0.f, 1.f, 0.f);

    hsMatrix44 w2light;
    w2light.MakeCamera(&from, &at, &up); // mf_flip_up - mf

    hsMatrix44 light2w;
    InverseOfPureRotTran(w2light, light2w);

#ifdef CHECK_INVERSE
    hsMatrix44 inv;
    w2light.GetInverse(&inv);
#endif // CHECK_INVERSE

    slave->fWorldToLight = w2light;
    slave->fLightToWorld = light2w;
}

void plPointShadowMaster::IComputeProjections(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{

    slave->fView.SetPerspective(true);


}

void plPointShadowMaster::IComputeISect(const hsBounds3Ext& bnd, plShadowSlave* slave) const
{
    plBoundsIsect* isect = fIsectPool.next([] { return new plBoundsIsect; }).get();

    const hsBounds3Ext& wBnd = slave->fWorldBounds;

    isect->SetBounds(wBnd);

    slave->fIsect = isect;
}

void plPointShadowMaster::IComputeBounds(const hsBounds3Ext& wBnd, plShadowSlave* slave) const
{
    // Plan here is to look at the bounds in the slave's local space.
    // Our slave's bounds will clearly contain the shadow caster's bounds. It will also
    // contain the bnd's corners projected out along the ray from the slave's position
    // through each corner. They will extend fAttenDist farther than the center point
    // of the bound.

    hsBounds3Ext sBnd = wBnd;
    sBnd.Transform(&slave->fWorldToLight);

    hsBounds3Ext bnd = sBnd;

    float dist = sBnd.GetCenter().fZ;
    dist += slave->fAttenDist;

    float minZ = sBnd.GetMaxs().fZ;

    hsPoint3 p(sBnd.GetMins().fX * dist / minZ, sBnd.GetMins().fY * dist / minZ, dist);
    bnd.Union(&p);

    p.Set(sBnd.GetMaxs().fX * dist / minZ, sBnd.GetMaxs().fY * dist / minZ, dist);
    bnd.Union(&p);

    bnd.Transform(&slave->fLightToWorld);

    slave->fWorldBounds = bnd;

}


