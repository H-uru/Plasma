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
#include "plPXPhysicalControllerCore.h"
#include "plKickableLog.h"
#include "plPXConvert.h"
#include "plPhysXAPI.h"
#include "plPXPhysical.h"
#include "plPXSimDefs.h"
#include "plPXSimulation.h"
#include "plPXSubWorld.h"
#include "plSimulationMgr.h"

#include <cmath>

#include "pnMessage/plSetNetGroupIDMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plMessage/plCollideMsg.h"
#include "plModifier/plDetectorLog.h"
#include "plPhysical/plPhysicalProxy.h"
#include "plStatusLog/plStatusLog.h"
#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

// ==========================================================================

static std::vector<plPXPhysicalControllerCore*> gControllers;
static std::set<plKey> gInvalidCacheWorlds;

#ifndef PLASMA_EXTERNAL_RELEASE
bool plPXPhysicalControllerCore::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE

constexpr float kSlopeLimit = 55.f;
constexpr float kContactOffset = 0.1f;
constexpr float kStepOffset = 0.4f;

/**
 * The expected maximum height of a "small" kickable.
 * Small kickables are things like firemarbles, sticks, rocks, and leaves that
 * we could simply run over without kicking. Small kickables use a less accurate
 * but more likely to succeed direction vector to allow them to be kicked.
 * \note The fish baskets in Eder Gira are 2.4ft tall.
 */
constexpr float kSmallKickableThreshold = 1.5f;

// ==========================================================================

class plPXControllerFilterCallback : public physx::PxQueryFilterCallback
{
public:
    physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData,
                                          const physx::PxShape* shape,
                                          const physx::PxRigidActor* actor,
                                          physx::PxHitFlags& queryFlags) override
    {
        auto data = static_cast<plPXActorData*>(actor->userData);
        if (!data || !data->GetPhysical())
            return physx::PxQueryHitType::eNONE;

        // Disabled Physicals are not hit.
        if (data->GetPhysical()->GetProperty(plSimulationInterface::kDisable))
            return physx::PxQueryHitType::eNONE;

        // For now, any hit is a blocking hit. This could be expanded later, if needed.
        return physx::PxQueryHitType::eBLOCK;
    }

    physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData,
                                           const physx::PxQueryHit& hit) override
    {
        hsAssert(0, "plPXControllerFilterCallback::postFilter() should not be used.");
        return physx::PxQueryHitType::eNONE;
    }
};

// ==========================================================================

class plPXControllerHitReport : public physx::PxUserControllerHitReport
{
protected:
    void IApplyHitForce(physx::PxRigidBody* body, const physx::PxExtendedVec3& worldPos, physx::PxVec3 force) const
    {
        // This is (supposedly) the avatar's mass. I intended for this to be the mass unit
        // for an eIMPULSE force, but the results of that suck. It works great as a scale
        // for eVELOCITY_CHANGE, however.
        force *= 120.f;

        // Nuts, eVELOCITY_CHANGE forces are banned in PxRigidBodyExt::addForceAtPos().
        // So, do it manually here so we can use any mode we want.
        physx::PxTransform globalPose = body->getGlobalPose();
        physx::PxVec3 centerOfMass = globalPose.transform(body->getCMassLocalPose().p);
        physx::PxVec3 torque = (physx::toVec3(worldPos) - centerOfMass).cross(force);
        body->addForce(force, physx::PxForceMode::eVELOCITY_CHANGE);
        body->addTorque(torque, physx::PxForceMode::eVELOCITY_CHANGE);
        plKickableLog_Green(
            "Applying force ({.2f}, {.2f}, {.2f})",
            force.x, force.y, force.z
        );
        plKickableLog_Green(
            "Applying torque ({.2f}, {.2f}, {.2f})",
            torque.x, torque.y, torque.z
        );
    }

public:
    void onShapeHit(const physx::PxControllerShapeHit& hit) override
    {
        plPXActorData* controller = static_cast<plPXActorData*>(hit.controller->getUserData());
        plPXActorData* actor = static_cast<plPXActorData*>(hit.actor->userData);
        if (controller == nullptr || actor == nullptr)
            return;

        controller->GetController()->AddContact(
            actor->GetPhysical(),
            plPXConvert::ExtPoint(hit.worldPos),
            plPXConvert::Vector(hit.worldNormal),
            plPXConvert::Vector(hit.dir),
            hit.length
        );

        // Apply some hit force to kickables - the avatar is not actually a simulated body,
        // so we fake it using some back-of-napkin math here.
        if (actor->GetPhysical()->IsDynamic()) {
            physx::PxRigidDynamic* dynamic = static_cast<physx::PxRigidDynamic*>(hit.actor);
            plKickableLog_White("-----");
            plKickableLog_Yellow(
                "Controller '{}' hit dynamic '{}'\nDirection: ({.2f}, {.2f}, {.2f}), Length: {.2f}",
                controller->str(), actor->str(), hit.dir.x, hit.dir.y, hit.dir.z, hit.length
            );

            // We need to be careful about when we apply vertical force. In the real world, if you stand
            // on the ground, you push down on it, and the ground pushes back up on you. In the simulation,
            // there are no opposite forces, so continued downward force can cause kickables to be killed through
            // the floor. This is bad news for Eder Gira. But, wait! We would prefer to not be able to ride a
            // kickable rocket up into the sky because of a log bouncing off the ground. Further complicating matters,
            // small kickables like said logs will pretty much always be considered hit from above. So that means
            //     - Only apply downward forces if we detect the kickable is not already close to the ground.
            //     - If the kickable is flat-ish, we need a fake force based on the avatar's direction of
            //       travel; otherwise, it's simply stuck underneath us.
            physx::PxVec3 hitDir = hit.dir;

            plPXFilterData filterData;
            filterData.SetGroups((1 << plSimDefs::kGroupStatic) |
                                 (1 << plSimDefs::kGroupDynamicBlocker));
            physx::PxQueryFilterData queryFilter(filterData, physx::PxQueryFlag::eSTATIC |
                                                             physx::PxQueryFlag::eDYNAMIC |
                                                             physx::PxQueryFlag::ePREFILTER);
            physx::PxSceneQueryFlags outputFlags;
            physx::PxSweepHit sweepHit;
            plPXControllerFilterCallback filterCallback;
            bool onGround = physx::PxSceneQueryExt::sweepSingle(
                *(hit.actor->getScene()),
                hit.shape->getGeometry().any(),
                hit.actor->getGlobalPose(),
                hitDir,
                0.1f,
                outputFlags,
                sweepHit,
                queryFilter,
                &filterCallback
            );

            // `hit.length` is is the fraction of the movement delta we were in contact.
            float hitTime = controller->GetController()->fSimLength * hit.length;

            const hsVector3& avLinVel = controller->GetController()->GetLinearVelocity();
            float hitUpComponent = std::fabs(hitDir.dot(hit.controller->getUpDirection()));
            if (onGround && hitUpComponent >= .2f) {
                plKickableLog_Blue(
                    "The kickable is on '{}' and we have a upward magnitude of {.2f}",
                    sweepHit.actor->getName(), hitUpComponent
                );
                physx::PxGeometryHolder geom = hit.shape->getGeometry();
                physx::PxBounds3 bbox = physx::PxGeometryQuery::getWorldBounds(geom.any(), hit.shape->getLocalPose());
                float zext = bbox.getExtents(2);
                // Tiny kickable, so shoot it off into the distance.
                if (zext <= kSmallKickableThreshold) {
                    plKickableLog_Blue("Standing on top of tiny kickable with Z-extent {.2f}", zext);
                    physx::PxVec3 force(avLinVel.fX, avLinVel.fY, 0.f);
                    force.normalize();
                    plKickableLog_Green(
                        "Using renormalized avatar velocity as direction: ({.2f}, {.2f}, {.2f})",
                        force.x, force.y, force.z
                    );
                    // This calculation has been fairly carefully empirically scienced to produce a good
                    // result. We need to give this force a slight boost to ensure that we don't get on
                    // top of some tiny kickable and start blasting off to the moon.
                    force *= std::max(3.f, avLinVel.Magnitude()) * hitTime;
                    IApplyHitForce(dynamic, hit.worldPos, force);
                } else {
                    plKickableLog_Red("Standing on top of tall kickable with Z-extent {.2f} - no force will be applied.", zext);
                }
            } else {
                plKickableLog_Green(
                    "Ground: [{}], using PhysX hit direction: ({.2f}, {.2f}, {.2f})",
                    onGround, hitDir.x, hitDir.y, hitDir.z
                );
                // This calculation has been fairly carefully empirically scienced to produce a good
                // result. The length value here is generally trustworthy, especially when running,
                // but walking into the object can result in tiny lengths that result in poor
                // object displacement, resulting in stuttering movement because the kickable moves
                // slightly, then we move forward the same distance and are blocked, the kickable moves
                // forward, then we move forward the same distance and are blocked... ad nauseum
                physx::PxVec3 force = hitDir * std::max(3.f, avLinVel.Magnitude()) * hitTime;
                IApplyHitForce(dynamic, hit.worldPos, force);
            }
        }
    }

    void onControllerHit(const physx::PxControllersHit& hit) override
    {
        // Right now, these are just avatars and quabs. So, we don't care.
    }

    void onObstacleHit(const physx::PxControllerObstacleHit& hit) override
    {
        // Obstacles are currently unused by Plasma.
        hsAssert(0, "How did we get here?");
    }
} static s_HitReport;

// ==========================================================================

class plPXControllerBehaviorCallback : public physx::PxControllerBehaviorCallback
{
    plPXPhysicalControllerCore* fController;

public:
    plPXControllerBehaviorCallback(plPXPhysicalControllerCore* controller)
        : fController(controller)
    { }

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape, const physx::PxActor& actor) override
    {
        physx::PxControllerBehaviorFlags flags;
        if (fController->fMovementStrategy->IsRiding())
            flags.set(physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT);
        if (fController->fMovementStrategy->AllowSliding())
            flags.set(physx::PxControllerBehaviorFlag::eCCT_SLIDE);

        plPXActorData* data = static_cast<plPXActorData*>(actor.userData);
        if (data == nullptr || data->GetPhysical() == nullptr)
            return flags;

        if (data->GetPhysical()->IsDynamic()) {
            physx::PxGeometryHolder geom = shape.getGeometry();
            physx::PxBounds3 bbox = physx::PxGeometryQuery::getWorldBounds(geom.any(), shape.getLocalPose());
            if (bbox.getExtents(2) < kSmallKickableThreshold) {
                // Slide off of small kickables. Don't ride them.
                flags.clear(physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT);
                flags.set(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
                flags.set(physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE);
            } else {
                flags.clear(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
            }
        } else if (data->GetPhysical()->IsInLOSDB(plSimDefs::kLOSDBAvatarWalkable)) {
            // Don't slide down walkable surfaces.
            flags.clear(physx::PxControllerBehaviorFlag::eCCT_SLIDE);
        }

        return flags;
    }

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller) override
    {
        return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
    }

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle) override
    {
        return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;
    }
};

// ==========================================================================

plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, float height, float width)
{
    float radius = width / 2.0f;
    float realHeight = (height - width) + .8f;
    return new plPXPhysicalControllerCore(std::move(ownerSO), realHeight, radius);
}

plPXPhysicalControllerCore::plPXPhysicalControllerCore(plKey ownerSO, float height, float radius)
    : plPhysicalControllerCore(std::move(ownerSO), height, radius),
      fController(),
      fBehaviorCallback(std::make_unique<plPXControllerBehaviorCallback>(this)),
      fProxyGen()
{
    ICreateController(fLocalPosition);
    gControllers.push_back(this);
}

plPXPhysicalControllerCore::~plPXPhysicalControllerCore()
{
    gControllers.erase(std::find(gControllers.begin(), gControllers.end(), this));
    IDeleteController();
}

void plPXPhysicalControllerCore::Enable(bool enable)
{
    if (IsEnabled() != enable) {
        hsChangeBits(fFlags, kDisableCollision, !enable);
        plPXFilterData::SetActorFlags(fController->getActor(), fFlags);
    }
}

void plPXPhysicalControllerCore::SetSubworld(const plKey& world)
{
    if (fWorldKey != world) {
        // We need our real global location here, not the interpolated location
        fLocalRotation.MakeMatrix(&fLastGlobalLoc);
        fLastGlobalLoc.SetTranslate(&fLocalPosition);
        if (fWorldKey)
        {
            hsMatrix44 prevSubL2W;
            fPrevSubworldW2L.GetInverse(&prevSubL2W);
            fLastGlobalLoc = prevSubL2W * fLastGlobalLoc;
        }
        // Update our scene object so the change isn't wiped out
        plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
        if (so)
        {
            hsMatrix44 globalLocInv;
            fLastGlobalLoc.GetInverse(&globalLocInv);
            so->SetTransform(fLastGlobalLoc, globalLocInv);
            so->FlushTransform();
        }

        // Update Local Position and rotation
        fWorldKey = world;
        const plCoordinateInterface* subworldCI = GetSubworldCI();
        if (subworldCI)
        {
            fPrevSubworldW2L = subworldCI->GetWorldToLocal();
            hsMatrix44 l2s = fPrevSubworldW2L * fLastGlobalLoc;
            l2s.GetTranslate(&fLocalPosition);
            fLocalRotation.SetFromMatrix44(l2s);
        }
        else
        {
            fPrevSubworldW2L.Reset();
            fLastGlobalLoc.GetTranslate(&fLocalPosition);
            fLocalRotation.SetFromMatrix44(fLastGlobalLoc);
        }

        fLastLocalPosition = fLocalPosition;

        // Move into the doggone new world.
        ICreateController(fLocalPosition);
    }
}

void plPXPhysicalControllerCore::GetState(hsPoint3& pos, float& zRot)
{
    // Temporarily use the position point while we get the z rotation
    fLocalRotation.NormalizeIfNeeded();
    fLocalRotation.GetAngleAxis(&zRot, (hsVector3*)&pos);

    if (pos.fZ < 0)
        zRot = hsConstants::two_pi<float> - zRot; // axis is backwards, so reverse the angle too

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

void plPXPhysicalControllerCore::SetLOSDB(plSimDefs::plLOSDB losDB)
{
    plPhysicalControllerCore::SetLOSDB(losDB);
    if (fController)
        plPXFilterData::Initialize(fController->getActor(), plSimDefs::kGroupAvatar, 0, losDB);
}

void plPXPhysicalControllerCore::SetMovementStrategy(plMovementStrategy* strategy)
{
    fMovementStrategy = strategy;
}

void plPXPhysicalControllerCore::SetGlobalLoc(const hsMatrix44& l2w)
{
    fLastGlobalLoc = l2w;

    // Update our local position and rotation
    const plCoordinateInterface* subworldCI = GetSubworldCI();
    if (subworldCI) {
        hsMatrix44 l2s = fPrevSubworldW2L * l2w;

        l2s.GetTranslate(&fLocalPosition);
        fLocalRotation.SetFromMatrix44(l2s);
    } else {
        l2w.GetTranslate(&fLocalPosition);
        fLocalRotation.SetFromMatrix44(l2w);
    }
    fLastLocalPosition = fLocalPosition;

    fController->setFootPosition(plPXConvert::ExtPoint(fLocalPosition));
}

void plPXPhysicalControllerCore::GetPositionSim(hsPoint3& pos)
{
    pos = plPXConvert::ExtPoint(fController->getFootPosition());
}

void plPXPhysicalControllerCore::SetPositionSim(const hsPoint3& pos)
{
    fController->setFootPosition(plPXConvert::ExtPoint(pos));
}

plPhysicalControllerCore::Collisions plPXPhysicalControllerCore::Move(const hsVector3& velocity,
                                                                      float delSecs,
                                                                      plSimDefs::Group simGroups)
{
    class plPXControllerFilterCallback : public physx::PxControllerFilterCallback
    {
    public:
        bool filter(const physx::PxController& a, const physx::PxController& b) override
        {
            // Characters never collide with one another.
            return false;
        }
    } controllerCallback;

    class plPXControllerQueryFilterCallback : public physx::PxQueryFilterCallback
    {
    public:
        physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData,
                                              const physx::PxShape* shape,
                                              const physx::PxRigidActor* actor,
                                              physx::PxHitFlags& queryFlags) override
        {
            auto data = static_cast<plPXActorData*>(actor->userData);
            if (!data)
                return physx::PxQueryHitType::eNONE;

            // Disabled physicals aren't hit.
            if (data->GetPhysical() && data->GetPhysical()->GetProperty(plSimulationInterface::kDisable))
                return physx::PxQueryHitType::eNONE;
            // should never happen
            if (data->GetController())
                return physx::PxQueryHitType::eNONE;

            return physx::PxQueryHitType::eBLOCK;
        }

        physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData,
                                               const physx::PxQueryHit& hit) override
        {
            return physx::PxQueryHitType::eBLOCK;
        }
    } queryCallback;

    // Stash this velocity for use in the hit report callback
    SetLinearVelocity(velocity);

    plPXFilterData filterData;
    // Always collide with detectors - they'll be converted into touches.
    filterData.SetGroups(simGroups);
    filterData.ToggleReportOn(plSimDefs::kGroupAvatar);
    physx::PxControllerFilters filters(&filterData, &queryCallback, &controllerCallback);

    hsVector3 displacement = velocity * delSecs;
    physx::PxControllerCollisionFlags result = fController->move(
        plPXConvert::Vector(displacement),
        0.001f,
        fSimLength,
        filters,
        nullptr
    );

    uint32_t col = 0;
    if (result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_UP))
        col |= plPhysicalControllerCore::kTop;
    if (result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_SIDES))
        col |= plPhysicalControllerCore::kSides;
    if (result.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
        col |= plPhysicalControllerCore::kBottom;
    return (Collisions)col;
}

// ==========================================================================

std::vector<plControllerHitRecord> plPXPhysicalControllerCore::ISweepMulti(const hsPoint3& origin,
                                                                           const hsVector3& dir,
                                                                           float distance,
                                                                           plSimDefs::Group simGroups) const
{
    auto [capsule, pose] = IGetCapsule(origin);
    plPXFilterData filterData;
    filterData.SetGroups(simGroups);
    physx::PxQueryFilterData queryFilter(filterData, physx::PxQueryFlag::eSTATIC |
                                                     physx::PxQueryFlag::eDYNAMIC |
                                                     physx::PxQueryFlag::ePREFILTER |
                                                     physx::PxQueryFlag::eNO_BLOCK);
    std::vector<plControllerHitRecord> results;

    class plPXSweepHitCallback : public physx::PxSweepCallback
    {
        decltype(results)& fHits;
        physx::PxSweepHit fHitBuf[10];

    public:
        plPXSweepHitCallback(decltype(results)& vec)
            : fHits(vec), physx::PxSweepCallback(fHitBuf, std::size(fHitBuf))
        { }

        physx::PxAgain processTouches(const physx::PxSweepHit* buffer, physx::PxU32 nbHits) override
        {
            fHits.reserve(fHits.size() + nbHits);
            for (size_t i = 0; i < nbHits; ++i) {
                const physx::PxSweepHit& hit = buffer[i];
                auto hitActor = static_cast<plPXActorData*>(hit.actor->userData);
                fHits.emplace_back(hitActor->GetPhysical()->GetKey(),
                                   plPXConvert::Point(hit.position),
                                   plPXConvert::Vector(hit.normal),
                                   plPXConvert::Vector(hit.normal),
                                   hit.distance);
            }
            return true;
        }
    } hitCallback(results);

    plPXControllerFilterCallback filterCallback;
    fController->getScene()->sweep(capsule, pose, plPXConvert::Vector(dir), distance, hitCallback,
                                   (physx::PxHitFlag::ePOSITION |
                                    physx::PxHitFlag::eNORMAL |
                                    physx::PxHitFlag::eASSUME_NO_INITIAL_OVERLAP),
                                   queryFilter, &filterCallback);
    return results;
}

std::optional<plControllerHitRecord> plPXPhysicalControllerCore::ISweepSingle(const hsPoint3& origin,
                                                                              const hsVector3& dir,
                                                                              float distance,
                                                                              plSimDefs::Group simGroups) const
{
    auto [capsule, pose] = IGetCapsule(origin);
    plPXFilterData filterData;
    filterData.SetGroups(simGroups);
    physx::PxQueryFilterData queryFilter(filterData, physx::PxQueryFlag::eSTATIC |
                                                     physx::PxQueryFlag::eDYNAMIC |
                                                     physx::PxQueryFlag::ePREFILTER);

    physx::PxSweepBuffer buf(nullptr, 0);
    plPXControllerFilterCallback filterCallback;
    fController->getScene()->sweep(capsule, pose, plPXConvert::Vector(dir), distance, buf,
                                   (physx::PxHitFlag::ePOSITION |
                                    physx::PxHitFlag::eNORMAL |
                                    physx::PxHitFlag::eASSUME_NO_INITIAL_OVERLAP),
                                   queryFilter, &filterCallback);

    std::optional<plControllerHitRecord> retval;
    if (buf.hasBlock) {
        retval.emplace(static_cast<plPXActorData*>(buf.block.actor->userData)->GetPhysical()->GetKey(),
                       plPXConvert::Point(buf.block.position),
                       plPXConvert::Vector(buf.block.normal),
                       plPXConvert::Vector(buf.block.normal),
                       buf.block.distance);
    }
    return retval;
}

std::vector<plControllerHitRecord> plPXPhysicalControllerCore::SweepMulti(const hsPoint3& startPos,
                                                                          const hsPoint3& endPos,
                                                                          plSimDefs::Group simGroups) const
{
    hsVector3 displacement = (hsVector3)(endPos - startPos);
    float magnitude = displacement.Magnitude();
    displacement.Normalize();
    return ISweepMulti(startPos, displacement, magnitude, simGroups);
}

std::optional<plControllerHitRecord> plPXPhysicalControllerCore::SweepSingle(const hsPoint3& startPos,
                                                                             const hsPoint3& endPos,
                                                                             plSimDefs::Group simGroups) const
{
    hsVector3 displacement = (hsVector3)(endPos - startPos);
    float magnitude = displacement.Magnitude();
    displacement.Normalize();
    return ISweepSingle(startPos, displacement, magnitude, simGroups);
}


// ==========================================================================

void plPXPhysicalControllerCore::LeaveAge()
{
    SetPushingPhysical(nullptr);
    SetSubworld(nullptr);
}

plDrawableSpans* plPXPhysicalControllerCore::CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo)
{
    plDrawableSpans* myDraw = addTo;
    bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

    // Remember the offset and skins.
    hsPoint3 footOffset = IGetCapsulePos({ 0.f, 0.f, 0.f });
    myDraw = plDrawableGenerator::GenerateCapsuleDrawable(footOffset, fRadius + .1f, fHeight + 1.f,
                                                          mat, fLastGlobalLoc, blended, nullptr,
                                                          &idx, myDraw);

    return myDraw;
}

// ==========================================================================

void plPXPhysicalControllerCore::Apply(float delSecs)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];

#ifndef PLASMA_EXTERNAL_RELEASE
        controller->fDbgCollisionInfo.clear();
#endif

        auto findIt = gInvalidCacheWorlds.find(controller->fWorldKey);
        if (findIt != gInvalidCacheWorlds.end())
            controller->fController->invalidateCache();

        controller->IApply(delSecs);
    }

    gInvalidCacheWorlds.clear();
}

void plPXPhysicalControllerCore::Update(int numSubSteps, float alpha)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];

        controller->IUpdate(numSubSteps, alpha);
        controller->ISynchProxy();
        controller->IChangePhysicalOwnership();

#ifndef PLASMA_EXTERNAL_RELEASE
        if (fDebugDisplay)
            controller->IDrawDebugDisplay(i);
#endif
    }
}

void plPXPhysicalControllerCore::UpdateNonPhysical(float alpha)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];
        controller->IUpdateNonPhysical(alpha);
        controller->ISynchProxy();

#ifndef PLASMA_EXTERNAL_RELEASE
        if (fDebugDisplay)
            controller->IDrawDebugDisplay(i);
#endif
    }
}

void plPXPhysicalControllerCore::InvalidateCache(plKey world)
{
    gInvalidCacheWorlds.emplace(std::move(world));
}

// ==========================================================================

void plPXPhysicalControllerCore::ISynchProxy()
{
    if (fProxyGen) {
        hsMatrix44 w2l;
        fLastGlobalLoc.GetInverse(&w2l);
        fProxyGen->SetTransform(fLastGlobalLoc, w2l);
    }
}

void plPXPhysicalControllerCore::IChangePhysicalOwnership()
{
    if (!(fLOSDB & plSimDefs::kLOSDBLocalAvatar)) {
        fHitObjects.clear();
        return;
    }

    for (const auto& i : fHitObjects) {
        plPXPhysical* obj = plPXPhysical::ConvertNoRef(i->ObjectIsLoaded());
        obj->SetNetGroupConstant(plNetGroup::kNetGroupLocalPhysicals);

        plSetNetGroupIDMsg* setNetGroupID = new plSetNetGroupIDMsg;
        setNetGroupID->fId = plNetGroup::kNetGroupRemotePhysicals;
        setNetGroupID->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
        setNetGroupID->SetBCastFlag(plMessage::kLocalPropagate, false);
        setNetGroupID->Send(i);

        plSimulationMgr::GetInstance()->ConsiderSynch(obj, nullptr);
    }
    fHitObjects.clear();
}

// ==========================================================================

hsPoint3 plPXPhysicalControllerCore::IGetCapsulePos(const hsPoint3& footPos,
                                                    const hsVector3& upDir) const
{
    return footPos + (upDir * (kContactOffset + fRadius + fHeight * 0.5f));
}

hsPoint3 plPXPhysicalControllerCore::IGetCapsuleFoot(const hsPoint3& capPos,
                                                     const hsVector3& upDir) const
{
    return capPos - (upDir * (kContactOffset + fRadius + fHeight * 0.5f));
}

physx::PxTransform plPXPhysicalControllerCore::IGetCapsulePose(const hsPoint3& pos, const hsVector3& upDir) const
{
    // Just want to rotate the capsule 90deg along a non-Z axis so it stands up.
    // Gravity/up should be fixed using the body's global pose.
    return physx::PxTransform(
        plPXConvert::Point(IGetCapsulePos(pos, upDir)),
        physx::PxShortestRotation(
            physx::PxVec3(1.f, 0.f, 0.f),
            plPXConvert::Vector(upDir)
        )
    );
}

std::tuple<physx::PxCapsuleGeometry, physx::PxTransform> plPXPhysicalControllerCore::IGetCapsule(const hsPoint3& pos) const
{
    physx::PxShape* shape;
    fController->getActor()->getShapes(&shape, 1);
    physx::PxCapsuleGeometry capsule;
    shape->getCapsuleGeometry(capsule);
    capsule.radius += kContactOffset;
    return std::make_tuple(
        capsule,
        IGetCapsulePose(
            pos,
            plPXConvert::Vector(fController->getUpDirection())
        )
    );
}

// ==========================================================================

void plPXPhysicalControllerCore::ICreateController(const hsPoint3& pos)
{
    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();

    physx::PxCapsuleControllerDesc desc;
    desc.position = plPXConvert::ExtPoint(IGetCapsulePos(pos));
    desc.upDirection = { 0.f, 0.f, 1.f };
    desc.slopeLimit = std::cos(hsDegreesToRadians(kSlopeLimit));
    desc.contactOffset = kContactOffset;
    desc.stepOffset = kStepOffset;
    desc.reportCallback = &s_HitReport;
    desc.behaviorCallback = fBehaviorCallback.get();
    desc.nonWalkableMode = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
    desc.userData = new plPXActorData(this);
    desc.height = fHeight;
    desc.radius = fRadius;
    desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED;

    IDeleteController();
    fController = sim->CreateCharacterController(desc, fWorldKey);
    plPXFilterData::Initialize(fController->getActor(), plSimDefs::kGroupAvatar, 0, fLOSDB, fFlags);
    fController->getActor()->userData = desc.userData;

    fFlags &= ~kSeeking;

    // Create proxy for the debug display
    // local avatar is light purple and transparent
    fProxyGen = new plPhysicalProxy({ 0.f, 0.f, 0.f, 1.f }, { .2f, .1f, .2f, 1.f }, .8f);
    fProxyGen->Init(this);
}

void plPXPhysicalControllerCore::IDeleteController()
{
    delete fProxyGen;
    fProxyGen = nullptr;

    if (fController) {
        plPXActorData* actor = static_cast<plPXActorData*>(fController->getUserData());
        fController->setUserData(nullptr);
        fController->getActor()->userData = nullptr;
        plSimulationMgr::GetInstance()->GetPhysX()->RemoveFromWorld(fController);
        fController = nullptr;
        delete actor;
    }
}

// ==========================================================================

plKey plPXPhysicalControllerCore::GetGround() const
{
    physx::PxControllerState state;
    fController->getState(state);
    if (state.touchedActor != nullptr) {
        plPXActorData* actor = static_cast<plPXActorData*>(state.touchedActor->userData);
        if (actor)
            return actor->GetPhysical()->GetKey();
    }
    return {};
}

void plPXPhysicalControllerCore::AddContact(plPXPhysical* phys, const hsPoint3& pos,
                                            const hsVector3& normal, const hsVector3& dir,
                                            float length)
{
    fMovementStrategy->AddContact(phys, pos, normal);

#ifndef PLASMA_EXTERNAL_RELEASE
    fDbgCollisionInfo.emplace_back(phys->GetKey(), pos, normal, dir, length);
#endif

    // If this is a Plasma dynamic, we may need to take ownership of it.
    if (phys->IsDynamic() &&
        !phys->IsLocallyOwned() &&
        !phys->GetProperty(plSimulationInterface::kNoOwnershipChange)) {
        fHitObjects.insert(phys->GetKey());
    }
}

#ifndef PLASMA_EXTERNAL_RELEASE
#include "pnNetCommon/plNetApp.h"
#include "plPipeline/plDebugText.h"

void plPXPhysicalControllerCore::IDrawDebugDisplay(int controllerIdx)
{
    plDebugText& debugTxt = plDebugText::Instance();
    ST::string debugString;
    int lineHeight = debugTxt.GetFontSize() + 4;
    int x = 10;     // Initial draw position
    static int y = 10;
    if (controllerIdx == 0) {
        y = 10;
        debugString = ST::format("Controller Count: {}", gControllers.size());
        debugTxt.DrawString(x, y, debugString);
        y += lineHeight;
    }

    const auto& controller = gControllers[controllerIdx];
    ST::string playerName = plNetClientApp::GetInstance()->GetPlayerName(controller->fOwner);

    debugString = ST::format("Controller #{} [Name: {}] [Ground: {}] [Subworld: {}] [Enabled: {}]",
                             controllerIdx + 1,
                             playerName.empty() ? controller->fOwner->GetName() : playerName,
                             controller->GetGround() ? controller->GetGround()->GetName() : "(none)",
                             controller->fWorldKey ? controller->fWorldKey->GetName() : "(main world)",
                             IsEnabled());
    debugTxt.DrawString(x, y, debugString);
    y += lineHeight;

    for (const auto& i : fDbgCollisionInfo) {
        float angle = hsRadiansToDegrees(acos(i.Normal * hsVector3(0.f, 0.f, 1.f)));
        debugString = ST::format(
            "\tCollision: {}, Normal: ({.2f}, {.2f}, {.2f}), Angle({.1f}), "
            "Direction: ({.2f}, {.2f}, {.2f}), Length({.2f})",
            i.PhysHit->GetName(),
            i.Normal.fX, i.Normal.fY, i.Normal.fZ, angle,
            i.Direction.fX, i.Direction.fY, i.Direction.fZ,
            i.Displacement
        );
        debugTxt.DrawString(x, y, debugString);
        y += lineHeight;
    }
}
#endif // PLASMA_EXTERNAL_RELEASE
