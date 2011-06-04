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
#if 0
// havok first
#include <hkdynamics/entity/rigidbody.h>
#include <hkdynamics/world/subspace.h>

#include "plAntiGravAction.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../plHavok1/plHKPhysical.h"
#include "../plAvatar/plSwimRegion.h"
#include "hsTimer.h"

// This is meant to be a specific physicsAction for the swim behavior
plAntiGravAction::plAntiGravAction(plHKPhysical *physical, plAGApplicator *rootApp) : 
	plAnimatedCallbackAction(physical, rootApp),
	fOnGround(false),
	fBuoyancy(1.f),
	fSurfaceHeight(0.f),
	fCurrentRegion(nil),
	fHadContacts(false)
{
}

plSimDefs::ActionType plAntiGravAction::GetType()
{
	return plSimDefs::kAntiGravAction;
}

void plAntiGravAction::apply(Havok::Subspace &space, Havok::hkTime time)
{
	double elapsed = time.asDouble() - getRefresh().asDouble();
	setRefresh(time);
	
	IAdjustBuoyancy();		
	Havok::RigidBody *body = fPhysical->GetBody();
	float mass = body->getMass();
	Havok::Vector3 gravity = space.getGravity();
	Havok::Vector3 force = -gravity * (mass * fBuoyancy);
	body->applyForce(force);
	
	hsVector3 vel;
	fPhysical->GetLinearVelocitySim(vel);
	fAnimPosVel.fZ = vel.fZ;
	
	hsVector3 linCurrent(0.f, 0.f, 0.f);
	hsScalar angCurrent = 0.f;
	if (fCurrentRegion != nil)
		fCurrentRegion->GetCurrent(fPhysical, linCurrent, angCurrent, (hsScalar)elapsed);
	
	int numContacts = fPhysical->GetNumContacts();
	fHadContacts = (numContacts > 0);

	const Havok::Vector3 straightUp(0.0f, 0.0f, 1.0f);	
	fOnGround = false;
	int i;
	for (i = 0; i < numContacts; i++)
	{
		const Havok::ContactPoint *contact = fPhysical->GetContactPoint(i);	
		hsScalar dotUp = straightUp.dot(contact->m_normal);
		if (dotUp > .5)
		{
			fOnGround = true;
			break;
		}
	}	

	fPhysical->SetLinearVelocitySim(fAnimPosVel + linCurrent);
	fPhysical->SetAngularVelocitySim(hsVector3(0.f, 0.f, fAnimAngVel + fTurnStr + angCurrent));	
}

void plAntiGravAction::SetSurface(plSwimRegionInterface *region, hsScalar surfaceHeight)
{
	fCurrentRegion = region;
	if (region != nil)
		fSurfaceHeight = surfaceHeight;
}

void plAntiGravAction::IAdjustBuoyancy()
{
	// "surface depth" refers to the depth our handle object should be below
	// the surface for the avatar to be "at the surface"
	static const float surfaceDepth = 4.0f;
	// 1.0 = neutral buoyancy
	// 0 = no buoyancy (normal gravity)
	// 2.0 = opposite of gravity, floating upwards
	static const float buoyancyAtSurface = 1.0f;

	if (fCurrentRegion == nil)
	{
		fBuoyancy = 0.f;
		return;
	}

	hsMatrix44 l2w, w2l;
	fPhysical->GetTransform(l2w, w2l);
	float depth = fSurfaceHeight - surfaceDepth - l2w.GetTranslate().fZ;	
	if (depth < -1)
		fBuoyancy = 0.f; // Same as being above ground. Plain old gravity.
	else if (depth < 0)
		fBuoyancy = 1 + depth;
	else 
	{
		hsVector3 vel;
		fPhysical->GetLinearVelocitySim(vel);
		if (vel.fZ > 0)
		{
			if (vel.fZ > fCurrentRegion->fMaxUpwardVel)
			{
				vel.fZ = fCurrentRegion->fMaxUpwardVel;
				fPhysical->SetLinearVelocitySim(vel);
			}
			else
			{
				if (depth > 1)
					fBuoyancy = fCurrentRegion->fUpBuoyancy;
				else
					fBuoyancy = (fCurrentRegion->fUpBuoyancy - 1) * depth + 1;
			}
		}
		else
		{
			if (depth > 1)
				fBuoyancy = fCurrentRegion->fDownBuoyancy;
			else
				fBuoyancy = (fCurrentRegion->fDownBuoyancy - 1) * depth + 1;
		}
	}	
}

#endif