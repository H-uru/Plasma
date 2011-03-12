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
#include "hsGeometry3.h"
#include "hsResMgr.h"

#include "plPhysicalControllerCore.h"
#include "plSwimRegion.h"
#include "plArmatureMod.h"

void plSwimRegionInterface::Read(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Read(s, mgr);

	fDownBuoyancy = s->ReadSwapScalar();
	fUpBuoyancy = s->ReadSwapScalar();
	fMaxUpwardVel = s->ReadSwapScalar();
}

void plSwimRegionInterface::Write(hsStream* s, hsResMgr* mgr)
{
	plObjInterface::Write(s, mgr);

	s->WriteSwapScalar(fDownBuoyancy);
	s->WriteSwapScalar(fUpBuoyancy);
	s->WriteSwapScalar(fMaxUpwardVel);
}

void plSwimRegionInterface::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed)
{
	linearResult.Set(0.f, 0.f, 0.f);
	angularResult = 0.f;
}

/////////////////////////////////////////////////////////////////////////

plSwimCircularCurrentRegion::plSwimCircularCurrentRegion() : 
	fCurrentSO(nil), 
	fRotation(0.f), 
	fPullNearDistSq(1.f),
	fPullNearVel(0.f),
	fPullFarDistSq(1.f),
	fPullFarVel(0.f)
{
}

void plSwimCircularCurrentRegion::Read(hsStream* stream, hsResMgr* mgr)
{
	plSwimRegionInterface::Read(stream, mgr);
	
	fRotation = stream->ReadSwapScalar();
	fPullNearDistSq = stream->ReadSwapScalar();
	fPullNearVel = stream->ReadSwapScalar();
	fPullFarDistSq = stream->ReadSwapScalar();
	fPullFarVel = stream->ReadSwapScalar();
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // currentSO		
}

void plSwimCircularCurrentRegion::Write(hsStream* stream, hsResMgr* mgr)
{
	plSwimRegionInterface::Write(stream, mgr);
	
	stream->WriteSwapScalar(fRotation);
	stream->WriteSwapScalar(fPullNearDistSq);
	stream->WriteSwapScalar(fPullNearVel);
	stream->WriteSwapScalar(fPullFarDistSq);
	stream->WriteSwapScalar(fPullFarVel);
	mgr->WriteKey(stream, fCurrentSO);
}

hsBool plSwimCircularCurrentRegion::MsgReceive(plMessage* msg)
{
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
	if (refMsg)
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
		if (so)
		{
			if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				fCurrentSO = so;
			else if (refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove))
				fCurrentSO = nil;
			
			return true;
		}
	}

	return plSwimRegionInterface::MsgReceive(msg);
}

void plSwimCircularCurrentRegion::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed)
{
	if (elapsed <= 0.f || fCurrentSO == nil || GetProperty(kDisable))
	{
		linearResult.Set(0.f, 0.f, 0.f);
		angularResult = 0.f;
		return;
	}

	hsPoint3 center, pos;
	center.Set(&fCurrentSO->GetLocalToWorld().GetTranslate());

	plKey worldKey = physical->GetSubworld();
	if (worldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
		center = so->GetWorldToLocal() * center;
	}

	center.fZ = 0.f; // Just doing 2D

	physical->GetPositionSim(pos);
	
	hsBool applyPull = true;
	hsVector3 pos2Center(center.fX - pos.fX, center.fY - pos.fY, 0.f);
	hsScalar pullVel;
	hsScalar distSq = pos2Center.MagnitudeSquared();
	if (distSq < .5)
	{
		// Don't want to pull us too close to the center, or we
		// get this annoying jitter.
		pullVel = 0.f;
	}
	else if (distSq <= fPullNearDistSq)
		pullVel = fPullNearVel;
	else if (distSq >= fPullFarDistSq)
		pullVel = fPullFarVel;
	else
		pullVel = fPullNearVel + (fPullFarVel - fPullNearVel) * (distSq - fPullNearDistSq) / (fPullFarDistSq - fPullNearDistSq);

	hsVector3 pull = pos2Center;
	pull.Normalize();
	linearResult.Set(pull.fY, -pull.fX, pull.fZ);
	
	pull *= pullVel;
	linearResult *= fRotation;
	linearResult += pull;

	hsVector3 v1 = linearResult * elapsed - pos2Center;
	hsVector3 v2 = -pos2Center;
	hsScalar invCos = v1.InnerProduct(v2) / v1.Magnitude() / v2.Magnitude();
	if (invCos > 1)
		invCos = 1;
	if (invCos < -1)
		invCos = -1;
	angularResult = hsACosine(invCos) / elapsed;

// 	hsAssert(real_finite(linearResult.fX) &&
// 			 real_finite(linearResult.fY) &&
// 			 real_finite(linearResult.fZ) &&
// 			 real_finite(angularResult), "Bad water current computation.");	
}	

/////////////////////////////////////////////////////////////////////////////////////

plSwimStraightCurrentRegion::plSwimStraightCurrentRegion() : 
	fCurrentSO(nil), 
	fNearDist(1.f),
	fNearVel(0.f),
	fFarDist(1.f),
	fFarVel(0.f)
{
}

void plSwimStraightCurrentRegion::Read(hsStream* stream, hsResMgr* mgr)
{
	plSwimRegionInterface::Read(stream, mgr);
	
	fNearDist = stream->ReadSwapScalar();
	fNearVel = stream->ReadSwapScalar();
	fFarDist = stream->ReadSwapScalar();
	fFarVel = stream->ReadSwapScalar();
	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, -1, -1), plRefFlags::kActiveRef); // currentSO		
}

void plSwimStraightCurrentRegion::Write(hsStream* stream, hsResMgr* mgr)
{
	plSwimRegionInterface::Write(stream, mgr);
	
	stream->WriteSwapScalar(fNearDist);
	stream->WriteSwapScalar(fNearVel);
	stream->WriteSwapScalar(fFarDist);
	stream->WriteSwapScalar(fFarVel);
	mgr->WriteKey(stream, fCurrentSO);
}

hsBool plSwimStraightCurrentRegion::MsgReceive(plMessage* msg)
{
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef(msg);
	if (refMsg)
	{
		plSceneObject *so = plSceneObject::ConvertNoRef(refMsg->GetRef());
		if (so)
		{
			if (refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace))
				fCurrentSO = so;
			else if (refMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove))
				fCurrentSO = nil;
			
			return true;
		}
	}

	return plSwimRegionInterface::MsgReceive(msg);
}

void plSwimStraightCurrentRegion::GetCurrent(plPhysicalControllerCore *physical, hsVector3 &linearResult, hsScalar &angularResult, hsScalar elapsed)
{
	angularResult = 0.f;

	if (elapsed <= 0.f || GetProperty(kDisable))
	{
		linearResult.Set(0.f, 0.f, 0.f);
		return;
	}

	hsPoint3 center, pos;
	hsVector3 current = fCurrentSO->GetLocalToWorld() * hsVector3(0.f, 1.f, 0.f);
	center.Set(&fCurrentSO->GetLocalToWorld().GetTranslate());
	physical->GetPositionSim(pos);

	plKey worldKey = physical->GetSubworld();
	if (worldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(worldKey->ObjectIsLoaded());
		hsMatrix44 w2l = so->GetWorldToLocal();
		center = w2l * center;
		current = w2l * current;
	}

	hsVector3 pos2Center(center.fX - pos.fX, center.fY - pos.fY, 0.f);
	hsScalar dist = current.InnerProduct(pos - center);
	hsScalar pullVel;
	
	if (dist <= fNearDist)
		pullVel = fNearVel;
	else if (dist >= fFarDist)
		pullVel = fFarVel;
	else
		pullVel = fNearVel + (fFarVel - fNearVel) * (dist - fNearDist) / (fFarDist - fNearDist);

	linearResult = current * pullVel;
}	
