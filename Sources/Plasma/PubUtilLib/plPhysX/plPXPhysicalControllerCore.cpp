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
#include "plSimulationMgr.h"

#include "plAvatar/plArmatureMod.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plMessage/plCollideMsg.h"
#include "plModifier/plDetectorLog.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plPhysical/plPhysicalProxy.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnMessage/plSetNetGroupIDMsg.h"

#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

static std::vector<plPXPhysicalControllerCore*> gControllers;

#ifndef PLASMA_EXTERNAL_RELEASE
bool plPXPhysicalControllerCore::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE
int plPXPhysicalControllerCore::fPXControllersMax = 0;

#define kCCTSkinWidth 0.1f
#define kCCTStepOffset 0.4f
#define kCCTZOffset ((fRadius + (fHeight / 2)) + kCCTSkinWidth)
#define kPhysHeightCorrection 0.8f
#define kPhysZOffset ((kCCTZOffset + (kPhysHeightCorrection / 2)) - 0.05f)
#define kAvatarMass 200.0f

class plPXControllerBehaviorCallback : public physx::PxControllerBehaviorCallback
{
    plPXPhysicalControllerCore* fController;

public:
    plPXControllerBehaviorCallback() = delete;
    plPXControllerBehaviorCallback(plPXPhysicalControllerCore* controller)
        : fController(controller)
    { }
    plPXControllerBehaviorCallback(const plPXControllerBehaviorCallback&) = delete;
    plPXControllerBehaviorCallback(plPXControllerBehaviorCallback&&) = delete;

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxShape& shape,
                                                      const physx::PxActor& actor) override
    {
        if (fController->fMovementStrategy->Ride())
            return physx::PxControllerBehaviorFlag::eCCT_CAN_RIDE_ON_OBJECT;

        // We don't want to slide on the actual walkable ground.
        auto data = static_cast<plPXActorData*>(actor.userData);
        if (data && data->GetPhysical()->IsInLOSDB(plSimDefs::kLOSDBAvatarWalkable))
            return physx::PxControllerBehaviorFlag::eCCT_USER_DEFINED_RIDE;

        // Some obstacle that we should probably not stand on top of. Slide off of it.
        return physx::PxControllerBehaviorFlag::eCCT_SLIDE;
    }

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxController& controller) override
    {
        return physx::PxControllerBehaviorFlag::eCCT_SLIDE;
    }

    physx::PxControllerBehaviorFlags getBehaviorFlags(const physx::PxObstacle& obstacle) override
    {
        // NOTE: Obstacles are never used.
        return physx::PxControllerBehaviorFlag::eCCT_SLIDE;
    }
};

class plPXControllerHitReport : public physx::PxUserControllerHitReport
{
public:
    void onShapeHit(const physx::PxControllerShapeHit& hit) override
    {
        auto hitter = static_cast<plPXActorData*>(hit.controller->getUserData());
        auto controller = hitter->GetController();

        physx::PxRigidActor* actor = hit.shape->getActor();
        auto hittee = static_cast<plPXActorData*>(actor->userData);
        auto phys = hittee->GetPhysical();
        if (!phys)
            return;

        hsPoint3 pos(hit.worldPos.x, hit.worldPos.y, hit.worldPos.z);
        hsVector3 normal = plPXConvert::Vector(hit.worldNormal);

#ifndef PLASMA_EXTERNAL_RELEASE
        plDbgCollisionInfo info;
        info.fNormal = normal;
        info.fSO = plSceneObject::ConvertNoRef(hittee->GetKey()->ObjectIsLoaded());

        controller->fDbgCollisionInfo.Append(info);
#endif // PLASMA_EXTERNAL_RELEASE

        // If the avatar hit a movable physical, apply some force to it.
        if (physx::PxRigidDynamic* dynActor = actor->is<physx::PxRigidDynamic>()) {
            // Don't apply force when standing on top of an object.
            if (normal.fZ < 0.85f) {
                hsVector3 velocity = controller->GetLinearVelocity();
                velocity.fZ = 0.0f;
                float length = velocity.Magnitude();
                if (length > 0) {
                    // Only allow horizontal pushes for now
                    physx::PxVec3 hitDir = hit.worldPos - hit.controller->getPosition();
                    hitDir.z = 0.0f;
                    hitDir.normalize();

                    // Get controller speed along the hitDir
                    float cctProj = velocity.fX * hitDir.x + velocity.fY * hitDir.y;
                    length = length + cctProj / 2.0f;

                    // Get hit actors speed along the hitDir
                    float hitProj = dynActor->getLinearVelocity().dot(hitDir);
                    if (hitProj > 0)
                        length -= hitProj;

                    length *= kAvatarMass;

                    hsPoint3 pos((float)hit.worldPos.x, (float)hit.worldPos.y, (float)hit.worldPos.z);
                    phys->SetHitForce(plPXConvert::Vector(hitDir * length), pos);
                    controller->AddDynamicHit(phys);
                }
            }
        }

        if (phys && phys->GetProperty(plSimulationInterface::kAvAnimPushable)) {
            hsQuat inverseRotation = controller->fLocalRotation.Inverse();
            hsVector3 normal = plPXConvert::Vector(hit.worldNormal);
            controller->SetPushingPhysical(phys);
            controller->SetFacingPushingPhysical((inverseRotation.Rotate(&kAvatarForward).InnerProduct(normal) < 0 ? true : false));
        }
    }

    void onControllerHit(const physx::PxControllersHit& hit) override { }
    void onObstacleHit(const physx::PxControllerObstacleHit& hit) override { }
} static gHitCallback;

plPhysicalControllerCore* plPhysicalControllerCore::Create(plKey ownerSO, float height, float width, bool human)
{
    if (!plPXPhysicalControllerCore::fPXControllersMax || gControllers.size() < plPXPhysicalControllerCore::fPXControllersMax)
    {
        float radius = width / 2.0f;
        float realHeight = height - width;
        return new plPXPhysicalControllerCore(ownerSO, realHeight, radius, human);
    }
    return nil;
}

plPXPhysicalControllerCore::plPXPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human)
    : fBehaviorCallback(std::make_unique<plPXControllerBehaviorCallback>(this)),
      fController(),
      fActor(),
      fProxyGen(),
      fHuman(human),
      plPhysicalControllerCore(ownerSO, height, radius)
{
    ICreateController(fLocalPosition);
    gControllers.push_back(this);
}
plPXPhysicalControllerCore::~plPXPhysicalControllerCore()
{
    gControllers.erase(std::find(gControllers.begin(), gControllers.end(), this));
    IDeleteController();

    // Release any queued messages we may have
    int numMsgs = fQueuedCollideMsgs.size();
    if (numMsgs)
    {
        for (int i = 0; i < numMsgs; ++i)
            delete fQueuedCollideMsgs[i];

        fQueuedCollideMsgs.clear();
    }

    delete fProxyGen;
}

void plPXPhysicalControllerCore::Enable(bool enable)
{
    if (fEnabled != enable) {
        fEnabled = enable;
        if (fEnabled) {
            // Defer until the next physics update.
            fEnableChanged = true;
        }
    }
}

void plPXPhysicalControllerCore::SetSubworld(const plKey& world)
{
    if (fWorldKey != world)
    {
        SimLog("Changing subworlds!");

        // Inform detectors in the old world that we are leaving
        IInformDetectors(false);
        IDeleteController();

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

        // Create new controller
        ICreateController(fLocalPosition);
    }
}

void plPXPhysicalControllerCore::GetState(hsPoint3& pos, float& zRot)
{
    // Temporarily use the position point while we get the z rotation
    fLocalRotation.NormalizeIfNeeded();
    fLocalRotation.GetAngleAxis(&zRot, (hsVector3*)&pos);

    if (pos.fZ < 0)
        zRot = (2 * float(M_PI)) - zRot; // axis is backwards, so reverse the angle too

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
    hsPoint3 prevPosition = fLocalPosition;
    const plCoordinateInterface* subworldCI = GetSubworldCI();
    if (subworldCI)
    {
        hsMatrix44 l2s = fPrevSubworldW2L * l2w;

        l2s.GetTranslate(&fLocalPosition);
        fLocalRotation.SetFromMatrix44(l2s);
    }
    else
    {
        l2w.GetTranslate(&fLocalPosition);
        fLocalRotation.SetFromMatrix44(l2w);
    }

    fLastLocalPosition = fLocalPosition;

    if (fProxyGen)
    {
        hsMatrix44 w2l;
        l2w.GetInverse(&w2l);
        fProxyGen->SetTransform(l2w, w2l);
    }

    physx::PxExtendedVec3 extPos(fLocalPosition.fX, fLocalPosition.fY, fLocalPosition.fZ + kCCTZOffset);
    fController->setPosition(extPos);
}

void plPXPhysicalControllerCore::GetPositionSim(hsPoint3& pos)
{
    physx::PxExtendedVec3 extPos = fController->getFootPosition();
    pos.Set(extPos.x, extPos.y, extPos.z);
}

void plPXPhysicalControllerCore::Move(const hsVector3& displacement, unsigned int collideWith,
                                      unsigned int& collisionResults)
{
    class plPXControllerFilterCallback : public physx::PxControllerFilterCallback
    {
    public:
        bool filter(const physx::PxController& a, const physx::PxController& b) override
        {
            auto data1 = static_cast<plPXActorData*>(a.getUserData());
            auto data2 = static_cast<plPXActorData*>(b.getUserData());
            if (!data1 || !data2)
                return false;
            // Quabs are solid. Player avatars, not so much.
            return !(data1->GetController()->fHuman && data2->GetController()->fHuman);
        }
    } filterCallback;

    class plPXControllerMoveFilterCallback : public physx::PxQueryFilterCallback
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
    } moveCallback;

    plPXFilterData filterData;
    filterData.SetGroups(collideWith);
    filterData.ToggleReportOn(plSimDefs::kGroupAvatar);
    physx::PxControllerFilters controllerFilters(&filterData, &moveCallback, &filterCallback);
    controllerFilters.mFilterFlags |= physx::PxQueryFlag::ePREFILTER |
                                      physx::PxQueryFlag::eSTATIC |
                                      physx::PxQueryFlag::eDYNAMIC;
    auto colFlags = fController->move(plPXConvert::Vector(displacement), 0.00001f,
                                      fSimLength, controllerFilters);

    collisionResults = 0;
    if (colFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_DOWN))
        collisionResults |= kBottom;
    if (colFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_UP))
        collisionResults |= kTop;
    if (colFlags.isSet(physx::PxControllerCollisionFlag::eCOLLISION_SIDES))
        collisionResults |= kSides;
}

int plPXPhysicalControllerCore::SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, bool vsDynamics, bool vsStatics,
                            uint32_t& vsSimGroups, std::vector<plControllerSweepRecord>& hits)
{
    hsVector3 displacement = (hsVector3)(endPos - startPos);
    float magnitude = displacement.Magnitude();
    displacement.Normalize();

    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);
    physx::PxCapsuleGeometry capsule;
    shape->getCapsuleGeometry(capsule);

    physx::PxTransform pose(fController->getPosition().x, fController->getPosition().y,
                            fController->getPosition().z - 0.1f, physx::PxIDENTITY::PxIdentity);
    pose.p.z -= 0.1f;

    plPXFilterData filterData;
    filterData.SetGroups(vsSimGroups);
    filterData.ToggleReportOn(plSimDefs::kGroupAvatar);
    physx::PxQueryFilterData filter(filterData, physx::PxQueryFlag::eNO_BLOCK);
    if (vsDynamics)
        filter.flags |= physx::PxQueryFlag::eDYNAMIC;
    if (vsStatics)
        filter.flags |= physx::PxQueryFlag::eSTATIC;

    class plPXSweepOverlap : public physx::PxSweepCallback
    {
        decltype(hits)& fHits;
        physx::PxSweepHit fHitBuf[10];

    public:
        plPXSweepOverlap(decltype(hits)& vec)
            : fHits(vec), physx::PxSweepCallback(fHitBuf, std::size(fHitBuf))
        { }

        physx::PxAgain processTouches(const physx::PxSweepHit* buffer, physx::PxU32 nbHits) override
        {
            for (size_t i = 0; i < nbHits; ++i) {
                const physx::PxSweepHit& hit = buffer[i];
                auto hitActor = static_cast<plPXActorData*>(hit.actor->userData);
                if (!hitActor || !hitActor->GetPhysical())
                    continue;
                fHits.emplace_back(hitActor->GetPhysical(),
                                   plPXConvert::Point(hit.position),
                                   plPXConvert::Vector(hit.normal));
            }
            return true;
        }
    } callback(hits);

    fActor->getScene()->sweep(capsule, pose,
                              plPXConvert::Vector(displacement), magnitude, callback,
                              physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL,
                              filter);
    return hits.size();
}

void plPXPhysicalControllerCore::LeaveAge()
{
    SetPushingPhysical(nil);
    if (fWorldKey)
        SetSubworld(nil);
}

plDrawableSpans* plPXPhysicalControllerCore::CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo)
{
    // FIXME
    plDrawableSpans* myDraw = addTo;
    bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));
    float radius = fRadius;
    myDraw = plDrawableGenerator::GenerateSphericalDrawable(fLocalPosition, radius,
        mat, fLastGlobalLoc, blended,
        nil, &idx, myDraw);

/*
    plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
    if (so)
    {
        bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

        myDraw = plDrawableGenerator::GenerateConicalDrawable(fRadius*10, fHeight*10,
            mat, so->GetLocalToWorld(), blended,
            nil, &idx, myDraw);
    }
*/
    return myDraw;
}

void plPXPhysicalControllerCore::AddDynamicHit(plPXPhysical* phys)
{
    int numHits = fDynamicHits.size();
    for (int i = 0; i < numHits; ++i)
    {
        if (fDynamicHits[i] == phys)
            return;
    }

    fDynamicHits.push_back(phys);
}

void plPXPhysicalControllerCore::Apply(float delSecs)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];

#ifndef PLASMA_EXTERNAL_RELEASE
        controller->fDbgCollisionInfo.SetCount(0);
#endif

        controller->IDispatchQueuedMsgs();
        controller->IApply(delSecs);
        controller->IProcessDynamicHits();
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
    }
}

int plPXPhysicalControllerCore::NumControllers() { return gControllers.size(); }

void plPXPhysicalControllerCore::IHandleEnableChanged()
{
    // Defered enable
    fEnableChanged = false;
}

void plPXPhysicalControllerCore::IInformDetectors(bool entering)
{
    plDetectorLog::Log("Informing from plPXPhysicalControllerCore::IInformDetectors");

    physx::PxShape* controllerShape;
    fActor->getShapes(&controllerShape, 1);
    physx::PxCapsuleGeometry capsule;
    controllerShape->getCapsuleGeometry(capsule);

    class DetectorOverlap : public physx::PxOverlapCallback
    {
        plKey fOwner;
        bool fEntering;
        decltype(fQueuedCollideMsgs)& fMsgs;
        physx::PxOverlapHit fHits[32];

    public:
        DetectorOverlap(plKey owner, bool entering, decltype(fQueuedCollideMsgs)& msgs)
            : fOwner(std::move(owner)), fEntering(entering), fMsgs(msgs),
              physx::PxOverlapCallback(fHits, std::size(fHits))
        { }

        physx::PxAgain processTouches(const physx::PxOverlapHit* buffer, physx::PxU32 nbHits) override
        {
            for (size_t i = 0; i < nbHits; ++i) {
                const physx::PxOverlapHit& hit = buffer[i];
                auto hitData = static_cast<plPXActorData*>(hit.actor->userData);
                if (hitData->GetPhysical())
                    fMsgs.push_back(new plCollideMsg(nullptr, hitData->GetKey(),
                                                     fOwner, fEntering));
            }
            return true;
        }
    };

    // Ensures that we only fire for detectors (aka "report on avatar")
    plPXFilterData filter;
    filter.ToggleGroup(plSimDefs::kGroupDetector);
    filter.ToggleReportOn(plSimDefs::kGroupAvatar);
    DetectorOverlap callbackFn(fOwner, entering, fQueuedCollideMsgs);
    const physx::PxQueryFlags hitFlags = physx::PxQueryFlag::eDYNAMIC |
                                         physx::PxQueryFlag::eSTATIC |
                                         physx::PxQueryFlag::eNO_BLOCK;
    fActor->getScene()->overlap(capsule, fActor->getGlobalPose(), callbackFn,
                                physx::PxQueryFilterData(filter, hitFlags));

    plDetectorLog::Log("Done informing from plPXPhysicalControllerCore::IInformDetectors");
}

void plPXPhysicalControllerCore::ICreateController(hsPoint3 pos)
{
    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();

    physx::PxCapsuleControllerDesc desc;
    desc.position.x       = pos.fX;
    desc.position.y       = pos.fY;
    desc.position.z       = pos.fZ + kCCTZOffset;
    desc.upDirection      = { 0.f, 0.f, 1.f };
    desc.slopeLimit       = kSlopeLimit;
    desc.contactOffset    = kCCTSkinWidth;
    desc.stepOffset       = kCCTStepOffset;
    desc.behaviorCallback = fBehaviorCallback.get();
    desc.reportCallback   = &gHitCallback;
    desc.nonWalkableMode  = physx::PxControllerNonWalkableMode::ePREVENT_CLIMBING;
    desc.userData         = new plPXActorData(this);
    desc.radius           = fRadius;
    desc.height           = fHeight;
    desc.climbingMode     = physx::PxCapsuleClimbingMode::eCONSTRAINED;
    fController           = (physx::PxCapsuleController*)sim->AddToWorld(desc, fWorldKey);
    fActor                = fController->getActor();

    plPXFilterData::Initialize(fActor, plSimDefs::kGroupAvatar, 0, fLOSDB);

    // In PhysX 2, the kinematic actors scale factor isn't exposed.
    // It is hardcoded at 0.8 which doesn't suit, so we have to manually adjust its dimensions.
    // TODO: PhysX 4.1 exposes the scale, but I'm not sure what the goal is here, so I'm not
    //       touching this disaster.
    physx::PxShape* shape;
    fActor->getShapes(&shape, 1);
    physx::PxCapsuleGeometry cap;
    shape->getCapsuleGeometry(cap);

    float kineRadius = fRadius + kCCTSkinWidth;
    float kineHeight = fHeight;
    if (fHuman) {
        physx::PxTransform xform = shape->getLocalPose();
        xform.p = { 0.f, (kPhysHeightCorrection / 2.0f), 0.f };
        shape->setLocalPose(xform);
        kineHeight += kPhysHeightCorrection;
    }
    cap.halfHeight = kineHeight;
    cap.radius = kineRadius;
    shape->setGeometry(cap);

    fSeeking = false;

    // Create proxy for the debug display
    /* FIXME
    // the avatar proxy doesn't seem to work... not sure why?
    hsColorRGBA physColor;
    float opac = 1.0f;

    // local avatar is light purple and transparent
    physColor.Set(.2f, .1f, .2f, 1.f);
    opac = 0.8f;

    fProxyGen = new plPhysicalProxy(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
    fProxyGen->Init(this);
    */
}
void plPXPhysicalControllerCore::IDeleteController()
{
    delete static_cast<plPXActorData*>(fActor->userData);
    fActor->userData = nullptr;
    if (fController)
        fController->setUserData(nullptr);

    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();
    sim->RemoveFromWorld(fController);

    fController = nullptr;
    fActor = nullptr;
}

void plPXPhysicalControllerCore::IDispatchQueuedMsgs()
{
    int numMsgs = fQueuedCollideMsgs.size();
    if (numMsgs)
    {
        plSimulationMgr* simMgr = plSimulationMgr::GetInstance();
        for (int i = 0; i < numMsgs; ++i)
            simMgr->AddCollisionMsg(fQueuedCollideMsgs[i]);

        fQueuedCollideMsgs.clear();
    }
}
void plPXPhysicalControllerCore::IProcessDynamicHits()
{
    int numHits = fDynamicHits.size();
    if (numHits)
    {
        plPXPhysical* phys;
        plSimulationMgr* simMgr = plSimulationMgr::GetInstance();
        for (int i = 0; i < numHits; ++i)
        {
            phys = fDynamicHits[i];

            // If this is the local avatar, we need to take ownership of this dynamic if we haven't already
            if (fLOSDB == plSimDefs::kLOSDBLocalAvatar && !phys->IsLocallyOwned() && !phys->GetProperty(plSimulationInterface::kNoOwnershipChange))
            {
                plSynchedObject* obj = plSynchedObject::ConvertNoRef(phys->GetObjectKey()->ObjectIsLoaded());
                obj->SetNetGroupConstant(plNetGroup::kNetGroupLocalPhysicals);

                plSetNetGroupIDMsg* setNetGroupID = new plSetNetGroupIDMsg;
                setNetGroupID->fId = plNetGroup::kNetGroupRemotePhysicals;
                setNetGroupID->SetBCastFlag(plMessage::kNetPropagate | plMessage::kNetForce);
                setNetGroupID->SetBCastFlag(plMessage::kLocalPropagate, false);
                setNetGroupID->Send(obj->GetKey());
            }

            phys->ApplyHitForce();
            simMgr->ConsiderSynch(phys, nil);
        }
        fDynamicHits.clear();
    }
}

#ifndef PLASMA_EXTERNAL_RELEASE
#include "pnNetCommon/plNetApp.h"
#include "plPipeline/plDebugText.h"

void plPXPhysicalControllerCore::IDrawDebugDisplay(int controllerIdx)
{
    plDebugText &debugTxt = plDebugText::Instance();
    ST::string debugString;
    int lineHeight = debugTxt.GetFontSize() + 4;
    int x = 10;     // Initial draw position
    static int y = 10;
    if (controllerIdx == 0)
    {
        y = 10;
        debugString = ST::format("Controller Count: {}", gControllers.size());
        debugTxt.DrawString(x, y, debugString);
        y += lineHeight;
    }

    const auto& controller = gControllers[controllerIdx];
    ST::string playerName = plNetClientApp::GetInstance()->GetPlayerName(controller->fOwner);

    debugString = ST::format("Kinematic CCT #{} [Name: {}] [Subworld: {}]",
                             controllerIdx + 1,
                             playerName.empty() ? controller->fOwner->GetName() : playerName,
                             controller->fWorldKey ? controller->fWorldKey->GetName() : "(main world)");
    debugTxt.DrawString(x, y, debugString);
    y += lineHeight;

    for (int i = 0; i < fDbgCollisionInfo.GetCount(); i++)
    {
        const hsVector3& normal = fDbgCollisionInfo[i].fNormal;
        float angle = hsRadiansToDegrees(acos(normal * hsVector3(0, 0, 1)));
        debugString = ST::format("\tCollision: {}, Normal: ({.2f}, {.2f}, {.2f}), Angle({.1f})",
                fDbgCollisionInfo[i].fSO->GetKeyName(),
                normal.fX, normal.fY, normal.fZ, angle);
        debugTxt.DrawString(x, y, debugString);
        y += lineHeight;
    }
}
#endif // PLASMA_EXTERNAL_RELEASE
