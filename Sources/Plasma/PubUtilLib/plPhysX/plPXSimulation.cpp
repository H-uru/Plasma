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
#include "plPXSimulation.h"
#include "plPXConvert.h"
#include "plPhysXAPI.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXSimDefs.h"
#include "plPXSubWorld.h"
#include "plSimulationMgr.h"

#include "plProfile.h"

#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plStatusLog/plStatusLog.h"

// ==========================================================================

/** if the step is greater than .15 seconds, clamp to that */
constexpr float kDefaultMaxDelta = 0.15f;

/** default simulation freqency is 120hz */
constexpr float kDefaultStepSize = 1.f / 120.f;

/** Approximate size of objects in the simulation */
constexpr float kToleranceScaleLength = 6.f;

/** Typical magnitude of actor velocities in the simulation */
constexpr float kToleranceScaleSpeed = 32.f;

// ==========================================================================

plProfile_CreateTimer(  "Apply Controller Animations", "Simulation", ApplyController);
plProfile_CreateTimer(  "PhysX Simulation", "Simulation", Step);
plProfile_CreateTimer(  "  Contact Modify Callback", "Simulation", ContactModifyCallback);
plProfile_CreateTimer(  "  Contact Callback", "Simulation", ContactCallback);
plProfile_CreateTimer(  "  Trigger Callback", "Simulation", TriggerCallback);
plProfile_CreateCounter("  Contacts for Modification", "Simulation", ContactModifyCount);
plProfile_CreateCounter("  Active Bodies", "Simulation", ActiveBodies);
plProfile_CreateCounter("    Active Dynamics", "Simulation", ActiveDynamics);
plProfile_CreateCounter("    Active Kinematics", "Simulation", ActiveKinematics);
plProfile_CreateCounter("  Total Bodies", "Simulation", TotalBodies);
plProfile_CreateCounter("    Dynamics", "Simulation", Dynamics);
plProfile_CreateCounter("    Kinematics", "Simulation", Kinematics);
plProfile_CreateCounter("    Statics", "Simulation", Statics);
plProfile_CreateTimer(  "Correct Controller Movement", "Simulation", CorrectController);

// ==========================================================================

plPXActorData::plPXActorData(plPXPhysical* phys)
    : fKey(phys->GetObjectKey()), fPhysical(phys), fController()
{
    phys->GetKeyName().to_buffer(fNameBuf);
}

plPXActorData::plPXActorData(plPXPhysicalControllerCore* controller)
    : fKey(controller->GetOwner()), fPhysical(), fController(controller)
{
    if (plNetClientApp* nc = plNetClientApp::GetInstance())
        nc->GetPlayerName(fKey).to_buffer(fNameBuf);
    if (fNameBuf.empty())
        fKey->GetName().to_buffer(fNameBuf);
}

// ==========================================================================

static physx::PxDefaultAllocator gPxAllocator;

class plPXErrorHandler : public physx::PxErrorCallback
{
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        plStatusLog* log = plStatusLogMgr::GetInstance().FindLog("Simulation.log");
        switch (code) {
        case physx::PxErrorCode::eABORT:
            log->AddLineF(plStatusLog::kRed, "PhysX ABORT: '{}' File: {} Line: {}",
                          message, file, line);
            ErrorAssert(line, file, "PhysX ABORT: %s", message);
            break;
        case physx::PxErrorCode::eINTERNAL_ERROR:
            log->AddLineF(plStatusLog::kRed, "PhysX INTERNAL ERROR: '{}' File: {} Line: {}",
                          message, file, line);
            ErrorAssert(line, file, "PhysX INTERNAL ERROR: %s", message);
            break;
        case physx::PxErrorCode::eINVALID_OPERATION:
            log->AddLineF(plStatusLog::kRed, "PhysX INVALID OPERATION: '{}' File: {} Line: {}",
                          message, file, line);
            break;
        case physx::PxErrorCode::eINVALID_PARAMETER:
            log->AddLineF(plStatusLog::kRed, "PhysX INVALID PARAMETER: '{}' File: {} Line: {}",
                          message, file, line);
            break;
        case physx::PxErrorCode::eDEBUG_INFO:
            log->AddLineF(plStatusLog::kYellow, "PhysX INFO: '{}' File: {} Line: {}",
                          message, file, line);
            break;
        case physx::PxErrorCode::eDEBUG_WARNING:
            log->AddLineF(plStatusLog::kYellow, "PhysX WARNING: '{}' File: {} Line: {}",
                          message, file, line);
            break;
        DEFAULT_FATAL(PxErrorCode)
        }
    }
} static gPxErrorCallback;

// ==========================================================================

class plPXSimulationEventHandler : public physx::PxSimulationEventCallback
{
    void IHandleControllerContacts(const physx::PxContactPair& pair) const
    {
        auto a1 = static_cast<plPXActorData*>(pair.shapes[0]->getActor()->userData);
        auto a2 = static_cast<plPXActorData*>(pair.shapes[1]->getActor()->userData);
        if (!a1 || !a2)
            return;

        auto controller = a1->GetController() ? a1->GetController() : a2->GetController();
        auto phys = a1->GetPhysical() ? a1->GetPhysical() : a2->GetPhysical();
        if (!controller || !phys)
            return;

        physx::PxContactPairPoint contactPoints[32];
        physx::PxU32 nbContacts = pair.extractContacts(contactPoints, std::size(contactPoints));
        for (physx::PxU32 i = 0; i < nbContacts; ++i) {
            controller->AddContact(phys,
                                   plPXConvert::Point(contactPoints[i].position),
                                   plPXConvert::Vector(contactPoints[i].normal));
        }
    }

    void IHandlePhysicalContacts(const physx::PxContactPair& pair) const
    {
        auto a1 = static_cast<plPXActorData*>(pair.shapes[0]->getActor()->userData);
        auto a2 = static_cast<plPXActorData*>(pair.shapes[1]->getActor()->userData);
        if (!a1 || !a2)
            return;
        // Normally, these are always valid because the avatar (who doesn't have
        // a physical) will push other physicals away before they actually touch
        // his actor.  However, if the avatar is warped to a new position he may
        // collide with the object for a few frames.  We just ignore it.
        if (!a1->GetPhysical() || !a2->GetPhysical())
            return;

        auto sim = plSimulationMgr::GetInstance();
        sim->ConsiderSynch(a1->GetPhysical(), a2->GetPhysical());
        if (a1->GetPhysical()->GetSoundGroup() && a2->GetPhysical()->GetSoundGroup()) {
            // Just grab the first contact point and call it a day.
            physx::PxContactPairPoint contactPoint;
            physx::PxU32 nbContacts = pair.extractContacts(&contactPoint, 1);
            if (nbContacts == 1) {
                sim->AddContactSound(a1->GetPhysical(), a2->GetPhysical(),
                                        plPXConvert::Point(contactPoint.position),
                                        plPXConvert::Vector(contactPoint.normal));
            }
        }
    }

public:
    void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { }
    void onWake(physx::PxActor** actors, physx::PxU32 count) override { }
    void onSleep(physx::PxActor** actors, physx::PxU32 count) override { }

    void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs,
                   physx::PxU32 nbPairs) override
    {
        plProfile_BeginTiming(ContactCallback);
        for (physx::PxU32 i = 0; i < nbPairs; ++i) {
            IHandleControllerContacts(pairs[i]);
            IHandlePhysicalContacts(pairs[i]);
        }
        plProfile_EndTiming(ContactCallback);
    }

    void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
    {
        plProfile_BeginTiming(TriggerCallback);

        // Detector hits
        plSimulationMgr* mgr = plSimulationMgr::GetInstance();
        for (physx::PxU32 i = 0; i < count; ++i) {
            const physx::PxTriggerPair& pair = pairs[i];
            auto hittee = static_cast<plPXActorData*>(pair.triggerActor->userData);
            auto hitter = static_cast<plPXActorData*>(pair.otherActor->userData);
            if (!hittee || !hitter)
                continue;

            bool entering = pair.status & physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
            mgr->AddCollisionMsg(hittee->GetKey(), hitter->GetKey(), entering);
        }

        plProfile_EndTiming(TriggerCallback);
    }

    void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer,
                   const physx::PxU32 count) override { }

} static s_PxSimulationEvent;

// ==========================================================================

class plPXContactModifyHandler : public physx::PxContactModifyCallback
{
    void IHandleContact(physx::PxContactModifyPair& pair)
    {
        plProfile_IncCount(ContactModifyCount, pair.contacts.size());

        auto a1 = static_cast<plPXActorData*>(pair.actor[0]->userData);
        auto a2 = static_cast<plPXActorData*>(pair.actor[1]->userData);
        if (!a1 || !a2)
            return;

        // For now, only character on collider contacts should be handled here, so
        // there's no need for any other logic.
        auto controller = a1->GetController() ? a1->GetController() : a2->GetController();
        auto phys = a1->GetPhysical() ? a1->GetPhysical() : a2->GetPhysical();
        if (controller && phys)
            controller->ModifyContacts(phys, pair.contacts);
    }

public:
    void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 nbPairs) override
    {
        plProfile_BeginTiming(ContactModifyCallback);
        for (physx::PxU32 i = 0; i < nbPairs; ++i)
            IHandleContact(pairs[i]);
        plProfile_EndTiming(ContactModifyCallback);
    }
} s_PxContactModify;

// ==========================================================================

static inline plPXFilterData& IConvertFilterData(physx::PxFilterData& pxFilterData)
{
    return *((plPXFilterData*)&pxFilterData);
}

static inline bool ITestGroupPair(const plPXFilterData& f1, const plPXFilterData& f2,
                                  plSimDefs::Group g1, plSimDefs::Group g2)
{
    if ((f1.TestGroup(g1) && f2.TestGroup(g2)) || (f1.TestGroup(g2) && f2.TestGroup(g1)))
        return true;
    return false;
}

template<plSimDefs::Group _Group1, plSimDefs::Group _Group2>
static inline bool IFilter(const plPXFilterData& f1, const plPXFilterData& f2,
                           physx::PxPairFlags::InternalType reqPairFlags,
                           physx::PxPairFlags& finalPairFlags)
{
    if (ITestGroupPair(f1, f2, _Group1, _Group2)) {
        finalPairFlags = (physx::PxPairFlag::Enum)reqPairFlags;
        return true;
    }
    return false;
}

static physx::PxFilterFlags ISimulationFilterShader(physx::PxFilterObjectAttributes attributes0,
                                                    physx::PxFilterData filterData0,
                                                    physx::PxFilterObjectAttributes attributes1,
                                                    physx::PxFilterData filterData1,
                                                    physx::PxPairFlags& pairFlags,
                                                    const void* constantBlock,
                                                    physx::PxU32 constantBlockSize)
{
    plPXFilterData& filterHelper0 = IConvertFilterData(filterData0);
    plPXFilterData& filterHelper1 = IConvertFilterData(filterData1);

    // If one of the two objects is a trigger, check the colliding object's report group for a match.
    bool obj0trigger = physx::PxFilterObjectIsTrigger(attributes0);
    bool obj1trigger = physx::PxFilterObjectIsTrigger(attributes1);
    if (obj0trigger || obj1trigger) {
        plPXFilterData& triggerData = obj0trigger ? filterHelper0 : filterHelper1;
        plPXFilterData& otherData = obj0trigger ? filterHelper1 : filterHelper0;

        // Simplified testing: only avatars and dynamics can trigger
        if ((triggerData.TestReportOn(plSimDefs::kGroupDynamic) && otherData.TestGroup(plSimDefs::kGroupDynamic)) ||
            (triggerData.TestReportOn(plSimDefs::kGroupAvatar) && otherData.TestGroup(plSimDefs::kGroupAvatar))) {
            pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
            return physx::PxFilterFlag::eDEFAULT;
        }

        return physx::PxFilterFlag::eSUPPRESS;
    }

    const physx::PxPairFlags defFlags = physx::PxPairFlag::eCONTACT_DEFAULT |
                                        // for physics sounds
                                        physx::PxPairFlag::eNOTIFY_TOUCH_FOUND |
                                        physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS |
                                        physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
    const physx::PxPairFlags modFlags = defFlags |
                                        physx::PxPairFlag::eMODIFY_CONTACTS;
    const physx::PxFilterFlags defResult = physx::PxFilterFlag::eDEFAULT;
    const physx::PxFilterFlags cbResult = physx::PxFilterFlag::eCALLBACK;

#define FILTER(g1, g2, f, r) \
    if (IFilter<g1, g2>(filterHelper0, filterHelper1, f, pairFlags)) \
        return r;

    FILTER(plSimDefs::kGroupStatic, plSimDefs::kGroupDynamic, defFlags, defResult);
    FILTER(plSimDefs::kGroupDynamic, plSimDefs::kGroupDynamic, defFlags, defResult);
    FILTER(plSimDefs::kGroupDynamic, plSimDefs::kGroupDynamicBlocker, defFlags, defResult);

    // So the fire marbles don't fly through the Relto hut door...
    FILTER(plSimDefs::kGroupDynamic, plSimDefs::kGroupExcludeRegion, defFlags, defResult);

    // Avatars should not collide if they are disabled...
    if (filterHelper0.TestGroup(plSimDefs::kGroupAvatar) || filterHelper1.TestGroup(plSimDefs::kGroupAvatar)) {
        if ((filterHelper0.TestGroup(plSimDefs::kGroupAvatar) && filterHelper0.TestFlag(plPhysicalControllerCore::kDisableCollision)) ||
             (filterHelper1.TestGroup(plSimDefs::kGroupAvatar) && filterHelper1.TestFlag(plPhysicalControllerCore::kDisableCollision)))
             return physx::PxFilterFlag::eSUPPRESS;

        FILTER(plSimDefs::kGroupAvatar, plSimDefs::kGroupAvatarBlocker, modFlags, defResult);
        FILTER(plSimDefs::kGroupAvatar, plSimDefs::kGroupStatic, modFlags, defResult);
        FILTER(plSimDefs::kGroupAvatar, plSimDefs::kGroupDynamic, defFlags, defResult);
        FILTER(plSimDefs::kGroupAvatar, plSimDefs::kGroupExcludeRegion, defFlags, defResult);
    }

#undef FILTER
    return physx::PxFilterFlag::eSUPPRESS;
}

// ==========================================================================

plPXSimulation::plPXSimulation()
    : fPxFoundation(), fDebugger(), fTransport(), fPxPhysics(), fPxCooking(),
      fPxCpuDispatcher(), fAccumulator()
{
}

plPXSimulation::~plPXSimulation()
{
    // This should only run for the empty main world.
    for (const auto& world : fWorlds)
        world.second->release();
    fWorlds.clear();

    if (fPxCooking)
        fPxCooking->release();
    if (fPxCpuDispatcher)
        fPxCpuDispatcher->release();
    if (fPxPhysics)
        fPxPhysics->release();
    PxCloseExtensions();
    if (fDebugger)
        fDebugger->release();
    if (fTransport)
        fTransport->release();
    if (fPxFoundation)
        fPxFoundation->release();
}

// ==========================================================================

bool plPXSimulation::Init()
{
    plStatusLog::AddLineSF("Simulation.log", "Attempting to initialize PhysX {}.{}.{}",
                           PX_PHYSICS_VERSION_MAJOR,
                           PX_PHYSICS_VERSION_MINOR,
                           PX_PHYSICS_VERSION_BUGFIX);

    fPxFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gPxAllocator, gPxErrorCallback);
    if (!fPxFoundation) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX Foundation failed to initialize!");
        return false;
    }

#ifndef PLASMA_EXTERNAL_RELEASE
    fDebugger = PxCreatePvd(*fPxFoundation);
    if (fDebugger) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kGreen,
                              "PhysX Visual Debugger is available.\n"
                              "Recall that scene visualization will ONLY be available with PhysX "
                              "SDK builds in the checked or debug configurations");
        ConnectDebugger();
    } else {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kYellow,
                              "PhysX Visual Debugger is NOT available.");
    }
#endif

    physx::PxTolerancesScale scale;
    scale.length = kToleranceScaleLength;
    scale.speed = kToleranceScaleSpeed;

    // Leaves out features that URU will never use
    fPxPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *fPxFoundation, scale, false, fDebugger);
    if (!fPxPhysics) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX failed to initialize!");
        return false;
    }

    if (!PxInitExtensions(*fPxPhysics, fDebugger)) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX Extension failed to initialize!");
        return false;
    }

    // Worker threads actually slow down our simulation - probably because Uru scenes are mostly
    // composed of static geometry, so the thread synchronization adds more overhead than the
    // threads help.
    fPxCpuDispatcher = physx::PxDefaultCpuDispatcherCreate(0);
    if (!fPxCpuDispatcher) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX CPU Dispatcher failed to initialize!");
        return false;
    }

    physx::PxCookingParams params(scale);
    // disable mesh cleaning - perform mesh validation on development configurations
    params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
    // disable edge precompute, edges are set for each triangle, slows contact generation
    params.meshPreprocessParams |= physx::PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;

    fPxCooking = PxCreateCooking(PX_PHYSICS_VERSION, *fPxFoundation, params);
    if (!fPxCooking) {
        plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX Cooking failed to initialize!");
        return false;
    }

    // Purposefully create AND LEAK the default material so it's always the first one we check.
    // In most Cyan Ages, this is the one and only material. This material will be destroyed by
    // fPxPhysics->release() in the dtor.
    fPxPhysics->createMaterial(0.5f, 0.5f, 0.5f);

    return true;
}

// ==========================================================================

static plFileName s_defaultDebuggerEndpoint;

void plPXSimulation::SetDefaultDebuggerEndpoint(plFileName endpoint)
{
    s_defaultDebuggerEndpoint = std::move(endpoint);
}

bool plPXSimulation::IConnectDebugger(physx::PxPvdTransport* transport)
{
    std::swap(transport, fTransport);
    bool retval = fDebugger->connect(*fTransport, physx::PxPvdInstrumentationFlag::eALL);
    if (transport)
        transport->release();
    return retval;
}

plPXDebuggerStatus plPXSimulation::ConnectDebugger(std::optional<plFileName> requestedEp)
{
    if (!fDebugger)
        return plPXDebuggerStatus::kNotSupported;

    plFileName endpoint = requestedEp ? requestedEp.value() : s_defaultDebuggerEndpoint;
    if (endpoint.IsValid()) {
        if (endpoint.GetFileExt().compare_i("pxd2") != 0)
            endpoint += ".pxd2";
        auto transport = physx::PxDefaultPvdFileTransportCreate(endpoint.AsString().c_str());
        if (IConnectDebugger(transport)) {
            plStatusLog::AddLineSF("Simulation.log", plStatusLog::kGreen,
                                   "PhysX Visual Debugger logging to file: {}", endpoint);
            return plPXDebuggerStatus::kConnectedToFile;
        }
    } else {
        auto transport = physx::PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        if (IConnectDebugger(transport)) {
            plStatusLog::AddLineS("Simulation.log", plStatusLog::kGreen,
                                   "PhysX Visual Debugger logging over TCP");
            return plPXDebuggerStatus::kConnectedToTcp;
        }
    }

    plStatusLog::AddLineS("Simulation.log", plStatusLog::kRed, "PhysX Visual Debugger connection failed.");
    return plPXDebuggerStatus::kConnectionFailed;
}

plPXDebuggerStatus plPXSimulation::DisconnectDebugger()
{
    if (!fDebugger)
        return plPXDebuggerStatus::kNotSupported;
    if (auto transport = fDebugger->getTransport())
        transport->disconnect();
    return plPXDebuggerStatus::kDisconnected;
}

bool plPXSimulation::IsDebuggerConnected() const
{
    if (!fDebugger)
        return false;
    return fDebugger->isConnected(false);
}

// ==========================================================================

physx::PxConvexMesh* plPXSimulation::InsertConvexHull(const std::vector<uint32_t>& tris,
                                                      const std::vector<hsPoint3>& verts)
{
    physx::PxConvexMeshDesc desc;
    desc.indices.count = tris.size();
    desc.indices.stride = sizeof(uint32_t);
    desc.indices.data = tris.empty() ? nullptr : &tris[0];
    desc.points.count = verts.size();
    desc.points.stride = sizeof(hsPoint3);
    desc.points.data = &verts[0];
    desc.flags = physx::PxConvexFlag::eDISABLE_MESH_VALIDATION |
                 physx::PxConvexFlag::eFAST_INERTIA_COMPUTATION;
    if (tris.empty())
        desc.flags |= physx::PxConvexFlag::eCOMPUTE_CONVEX;

    return fPxCooking->createConvexMesh(desc, fPxPhysics->getPhysicsInsertionCallback());
}

physx::PxTriangleMesh* plPXSimulation::InsertTriangleMesh(const std::vector<uint32_t>& tris,
                                                          const std::vector<hsPoint3>& verts)
{
    physx::PxTriangleMeshDesc desc;
    desc.points.count = verts.size();
    desc.points.stride = sizeof(hsPoint3);
    desc.points.data = &verts[0];
    desc.triangles.count = tris.size() / 3;
    desc.triangles.stride = sizeof(uint32_t) * 3;
    desc.triangles.data = &tris[0];

    return fPxCooking->createTriangleMesh(desc, fPxPhysics->getPhysicsInsertionCallback());
}

physx::PxRigidActor* plPXSimulation::CreateRigidActor(const physx::PxGeometry& geometry,
                                                      const physx::PxTransform& globalPose,
                                                      const physx::PxTransform& localPose,
                                                      float uStatic, float uDynamic, float restitution,
                                                      plPXActorType type)
{
    physx::PxRigidActor* actor;
    switch (type) {
    case plPXActorType::kStaticActor:
        actor = fPxPhysics->createRigidStatic(globalPose);
        break;
    case plPXActorType::kDynamicActor:
        {
            physx::PxRigidDynamic* dynamic = fPxPhysics->createRigidDynamic(globalPose);
            dynamic->setMaxDepenetrationVelocity(kToleranceScaleSpeed);
            actor = dynamic;
            break;
        }
    case plPXActorType::kKinematicActor:
        {
            physx::PxRigidDynamic* dynamic = fPxPhysics->createRigidDynamic(globalPose);
            dynamic->setRigidBodyFlag(physx::PxRigidBodyFlag::eKINEMATIC, true);
            actor = dynamic;
        }
        break;
    DEFAULT_FATAL(type);
    }

    physx::PxMaterial* material = InitMaterial(uStatic, uDynamic, restitution);
    physx::PxShape* shape = fPxPhysics->createShape(geometry, *material, true);
    shape->setLocalPose(localPose);
    actor->attachShape(*shape);
    return actor;
}

// ==========================================================================

physx::PxScene* plPXSimulation::InitSubworld(const plKey& world)
{
    plStatusLog::AddLineSF("Simulation.log", plStatusLog::kGreen,
                           "Initializing world '{}'",
                           world ? world->GetUoid().StringIze() : "(main world)");

    // The world key is assumed to be loaded (or null for main world) if we are here.
    // As such, let us grab the plSceneObject's PXSubWorld definition to figure out
    // what gravity should look like. Who knows, we might be in MC Escher land...
    physx::PxVec3 gravity(X_GRAVITY, Y_GRAVITY, Z_GRAVITY);
    if (world) {
        if (plSceneObject* so = plSceneObject::ConvertNoRef(world->VerifyLoaded())) {
            if (plPXSubWorld* subworld = plPXSubWorld::ConvertNoRef(so->GetGenericInterface(plPXSubWorld::Index()))) {
                gravity = plPXConvert::Vector(subworld->GetGravity());
            }
        }
    }

    physx::PxTolerancesScale scale;
    scale.length = kToleranceScaleLength;
    scale.speed = kToleranceScaleSpeed;

    physx::PxSceneDesc desc(scale);
    desc.gravity = gravity;
    desc.simulationEventCallback = &s_PxSimulationEvent;
    desc.contactModifyCallback = &s_PxContactModify;
    desc.filterShader = ISimulationFilterShader;
    desc.frictionType = physx::PxFrictionType::eTWO_DIRECTIONAL;
    desc.solverType = physx::PxSolverType::eTGS;
    desc.flags = physx::PxSceneFlag::eENABLE_PCM |
                 physx::PxSceneFlag::eENABLE_AVERAGE_POINT;
    desc.cpuDispatcher = fPxCpuDispatcher;
    desc.userData = world ? world->ObjectIsLoaded() : nullptr;

    physx::PxScene* scene = fPxPhysics->createScene(desc);
#ifndef PLASMA_EXTERNAL_RELEASE
    if (auto pvd = scene->getScenePvdClient()) {
        pvd->setScenePvdFlags(physx::PxPvdSceneFlag::eTRANSMIT_CONTACTS |
                              physx::PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES);
    }
#endif

    auto it = fWorlds.try_emplace(world, scene);
    return it.first->second;
}

void plPXSimulation::ReleaseSubworld(const hsKeyedObject* world)
{
    // Optimization: don't release the main world
    if (!world)
        return;

    auto it = fWorlds.find(world->GetKey());
    hsAssert(it != fWorlds.end(), "Releasing a nonextant subworld, eh?");

    if (it != fWorlds.end()) {
        if (it->second->getNbActors(physx::PxActorTypeFlag::eRIGID_DYNAMIC |
                                    physx::PxActorTypeFlag::eRIGID_STATIC) == 0) {

            plStatusLog::AddLineSF("Simulation.log", plStatusLog::kGreen,
                                   "Releasing world '{}'",
                                   world ? world->GetKey()->GetUoid().StringIze() : "(main world)");
            it->second->release();
            fWorlds.erase(it);
        }
    }
}

// ==========================================================================

physx::PxMaterial* plPXSimulation::InitMaterial(float uStatic, float uDynamic, float restitution)
{
    physx::PxMaterial* materials[128];
    for (physx::PxU32 i = 0; i < fPxPhysics->getNbMaterials();) {
        physx::PxU32 count = fPxPhysics->getMaterials(materials, std::size(materials), i);
        for (physx::PxU32 j = 0; j < count; ++j) {
            if (materials[j]->getStaticFriction() == uStatic &&
                materials[j]->getDynamicFriction() == uDynamic &&
                materials[j]->getRestitution() == restitution)
                return materials[j];
        }
        i += count;
    }

    return fPxPhysics->createMaterial(uStatic, uDynamic, restitution);
}

// ==========================================================================

void plPXSimulation::AddToWorld(physx::PxActor* actor, const plKey& world)
{
    actor->setName(static_cast<plPXActorData*>(actor->userData)->c_str());
    if (physx::PxScene* scene = actor->getScene()) {
        scene->removeActor(*actor);
        ReleaseSubworld((hsKeyedObject*)scene->userData);
    }

    physx::PxScene* scene;
    auto it = fWorlds.find(world);
    if (it == fWorlds.end()) {
        scene = InitSubworld(world);
    } else {
        scene = it->second;
    }

    scene->addActor(*actor);
}

physx::PxScene* plPXSimulation::FindScene(const plKey& world)
{
    auto it = fWorlds.find(world);
    if (it != fWorlds.end())
        return it->second;
    return nullptr;
}

void plPXSimulation::RemoveFromWorld(physx::PxRigidActor* actor)
{
    physx::PxScene* scene = actor->getScene();
    hsAssert(scene, "actor not in a scene");

    scene->removeActor(*actor);
    ReleaseSubworld((hsKeyedObject*)scene->userData);

    // Implicitly releases all actor shapes
    actor->release();
}

// ==========================================================================

bool plPXSimulation::Advance(float delta)
{
    fAccumulator += delta;
    if (fAccumulator < kDefaultStepSize) {
        // Not enough time has passed to perform a physics substep, but we need to propagate
        // corrected+interpolated controller movement.
        plProfile_BeginTiming(CorrectController);
        plPXPhysicalControllerCore::UpdateNonPhysical(fAccumulator / kDefaultStepSize);
        plProfile_EndTiming(CorrectController);
        return false;
    } else if (fAccumulator > kDefaultMaxDelta) {
        fAccumulator = kDefaultMaxDelta;
    }

    // Perform as many whole substeps as possible saving the remainder in our accumulator.
    int numSubSteps = (int)(fAccumulator / kDefaultStepSize + 0.000001f);
    delta = numSubSteps * kDefaultStepSize;
    fAccumulator -= delta;

    // Avatars have pre-baked movements defined by artist made animations, however, the final
    // motion of the avatar needs to take into account things like friction from the ground
    // and gravity. So, avatars have to be handled in three stages. We first apply the animation.
    // Then, we run the simulation with the velocities determined from the animations. Finally,
    // the results of the simulation are sent out as corrections.
    plProfile_BeginTiming(ApplyController);
    plPXPhysicalControllerCore::Apply(delta);
    plProfile_EndTiming(ApplyController);

    plProfile_BeginTiming(Step);
    for (auto& it : fWorlds) {
        it.second->simulate(delta);
        it.second->fetchResults(true);

        physx::PxSimulationStatistics stats;
        it.second->getSimulationStatistics(stats);
        plProfile_IncCount(ActiveBodies, stats.nbActiveDynamicBodies + stats.nbActiveDynamicBodies);
        plProfile_IncCount(ActiveDynamics, stats.nbActiveDynamicBodies);
        plProfile_IncCount(ActiveKinematics, stats.nbActiveKinematicBodies);
        plProfile_IncCount(TotalBodies, stats.nbDynamicBodies + stats.nbKinematicBodies + stats.nbStaticBodies);
        plProfile_IncCount(Dynamics, stats.nbDynamicBodies);
        plProfile_IncCount(Kinematics, stats.nbKinematicBodies);
        plProfile_IncCount(Statics, stats.nbStaticBodies);
    }
    plProfile_EndTiming(Step);

    // Propagate the simulated controller movement to the SceneObjects for rendering purposes.
    plProfile_BeginTiming(CorrectController);
    plPXPhysicalControllerCore::Update(numSubSteps, fAccumulator / kDefaultStepSize);
    plProfile_EndTiming(CorrectController);

    return true;
}
