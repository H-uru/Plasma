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
#include "plPhysicalControllerCore.h"

#include "plPhysical.h"

#include <algorithm>
#include <cmath>

#include "plArmatureMod.h"
#include "plAvatarMgr.h"
#include "plSwimRegion.h"

#include "pnMessage/plCorrectionMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plAnimation/plMatrixChannel.h"

// Gravity constants
#define kGravity -32.174f
#define kTerminalVelocity kGravity

static inline hsVector3 GetYAxis(hsMatrix44 &mat) { return hsVector3(mat.fMap[1][0], mat.fMap[1][1], mat.fMap[1][2]); }
static float AngleRad2d(float x1, float y1, float x3, float y3);

// plPhysicalControllerCore
plPhysicalControllerCore::plPhysicalControllerCore(plKey OwnerSceneObject, float height, float radius)
    : fOwner(std::move(OwnerSceneObject)),
    fWorldKey(),
    fHeight(height),
    fRadius(radius),
    fLOSDB(plSimDefs::kLOSDBNone),
    fMovementStrategy(),
    fSimLength(),
    fFlags(),
    fLocalPosition(0.0f, 0.0f, -2000.0f),
    fPushingPhysical(),
    fFacingPushingPhysical()
{
    fLastGlobalLoc.Reset();
    fPrevSubworldW2L.Reset();
}

const plCoordinateInterface* plPhysicalControllerCore::GetSubworldCI()
{
    if (fWorldKey)
    {
        plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
        if (so)
            return so->GetCoordinateInterface();
    }

    return nullptr;
}

void plPhysicalControllerCore::IncrementAngle(float deltaAngle)
{
    /// FIXME: this local rotation thing assumes +Z is up
    hsVector3 axis;
    float angle;

    fLocalRotation.NormalizeIfNeeded();
    fLocalRotation.GetAngleAxis(&angle, &axis);
    if (axis.fZ < 0)
        angle = hsConstants::two_pi<float> - angle; // axis is backwards, so reverse the angle too

    angle += deltaAngle;

    // make sure we wrap around
    if (angle < 0.0f)
        angle += hsConstants::two_pi<float>; // angle is -, so this works like a subtract
    if (angle >= hsConstants::two_pi<float>)
        angle -= hsConstants::two_pi<float>;

    // set the new angle
    axis.Set(0.0f, 0.0f, 1.0f);
    fLocalRotation.SetAngleAxis(angle, axis);
}

void plPhysicalControllerCore::IApply(float delSecs)
{
    fSimLength = delSecs;

    // Match controller to owner if transform has changed since the last frame
    plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
    const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
    if (!fLastGlobalLoc.Compare(l2w, 0.0001f))
        SetGlobalLoc(l2w);

    if (IsEnabled())
    {
        // Convert velocity from avatar to world space
        if (!fLinearVelocity.IsEmpty())
        {
            fLinearVelocity = l2w * fLinearVelocity;

            const plCoordinateInterface* subworldCI = GetSubworldCI();
            if (subworldCI)
                fLinearVelocity = subworldCI->GetWorldToLocal() * fLinearVelocity;
        }

        fMovementStrategy->Apply(delSecs);
    }
}
void plPhysicalControllerCore::IUpdate(int numSubSteps, float alpha)
{
    if (IsEnabled())
    {
        // Update local position and acheived velocity
        fLastLocalPosition = fLocalPosition;
        GetPositionSim(fLocalPosition);
        hsVector3 displacement = (hsVector3)(fLocalPosition - fLastLocalPosition);
        fAchievedLinearVelocity = displacement / fSimLength;

        displacement /= (float)numSubSteps;
        fLastLocalPosition = fLocalPosition - displacement;
        hsPoint3 interpLocalPos = fLastLocalPosition + (displacement * alpha);

        // Update global location
        fLocalRotation.MakeMatrix(&fLastGlobalLoc);
        fLastGlobalLoc.SetTranslate(&interpLocalPos);
        const plCoordinateInterface* subworldCI = GetSubworldCI();
        if (subworldCI)
        {
            const hsMatrix44& subL2W = subworldCI->GetLocalToWorld();
            fLastGlobalLoc = subL2W * fLastGlobalLoc;
            fPrevSubworldW2L = subworldCI->GetWorldToLocal();
        }

        fMovementStrategy->Update(fSimLength);
        ISendCorrectionMessages(true);
    }
    else
    {
        fAchievedLinearVelocity.Set(0.0f, 0.0f, 0.0f);

        // Update global location if in a subworld
        const plCoordinateInterface* subworldCI = GetSubworldCI();
        if (subworldCI)
        {
            hsMatrix44 l2s = fPrevSubworldW2L * fLastGlobalLoc;
            const hsMatrix44& subL2W = subworldCI->GetLocalToWorld();
            fLastGlobalLoc = subL2W * l2s;
            fPrevSubworldW2L = subworldCI->GetWorldToLocal();

            ISendCorrectionMessages();
        }
    }
}
void plPhysicalControllerCore::IUpdateNonPhysical(float alpha)
{
    // Update global location if owner transform hasn't changed.
    plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
    const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
    if (fLastGlobalLoc.Compare(l2w, 0.0001f))
    {
        if (IsEnabled())
        {
            hsVector3 displacement = (hsVector3)(fLocalPosition - fLastLocalPosition);
            hsPoint3 interpLocalPos = fLastLocalPosition + (displacement * alpha);

            fLocalRotation.MakeMatrix(&fLastGlobalLoc);
            fLastGlobalLoc.SetTranslate(&interpLocalPos);
            const plCoordinateInterface* subworldCI = GetSubworldCI();
            if (subworldCI)
            {
                const hsMatrix44& subL2W = subworldCI->GetLocalToWorld();
                fLastGlobalLoc = subL2W * fLastGlobalLoc;
                fPrevSubworldW2L = subworldCI->GetWorldToLocal();
            }

            ISendCorrectionMessages();
        }
        else
        {
            // Update global location if in a subworld
            const plCoordinateInterface* subworldCI = GetSubworldCI();
            if (subworldCI)
            {
                hsMatrix44 l2s = fPrevSubworldW2L * fLastGlobalLoc;
                const hsMatrix44& subL2W = subworldCI->GetLocalToWorld();
                fLastGlobalLoc = subL2W * l2s;
                fPrevSubworldW2L = subworldCI->GetWorldToLocal();


                ISendCorrectionMessages();
            }
        }
    }
}

void plPhysicalControllerCore::ISendCorrectionMessages(bool dirtySynch)
{
    plCorrectionMsg* corrMsg = new plCorrectionMsg();
    corrMsg->fLocalToWorld = fLastGlobalLoc;
    corrMsg->fLocalToWorld.GetInverse(&corrMsg->fWorldToLocal);
    corrMsg->fDirtySynch = dirtySynch;
    corrMsg->AddReceiver(fOwner);
    corrMsg->Send();
}


// Movement Strategy
plMovementStrategy::plMovementStrategy(plPhysicalControllerCore* controller)
    : fController(controller)
{
}

void plMovementStrategy::Reset(bool newAge) { fController->SetMovementStrategy(this); }


// Animated Movement Strategy
plAnimatedMovementStrategy::plAnimatedMovementStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller)
    : plMovementStrategy(controller),
    fRootApp(rootApp),
    fAnimLinearVel(0.0f, 0.0f, 0.0f),
    fAnimAngularVel(0.0f),
    fTurnStr(0.0f)
{
}

void plAnimatedMovementStrategy::RecalcVelocity(double timeNow, float elapsed, bool useAnim)
{
    if (useAnim)
    {
        // while you may think it would be correct to cache this, what we're actually asking is "what would the animation's
        // position be at the previous time given its *current* parameters (particularly blends)"
        hsMatrix44 prevMat = ((plMatrixChannel *)fRootApp->GetChannel())->Value(timeNow - elapsed, true);
        hsMatrix44 curMat = ((plMatrixChannel *)fRootApp->GetChannel())->Value(timeNow, true);
        
        IRecalcLinearVelocity(elapsed, prevMat, curMat);
        IRecalcAngularVelocity(elapsed, prevMat, curMat);
    }
    else
    {
        fAnimLinearVel.Set(0.0f, 0.0f, 0.0f);
        fAnimAngularVel = 0.0f;
    }

    // Update controller rotation
    float zRot = fAnimAngularVel + fTurnStr;
    if (fabs(zRot) > 0.0001f)
        fController->IncrementAngle(zRot * elapsed);

    // Update controller velocity
    fController->SetLinearVelocity(fAnimLinearVel);
}

void plAnimatedMovementStrategy::IRecalcLinearVelocity(float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat)
{
    hsPoint3 startPos;                                      // default position (at start of anim)
    hsPoint3 prevPos = prevMat.GetTranslate();              // position previous frame
    hsPoint3 nowPos = curMat.GetTranslate();                // position current frame

    hsVector3 prev2Now = (hsVector3)(nowPos - prevPos);     // frame-to-frame delta

    if (fabs(prev2Now.fX) < 0.0001f && fabs(prev2Now.fY) < 0.0001f && fabs(prev2Now.fZ) < 0.0001f)
    {
        fAnimLinearVel.Set(0.f, 0.f, 0.f);
    }
    else
    {
        hsVector3 start2Now = (hsVector3)(nowPos - startPos);    // start-to-frame delta

        float dot = prev2Now.InnerProduct(start2Now);

        // HANDLING ANIMATION WRAPPING:
        // the vector from the animation origin to the current frame should point in roughly
        // the same direction as the vector from the previous animation position to the
        // current animation position.
        //
        // If they don't agree (dot < 0,) then we probably mpst wrapped around.
        // The right answer would be to compare the current frame to the start of
        // the anim loop, but it's cheaper to cheat and use the previous frame's velocity.
        if (dot > 0.0f)
        {
            prev2Now /= elapsed;

            float xfabs = fabs(prev2Now.fX);
            float yfabs = fabs(prev2Now.fY);
            float zfabs = fabs(prev2Now.fZ);
            static const float maxVel = 20.0f;
            bool valid = xfabs < maxVel && yfabs < maxVel && zfabs < maxVel;

            if (valid)
            {
                fAnimLinearVel = prev2Now;
            }
        }
    }
}

void plAnimatedMovementStrategy::IRecalcAngularVelocity(float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat)
{
    fAnimAngularVel = 0.0f;
    float appliedVelocity = 0.0f;
    hsVector3 prevForward = GetYAxis(prevMat);
    hsVector3 curForward = GetYAxis(curMat);

    float angleSincePrev = AngleRad2d(curForward.fX, curForward.fY, prevForward.fX, prevForward.fY);
    bool sincePrevSign = angleSincePrev > 0.0f;
    if (angleSincePrev > hsConstants::pi<float>)
        angleSincePrev -= hsConstants::two_pi<float>;

    const hsVector3 startForward = hsVector3(0.0f, -1.0f, 0.0f);    // the Y orientation of a "resting" armature....
    float angleSinceStart = AngleRad2d(curForward.fX, curForward.fY, startForward.fX, startForward.fY);
    bool sinceStartSign = angleSinceStart > 0.0f;
    if (angleSinceStart > hsConstants::pi<float>)
        angleSinceStart -= hsConstants::two_pi<float>;

    // HANDLING ANIMATION WRAPPING:
    // under normal conditions, the angle from rest to the current frame will have the same
    // sign as the angle from the previous frame to the current frame.
    // if it does not, we have (most likely) wrapped the motivating animation from frame n back
    // to frame zero, creating a large angle from the previous frame to the current one
    if (sincePrevSign == sinceStartSign)
    {
        // signs are the same; didn't wrap; use the frame-to-frame angle difference
        appliedVelocity = angleSincePrev / elapsed;	// rotation / time
        if (fabs(appliedVelocity) < 3)
        {
            fAnimAngularVel = appliedVelocity;
        }
    }
}

// Walking Strategy
plWalkingStrategy::plWalkingStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller)
    : plAnimatedMovementStrategy(rootApp, controller),
    fImpactVelocity(0.0f, 0.0f, 0.0f),
    fImpactTime(),
    fTimeInAir(),
    fControlledFlightTime(),
    fControlledFlight(),
    fFlags()
{
}

static inline bool IFilterGround(const plControllerHitRecord& candidate, float zMax)
{
    if (candidate.Point.fZ >= zMax)
        return false;
    // Removes near-perpendicular turds
    if (candidate.Normal.fZ < 0.02f)
        return false;
    return true;
}

std::optional<plControllerHitRecord> plWalkingStrategy::IFindGroundCandidate() const
{
    hsPoint3 pos;
    fController->GetPositionSim(pos);
    float zMax = pos.fZ + (fController->GetRadius() * 0.75f);

    std::optional<plControllerHitRecord> result = std::nullopt;
    for (const auto& i : fContacts) {
        if (!IFilterGround(i, zMax))
            continue;
        if (result && result->Normal.fZ > i.Normal.fZ)
            continue;
        result = i;
    }
    return result;
}

void plWalkingStrategy::Apply(float delSecs)
{
    hsVector3 velocity = fController->GetLinearVelocity();
    hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

    if (!(fFlags & kGroundContact)) {
        // Maintain momentum from the previous frame
        if (std::abs(velocity.fX) < 0.001f)
            velocity.fX = achievedVelocity.fX;
        if (std::abs(velocity.fY) < 0.001f)
            velocity.fY = achievedVelocity.fY;
        if (std::fabs(velocity.fZ) < 0.001f)
            velocity.fZ = achievedVelocity.fZ + (kGravity * delSecs);
    } else if (fFlags & kFallingNormal) {
        if (std::fabs(velocity.fZ) < 0.001f) {
            velocity.fZ = std::min(0.f, achievedVelocity.fZ) + (kGravity * delSecs);
            hsVector3 velNorm = velocity;
            hsVector3 offset;
            for (const auto& collision : fContacts) {
                if (collision.Normal.fZ >= GetFallStopThreshold())
                    continue;
                offset += collision.Normal;
                if (velNorm.MagnitudeSquared() > 0.0f)
                    velNorm.Normalize();
                if (velNorm * collision.Normal < 0.f) {
                    hsVector3 proj = (velNorm % collision.Normal) % collision.Normal;
                    if (velNorm * proj < 0.0f)
                        proj *= -1.0f;

                    velocity = velocity.Magnitude() * proj;
                }
            }
            if (offset.MagnitudeSquared() > 0.0f) {
                offset.Normalize();
                velocity += offset * 5.0f;
            }
        }
    } else {
        // OK, so, you're on the ground and not falling down a cliff. Great. BUT WHAT IS THIS GOUND?
        // If the ground can move, we should inherit the higher of our anticipated velocity due to
        // gravity or the Z velocity of the ground... unless the ride platform bits were set, in
        // which case we just add the platform's velocity to ours and move along with life.
        hsVector3 groundVel;
        auto ground = IFindGroundCandidate();
        if (!ground || !ground->GetPhysical()) {
            // This will happen if the ground got paged out from under us.
            // Yes, I am looking at you, Minkata.
            velocity.fZ = achievedVelocity.fZ + (kGravity * delSecs);
        } else if (ground->GetPhysical()->GetLinearVelocitySim(groundVel)) {
            // Don't want to inherit the velocity of a kickable -- that results in near infinite
            // acceleration, which sucks.
            if ((fFlags & kRidePlatform) && ground->GetPhysical()->GetGroup() != plSimDefs::kGroupDynamic) {
                velocity.fX += groundVel.fX;
                velocity.fY += groundVel.fY;
            }
            if (std::fabs(velocity.fZ) < 0.001f) {
                if (std::fabs(groundVel.fZ) < 0.001f) {
                    float zcomp = 1.f - std::max(0.f, ground->Normal.fZ);
                    velocity.fZ = achievedVelocity.fZ + (kGravity * zcomp * delSecs);
                } else {
                    velocity.fZ = groundVel.fZ;
                }
            }
        } else if (std::fabs(velocity.fZ) < 0.001f) {
            float zcomp = 1.f - std::max(0.f, ground->Normal.fZ);
            velocity.fZ = achievedVelocity.fZ + (kGravity * zcomp * delSecs);
        }

        // Kill upward velocity if we're just running along the ground.
        if (ground && !IsControlledFlight())
            velocity.fZ = std::min(0.f, velocity.fZ);
    }

    // Limit final requested downward velocity to the magnitude of the acceleration of gravity.
    // It's not the way R/L works, but it yields a better visual result.
    if (!(fFlags & kGroundContact))
        velocity.fZ = std::max(velocity.fZ, kGravity);

    // Reset vars and move the controller
    fController->SetPushingPhysical(nullptr);
    fController->SetFacingPushingPhysical(false);
    fContacts.clear();

    // If we are jumping against an object, our z-displacement may be killed by PhysX, so we must
    // force the displacement. But, also, we need to make sure we don't get wedged into a stuck
    // position or thrown back due to depenetrations.
    if (IsControlledFlight()) {
        hsPoint3 startPos;
        fController->GetPositionSim(startPos);
        hsPoint3 endPos = startPos;
        endPos.fZ += velocity.fZ * delSecs;
        velocity.fZ = 0.f;

        const plSimDefs::Group simGroups = (plSimDefs::Group)((1 << plSimDefs::kGroupStatic) |
                                                              (1 << plSimDefs::kGroupAvatarBlocker));
        auto sweep = fController->SweepMulti(startPos, endPos, simGroups, true);
        for (const auto& hit : sweep) {
            // Hopefully prevent forced penetrations.
            if (hit.Displacement < 0.f)
                endPos += hit.Normal * hit.Displacement * -1.f;
        }
        fController->SetPositionSim(endPos);
    }

    fController->SetLinearVelocitySim(velocity);
}

void plWalkingStrategy::ICheckGroundSteepness(const plControllerHitRecord& ground)
{
    if (ground.GetPhysical() && ground.GetPhysical()->GetGroup() != plSimDefs::kGroupDynamic) {
        if (ground.Normal.fZ >= GetFallStopThreshold())
            fFlags &= ~kFallingNormal;
        else if (ground.Normal.fZ < GetFallStartThreshold())
            fFlags |= kFallingNormal;
    } else {
        fFlags &= ~kFallingNormal;
    }
}

void plWalkingStrategy::IStepUp(const plControllerHitRecord& ground, float delSecs)
{
    // No, I'm, not going to help you glitch up cliffs.
    if (ground.Normal.fZ < GetFallStopThreshold())
        return;

    // We are on the ground - good. However, there is a problem. Some ground colliders are
    // built sloppily such that they intersect at (near) perpendicular angles and are just
    // slightly too tall for the simulation to handle. So, if the actor did not move much this
    // frame, we want to offset the character by some "step height" and sweep along the desired
    // movement path. If we find that the shortest hit is a nontrival displacement, we should
    // step up and move that far.
    const hsVector3& animVelocity = fController->GetLinearVelocity();
    const hsVector3& simVelocity = fController->GetAchievedLinearVelocity();
    if (animVelocity.MagnitudeSquared() < 0.0001f || simVelocity.MagnitudeSquared() >= 0.0001f)
        return;

    hsPoint3 startPos;
    fController->GetPositionSim(startPos);
    hsPoint3 endPos = startPos + (animVelocity * delSecs * 2.5f);
    startPos.fZ += fController->GetRadius() * 1.2f;

    hsVector3 dir = (hsVector3)(endPos - startPos);
    float delta = dir.Magnitude();
    dir.Normalize();

    // We don't want to step up onto kickables!
    uint32_t simGroups = (1 << plSimDefs::kGroupStatic) | (1 << plSimDefs::kGroupAvatarBlocker);
    auto newGround = fController->SweepSingle(startPos, dir, delta, (plSimDefs::Group)simGroups);

    // Only step up onto the new ground if it's below our steepness limit
    if (newGround && newGround->Displacement >= 0.001f &&
        newGround->Normal.fZ >= GetFallStartThreshold() &&
        newGround->Normal.fZ > 0.2f) {
        hsVector3 displacement = dir * newGround->Displacement;
        fController->SetPositionSim(startPos + displacement);
        fController->OverrideAchievedLinearVelocity(displacement / delSecs);
    }
}

void plWalkingStrategy::Update(float delSecs)
{
    fFlags &= ~kResetMask;

    if (auto ground = IFindGroundCandidate()) {
        fFlags |= kGroundContact | kHitGroundInThisAge;
        ICheckGroundSteepness(ground.value());
        if (!IsControlledFlight())
            IStepUp(ground.value(), delSecs);
    }

    if ((fFlags & kGroundContact) && !(fFlags & kFallingNormal)) {
        fTimeInAir = 0.f;
    } else {
        fTimeInAir += delSecs;
    }

    // If we're falling but going nowhere, kill perpendicular collisions to prevent
    // getting stuck falling in place between two or more objects.
    if ((fFlags & kFallingNormal) && fTimeInAir > 0.f && fController->GetAchievedLinearVelocity().MagnitudeSquared() < 0.01f) {
        fController->DisableNearPerpendicularContacts(true);
    }

    hsVector3 zeroVelocity;
    fController->SetLinearVelocity(zeroVelocity);

    if (fFlags & kClearImpact) {
        fImpactTime = 0.0f;
        fImpactVelocity.Set(0.0f, 0.0f, 0.0f);
    }

    if (IsOnGround()) {
        fFlags |= kClearImpact;
        fController->DisableNearPerpendicularContacts(false);
    } else {
        fImpactTime = fTimeInAir;
        fImpactVelocity = fController->GetAchievedLinearVelocity();
        // convert orientation from subworld to avatar-local coordinates
        fImpactVelocity = (hsVector3)fController->GetLocalRotation().Rotate(&fImpactVelocity);
        fFlags &= ~kClearImpact;
    }
}

void plWalkingStrategy::Reset(bool newAge)
{
    plMovementStrategy::Reset(newAge);
    fImpactVelocity.Set(0.0f, 0.0f, 0.0f);
    fImpactTime = 0.0f;
    fFlags &= ~kResetMask;
    if (newAge) {
        fTimeInAir = 0.0f;
        fFlags |= kClearImpact;
        fFlags &= ~kHitGroundInThisAge;
    }
}

void plWalkingStrategy::AddContact(plPhysical* phys, const hsPoint3& pos, const hsVector3& normal)
{
    fContacts.emplace_back(phys->GetKey(), pos, normal);
}

void plWalkingStrategy::RecalcVelocity(double timeNow, float elapsed, bool useAnim)
{
    if (fControlledFlight != 0)
    {
        if (IsOnGround())
            fControlledFlightTime = fTimeInAir;

        if (fControlledFlightTime > kControlledFlightThreshold)
            EnableControlledFlight(false);
    }

    plAnimatedMovementStrategy::RecalcVelocity(timeNow, elapsed, useAnim);
}

bool plWalkingStrategy::EnableControlledFlight(bool status)
{
    if (status)
    {
        if (fControlledFlight == 0)
            fControlledFlightTime = 0.0f;

        ++fControlledFlight;
    }
    else
        fControlledFlight = std::max(fControlledFlight - 1, 0);

    return status;
}

plPhysical* plWalkingStrategy::GetPushingPhysical() const { return fController->GetPushingPhysical(); }
bool plWalkingStrategy::GetFacingPushingPhysical() const { return fController->GetFacingPushingPhysical(); }

const float plWalkingStrategy::kAirTimeThreshold = 0.1f;
const float plWalkingStrategy::kControlledFlightThreshold = 1.0f;


// Swim Strategy
plSwimStrategy::plSwimStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller)
    : plAnimatedMovementStrategy(rootApp, controller),
    fBuoyancy(),
    fSurfaceHeight(),
    fCurrentRegion(),
    fHadContacts()
{
}

void plSwimStrategy::Apply(float delSecs)
{
    hsVector3 velocity = fController->GetLinearVelocity();
    hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

    IAdjustBuoyancy();

    //trying to dampen the oscillations
    float retardent = 0.0f;
    static float finalBobSpeed = 0.5f;
    if ((achievedVelocity.fZ > finalBobSpeed) || (achievedVelocity.fZ < -finalBobSpeed))
        retardent = achievedVelocity.fZ * -0.90f;

    float zacc = (1.0f - fBuoyancy) * kGravity + retardent;
    velocity.fZ += (zacc * delSecs);

    velocity.fZ += achievedVelocity.fZ;

    // Water Current
    if (fCurrentRegion != nullptr)
    {
        float angCurrent = 0.0f;
        hsVector3 linCurrent;
        fCurrentRegion->GetCurrent(fController, linCurrent, angCurrent, delSecs);

        if (fabs(angCurrent) > 0.0001f)
            fController->IncrementAngle(angCurrent * delSecs);

        velocity += linCurrent;

        if (velocity.fZ > fCurrentRegion->fMaxUpwardVel)
            velocity.fZ = fCurrentRegion->fMaxUpwardVel;
    }

    if (velocity.fZ < kTerminalVelocity)
        velocity.fZ = kTerminalVelocity;

    // Reset vars and move controller //
    fController->SetPushingPhysical(nullptr);
    fController->SetFacingPushingPhysical(false);
    fHadContacts = false;

    fController->SetLinearVelocitySim(velocity);
}

void plSwimStrategy::SetSurface(plSwimRegionInterface *region, float surfaceHeight)
{
    fCurrentRegion = region;
    fSurfaceHeight = surfaceHeight;
}
void plSwimStrategy::IAdjustBuoyancy()
{
    // "surface depth" refers to the depth our handle object should be below
    // the surface for the avatar to be "at the surface"
    static const float surfaceDepth = 4.0f;

    if (fCurrentRegion == nullptr)
    {
        fBuoyancy = 0.f;
        return;
    }

    hsPoint3 posSim;
    fController->GetPositionSim(posSim);
    float depth = fSurfaceHeight - posSim.fZ;

    // this isn't a smooth transition but hopefully it won't be too obvious
    if (depth <= 0.0) //all the away above water
        fBuoyancy = 0.f; // Same as being above ground. Plain old gravity.
    else if (depth >= 5.0f)
        fBuoyancy = 3.0f; //completely Submereged
    else
        fBuoyancy = depth / surfaceDepth;
}


//////////////////////////////////////////////////////////////////////////

/*
Purpose:

ANGLE_RAD_2D returns the angle in radians swept out between two rays in 2D.

Discussion:

Except for the zero angle case, it should be true that

ANGLE_RAD_2D(X1,Y1,X2,Y2,X3,Y3)
+ ANGLE_RAD_2D(X3,Y3,X2,Y2,X1,Y1) = 2 * PI

Modified:

19 April 1999

Author:

John Burkardt

Parameters:

Input, float X1, Y1, X2, Y2, X3, Y3, define the rays
( X1-X2, Y1-Y2 ) and ( X3-X2, Y3-Y2 ) which in turn define the
angle, counterclockwise from ( X1-X2, Y1-Y2 ).

Output, float ANGLE_RAD_2D, the angle swept out by the rays, measured
in radians.  0 <= ANGLE_DEG_2D < 2 PI.  If either ray has zero length,
then ANGLE_RAD_2D is set to 0.
*/

static float AngleRad2d( float x1, float y1, float x3, float y3 )
{
    float value;
    float x;
    float y;

    x = ( x1 ) * ( x3 ) + ( y1 ) * ( y3 );
    y = ( x1 ) * ( y3 ) - ( y1 ) * ( x3 );

    if (x == 0.f && y == 0.f) {
        value = 0.f;
    }
    else
    {
        value = atan2 ( y, x );

        if (value < 0.f)
            value += hsConstants::two_pi<float>;
    }
    return value;
}
