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
#include "plSimulationMgr.h"

#include <NxPhysics.h>

#include "plgDispatch.h"
#include "hsTimer.h"
#include "plProfile.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXConvert.h"
#include "plLOSDispatch.h"
#include "plPhysical/plPhysicsSoundMgr.h"
#include "plStatusLog/plStatusLog.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnNetCommon/plSDLTypes.h"
#include "plMessage/plCollideMsg.h"
#include "plMessage/plAgeLoadedMsg.h"

#include "plModifier/plDetectorLog.h"

#ifndef PLASMA_EXTERNAL_RELEASE
#include "plPipeline/plDebugText.h"
bool plSimulationMgr::fDisplayAwakeActors=false;
#endif //PLASMA_EXTERNAL_RELEASE
// This gets called by PhysX whenever a trigger gets penetrated.  This is used
// for any Plasma detectors.
class SensorReport : public NxUserTriggerReport
{
    virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status)
    {
        plKey otherKey = nil;
        bool doReport = false;

        // Get our trigger physical.  This should definitely have a plPXPhysical
        plPXPhysical* triggerPhys = (plPXPhysical*)triggerShape.getActor().userData;

        // Get the triggerer. If it doesn't have a plPXPhyscial, it's an avatar
        plPXPhysical* otherPhys = (plPXPhysical*)otherShape.getActor().userData;
        if (otherPhys)
        {
            otherKey = otherPhys->GetObjectKey();
            doReport = triggerPhys->DoReportOn((plSimDefs::Group)otherPhys->GetGroup());
        }
        else
        {
            plPXPhysicalControllerCore* controller = plPXPhysicalControllerCore::GetController(otherShape.getActor());
            if (controller)
            {
                otherKey = controller->GetOwner();
                doReport = triggerPhys->DoReportOn(plSimDefs::kGroupAvatar);
            }
        }

        if (doReport)
        {
            if (status & NX_TRIGGER_ON_ENTER)
            {
                if (plSimulationMgr::fExtraProfile)
                    DetectorLogRed("-->Send Collision %s enter",triggerPhys->GetObjectKey()->GetName().c_str());
                plSimulationMgr::GetInstance()->AddCollisionMsg(triggerPhys->GetObjectKey(), otherKey, true);
            }
            else if (status & NX_TRIGGER_ON_LEAVE)
            {
                if (plSimulationMgr::fExtraProfile)
                    DetectorLogRed("-->Send Collision %s exit",triggerPhys->GetObjectKey()->GetName().c_str());
                plSimulationMgr::GetInstance()->AddCollisionMsg(triggerPhys->GetObjectKey(), otherKey, false);
            }
        }
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
        case NXE_INVALID_PARAMETER: errorType = "invalid parameter";    break;
        case NXE_INVALID_OPERATION: errorType = "invalid operation";    break;
        case NXE_OUT_OF_MEMORY:     errorType = "out of memory";        break;
        case NXE_DB_INFO:           errorType = "info";                 break;
        case NXE_DB_WARNING:        errorType = "warning";              break;
        default:                    errorType = "unknown error";
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


/////////////////////////////////////////////////////////////////
//
// DEFAULTS
//
/////////////////////////////////////////////////////////////////

#define kDefaultMaxDelta    (0.15)        // if the step is greater than .15 seconds, clamp to that
#define kDefaultStepSize    (1.f / 60.f)  // default simulation freqency is 60hz

/////////////////////////////////////////////////////////////////
//
// CONSTRUCTION, INITIALIZATION, DESTRUCTION
//
/////////////////////////////////////////////////////////////////

// 
// Alloc all the sim timers here so they make a nice pretty display
//
//plProfile_CreateTimer(    "ClearContacts", "Simulation", ClearContacts);
plProfile_CreateTimer(  "Step", "Simulation", Step);
// plProfile_CreateTimer(   "  Broadphase", "Simulation", Broadphase);
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
// plProfile_CreateTimer(   "Narrowphase", "Simulation", Narrowphase);
// plProfile_CreateTimer(   "ProcessInterpenetration", "Simulation", ProcessInterpenetration);
plProfile_CreateTimer(  "LineOfSight", "Simulation", LineOfSight);
plProfile_CreateTimer(  "ProcessSyncs", "Simulation", ProcessSyncs);
plProfile_CreateTimer(  "UpdateContexts", "Simulation", UpdateContexts);
// plProfile_CreateCounter("  ContextUpdates", "Simulation", ContextUpdates);
plProfile_CreateCounter("  MaySendLocation", "Simulation", MaySendLocation);
plProfile_CreateCounter("  LocationsSent", "Simulation", LocationsSent);
plProfile_CreateTimer(  "  PhysicsUpdates","Simulation",PhysicsUpdates);
// plProfile_CreateTimer(   "EntityCleanup", "Simulation", EntityCleanup);
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
    gTheInstance = new plSimulationMgr();
    if (gTheInstance->InitSimulation())
        gTheInstance->RegisterAs(kSimulationMgr_KEY);
    else
    {
        // There was an error when creating the PhysX simulation
        // ...then get rid of the simulation instance
        delete gTheInstance; // clean up the memory we allocated
        gTheInstance = nil;
    }
}

// when the app is going away completely
void plSimulationMgr::Shutdown()
{
    hsAssert(gTheInstance, "Simulation manager missing during shutdown.");
    if (gTheInstance)
    {
        gTheInstance->UnRegisterAs(kSimulationMgr_KEY);     // this will destroy the instance
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
    , fAccumulator(0.0f)
    , fStepCount(0)
    , fLOSDispatch(new plLOSDispatch())
    , fSoundMgr(new plPhysicsSoundMgr)
    , fLog(nil)
{

}

bool plSimulationMgr::InitSimulation()
{
    fSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, NULL, &gErrorStream);
    if (!fSDK)
        return false; // client will handle this and ask user to install

    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Simulation.log", plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);
    fLog->AddLine("Initialized simulation mgr");

#ifndef PLASMA_EXTERNAL_RELEASE
    // If this is an internal build, enable the PhysX debugger
    fSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425);
#endif

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
        NxSceneDesc sceneDesc;
        sceneDesc.gravity.set(0, 0, -32.174049f);
        sceneDesc.userTriggerReport = &gSensorReport;
        sceneDesc.userContactReport = &gContactReport;
        scene = fSDK->createScene(sceneDesc);

        // See "Advancing The Simulation State" in the PhysX SDK Documentation
        // This will cause PhysX to only update for our step size. If we call simulate
        // faster than that, PhysX will return immediately. If we call it slower than that,
        // PhysX will do some extra steps for us (isn't that nice?).
        // Anyway, this should be a good way to make us independent of the framerate.
        // If not, I blame the usual suspects (Tye, eap, etc...)
        scene->setTiming(kDefaultStepSize);

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
            scene->setGroupCollisionFlag(i, plSimDefs::kGroupAvatarKinematic, false);
        }
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatar, false);
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatarBlocker, true);
        scene->setGroupCollisionFlag(plSimDefs::kGroupDynamic, plSimDefs::kGroupDynamicBlocker, true);
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupStatic, true);
        scene->setGroupCollisionFlag(plSimDefs::kGroupStatic, plSimDefs::kGroupAvatar, true);
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatar, plSimDefs::kGroupDynamic, true);

        // Kinematically controlled avatars interact with detectors and dynamics
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatarKinematic, plSimDefs::kGroupDetector, true);
        scene->setGroupCollisionFlag(plSimDefs::kGroupAvatarKinematic, plSimDefs::kGroupDynamic, true);

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

void plSimulationMgr::AddCollisionMsg(plKey hitee, plKey hitter, bool enter)
{
    // First, make sure we have no dupes
    for (CollisionVec::iterator it = fCollideMsgs.begin(); it != fCollideMsgs.end(); ++it)
    {
        plCollideMsg* pMsg = *it;

        // Should only ever be one receiver.
        // Oh, it seems we should update the hit status. The latest might be different than the older...
        // Even in the same frame >.<
        if (pMsg->fOtherKey == hitter && pMsg->GetReceiver(0) == hitee)
        {
            pMsg->fEntering = enter;
            DetectorLogRed("DUPLICATE COLLISION: %s hit %s",
                (hitter ? hitter->GetName().c_str() : "(nil)"),
                (hitee ? hitee->GetName().c_str() : "(nil)"));
            return;
        }
    }

    // Still here? Then this must be a unique hit!
    plCollideMsg* pMsg = new plCollideMsg;
    pMsg->AddReceiver(hitee);
    pMsg->fOtherKey = hitter;
    pMsg->fEntering = enter;
    fCollideMsgs.push_back(pMsg);
}
void plSimulationMgr::AddCollisionMsg(plCollideMsg* msg)
{
    fCollideMsgs.push_back(msg);
}

void plSimulationMgr::Advance(float delSecs)
{
    if (fSuspended)
        return;

    fAccumulator += delSecs;
    if (fAccumulator < kDefaultStepSize)
    {
        // Not enough time has passed to perform a substep.
        plPXPhysicalControllerCore::UpdateNonPhysical(fAccumulator / kDefaultStepSize);
        return;
    }
    else if (fAccumulator > kDefaultMaxDelta)
    {
        if (fExtraProfile)
            Log("Step clamped from %f to limit of %f", fAccumulator, kDefaultMaxDelta);
        fAccumulator = kDefaultMaxDelta;
    }

    ++fStepCount;

    // Perform as many whole substeps as possible saving the remainder in our accumulator.
    int numSubSteps = (int)(fAccumulator / kDefaultStepSize + 0.000001f);
    float delta = numSubSteps * kDefaultStepSize;
    fAccumulator -= delta;

    plProfile_IncCount(StepLen, (int)(delta*1000));
    plProfile_BeginTiming(Step);

    plPXPhysicalControllerCore::Apply(delta);
    
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
            scene->simulate(delta);
            scene->flushStream();
            scene->fetchResults(NX_RIGID_BODY_FINISHED, true);
        }
    }

    plPXPhysicalControllerCore::Update(numSubSteps, fAccumulator / kDefaultStepSize);

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
    for (CollisionVec::iterator it = fCollideMsgs.begin(); it != fCollideMsgs.end(); ++it)
    {
        plCollideMsg* pMsg = *it;
        DetectorLogYellow("Collision: %s was triggered by %s. Sending an %s msg", pMsg->GetReceiver(0)->GetName().c_str(),
                          pMsg->fOtherKey ? pMsg->fOtherKey->GetName().c_str() : "(nil)" , pMsg->fEntering ? "'enter'" : "'exit'");
        plgDispatch::Dispatch()->MsgSend(pMsg);
    }
    fCollideMsgs.clear();

    SceneMap::iterator it = fScenes.begin();
    for (; it != fScenes.end(); it++)
    {
        NxScene* scene = it->second;
        uint32_t numActors = scene->getNbActors();
        NxActor** actors = scene->getActors();

        for (int i = 0; i < numActors; i++)
        {
            plPXPhysical* physical = (plPXPhysical*)actors[i]->userData;
            if (physical)
            {
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
                        const plString &physName = physical->GetKeyName();
                        if (!physName.IsNull())
                        {
                            plSimulationMgr::Log("Removing physical <%s> because of missing scene node.\n", physName.c_str());
                        }
                    }
//                  Remove(physical);
                }
            }
        }

//      // iterate through the db types, which are powers-of-two enums.
//      for( plLOSDB db = static_cast<plLOSDB>(1) ;
//          db < plSimDefs::kLOSDBMax;
//          db = static_cast<plLOSDB>(db << 1) )
//      {
//          fLOSSolvers[db]->Resolve(fSubspace);
//      }
//      if(fNeedLOSCullPhase)
//      {
//          for( plLOSDB db = static_cast<plLOSDB>(1) ;
//              db < plSimDefs::kLOSDBMax;
//              db = static_cast<plLOSDB>(db << 1) )
//          {
//              fLOSSolvers[db]->Resolve(fSubspace);
//          }
//          fNeedLOSCullPhase = false;
//      }
    }
}

/////////////////////////////////////////////////////////////////
//
//  RESOLUTION & TIMEOUT PARAMETERS
//
/////////////////////////////////////////////////////////////////

int plSimulationMgr::GetMaterialIdx(NxScene* scene, float friction, float restitution)
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
    if (mat)
        return mat->getMaterialIndex();
    else
        return NULL;
}

/////////////////////////////////////////////////////////////////
//
//  SYNCHRONIZATION
//  Very much a work in progress.
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

void plSimulationMgr::IDrawActiveActorList()
{
    plDebugText     &debugTxt = plDebugText::Instance();
    char            strBuf[ 2048 ];
    int             lineHeight = debugTxt.GetFontSize() + 4;
    uint32_t          scrnWidth, scrnHeight;

    debugTxt.GetScreenSize( &scrnWidth, &scrnHeight );
    int y = 10;
    int x = 10;

    sprintf(strBuf, "Number of scenes: %d", fScenes.size());
    debugTxt.DrawString(x, y, strBuf);
    y += lineHeight;
    int sceneNumber=1;
    for (SceneMap::iterator it = fScenes.begin(); it != fScenes.end(); it++)
    {
        
        sprintf(strBuf, "Scene: %s",it->first->GetName().c_str());
        debugTxt.DrawString(x, y, strBuf);
        y += lineHeight;
        uint32_t numActors =it->second->getNbActors();
        NxActor** actors =it->second->getActors();
        for(uint32_t i=0;i<numActors;i++)
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
