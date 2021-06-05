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
#include "plPXPhysical.h"
#include "plPXConvert.h"
#include "plPXCooking.h"
#include "plPXPhysicalControllerCore.h"
#include "plPhysXAPI.h"
#include "plPXSimDefs.h"
#include "plPXSimulation.h"
#include "plSimulationMgr.h"

#include "plProfile.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsQuat.h"

#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plDrawable/plDrawableGenerator.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"

// ==========================================================================

/**
 * RAII Simulation Enable Toggle for PhysX Actors
 * \remarks Some actor mutators, such as those for pose and velocity require that the actor
 *          be simulating when they are used. While this does make sense from some perspectives,
 *          (why are you changing the velocity of a disabled actor, moron?), in our event-driven
 *          architecture, we would like to handle these kinds of things immediately.
 */
class plPXActorSimulationLock
{
    physx::PxRigidActor* fActor;
    bool fDisabled;

public:
    plPXActorSimulationLock() = delete;
    plPXActorSimulationLock(const plPXActorSimulationLock&) = delete;
    plPXActorSimulationLock(plPXActorSimulationLock&&) = delete;

    plPXActorSimulationLock(physx::PxRigidActor* actor)
        : fActor(actor),
          fDisabled(actor->getActorFlags().isSet(physx::PxActorFlag::eDISABLE_SIMULATION))
    {
        if (fDisabled)
            actor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, false);
    }

    ~plPXActorSimulationLock()
    {
        if (fDisabled)
            fActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, true);
    }
};

// ==========================================================================

bool plPXPhysical::IsKinematic() const
{
    return fRecipe.mass != 0.f && (fGroup != plSimDefs::kGroupDynamic ||
                                   GetProperty(plSimulationInterface::kPhysAnim));
}

bool plPXPhysical::IsDynamic() const
{
    return fRecipe.mass != 0.f && (fGroup == plSimDefs::kGroupDynamic &&
                                   !GetProperty(plSimulationInterface::kPhysAnim));
}

bool plPXPhysical::IsStatic() const
{
    return fRecipe.mass == 0.f && fGroup != plSimDefs::kGroupDynamic &&
           !GetProperty(plSimulationInterface::kPhysAnim);
}

bool plPXPhysical::IsTrigger() const
{
    return fGroup == plSimDefs::kGroupDetector;
}

// ==========================================================================

void plPXPhysical::IUpdateShapeFlags()
{
    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);

    // Don't set the flags in place -- you might cause a fatal error in the SDK due to some actor
    // changing groups (eg exclude regions)
    physx::PxShapeFlags flags = physx::PxShapeFlag::eSCENE_QUERY_SHAPE;
    if (IsTrigger())
        flags |= physx::PxShapeFlag::eTRIGGER_SHAPE;
    else
        flags |= physx::PxShapeFlag::eSIMULATION_SHAPE;

    if (!GetProperty(plSimulationInterface::kDisable))
        flags |= physx::PxShapeFlag::eVISUALIZATION;
    shape->setFlags(flags);
}

void plPXPhysical::ISyncFilterData()
{
    if (fActor)
        plPXFilterData::Initialize(fActor, fGroup, fReportsOn, (plSimDefs::plLOSDB)fLOSDBs);
}

// ==========================================================================

static inline void IForceNonzero(float& component, const char* name, const plKey& key)
{
    if (component == 0.f) {
        plSimulationMgr::LogYellow("WARNING: '{}' has a zero {} component", key->GetName(), name);
        component = 0.0001f;
    }
}

void plPXPhysical::ISanityCheckGeometry(physx::PxBoxGeometry& geometry) const
{
    IForceNonzero(geometry.halfExtents.x, "x", fObjectKey);
    IForceNonzero(geometry.halfExtents.y, "y", fObjectKey);
    IForceNonzero(geometry.halfExtents.z, "z", fObjectKey);
}

void plPXPhysical::ISanityCheckGeometry(physx::PxSphereGeometry& geometry) const
{
    IForceNonzero(geometry.radius, "radius", fObjectKey);
}

void plPXPhysical::ISanityCheckBounds()
{
    // PhysX 4.1 cannot handle dynamic triangle meshes, so we force these to be hulls. Sad.
    switch (fBounds) {
    case plSimDefs::kProxyBounds:
    case plSimDefs::kExplicitBounds:
        if (IsDynamic()) {
            plSimulationMgr::LogYellow("WARNING: '{}' is a dynamic triangle mesh; this is not "
                                       "supported in PhysX 4... forcing to convex hull, sorry.",
                                       GetKeyName());
            fBounds = plSimDefs::kHullBounds;
        }
        break;

    default:
        break;
    }
}

void plPXPhysical::ISanityCheckRecipe()
{
    hsQuat& rot = fRecipe.l2sQ;
    if (rot.fX == 0.f && rot.fY == 0.f && rot.fZ == 0.f && rot.fW == 0.f)
        rot.fW = 1.f;
}

// ==========================================================================

bool plPXPhysical::InitActor()
{
    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();

    plPXActorType actorType = plPXActorType::kUnset;
    if (IsStatic())
        actorType = plPXActorType::kStaticActor;
    if (IsKinematic())
        actorType = plPXActorType::kKinematicActor;
    if (IsDynamic())
        actorType = plPXActorType::kDynamicActor;
    physx::PxTransform globalPose = plPXConvert::Transform(fRecipe.l2sP, fRecipe.l2sQ);

    // Reminder: fRecipe.bounds represents what the artist wanted. This may
    // be adjusted by our smarter code such that you do not have the shape
    // described by fRecipe.bounds after a read-in. Use fBounds.
    switch (fBounds) {
    case plSimDefs::kBoxBounds:
    {
        physx::PxBoxGeometry geometry(plPXConvert::Point(fRecipe.bDimensions));
        ISanityCheckGeometry(geometry);
        physx::PxTransform localPose(plPXConvert::Point(fRecipe.bOffset));
        fActor = sim->CreateRigidActor(geometry, globalPose, localPose,
                                       fRecipe.friction, fRecipe.friction, fRecipe.restitution,
                                       actorType);
    }
    break;

    case plSimDefs::kSphereBounds:
    {
        physx::PxSphereGeometry geometry(fRecipe.radius);
        ISanityCheckGeometry(geometry);
        physx::PxTransform localPose(plPXConvert::Point(fRecipe.offset));
        fActor = sim->CreateRigidActor(geometry, globalPose, localPose,
                                       fRecipe.friction, fRecipe.friction, fRecipe.restitution,
                                       actorType);
    }
    break;

    case plSimDefs::kHullBounds:
    {
        physx::PxConvexMeshGeometry geometry(fRecipe.convexMesh);
        geometry.meshFlags.set(physx::PxConvexMeshGeometryFlag::eTIGHT_BOUNDS);
        physx::PxTransform localPose(physx::PxIdentity);
        fActor = sim->CreateRigidActor(geometry, globalPose, localPose,
                                       fRecipe.friction, fRecipe.friction, fRecipe.restitution,
                                       actorType);
    }
    break;

    case plSimDefs::kProxyBounds:
    case plSimDefs::kExplicitBounds:
    {
        physx::PxTriangleMeshGeometry geometry(fRecipe.triMesh);
        physx::PxTransform localPose(physx::PxIdentity);
        fActor = sim->CreateRigidActor(geometry, globalPose, localPose,
                                       fRecipe.friction, fRecipe.friction, fRecipe.restitution,
                                       actorType);
    }
    break;

    DEFAULT_FATAL(fBounds)
    }

    fActor->userData = new plPXActorData(this);
    IUpdateShapeFlags();
    ISyncFilterData();
    sim->AddToWorld(fActor, fWorldKey);
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        if (!dynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
            physx::PxRigidBodyExt::setMassAndUpdateInertia(*dynamic, fRecipe.mass);
            if (GetProperty(plSimulationInterface::kStartInactive))
                dynamic->putToSleep();
        }
    }

    InitRefs();

    // only dynamic physicals without noSync need SDLs
    if (IsDynamic() && !GetProperty(plSimulationInterface::kNoSynchronize))
        InitSDL();

    return true;
}

void plPXPhysical::IUpdateSubworld()
{
    if (fActor)
        plSimulationMgr::GetInstance()->GetPhysX()->AddToWorld(fActor, fWorldKey);
}

void plPXPhysical::DestroyActor()
{
    if (fActor) {
        // When the actor is removed from the world, it eventually receives eNOTIFY_TOUCH_LOST
        // after the keyed objects are destroyed but before the PhysX SDK destroys the actor.
        // So, we null out the Plasma data.
        delete static_cast<plPXActorData*>(fActor->userData);
        fActor->userData = nullptr;

        plSimulationMgr::GetInstance()->GetPhysX()->RemoveFromWorld(fActor);
        fActor = nullptr;
    }

    if (fRecipe.triMesh) {
        fRecipe.triMesh->release();
        fRecipe.triMesh = nullptr;
    }

    if (fRecipe.convexMesh) {
        fRecipe.convexMesh->release();
        fRecipe.convexMesh = nullptr;
    }
}

// ==========================================================================

void plPXPhysical::IEnable(bool enable)
{
    fProps.SetBit(plSimulationInterface::kDisable, !enable);
    fActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, !enable);
    IUpdateShapeFlags();
}

plPhysical& plPXPhysical::SetProperty(int prop, bool status)
{
    if (GetProperty(prop) == status)
    {
        const char* propName = "(unknown)";
        switch (prop)
        {
        case plSimulationInterface::kDisable:           propName = "kDisable";              break;
        case plSimulationInterface::kPinned:            propName = "kPinned";               break;
        case plSimulationInterface::kPassive:           propName = "kPassive";              break;
        case plSimulationInterface::kPhysAnim:          propName = "kPhysAnim";             break;
        case plSimulationInterface::kStartInactive:     propName = "kStartInactive";        break;
        case plSimulationInterface::kNoSynchronize:     propName = "kNoSynchronize";        break;
        }

        ST::string name = ST_LITERAL("(unknown)");
        if (GetKey())
            name = GetKeyName();
        if (plSimulationMgr::fExtraProfile)
            plSimulationMgr::Log("Warning: Redundant physical property set (property {}, value {}) on {}", propName, status ? "true" : "false", name);
    }

    switch (prop)
    {
    case plSimulationInterface::kDisable:
        IEnable(!status);
        break;

    case plSimulationInterface::kPinned:
        auto dynamic = fActor->is<physx::PxRigidDynamic>();
        if (dynamic && !dynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
            if (status) {
                dynamic->setMaxAngularVelocity(0.f);
                dynamic->setMaxLinearVelocity(0.f);
            } else {
                dynamic->setMaxAngularVelocity(100.f);
                dynamic->setMaxLinearVelocity(PX_MAX_F32);
            }
        }
        break;
    }

    fProps.SetBit(prop, status);

    return *this;
}

bool plPXPhysical::CanSynchPosition(bool isSynchUpdate) const
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>())
        return !dynamic->isSleeping() || isSynchUpdate;
    return false;
}

void plPXPhysical::ISetTransformGlobal(const hsMatrix44& l2w)
{
    physx::PxTransform pose;
    if (fWorldKey) {
        plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
        hsAssert(so, "Scene object not loaded while accessing subworld.");
        // physical to subworld (simulation space)
        hsMatrix44 p2s = so->GetCoordinateInterface()->GetWorldToLocal() * l2w;
        pose = plPXConvert::Transform(p2s);
        IMoveProxy(p2s);
    } else {
        // No need to localize
        pose = plPXConvert::Transform(l2w);
        IMoveProxy(l2w);
    }

    {
        plPXActorSimulationLock lock(fActor);
        auto dynamic = fActor->is<physx::PxRigidDynamic>();
        if (dynamic && dynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC))
            dynamic->setKinematicTarget(pose);
        else
            fActor->setGlobalPose(pose);
    }
}

// the physical may have several parents between it and the subworld object,
// but the *havok* transform is only one level away from the subworld.
// to avoid any confusion about this difference, we avoid referring to the 
// subworld as "parent" and use, for example, "l2s" (local-to-sub) instead
// of the canonical plasma "l2p" (local-to-parent)
void plPXPhysical::IGetTransformGlobal(hsMatrix44& l2w) const
{
    l2w = plPXConvert::Transform(fActor->getGlobalPose());

    if (fWorldKey) {
        plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
        hsAssert(so, "Scene object not loaded while accessing subworld.");
        if (so->GetCoordinateInterface()) {
             const hsMatrix44& s2w = so->GetCoordinateInterface()->GetLocalToWorld();
             l2w = s2w * l2w;
        }
    }
}

void plPXPhysical::IGetPoseSim(hsPoint3& pos, hsQuat& rot) const
{
    physx::PxTransform pose = fActor->getGlobalPose();
    pos = plPXConvert::Point(pose.p);
    rot = plPXConvert::Quat(pose.q);
}

void plPXPhysical::ISetPoseSim(const hsPoint3* pos, const hsQuat* rot, bool wakeup)
{
    physx::PxTransform pose = fActor->getGlobalPose();
    if (pos)
        pose.p = plPXConvert::Point(*pos);
    if (rot)
        pose.q = plPXConvert::Quat(*rot);

    {
        plPXActorSimulationLock lock(fActor);
        auto dynamic = fActor->is<physx::PxRigidDynamic>();
        if (dynamic && dynamic->getRigidBodyFlags().isSet(physx::PxRigidBodyFlag::eKINEMATIC)) {
            dynamic->setKinematicTarget(pose);
        } else {
            if (fActor->is<physx::PxRigidStatic>())
                plStatusLog::AddLineSF("Simulation.log", plStatusLog::kYellow,
                                       "Warning: moving static actor '{}'", GetKeyName());
            fActor->setGlobalPose(pose, wakeup);
        }
    }
}

bool plPXPhysical::GetLinearVelocitySim(hsVector3& vel) const
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        vel = plPXConvert::Vector(dynamic->getLinearVelocity());
        return true;
    } else {
        vel.Set(0.f, 0.f, 0.f);
        return false;
    }
}

void plPXPhysical::SetLinearVelocitySim(const hsVector3& vel, bool wakeup)
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        plPXActorSimulationLock lock(fActor);
        dynamic->setLinearVelocity(plPXConvert::Vector(vel), wakeup);
    }
}

void plPXPhysical::ClearLinearVelocity()
{
    SetLinearVelocitySim({});
}

bool plPXPhysical::GetAngularVelocitySim(hsVector3& vel) const
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        vel = plPXConvert::Vector(dynamic->getAngularVelocity());
        return true;
    } else {
        vel.Set(0.f, 0.f, 0.f);
        return false;
    }
}

void plPXPhysical::SetAngularVelocitySim(const hsVector3& vel, bool wakeup)
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        plPXActorSimulationLock lock(fActor);
        dynamic->setAngularVelocity(plPXConvert::Vector(vel), wakeup);
    }
}

void plPXPhysical::SetImpulseSim(const hsVector3& imp, bool wakeup)
{
    if (auto dynamic = fActor->is<physx::PxRigidDynamic>()) {
        plPXActorSimulationLock lock(fActor);
        dynamic->addForce(plPXConvert::Vector(imp), physx::PxForceMode::eIMPULSE, wakeup);
    }
}

// ==========================================================================

physx::PxConvexMesh* plPXPhysical::ICookHull(hsStream* s)
{
    std::vector<uint32_t> tris;
    std::vector<hsPoint3> verts;

    switch (fRecipe.bounds) {
    case plSimDefs::kHullBounds:
        try {
            plPXCooking::ReadConvexHull26(s, tris, verts);
        } catch (const plPXCookingException& ex) {
            SimLog("Failed to uncook convex hull '{}': {}", GetKeyName(), ex.what());
            return nullptr;
        }
        break;

    case plSimDefs::kExplicitBounds:
    case plSimDefs::kProxyBounds:
        try {
            plPXCooking::ReadTriMesh26(s, tris, verts);
        } catch (const plPXCookingException& ex) {
            SimLog("Failed to uncook triangle mesh (for hull bounds) '{}': {}", GetKeyName(), ex.what());
            return nullptr;
        }

        // Forces PhysX to compute a hull
        tris.clear();
        break;

    DEFAULT_FATAL(fRecipe.bounds);
    }

    return plSimulationMgr::GetInstance()->GetPhysX()->InsertConvexHull(tris, verts);
}

physx::PxTriangleMesh* plPXPhysical::ICookTriMesh(hsStream* s)
{
    std::vector<uint32_t> tris;
    std::vector<hsPoint3> verts;

    switch (fRecipe.bounds) {
    case plSimDefs::kExplicitBounds:
    case plSimDefs::kProxyBounds:
        try {
             plPXCooking::ReadTriMesh26(s, tris, verts);
        } catch (const plPXCookingException& ex) {
            SimLog("Failed to uncook triangle mesh '{}': {}", GetKeyName(), ex.what());
            return nullptr;
        }
        break;

    DEFAULT_FATAL(fRecipe.bounds);
    }

    return plSimulationMgr::GetInstance()->GetPhysX()->InsertTriangleMesh(tris, verts);
}

// ==========================================================================

static plDrawableSpans* IGenerateProxy(plDrawableSpans* drawable,
                                       std::vector<uint32_t>& idx,
                                       const physx::PxShape* shape,
                                       const physx::PxBoxGeometry& geometry,
                                       const hsMatrix44& l2w, hsGMaterial* mat, bool blended)
{
    hsPoint3 dim = plPXConvert::Point(geometry.halfExtents);
    return plDrawableGenerator::GenerateBoxDrawable(dim.fX * 2.f, dim.fY * 2.f, dim.fZ * 2.f,
                                                    mat, l2w, blended, nullptr, &idx, drawable);
}

static plDrawableSpans* IGenerateProxy(plDrawableSpans* drawable,
                                       std::vector<uint32_t>& idx,
                                       const physx::PxShape* shape,
                                       const physx::PxConvexMeshGeometry& geometry,
                                       const hsMatrix44& l2w, hsGMaterial* mat, bool blended)
{
    std::vector<hsPoint3> verts(geometry.convexMesh->getNbVertices());
    memcpy(verts.data(), geometry.convexMesh->getVertices(),
           verts.size() * sizeof(decltype(verts)::value_type));

    std::vector<uint16_t> tris;
    for (physx::PxU32 i = 0; i < geometry.convexMesh->getNbPolygons(); ++i) {
        physx::PxHullPolygon polygon;
        if (!geometry.convexMesh->getPolygonData(i, polygon))
            continue;
        const physx::PxU8* indices = geometry.convexMesh->getIndexBuffer();
        indices += polygon.mIndexBase;

        switch (polygon.mNbVerts) {
        case 1:
        case 2:
            hsAssert(0, "what the triangle?");
            break;

        case 3:
            tris.push_back(*indices++);
            tris.push_back(*indices++);
            tris.push_back(*indices++);
            break;

        default:
            {
                // I suck at 3d math, so let's just draw a line from the center of the face to each vert,
                // complete with either the next one or the first one. If you don't like it, write
                // your own fucking ngon algorithm. As for me, this is what you get. Use the
                // damn PhysX Visual Debugger if it really bothers you.
                physx::PxVec3 center(physx::PxZero);
                const physx::PxU8* ptr = indices;
                const physx::PxU8* end = indices + (polygon.mNbVerts * sizeof(physx::PxU8));
                do {
                    center += geometry.convexMesh->getVertices()[*ptr++];
                } while (ptr < end);
                center /= polygon.mNbVerts;

                hsAssert(verts.size() < std::numeric_limits<uint16_t>::max(), "Too many verts");
                uint16_t centerIdx = (uint16_t)verts.size();
                verts.push_back(plPXConvert::Point(center));

                ptr = indices;
                do {
                    tris.push_back(centerIdx);
                    tris.push_back(*ptr++);
                    tris.push_back(ptr < end ? *ptr : *indices);
                } while (ptr < end);
            }
            break;
        }
    }

    return plDrawableGenerator::GenerateDrawable(verts.size(), verts.data(), nullptr, nullptr, 0,
                                                 nullptr, true, nullptr, tris.size(), tris.data(),
                                                 mat, l2w, blended, &idx, drawable);
}

static plDrawableSpans* IGenerateProxy(plDrawableSpans* drawable,
                                       std::vector<uint32_t>& idx,
                                       const physx::PxShape* shape,
                                       const physx::PxSphereGeometry& geometry,
                                       const hsMatrix44& l2w, hsGMaterial* mat, bool blended)
{
    hsPoint3 pos = plPXConvert::Point(shape->getLocalPose().p);
    return plDrawableGenerator::GenerateSphericalDrawable(pos, geometry.radius, mat, l2w, blended,
                                                          nullptr, &idx, drawable);
}

static plDrawableSpans* IGenerateProxy(plDrawableSpans* drawable,
                                       std::vector<uint32_t>& idx,
                                       const physx::PxShape* shape,
                                       const physx::PxTriangleMeshGeometry& geometry,
                                       const hsMatrix44& l2w, hsGMaterial* mat, bool blended)
{
    uint16_t idxCount = (uint16_t)std::min(geometry.triangleMesh->getNbTriangles() * 3, (physx::PxU32)UINT16_MAX);
    std::vector<uint16_t> tris(idxCount);
    if (geometry.triangleMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES)) {
        memcpy(tris.data(), geometry.triangleMesh->getTriangles(), sizeof(physx::PxU16) * idxCount);
    } else {
        const physx::PxU32* data = (const physx::PxU32*)geometry.triangleMesh->getTriangles();
        for (physx::PxU32 i = 0; i < idxCount; ++i)
            tris[i] = data[i];
    }

    return plDrawableGenerator::GenerateDrawable(geometry.triangleMesh->getNbVertices(),
                                                 const_cast<hsPoint3*>(reinterpret_cast<const hsPoint3*>(geometry.triangleMesh->getVertices())),
                                                 nullptr, nullptr, 0,
                                                 nullptr, true, nullptr,
                                                 tris.size(), tris.data(),
                                                 mat, l2w, blended, &idx, drawable);
}

plDrawableSpans* plPXPhysical::CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo)
{
    plDrawableSpans* myDraw = addTo;
    hsMatrix44 l2w, unused;
    GetTransform(l2w, unused);

    bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);
    switch (shape->getGeometryType()) {
    case physx::PxGeometryType::eBOX:
    {
        physx::PxBoxGeometry geometry;
        shape->getBoxGeometry(geometry);
        return IGenerateProxy(addTo, idx, shape, geometry, l2w, mat, blended);
    }

    case physx::PxGeometryType::eCONVEXMESH:
    {
        physx::PxConvexMeshGeometry geometry;
        shape->getConvexMeshGeometry(geometry);
        return IGenerateProxy(addTo, idx, shape, geometry, l2w, mat, blended);
    }

    case physx::PxGeometryType::eSPHERE:
    {
        physx::PxSphereGeometry geometry;
        shape->getSphereGeometry(geometry);
        return IGenerateProxy(addTo, idx, shape, geometry, l2w, mat, blended);
    }

    case physx::PxGeometryType::eTRIANGLEMESH:
    {
        physx::PxTriangleMeshGeometry geometry;
        shape->getTriangleMeshGeometry(geometry);
        return IGenerateProxy(addTo, idx, shape, geometry, l2w, mat, blended);
    }

    DEFAULT_FATAL(shape->getGeometryType())
    }

    return myDraw;
}
