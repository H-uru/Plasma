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
#include "plPXPhysicalControllerCore.h"
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
#include "../plModifier/plDetectorLog.h"
//#include "NxVecExtendedVec3.h"

#include "NxPhysics.h"
#include "ControllerManager.h" 
#include "NxCapsuleController.h"
#include "NxCapsuleShape.h"
#define kPhysxSkinWidth 0.1f
#define kPhysZOffset ((fRadius + (fHeight / 2)) + kPhysxSkinWidth)
//#define kSLOPELIMIT (cosf(NxMath::degToRad(55.f)))
//#define kPhysicalHeightFudge 0.4f   // this fudge was used for PhysX 2.4
#define kPhysicalHeightFudge 0.0f

//#define STEP_OFFSET	1.0f
#define STEP_OFFSET	0.5f
//#define STEP_OFFSET	0.15f


#ifndef PLASMA_EXTERNAL_RELEASE
hsBool plPXPhysicalControllerCore::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE
int plPXPhysicalControllerCore::fPXControllersMax = 0;

static ControllerManager gControllerMgr;
static std::vector<plPXPhysicalControllerCore*> gControllers;
static bool gRebuildCache=false;

#define AvatarMass 200.f

class PXControllerHitReportWalk : public NxUserControllerHitReport
{
public:
	virtual NxControllerAction onShapeHit(const NxControllerShapeHit& hit)
	{
		plPXPhysicalControllerCore* ac = plPXPhysicalControllerCore::FindController(hit.controller);
		NxActor& actor = hit.shape->getActor();
		plPXPhysical* phys = (plPXPhysical*)actor.userData;
		static hsScalar SlopeLimit = kSLOPELIMIT;
		hsVector3 normal = plPXConvert::Vector(hit.worldNormal);
		ac->fMovementInterface->IAddContactNormals(normal);
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
		hsVector3 dir = plPXConvert::Vector(hit.dir);
		float dirdotup=dir.fZ;
		hsPoint3 pos((hsScalar)hit.worldPos.x, (hsScalar)hit.worldPos.y, (hsScalar)hit.worldPos.z);
		NxExtendedVec3 controllerPos=hit.controller->getPosition();
		hsVector3 bottomOfTheCapsule((hsScalar)controllerPos.x,(hsScalar)controllerPos.y,(hsScalar)controllerPos.z);
		bottomOfTheCapsule.fZ=bottomOfTheCapsule.fZ-(ac->fHeight/2.0f + ac->fRadius);
		if (actor.isDynamic() )
		{
			if((hit.worldPos.z- bottomOfTheCapsule.fZ)<=ac->fRadius)//bottom hemisphere
			{
				//onTopOfSlopeLimit
				if (phys && phys->GetProperty(plSimulationInterface::kPhysAnim))
				{
					if(normal.fZ>=0)
					{//we consider this ground
						ac->fMovementInterface->AddOnTopOfObject(phys);
					}
				}
			}
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
				// We only allow horizontal pushes. Vertical pushes when we stand on
				// dynamic objects creates useless stress on the solver.
				
				hsVector3 vel=ac->GetLinearVelocity()- plPXConvert::Vector( actor.getLinearVelocity());
				if(dirdotup>=0)vel.fZ=0.001f;
				else vel.fZ=0.0f;
				static hsScalar kAvieMass = 140.f/32.f;			
				if (!vel.IsEmpty())
				{
					static hsScalar kForceScale = 140.0f;
					NxF32 coeff;
					NxExtendedVec3 norm2=hit.controller->getPosition();
					norm2.x=hit.worldPos.x-bottomOfTheCapsule.fX;
					norm2.y=hit.worldPos.y-bottomOfTheCapsule.fY;
					if((hit.worldPos.z- bottomOfTheCapsule.fZ)<ac->fRadius)//bottom hemisphere
					{
						norm2.normalize();
						norm2.z=0.01f;
					}
					else if((hit.worldPos.z- bottomOfTheCapsule.fZ)<(ac->fRadius+ac->fHeight))
					{
						norm2.z=0.0f;
						norm2.normalize();
					}
					else
					{//must be the top so the normal is displacement from the pos - center
						//of top hemisphere
						norm2.z=hit.worldPos.z - ((ac->fRadius+ac->fHeight + bottomOfTheCapsule.fZ));
						norm2.normalize();
					}
					
					
					float proj=(float)(norm2.x*dir.fX+dir.fY*norm2.y+dir.fZ*norm2.z);
					coeff =abs(proj*kForceScale*vel.Magnitude());
					vel.fZ=(hsScalar)norm2.z;
					vel.fY=(hsScalar)norm2.y;
					vel.fX=(hsScalar)norm2.x;
					phys->SetHitForce(vel*coeff, pos);
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
			ac->SetPushingPhysical( phys);
			ac->SetFacingPushingPhysical((inverseRotation.Rotate(&kAvatarForward).InnerProduct(normal) < 0 ? true : false));
		}
		return NX_ACTION_NONE;
	}
	virtual NxControllerAction onControllerHit(const NxControllersHit& hit)
	{
		return NX_ACTION_NONE;
	}

} gMyReport;


plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, hsScalar height, hsScalar width)
{
	// Test to see how many controller there already is
	if ( !plPXPhysicalControllerCore::fPXControllersMax || plPXPhysicalControllerCore::NumControllers() < plPXPhysicalControllerCore::fPXControllersMax )
	{
		hsScalar radius = width / 2.f;
		hsScalar realHeight = height - width + kPhysicalHeightFudge;
		return TRACKED_NEW plPXPhysicalControllerCore(ownerSO, realHeight,radius);
	}
	return nil;
}

//Static Helper Func
plPXPhysicalControllerCore* plPXPhysicalControllerCore::FindController(NxController* controller)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
		if (ac->fController == controller)
			return ac;
	}
	return nil;
}
void plPXPhysicalControllerCore::RebuildCache(){gRebuildCache=true;}

plPXPhysicalControllerCore* plPXPhysicalControllerCore::GetController(NxActor& actor, bool* isController)
{
	*isController = false;
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
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
void plPXPhysicalControllerCore::GetWorldSpaceCapsule(NxCapsule& cap) const
{
	if(this->fKinematicActor)
	{
		int numshapes=fKinematicActor->getNbShapes();
		if (numshapes==1) 
		{
			//there should only be one shape on a controller
			NxShape* const *shapes=fKinematicActor->getShapes();
			//and since it is a capsule controller it better be a capsule;
			NxCapsuleShape *capShape = shapes[0]->isCapsule();
			if(capShape) capShape->getWorldCapsule(cap);
		}
	}
}
bool plPXPhysicalControllerCore::AnyControllersInThisWorld(plKey world)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
		if (ac->GetSubworld() == world)
			return true;
	}
	return false;
}

int plPXPhysicalControllerCore::NumControllers()
{
	return gControllers.size();
}
int plPXPhysicalControllerCore::GetControllersInThisSubWorld(plKey world, int maxToReturn,plPXPhysicalControllerCore** bufferout)
{
	int i=0;
	for (int j=0;j<gControllers.size();j++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
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
int plPXPhysicalControllerCore::GetNumberOfControllersInThisSubWorld(plKey world)
{
	int i=0;
	for (int j=0;j<gControllers.size();j++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
		if (ac->GetSubworld()==world)i++;
	}
	return i;
}
//
plPXPhysicalControllerCore::plPXPhysicalControllerCore(plKey ownerSO, hsScalar height, hsScalar radius)
	: plPhysicalControllerCore(ownerSO,height,radius)
	, fController(nil)
	, fProxyGen(nil)
	, fKinematicActor(nil)
	,fPreferedRadius(radius)
	,fPreferedHeight(height)
	, fBehavingLikeAnimatedPhys(true)
{
	fLocalPosition.Set(0, 0, 0);
	fLocalRotation.Set(0, 0, 0, 1);
	gControllers.push_back(this);
	fLastGlobalLoc.Reset();
	ICreateController();
	Enable(false);
}

void plPXPhysicalControllerCore::ISetGlobalLoc(const hsMatrix44& l2w)
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
plPXPhysicalControllerCore::~plPXPhysicalControllerCore()
{
	IDeleteController();
	//need to make sure my queued messages are released
	for(int j=0;j<fQueuedCollideMsgs.GetCount();j++)
	{
		delete fQueuedCollideMsgs[j];
		fQueuedCollideMsgs[j]=nil;
	}
	fQueuedCollideMsgs.SetCount(0);
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
void plPXPhysicalControllerCore::IMatchKinematicToController()
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
void plPXPhysicalControllerCore::UpdateControllerAndPhysicalRep()
{
	if ( fKinematicActor)
	{
		if(this->fBehavingLikeAnimatedPhys)
		{//this means we are moving the controller and then synchnig the kin
			NxExtendedVec3 ControllerPos= fController->getPosition();
			NxVec3 NewKinPos((NxReal)ControllerPos.x, (NxReal)ControllerPos.y, (NxReal)ControllerPos.z);
			if (fEnabled || fKinematic)
			{
				if (plSimulationMgr::fExtraProfile)
					SimLog("Moving kinematic to %f,%f,%f",NewKinPos.x, NewKinPos.y, NewKinPos.z );
				// use the position
				fKinematicActor->moveGlobalPosition(NewKinPos);

			}
			else
			{
				if (plSimulationMgr::fExtraProfile)
					SimLog("Setting kinematic to %f,%f,%f", NewKinPos.x, NewKinPos.y, NewKinPos.z );
				fKinematicActor->setGlobalPosition(NewKinPos);
			}

		}
		else
		{
			NxVec3 KinPos= fKinematicActor->getGlobalPosition();
			NxExtendedVec3 NewControllerPos(KinPos.x, KinPos.y, KinPos.z);
			if (plSimulationMgr::fExtraProfile)
					SimLog("Setting Controller to %f,%f,%f", NewControllerPos.x, NewControllerPos.y, NewControllerPos.z );
			fController->setPosition(NewControllerPos);
		}
		hsPoint3 curLocalPos;	
		GetPositionSim(curLocalPos);
		fLocalPosition = curLocalPos;
	}
}
void plPXPhysicalControllerCore::MoveKinematicToController(hsPoint3& pos)
{
	if ( fKinematicActor)
	{
		NxVec3 kinPos = fKinematicActor->getGlobalPosition();
		if ( abs(kinPos.x-pos.fX) + abs(kinPos.y-pos.fY) + (abs(kinPos.z-pos.fZ+kPhysZOffset)) > 0.0001f)
		{
			NxVec3 newPos;
			newPos.x = (NxReal)pos.fX;
			newPos.y = (NxReal)pos.fY;
			newPos.z = (NxReal)pos.fZ+kPhysZOffset;
			if ((fEnabled || fKinematic) && fBehavingLikeAnimatedPhys)
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

void plPXPhysicalControllerCore::ISetKinematicLoc(const hsMatrix44& l2w)
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
	if (fEnabled|| fKinematic)
		fKinematicActor->moveGlobalPosition(plPXConvert::Point(kPos));
	else
		fKinematicActor->setGlobalPosition(plPXConvert::Point(kPos));
}
void plPXPhysicalControllerCore::IGetPositionSim(hsPoint3& pos) const
{
	
	if(this->fBehavingLikeAnimatedPhys)
	{
		const NxExtendedVec3& nxPos = fController->getPosition();
		pos.Set(hsScalar(nxPos.x), hsScalar(nxPos.y), hsScalar(nxPos.z) - kPhysZOffset);
	}
	else
	{
		NxVec3 Pos = fKinematicActor->getGlobalPosition();
		pos.Set(hsScalar(Pos.x), hsScalar(Pos.y), hsScalar(Pos.z) - kPhysZOffset);
	}
}
void plPXPhysicalControllerCore::ICreateController()
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
	capDesc.materialIndex= plSimulationMgr::GetInstance()->GetMaterialIdx(scene, 0.0,0.0);
	actorDesc.shapes.pushBack(&capDesc);
	NxBodyDesc bodyDesc;
	bodyDesc.mass = AvatarMass;//1.f;
	actorDesc.body = &bodyDesc;
	bodyDesc.flags = NX_BF_KINEMATIC;
	bodyDesc.flags |=NX_BF_DISABLE_GRAVITY ;
	
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

	/*
	// the avatar proxy doesn't seem to work... not sure why?
	fProxyGen = TRACKED_NEW plPhysicalProxy(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
	fProxyGen->Init(this);
	*/
}
void plPXPhysicalControllerCore::ICreateController(const hsPoint3& pos)
{
	NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
	NxCapsuleControllerDesc desc;
	desc.position.x		= pos.fX;
	desc.position.y		= pos.fY;
	desc.position.z		= pos.fZ;
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
	capDesc.materialIndex= plSimulationMgr::GetInstance()->GetMaterialIdx(scene, 0.0,0.0);
	actorDesc.globalPose=actor->getGlobalPose();
	NxBodyDesc bodyDesc;
	bodyDesc.mass = AvatarMass;
	actorDesc.body = &bodyDesc;
	bodyDesc.flags = NX_BF_KINEMATIC;
	bodyDesc.flags |=NX_BF_DISABLE_GRAVITY ;
	actorDesc.name = "AvatarTriggerKinematicGuy";
	fSeeking=false;
	try
	{
		fKinematicActor = scene->createActor(actorDesc);
	}
	catch (...)
	{
		hsAssert(false, "Actor creation crashed");
	}

	// set the matrix to be the same as the controller's actor... that should orient it to be the same
	//fKinematicActor->setGlobalPose(actor->getGlobalPose());

	// the proxy for the debug display
	//hsAssert(!fProxyGen, "Already have proxy gen, double read?");

	hsColorRGBA physColor;
	hsScalar opac = 1.0f;

	// local avatar is light purple and transparent
	physColor.Set(.2f, .1f, .2f, 1.f);
	opac = 0.8f;

	/*
	// the avatar proxy doesn't seem to work... not sure why?
	fProxyGen = TRACKED_NEW plPhysicalProxy(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
	fProxyGen->Init(this);
	*/

}
void plPXPhysicalControllerCore::IDeleteController()
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

void plPXPhysicalControllerCore::IInformDetectors(bool entering,bool deferUntilNextSim=true)
{
	static const NxU32 DetectorFlag= 1<<plSimDefs::kGroupDetector;
	if (fController)
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		DetectorLog("Informing from plPXPhysicalControllerCore::IInformDetectors");
#endif 	
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
					bool doReport = physical->DoReportOn(plSimDefs::kGroupAvatar);
					if(doReport)
					{
						plCollideMsg* msg = TRACKED_NEW plCollideMsg;
						msg->fOtherKey = fOwner;
						msg->fEntering = entering;
						msg->AddReceiver(physical->GetObjectKey());
						if(!deferUntilNextSim)
						{
#ifndef PLASMA_EXTERNAL_RELEASE
						DetectorLog("Sending an %s msg to %s" , entering? "entering":"exit", physical->GetObjectKey()->GetName());
#endif						
						msg->Send();
						}
						else
						{
#ifndef PLASMA_EXTERNAL_RELEASE
						DetectorLog("Queuing an %s msg to %s, which will be sent after the next simstep" , entering? "entering":"exit", physical->GetObjectKey()->GetName());
#endif						
						//these will be fired in update prestep on the next lap
						fQueuedCollideMsgs.Append(msg);
						}
					}
				}
			}
		}
#ifndef PLASMA_EXTERNAL_RELEASE
		DetectorLog("Done informing from plPXPhysicalControllerCore::IInformDetectors");
#endif
	}
}
void plPXPhysicalControllerCore::Move(hsVector3 displacement, unsigned int collideWith, unsigned int &collisionResults)
{
	collisionResults=0;
	if(fController)
	{
		NxVec3 dis(displacement.fX,displacement.fY,displacement.fZ);
		NxU32 colFlags = 0;
		this->fController->move(dis,collideWith,.00001,colFlags);
		if(colFlags&NXCC_COLLISION_DOWN)collisionResults|=kBottom;
		if(colFlags&NXCC_COLLISION_UP)collisionResults|=kTop;
		if(colFlags&&NXCC_COLLISION_SIDES)collisionResults|=kSides;
	}
	return;	
}
void plPXPhysicalControllerCore::Enable(bool enable)
{
	if (fEnabled != enable)
	{
		fEnabled = enable;
		if (fEnabled)
			fEnableChanged = true;
		else
		{
			// See ISendUpdates for why we don't re-enable right away
			fController->setCollision(fEnabled);
		}
	}
}

void plPXPhysicalControllerCore::SetSubworld(plKey world) 
{	
	if (fWorldKey != world)
	{
		bool wasEnabled = fEnabled;
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
		// PHYSX FIXME - before leaving this world, sending leaving detector events if we are inside a convex hull detector
		hsPoint3 pos;
		IGetPositionSim(pos);
		plSimulationMgr::GetInstance()->UpdateDetectorsInScene(fWorldKey,GetOwner(),pos,false);
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
		//need to inform detectors in the old world that we are leaving
		IInformDetectors(false);
		//done informing old world
		SimLog("Changing subworlds!");
 		IDeleteController();
		SimLog("Deleted old controller");
 		fWorldKey = world;
		if (GetSubworldCI())
			fPrevSubworldW2L = GetSubworldCI()->GetWorldToLocal();
		// Update our subworld position and rotation
		const plCoordinateInterface* subworldCI = GetSubworldCI();
		if (subworldCI)
		{
			const hsMatrix44& w2s = fPrevSubworldW2L;
			hsMatrix44 l2s = w2s * fLastGlobalLoc;
			l2s.GetTranslate(&fLocalPosition);
			fLocalRotation.SetFromMatrix44(l2s);
		}
		else
		{
			fLastGlobalLoc.GetTranslate(&fLocalPosition);
			fLocalRotation.SetFromMatrix44(fLastGlobalLoc);
		}
		hsMatrix44 w2l;
		fLastGlobalLoc.GetInverse(&w2l);
		if (fProxyGen)
			fProxyGen->SetTransform(fLastGlobalLoc, w2l);
		// Update the physical position
		SimLog("creating new controller");
		hsPoint3 PositionPlusOffset=fLocalPosition;
		PositionPlusOffset.fZ +=kPhysZOffset;
		//placing new controller and kinematic in the appropriate location
		ICreateController(PositionPlusOffset);
		RebuildCache();
	}
}
const plCoordinateInterface* plPXPhysicalControllerCore::GetSubworldCI() const 
{
	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		if (so)
			return so->GetCoordinateInterface();
	}
	return nil;
}
// For the avatar SDL only
void plPXPhysicalControllerCore::GetState(hsPoint3& pos, float& zRot)
{	
	// Temporarily use the position point while we get the z rotation
	fLocalRotation.NormalizeIfNeeded();
	fLocalRotation.GetAngleAxis(&zRot, (hsVector3*)&pos);

	if (pos.fZ < 0)
		zRot = (2 * hsScalarPI) - zRot; // axis is backwards, so reverse the angle too

	pos = fLocalPosition;

}

void plPXPhysicalControllerCore::SetState(const hsPoint3& pos, float zRot)
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
// kinematic stuff .... should be just for when playing a behavior...
void plPXPhysicalControllerCore::Kinematic(bool state)
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
bool plPXPhysicalControllerCore::IsKinematic()
{
	return fKinematic;
}
void plPXPhysicalControllerCore::GetKinematicPosition(hsPoint3& pos)
{
	pos.Set(-1,-1,-1);
	if ( fKinematicActor )
	{
		NxVec3 klPos = fKinematicActor->getGlobalPosition();
		pos.Set(hsScalar(klPos.x), hsScalar(klPos.y), hsScalar(klPos.z) - kPhysZOffset);
	}
}
void plPXPhysicalControllerCore::UpdatePoststep( hsScalar delSecs)
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
		plPXPhysicalControllerCore* ac = gControllers[i];

		hsAssert(ac, "Bad avatar controller");
	
			gControllerMgr.updateControllers();
			if(ac->fMovementInterface)
				ac->Update(delSecs);
				
			if (ac->GetSubworldCI())
				ac->fPrevSubworldW2L = ac->GetSubworldCI()->GetWorldToLocal();
			else
			{
				if (!ac->fPrevSubworldW2L.IsIdentity())
					ac->fPrevSubworldW2L.Reset();
			}
	}
}
void plPXPhysicalControllerCore::UpdatePrestep(hsScalar delSecs)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];

		hsAssert(ac, "Bad avatar controller");
		//FIXME
#ifndef PLASMA_EXTERNAL_RELEASE
		ac->fDbgCollisionInfo.SetCount(0);
#endif // PLASMA_EXTERNAL_RELEASE

		if (gRebuildCache&&ac->fController)	
			ac->fController->reportSceneChanged();
		//hsAssert(ac->fMovementInterface,"Updating a controller with out a movement strategy");
		if(ac)
		{	
			if(ac->fNeedsResize)ac->IHandleResize();
			int storedCollideMsgs=ac->fQueuedCollideMsgs.GetCount();
			if(storedCollideMsgs)
			{
				plSimulationMgr* simMgr=plSimulationMgr::GetInstance();
				for(int j=0; j<storedCollideMsgs;j++)
				{
					simMgr->AddCollisionMsg(ac->fQueuedCollideMsgs[j]);
				}
				ac->fQueuedCollideMsgs.SetCount(0);
			}
			ac->Apply(delSecs);
		}
#ifndef PLASMA_EXTERNAL_RELEASE
		if (fDebugDisplay)
			ac->IDrawDebugDisplay();
#endif // PLASMA_EXTERNAL_RELEASE
	}
	gRebuildCache = false;
}
void plPXPhysicalControllerCore::UpdatePostSimStep(hsScalar delSecs)
{
	for (int i = 0; i < gControllers.size(); i++)
	{
		plPXPhysicalControllerCore* ac = gControllers[i];
		hsAssert(ac, "Bad avatar controller");
		ac->PostStep(delSecs);
		
	}
}
void plPXPhysicalControllerCore::HandleEnableChanged()
{
		fEnableChanged = false;
		if(this->fBehavingLikeAnimatedPhys)
		{
			fController->setCollision(fEnabled);
		}
		else
		{
			fController->setCollision(false);
		}
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
		// PHYSX FIXME - after re-enabling check to see if we are inside any convex hull detector regions
		hsPoint3 pos;
		IGetPositionSim(pos);
		plSimulationMgr::GetInstance()->UpdateDetectorsInScene(fWorldKey,GetOwner(),pos,fEnable);
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
		//IInformDetectors(true);
}

void plPXPhysicalControllerCore::HandleKinematicChanged()
{
		fKinematicChanged = false;
		if(this->fBehavingLikeAnimatedPhys)
		{
			fController->setCollision(true);
		}
		else
		{
			fController->setCollision(false);
		}
#ifdef PHYSX_KINEMATIC_IS_DISABLED
		fKinematicActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
#endif  // PHYSX_KINEMATIC_IS_DISABLED
}
void plPXPhysicalControllerCore::HandleKinematicEnableNextUpdate()
{
	fKinematicActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
		fKinematicEnableNextUpdate = false;
}
void plPXPhysicalControllerCore::IHandleResize()
{

	UInt32 collideFlags =
		1<<plSimDefs::kGroupStatic |
		1<<plSimDefs::kGroupAvatarBlocker |
		1<<plSimDefs::kGroupDynamic;
	if(!IsSeeking())
	{
		collideFlags|=(1<<plSimDefs::kGroupExcludeRegion);
	}
	NxScene* myscene = plSimulationMgr::GetInstance()->GetScene(this->fWorldKey);
//	NxShape** response=TRACKED_NEW NxShape*[2];
	
	NxVec3 center(fLocalPosition.fX,fLocalPosition.fY,fLocalPosition.fZ+fPreferedRadius);
	NxSegment Seg(center,center);
	const NxCapsule newCap(Seg,fPreferedRadius);
	int numintersect =myscene->checkOverlapCapsule(newCap,NX_ALL_SHAPES,collideFlags);
	//with new capsule dimensions check for overlap
	//with objects we would collide with
	
	if(numintersect==0)
	{
		fHeight=fPreferedHeight;
		fRadius=fPreferedRadius;
		fController->setRadius(fRadius);
		fController->setHeight(fHeight);
		
		fNeedsResize=false;
	}

//	delete[] response;
}
void plPXPhysicalControllerCore::SetControllerDimensions(hsScalar radius, hsScalar height)
{
	fNeedsResize=false;
	if(fRadius!=radius)
	{
		fNeedsResize=true;
	}
	if(fHeight!=height)
	{
		fNeedsResize=true;
	}
	fPreferedRadius=radius;
	fPreferedHeight=height;
}

void plPXPhysicalControllerCore::LeaveAge()
{
	SetPushingPhysical(nil);
	if(fWorldKey) this->SetSubworld(nil);
	this->fMovementInterface->LeaveAge();
}
int plPXPhysicalControllerCore::SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, hsBool vsDynamics, hsBool vsStatics, 
							UInt32& vsSimGroups, std::multiset< plControllerSweepRecord >& WhatWasHitOut)
{
	NxCapsule tempCap;
	tempCap.p0 =plPXConvert::Point( startPos);
	tempCap.p0.z = tempCap.p0.z + fPreferedRadius;
	tempCap.radius = fPreferedRadius ;
	tempCap.p1 = tempCap.p0;
	tempCap.p1.z = tempCap.p1.z + fPreferedHeight;

	NxVec3 vec;
	vec.x = endPos.fX - startPos.fX;
	vec.y = endPos.fY - startPos.fY;
	vec.z = endPos.fZ - startPos.fZ;

	int numberofHits = 0;
	int HitsReturned = 0;
	WhatWasHitOut.clear();
	NxScene *myscene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
	NxSweepQueryHit whatdidIhit[10];
	unsigned int flags = NX_SF_ALL_HITS;
	if(vsDynamics)
		flags |= NX_SF_DYNAMICS;
	if(vsStatics)
		flags |= NX_SF_STATICS;
	numberofHits = myscene->linearCapsuleSweep(tempCap, vec, flags, nil, 10, whatdidIhit, nil, vsSimGroups);
	if(numberofHits)
	{//we hit a dynamic object lets make sure it is not animatable
		for(int i=0; i<numberofHits; i++)
		{
			plControllerSweepRecord CurrentHit;
			CurrentHit.ObjHit=(plPhysical*)whatdidIhit[i].hitShape->getActor().userData;
			CurrentHit.Norm.fX = whatdidIhit[i].normal.x;
			CurrentHit.Norm.fY = whatdidIhit[i].normal.y;
			CurrentHit.Norm.fZ = whatdidIhit[i].normal.z;
			if(CurrentHit.ObjHit != nil)
			{
				hsPoint3 where;
				where.fX = whatdidIhit[i].point.x;
				where.fY = whatdidIhit[i].point.y;
				where.fZ = whatdidIhit[i].point.z;
				CurrentHit.locHit = where;
				CurrentHit.TimeHit = whatdidIhit[i].t ;
				WhatWasHitOut.insert(CurrentHit);
				HitsReturned++;
			}
		}
	}

	return HitsReturned;
}
void plPXPhysicalControllerCore::BehaveLikeAnimatedPhysical(hsBool actLikeAnAnimatedPhys)
{
	hsAssert(fKinematicActor, "Changing behavior, but plPXPhysicalControllerCore has no Kinematic actor associated with it");
	if(fBehavingLikeAnimatedPhys!=actLikeAnAnimatedPhys)
	{
		fBehavingLikeAnimatedPhys=actLikeAnAnimatedPhys;
		if(fKinematicActor)
		{
			if(actLikeAnAnimatedPhys)
			{
				//need to set BX Kinematic if true and kill any rotation
				fController->setCollision(fEnabled);
				fKinematicActor->raiseBodyFlag(NX_BF_KINEMATIC);
				fKinematicActor->clearBodyFlag(NX_BF_FROZEN_ROT);
				fKinematicActor->raiseBodyFlag(NX_BF_DISABLE_GRAVITY);
			}
			else
			{
				//don't really use the controller now don't bother with collisions 
				fController->setCollision(false);
				fKinematicActor->clearBodyFlag(NX_BF_KINEMATIC);
				fKinematicActor->raiseBodyFlag(NX_BF_FROZEN_ROT_X);
				fKinematicActor->raiseBodyFlag(NX_BF_FROZEN_ROT_Y);
				fKinematicActor->clearBodyFlag(NX_BF_DISABLE_GRAVITY);
				

			}
		}
	}
}

hsBool plPXPhysicalControllerCore::BehavingLikeAnAnimatedPhysical()
{
	hsAssert(fKinematicActor, "plPXPhysicalControllerCore is missing a kinematic actor");
	return fBehavingLikeAnimatedPhys;
}

void plPXPhysicalControllerCore::SetLinearVelocity(const hsVector3& linearVel)
{
	plPhysicalControllerCore::SetLinearVelocity(linearVel);
	if(fKinematicActor && !fBehavingLikeAnimatedPhys)
	{
		NxVec3 vel= plPXConvert::Vector(linearVel);
		fKinematicActor->setLinearVelocity(vel);
	}
}
void plPXPhysicalControllerCore::SetAngularVelocity(const hsScalar angvel)
{
	plPhysicalControllerCore::SetAngularVelocity(angvel);
	if(fKinematicActor && !fBehavingLikeAnimatedPhys)
	{
		NxVec3 vel(0.0f, 0.0f, angvel);
		fKinematicActor->setAngularVelocity(vel);
	}
}
void plPXPhysicalControllerCore::SetVelocities(const hsVector3& linearVel, hsScalar angVel)
{
	SetLinearVelocity(linearVel);
	SetAngularVelocity(angVel);
}

void plPXPhysicalControllerCore::IMatchControllerToKinematic()
{
	NxExtendedVec3 newpos;
	NxVec3 pos=fKinematicActor->getGlobalPosition();
	newpos.x=pos.x;
	newpos.y=pos.y;
	newpos.z=pos.z;
	fController->setPosition(newpos);
}
const hsVector3& plPXPhysicalControllerCore::GetLinearVelocity()
{
	if(BehavingLikeAnAnimatedPhysical())
		return fLinearVelocity;
	else
	{
		fLinearVelocity = plPXConvert::Vector(fKinematicActor->getLinearVelocity());
		return fLinearVelocity;
	}
}

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

// Make a visible object that can be viewed by users for debugging purposes.
plDrawableSpans* plPXPhysicalControllerCore::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
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

void plPXPhysicalControllerCore::IDrawDebugDisplay()
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
