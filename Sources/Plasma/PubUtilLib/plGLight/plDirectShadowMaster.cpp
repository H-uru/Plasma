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

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/

#include "hsTypes.h"

#include "plDirectShadowMaster.h"
#include "plShadowSlave.h"
#include "plPerspDirSlave.h"
#include "plShadowCaster.h"
#include "../plMessage/plShadowCastMsg.h"

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
	fIsectPool.SetCount(fIsectPool.GetNumAlloc());
	int i;
	for( i = 0; i < fIsectPool.GetCount(); i++ )
		delete fIsectPool[i];
}

plShadowSlave* plDirectShadowMaster::INewSlave(const plShadowCaster* caster)
{
	if( caster->GetPerspective() )
		return TRACKED_NEW plPerspDirSlave;
	
	return TRACKED_NEW plDirectShadowSlave;
}

plShadowSlave* plDirectShadowMaster::INextSlave(const plShadowCaster* caster)
{
	if( !caster->GetPerspective() )
		return plShadowMaster::INextSlave(caster);

	int iSlave = fPerspSlavePool.GetCount();
	fPerspSlavePool.ExpandAndZero(iSlave+1);
	plShadowSlave* slave = fPerspSlavePool[iSlave];
	if( !slave )
	{
		fPerspSlavePool[iSlave] = slave = INewSlave(caster);
	}
	return slave;
}

plShadowSlave* plDirectShadowMaster::IRecycleSlave(plShadowSlave* slave)
{
	if( fSlavePool.GetCount() && (fSlavePool[fSlavePool.GetCount()-1] == slave) )
		fSlavePool.SetCount(fSlavePool.GetCount()-1);
	else
	if( fPerspSlavePool.GetCount() && (fPerspSlavePool[fPerspSlavePool.GetCount()-1] == slave) )
		fPerspSlavePool.SetCount(fPerspSlavePool.GetCount()-1);

	return nil;
}

void plDirectShadowMaster::IBeginRender()
{
	plShadowMaster::IBeginRender();

	fPerspSlavePool.SetCount(0);
	fIsectPool.SetCount(0);
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
	int iIsect = fIsectPool.GetCount();
	fIsectPool.ExpandAndZero(iIsect+1);
	if( !fIsectPool[iIsect] )
	{
		fIsectPool[iIsect] = TRACKED_NEW plBoundsIsect;
	}
	plBoundsIsect* isect = fIsectPool[iIsect];

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
