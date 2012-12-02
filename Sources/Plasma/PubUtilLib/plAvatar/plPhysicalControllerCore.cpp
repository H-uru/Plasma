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

#include "plArmatureMod.h"
#include "plSwimRegion.h"
#include "plMatrixChannel.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plPhysical.h"
#include "pnMessage/plCorrectionMsg.h"

// Gravity constants
#define kGravity -32.174f
#define kTerminalVelocity kGravity

static inline hsVector3 GetYAxis(hsMatrix44 &mat) { return hsVector3(mat.fMap[1][0], mat.fMap[1][1], mat.fMap[1][2]); }
static float AngleRad2d(float x1, float y1, float x3, float y3);

bool CompareMatrices(const hsMatrix44 &matA, const hsMatrix44 &matB, float tolerance);


// plPhysicalControllerCore
plPhysicalControllerCore::plPhysicalControllerCore(plKey OwnerSceneObject, float height, float radius)
    : fOwner(OwnerSceneObject),
    fWorldKey(nil),
    fHeight(height),
    fRadius(radius),
    fLOSDB(plSimDefs::kLOSDBNone),
    fMovementStrategy(nil),
    fSimLength(0.0f),
    fLocalRotation(0.0f, 0.0f, 0.0f, 1.0f),
    fLocalPosition(0.0f, 0.0f, 0.0f),
    fLastLocalPosition(0.0f, 0.0f, 0.0f),
    fLinearVelocity(0.0f, 0.0f, 0.0f),
    fAchievedLinearVelocity(0.0f, 0.0f, 0.0f),
    fPushingPhysical(nil),
    fFacingPushingPhysical(false),
    fSeeking(false),
    fEnabled(false),
    fEnableChanged(false)
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

    return nil;
}

void plPhysicalControllerCore::IncrementAngle(float deltaAngle)
{
    hsVector3 axis;
    float angle;

    fLocalRotation.NormalizeIfNeeded();
    fLocalRotation.GetAngleAxis(&angle, &axis);
    if (axis.fZ < 0)
        angle = (2.0f * float(M_PI)) - angle; // axis is backwards, so reverse the angle too

    angle += deltaAngle;

    // make sure we wrap around
    if (angle < 0.0f)
        angle = (2.0f * float(M_PI)) + angle; // angle is -, so this works like a subtract
    if (angle >= (2.0f * float(M_PI)))
        angle = angle - (2.0f * float(M_PI));

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
    if (!CompareMatrices(fLastGlobalLoc, l2w, 0.0001f))
        SetGlobalLoc(l2w);

    if (fEnabled)
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
    if (fEnabled)
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

    if (fEnableChanged)
        IHandleEnableChanged();
}
void plPhysicalControllerCore::IUpdateNonPhysical(float alpha)
{
    // Update global location if owner transform hasn't changed.
    plSceneObject* so = plSceneObject::ConvertNoRef(fOwner->ObjectIsLoaded());
    const hsMatrix44& l2w = so->GetCoordinateInterface()->GetLocalToWorld();
    if (CompareMatrices(fLastGlobalLoc, l2w, 0.0001f))
    {
        if (fEnabled)
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
    if (hsABS(zRot) > 0.0001f)
        fController->IncrementAngle(zRot * elapsed);

    // Update controller velocity
    fController->SetLinearVelocity(fAnimLinearVel);
}

void plAnimatedMovementStrategy::IRecalcLinearVelocity(float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat)
{
    hsPoint3 startPos(0.0f, 0.0f, 0.0f);                    // default position (at start of anim)
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

        float prev2NowMagSqr = prev2Now.MagnitudeSquared();
        float start2NowMagSqr = start2Now.MagnitudeSquared();

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
    if (angleSincePrev > float(M_PI))
        angleSincePrev = angleSincePrev - TWO_PI;

    const hsVector3 startForward = hsVector3(0.0f, -1.0f, 0.0f);    // the Y orientation of a "resting" armature....
    float angleSinceStart = AngleRad2d(curForward.fX, curForward.fY, startForward.fX, startForward.fY);
    bool sinceStartSign = angleSinceStart > 0.0f;
    if (angleSinceStart > float(M_PI))
        angleSinceStart = angleSinceStart - TWO_PI;

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
    fSlidingNormals(),
    fImpactVelocity(0.0f, 0.0f, 0.0f),
    fImpactTime(0.0f),
    fTimeInAir(0.0f),
    fControlledFlightTime(0.0f),
    fControlledFlight(0),
    fGroundHit(false),
    fFalseGround(false),
    fHeadHit(false),
    fClearImpact(false),
    fHitGroundInThisAge(false)
{
}

void plWalkingStrategy::Apply(float delSecs)
{
    hsVector3 velocity = fController->GetLinearVelocity();
    hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

    // Add in gravity if the avatar's z velocity isn't being set explicitly
    if (hsABS(velocity.fZ) < 0.001f)
    {
        // Get our previous z velocity.  If we're on the ground, clamp it to zero at
        // the largest, so we won't launch into the air if we're running uphill.
        float prevZVel = achievedVelocity.fZ;
        if (IsOnGround())
            prevZVel = hsMinimum(prevZVel, 0.0f);

        velocity.fZ = prevZVel + (kGravity * delSecs);
    }

    // If we're airborne and the velocity isn't set, use the velocity from
    // the last frame so we maintain momentum.
    if (!IsOnGround() && velocity.fX == 0.0f && velocity.fY == 0.0f)
    {
        velocity.fX = achievedVelocity.fX;
        velocity.fY = achievedVelocity.fY;
    }

    if (!fGroundHit && fSlidingNormals.Count())
    {
        // We're not on solid ground, so we should be sliding against whatever
        // we're hitting (like a rock cliff). Each vector in fSlidingNormals is
        // the surface normal of a collision that's too steep to be ground, so
        // we project our current velocity onto that plane and slide along the
        // wall.
        //
        // Also, sometimes PhysX reports a bunch of collisions from the wall,
        // but nothing from underneath (when there should be). So if we're not
        // touching ground, we offset the avatar in the direction of the
        // surface normal(s). This doesn't fix the issue 100%, but it's a hell
        // of a lot better than nothing, and suitable duct tape until a future
        // PhysX revision fixes the issue.
        //
        // Yes, there's room for optimization here if we care.
        hsVector3 offset(0.0f, 0.0f, 0.0f);
        for (int i = 0; i < fSlidingNormals.GetCount(); i++)
        {
            offset += fSlidingNormals[i];
            hsVector3 velNorm = velocity;

            if (velNorm.MagnitudeSquared() > 0.0f)
                velNorm.Normalize();

            if (velNorm * fSlidingNormals[i] < 0.0f)
            {
                hsVector3 proj = (velNorm % fSlidingNormals[i]) % fSlidingNormals[i];
                if (velNorm * proj < 0.0f)
                    proj *= -1.0f;

                velocity = velocity.Magnitude() * proj;
            }
        }
        if (offset.MagnitudeSquared() > 0.0f)
        {
            // 5 ft/sec is roughly the speed we walk backwards.
            // The higher the value, the less likely you'll trip
            // the bug, and this seems reasonable.
            offset.Normalize();
            velocity += offset * 5.0f;
        }
    }

    if (velocity.fZ < kTerminalVelocity)
        velocity.fZ = kTerminalVelocity;

    // Convert to a displacement vector
    hsVector3 displacement = velocity * delSecs;

    // Reset vars and move the controller
    fController->SetPushingPhysical(nil);
    fController->SetFacingPushingPhysical(false);
    fGroundHit = fFalseGround = fHeadHit = false;
    fSlidingNormals.SetCount(0);

    unsigned int collideResults = 0;
    unsigned int collideFlags = 1<<plSimDefs::kGroupStatic | 1<<plSimDefs::kGroupAvatarBlocker | 1<<plSimDefs::kGroupDynamic;
    if (!fController->IsSeeking())
        collideFlags |= (1<<plSimDefs::kGroupExcludeRegion);

    fController->Move(displacement, collideFlags, collideResults);

    if ((!fGroundHit) && (collideResults & kBottom))
        fFalseGround = true;

    if (collideResults & kTop)
        fHeadHit = true;
}
void plWalkingStrategy::Update(float delSecs)
{
    if (fGroundHit || fFalseGround)
        fTimeInAir = 0.0f;
    else
    {
        fTimeInAir += delSecs;
        if (fHeadHit)
        {
            // If we're airborne and hit our head, override achieved velocity to avoid being shoved sideways
            hsVector3 velocity = fController->GetLinearVelocity();
            hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

            achievedVelocity.fX = velocity.fX;
            achievedVelocity.fY = velocity.fY;
            if (achievedVelocity.fZ > 0.0f)
                achievedVelocity.fZ = 0.0f;

            fController->OverrideAchievedLinearVelocity(achievedVelocity);
        }
    }

    hsVector3 zeroVelocity(0.f, 0.f, 0.f);
    fController->SetLinearVelocity(zeroVelocity);

    if (!fHitGroundInThisAge && IsOnGround())
        fHitGroundInThisAge = true;

    if (fClearImpact)
    {
        fImpactTime = 0.0f;
        fImpactVelocity.Set(0.0f, 0.0f, 0.0f);
    }

    if (IsOnGround())
        fClearImpact = true;
    else
    {
        fImpactTime = fTimeInAir;
        fImpactVelocity = fController->GetAchievedLinearVelocity();
        // convert orientation from subworld to avatar-local coordinates
        fImpactVelocity = (hsVector3)fController->GetLocalRotation().Rotate(&fImpactVelocity);
        fClearImpact = false;
    }
}

void plWalkingStrategy::AddContactNormals(hsVector3& vec)
{
    float dot = vec * kAvatarUp;
    if (dot >= 0.5f)
        fGroundHit = true;
    else
        fSlidingNormals.Append(vec);
}

void plWalkingStrategy::Reset(bool newAge)
{
    plMovementStrategy::Reset(newAge);
    fImpactVelocity.Set(0.0f, 0.0f, 0.0f);
    fImpactTime = 0.0f;
    if (newAge)
    {
        fTimeInAir = 0.0f;
        fClearImpact = true;
        fHitGroundInThisAge = false;
        fSlidingNormals.SetCount(0);
    }
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
        fControlledFlight = max(--fControlledFlight, 0);

    return status;
}

plPhysical* plWalkingStrategy::GetPushingPhysical() const { return fController->GetPushingPhysical(); }
bool plWalkingStrategy::GetFacingPushingPhysical() const { return fController->GetFacingPushingPhysical(); }

const float plWalkingStrategy::kAirTimeThreshold = 0.1f;
const float plWalkingStrategy::kControlledFlightThreshold = 1.0f;


// Swim Strategy
plSwimStrategy::plSwimStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller)
    : plAnimatedMovementStrategy(rootApp, controller),
    fBuoyancy(0.0f),
    fSurfaceHeight(0.0f),
    fCurrentRegion(nil),
    fOnGround(false),
    fHadContacts(false)
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
    if (fCurrentRegion != nil)
    {
        float angCurrent = 0.0f;
        hsVector3 linCurrent(0.0f, 0.0f, 0.0f);
        fCurrentRegion->GetCurrent(fController, linCurrent, angCurrent, delSecs);

        if (hsABS(angCurrent) > 0.0001f)
            fController->IncrementAngle(angCurrent * delSecs);

        velocity += linCurrent;

        if (velocity.fZ > fCurrentRegion->fMaxUpwardVel)
            velocity.fZ = fCurrentRegion->fMaxUpwardVel;
    }

    if (velocity.fZ < kTerminalVelocity)
        velocity.fZ = kTerminalVelocity;

    // Convert to displacement vector
    hsVector3 displacement = velocity * delSecs;

    // Reset vars and move controller //
    fController->SetPushingPhysical(nil);
    fController->SetFacingPushingPhysical(false);
    fHadContacts = fOnGround = false;

    unsigned int collideResults = 0;
    unsigned int collideFlags = 1<<plSimDefs::kGroupStatic | 1<<plSimDefs::kGroupAvatarBlocker | 1<<plSimDefs::kGroupDynamic;
    if (!fController->IsSeeking())
        collideFlags |= (1<<plSimDefs::kGroupExcludeRegion);

    fController->Move(displacement, collideFlags, collideResults);

    if ((collideResults & kBottom) || (collideResults & kSides))
        fHadContacts = true;
}

void plSwimStrategy::AddContactNormals(hsVector3& vec)
{
    float dot = vec * kAvatarUp;
    if (dot >= kSlopeLimit)
        fOnGround = true;
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
    // 1.0 = neutral buoyancy
    // 0 = no buoyancy (normal gravity)
    // 2.0 = opposite of gravity, floating upwards
    static const float buoyancyAtSurface = 1.0f;

    if (fCurrentRegion == nil)
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


// Dynamic Walking Strategy
plDynamicWalkingStrategy::plDynamicWalkingStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller)
    : plWalkingStrategy(rootApp, controller)
{
}

void plDynamicWalkingStrategy::Apply(float delSecs)
{
    hsVector3 velocity = fController->GetLinearVelocity();
    hsVector3 achievedVelocity = fController->GetAchievedLinearVelocity();

    // Add in gravity if the avatar's z velocity isn't being set explicitly
    if (hsABS(velocity.fZ) < 0.001f)
    {
        // Get our previous z velocity.  If we're on the ground, clamp it to zero at
        // the largest, so we won't launch into the air if we're running uphill.
        float prevZVel = achievedVelocity.fZ;
        if (IsOnGround())
            prevZVel = hsMinimum(prevZVel, 0.f);

        velocity.fZ = prevZVel + (kGravity * delSecs);
    }

    if (velocity.fZ < kTerminalVelocity)
        velocity.fZ = kTerminalVelocity;

    fController->SetPushingPhysical(nil);
    fController->SetFacingPushingPhysical(false);
    fGroundHit = fFalseGround = false;

    float groundZVelocity;
    if (ICheckForGround(groundZVelocity))
        velocity.fZ += groundZVelocity;

    fController->SetLinearVelocitySim(velocity);
}

bool plDynamicWalkingStrategy::ICheckForGround(float& zVelocity)
{
    std::vector<plControllerSweepRecord> groundHits;
    uint32_t collideFlags = 1<<plSimDefs::kGroupStatic | 1<<plSimDefs::kGroupAvatarBlocker | 1<<plSimDefs::kGroupDynamic;

    hsPoint3 startPos;
    fController->GetPositionSim(startPos);
    hsPoint3 endPos = startPos;

    // Set sweep length
    startPos.fZ += 0.05f;
    endPos.fZ -= 0.05f;

    int possiblePlatformCount = fController->SweepControllerPath(startPos, endPos, true, true, collideFlags, groundHits);
    if (possiblePlatformCount)
    {
        zVelocity = -FLT_MAX;

        std::vector<plControllerSweepRecord>::iterator curRecord;
        for (curRecord = groundHits.begin(); curRecord != groundHits.end(); ++curRecord)
        {
            if (curRecord->ObjHit != nil)
            {
                hsVector3 objVelocity;
                curRecord->ObjHit->GetLinearVelocitySim(objVelocity);
                if (objVelocity.fZ > zVelocity)
                    zVelocity = objVelocity.fZ;

                fGroundHit = true;
            }
        }
    }

    return fGroundHit;
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

    if ( x == 0.0 && y == 0.0 ) {
        value = 0.0;
    }
    else
    {
        value = atan2 ( y, x );

        if ( value < 0.0 )
        {
            value = (float)(value + TWO_PI);
        }
    }
    return value;
}

