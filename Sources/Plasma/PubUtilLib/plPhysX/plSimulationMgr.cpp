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
#include "plSimulationMgr.h"

#include "NxPhysics.h"

#include "hsTimer.h"
#include "plProfile.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXConvert.h"
#include "plLOSDispatch.h"
#include "../plPhysical/plPhysicsSoundMgr.h"
#include "../plStatusLog/plStatusLog.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnNetCommon/plSDLTypes.h"
#include "../plMessage/plCollideMsg.h"

#include "../plModifier/plDetectorLog.h"

#ifndef PLASMA_EXTERNAL_RELEASE
bool plSimulationMgr::fDisplayAwakeActors=false;
#endif //PLASMA_EXTERNAL_RELEASE
// This gets called by PhysX whenever a trigger gets penetrated.  This is used
// for any Plasma detectors.
class SensorReport : public NxUserTriggerReport
{
	virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
	{
		// Get our trigger physical.  This should definitely have a plPXPhysical
		plPXPhysical* triggerPhys = (plPXPhysical*)triggerShape.getActor().userData;
		hsBool doReport = false;

		// Get the triggerer.  This may be an avatar, which doesn't have a
		// plPXPhysical, so we have to extract the necessary info.
		plKey otherKey = nil;
		hsPoint3 otherPos = plPXConvert::Point(otherShape.getGlobalPosition());

		if (plSimulationMgr::fExtraProfile)
			DetectorLogRed("-->%s %s (status=%x) other@(%f,%f,%f)",triggerPhys->GetObjectKey()->GetName(),status & NX_TRIGGER_ON_ENTER ? "enter" : "exit",status,otherPos.fX,otherPos.fY,otherPos.fZ);

		plPXPhysical* otherPhys = (plPXPhysical*)otherShape.getActor().userData;
		if (otherPhys)
		{
			otherKey = otherPhys->GetObjectKey();
			doReport = triggerPhys->DoReportOn((plSimDefs::Group)otherPhys->GetGroup());
			if (!doReport)
			{
				if (plSimulationMgr::fExtraProfile)
					DetectorLogRed("<--Kill collision %s :failed group. US=%x OTHER=(%s)%x",triggerPhys->GetObjectKey()->GetName(),triggerPhys->GetGroup(),otherPhys->GetObjectKey()->GetName(),otherPhys->GetGroup());
			}
		}
		else
		{
			bool isController;
			plPXPhysicalControllerCore* controller = plPXPhysicalControllerCore::GetController(otherShape.getActor(),&isController);
			if (controller)
			{
				if (isController)
				{
#ifdef PHYSX_ONLY_TRIGGER_FROM_KINEMATIC
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("<--Kill collision %s : ignoring controller events.",triggerPhys->GetObjectKey()->GetName());
					return;
#else // else if trigger on both controller and kinematic
					// only suppress controller collision 'enters' when disabled but let 'exits' continue
					// ...this is because there are detector regions that are on the edge on ladders that the exit gets missed.
					if ( ( !controller->IsEnabled() /*&& (status & NX_TRIGGER_ON_ENTER)*/ ) || controller->IsKinematic() )
					{
						if (plSimulationMgr::fExtraProfile)
							DetectorLogRed("<--Kill collision %s : controller is not enabled.",triggerPhys->GetObjectKey()->GetName());
						return;
					}
#endif  // PHYSX_ONLY_TRIGGER_FROM_KINEMATIC
				}
#ifndef PHYSX_ONLY_TRIGGER_FROM_KINEMATIC  // if triggering only kinematics, then all should trigger
				else
				{
					// only suppress kinematic collision 'enters' when disabled but let 'exits' continue
					// ...this is because there are detector regions that are on the edge on ladders that the exit gets missed.
					if ( !controller->IsKinematic() /*&& (status & NX_TRIGGER_ON_ENTER) */ )
					{
						if (plSimulationMgr::fExtraProfile)
							DetectorLogRed("<--Kill collision %s : kinematic is not enabled.",triggerPhys->GetObjectKey()->GetName());
						return;
					}
				}
#endif  // PHYSX_ONLY_TRIGGER_FROM_KINEMATIC
				otherKey = controller->GetOwner();
				doReport = triggerPhys->DoReportOn(plSimDefs::kGroupAvatar);
				if (plSimulationMgr::fExtraProfile )
				{
					if (!doReport)
					{
						DetectorLogRed("<--Kill collision %s :failed group. US=%x OTHER=(NotAvatar)",triggerPhys->GetObjectKey()->GetName(),triggerPhys->GetGroup());
					}
					else
					{
						hsPoint3 avpos;
						controller->GetPositionSim(avpos);
						DetectorLogRed("-->Avatar at (%f,%f,%f)",avpos.fX,avpos.fY,avpos.fZ);
					}
				}
			}
		}

		if (doReport)
		{
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
			if ( triggerPhys->DoDetectorHullWorkaround() )
			{
				if (status & NX_TRIGGER_ON_ENTER && triggerPhys->Should_I_Trigger(status & NX_TRIGGER_ON_ENTER, otherPos) )
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("-->Send Collision (CH) %s %s",triggerPhys->GetObjectKey()->GetName(),status & NX_TRIGGER_ON_ENTER ? "enter" : "exit");
					SendCollisionMsg(triggerPhys->GetObjectKey(), otherKey, true);
				}
				else if (status & NX_TRIGGER_ON_ENTER)
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("<--Kill collision %s :failed Should I trigger",triggerPhys->GetObjectKey()->GetName());
				}
				if (status & NX_TRIGGER_ON_LEAVE && triggerPhys->Should_I_Trigger(status & NX_TRIGGER_ON_ENTER, otherPos) )
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("-->Send Collision (CH) %s %s",triggerPhys->GetObjectKey()->GetName(),status & NX_TRIGGER_ON_ENTER ? "enter" : "exit");
					SendCollisionMsg(triggerPhys->GetObjectKey(), otherKey, false);
				}
				else if (status & NX_TRIGGER_ON_LEAVE)
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("<--Kill collision %s :failed Should I trigger",triggerPhys->GetObjectKey()->GetName());
				}
				if (!(status & NX_TRIGGER_ON_ENTER) && !(status & NX_TRIGGER_ON_LEAVE) )
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("<--Kill collision %s :failed event(CH)",triggerPhys->GetObjectKey()->GetName());
				}
			}
			else
			{
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
				if (status & NX_TRIGGER_ON_ENTER)
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("-->Send Collision %s %s",triggerPhys->GetObjectKey()->GetName(),status & NX_TRIGGER_ON_ENTER ? "enter" : "exit");
					SendCollisionMsg(triggerPhys->GetObjectKey(), otherKey, true);
				}
				if (status & NX_TRIGGER_ON_LEAVE)
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("-->Send Collision %s %s",triggerPhys->GetObjectKey()->GetName(),status & NX_TRIGGER_ON_ENTER ? "enter" : "exit");
					SendCollisionMsg(triggerPhys->GetObjectKey(), otherKey, false);
				}
				if (!(status & NX_TRIGGER_ON_ENTER) && !(status & NX_TRIGGER_ON_LEAVE) )
				{
					if (plSimulationMgr::fExtraProfile)
						DetectorLogRed("<--Kill collision %s :failed event",triggerPhys->GetObjectKey()->GetName());
				}
#ifdef USE_PHYSX_CONVEXHULL_WORKAROUND
			}
#endif  // USE_PHYSX_CONVEXHULL_WORKAROUND
		}
	}

	void SendCollisionMsg(plKey receiver, plKey hitter, hsBool entering)
	{
		plCollideMsg* msg = TRACKED_NEW plCollideMsg;
		msg->fOtherKey = hitter;
		msg->fEntering = entering;
		msg->AddReceiver(receiver);
//		msg->Send();\
		//placing message in a list to be fired off after sim step
		plSimulationMgr::GetInstance()->AddCollisionMsg(msg);
	}

} gSensorReport;

// This gets called by PhysX whenever two actor groups that are set to report
// have a collision.  We enable this for when a dynamic collides with anything.
class ContactReport : public NxUserContactReport
{
	virtual void onContactNotify(NxContactPair& pair, NxU32 events)
	{
		plPXPhysical* phys1 = (plPXPhysical*)pair.actors[0]->userData;
		plPXPhysical* phys2 = (plPXPhysical*)pair.actors[1]->userData;

		// Normally, these are always valid because the avatar (who doesn't have
		// a physical) will push other physicals away before they actually touch
		// his actor.  However, if the avatar is warped to a new position he may
		// collide with the object for a few frames.  We just ignore it.
		if (!phys1 || !phys2)
			return;

		plSimulationMgr::GetInstance()->ConsiderSynch(phys1, phys2);

		if (phys1->GetSoundGroup() && phys2->GetSoundGroup())
		{
			hsPoint3 contactPoint(0, 0, 0);

			// Just grab the last contact point
			NxContactStreamIterator i(pair.stream);
			while (i.goNextPair())
			{
				while (i.goNextPatch())
				{
					const NxVec3& contactNormal = i.getPatchNormal();
					while (i.goNextPoint())
					{
						contactPoint = plPXConvert::Point(i.getPoint());
					}
				}
			}

			plSimulationMgr::GetInstance()->fSoundMgr->AddContact(
				phys1, phys2, contactPoint,
				plPXConvert::Vector(pair.sumNormalForce));
		}
	}
} gContactReport;

// This directs any errors or warnings from PhysX to the simulation log.
class ErrorStream : public NxUserOutputStream
{
	virtual void reportError(NxErrorCode e, const char* message, const char* file, int line)
	{
		const char* errorType = nil;
		switch (e)
		{
		case NXE_INVALID_PARAMETER:	errorType = "invalid parameter";	break;
		case NXE_INVALID_OPERATION:	errorType = "invalid operation";	break;
		case NXE_OUT_OF_MEMORY:		errorType = "out of memory";		break;
		case NXE_DB_INFO:			errorType = "info";					break;
		case NXE_DB_WARNING:		errorType = "warning";				break;
		default:					errorType = "unknown error";
		}

		plSimulationMgr::Log("%s(%d) : %s: %s", file, line, errorType, message);
	}

	virtual NxAssertResponse reportAssertViolation(const char* message, const char* file, int line)
	{
		plSimulationMgr::Log("access violation : %s (%s(%d))", message, file, line);
		hsAssert(0, "PhysX assert, see simulation log for details");
		return NX_AR_CONTINUE;
	}

	virtual void print(const char* message)
	{
		plSimulationMgr::Log(message);
	}
} gErrorStream;

// This class allows PhysX to use our heap manager
static class HeapAllocator : public NxUserAllocator {
public:
	void * malloc (NxU32 size) {
		return ALLOC(size);
	}
	void * mallocDEBUG (NxU32 size, const char * fileName, int line) {
		return MemAlloc(size, 0, fileName, line);
	}
	void * realloc (void * memory, NxU32 size) {
		return REALLOC(memory, size);
	}
	void free (void * memory) {
		FREE(memory);
	}
} gHeapAllocator;


/////////////////////////////////////////////////////////////////
//
// DEFAULTS
//
/////////////////////////////////////////////////////////////////

#define kDefaultMaxDelta	0.1			// if the step is greater than .1 seconds, clamp to that
#define kDefaultStepSize	1.f / 60.f	// default simulation freqency is 60hz

/////////////////////////////////////////////////////////////////
//
// CONSTRUCTION, INITIALIZATION, DESTRUCTION
//
/////////////////////////////////////////////////////////////////

// 
// Alloc all the sim timers here so they make a nice pretty display
//
//plProfile_CreateTimer(	"ClearContacts", "Simulation", ClearContacts);
plProfile_CreateTimer(	"Step", "Simulation", Step);
// plProfile_CreateTimer(	"  Broadphase", "Simulation", Broadphase);
plProfile_CreateCounter("  Awake", "Simulation", Awake);
plProfile_CreateCounter("  Contacts", "Simulation", Contacts);
plProfile_CreateCounter("  DynActors", "Simulation", DynActors);
plProfile_CreateCounter("  DynShapes", "Simulation", DynShapes);
plProfile_CreateCounter("  StaticShapes", "Simulation", StaticShapes);
plProfile_CreateCounter("  Actors", "Simulation", Actors);
plProfile_CreateCounter("  PhyScenes", "Simulation", Scenes);

// plProfile_CreateCounter("  Broadphase Rejected", "Simulation", BroadphaseReject);
// plProfile_CreateCounter("  Broadphase Accepted", "Simulation", BroadphaseAccept);
// plProfile_CreateCounter("  Impact", "Simulation", Impact);
// plProfile_CreateCounter("  Penetration", "Simulation", Penetration);
// plProfile_CreateTimer(	"Narrowphase", "Simulation", Narrowphase);
// plProfile_CreateTimer(	"ProcessInterpenetration", "Simulation", ProcessInterpenetration);
plProfile_CreateTimer(	"LineOfSight", "Simulation", LineOfSight);
plProfile_CreateTimer(	"ProcessSyncs", "Simulation", ProcessSyncs);
plProfile_CreateTimer(	"UpdateContexts", "Simulation", UpdateContexts);
// plProfile_CreateCounter("  ContextUpdates", "Simulation", ContextUpdates);
plProfile_CreateCounter("  MaySendLocation", "Simulation", MaySendLocation);
plProfile_CreateCounter("  LocationsSent", "Simulation", LocationsSent);
plProfile_CreateTimer(	"  PhysicsUpdates","Simulation",PhysicsUpdates);
// plProfile_CreateTimer(	"EntityCleanup", "Simulation", EntityCleanup);
plProfile_CreateCounter("SetTransforms Accepted", "Simulation", SetTransforms);
plProfile_CreateCounter("AnimatedPhysicals", "Simulation", AnimatedPhysicals);
plProfile_CreateCounter("AnimatedActivators", "Simulation", AnimatedActivators);
plProfile_CreateCounter("Controllers", "Simulation", Controllers);
// plProfile_CreateCounter("NumSteps", "Simulation", NumSteps);
plProfile_CreateCounter("StepLength", "Simulation", StepLen);

// declared at file scope so that both GetInstance and the destructor can access it.
static plSimulationMgr* gTheInstance = NULL;
bool plSimulationMgr::fExtraProfile = false;
bool plSimulationMgr::fSubworldOptimization = false;
bool plSimulationMgr::fDoClampingOnStep=true;

void plSimulationMgr::Init()
{
	hsAssert(!gTheInstance, "Initializing the sim when it's already been done");
	gTheInstance = TRACKED_NEW plSimulationMgr();
	if (gTheInstance->InitSimulation())
	{
		gTheInstance->RegisterAs(kSimulationMgr_KEY);
		gTheInstance->GetKey()->RefObject();
	}
	else
	{
		// There was an error when creating the PhysX simulation
		// ...then get rid of the simulation instance
		DEL(gTheInstance); // clean up the memory we allocated
		gTheInstance = nil;
	}
}

// when the app is going away completely
void plSimulationMgr::Shutdown()
{
	hsAssert(gTheInstance, "Simulation manager missing during shutdown.");
	if (gTheInstance)
	{
		// UnRef to match our Ref in Init(). Unless something strange is
		// going on, this should destroy the instance and set gTheInstance to nil.
//		gTheInstance->GetKey()->UnRefObject();
		gTheInstance->UnRegister();		// this will destroy the instance
		gTheInstance = nil;
	}
}

plSimulationMgr* plSimulationMgr::GetInstance()
{
	return gTheInstance;
}

//////////////////////////////////////////////////////////////////////////

plSimulationMgr::plSimulationMgr()
	: fSuspended(true)
	, fMaxDelta(kDefaultMaxDelta)
	, fStepSize(kDefaultStepSize)
	, fLOSDispatch(TRACKED_NEW plLOSDispatch())
	, fSoundMgr(new plPhysicsSoundMgr)
	, fLog(nil)
{

}

bool plSimulationMgr::InitSimulation()
{
	fSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, &gHeapAllocator, &gErrorStream);
	if (!fSDK)
		return false; // client will handle this and ask user to install

	fLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Simulation.log", plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);
	fLog->AddLine("Initialized simulation mgr");

#ifndef PLASMA_EXTERNAL_RELEASE
	// If this is an internal build, enable the PhysX debugger
	fSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425);
#endif

	if ( !plPXConvert::Validate() )
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		hsMessageBox("Ageia's PhysX or Plasma offsets have changed, need to rewrite conversion code.","PhysX Error",MB_OK);
#endif
		return false; // client will handle this and ask user to install
	}

	return true;
}

plSimulationMgr::~plSimulationMgr()
{
	fLOSDispatch->UnRef();
	fLOSDispatch = nil;

	delete fSoundMgr;
	fSoundMgr = nil;

	hsAssert(fScenes.empty(), "Unreleased scenes at shutdown");

	if (fSDK)
		fSDK->release();

	delete fLog;
	fLog = nil;
}

NxScene* plSimulationMgr::GetScene(plKey world)
{
	if (!world)
		world = GetKey();

	NxScene* scene = fScenes[world];

	if (!scene)
	{
		UInt32 maxSteps = (UInt32)hsCeil(fMaxDelta / fStepSize);

		NxSceneDesc sceneDesc;
		sceneDesc.gravity.set(0, 0, -32.174049f);
		sceneDesc.userTriggerReport = &gSensorReport;
		sceneDesc.userContactReport = &gContactReport;
		sceneDesc.maxTimestep = fStepSize;
		sceneDesc.maxIter = maxSteps;
		scene = fSDK->createScene(sceneDesc);

		// Most physicals use the default friction and restitution values, so we
		// make them the default.
		NxMaterial* mat = scene->getMaterialFromIndex(0);
		float rest = mat->getRestitution();
		float sfriction = mat->getStaticFriction();
		float dfriction = mat->getDynamicFriction();
		mat->setRestitution(0.5);
		mat->setStaticFriction(0.5);
		mat->setDynamicFriction(0.5);

		// By default we just leave all the collision groups enabled, since
		// PhysX already makes sure that things like statics and statics don't
		// collide.  However, we do make it so the avatar and dynamic blockers
		// only block avatars and dynamics.
		for (int i = 0; i < plSimDefs::kGroupMax; i++)
		{
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupAvatarBlocker, false);
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupDynamicBlocker, false);
			scene->setGroupCollisionFlag(i, plSimDefs::kGroupLOSOnly, false);
			scene->setGroupCollisionFlag(plSimDefs::kGroupLOSOnly, i, false);
		}
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatar, false);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatarBlocker, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupDynamic, plSimDefs::kGroupDynamicBlocker, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupStatic, true);
		scene->setGroupCollisionFlag( plSimDefs::kGroupStatic, plSimDefs::kGroupAvatar, true);
		scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupDynamic, true);
		
		// The dynamics are in actor group 1, everything else is in 0.  Request
		// a callback for whenever a dynamic touches something.
		scene->setActorGroupPairFlags(0, 1, NX_NOTIFY_ON_TOUCH);
		scene->setActorGroupPairFlags(1, 1, NX_NOTIFY_ON_TOUCH);

		fScenes[world] = scene;
	}

	return scene;
}

void plSimulationMgr::ReleaseScene(plKey world)
{
	if (!world)
		world = GetKey();

	SceneMap::iterator it = fScenes.find(world);
	hsAssert(it != fScenes.end(), "Unknown scene");
	if (it != fScenes.end())
	{
		NxScene* scene = it->second;
		if (scene->getNbActors() == 0)
		{
			fSDK->releaseScene(*scene);
			fScenes.erase(it);
		}
	}
}

void plSimulationMgr::ISendCollisionMsg(plKey receiver, plKey hitter, hsBool entering)
{
	plCollideMsg* msg = TRACKED_NEW plCollideMsg;
	msg->fOtherKey = hitter;
	msg->fEntering = entering;
	msg->AddReceiver(receiver);
	msg->Send();
}

void plSimulationMgr::AddCollisionMsg(plCollideMsg* msg)
{
	fCollisionMessages.Append(msg);
}
void plSimulationMgr::IDispatchCollisionMessages()
{
	if(fCollisionMessages.GetCount())
	{
#ifndef PLASMA_EXTERNAL_RELEASE
		DetectorLog("--------------------------------------------------");
		DetectorLog("Dispatching collision messages from last sim step");	
#endif
		for(int i=0; i<fCollisionMessages.GetCount();i++)
		{
#ifndef PLASMA_EXTERNAL_RELEASE
			DetectorLog("%s was hit by %s. Sending an %s message",fCollisionMessages[i]->GetReceiver(0)->GetName(),
				fCollisionMessages[i]->GetSender()?fCollisionMessages[i]->GetSender()->GetName():"An Avatar",
				fCollisionMessages[i]->fEntering? "enter" : "exit");
#endif
			fCollisionMessages[i]->Send();
		}
#ifndef PLASMA_EXTERNAL_RELEASE
		DetectorLog("--------------------------------------------------");
#endif
		fCollisionMessages.SetCount(0);
	}
}

void plSimulationMgr::UpdateDetectorsInScene(plKey world, plKey avatar, hsPoint3& pos, bool entering)
{
	// search thru the actors in a scene looking for convex hull detectors and see if the avatar is inside it
	// ... and then send appropiate collision message if needed
	NxScene* scene = GetScene(world);
	plSceneObject* avObj = plSceneObject::ConvertNoRef(avatar->ObjectIsLoaded());
	const plCoordinateInterface* ci = avObj->GetCoordinateInterface();
	hsPoint3 soPos = ci->GetWorldPos();
	if (scene)
	{
		UInt32 numActors = scene->getNbActors();
		NxActor** actors = scene->getActors();

		for (int i = 0; i < numActors; i++)
		{
			plPXPhysical* physical = (plPXPhysical*)actors[i]->userData;
			if (physical && physical->DoDetectorHullWorkaround())
			{
				if ( physical->IsObjectInsideHull(pos) )
				{
					physical->SetInsideConvexHull(entering);
					// we are entering this world... say we entered this detector
					ISendCollisionMsg(physical->GetObjectKey(), avatar, entering);
				}
			}
		}
	}
}

void plSimulationMgr::UpdateAvatarInDetector(plKey world, plPXPhysical* detector)
{
	// search thru the actors in a scene looking for avatars that might be in the newly enabled detector region
	// ... and then send appropiate collision message if needed
	if ( detector->DoDetectorHullWorkaround() )
	{
		NxScene* scene = GetScene(world);
		if (scene)
		{
			UInt32 numActors = scene->getNbActors();
			NxActor** actors = scene->getActors();

			for (int i = 0; i < numActors; i++)
			{
				if ( actors[i]->userData == nil )
				{
					// we go a controller
					bool isController;
					plPXPhysicalControllerCore* controller = plPXPhysicalControllerCore::GetController(*actors[i],&isController);
					if (controller && controller->IsEnabled())
					{
						plKey avatar = controller->GetOwner();
						plSceneObject* avObj = plSceneObject::ConvertNoRef(avatar->ObjectIsLoaded());
						const plCoordinateInterface* ci;
						if ( avObj && ( ci = avObj->GetCoordinateInterface() ) )
						{
							if ( detector->IsObjectInsideHull(ci->GetWorldPos()) )
							{
								detector->SetInsideConvexHull(true);
								// we are entering this world... say we entered this detector
								ISendCollisionMsg(detector->GetObjectKey(), avatar, true);
							}
						}
					}
				}
			}
		}
	}
}

void plSimulationMgr::Advance(float delSecs)
{
	if (fSuspended)
		return;

	if (delSecs > fMaxDelta)
	{
		if (fExtraProfile)
			Log("Step clamped from %f to limit of %f", delSecs, fMaxDelta);
		delSecs = fMaxDelta;
	}
	plProfile_IncCount(StepLen, (int)(delSecs*1000));

#ifndef PLASMA_EXTERNAL_RELASE
	UInt32 stepTime = hsTimer::GetPrecTickCount();
#endif
	plProfile_BeginTiming(Step);
	plPXPhysicalControllerCore::UpdatePrestep(delSecs);
	plPXPhysicalControllerCore::UpdatePoststep( delSecs);
	
	for (SceneMap::iterator it = fScenes.begin(); it != fScenes.end(); it++)
	{
		NxScene* scene = it->second;
		bool do_advance = true;
		if (fSubworldOptimization)
		{
			plKey world = (plKey)it->first;
			if (world == GetKey())
				world = nil;
			do_advance = plPXPhysicalControllerCore::AnyControllersInThisWorld(world);
		}
		if (do_advance)
		{
			scene->simulate(delSecs);
			scene->flushStream();
			scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
		}
	}
	plPXPhysicalControllerCore::UpdatePostSimStep(delSecs);
	
	//sending off and clearing the Collision Messages generated by scene->simulate
	IDispatchCollisionMessages();

	plProfile_EndTiming(Step);
#ifndef PLASMA_EXTERNAL_RELEASE
	if(plSimulationMgr::fDisplayAwakeActors)IDrawActiveActorList();
#endif 
	if (fExtraProfile)
	{
		int contacts = 0, dynActors = 0, dynShapes = 0, awake = 0, stShapes=0, actors=0, scenes=0, controllers=0 ;
		for (SceneMap::iterator it = fScenes.begin(); it != fScenes.end(); it++)
		{
			bool do_advance = true;
			if (fSubworldOptimization)
			{
				plKey world = (plKey)it->first;
				if (world == GetKey())
					world = nil;
				do_advance = plPXPhysicalControllerCore::AnyControllersInThisWorld(world);
			}
			if (do_advance)
			{
				NxScene* scene = it->second;
				NxSceneStats stats;
				scene->getStats(stats);

				contacts += stats.numContacts;
				dynActors += stats.numDynamicActors;
				dynShapes += stats.numDynamicShapes;
				awake += stats.numDynamicActorsInAwakeGroups;
				stShapes += stats.numStaticShapes;
				actors += stats.numActors;
				scenes += 1;
				controllers += plPXPhysicalControllerCore::NumControllers();
			}
		}

		plProfile_IncCount(Awake, awake);
		plProfile_IncCount(Contacts, contacts);
		plProfile_IncCount(DynActors, dynActors);
		plProfile_IncCount(DynShapes, dynShapes);
		plProfile_IncCount(StaticShapes, stShapes);
		plProfile_IncCount(Actors, actors);
		plProfile_IncCount(Scenes, scenes);
		plProfile_IncCount(Controllers, controllers);
	}

	plProfile_IncCount(AnimatedPhysicals, plPXPhysical::fNumberAnimatedPhysicals);
	plProfile_IncCount(AnimatedActivators, plPXPhysical::fNumberAnimatedActivators);

	fSoundMgr->Update();

 	plProfile_BeginTiming(ProcessSyncs);
 	IProcessSynchs();
	plProfile_EndTiming(ProcessSyncs);

 	plProfile_BeginTiming(UpdateContexts);
	ISendUpdates();
 	plProfile_EndTiming(UpdateContexts);
}

void plSimulationMgr::ISendUpdates()
{
	SceneMap::iterator it = fScenes.begin();
	for (; it != fScenes.end(); it++)
	{
		NxScene* scene = it->second;
		UInt32 numActors = scene->getNbActors();
		NxActor** actors = scene->getActors();

		for (int i = 0; i < numActors; i++)
		{
			plPXPhysical* physical = (plPXPhysical*)actors[i]->userData;
			if (physical)
			{
				// apply any hit forces
				physical->ApplyHitForce();

				if (physical->GetSceneNode())
				{
					physical->SendNewLocation();
				}
				else
				{
					// if there's no scene node, it's not active (probably about to be collected)
					const plKey physKey = physical->GetKey();
					if (physKey)
					{
						const char *physName = physical->GetKeyName();
						if (physName)
						{
							plSimulationMgr::Log("Removing physical <%s> because of missing scene node.\n", physName);
						}
					}
//					Remove(physical);
				}
			}
		}

// 		// iterate through the db types, which are powers-of-two enums.
// 		for( plLOSDB db = static_cast<plLOSDB>(1) ;
// 			db < plSimDefs::kLOSDBMax;
// 			db = static_cast<plLOSDB>(db << 1) )
// 		{
// 			fLOSSolvers[db]->Resolve(fSubspace);
// 		}
// 		if(fNeedLOSCullPhase)
// 		{
// 			for( plLOSDB db = static_cast<plLOSDB>(1) ;
// 				db < plSimDefs::kLOSDBMax;
// 				db = static_cast<plLOSDB>(db << 1) )
// 			{
// 				fLOSSolvers[db]->Resolve(fSubspace);
// 			}
// 			fNeedLOSCullPhase = false;
// 		}
	}
}

hsBool plSimulationMgr::MsgReceive(plMessage *msg)
{
	return hsKeyedObject::MsgReceive(msg);
}

/////////////////////////////////////////////////////////////////
//
//  RESOLUTION & TIMEOUT PARAMETERS
//
/////////////////////////////////////////////////////////////////

void plSimulationMgr::SetMaxDelta(float maxDelta)
{
	fMaxDelta = maxDelta;
}

float plSimulationMgr::GetMaxDelta() const
{
	return fMaxDelta;
}

void plSimulationMgr::SetStepsPerSecond(int stepsPerSecond)
{
	fStepSize = 1.0f / (float)stepsPerSecond;
}

int plSimulationMgr::GetStepsPerSecond()
{
	return (int)((1.0 / fStepSize) + 0.5f);	// round to nearest int
}

int plSimulationMgr::GetMaterialIdx(NxScene* scene, hsScalar friction, hsScalar restitution)
{
	if (friction == 0.5f && restitution == 0.5f)
		return 0;

	// Use the nutty PhysX method to search for a matching material
	#define kNumMatsPerCall 32
	NxMaterial* materials[kNumMatsPerCall];
	NxU32 iterator = 0;
	bool getMore = true;
	while (getMore)
	{
		int numMats = scene->getMaterialArray(materials, kNumMatsPerCall, iterator);

		for (int i = 0; i < numMats; i++)
		{
			if (materials[i]->getDynamicFriction() == friction &&
				materials[i]->getRestitution() == restitution)
			{
				return materials[i]->getMaterialIndex();
			}
		}

		getMore = (numMats == kNumMatsPerCall);
	}

	// Couldn't find the material, so create it
	NxMaterialDesc desc;
	desc.restitution = restitution;
	desc.dynamicFriction = friction;
	desc.staticFriction = friction;
	NxMaterial* mat = scene->createMaterial(desc);
	return mat->getMaterialIndex();
}

/////////////////////////////////////////////////////////////////
//
//  SYNCHRONIZATION
//	Very much a work in progress.
//  *** would like to synchronize interacting groups as an atomic unit
//  *** need a "morphing synch" that incrementally approaches the target
//
/////////////////////////////////////////////////////////////////

const double plSimulationMgr::SynchRequest::kDefaultTime = -1000.0;

void plSimulationMgr::ConsiderSynch(plPXPhysical* physical, plPXPhysical* other)
{
	if (physical->GetProperty(plSimulationInterface::kNoSynchronize) &&
		(!other || other->GetProperty(plSimulationInterface::kNoSynchronize)))
		return;

	// We only need to sync if a dynamic is colliding with something.
	// Set it up so the dynamic is in 'physical'
	if (other && other->GetGroup() == plSimDefs::kGroupDynamic)
	{
		plPXPhysical* temp = physical;
		physical = other;
		other = temp;
	}
	// Neither is dynamic, so we can exit now
	else if (physical->GetGroup() != plSimDefs::kGroupDynamic)
		return;

	bool syncPhys = !physical->GetProperty(plSimulationInterface::kNoSynchronize) &&
					physical->IsDynamic() &&
					physical->IsLocallyOwned();
	bool syncOther = other &&
					!other->GetProperty(plSimulationInterface::kNoSynchronize) &&
					other->IsDynamic() != 0.f &&
					other->IsLocallyOwned();

	if (syncPhys)
	{
		double timeNow = hsTimer::GetSysSeconds();
		double timeElapsed = timeNow - physical->GetLastSyncTime();

		// If both objects are capable of syncing, we want to do it at the same
		// time, so no interpenetration issues pop up on other clients
		if (syncOther)
			timeElapsed = hsMaximum(timeElapsed, timeNow - other->GetLastSyncTime());

		// Set the sync time to 1 second from the last sync
		double syncTime = 0.0;
		if (timeElapsed > 1.0)
			syncTime = hsTimer::GetSysSeconds();
		else
			syncTime = hsTimer::GetSysSeconds() + (1.0 - timeElapsed);

		// This line will create and insert the request if it's not there already.
		SynchRequest& physReq = fPendingSynchs[physical];
		if (physReq.fTime == SynchRequest::kDefaultTime)
			physReq.fKey = physical->GetKey();
		physReq.fTime = syncTime;

		if (syncOther)
		{
			SynchRequest& otherReq = fPendingSynchs[other];
			if (otherReq.fTime == SynchRequest::kDefaultTime)
				otherReq.fKey = other->GetKey();
			otherReq.fTime = syncTime;
		}
	}
}

void plSimulationMgr::IProcessSynchs()
{
	double time = hsTimer::GetSysSeconds();

	PhysSynchMap::iterator i = fPendingSynchs.begin();

	while (i != fPendingSynchs.end())
	{
		SynchRequest req = (*i).second;
		if (req.fKey->ObjectIsLoaded())
		{
			plPXPhysical* phys = (*i).first;
			bool timesUp = (time >= req.fTime);
			bool allQuiet = false;//phys->GetActo GetBody()->isActive() == false;

			if (timesUp || allQuiet)
			{
				phys->DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
				i = fPendingSynchs.erase(i);
			}
			else
			{
				i++;
			}
		}
		else
		{
			i = fPendingSynchs.erase(i);
		}
	}
}

void plSimulationMgr::Log(const char * fmt, ...)
{
	if(gTheInstance)
	{
		plStatusLog* log = GetInstance()->fLog;
		if(log)
		{
			va_list args;
			va_start(args, fmt);
			log->AddLineV(fmt, args);
			va_end(args);
		}
	}
}

void plSimulationMgr::LogV(const char* formatStr, va_list args)
{
	if(gTheInstance)
	{
		plStatusLog * log = GetInstance()->fLog;
		if(log)
		{
			log->AddLineV(formatStr, args);
		}
	}
}

void plSimulationMgr::ClearLog()
{
	if(gTheInstance)
	{
		plStatusLog *log = GetInstance()->fLog;
		if(log)
		{
			log->Clear();
		}
	}
}
#ifndef PLASMA_EXTERNAL_RELEASE
#include "../plPipeline/plDebugText.h"

void plSimulationMgr::IDrawActiveActorList()
{
	plDebugText		&debugTxt = plDebugText::Instance();
	char			strBuf[ 2048 ];
	int				lineHeight = debugTxt.GetFontSize() + 4;
	UInt32			scrnWidth, scrnHeight;

	debugTxt.GetScreenSize( &scrnWidth, &scrnHeight );
	int	y = 10;
	int x = 10;

	sprintf(strBuf, "Number of scenes: %d", fScenes.size());
	debugTxt.DrawString(x, y, strBuf);
	y += lineHeight;
	int sceneNumber=1;
	for (SceneMap::iterator it = fScenes.begin(); it != fScenes.end(); it++)
	{
		
		sprintf(strBuf, "Scene: %s",it->first->GetName());
		debugTxt.DrawString(x, y, strBuf);
		y += lineHeight;
		UInt32 numActors =it->second->getNbActors();
		NxActor** actors =it->second->getActors();
		for(UInt32 i=0;i<numActors;i++)
		{
			if(!actors[i]->isSleeping())
			{
				sprintf(strBuf,"\t%s",actors[i]->getName());
				debugTxt.DrawString(x, y, strBuf);
				y += lineHeight;
			}
		}
		sceneNumber++;
	}
}
#endif //PLASMA_EXTERNAL_RELEASE
