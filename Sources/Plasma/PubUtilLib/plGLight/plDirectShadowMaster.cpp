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

#include "plDirectShadowMaster.h"
#include "plShadowSlave.h"
#include "plPerspDirSlave.h"
#include "plShadowCaster.h"

#include "plIntersect/plVolumeIsect.h"
#include "plMessage/plShadowCastMsg.h"

#include "plLightInfo.h"

#include "hsMatrix44.h"
#include "hsBounds.h"
#include "hsFastMath.h"

////////////////////////////////////////////////////////////////////////////////////
// Point first, directional lights follow
////////////////////////////////////////////////////////////////////////////////////

plDirectShadowMaster::plDirectShadowMaster()
{
}

plDirectShadowMaster::~plDirectShadowMaster()
{
}

std::unique_ptr<plShadowSlave> plDirectShadowMaster::INewSlave(const plShadowCaster* caster)
{
    if( caster->GetPerspective() )
        return std::make_unique<plPerspDirSlave>();
    
    return std::make_unique<plDirectShadowSlave>();
}

plShadowSlave* plDirectShadowMaster::INextSlave(const plShadowCaster* caster)
{
    if( !caster->GetPerspective() )
        return plShadowMaster::INextSlave(caster);

    return fPerspSlavePool.next([this, caster] { return INewSlave(caster); }).get();
}

plShadowSlave* plDirectShadowMaster::IRecycleSlave(plShadowSlave* slave)
{
    if (!fSlavePool.empty() && (fSlavePool.back().get() == slave))
        fSlavePool.pop_back();
    else if (!fPerspSlavePool.empty() && (fPerspSlavePool.back().get() == slave))
        fPerspSlavePool.pop_back();

    return nullptr;
}

void plDirectShadowMaster::IBeginRender()
{
    plShadowMaster::IBeginRender();

    fPerspSlavePool.clear();
    fIsectPool.clear();
}

void plDirectShadowMaster::IComputeWorldToLight(const hsBounds3Ext& wBnd, plShadowSlave* slave) const
{
    hsMatrix44 kFlipDir;
    kFlipDir.Reset();
    kFlipDir.NotIdentity();
    kFlipDir.fMap[2][2] = -1.f;

    hsMatrix44 worldToLight = kFlipDir * fLightInfo->GetWorldToLight();
    hsMatrix44 lightToWorld = fLightInfo->GetLightToWorld() * kFlipDir;

    hsBounds3Ext bnd = wBnd;
    bnd.Transform(&worldToLight);

    hsPoint3 pos = bnd.GetCenter();

    pos.fZ = bnd.GetMins().fZ;

    hsPoint3 wPos = lightToWorld * pos;

    lightToWorld.NotIdentity();
    lightToWorld.fMap[0][3] = wPos[0];
    lightToWorld.fMap[1][3] = wPos[1];
    lightToWorld.fMap[2][3] = wPos[2];

    // Need worldToLight and hate doing an inverse.
    // worldToLight = pureTrans * pureRot;
    // lightToWorld = Inv(pureRot) * Inv(pureTran);
    // So Inv(pureTran) = pureRot * Inv(pureRot) * Inv(pureTran) = pureRot * lightToWorld
    // Make worldToLight pure rotation inverse of lightToWorld
    worldToLight.fMap[0][3] = 0;
    worldToLight.fMap[1][3] = 0;
    worldToLight.fMap[2][3] = 0;

    hsMatrix44 trans = worldToLight * lightToWorld;
    worldToLight.fMap[0][3] = -trans.fMap[0][3];
    worldToLight.fMap[1][3] = -trans.fMap[1][3];
    worldToLight.fMap[2][3] = -trans.fMap[2][3];

//#define CHECK_INVERSE
#ifdef CHECK_INVERSE
    hsMatrix44 inv;
    lightToWorld.GetInverse(&inv);
#endif // CHECK_INVERSE

    slave->fWorldToLight = worldToLight;
    slave->fLightToWorld = lightToWorld;
}

void plDirectShadowMaster::IComputeProjections(plShadowCastMsg* castMsg, plShadowSlave* slave) const
{

    slave->fView.SetPerspective(false);
}

void plDirectShadowMaster::IComputeISect(const hsBounds3Ext& casterBnd, plShadowSlave* slave) const
{
    plBoundsIsect* isect = fIsectPool.next([] { return new plBoundsIsect; }).get();

    const hsBounds3Ext& wBnd = slave->fWorldBounds;

    isect->SetBounds(wBnd);

    slave->fIsect = isect;
}

void plDirectShadowMaster::IComputeBounds(const hsBounds3Ext& casterBnd, plShadowSlave* slave) const
{
    // Plan here is to look at the bounds in the slave's local space.
    // Our slave's bounds will clearly contain the shadow caster's bounds. It will also
    // contain the bnd's corners extended out in light space Z.
    // They will extend fAttenDist farther than the center pointof the bound.

    hsBounds3Ext bnd = casterBnd;
    bnd.Transform(&slave->fWorldToLight);
    
    hsPoint3 farPt = bnd.GetCenter();

    farPt.fZ += slave->fAttenDist;
    
    bnd.Union(&farPt);

    bnd.Transform(&slave->fLightToWorld);

    slave->fWorldBounds = bnd;
}
