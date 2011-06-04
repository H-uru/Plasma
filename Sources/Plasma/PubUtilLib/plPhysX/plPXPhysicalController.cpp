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
#include "plPXPhysicalController.h"
#include "plSimulationMgr.h"
#include "plPXPhysical.h"
#include "plPXConvert.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnMessage/plCorrectionMsg.h"
#include "../plAvatar/plArmatureMod.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../plPhysical/plPhysicalProxy.h"

#include "../pnMessage/plSetNetGroupIDMsg.h"
#include "../plMessage/plCollideMsg.h"

#include "NxPhysics.h"
#include "ControllerManager.h" 
#include "NxCapsuleController.h"
#include "NxCapsuleShape.h"

#define kPhysxSkinWidth 0.1f
#define kPhysZOffset ((fRadius + (fHeight / 2)) + kPhysxSkinWidth)
#define kSLOPELIMIT (cosf(NxMath::degToRad(55.f)))
//#define kPhysicalHeightFudge 0.4f   // this fudge was used for PhysX 2.4
#define kPhysicalHeightFudge 0.0f

//#define STEP_OFFSET	1.0f
#define STEP_OFFSET	0.5f
//#define STEP_OFFSET	0.15f


#ifndef PLASMA_EXTERNAL_RELEASE
hsBool plPXPhysicalController::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE

static ControllerManager gControllerMgr;
static std::vector<plPXPhysicalController*> gControllers;
static gRebuildCache = false;

// KLUDGE: From plPXPhysical.cpp
bool CompareMatrices(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance);

plPhysicalController* plPhysicalController::Create(plKey ownerSO, hsScalar height, hsScalar width)
{
	hsScalar radius = width / 2.f;
	//hsScalar realHeight = height - width;
	hsScalar realHeight = height - radius + kPhysicalHeightFudge;
	return TRACKED_NEW plPXPhysicalController(ownerSO, radius, realHeight);
}

//////////////////////////////////////////////////////////////////////////

plPXPhysicalController* plPXPhysicalController::FindController(NxController* controller)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalController* ac = gControllers[i];
		if (ac->fController == controller)
			return ac;
	}
	return nil;
}

plPXPhysicalController* plPXPhysicalController::GetController(NxActor& actor, bool* isController)
{
	*isController = false;
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalController* ac = gControllers[i];
		if (ac->fController && ac->fController->getActor() == &actor)
		{
			*isController = true;
			return ac;
		}
		if ( ac->fKinematicActor == &actor)
		{
			return ac;
		}
	}

	return nil;
}
void plPXPhysicalController::GetWorldSpaceCapsule(NxCapsule& cap)
{
	if(fController){
		int numshapes=fController->getActor()->getNbShapes();
		if (numshapes==1) 
		{//there should only be one shape on a controller
			NxShape* const *shapes=fController->getActor()->getShapes();
		//and since it is a capsule controller it better be a capsule;
			NxCapsuleShape *capShape = shapes[0]->isCapsule();
			if(capShape) capShape->getWorldCapsule(cap);
			
		}
	}

}
bool plPXPhysicalController::AnyControllersInThisWorld(plKey world)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalController* ac = gControllers[i];
		if (ac->GetSubworld() == world)
			return true;
	}
	return false;
}

int plPXPhysicalController::NumControllers()
{
	return gControllers.size();
}
int plPXPhysicalController::GetControllersInThisSubWorld(plKey world, int maxToReturn,plPXPhysicalController** bufferout)
{
	int i=0;
	for (int j=0;j<gControllers.size();j++)
	{
		plPXPhysicalController* ac = gControllers[i];
		if (ac->GetSubworld()==world)
		{
			if(i<maxToReturn)
			{
				bufferout[i]=ac;
				i++;
			}
		}
	}
	return i;

}
int plPXPhysicalController::GetNumberOfControllersInThisSubWorld(plKey world)
{
	int i=0;
	for (int j=0;j<gControllers.size();j++)
	{
		plPXPhysicalController* ac = gControllers[i];
		if (ac->GetSubworld()==world)i++;
	}
	return i;
}
void plPXPhysicalController::Update(bool prestep, hsScalar delSecs)
{
	// Apparently the user data field of the controllers is broken
// 	UInt32 count = gControllerMgr.getNbControllers();
// 	NxController* controllers = (NxController*)gControllerMgr.getControllers();
// 
// 	for (int i = 0; i < count; i++)
// 	{
// 		plPXPhysicalController* ac = (plPXPhysicalController*)controllers[i].getAppData();
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalController* ac = gControllers[i];

		hsAssert(ac, "Bad avatar controller");
		if (prestep)
		{
			if (gRebuildCache)
				ac->fController->reportSceneChanged();
			ac->IApply(delSecs);
		}
		else
		{
			gControllerMgr.updateControllers();
			ac->ISendUpdates(delSecs);

			if (ac->GetSubworldCI())
				ac->fPrevSubworldW2L = ac->GetSubworldCI()->GetWorldToLocal();
			else
			{
				if (!ac->fPrevSubworldW2L.IsIdentity())
					ac->fPrevSubworldW2L.Reset();
			}
		}
	}

	gRebuildCache = false;
}

void plPXPhysicalController::RebuildCache()
{
	gRebuildCache = true;
}
void plPXPhysicalController::IInformDetectors(bool entering)
{
	static const NxU32 DetectorFlag= 1<<plSimDefs::kGroupDetector;
	if (fController)
	{
		NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
		int kNumofShapesToStore=30;
		NxCapsule cap;
		GetWorldSpaceCapsule(cap);
		NxShape* shapes[30];
		int numCollided=scene->overlapCapsuleShapes(cap,NX_ALL_SHAPES,kNumofShapesToStore,shapes,NULL,DetectorFlag,NULL,true);
		for (int i=0;i<numCollided;i++)
		{
			NxActor* myactor=&(shapes[i]->getActor());
			
			if (myactor)
			{
				
				plPXPhysical* physical = (plPXPhysical*)myactor->userData;
				if (physical)
				{
					plCollideMsg* msg = TRACKED_NEW plCollideMsg;
					
					msg->fOtherKey = fOwner;
					msg->fEntering = entering;
					msg->AddReceiver(physical->GetKey());
					msg->Send();
				}
			}
		}


	}

}

//////////////////////////////////////////////////////////////////////////


class PXControllerHitReport : public NxUserControllerHitReport
{
public:
	virtual NxControllerAction onShapeHit(const NxControllerShapeHit& hit)
	{
		plPXPhysicalController* ac = plPXPhysicalController::FindController(hit.controller);

		NxActor& actor = hit.shape->getActor();
		plPXPhysical* phys = (plPXPhysical*)actor.userData;

		static hsScalar SlopeLimit = kSLOPELIMIT;
		hsVector3 normal = plPXConvert::Vector(hit.worldNormal);
		hsScalar dot = normal * kAvatarUp;
		if ( dot < SlopeLimit )
			ac->AddSlidingNormal(normal);
		else
			ac->GroundHit();

#ifndef PLASMA_EXTERNAL_RELEASE
		plDbgCollisionInfo info;
		info.fNormal = normal;
		info.fSO = plSceneObject::ConvertNoRef(phys->GetObjectKey()->ObjectIsLoaded());
		info.fOverlap = false;
		NxShape* const *shapes = hit.controller->getActor()->getShapes();
		int numShapes = hit.controller->getActor()->getNbShapes();
		int i;
		for (i = 0; i < numShapes; i++)
		{
			// should only be one capsule shape
			const NxCapsuleShape *capShape = shapes[i]->isCapsule();
			if (capShape)
			{
				NxCapsule cap;
				capShape->getWorldCapsule(cap);
				if (hit.shape->checkOverlapCapsule(cap))
					info.fOverlap = true;
			}
		}
		ac->fDbgCollisionInfo.Append(info);
#endif PLASMA_EXTERNAL_RELEASE

		// If the avatar hit a movable physical, apply some force to it.
		if (actor.isDynamic() )
		{
			if ( !actor.readBodyFlag(NX_BF_KINEMATIC) && !actor.readBodyFlag(NX_BF_FROZEN))
			{
				// If this is the local avatar, we need to take ownership of this
				// dynamic if we haven't already
				if (ac->fLOSDB == plSimDefs::kLOSDBLocalAvatar && !phys->IsLocallyOwned() &&
					!phys->GetProperty(plSimulationInterface::kNoOwnershipChange))
				{
					plSynchedObject* obj = plSynchedObject::ConvertNoRef(phys->GetObjectKey()->ObjectIsLoaded());
					
					obj->SetNetGroupConstant(plNetGroup::kNetGroupLocalPhysicals);

					// Tell all the other clients that we own this physical
					plSetNetGroupIDMsg* setNetGroupID = TRACKED_NEW plSetNetGroupIDMsg;
					setNetGroupID->fId = plNetGroup::kNetGroupRemotePhysicals;
					setNetGroupID->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
					setNetGroupID->SetBCastFlag(plMessage::kLocalPropagate, false);
					setNetGroupID->Send(obj->GetKey());
				}

				plSimulationMgr::GetInstance()->ConsiderSynch(phys, nil);

				hsVector3 dir = plPXConvert::Vector(hit.dir);
				// We only allow horizontal pushes. Vertical pushes when we stand on
				// dynamic objects creates useless stress on the solver.
				if (dir.fZ < 0)
				{
					dir.fZ = 0;
					dir.Normalize();
				}

				if (!dir.IsEmpty())
				{
					static hsScalar kForceScale = 5.f;
					//static hsScalar kForceScale = 4.f;
					NxF32 coeff = actor.getMass() * hit.length * kForceScale;
					hsPoint3 pos((hsScalar)hit.worldPos.x, (hsScalar)hit.worldPos.y, (hsScalar)hit.worldPos.z);
					phys->SetHitForce((dir*coeff), pos);
				}
			}
		}
		else  // else if the avatar hit a static
		{
			return NX_ACTION_NONE;
		}

		if (phys && phys->GetProperty(plSimulationInterface::kAvAnimPushable))
		{
 			hsQuat inverseRotation = ac->fLocalRotation.Inverse();
			hsVector3 normal = plPXConvert::Vector(hit.worldNormal);
			ac->fPushingPhysical = phys;
			ac->fFacingPushingPhysical = (inverseRotation.Rotate(&kAvatarForward).InnerProduct(normal) < 0 ? true : false);
		}

		return NX_ACTION_NONE;
	}

	virtual NxControllerAction onControllerHit(const NxControllersHit& hit)
	{
		return NX_ACTION_NONE;
	}

} gMyReport;


//////////////////////////////////////////////////////////////////////////

const hsScalar plPXPhysicalController::kAirTimeThreshold = .1f; // seconds

plPXPhysicalController::plPXPhysicalController(plKey ownerSO, hsScalar radius, hsScalar height)
	: fOwner(ownerSO)
	, fWorldKey(nil)
	, fRadius(radius)
	, fHeight(height)
	, fController(nil)
	, fLinearVelocity(0, 0, 0)
	, fAngularVelocity(0)
	, fAchievedLinearVelocity(0, 0, 0)
	, fLocalPosition(0, 0, 0)
	, fLocalRotation(0, 0, 0, 1)
	, fEnable(true)
	, fEnableChanged(false)
	, fLOSDB(plSimDefs::kLOSDBNone)
	, fGroundHit(false)
	, fFalseGround(false)
	, fTimeInAir(0)
	, fPushingPhysical(nil)
	, fFacingPushingPhysical(false)
	, fProxyGen(nil)
	, fKinematicActor(nil)
	, fKinematic(false)
	, fKinematicChanged(false)
	, fKinematicEnableNextUpdate(false)
	, fHitHead(false)
{
	gControllers.push_back(this);
	fLastGlobalLoc.Reset();
	ICreateController();
	Enable(false);
}

plPXPhysicalController::~plPXPhysicalController()
{
	IDeleteController();

	for (int i = 0; i < gControllers.size(); i++)
	{
		if (gControllers[i] == this)
		{
			gControllers.erase(gControllers.begin()+i);
			break;
		}
	}

	delete fProxyGen;
}

// WARNING: If this is an armatureMod, it'll have its own idea about when
// physics should be enabled/disabled. Use plArmatureModBase::EnablePhysics() instead.
void plPXPhysicalController::Enable(bool enable)
{
	if (fEnable != enable)
	{
		fEnable = enable;
		if (fEnable)
			fEnableChanged = true;
		else
		{
			// See ISendUpdates for why we don't re-enable right away
			fController->setCollision(fEnable);
		}
	}
}

void plPXPhysicalController::AddSlidingNormal(hsVector3 vec) 
{ 
	// We get lots of duplicates, so check.
	int i;
	for (i = 0; i < fSlidingNormals.GetCount(); i++)
	{
		if (hsABS(fSlidingNormals[i].fX - vec.fX) <= .01 &&
			hsABS(fSlidingNormals[i].fY - vec.fY) <= .01 &&
			hsABS(fSlidingNormals[i].fZ - vec.fZ) <= .01)
		{
			return;
		}
	}
	fSlidingNormals.Append(vec); 
}


void plPXPhysicalController::IGetPositionSim(hsPoint3& pos) const
{
	const NxExtendedVec3& nxPos = fController->getPosition();
	pos.Set(hsScalar(nxPos.x), hsScalar(nxPos.y), hsScalar(nxPos.z) - kPhysZOffset);
}

void plPXPhysicalController::SetSubworld(plKey world)
{
	if (fWorldKey != world)
	{
		bool wasEnabled = fEnable;
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
		// PHYSX FIXME - before leaving this world, sending leaving detector events if we are inside a convex hull detector
		hsPoint3 pos;
		IGetPositionSim(pos);
		plSimulationMgr::GetInstance()->UpdateDetectorsInScene(fWorldKey,GetOwner(),pos,false);
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
		//need to inform detectors in the old world that we are leaving
		//IInformDetectors(false);
		//done informing old world

 		IDeleteController();
 		fWorldKey = world;
		ICreateController();
		if (wasEnabled)
			Enable(false);
		// need to disable the kinematic also so that it doesn't trip over random detector regions
		fKinematicActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
		hsMatrix44 globalLoc = fLastGlobalLoc;
		if (GetSubworldCI())
			fPrevSubworldW2L = GetSubworldCI()->GetWorldToLocal();
		ISetGlobalLoc(globalLoc);
		// we need to let the re-enable code put thing in order... so that 0,0,0 is not triggered and ISendUpdates do the enable and update detectors
		if (wasEnabled)
			Enable(true);
		// and then re-enable the kinematic on the next update (ISendUpdates)
		fKinematicEnableNextUpdate = true;
		plPXPhysicalController::RebuildCache();
	}
}

const plCoordinateInterface* plPXPhysicalController::GetSubworldCI() const
{
	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		if (so)
			return so->GetCoordinateInterface();
	}
	return nil;
}

void plPXPhysicalController::GetState(hsPoint3& pos, float& zRot)
{
	// Temporarily use the position point while we get the z rotation
	fLocalRotation.NormalizeIfNeeded();
	fLocalRotation.GetAngleAxis(&zRot, (hsVector3*)&pos);

	if (pos.fZ < 0)
		zRot = (2 * hsScalarPI) - zRot; // axis is backwards, so reverse the angle too

	pos = fLocalPosition;
}

void plPXPhysicalController::SetState(const hsPoint3& pos, float zRot)
{
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		hsQuat worldRot;
		hsVector3 zAxis(0.f, 0.f, 1.f);
		worldRot.SetAngleAxis(zRot, zAxis);

		hsMatrix44 l2w, w2l;
		worldRot.MakeMatrix(&l2w);
		l2w.SetTranslate(&pos);

		// Localize new position and rotation to global coords if we're in a subworld
		const plCoordinateInterface* ci = GetSubworldCI();
		if (ci)
		{
			const hsMatrix44& subworldL2W = ci->GetLocalToWorld();
			l2w = subworldL2W * l2w;
		}

		l2w.GetInverse(&w2l);
		so->SetTransform(l2w, w2l);
		so->FlushTransform();
	}
}

void plPXPhysicalController::ISetGlobalLoc(const hsMatrix44& l2w)
{
	fLastGlobalLoc = l2w;

	// Update our subworld position and rotation
	const plCoordinateInterface* subworldCI = GetSubworldCI();
	if (subworldCI)
	{
		const hsMatrix44& w2s = fPrevSubworldW2L;
		hsMatrix44 l2s = w2s * l2w;

		l2s.GetTranslate(&fLocalPosition);
		fLocalRotation.SetFromMatrix44(l2s);
	}
	else
	{
		l2w.GetTranslate(&fLocalPosition);
		fLocalRotation.SetFromMatrix44(l2w);
	}

	hsMatrix44 w2l;
	l2w.GetInverse(&w2l);
	if (fProxyGen)
		fProxyGen->SetTransform(l2w, w2l);

	// Update the physical position
	NxExtendedVec3 nxPos(fLocalPosition.fX, fLocalPosition.fY, fLocalPosition.fZ + kPhysZOffset);
	fController->setPosition(nxPos);
	IMatchKinematicToController();
}

void plPXPhysicalController::IMatchKinematicToController()
{
	if ( fKinematicActor)
	{
		NxExtendedVec3 cPos = fController->getPosition();
		NxVec3 prevKinPos = fKinematicActor->getGlobalPosition();
		NxVec3 kinPos;
		kinPos.x = (NxReal)cPos.x;
		kinPos.y = (NxReal)cPos.y;
		kinPos.z = (NxReal)cPos.z;
		if (plSimulationMgr::fExtraProfile)
			SimLog("Match setting kinematic from %f,%f,%f to %f,%f,%f",prevKinPos.x,prevKinPos.y,prevKinPos.z,kinPos.x,kinPos.y,kinPos.z );
		fKinematicActor->setGlobalPosition(kinPos);
	}
}

void plPXPhysicalController::IMoveKinematicToController(hsPoint3& pos)
{
	if ( fKinematicActor)
	{
		NxVec3 kinPos = fKinematicActor->getGlobalPosition();
		if ( abs(kinPos.x-pos.fX) + abs(kinPos.y-pos.fY) + (abs(kinPos.z-pos.fZ-kPhysZOffset)) > 0.0001f)
		{
			NxVec3 newPos;
			newPos.x = (NxReal)pos.fX;
			newPos.y = (NxReal)pos.fY;
			newPos.z = (NxReal)pos.fZ+kPhysZOffset;
			if (fEnable || fKinematic)
			{
				if (plSimulationMgr::fExtraProfile)
					SimLog("Moving kinematic from %f,%f,%f to %f,%f,%f",pos.fX,pos.fY,pos.fZ+kPhysZOffset,kinPos.x,kinPos.y,kinPos.z );
				// use the position
				fKinematicActor->moveGlobalPosition(newPos);
			}
			else
			{
				if (plSimulationMgr::fExtraProfile)
					SimLog("Setting kinematic from %f,%f,%f to %f,%f,%f",pos.fX,pos.fY,pos.fZ+kPhysZOffset,kinPos.x,kinPos.y,kinPos.z );
				fKinematicActor->setGlobalPosition(newPos);
			}
		}
	}
}

void plPXPhysicalController::ISetKinematicLoc(const hsMatrix44& l2w)
{

	hsPoint3 kPos;
	// Update our subworld position and rotation
	const plCoordinateInterface* subworldCI = GetSubworldCI();
	if (subworldCI)
	{
		const hsMatrix44& w2s = subworldCI->GetWorldToLocal();
		hsMatrix44 l2s = w2s * l2w;

		l2s.GetTranslate(&kPos);
	}
	else
	{
		l2w.GetTranslate(&kPos);
	}

	hsMatrix44 w2l;
	l2w.GetInverse(&w2l);
	if (fProxyGen)
		fProxyGen->SetTransform(l2w, w2l);

	// add z offset
	kPos.fZ += kPhysZOffset;
	// Update the physical position of kinematic
	if (fEnable || fKinematic)
		fKinematicActor->moveGlobalPosition(plPXConvert::Point(kPos));
	else
		fKinematicActor->setGlobalPosition(plPXConvert::Point(kPos));
}


void plPXPhysicalController::Kinematic(bool state)
{
	if (fKinematic != state)
	{
		fKinematic = state;
		if (fKinematic)
		{
			// See ISendUpdates for why we don't re-enable right away
			fController->setCollision(false);
#ifdef PHYSX_KINEMATIC_IS_DISABLED
			fKinematicActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
#endif
		}
		else
		{
			fKinematicChanged = true;
		}
	}
}

bool plPXPhysicalController::IsKinematic()
{
	if (fKinematicActor)
	{
#ifdef PHYSX_KINEMATIC_IS_DISABLED
		if (!fKinematicActor->readActorFlag(NX_AF_DISABLE_COLLISION))
			return true;
#else
		return fKinematic;
#endif
	}
	return false;
}

void plPXPhysicalController::GetKinematicPosition(hsPoint3& pos)
{
	pos.Set(-1,-1,-1);
	if ( fKinematicActor )
	{
		NxVec3 klPos = fKinematicActor->getGlobalPosition();
		pos.Set(hsScalar(klPos.x), hsScalar(klPos.y), hsScalar(klPos.z) - kPhysZOffset);
	}
}


void plPXPhysicalController::IApply(hsScalar delSecs)
{
	/*static const UInt32 collideFlags =
		1<<plSimDefs::kGroupStatic |
		1<<plSimDefs::kGroupAvatarBlocker |
		1<<plSimDefs::kGroupDynamic;*/
	UInt32 collideFlags =
		1<<plSimDefs::kGroupStatic |
		1<<plSimDefs::kGroupAvatarBlocker |
		1<<plSimDefs::kGroupDynamic;
	if(!fSeeking)
	{
		collideFlags|=(1<<plSimDefs::kGroupExcludeRegion);
	}
	int i;
	if (fKinematic)
	{
		// first apply sceneobject update to the kinematic
		plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
		if (so)
		{
			// If we've been moved since the last physics update (somebody warped us),
			// update the physics before we apply velocity.
			const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
			if (!CompareMatrices(l2w, fLastGlobalLoc, .0001f))
			{
				ISetKinematicLoc(l2w);
			}
		}
		// then jump out
		return;
	}

	if (!fEnable)
		return;

	bool gotGroundHit = fGroundHit;
	fGroundHit = false;

	fPushingPhysical = nil;
	fFacingPushingPhysical = false;

	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		// If we've been moved since the last physics update (somebody warped us),
		// update the physics before we apply velocity.
		const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
		if (!CompareMatrices(l2w, fLastGlobalLoc, .0001f))
			ISetGlobalLoc(l2w);

		// Convert our avatar relative velocity to subworld relative
		if (!fLinearVelocity.IsEmpty())
		{
			fLinearVelocity = l2w * fLinearVelocity;

			const plCoordinateInterface* subworldCI = GetSubworldCI();
			if (subworldCI)
				fLinearVelocity = subworldCI->GetWorldToLocal() * fLinearVelocity;
		}

		// Add in gravity if the avatar's z velocity isn't being set explicitly
		// (Add in a little fudge factor, since the animations usually add a
		// tiny bit of z.)
		if (hsABS(fLinearVelocity.fZ) < 0.001f)
		{
			static const float kGravity = -32.f;

			// Get our previous z velocity.  If we're on the ground, clamp it to zero at
			// the largest, so we won't launch into the air if we're running uphill.
			hsScalar prevZVel = fAchievedLinearVelocity.fZ;
			if (IsOnGround())
				prevZVel = hsMinimum(prevZVel, 0.f);

			hsScalar grav = kGravity * delSecs;

			// If our gravity contribution isn't high enough this frame, we won't
			// report a collision even when standing on solid ground.
			hsScalar maxGrav = -.001f / delSecs;
			if (grav > maxGrav)
				grav = maxGrav;

			fLinearVelocity.fZ = prevZVel + grav;

			// Technically this is nonsensical and wrong, capping our velocity to
			// an accelleration constant. But no one seems to really mind.
			if (fLinearVelocity.fZ < kGravity)
				fLinearVelocity.fZ = kGravity;
		}

		// If we're airborne and the velocity isn't set, use the velocity from
		// the last frame so we maintain momentum.
		if (!IsOnGround() && fLinearVelocity.fX == 0.f && fLinearVelocity.fY == 0.f)
		{
			fLinearVelocity.fX = fAchievedLinearVelocity.fX;
			fLinearVelocity.fY = fAchievedLinearVelocity.fY;
		}

		if (!IsOnGround() || IsOnFalseGround())
		{
			// We're not on solid ground, so we should be sliding against whatever
			// we're hitting (like a rock cliff). Each vector in fSlidingNormals is
			// the surface normal of a collision that's too steep to be ground, so
			// we project our current velocity onto that plane and slide along the
			// wall.
			//
			// Also, sometimes PhysX reports a bunch of collisions from the wall,
			// but nothing from underneath (when there should be). So if we're not
			// touching ground, we offset the avatar in the direction of the 
			// surface normal(s). This doesn't fix the issue 100%, but it's a hell
			// of a lot better than nothing, and suitable duct tape until a future
			// PhysX revision fixes the issue.
			//
			// Yes, there's room for optimization here if we care.
			hsVector3 offset(0.f, 0.f, 0.f);
			for (i = 0; i < fSlidingNormals.GetCount(); i++)
			{
				offset += fSlidingNormals[i];

				hsVector3 velNorm = fLinearVelocity;
				if (velNorm.MagnitudeSquared() > 0)
					velNorm.Normalize();

				if (velNorm * fSlidingNormals[i] < 0)
				{
					hsVector3 proj = (velNorm % fSlidingNormals[i]) % fSlidingNormals[i];
					if (velNorm * proj < 0)
						proj *= -1.f;

					fLinearVelocity = fLinearVelocity.Magnitude() * proj;
				}
			}
			if (offset.MagnitudeSquared() > 0)
			{
				// 5 ft/sec is roughly the speed we walk backwards.
				// The higher the value, the less likely you'll trip
				// the bug, and this seems reasonable.
				offset.Normalize();
				fLinearVelocity += offset * 5;
			}
		}

		// Scale the velocity to our actual step size (by default it's feet/sec)
		NxVec3 vel(fLinearVelocity.fX * delSecs, fLinearVelocity.fY * delSecs, fLinearVelocity.fZ * delSecs);
		NxU32 colFlags = 0;

		fGroundHit = false;
		fFalseGround = false;
		fSlidingNormals.Swap(fPrevSlidingNormals);
		fSlidingNormals.SetCount(0);

#ifndef PLASMA_EXTERNAL_RELEASE
		fDbgCollisionInfo.SetCount(0);
#endif // PLASMA_EXTERNAL_RELEASE

		fController->move(vel, collideFlags, 0.0001, colFlags);
		
		
		ICheckForFalseGround();
		/*If the Physx controller  thinks we have a collision from below, need to make sure we
		have at least have false ground, otherwise Autostepping can send us into the air, and we will some times 
		float/panic link. For some reason the NxControllerHitReport does not always send messages
		regarding Controller contact with ground plane, but will (almost) always return NXCC_COLLISION_DOWN
		with the move method.
		*/
		if(((colFlags&NXCC_COLLISION_DOWN )==NXCC_COLLISION_DOWN )&&(fGroundHit==false))
		{
			fFalseGround=true;
		}
		/*
			The top sphere half was hit, but the ControllerHit Report doesn't know
			In IUpdate fHitHead will be used to keep from gaining unrealistic velocity in the x&y Direction
		*/
		if(colFlags&NXCC_COLLISION_UP)
		{
			fHitHead=true;
		}

#ifndef PLASMA_EXTERNAL_RELEASE
		if (fDebugDisplay)
			IDrawDebugDisplay();
#endif // PLASMA_EXTERNAL_RELEASE
	}
}

void plPXPhysicalController::ISendUpdates(hsScalar delSecs)
{
	if (!fEnable || fKinematic)
	{
		// When we're in non-phys or a behavior we can't go through the rest of the function
		// so we need to get out early, but we need to update the current position if we're
		// in a subworld.

		plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
		const plCoordinateInterface* ci = GetSubworldCI();
		if (ci && so)
		{
			const hsMatrix44& soL2W = so->GetCoordinateInterface()->GetLocalToWorld();
			const hsMatrix44& ciL2W = ci->GetLocalToWorld();

			hsMatrix44 l2w = fPrevSubworldW2L * soL2W;
			l2w = ciL2W * l2w;

			hsMatrix44 w2l;
			l2w.GetInverse(&w2l);

			((plCoordinateInterface*)so->GetCoordinateInterface())->SetTransform(l2w, w2l);
			((plCoordinateInterface*)so->GetCoordinateInterface())->FlushTransform();

			ISetGlobalLoc(l2w);
		}

		return;
	}
	
	// PhysX loves to cache stuff.  However, it doesn't like to update it (see
	// the RebuildCache crap above).  Say the avatar is disabled and sitting at
	// point 0,0,0.  We warp him to some other position and enable him.  If you
	// do the enable before the sim step is done, regardless of whether you move
	// him first, he will send out a penetration with any detector at 0,0,0.  As
	// far as I can tell there's no way around this, and I tried a lot of things.
	// The only solution I found is to move the avatar, run the sim step, then
	// enable him.  This means he won't trigger any detectors at his new position
	// until the next frame, but hopefully that won't be too noticeable.
	if (fEnableChanged)
	{
		fEnableChanged = false;
		fController->setCollision(fEnable);
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
		// PHYSX FIXME - after re-enabling check to see if we are inside any convex hull detector regions
		hsPoint3 pos;
		IGetPositionSim(pos);
		plSimulationMgr::GetInstance()->UpdateDetectorsInScene(fWorldKey,GetOwner(),pos,fEnable);
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
		//IInformDetectors(true);
	}
	if (fKinematicChanged)
	{
		fKinematicChanged = false;
		fController->setCollision(true);
#ifdef PHYSX_KINEMATIC_IS_DISABLED
		fKinematicActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
#endif  // PHYSX_KINEMATIC_IS_DISABLED
	}
	if (fKinematicEnableNextUpdate)
	{
		fKinematicActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
		fKinematicEnableNextUpdate = false;
	}

	if (!fGroundHit && !fFalseGround)
		fTimeInAir += delSecs;
	else
		fTimeInAir = 0.f;

	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		// Get the current position of the physical
		hsPoint3 curLocalPos;
		IGetPositionSim(curLocalPos);
		IMoveKinematicToController(curLocalPos);

		{
			const hsMatrix44& w2l = so->GetCoordinateInterface()->GetLocalToWorld();
			fAchievedLinearVelocity = hsVector3(curLocalPos - fLocalPosition);
			fAchievedLinearVelocity /= delSecs;
		}
		/*if we hit our head the sweep api might try to 
		move us laterally to  go as high as we requested kind of like autostep, to top it off the
		way the NxCharacter and the sweep api work as a whole  NxControllerHitReport::OnShapeHit
		wont be called regarding the head blow. 
		if we are airborne: with out this we will gain large amounts of velocity in the x y plane
		and on account of fAchievedLinearVelocity being used in the next step we will fly sideways 
		*/
		bool headhit=fHitHead;
		fHitHead=false;
		if(headhit&&!(fGroundHit||fFalseGround))
		{
			//we have hit our head and we don't have anything beneath our feet			
			//not really friction just a way to make it seem more realistic keep between 0 and 1
			hsScalar headFriction=0.0;
			fAchievedLinearVelocity.fX=(1.0-headFriction)*fLinearVelocity.fX;
			fAchievedLinearVelocity.fY=(1.0-headFriction)*fLinearVelocity.fY;
			//only clamping when hitting head and going upwards, if going down leave it be
			// this should only occur when going down stairwells with low ceilings like in cleft
			//kitchen area
			if(fAchievedLinearVelocity.fZ>0.0)
			{
				fAchievedLinearVelocity.fZ=0.0;
			}
			
		}
		
		// Apply angular velocity
		if (fAngularVelocity != 0.f)
		{
			hsScalar angle;
			hsVector3 axis;
			fLocalRotation.NormalizeIfNeeded();
			fLocalRotation.GetAngleAxis(&angle, &axis);

			// adjust it (quaternions are weird...)
			if (axis.fZ < 0)
				angle = (2*hsScalarPI) - angle; // axis is backwards, so reverse the angle too

			angle += fAngularVelocity * delSecs;

			// make sure we wrap around
			if (angle < 0)
				angle = (2*hsScalarPI) + angle; // angle is -, so this works like a subtract
			if (angle >= (2*hsScalarPI))
				angle = angle - (2*hsScalarPI);

			// and set the new angle
			fLocalRotation.SetAngleAxis(angle, hsVector3(0,0,1));
		}

		// We can't only send updates when the physical position changes because the
		// world relative position may be changing all the time if we're in a subworld.
//		if (curLocalPos != fLocalPosition || fAngularVelocity != 0.f)
		{
			fLocalPosition = curLocalPos;

			// Apply rotation and translation
			fLocalRotation.MakeMatrix(&fLastGlobalLoc);
			fLastGlobalLoc.SetTranslate(&fLocalPosition);

			// Localize to global coords if in a subworld
			const plCoordinateInterface* ci = GetSubworldCI();
			if (ci)
			{
				const hsMatrix44& l2w = ci->GetLocalToWorld();
				fLastGlobalLoc = l2w * fLastGlobalLoc;
			}

			plCorrectionMsg* corrMsg = TRACKED_NEW plCorrectionMsg;
			corrMsg->fLocalToWorld = fLastGlobalLoc;
			corrMsg->fLocalToWorld.GetInverse(&corrMsg->fWorldToLocal);
			corrMsg->fDirtySynch = true;

			hsMatrix44 w2l;
			fLastGlobalLoc.GetInverse(&w2l);
			//if (fProxyGen)
			//	fProxyGen->SetTransform(fLastGlobalLoc, w2l);

			// Send the new position to the plArmatureMod and the scene object
			const plArmatureMod* armMod = plArmatureMod::ConvertNoRef(so->GetModifierByType(plArmatureMod::Index()));
			if (armMod)
				corrMsg->AddReceiver(armMod->GetKey());
			corrMsg->AddReceiver(fOwner);

			corrMsg->Send();
		}
	}

	fLinearVelocity.Set(0, 0, 0);
	fAngularVelocity = 0;
}

void plPXPhysicalController::ICheckForFalseGround()
{
	if (fGroundHit)
		return; // Already collided with "real" ground.

	// We need to check for the case where the avatar hasn't collided with "ground", but is colliding
	// with a few other objects so that he's not actually falling (wedged in between some slopes).
	// We do this by answering the following question (in 2d top-down space): "If you sort the contact 
	// normals by angle, is there a large enough gap between normals?"
	// 
	// If you think in terms of geometry, this means a collection of surfaces are all pushing on you.
	// If they're pushing from all sides, you have nowhere to go, and you won't fall. There needs to be
	// a gap, so that you're pushed out and have somewhere to fall. This is the same as finding a gap 
	// larger than 180 degrees between sorted normals.
	// 
	// The problem is that on top of that, the avatar needs enough force to shove him out that gap (he
	// has to overcome friction). I deal with that by making the threshold (360 - (180 - 60) = 240). I've
	// seen up to 220 reached in actual gameplay in a situation where we'd want this to take effect. 
	// This is the same running into 2 walls where the angle between them is 60.
	int i, j;
	const hsScalar threshold = hsScalarDegToRad(240);
	int numContacts = fSlidingNormals.GetCount() + fPrevSlidingNormals.GetCount();
	if (numContacts >= 2)
	{
		// For extra fun... PhysX will actually report some collisions every other frame, as though
		// we're bouncing back and forth between the two (or more) objects blocking us. So it's not
		// enough to look at this frame's collisions, we have to check previous frames too.
		hsTArray<hsScalar> fCollisionAngles;
		fCollisionAngles.SetCount(numContacts);
		int angleIdx = 0;
		for (i = 0; i < fSlidingNormals.GetCount(); i++, angleIdx++)
		{
			fCollisionAngles[angleIdx] = hsATan2(fSlidingNormals[i].fY, fSlidingNormals[i].fX);
		}
		for (i = 0; i < fPrevSlidingNormals.GetCount(); i++, angleIdx++)
		{
			fCollisionAngles[angleIdx] = hsATan2(fPrevSlidingNormals[i].fY, fPrevSlidingNormals[i].fX);
		}

		// numContacts is rarely larger than 6, so let's do a simple bubble sort.
		for (i = 0; i < numContacts; i++)
		{
			for (j = i + 1; j < numContacts; j++)
			{
				if (fCollisionAngles[i] > fCollisionAngles[j])
				{
					hsScalar tempAngle = fCollisionAngles[i];
					fCollisionAngles[i] = fCollisionAngles[j];
					fCollisionAngles[j] = tempAngle;
				}
			}
		}
		
		// sorted, now we check.
		for (i = 1; i < numContacts; i++)
		{
			if (fCollisionAngles[i] - fCollisionAngles[i - 1] >= threshold)
				break;
		}
		
		if (i == numContacts)
		{
			// We got to the end. Check the last with the first and make your decision.
			if (!(fCollisionAngles[0] - fCollisionAngles[numContacts - 1] >= (threshold - 2 * hsScalarPI)))
				fFalseGround = true;
		}
	}
}

void plPXPhysicalController::ICreateController()
{
	NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

	NxCapsuleControllerDesc desc;
	desc.position.x		= 0;
	desc.position.y		= 0;
	desc.position.z		= 0;
	desc.upDirection	= NX_Z;
	desc.slopeLimit		= kSLOPELIMIT;
	desc.skinWidth		= kPhysxSkinWidth;
	desc.stepOffset		= STEP_OFFSET;
	desc.callback		= &gMyReport;
	desc.userData		= this;
	desc.radius			= fRadius;
	desc.height			= fHeight;
	desc.interactionFlag = NXIF_INTERACTION_EXCLUDE;
	//desc.interactionFlag = NXIF_INTERACTION_INCLUDE;
	fController = (NxCapsuleController*)gControllerMgr.createController(scene, desc);

	// Change the avatars shape groups.  The avatar doesn't actually use these when
	// it's determining collision, but if you're standing still and an object runs
	// into you, it'll pass through without this.
	NxActor* actor = fController->getActor();
	NxShape* shape = actor->getShapes()[0];
	shape->setGroup(plSimDefs::kGroupAvatar);

	// need to create the non-bouncing object that can be used to trigger things while the avatar is doing behaviors.
	NxActorDesc actorDesc;
	NxCapsuleShapeDesc capDesc;
	capDesc.radius = fRadius;
	capDesc.height = fHeight;
	capDesc.group = plSimDefs::kGroupAvatar;
	actorDesc.shapes.pushBack(&capDesc);
	NxBodyDesc bodyDesc;
	bodyDesc.mass = 1.f;
	actorDesc.body = &bodyDesc;
	bodyDesc.flags |= NX_BF_KINEMATIC;
	actorDesc.name = "AvatarTriggerKinematicGuy";
	fSeeking=false;
	try
	{
		fKinematicActor = scene->createActor(actorDesc);
	} catch (...)
	{
		hsAssert(false, "Actor creation crashed");
	}
#ifdef PHYSX_KINEMATIC_IS_DISABLED
	// initially start as in-active
	fKinematicActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
#endif
	// set the matrix to be the same as the controller's actor... that should orient it to be the same
	fKinematicActor->setGlobalPose(actor->getGlobalPose());

	// the proxy for the debug display
	//hsAssert(!fProxyGen, "Already have proxy gen, double read?");

	hsColorRGBA physColor;
	hsScalar opac = 1.0f;

	// local avatar is light purple and transparent
	physColor.Set(.2f, .1f, .2f, 1.f);
	opac = 0.8f;

	// the avatar proxy doesn't seem to work... not sure why?
	fProxyGen = TRACKED_NEW plPhysicalProxy(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
	fProxyGen->Init(this);
}

void plPXPhysicalController::IDeleteController()
{
	if (fController)
	{
		gControllerMgr.releaseController(*fController);
		fController = nil;

		if (fKinematicActor)
		{
			NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
			scene->releaseActor(*fKinematicActor);
			fKinematicActor = nil;
		}
		plSimulationMgr::GetInstance()->ReleaseScene(fWorldKey);
	}
}

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

// Make a visible object that can be viewed by users for debugging purposes.
plDrawableSpans* plPXPhysicalController::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	plDrawableSpans* myDraw = addTo;

	hsBool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));
	float radius = fRadius;
	myDraw = plDrawableGenerator::GenerateSphericalDrawable(fLocalPosition, radius,
		mat, fLastGlobalLoc, blended,
		nil, &idx, myDraw);

/*
	plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
	if (so)
	{
		hsBool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

		myDraw = plDrawableGenerator::GenerateConicalDrawable(fRadius*10, fHeight*10,
			mat, so->GetLocalToWorld(), blended,
			nil, &idx, myDraw);
	}
*/
	return myDraw;
}

#ifndef PLASMA_EXTERNAL_RELEASE
#include "../plPipeline/plDebugText.h"

void plPXPhysicalController::IDrawDebugDisplay()
{
	plDebugText		&debugTxt = plDebugText::Instance();
	char			strBuf[ 2048 ];
	int				lineHeight = debugTxt.GetFontSize() + 4;
	UInt32			scrnWidth, scrnHeight;

	debugTxt.GetScreenSize( &scrnWidth, &scrnHeight );
	int	y = 10;
	int x = 10;

	sprintf(strBuf, "Controller Count: %d", gControllers.size());
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;

	debugTxt.DrawString(x, y, "Avatar Collisions:");
	y += lineHeight;

	int i;
	for (i = 0; i < fDbgCollisionInfo.GetCount(); i++)
	{
		hsVector3 normal = fDbgCollisionInfo[i].fNormal;
		char *overlapStr = fDbgCollisionInfo[i].fOverlap ? "yes" : "no";
		hsScalar angle = hsScalarRadToDeg(hsACosine(normal * hsVector3(0, 0, 1)));
		sprintf(strBuf, "    Obj: %s, Normal: (%.2f, %.2f, %.2f), Angle(%.1f), Overlap(%3s)",
				fDbgCollisionInfo[i].fSO->GetKeyName(),
				normal.fX, normal.fY, normal.fZ, angle, overlapStr);
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;
	}
}

#endif PLASMA_EXTERNAL_RELEASE
