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

void plWalkingStrategy::Apply(float delSecs)
{
    hsVector3 velocity = fController->GetLinearVelocity();
    hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

    // Add in gravity if the avatar's z velocity isn't being set explicitly
    if (std::fabs(velocity.fZ) < 0.001f) {
        // Get our previous z velocity.  If we're on the ground, clamp it to zero at
        // the largest, so we won't launch into the air if we're running uphill.
        float prevZVel = achievedVelocity.fZ;
        if (IsOnGround())
            prevZVel = std::min(prevZVel, 0.0f);

        velocity.fZ = prevZVel + (kGravity * delSecs);
    }
    velocity.fZ = std::max(velocity.fZ, kGravity);

    // Some extra oomph to keep us on the ground.
    if (IsOnGround())
        velocity.fZ *= 1.1f;

    if (!IsOnGround() && std::fabs(velocity.fX) < 0.001f && std::fabs(velocity.fY) < 0.001f) {
        // If we're airborne and the velocity isn't set, use the velocity from
        // the last frame so we maintain momentum.
        velocity.fX = achievedVelocity.fX;
        velocity.fY = achievedVelocity.fY;
    } else if (IsOnGround()) {
        // Project velocity by the ground normal to prevent speeding up
        // when running uphill (grrr...) Note that if the ground is a proxy
        // collider, you may have collisions from both the wall (z=0.0) and
        // the floor (z=-1.0)... be sure to use the floor.
        plKey groundPhys = fController->GetGround();
        const hsVector3* groundNormal = nullptr;
        for (const auto& contact : fContacts) {
            if (contact.PhysHit == groundPhys) {
                if (!groundNormal || std::abs(contact.Normal.fZ) > std::abs(groundNormal->fZ))
                    groundNormal = &contact.Normal;
            }
        }

        if (groundNormal) {
            // The Y component is forward, so use it always. Further, we can go up some pretty
            // doggone steep slopes, resulting in the theoretical variantion of half of our velocity.
            // Debounce that by capping the increase or decrease.
            const float yFactor = (achievedVelocity.fZ < 0.f) ?
                // Going downhill leg - note that even a small increase in velocity is VERY obvious here.
                // Therefore, we make no adjustment.
                1.f :
                // Going uphill leg. Ideally this would be some kind of non-linear calculation.
                // That way, going up stairs is much more noticable than running uphill in Relto.
                // But MEH, you do it.
                std::max(1.f - std::abs(groundNormal->fY), .9f);
            velocity.fX *= yFactor;
            velocity.fY *= yFactor;
        }
    }

    // Now Cyan's good old "slide along all the contacts" code.
    hsVector3 offset(0.f, 0.f, 0.f);
    for (const auto contact : fContacts) {
        if (!contact.GetPhysical() || contact.GetPhysical()->GetGroup() == plSimDefs::kGroupDynamic)
            continue;
        if (contact.Normal.fZ >= .5f)
            continue;
        offset += contact.Normal;
        hsVector3 velNorm = velocity;

        if (velNorm.MagnitudeSquared() > 0.0f)
            velNorm.Normalize();

        if (velNorm * contact.Normal < 0.0f) {
            hsVector3 proj = (velNorm % contact.Normal) % contact.Normal;
            if (velNorm * proj < 0.0f)
                proj *= -1.0f;

            velocity = velocity.Magnitude() * proj;
        }
    }
    if (offset.MagnitudeSquared() > 0.0f) {
        // 5 ft/sec is roughly the speed we walk backwards.
        offset.Normalize();
        velocity += offset * 5.f;
    }

    fController->SetPushingPhysical(nullptr);
    fController->SetFacingPushingPhysical(false);
    fContacts.clear();
    fFlags &= ~kResetMask;

    uint32_t simGroups = (1 << plSimDefs::kGroupStatic) |
                         (1 << plSimDefs::kGroupDynamic) |
                         (1 << plSimDefs::kGroupAvatarBlocker);
    if (!fController->IsSeeking())
        simGroups |= (1 << plSimDefs::kGroupExcludeRegion);
    uint32_t result = fController->Move(velocity, delSecs, (plSimDefs::Group)simGroups);
    if (result & plPhysicalControllerCore::kBottom)
        fFlags |= kGroundContact;
}

void plWalkingStrategy::Update(float delSecs)
{
    if (fFlags & kGroundContact) {
        fTimeInAir = 0.f;
        fFlags |= kHitGroundInThisAge;
    } else {
        fTimeInAir += delSecs;
    }

    hsVector3 zeroVelocity;
    fController->SetLinearVelocity(zeroVelocity);

    if (fFlags & kClearImpact) {
        fImpactTime = 0.0f;
        fImpactVelocity.Set(0.0f, 0.0f, 0.0f);
    }

    if (IsOnGround()) {
        fFlags |= kClearImpact;
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

    uint32_t simGroups = (1 << plSimDefs::kGroupStatic) |
                         (1 << plSimDefs::kGroupDynamic) |
                         (1 << plSimDefs::kGroupAvatarBlocker);
    fController->Move(velocity, delSecs, (plSimDefs::Group)simGroups);
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
