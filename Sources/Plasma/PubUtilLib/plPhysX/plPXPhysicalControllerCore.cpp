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
#include "plPXConvert.h"
#include "plPhysXAPI.h"
#include "plPXPhysical.h"
#include "plPXSimDefs.h"
#include "plPXSimulation.h"
#include "plPXSubWorld.h"
#include "plSimulationMgr.h"

#include "pnMessage/plSetNetGroupIDMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plMessage/plCollideMsg.h"
#include "plModifier/plDetectorLog.h"
#include "plPhysical/plPhysicalProxy.h"
#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

// ==========================================================================

static std::vector<plPXPhysicalControllerCore*> gControllers;

#ifndef PLASMA_EXTERNAL_RELEASE
bool plPXPhysicalControllerCore::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE

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

plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, float height, float width)
{
    float radius = width / 2.0f;
    float realHeight = height - width;
    return new plPXPhysicalControllerCore(ownerSO, realHeight, radius);
}

plPXPhysicalControllerCore::plPXPhysicalControllerCore(plKey ownerSO, float height, float radius)
    : fActor(),
      fProxyGen(),
      plPhysicalControllerCore(ownerSO, height, radius)
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
        fActor->setLinearVelocity({ 0.f, 0.f, 0.f });
        hsChangeBits(fFlags, kDisableCollision, !enable);
        plPXFilterData::SetActorFlags(fActor, fFlags);
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
        plSimulationMgr::GetInstance()->GetPhysX()->AddToWorld(fActor, world);
        physx::PxTransform globalPose(plPXConvert::Point(IGetCapsulePos(fLocalPosition)),
                                      physx::PxIdentity);
        fActor->setGlobalPose(globalPose);
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
    if (fActor)
        plPXFilterData::Initialize(fActor, plSimDefs::kGroupAvatar, 0, losDB);
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

    physx::PxTransform globalPose(plPXConvert::Point(IGetCapsulePos(fLocalPosition)),
                                  physx::PxIdentity);
    fActor->setGlobalPose(globalPose);
}

void plPXPhysicalControllerCore::GetPositionSim(hsPoint3& pos)
{
    pos = IGetCapsuleFoot(plPXConvert::Point(fActor->getGlobalPose().p));
}

void plPXPhysicalControllerCore::SetPositionSim(const hsPoint3& pos)
{
    physx::PxTransform globalPose(plPXConvert::Point(IGetCapsulePos(pos)),
                                  physx::PxIdentity);
    fActor->setGlobalPose(globalPose);
}

void plPXPhysicalControllerCore::SetLinearVelocitySim(const hsVector3& velocity)
{
    fActor->setLinearVelocity(plPXConvert::Vector(velocity));
}

// ==========================================================================

std::vector<plControllerHitRecord> plPXPhysicalControllerCore::ISweepMulti(const hsPoint3& origin,
                                                                           const hsVector3& dir,
                                                                           float distance,
                                                                           plSimDefs::Group simGroups) const
{
    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);
    physx::PxCapsuleGeometry capsule;
    shape->getCapsuleGeometry(capsule);
    physx::PxTransform pose(plPXConvert::Point(IGetCapsulePos(origin)),
                            shape->getLocalPose().q);

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
                                   hit.distance);
            }
            return true;
        }
    } hitCallback(results);

    plPXControllerFilterCallback filterCallback;
    fActor->getScene()->sweep(capsule, pose, plPXConvert::Vector(dir), distance, hitCallback,
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
    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);
    physx::PxCapsuleGeometry capsule;
    shape->getCapsuleGeometry(capsule);
    physx::PxTransform pose(plPXConvert::Point(IGetCapsulePos(origin)),
                            shape->getLocalPose().q);

    plPXFilterData filterData;
    filterData.SetGroups(simGroups);
    physx::PxQueryFilterData queryFilter(filterData, physx::PxQueryFlag::eSTATIC |
                                                     physx::PxQueryFlag::eDYNAMIC |
                                                     physx::PxQueryFlag::ePREFILTER);

    physx::PxSweepBuffer buf(nullptr, 0);
    plPXControllerFilterCallback filterCallback;
    fActor->getScene()->sweep(capsule, pose, plPXConvert::Vector(dir), distance, buf,
                              (physx::PxHitFlag::ePOSITION |
                               physx::PxHitFlag::eNORMAL |
                               physx::PxHitFlag::eASSUME_NO_INITIAL_OVERLAP),
                              queryFilter, &filterCallback);

    std::optional<plControllerHitRecord> retval;
    if (buf.hasBlock) {
        retval.emplace(static_cast<plPXActorData*>(buf.block.actor->userData)->GetPhysical()->GetKey(),
                       plPXConvert::Point(buf.block.position),
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
    myDraw = plDrawableGenerator::GenerateSphericalDrawable(footOffset, fRadius + .1f,
                                                            mat, fLastGlobalLoc, blended,
                                                            nullptr, &idx, myDraw);

    return myDraw;
}

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

        controller->IApply(delSecs);
    }
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
    float halfHeight, radius;
    std::tie(halfHeight, radius) = IGetCapsuleDimensions();
    return footPos + (upDir * (radius + halfHeight));
}

hsPoint3 plPXPhysicalControllerCore::IGetCapsuleFoot(const hsPoint3& capPos,
                                                     const hsVector3& upDir) const
{
    float halfHeight, radius;
    std::tie(halfHeight, radius) = IGetCapsuleDimensions();
    return capPos - (upDir * (radius + halfHeight));
}

std::tuple<float, float> plPXPhysicalControllerCore::IGetCapsuleDimensions() const
{
    float halfHeight = fHeight / 2.f;
    halfHeight += fRadius * 0.4f;
    return std::make_tuple(halfHeight, fRadius);
}

physx::PxTransform plPXPhysicalControllerCore::IGetCapsulePose(const hsVector3& upDir) const
{
    // Just want to rotate the capsule 90deg along a non-Z axis so it stands up.
    // Gravity/up should be fixed using the body's global pose.
    return physx::PxTransform(physx::PxShortestRotation(physx::PxVec3(1.f, 0.f, 0.f),
                                                        plPXConvert::Vector(upDir)));
}

// ==========================================================================

void plPXPhysicalControllerCore::ICreateController(hsPoint3 pos)
{
    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();

    physx::PxCapsuleGeometry capsule;
    std::tie(capsule.halfHeight, capsule.radius) = IGetCapsuleDimensions();
    physx::PxTransform localPose = IGetCapsulePose();
    physx::PxTransform globalPose(plPXConvert::Point(IGetCapsulePos(pos)));

    fActor = (physx::PxRigidDynamic*)sim->CreateRigidActor(capsule, globalPose, localPose,
                                                           1.f, 0.f, 0.f,
                                                           plPXActorType::kDynamicActor);
    fActor->userData = new plPXActorData(this);
    plPXFilterData::Initialize(fActor, plSimDefs::kGroupAvatar, 0, fLOSDB);

    // Gravity is baked into the character movement code, so there is no need for it to
    // be applied by the simulation.
    fActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);

    // Initialize avatar mass such that the center of mass is at the capsule's foot and lock
    // the rotation to prevent tipping over during simulation.
    physx::PxVec3 cmass = plPXConvert::Point(IGetCapsuleFoot(pos));
    physx::PxRigidBodyExt::setMassAndUpdateInertia(*fActor, GetMass(), &cmass);
    fActor->setRigidDynamicLockFlags(physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_X |
                                     physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Y |
                                     physx::PxRigidDynamicLockFlag::eLOCK_ANGULAR_Z);

    sim->AddToWorld(fActor, fWorldKey);
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

    delete static_cast<plPXActorData*>(fActor->userData);
    fActor->userData = nullptr;
    plSimulationMgr::GetInstance()->GetPhysX()->RemoveFromWorld(fActor);
    fActor = nullptr;
}

// ==========================================================================

void plPXPhysicalControllerCore::AddContact(plPXPhysical* phys, const hsPoint3& pos,
                                            const hsVector3& normal)
{
    fMovementStrategy->AddContact(phys, pos, normal);

#ifndef PLASMA_EXTERNAL_RELEASE
    fDbgCollisionInfo.emplace_back(phys->GetKey(), pos, normal);
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

    debugString = ST::format("Controller #{} [Name: {}] [Subworld: {}] [Enabled: {}]",
                             controllerIdx + 1,
                             playerName.empty() ? controller->fOwner->GetName() : playerName,
                             controller->fWorldKey ? controller->fWorldKey->GetName() : "(main world)",
                             IsEnabled());
    debugTxt.DrawString(x, y, debugString);
    y += lineHeight;

    for (const auto& i : fDbgCollisionInfo) {
        float angle = hsRadiansToDegrees(acos(i.Normal * hsVector3(0.f, 0.f, 1.f)));
        debugString = ST::format("\tCollision: {}, Normal: ({.2f}, {.2f}, {.2f}), Angle({.1f})",
                i.PhysHit->GetName(),
                i.Normal.fX, i.Normal.fY, i.Normal.fZ, angle);
        debugTxt.DrawString(x, y, debugString);
        y += lineHeight;
    }
}
#endif // PLASMA_EXTERNAL_RELEASE
