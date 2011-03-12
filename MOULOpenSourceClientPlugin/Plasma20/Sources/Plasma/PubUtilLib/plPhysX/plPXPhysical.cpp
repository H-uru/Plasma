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
#include "plPXPhysical.h"

#include "NxPhysics.h"

#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"
#include "plProfile.h"
#include "hsQuat.h"
#include "hsSTLStream.h"

#include "plSimulationMgr.h"
#include "../plPhysical/plPhysicalSDLModifier.h"
#include "../plPhysical/plPhysicalSndGroup.h"
#include "../plPhysical/plPhysicalProxy.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "../pnKeyedObject/plKey.h"
#include "../pnMessage/plCorrectionMsg.h"
#include "../pnMessage/plNodeRefMsg.h"
#include "../pnMessage/plSDLModifierMsg.h"
#include "../plMessage/plSimStateMsg.h"
#include "../plMessage/plSimInfluenceMsg.h"
#include "../plMessage/plLinearVelocityMsg.h"
#include "../plMessage/plAngularVelocityMsg.h"
#include "../plDrawable/plDrawableGenerator.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plNetTransport/plNetTransportMember.h"
#include "../plStatusLog/plStatusLog.h"
#include "plPXConvert.h"
#include "plPXPhysicalControllerCore.h"

#include "../plModifier/plDetectorLog.h"


#if 0
	#define SpamMsg(x) x
#else
	#define SpamMsg(x)
#endif
              
#define LogActivate(func) if (fActor->isSleeping()) SimLog("%s activated by %s", GetKeyName(), func);

PhysRecipe::PhysRecipe()
	: mass(0.f)
	, friction(0.f)
	, restitution(0.f)
	, bounds(plSimDefs::kBoundsMax)
	, group(plSimDefs::kGroupMax)
	, reportsOn(0)
	, objectKey(nil)
	, sceneNode(nil)
	, worldKey(nil)
	, convexMesh(nil)
	, triMesh(nil)
	, radius(0.f)
	, offset(0.f, 0.f, 0.f)
	, meshStream(nil)
{
	l2s.Reset();
}

plProfile_Extern(MaySendLocation);
plProfile_Extern(LocationsSent);
plProfile_Extern(PhysicsUpdates);

static void ClearMatrix(hsMatrix44 &m)
{
	m.fMap[0][0] = 0.0f; m.fMap[0][1] = 0.0f; m.fMap[0][2] = 0.0f; m.fMap[0][3]  = 0.0f;
	m.fMap[1][0] = 0.0f; m.fMap[1][1] = 0.0f; m.fMap[1][2] = 0.0f; m.fMap[1][3]  = 0.0f;
	m.fMap[2][0] = 0.0f; m.fMap[2][1] = 0.0f; m.fMap[2][2] = 0.0f; m.fMap[2][3]  = 0.0f;
	m.fMap[3][0] = 0.0f; m.fMap[3][1] = 0.0f; m.fMap[3][2] = 0.0f; m.fMap[3][3]  = 0.0f;
	m.NotIdentity();
}

int	plPXPhysical::fNumberAnimatedPhysicals = 0;
int	plPXPhysical::fNumberAnimatedActivators = 0;

/////////////////////////////////////////////////////////////////
//
// plPXPhysical IMPLEMENTATION
//
/////////////////////////////////////////////////////////////////

plPXPhysical::plPXPhysical()
	: fSDLMod(nil)
	, fActor(nil)
	, fBoundsType(plSimDefs::kBoundsMax)
	, fLOSDBs(plSimDefs::kLOSDBNone)
	, fGroup(plSimDefs::kGroupMax)
	, fReportsOn(0)
	, fLastSyncTime(0.0f)
	, fProxyGen(nil)
	, fSceneNode(nil)
	, fWorldKey(nil)
	, fSndGroup(nil)
	, fWorldHull(nil)
	, fSaveTriangles(nil)
	, fHullNumberPlanes(0)
	, fMass(0.f)
	, fWeWereHit(false)
	, fHitForce(0,0,0)
	, fHitPos(0,0,0)
	, fInsideConvexHull(false)
{
}

plPXPhysical::~plPXPhysical()
{
	SpamMsg(plSimulationMgr::Log("Destroying physical %s", GetKeyName()));

	if (fActor)
	{
		// Grab any mesh we may have (they need to be released manually)
		NxConvexMesh* convexMesh = nil;
		NxTriangleMesh* triMesh = nil;
		NxShape* shape = fActor->getShapes()[0];
		if (NxConvexShape* convexShape = shape->isConvexMesh())
			convexMesh = &convexShape->getConvexMesh();
		else if (NxTriangleMeshShape* trimeshShape = shape->isTriangleMesh())
			triMesh = &trimeshShape->getTriangleMesh();

		if (!fActor->isDynamic())
			plPXPhysicalControllerCore::RebuildCache();

		if (fActor->isDynamic() && fActor->readBodyFlag(NX_BF_KINEMATIC))
		{
			if (fGroup == plSimDefs::kGroupDynamic)
				fNumberAnimatedPhysicals--;
			else
				fNumberAnimatedActivators--;
		}

		// Release the actor
		NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
		scene->releaseActor(*fActor);
		fActor = nil;

		// Now that the actor is freed, release the mesh
		if (convexMesh)
			plSimulationMgr::GetInstance()->GetSDK()->releaseConvexMesh(*convexMesh);
		if (triMesh)
			plSimulationMgr::GetInstance()->GetSDK()->releaseTriangleMesh(*triMesh);

		// Release the scene, so it can be cleaned up if no one else is using it
		plSimulationMgr::GetInstance()->ReleaseScene(fWorldKey);
	}

	if (fWorldHull)
		delete [] fWorldHull;
	if (fSaveTriangles)
		delete [] fSaveTriangles;

	delete fProxyGen;

	// remove sdl modifier
	plSceneObject* sceneObj = plSceneObject::ConvertNoRef(fObjectKey->ObjectIsLoaded());
	if (sceneObj && fSDLMod)
	{
		sceneObj->RemoveModifier(fSDLMod);
	}
	delete fSDLMod;
}

static void MakeBoxFromHull(NxConvexMesh* convexMesh, NxBoxShapeDesc& box)
{
	NxConvexMeshDesc desc;
	convexMesh->saveToDesc(desc);

	hsScalar minX, minY, minZ, maxX, maxY, maxZ;
	minX = minY = minZ = FLT_MAX;
	maxX = maxY = maxZ = -FLT_MAX;

	for (int i = 0; i < desc.numVertices; i++)
	{
		float* point = (float*)(((char*)desc.points) + desc.pointStrideBytes*i);
		float x = point[0];
		float y = point[1];
		float z = point[2];

		minX = hsMinimum(minX, x);
		minY = hsMinimum(minY, y);
		minZ = hsMinimum(minZ, z);
		maxX = hsMaximum(maxX, x);
		maxY = hsMaximum(maxY, y);
		maxZ = hsMaximum(maxZ, z);
	}

	float xWidth = maxX - minX;
	float yWidth = maxY - minY;
	float zWidth = maxZ - minZ;
	box.dimensions.x = xWidth / 2;
	box.dimensions.y = yWidth / 2;
	box.dimensions.z = zWidth / 2;

	//hsMatrix44 mat;
	//box.localPose.getRowMajor44(&mat.fMap[0][0]);
	hsPoint3 trans(minX + (xWidth / 2), minY + (yWidth / 2), minZ + (zWidth / 2));
	//mat.SetTranslate(&trans);
	//box.localPose.setRowMajor44(&mat.fMap[0][0]);

	hsMatrix44 boxL2W;
	boxL2W.Reset();
	boxL2W.SetTranslate(&trans);
	plPXConvert::Matrix(boxL2W, box.localPose);

}

void plPXPhysical::IMakeHull(NxConvexMesh* convexMesh, hsMatrix44 l2w)
{
	NxConvexMeshDesc desc;
	convexMesh->saveToDesc(desc);

	// make sure there are some triangles to work with
    if (desc.numTriangles == 0)
		return;

	// get rid of any we may have already had
	if (fSaveTriangles)
		delete [] fSaveTriangles;

	fHullNumberPlanes = desc.numTriangles;
	fSaveTriangles = TRACKED_NEW hsPoint3[fHullNumberPlanes*3];

	for (int i = 0; i < desc.numTriangles; i++)
	{
		UInt32* triangle = (UInt32*)(((char*)desc.triangles) + desc.triangleStrideBytes*i);
		float* vertex1 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[0]);
		float* vertex2 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[1]);
		float* vertex3 = (float*)(((char*)desc.points) + desc.pointStrideBytes*triangle[2]);
		hsPoint3 pt1(vertex1[0],vertex1[1],vertex1[2]);
		hsPoint3 pt2(vertex2[0],vertex2[1],vertex2[2]);
		hsPoint3 pt3(vertex3[0],vertex3[1],vertex3[2]);

		fSaveTriangles[(i*3)+0] = pt1;
		fSaveTriangles[(i*3)+1] = pt2;
		fSaveTriangles[(i*3)+2] = pt3;
	}
}

void plPXPhysical::ISetHullToWorldWTriangles()
{
	// if we have a detector hull and the world hasn't been updated
	if (fWorldHull == nil)
	{
		fWorldHull = TRACKED_NEW hsPlane3[fHullNumberPlanes];
		// use the local2world from the physics engine so that it matches the transform of the positions from the triggerees
		hsMatrix44 l2w;
		plPXConvert::Matrix(fActor->getGlobalPose(), l2w);
		int i;
		for( i = 0; i < fHullNumberPlanes; i++ )
		{
			hsPoint3 pt1 = fSaveTriangles[i*3];
			hsPoint3 pt2 = fSaveTriangles[(i*3)+1];
			hsPoint3 pt3 = fSaveTriangles[(i*3)+2];

			// local to world translation
			pt1 = l2w * pt1;
			pt2 = l2w * pt2;
			pt3 = l2w * pt3;

			hsPlane3 plane(&pt1, &pt2, &pt3);
			fWorldHull[i] = plane;
		}
	}
}


hsBool plPXPhysical::IsObjectInsideHull(const hsPoint3& pos)
{
	if (fSaveTriangles)
	{
		ISetHullToWorldWTriangles();
		int i;
		for( i = 0; i < fHullNumberPlanes; i++ )
		{
			if (!ITestPlane(pos, fWorldHull[i]))
				return false;
		}
		return true;
	}
	return false;
}

hsBool plPXPhysical::Should_I_Trigger(hsBool enter, hsPoint3& pos)
{
	// see if we are inside the detector hull, if so, then don't trigger
	bool trigger = false;
	bool inside = IsObjectInsideHull(pos);
	if ( !inside)
	{
		trigger = true;
		fInsideConvexHull = enter;
	}
	else
	{
		// catch those rare cases on slow machines that miss the collision before avatar penetrated the face
		if (enter && !fInsideConvexHull)
		{
#ifdef PHYSX_SAVE_TRIGGERS_WORKAROUND
			trigger = true;
			fInsideConvexHull = enter;
			DetectorLogSpecial("**>Saved a missing enter collision: %s",GetObjectKey()->GetName());
#else
			DetectorLogSpecial("**>Could have saved a missing enter collision: %s",GetObjectKey()->GetName());
#endif PHYSX_SAVE_TRIGGERS_WORKAROUND
		}
	}

	return trigger;
}


hsBool plPXPhysical::Init(PhysRecipe& recipe)
{
	hsBool	startAsleep = false;
	fBoundsType = recipe.bounds;
	fGroup = recipe.group;
	fReportsOn = recipe.reportsOn;
	fObjectKey = recipe.objectKey;
	fSceneNode = recipe.sceneNode;
	fWorldKey = recipe.worldKey;

	NxActorDesc actorDesc;
	NxSphereShapeDesc sphereDesc;
	NxConvexShapeDesc convexShapeDesc;
	NxTriangleMeshShapeDesc trimeshShapeDesc;
	NxBoxShapeDesc boxDesc;

	plPXConvert::Matrix(recipe.l2s, actorDesc.globalPose);

	switch (fBoundsType)
	{
	case plSimDefs::kSphereBounds:
		{
			hsMatrix44 sphereL2W;
			sphereL2W.Reset();
			sphereL2W.SetTranslate(&recipe.offset);

 			sphereDesc.radius = recipe.radius;
			plPXConvert::Matrix(sphereL2W, sphereDesc.localPose);
 			sphereDesc.group = fGroup;
 			actorDesc.shapes.pushBack(&sphereDesc);
		}
		break;
	case plSimDefs::kHullBounds:
		// FIXME PHYSX - Remove when hull detection is fixed
		// If this is read time (ie, meshStream is nil), turn the convex hull
		// into a box.  That way the data won't have to change when convex hulls
		// actually work right.
		if (fGroup == plSimDefs::kGroupDetector && recipe.meshStream == nil)
		{
#ifdef USE_BOXES_FOR_DETECTOR_HULLS
			MakeBoxFromHull(recipe.convexMesh, boxDesc);
			plSimulationMgr::GetInstance()->GetSDK()->releaseConvexMesh(*recipe.convexMesh);
			boxDesc.group = fGroup;
			actorDesc.shapes.push_back(&boxDesc);
#else
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
			// make a hull of planes for testing IsInside
			IMakeHull(recipe.convexMesh,recipe.l2s);
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
			convexShapeDesc.meshData = recipe.convexMesh;
			convexShapeDesc.userData = recipe.meshStream;
			convexShapeDesc.group = fGroup;
			actorDesc.shapes.pushBack(&convexShapeDesc);
#endif // USE_BOXES_FOR_DETECTOR_HULLS
		}
		else
		{
			convexShapeDesc.meshData = recipe.convexMesh;
			convexShapeDesc.userData = recipe.meshStream;
			convexShapeDesc.group = fGroup;
			actorDesc.shapes.pushBack(&convexShapeDesc);
		}
		break;
	case plSimDefs::kBoxBounds:
		{
			boxDesc.dimensions = plPXConvert::Point(recipe.bDimensions);

			hsMatrix44 boxL2W;
			boxL2W.Reset();
			boxL2W.SetTranslate(&recipe.bOffset);
			plPXConvert::Matrix(boxL2W, boxDesc.localPose);

			boxDesc.group = fGroup;
			actorDesc.shapes.push_back(&boxDesc);
		}
		break;
	case plSimDefs::kExplicitBounds:
	case plSimDefs::kProxyBounds:
		if (fGroup == plSimDefs::kGroupDetector)
		{
			SimLog("Someone using an Exact on a detector region: %s", GetKeyName());
		}
		trimeshShapeDesc.meshData = recipe.triMesh;
		trimeshShapeDesc.userData = recipe.meshStream;
		trimeshShapeDesc.group = fGroup;
		actorDesc.shapes.pushBack(&trimeshShapeDesc);
		break;
	default:
		hsAssert(false, "Unknown geometry type during read.");
		return false;
		break;
	}

	//  Now fill out the body, or dynamic part of the physical
	NxBodyDesc bodyDesc;
	fMass = recipe.mass;
	if (recipe.mass != 0)
	{
		bodyDesc.mass = recipe.mass;
		actorDesc.body = &bodyDesc;

		if (GetProperty(plSimulationInterface::kPinned))
		{
			bodyDesc.flags |= NX_BF_FROZEN;
			startAsleep = true;				// put it to sleep if they are going to be frozen
		}

		if (fGroup != plSimDefs::kGroupDynamic || GetProperty(plSimulationInterface::kPhysAnim))
		{
			SetProperty(plSimulationInterface::kPassive, true);

			// Even though the code for animated physicals and animated activators are the same
			// keep these code snippets separated for fine tuning. Thanks.
			if (fGroup == plSimDefs::kGroupDynamic)
			{
				// handle the animated physicals.... make kinematic for now.
				fNumberAnimatedPhysicals++;
				bodyDesc.flags |= NX_BF_KINEMATIC;
				startAsleep = true;
			}
			else
			{
				// handle the animated activators.... 
				fNumberAnimatedActivators++;
				bodyDesc.flags |= NX_BF_KINEMATIC;
				startAsleep = true;
			}

		}
	}
	else
	{
		if ( GetProperty(plSimulationInterface::kPhysAnim) )
			SimLog("An animated physical that has no mass: %s", GetKeyName());
	}

	actorDesc.userData = this;
	actorDesc.name = GetKeyName();

	// Put the dynamics into actor group 1.  The actor groups are only used for
	// deciding who we get contact reports for.
	if (fGroup == plSimDefs::kGroupDynamic)
		actorDesc.group = 1;

	NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
	try
	{
		fActor = scene->createActor(actorDesc);
	} catch (...)
	{
		hsAssert(false, "Actor creation crashed");
		return false;
	}
	hsAssert(fActor, "Actor creation failed");
	if (!fActor)
		return false;

	NxShape* shape = fActor->getShapes()[0];
	shape->setMaterial(plSimulationMgr::GetInstance()->GetMaterialIdx(scene, recipe.friction, recipe.restitution));

	// Turn on the trigger flags for any detectors.
	//
	// Normally, we'd set these flags on the shape before it's created.  However,
	// in the case where the detector is going to be animated, it'll have a rigid
	// body too, and that will cause problems at creation.  According to Ageia,
	// a detector shape doesn't actually count as a shape, so the SDK will have
	// problems trying to calculate an intertial tensor.  By letting it be
	// created as a normal dynamic first, then setting the flags, we work around
	// that problem.
	if (fGroup == plSimDefs::kGroupDetector)
	{
		shape->setFlag(NX_TRIGGER_ON_ENTER, true);
		shape->setFlag(NX_TRIGGER_ON_LEAVE, true);
	}

	if (GetProperty(plSimulationInterface::kStartInactive) || startAsleep)
	{
		if (!fActor->isSleeping())
		{
			if (plSimulationMgr::fExtraProfile)
				SimLog("Deactivating %s in SetPositionAndRotationSim", GetKeyName());
			fActor->putToSleep();
		}
	}

	if (GetProperty(plSimulationInterface::kDisable))
		IEnable(false);
	if (GetProperty(plSimulationInterface::kSuppressed_DEAD))
		IEnable(false);

	plNodeRefMsg* refMsg = TRACKED_NEW plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kPhysical); 
	hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kActiveRef);

	if (fWorldKey)
	{
		plGenRefMsg* ref = TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefWorld);
		hsgResMgr::ResMgr()->AddViaNotify(fWorldKey, ref, plRefFlags::kActiveRef);
	}

  	// only dynamic physicals without noSync need SDLs
	if ( fGroup == plSimDefs::kGroupDynamic && !fProps.IsBitSet(plSimulationInterface::kNoSynchronize) )
	{
		// add SDL modifier
		plSceneObject* sceneObj = plSceneObject::ConvertNoRef(fObjectKey->ObjectIsLoaded());
		hsAssert(sceneObj, "nil sceneObject, failed to create and attach SDL modifier");

		delete fSDLMod;
		fSDLMod = TRACKED_NEW plPhysicalSDLModifier;
		sceneObj->AddModifier(fSDLMod);
	}

	return true;
}

/////////////////////////////////////////////////////////////////
//
// MESSAGE HANDLING
//
/////////////////////////////////////////////////////////////////

// MSGRECEIVE
hsBool plPXPhysical::MsgReceive( plMessage* msg )
{
	if(plGenRefMsg *refM = plGenRefMsg::ConvertNoRef(msg))
	{
		return HandleRefMsg(refM);
	}
	else if(plSimulationMsg *simM = plSimulationMsg::ConvertNoRef(msg))
	{
		plLinearVelocityMsg* velMsg = plLinearVelocityMsg::ConvertNoRef(msg);
		if(velMsg)
		{
			SetLinearVelocitySim(velMsg->Velocity());
			return true;
		}
		plAngularVelocityMsg* angvelMsg = plAngularVelocityMsg::ConvertNoRef(msg);
		if(angvelMsg)
		{
			SetAngularVelocitySim(angvelMsg->AngularVelocity());
			return true;
		}

		
		return false;
	}
	// couldn't find a local handler: pass to the base
	else
		return plPhysical::MsgReceive(msg);
}

// IHANDLEREFMSG
// there's two things we hold references to: subworlds
// and the simulation manager.
// right now, we're only worrying about the subworlds
hsBool plPXPhysical::HandleRefMsg(plGenRefMsg* refMsg)
{
	UInt8 refCtxt = refMsg->GetContext();
	plKey refKey = refMsg->GetRef()->GetKey();
	plKey ourKey = GetKey();
	PhysRefType refType = PhysRefType(refMsg->fType);

	const char* refKeyName = refKey ? refKey->GetName() : "MISSING";

	if (refType == kPhysRefWorld)
	{
		if (refCtxt == plRefMsg::kOnCreate || refCtxt == plRefMsg::kOnRequest)
		{
			// Cache the initial transform, since we assume the sceneobject already knows
			// that and doesn't need to be told again
			IGetTransformGlobal(fCachedLocal2World);
		}
		if (refCtxt == plRefMsg::kOnDestroy)
		{
			// our world was deleted out from under us: move to the main world
//			hsAssert(0, "Lost world");
		}
	}
	else if (refType == kPhysRefSndGroup)
	{
		switch (refCtxt)
		{
		case plRefMsg::kOnCreate:
		case plRefMsg::kOnRequest:
			fSndGroup = plPhysicalSndGroup::ConvertNoRef( refMsg->GetRef() );
			break;

		case plRefMsg::kOnDestroy:
			fSndGroup = nil;
			break;
		}
	}
	else
	{
		hsAssert(0, "Unknown ref type, who sent us this?");
	}

	return true;
}

void plPXPhysical::IEnable(hsBool enable)
{
 	fProps.SetBit(plSimulationInterface::kDisable, !enable);
	if (!enable)
	{
		fActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
		if (fActor->isDynamic())
			fActor->raiseBodyFlag(NX_BF_FROZEN);
		else
			plPXPhysicalControllerCore::RebuildCache();
	}
	else
	{
		fActor->clearActorFlag(NX_AF_DISABLE_COLLISION);

		// PHYSX FIXME - after re-enabling a possible detector, we need to check to see if any avatar is already in the PhysX turdy hull detector region
		plSimulationMgr::GetInstance()->UpdateAvatarInDetector(fWorldKey, this);

		if (fActor->isDynamic())
			fActor->clearBodyFlag(NX_BF_FROZEN);
		else
			plPXPhysicalControllerCore::RebuildCache();
	}
}

plPhysical& plPXPhysical::SetProperty(int prop, hsBool status)
{
	if (GetProperty(prop) == status)
	{
		const char* propName = "(unknown)";
		switch (prop)
		{
		case plSimulationInterface::kDisable:			propName = "kDisable";				break;
		case plSimulationInterface::kPinned:			propName = "kPinned";				break;
 		case plSimulationInterface::kPassive:			propName = "kPassive";				break;
		case plSimulationInterface::kPhysAnim:			propName = "kPhysAnim";				break;
		case plSimulationInterface::kStartInactive:		propName = "kStartInactive";		break;
		case plSimulationInterface::kNoSynchronize:		propName = "kNoSynchronize";		break;
		}

		const char* name = "(unknown)";
		if (GetKey())
			name = GetKeyName();
		if (plSimulationMgr::fExtraProfile)
			plSimulationMgr::Log("Warning: Redundant physical property set (property %s, value %s) on %s", propName, status ? "true" : "false", name);
	}

	switch (prop)
	{
	case plSimulationInterface::kDisable:
		IEnable(!status);
		break;

	case plSimulationInterface::kPinned:
		if (fActor->isDynamic())
		{
			// if the body is already unpinned and you unpin it again,
			// you'll wipe out its velocity. hence the check.
			hsBool current = fActor->readBodyFlag(NX_BF_FROZEN);
			if (status != current)
			{
				if (status)
                    fActor->raiseBodyFlag(NX_BF_FROZEN);
				else
				{
					fActor->clearBodyFlag(NX_BF_FROZEN);
					LogActivate("SetProperty");
					fActor->wakeUp();
				}
			}
		}
		break;
	}

	fProps.SetBit(prop, status);

	return *this;
}

plProfile_Extern(SetTransforms);

#define kMaxNegativeZPos -2000.f

bool CompareMatrices(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance)
{
	return 
		(fabs(matA.fMap[0][0] - matB.fMap[0][0]) < tolerance) &&
		(fabs(matA.fMap[0][1] - matB.fMap[0][1]) < tolerance) &&
		(fabs(matA.fMap[0][2] - matB.fMap[0][2]) < tolerance) &&
		(fabs(matA.fMap[0][3] - matB.fMap[0][3]) < tolerance) &&

		(fabs(matA.fMap[1][0] - matB.fMap[1][0]) < tolerance) &&
		(fabs(matA.fMap[1][1] - matB.fMap[1][1]) < tolerance) &&
		(fabs(matA.fMap[1][2] - matB.fMap[1][2]) < tolerance) &&
		(fabs(matA.fMap[1][3] - matB.fMap[1][3]) < tolerance) &&

		(fabs(matA.fMap[2][0] - matB.fMap[2][0]) < tolerance) &&
		(fabs(matA.fMap[2][1] - matB.fMap[2][1]) < tolerance) &&
		(fabs(matA.fMap[2][2] - matB.fMap[2][2]) < tolerance) &&
		(fabs(matA.fMap[2][3] - matB.fMap[2][3]) < tolerance) &&

		(fabs(matA.fMap[3][0] - matB.fMap[3][0]) < tolerance) &&
		(fabs(matA.fMap[3][1] - matB.fMap[3][1]) < tolerance) &&
		(fabs(matA.fMap[3][2] - matB.fMap[3][2]) < tolerance) &&
		(fabs(matA.fMap[3][3] - matB.fMap[3][3]) < tolerance);
}

// Called after the simulation has run....sends new positions to the various scene objects
// *** want to do this in response to an update message....
void plPXPhysical::SendNewLocation(hsBool synchTransform, hsBool isSynchUpdate)
{
	// we only send if:
	// - the body is active or forceUpdate is on
	// - the mass is non-zero
	// - the physical is not passive
	hsBool bodyActive = !fActor->isSleeping();
	hsBool dynamic = fActor->isDynamic();
	
	if ((bodyActive || isSynchUpdate) && dynamic)// && fInitialTransform)
	{
		plProfile_Inc(MaySendLocation);

		if (!GetProperty(plSimulationInterface::kPassive))
		{
			hsMatrix44 curl2w = fCachedLocal2World;
			// we're going to cache the transform before sending so we can recognize if it comes back
			IGetTransformGlobal(fCachedLocal2World);

			if (!CompareMatrices(curl2w, fCachedLocal2World, .0001f))
			{
				plProfile_Inc(LocationsSent);
				plProfile_BeginLap(PhysicsUpdates, GetKeyName());

				// quick peek at the translation...last time it was corrupted because we applied a non-unit quaternion
// 				hsAssert(real_finite(fCachedLocal2World.fMap[0][3]) &&
// 						 real_finite(fCachedLocal2World.fMap[1][3]) &&
// 						 real_finite(fCachedLocal2World.fMap[2][3]), "Bad transform outgoing");

				if (fCachedLocal2World.GetTranslate().fZ < kMaxNegativeZPos)
				{
					SimLog("Physical %s fell to %.1f (%.1f is the max).  Suppressing.", GetKeyName(), fCachedLocal2World.GetTranslate().fZ, kMaxNegativeZPos);
					// Since this has probably been falling for a while, and thus not getting any syncs,
					// make sure to save it's current pos so we'll know to reset it later
					DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
					IEnable(false);
				}

				hsMatrix44 w2l;
				fCachedLocal2World.GetInverse(&w2l);
				plCorrectionMsg *pCorrMsg = TRACKED_NEW plCorrectionMsg(GetObjectKey(), fCachedLocal2World, w2l, synchTransform);
				pCorrMsg->Send();
				if (fProxyGen)
					fProxyGen->SetTransform(fCachedLocal2World, w2l);
				plProfile_EndLap(PhysicsUpdates, GetKeyName());
			}
		}
	}
}

void plPXPhysical::ApplyHitForce()
{
	if (fActor && fWeWereHit)
	{
		fActor->addForceAtPos(plPXConvert::Vector(fHitForce), plPXConvert::Point(fHitPos), NX_FORCE);
		fWeWereHit = false;
	}
}


void plPXPhysical::ISetTransformGlobal(const hsMatrix44& l2w)
{
	hsAssert(fActor->isDynamic(), "Shouldn't move a static actor");

	NxMat34 mat;

	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		hsAssert(so, "Scene object not loaded while accessing subworld.");
		// physical to subworld (simulation space)
		hsMatrix44 p2s = so->GetCoordinateInterface()->GetWorldToLocal() * l2w;
		plPXConvert::Matrix(p2s, mat);
		if (fProxyGen)
		{
			hsMatrix44 w2l;
			p2s.GetInverse(&w2l);
			fProxyGen->SetTransform(p2s, w2l);
		}
	}
	// No need to localize
	else
	{
		plPXConvert::Matrix(l2w, mat);
		if (fProxyGen)
		{
			hsMatrix44 w2l;
			l2w.GetInverse(&w2l);
			fProxyGen->SetTransform(l2w, w2l);
		}
	}

	if (GetProperty(plSimulationInterface::kPhysAnim))
	{	hsAssert(fActor->readBodyFlag(NX_BF_KINEMATIC),"This Should be kinematic");
		fActor->moveGlobalPose(mat);
	}
	else
		fActor->setGlobalPose(mat);
}

// the physical may have several parents between it and the subworld object,
// but the *havok* transform is only one level away from the subworld.
// to avoid any confusion about this difference, we avoid referring to the 
// subworld as "parent" and use, for example, "l2s" (local-to-sub) instead
// of the canonical plasma "l2p" (local-to-parent)
void plPXPhysical::IGetTransformGlobal(hsMatrix44& l2w) const
{
	plPXConvert::Matrix(fActor->getGlobalPose(), l2w);

	if (fWorldKey)
	{
		plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
		hsAssert(so, "Scene object not loaded while accessing subworld.");
		// We'll hit this at export time, when the ci isn't ready yet, so do a check
		if (so->GetCoordinateInterface())
		{
			const hsMatrix44& s2w = so->GetCoordinateInterface()->GetLocalToWorld();
			l2w = s2w * l2w;
		}
	}
}

void plPXPhysical::IGetPositionSim(hsPoint3& pos) const
{
	pos = plPXConvert::Point(fActor->getGlobalPosition());
}

void plPXPhysical::IGetRotationSim(hsQuat& rot) const
{
	rot = plPXConvert::Quat(fActor->getGlobalOrientationQuat());
}
void plPXPhysical::ISetPositionSim(const hsPoint3& pos)
{
	if (GetProperty(plSimulationInterface::kPhysAnim))
		fActor->moveGlobalPosition(plPXConvert::Point(pos));
	else
		fActor->setGlobalPosition(plPXConvert::Point(pos));
}

void plPXPhysical::ISetRotationSim(const hsQuat& rot)
{
	if (GetProperty(plSimulationInterface::kPhysAnim))
		fActor->moveGlobalOrientation(plPXConvert::Quat(rot));
	else
		fActor->setGlobalOrientation(plPXConvert::Quat(rot));
}

// This form is assumed by convention to be global.
void plPXPhysical::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, hsBool force)
{
//	hsAssert(real_finite(l2w.fMap[0][3]) && real_finite(l2w.fMap[1][3]) && real_finite(l2w.fMap[2][3]), "Bad transform incoming");


	// make sure the physical is dynamic.
	//  also make sure there is some difference between the matrices...
	// ... but not when a subworld... because the subworld maybe animating and if the object is still then it is actually moving within the subworld
	if (force || (fActor->isDynamic() && (fWorldKey || !CompareMatrices(l2w, fCachedLocal2World, .0001f))) )
	{
		ISetTransformGlobal(l2w);
		plProfile_Inc(SetTransforms);
	}
	else
	{
		if ( !fActor->isDynamic()  && plSimulationMgr::fExtraProfile)
			SimLog("Setting transform on non-dynamic: %s.", GetKeyName());
	}
}

// GETTRANSFORM
void plPXPhysical::GetTransform(hsMatrix44& l2w, hsMatrix44& w2l)
{
	IGetTransformGlobal(l2w);
	l2w.GetInverse(&w2l);
}

hsBool plPXPhysical::GetLinearVelocitySim(hsVector3& vel) const
{
	hsBool result = false;

	if (fActor->isDynamic())
	{
		vel = plPXConvert::Vector(fActor->getLinearVelocity());
		result = true;
	}
	else
		vel.Set(0, 0, 0);

	return result;
}

void plPXPhysical::SetLinearVelocitySim(const hsVector3& vel)
{
	if (fActor->isDynamic())
		fActor->setLinearVelocity(plPXConvert::Vector(vel));
}

void plPXPhysical::ClearLinearVelocity()
{
	SetLinearVelocitySim(hsVector3(0, 0, 0));
}

hsBool plPXPhysical::GetAngularVelocitySim(hsVector3& vel) const
{
	hsBool result = false;
	if (fActor->isDynamic())
	{
		vel = plPXConvert::Vector(fActor->getAngularVelocity());
		result = true;
	}
	else
		vel.Set(0, 0, 0);

	return result;
}

void plPXPhysical::SetAngularVelocitySim(const hsVector3& vel)
{
	if (fActor->isDynamic())
		fActor->setAngularVelocity(plPXConvert::Vector(vel));
}

///////////////////////////////////////////////////////////////
//
// NETWORK SYNCHRONIZATION
//
///////////////////////////////////////////////////////////////

plKey plPXPhysical::GetSceneNode() const
{
	return fSceneNode;
}

void plPXPhysical::SetSceneNode(plKey newNode)
{
#ifdef HS_DEBUGGING
	plKey oldNode = GetSceneNode();
	char msg[1024];
	if (newNode)
		sprintf(msg,"Physical object %s cannot change scenes. Already in %s, trying to switch to %s.",fObjectKey->GetName(),oldNode->GetName(),newNode->GetName());
	else
		sprintf(msg,"Physical object %s cannot change scenes. Already in %s, trying to switch to <nil key>.",fObjectKey->GetName(),oldNode->GetName());
	hsAssert(oldNode == newNode, msg);
#endif  // HS_DEBUGGING
}

/////////////////////////////////////////////////////////////////////
//
// READING AND WRITING
//
/////////////////////////////////////////////////////////////////////

#include "plPXStream.h"

void plPXPhysical::Read(hsStream* stream, hsResMgr* mgr)
{
	plPhysical::Read(stream, mgr);	
	ClearMatrix(fCachedLocal2World);

	PhysRecipe recipe;
	recipe.mass = stream->ReadSwapScalar();
	recipe.friction = stream->ReadSwapScalar();
	recipe.restitution = stream->ReadSwapScalar();
	recipe.bounds = (plSimDefs::Bounds)stream->ReadByte();
	recipe.group = (plSimDefs::Group)stream->ReadByte();
	recipe.reportsOn = stream->ReadSwap32();
	fLOSDBs = stream->ReadSwap16();
	//hack for swim regions currently they are labeled as static av blockers
	if(fLOSDBs==plSimDefs::kLOSDBSwimRegion)
	{
		recipe.group=plSimDefs::kGroupMax;
	}
	//
	recipe.objectKey = mgr->ReadKey(stream);
	recipe.sceneNode = mgr->ReadKey(stream);
	recipe.worldKey = mgr->ReadKey(stream);

	mgr->ReadKeyNotifyMe(stream, TRACKED_NEW plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefSndGroup), plRefFlags::kActiveRef);

	hsPoint3 pos;
	hsQuat rot;
	pos.Read(stream);
	rot.Read(stream);
	rot.MakeMatrix(&recipe.l2s);
	recipe.l2s.SetTranslate(&pos);

	fProps.Read(stream);

	if (recipe.bounds == plSimDefs::kSphereBounds)
	{
		recipe.radius = stream->ReadSwapScalar();
		recipe.offset.Read(stream);
	}
	else if (recipe.bounds == plSimDefs::kBoxBounds)
	{
		recipe.bDimensions.Read(stream);
		recipe.bOffset.Read(stream);
	}
	else
	{
		// Read in the cooked mesh
		plPXStream pxs(stream);
		if (recipe.bounds == plSimDefs::kHullBounds)
			recipe.convexMesh = plSimulationMgr::GetInstance()->GetSDK()->createConvexMesh(pxs);
		else
			recipe.triMesh = plSimulationMgr::GetInstance()->GetSDK()->createTriangleMesh(pxs);
	}

	Init(recipe);

	hsAssert(!fProxyGen, "Already have proxy gen, double read?");

	hsColorRGBA physColor;
	hsScalar opac = 1.0f;

	if (fGroup == plSimDefs::kGroupAvatar)
	{
		// local avatar is light purple and transparent
		physColor.Set(.2f, .1f, .2f, 1.f);
		opac = 0.4f;
	}
	else if (fGroup == plSimDefs::kGroupDynamic)
	{
		// Dynamics are red
		physColor.Set(1.f,0.f,0.f,1.f);
	}
	else if (fGroup == plSimDefs::kGroupDetector)
	{
		if(!fWorldKey)
		{
			// Detectors are blue, and transparent
			physColor.Set(0.f,0.f,1.f,1.f);
			opac = 0.3f;
		}
		else
		{
			// subworld Detectors are green
			physColor.Set(0.f,1.f,0.f,1.f);
			opac = 0.3f;
		}
	}
	else if (fGroup == plSimDefs::kGroupStatic)
	{
		if (GetProperty(plSimulationInterface::kPhysAnim))
			// Statics that are animated are more reddish?
			physColor.Set(1.f,0.6f,0.2f,1.f);
		else
			// Statics are yellow
			physColor.Set(1.f,0.8f,0.2f,1.f);
		// if in a subworld... slightly transparent
		if(fWorldKey)
			opac = 0.6f;
	}
	else
	{
		// don't knows are grey
		physColor.Set(0.6f,0.6f,0.6f,1.f);
	}

	fProxyGen = TRACKED_NEW plPhysicalProxy(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
	fProxyGen->Init(this);
}

void plPXPhysical::Write(hsStream* stream, hsResMgr* mgr)
{
	plPhysical::Write(stream, mgr);

	hsAssert(fActor, "nil actor");	
	hsAssert(fActor->getNbShapes() == 1, "Can only write actors with one shape. Writing first only.");
	NxShape* shape = fActor->getShapes()[0];

	NxMaterialIndex matIdx = shape->getMaterial();
	NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
	NxMaterial* mat = scene->getMaterialFromIndex(matIdx);
	float friction = mat->getStaticFriction();
	float restitution = mat->getRestitution();

	stream->WriteSwapScalar(fActor->getMass());
	stream->WriteSwapScalar(friction);
	stream->WriteSwapScalar(restitution);
	stream->WriteByte(fBoundsType);
	stream->WriteByte(fGroup);
	stream->WriteSwap32(fReportsOn);
	stream->WriteSwap16(fLOSDBs);
	mgr->WriteKey(stream, fObjectKey);
	mgr->WriteKey(stream, fSceneNode);
	mgr->WriteKey(stream, fWorldKey);
	mgr->WriteKey(stream, fSndGroup);

	hsPoint3 pos;
	hsQuat rot;
	IGetPositionSim(pos);
	IGetRotationSim(rot);
	pos.Write(stream);
	rot.Write(stream);

	fProps.Write(stream);

	if (fBoundsType == plSimDefs::kSphereBounds)
	{
		const NxSphereShape* sphereShape = shape->isSphere();
		stream->WriteSwapScalar(sphereShape->getRadius());
		hsPoint3 localPos = plPXConvert::Point(sphereShape->getLocalPosition());
		localPos.Write(stream);
	}
	else if (fBoundsType == plSimDefs::kBoxBounds)
	{
		const NxBoxShape* boxShape = shape->isBox();
		hsPoint3 dim = plPXConvert::Point(boxShape->getDimensions());
		dim.Write(stream);
		hsPoint3 localPos = plPXConvert::Point(boxShape->getLocalPosition());
		localPos.Write(stream);
	}
	else
	{
		if (fBoundsType == plSimDefs::kHullBounds)
			hsAssert(shape->isConvexMesh(), "Hull shape isn't a convex mesh");
		else
			hsAssert(shape->isTriangleMesh(), "Exact shape isn't a trimesh");

		// We hide the stream we used to create this mesh away in the shape user data.
		// Pull it out and write it to disk.
		hsVectorStream* vecStream = (hsVectorStream*)shape->userData;
		stream->Write(vecStream->GetEOF(), vecStream->GetData());
		delete vecStream;
	}
}

//
// TESTING SDL
// Send phys sendState msg to object's plPhysicalSDLModifier
//
hsBool plPXPhysical::DirtySynchState(const char* SDLStateName, UInt32 synchFlags )
{
	if (GetObjectKey())
	{
		plSynchedObject* so=plSynchedObject::ConvertNoRef(GetObjectKey()->ObjectIsLoaded());
		if (so)
		{
			fLastSyncTime = hsTimer::GetSysSeconds();
			return so->DirtySynchState(SDLStateName, synchFlags);
		}
	}

	return false;
}

void plPXPhysical::GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV)
{
	IGetPositionSim(pos);
	IGetRotationSim(rot);
	GetLinearVelocitySim(linV);
	GetAngularVelocitySim(angV);
}

void plPXPhysical::SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV)
{
	bool initialSync =	plNetClientApp::GetInstance()->IsLoadingInitialAgeState() &&
						plNetClientApp::GetInstance()->GetJoinOrder() == 0;

	// If the physical has fallen out of the sim, and this is initial age state, and we're
	// the first person in, reset it to the original position.  (ie, prop the default state
	// we've got right now)
	if (pos && pos->fZ < kMaxNegativeZPos && initialSync)
	{
		SimLog("Physical %s loaded out of range state.  Forcing initial state to server.", GetKeyName());
		DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
		return;
	}

	if (pos)
		ISetPositionSim(*pos);
	if (rot)
		ISetRotationSim(*rot);

	if (linV)
		SetLinearVelocitySim(*linV);
	if (angV)
		SetAngularVelocitySim(*angV);

	SendNewLocation(false, true);
}

void plPXPhysical::ExcludeRegionHack(hsBool cleared)
{
	NxShape* shape = fActor->getShapes()[0];
	shape->setFlag(NX_TRIGGER_ON_ENTER, !cleared);
	shape->setFlag(NX_TRIGGER_ON_LEAVE, !cleared);
	fGroup = cleared ? plSimDefs::kGroupExcludeRegion : plSimDefs::kGroupDetector;
	shape->setGroup(fGroup);
	/*if switching a static need to inform the controller that it needs to rebuild
	the collision cache otherwise will still think that the detector is still static or that
	the static is still a detector*/
	plPXPhysicalControllerCore::RebuildCache();

}
hsBool plPXPhysical::OverlapWithCapsule(NxCapsule& cap)
{
	NxShape* shape = fActor->getShapes()[0];
	return shape->checkOverlapCapsule(cap);
}

hsBool plPXPhysical::IsDynamic() const
{
	return fGroup == plSimDefs::kGroupDynamic &&
		!GetProperty(plSimulationInterface::kPhysAnim);
}

#include "../plSurface/hsGMaterial.h"
#include "../plSurface/plLayerInterface.h"

// Some helper functions for pulling info out of a PhysX trimesh description
inline hsPoint3& GetTrimeshVert(NxTriangleMeshDesc& desc, int idx)
{
	return *((hsPoint3*)(((char*)desc.points)+desc.pointStrideBytes*idx));
}

void GetTrimeshTri(NxTriangleMeshDesc& desc, int idx, UInt16* out)
{
	if (hsCheckBits(desc.flags, NX_MF_16_BIT_INDICES))
	{
		UInt16* descTris = ((UInt16*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
		out[0] = descTris[0];
		out[1] = descTris[1];
		out[2] = descTris[2];
	}
	else
	{
		UInt32* descTris = ((UInt32*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
		out[0] = (UInt16)descTris[0];
		out[1] = (UInt16)descTris[1];
		out[2] = (UInt16)descTris[2];
	}
}

// Some helper functions for pulling info out of a PhysX trimesh description
inline hsPoint3& GetConvexVert(NxConvexMeshDesc& desc, int idx)
{
	return *((hsPoint3*)(((char*)desc.points)+desc.pointStrideBytes*idx));
}

void GetConvexTri(NxConvexMeshDesc& desc, int idx, UInt16* out)
{
	if (hsCheckBits(desc.flags, NX_MF_16_BIT_INDICES))
	{
		UInt16* descTris = ((UInt16*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
		out[0] = descTris[0];
		out[1] = descTris[1];
		out[2] = descTris[2];
	}
	else
	{
		UInt32* descTris = ((UInt32*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
		out[0] = (UInt16)descTris[0];
		out[1] = (UInt16)descTris[1];
		out[2] = (UInt16)descTris[2];
	}
}

// Make a visible object that can be viewed by users for debugging purposes.
plDrawableSpans* plPXPhysical::CreateProxy(hsGMaterial* mat, hsTArray<UInt32>& idx, plDrawableSpans* addTo)
{
	plDrawableSpans* myDraw = addTo;
	hsMatrix44 l2w, unused;
	GetTransform(l2w, unused);
	
	hsBool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

	NxShape* shape = fActor->getShapes()[0];

	NxTriangleMeshShape* trimeshShape = shape->isTriangleMesh();
	if (trimeshShape)
	{
		NxTriangleMeshDesc desc;
		trimeshShape->getTriangleMesh().saveToDesc(desc);

		hsTArray<hsPoint3>	pos;
		hsTArray<UInt16>	tris;

		const int kMaxTris = 10000;
		const int kMaxVerts = 32000;
		if ((desc.numVertices < kMaxVerts) && (desc.numTriangles < kMaxTris))
		{
			pos.SetCount(desc.numVertices);
			tris.SetCount(desc.numTriangles * 3);

			for (int i = 0; i < desc.numVertices; i++ )
				pos[i] = GetTrimeshVert(desc, i);

			for (int i = 0; i < desc.numTriangles; i++)
				GetTrimeshTri(desc, i, &tris[i*3]);

			myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
											pos.AcquireArray(),
											nil,	// normals - def to avg (smooth) norm
											nil,	// uvws
											0,		// uvws per vertex
											nil,	// colors - def to white
											true,	// do a quick fake shade
											nil,	// optional color modulation
											tris.GetCount(),
											tris.AcquireArray(),
											mat,
											l2w,
											blended,
											&idx,
											myDraw);
		}
		else
		{
			int curTri = 0;
			int trisToDo = desc.numTriangles;
			while (trisToDo > 0)
			{
				int trisThisRound = trisToDo > kMaxTris ? kMaxTris : trisToDo;
				
				trisToDo -= trisThisRound;

				pos.SetCount(trisThisRound * 3);
				tris.SetCount(trisThisRound * 3);

				for (int i = 0; i < trisThisRound; i++)
				{
					GetTrimeshTri(desc, curTri, &tris[i*3]);
					pos[i*3 + 0] = GetTrimeshVert(desc, tris[i*3+0]);
					pos[i*3 + 1] = GetTrimeshVert(desc, tris[i*3+1]);
					pos[i*3 + 2] = GetTrimeshVert(desc, tris[i*3+2]);

					curTri++;
				}
				myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
												pos.AcquireArray(),
												nil,	// normals - def to avg (smooth) norm
												nil,	// uvws
												0,		// uvws per vertex
												nil,	// colors - def to white
												true,	// do a quick fake shade
												nil,	// optional color modulation
												tris.GetCount(),
												tris.AcquireArray(),
												mat,
												l2w,
												blended,
												&idx,
												myDraw);
			}
		}
	}

	NxConvexShape* convexShape = shape->isConvexMesh();
	if (convexShape)
	{
		NxConvexMeshDesc desc;
		convexShape->getConvexMesh().saveToDesc(desc);

		hsTArray<hsPoint3>	pos;
		hsTArray<UInt16>	tris;

		pos.SetCount(desc.numVertices);
		tris.SetCount(desc.numTriangles * 3);

		for (int i = 0; i < desc.numVertices; i++ )
			pos[i] = GetConvexVert(desc, i);

		for (int i = 0; i < desc.numTriangles; i++)
			GetConvexTri(desc, i, &tris[i*3]);

		myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
			pos.AcquireArray(),
			nil,	// normals - def to avg (smooth) norm
			nil,	// uvws
			0,		// uvws per vertex
			nil,	// colors - def to white
			true,	// do a quick fake shade
			nil,	// optional color modulation
			tris.GetCount(),
			tris.AcquireArray(),
			mat,
			l2w,
			blended,
			&idx,
			myDraw);
	}

	NxSphereShape* sphere = shape->isSphere();
	if (sphere)
	{
		float radius = sphere->getRadius();
		hsPoint3 offset = plPXConvert::Point(sphere->getLocalPosition());
		myDraw = plDrawableGenerator::GenerateSphericalDrawable(offset, radius,
			mat, l2w, blended,
			nil, &idx, myDraw);
	}

	NxBoxShape* box = shape->isBox();
	if (box)
	{
		hsPoint3 dim = plPXConvert::Point(box->getDimensions());
		myDraw = plDrawableGenerator::GenerateBoxDrawable(dim.fX*2.f, dim.fY*2.f, dim.fZ*2.f,
			mat,l2w,blended,
			nil,&idx,myDraw);
	}
	return myDraw;
}
