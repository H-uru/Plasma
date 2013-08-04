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
#include "plSimulationMgr.h"
#include "plPXPhysical.h"
#include "plPXConvert.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "plAvatar/plArmatureMod.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plDrawable/plDrawableGenerator.h"
#include "plPhysical/plPhysicalProxy.h"
#include "pnMessage/plSetNetGroupIDMsg.h"
#include "plMessage/plCollideMsg.h"
#include "plModifier/plDetectorLog.h"

#include "plSurface/hsGMaterial.h"      // For our proxy
#include "plSurface/plLayerInterface.h" // For our proxy

#include "NxPhysics.h"
#include "NxCapsuleController.h"
#include "NxCapsuleShape.h"
#include "ControllerManager.h"

static ControllerManager gControllerMgr;
static std::vector<plPXPhysicalControllerCore*> gControllers;
static bool gRebuildCache = false;

#ifndef PLASMA_EXTERNAL_RELEASE
bool plPXPhysicalControllerCore::fDebugDisplay = false;
#endif // PLASMA_EXTERNAL_RELEASE
int plPXPhysicalControllerCore::fPXControllersMax = 0;

#define kCCTSkinWidth 0.1f
#define kCCTStepOffset 0.7f
#define kCCTZOffset ((fRadius + (fHeight / 2)) + kCCTSkinWidth)
#define kPhysHeightCorrection 0.8f
#define kPhysZOffset ((kCCTZOffset + (kPhysHeightCorrection / 2)) - 0.05f)
#define kAvatarMass 200.0f

static class PXControllerHitReport : public NxUserControllerHitReport
{
public:
    virtual NxControllerAction onShapeHit(const NxControllerShapeHit& hit)
    {
        plPXPhysicalControllerCore* controller = (plPXPhysicalControllerCore*)hit.controller->getUserData();
        NxActor& actor = hit.shape->getActor();
        plPXPhysical* phys = (plPXPhysical*)actor.userData;
        hsVector3 normal = plPXConvert::Vector(hit.worldNormal);

#ifndef PLASMA_EXTERNAL_RELEASE
        plDbgCollisionInfo info;
        info.fNormal = normal;
        info.fSO = plSceneObject::ConvertNoRef(phys->GetObjectKey()->ObjectIsLoaded());

        NxCapsule capsule;
        controller->GetWorldSpaceCapsule(capsule);
        info.fOverlap = hit.shape->checkOverlapCapsule(capsule);

        controller->fDbgCollisionInfo.Append(info);
#endif PLASMA_EXTERNAL_RELEASE

        // If the avatar hit a movable physical, apply some force to it.
        if (actor.isDynamic())
        {
            if (!actor.readBodyFlag(NX_BF_KINEMATIC) && !actor.readBodyFlag(NX_BF_FROZEN))
            {
                // Don't apply force when standing on top of an object.
                if (normal.fZ < 0.85f)
                {
                    hsVector3 velocity = controller->GetLinearVelocity();
                    velocity.fZ = 0.0f;
                    float length = velocity.Magnitude();
                    if (length > 0)
                    {
                        // Only allow horizontal pushes for now
                        NxVec3 hitDir = hit.worldPos - hit.controller->getPosition();
                        hitDir.z = 0.0f;
                        hitDir.normalize();

                        // Get controller speed along the hitDir
                        float cctProj = velocity.fX * hitDir.x + velocity.fY * hitDir.y;
                        length = length + cctProj / 2.0f;

                        // Get hit actors speed along the hitDir
                        float hitProj = actor.getLinearVelocity().dot(hitDir);
                        if (hitProj > 0)
                            length -= hitProj;

                        length *= kAvatarMass;

                        hsPoint3 pos((float)hit.worldPos.x, (float)hit.worldPos.y, (float)hit.worldPos.z);
                        phys->SetHitForce(plPXConvert::Vector(hitDir * length), pos);
                        controller->AddDynamicHit(phys);
                    }
                }
            }
        }
        else  // else if the avatar hit a static
        {
            controller->fMovementStrategy->AddContactNormals(normal);
            return NX_ACTION_NONE;
        }
        if (phys && phys->GetProperty(plSimulationInterface::kAvAnimPushable))
        {
            hsQuat inverseRotation = controller->fLocalRotation.Inverse();
            hsVector3 normal = plPXConvert::Vector(hit.worldNormal);
            controller->SetPushingPhysical(phys);
            controller->SetFacingPushingPhysical((inverseRotation.Rotate(&kAvatarForward).InnerProduct(normal) < 0 ? true : false));
        }
        return NX_ACTION_NONE;
    }
    virtual NxControllerAction onControllerHit(const NxControllersHit& hit)
    {
        return NX_ACTION_NONE;
    }
} gControllerHitReport;

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
    : plPhysicalControllerCore(ownerSO, height, radius),
    fController(nil),
    fActor(nil),
    fProxyGen(nil),
    fKinematicCCT(true),
    fHuman(human)
{
    ICreateController(fLocalPosition);
    fActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
    gControllers.push_back(this);
}
plPXPhysicalControllerCore::~plPXPhysicalControllerCore()
{
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        if (gControllers[i] == this)
        {
            gControllers.erase(gControllers.begin()+i);
            break;
        }
    }
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
    if (fEnabled != enable)
    {
        fEnabled = enable;
        if (fEnabled)
        {
            // Defer until the next physics update.
            fEnableChanged = true;
        }
        else
        {
            if (!fKinematicCCT)
            {
                // Dynamic controllers are forced kinematic
                fActor->raiseBodyFlag(NX_BF_KINEMATIC);
                NxShape* shape = fActor->getShapes()[0];
                shape->setGroup(plSimDefs::kGroupAvatarKinematic);
            }
        }
    }
}

void plPXPhysicalControllerCore::SetSubworld(plKey world) 
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
        RebuildCache();
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

void plPXPhysicalControllerCore::SetMovementStrategy(plMovementStrategy* strategy)
{
    if (fKinematicCCT != strategy->IsKinematic())
    {
        IDeleteController();
        fKinematicCCT = !fKinematicCCT;
        ICreateController(fLocalPosition);
    }

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

    // Update the physical position
    if (fKinematicCCT)
    {
        hsVector3 disp(&fLocalPosition, &prevPosition);
        if (disp.Magnitude() > 2.f)
        {
            // Teleport the underlying actor most of the way
            disp.Normalize();
            disp *= 0.001f;

            hsPoint3 teleportPos = fLocalPosition - disp;
            NxVec3 pos(teleportPos.fX, teleportPos.fY, teleportPos.fZ + kPhysZOffset);
            fActor->setGlobalPosition(pos);
        }

        NxExtendedVec3 extPos(fLocalPosition.fX, fLocalPosition.fY, fLocalPosition.fZ + kCCTZOffset);
        fController->setPosition(extPos);
    }
    else
    {
        NxVec3 pos(fLocalPosition.fX, fLocalPosition.fY, fLocalPosition.fZ + kPhysZOffset);
        if (fActor->readBodyFlag(NX_BF_KINEMATIC))
            fActor->moveGlobalPosition(pos);
        else
            fActor->setGlobalPosition(pos);
    }
}

void plPXPhysicalControllerCore::GetPositionSim(hsPoint3& pos)
{
    if (fKinematicCCT)
    {
        const NxExtendedVec3& extPos = fController->getPosition();
        pos.Set((float)extPos.x, (float)extPos.y, (float)extPos.z - kCCTZOffset);
    }
    else
    {
        NxVec3 nxPos = fActor->getGlobalPosition();
        pos.Set(nxPos.x, nxPos.y, nxPos.z - kPhysZOffset);
    }
}

void plPXPhysicalControllerCore::Move(hsVector3 displacement, unsigned int collideWith, unsigned int &collisionResults)
{
    NxU32 colFlags = 0;

    fController->move(plPXConvert::Vector(displacement), collideWith, 0.00001f, colFlags);

    collisionResults = 0;
    if (colFlags & NXCC_COLLISION_DOWN)
        collisionResults |= kBottom;
    if (colFlags & NXCC_COLLISION_UP)
        collisionResults |= kTop;
    if (colFlags & NXCC_COLLISION_SIDES)
        collisionResults |= kSides;
}

void plPXPhysicalControllerCore::SetLinearVelocitySim(const hsVector3& linearVel)
{
    if (!fKinematicCCT)
    {
        NxVec3 vel = plPXConvert::Vector(linearVel);
        fActor->setLinearVelocity(vel);
    }
}

int plPXPhysicalControllerCore::SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, bool vsDynamics, bool vsStatics,
                            uint32_t& vsSimGroups, std::vector<plControllerSweepRecord>& hits)
{
    NxSweepQueryHit queryHit[10];

    unsigned int flags = NX_SF_ALL_HITS;
    if (vsDynamics)
        flags |= NX_SF_DYNAMICS;
    if (vsStatics)
        flags |= NX_SF_STATICS;

    NxVec3 vec;
    vec.x = endPos.fX - startPos.fX;
    vec.y = endPos.fY - startPos.fY;
    vec.z = endPos.fZ - startPos.fZ;

    NxShape* shape = fActor->getShapes()[0];
    NxCapsuleShape* capShape = shape->isCapsule();
    float radius = capShape->getRadius();
    float height = capShape->getHeight();

    NxCapsule capsule;
    capsule.p0 = plPXConvert::Point(startPos);
    capsule.p0.z = capsule.p0.z + radius;
    capsule.radius = radius;
    capsule.p1 = capsule.p0;
    capsule.p1.z = capsule.p1.z + height;

    NxScene *scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
    int numHits = scene->linearCapsuleSweep(capsule, vec, flags, nil, 10, queryHit, nil, vsSimGroups);
    if (numHits)
    {
        for (int i = 0; i < numHits; ++i)
        {
            plControllerSweepRecord currentHit;
            currentHit.ObjHit = (plPhysical*)queryHit[i].hitShape->getActor().userData;
            if (currentHit.ObjHit)
            {
                currentHit.Point = plPXConvert::Point(queryHit[i].point);
                currentHit.Normal = plPXConvert::Vector(queryHit[i].normal);
                hits.push_back(currentHit);
            }
        }
    }

    return hits.size();
}

void plPXPhysicalControllerCore::LeaveAge()
{
    SetPushingPhysical(nil);
    if (fWorldKey)
        SetSubworld(nil);

    // Disable all collisions
    fActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
}

void plPXPhysicalControllerCore::GetWorldSpaceCapsule(NxCapsule& cap) const
{
    NxShape* shape = fActor->getShapes()[0];
    NxCapsuleShape* capShape = shape->isCapsule();
    capShape->getWorldCapsule(cap);
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
        if (gRebuildCache && controller->fController)
            controller->fController->reportSceneChanged();

#ifndef PLASMA_EXTERNAL_RELEASE
        controller->fDbgCollisionInfo.SetCount(0);
#endif

        controller->IDispatchQueuedMsgs();
        controller->IApply(delSecs);
        controller->IProcessDynamicHits();
    }

    gRebuildCache = false;
}
void plPXPhysicalControllerCore::Update(int numSubSteps, float alpha)
{
    gControllerMgr.updateControllers();

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

void plPXPhysicalControllerCore::RebuildCache() { gRebuildCache = true; }

plPXPhysicalControllerCore* plPXPhysicalControllerCore::GetController(NxActor& actor)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];
        if (controller->fActor == &actor)
            return controller;
    }

    return nil;
}

bool plPXPhysicalControllerCore::AnyControllersInThisWorld(plKey world)
{
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];
        if (controller->GetSubworld() == world)
            return true;
    }

    return false;
}
int plPXPhysicalControllerCore::GetNumberOfControllersInThisSubWorld(plKey world)
{
    int count = 0;
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];
        if (controller->GetSubworld() == world)
            ++count;
    }

    return count;
}
int plPXPhysicalControllerCore::GetControllersInThisSubWorld(plKey world, int maxToReturn, plPXPhysicalControllerCore** bufferout)
{
    int count = 0;
    plPXPhysicalControllerCore* controller;
    int numControllers = gControllers.size();
    for (int i = 0; i < numControllers; ++i)
    {
        controller = gControllers[i];
        if (controller->GetSubworld() == world)
        {
            if (count < maxToReturn)
            {
                bufferout[count] = controller;
                ++count;
            }
        }
    }

    return count;
}

int plPXPhysicalControllerCore::NumControllers() { return gControllers.size(); }

void plPXPhysicalControllerCore::IHandleEnableChanged()
{
    // Defered enable
    fEnableChanged = false;

    if (!fKinematicCCT)
    {
        // Restore dynamic controller
        fActor->clearBodyFlag(NX_BF_KINEMATIC);
        NxShape* shape = fActor->getShapes()[0];
        shape->setGroup(plSimDefs::kGroupAvatar);
    }

    // Enable actor collisions
    if (fActor->readActorFlag(NX_AF_DISABLE_COLLISION))
        fActor->clearActorFlag(NX_AF_DISABLE_COLLISION);
}

void plPXPhysicalControllerCore::IInformDetectors(bool entering)
{
    static const NxU32 kDetectorFlag = 1<<plSimDefs::kGroupDetector;
    static const int kNumShapes = 30;

    DetectorLog("Informing from plPXPhysicalControllerCore::IInformDetectors");

    NxShape* shapes[kNumShapes];
    NxCapsule capsule;
    GetWorldSpaceCapsule(capsule);
    NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
    int numCollided = scene->overlapCapsuleShapes(capsule, NX_ALL_SHAPES, kNumShapes, shapes, NULL, kDetectorFlag, NULL, true);
    for (int i = 0; i < numCollided; ++i)
    {
        plPXPhysical* physical = (plPXPhysical*)shapes[i]->getActor().userData;
        if (physical && physical->DoReportOn(plSimDefs::kGroupAvatar))
        {
            plCollideMsg* msg = new plCollideMsg();
            msg->fOtherKey = fOwner;
            msg->fEntering = entering;
            msg->AddReceiver(physical->GetObjectKey());

            // Queue until the next sim step
            fQueuedCollideMsgs.push_back(msg);
        }
    }

    DetectorLog("Done informing from plPXPhysicalControllerCore::IInformDetectors");
}

void plPXPhysicalControllerCore::ICreateController(const hsPoint3& pos)
{
    NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);

    if (fKinematicCCT)
    {
        // Use PhysX character controller
        NxCapsuleControllerDesc desc;
        desc.position.x     = pos.fX;
        desc.position.y     = pos.fY;
        desc.position.z     = pos.fZ + kCCTZOffset;
        desc.upDirection    = NX_Z;
        desc.slopeLimit     = kSlopeLimit;
        desc.skinWidth      = kCCTSkinWidth;
        desc.stepOffset     = kCCTStepOffset;
        desc.callback       = &gControllerHitReport;
        desc.userData       = this;
        desc.radius         = fRadius;
        desc.height         = fHeight;
        desc.interactionFlag = NXIF_INTERACTION_EXCLUDE;
        fController = (NxCapsuleController*)gControllerMgr.createController(scene, desc);
        fActor = fController->getActor();

        // Set the actor group - Dynamics are in group 1 and will report on everything in group 0.
        // We don't want notifications
        fActor->setGroup(2);

        // Set the shape group.  Not used by the NxController itself,
        // But required for correct group interactions in the simulation.
        NxShape* shape = fActor->getShapes()[0];
        shape->setGroup(plSimDefs::kGroupAvatarKinematic);

        // In PhysX 2, the kinematic actors scale factor isn't exposed.
        // It is hardcoded at 0.8 which doesn't suit, so we have to manually adjust its dimensions.
        float kineRadius = fRadius + kCCTSkinWidth;
        float kineHeight = fHeight;
        NxCapsuleShape* capShape = shape->isCapsule();
        if (fHuman)
        {
            kineHeight += kPhysHeightCorrection;
            capShape->setLocalPosition(NxVec3(0.0f, (kPhysHeightCorrection / 2.0f), 0.0f));
        }
        capShape->setDimensions(kineRadius, kineHeight);
    }
    else
    {
        // Use dynamic actor for the character controller
        NxCapsuleShapeDesc capDesc;
        capDesc.materialIndex = plSimulationMgr::GetInstance()->GetMaterialIdx(scene, 0.0f, 0.0f);
        capDesc.radius = fRadius + kCCTSkinWidth;
        capDesc.height = fHeight + kPhysHeightCorrection;

        NxBodyDesc bodyDesc;
        bodyDesc.mass = kAvatarMass;
        bodyDesc.flags = NX_BF_DISABLE_GRAVITY;
        bodyDesc.flags |= NX_BF_FROZEN_ROT;

        if (fEnabled)
            capDesc.group = plSimDefs::kGroupAvatar;
        else
        {
            bodyDesc.flags |= NX_BF_KINEMATIC;
            capDesc.group = plSimDefs::kGroupAvatarKinematic;
        }

        NxActorDesc actorDesc;
        actorDesc.shapes.pushBack(&capDesc);
        actorDesc.body = &bodyDesc;
        actorDesc.group = 2;

        actorDesc.globalPose.M.rotX(NxHalfPiF32);
        actorDesc.globalPose.t.x = pos.fX;
        actorDesc.globalPose.t.y = pos.fY;
        actorDesc.globalPose.t.z = pos.fZ + kPhysZOffset;

        fActor = scene->createActor(actorDesc);
    }

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
    if (fKinematicCCT)
    {
        gControllerMgr.releaseController(*fController);
        fController = nil;
    }
    else
    {
        NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
        scene->releaseActor(*fActor);
    }

    fActor = nil;
    plSimulationMgr::GetInstance()->ReleaseScene(fWorldKey);
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
#include "../plPipeline/plDebugText.h"

void plPXPhysicalControllerCore::IDrawDebugDisplay(int controllerIdx)
{
    plDebugText &debugTxt = plDebugText::Instance();
    plString debugString;
    int lineHeight = debugTxt.GetFontSize() + 4;
    int x = 10;     // Initial draw position
    static int y = 10;
    if (controllerIdx == 0)
    {
        y = 10;
        debugString = plString::Format("Controller Count: %d", gControllers.size());
        debugTxt.DrawString(x, y, debugString.c_str());
        y += lineHeight;
    }

    // Only display avatar collisions if any exist...
    int collisionCount = fDbgCollisionInfo.GetCount();
    if (collisionCount > 0)
    {
        debugString = plString::Format("Controller #%d (%s) Collisions:",
            controllerIdx + 1, gControllers[controllerIdx]->fOwner->GetName().c_str());
        debugTxt.DrawString(x, y, debugString.c_str());
        y += lineHeight;

        for (int i = 0; i < collisionCount; i++)
        {
            hsVector3 normal = fDbgCollisionInfo[i].fNormal;
            const char* overlapStr = fDbgCollisionInfo[i].fOverlap ? "yes" : "no";
            float angle = hsRadiansToDegrees(acos(normal * hsVector3(0, 0, 1)));
            debugString = plString::Format("\tObj: %s, Normal: (%.2f, %.2f, %.2f), Angle(%.1f), Overlap(%s)",
                    fDbgCollisionInfo[i].fSO->GetKeyName().c_str(),
                    normal.fX, normal.fY, normal.fZ, angle,
                    overlapStr);
            debugTxt.DrawString(x, y, debugString.c_str());
            y += lineHeight;
        }
    }
}
#endif PLASMA_EXTERNAL_RELEASE

