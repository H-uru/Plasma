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
#ifndef PLPHYSICALCONTROLLERCORE_H
#define PLPHYSICALCONTROLLERCORE_H

#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "plPhysical.h"
#include "hsQuat.h"

#include "pnKeyedObject/plKey.h"

#include "plPhysical/plSimDefs.h"

#include <optional>
#include <vector>

class plCoordinateInterface;
class plPhysical;
class plMovementStrategy;
class plAGApplicator;
class plSwimRegionInterface;
class plSceneObject;

struct plControllerHitRecord
{
    plKey PhysHit;
    hsPoint3 Point;
    hsVector3 Normal;
    hsVector3 Direction;
    float Displacement;

    plControllerHitRecord()
        : Point(), Normal(), Direction(), Displacement()
    { }
    plControllerHitRecord(plKey physHit, const hsPoint3& p, const hsVector3& n, const hsVector3& d={}, float l = 0.f)
        : PhysHit(std::move(physHit)), Point(p), Normal(n), Direction(d), Displacement(l)
    {
    }

    plPhysical* GetPhysical() const
    {
        if (PhysHit)
            return plPhysical::ConvertNoRef(PhysHit->ObjectIsLoaded());
        return nullptr;
    }
};

class plPhysicalControllerCore
{
public:
    enum
    {
        /**
         * Disable rigid body collision.
         * This disables collision with simulation shapes such as static colliders and kickables.
         * Trigger shapes (detectors) are unaffected by this flag.
         */
        kDisableCollision = (1<<0),

        /**
         * The character is seeking.
         * The character's brain is seeking or otherwise moving on autopilot to a specific point.
         */
        kSeeking = (1<<1),
    };

    enum Collisions
    {
        kBottom = (1<<0),
        kTop = (1<<1),
        kSides = (1<<2),
    };

public:
    plPhysicalControllerCore(plKey ownerSceneObject, float height, float radius);
    virtual ~plPhysicalControllerCore() { }

    // An ArmatureMod has its own idea about when physics should be enabled/disabled.
    // Use plArmatureModBase::EnablePhysics() instead.
    virtual void Enable(bool enable) = 0;

    [[nodiscard]]
    bool IsEnabled() const { return !(fFlags & kDisableCollision); }

    // Subworld
    virtual plKey GetSubworld() { return fWorldKey; }
    virtual void SetSubworld(const plKey& world) = 0;
    virtual const plCoordinateInterface* GetSubworldCI();

    // For the avatar SDL only
    virtual void GetState(hsPoint3& pos, float& zRot) = 0;
    virtual void SetState(const hsPoint3& pos, float zRot) = 0;

    // The LOS DB this avatar is in (only one)
    virtual plSimDefs::plLOSDB GetLOSDB() { return fLOSDB; }
    virtual void SetLOSDB(plSimDefs::plLOSDB losDB) { fLOSDB = losDB; }

    // Movement strategy
    virtual void SetMovementStrategy(plMovementStrategy* strategy) = 0;

    // Global location
    virtual const hsMatrix44& GetLastGlobalLoc() { return fLastGlobalLoc; }
    virtual void SetGlobalLoc(const hsMatrix44& l2w) = 0;

    // Local sim position
    virtual void GetPositionSim(hsPoint3& pos) = 0;
    virtual void SetPositionSim(const hsPoint3& pos) = 0;

    /** Moves the controller using a collide and slide algorithm. */
    virtual Collisions Move(const hsVector3& velocity, float delSecs, plSimDefs::Group colGroups) = 0;

    /**
     * Sweeps the character's capsule from startPos through endPos and reports all hits
     * along the path.
     * \param[in] startPos: Position from which the capsule sweep should begin.
     * \param[in] endPos: Position from which the capsule sweep should end.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns All hits along the path of the sweep.
     */
    [[nodiscard]]
    virtual std::vector<plControllerHitRecord> SweepMulti(const hsPoint3& startPos,
                                                          const hsPoint3& endPos,
                                                          plSimDefs::Group simGroups) const = 0;

    /**
     * Sweeps the character's capsule from startPos through endPos and reports all hits
     * along the path.
     * \param[in] origin Position from which the capsule sweep should begin.
     * \param[in] dir Unit vector defining the direction of the capsule sweep.
     * \param[in] distance Distance over which the capsule should be swept.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns All hits along the path of the sweep.
     */
    [[nodiscard]]
    virtual std::vector<plControllerHitRecord> SweepMulti(const hsPoint3& origin,
                                                          const hsVector3& dir,
                                                          float distance,
                                                          plSimDefs::Group simGroups) const = 0;

    /**
     * Sweeps the character's capsule from startPos through endPos and reports the first blocking
     * hit along the path.
     * \param[in] startPos: Position from which the capsule sweep should begin.
     * \param[in] endPos: Position from which the capsule sweep should end.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns The first hit along the path of the sweep.
     */
    [[nodiscard]]
    virtual std::optional<plControllerHitRecord> SweepSingle(const hsPoint3& startPos,
                                                             const hsPoint3& endPos,
                                                             plSimDefs::Group simGroups) const = 0;

    /**
     * Sweeps the character's capsule from startPos through endPos and reports the first blocking
     * hit along the path.
     * \param[in] origin Position from which the capsule sweep should begin.
     * \param[in] dir Unit vector defining the direction of the capsule sweep.
     * \param[in] distance Distance over which the capsule should be swept.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns The first hit along the path of the sweep.
     */
    [[nodiscard]]
    virtual std::optional<plControllerHitRecord> SweepSingle(const hsPoint3& origin,
                                                             const hsVector3& dir,
                                                             float distance,
                                                             plSimDefs::Group simGroups) const = 0;

    // any clean up for the controller should go here
    virtual void LeaveAge() = 0;

    // Local rotation
    const hsQuat& GetLocalRotation() const { return fLocalRotation; }
    void IncrementAngle(float deltaAngle);

    /** Sets the controller's desired linear velocity in character space. */
    void SetLinearVelocity(const hsVector3& linearVel) { fLinearVelocity = linearVel; }

    /** Gets the controller's desired linear velocity in character space. */
    const hsVector3& GetLinearVelocity() const { return fLinearVelocity; }

    /** Gets the controller's simulated linear velocity in character space. */
    const hsVector3& GetAchievedLinearVelocity() const { return fAchievedLinearVelocity; }

    /** Overrides the controller's simulated linear velocity in character space. */
    void OverrideAchievedLinearVelocity(const hsVector3& linearVel) { fAchievedLinearVelocity = linearVel; }

    /** Resets the controller's simulated linear velocity. */
    void ResetAchievedLinearVelocity() { fAchievedLinearVelocity.Set(0.f, 0.f, 0.f); }

    // SceneObject
    plKey GetOwner() { return fOwner; }

    // When seeking no longer want to interact with exclude regions
    void SetSeek(bool seek) { hsChangeBits(fFlags, kSeeking, seek); }
    bool IsSeeking() const { return fFlags & kSeeking; }

    // Pushing physical
    plPhysical* GetPushingPhysical() const { return fPushingPhysical; }
    void SetPushingPhysical(plPhysical* phys) { fPushingPhysical = phys; }
    bool GetFacingPushingPhysical() const { return fFacingPushingPhysical; }
    void SetFacingPushingPhysical(bool facing) { fFacingPushingPhysical = facing; }

    /**
     * Gets the key of the physical representing the ground collision.
     */
    virtual plKey GetGround() const = 0;

    // Controller dimensions
    float GetRadius() const { return fRadius; }
    float GetHeight() const { return fHeight; }

    float GetMass() const { return 100.f; }

    // Create a new controller instance - Implemented in the physics system
    static plPhysicalControllerCore* Create(plKey ownerSO, float height, float radius);

protected:
    void IApply(float delSecs);
    void IUpdate(int numSubSteps, float alpha);
    void IUpdateNonPhysical(float alpha);

    void ISendCorrectionMessages(bool dirtySynch = false);

    plKey fOwner;
    plKey fWorldKey;

    float fHeight;
    float fRadius;

    plSimDefs::plLOSDB fLOSDB;

    plMovementStrategy* fMovementStrategy;

    float fSimLength;
    uint32_t fFlags;

    hsQuat fLocalRotation;
    hsPoint3 fLocalPosition;
    hsPoint3 fLastLocalPosition;

    hsMatrix44 fLastGlobalLoc;
    hsMatrix44 fPrevSubworldW2L;

    hsVector3 fLinearVelocity;
    hsVector3 fAchievedLinearVelocity;

    plPhysical* fPushingPhysical;
    bool fFacingPushingPhysical;
};

class plMovementStrategy
{
public:
    plMovementStrategy(plPhysicalControllerCore* controller);
    virtual ~plMovementStrategy() = default;

    virtual void Apply(float delSecs) = 0;
    virtual void Update(float delSecs) { }

    virtual void AddContact(plPhysical* phys, const hsPoint3& pos, const hsVector3& normal) { }
    virtual void Reset(bool newAge);

    virtual bool IsRiding() const { return false; }
    virtual bool AllowSliding() const { return true; }

protected:
    plPhysicalControllerCore* fController;
};

class plAnimatedMovementStrategy : public plMovementStrategy
{
public:
    plAnimatedMovementStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller);

    virtual void RecalcVelocity(double timeNow, float elapsed, bool useAnim = true);
    void SetTurnStrength(float val) { fTurnStr = val; }
    float GetTurnStrength() const { return fTurnStr; }

private:
    void IRecalcLinearVelocity(float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat);
    void IRecalcAngularVelocity(float elapsed, hsMatrix44 &prevMat, hsMatrix44 &curMat);

    plAGApplicator* fRootApp;
    hsVector3	fAnimLinearVel;
    float	fAnimAngularVel;
    float	fTurnStr;
};

class plWalkingStrategy : public plAnimatedMovementStrategy
{
public:
    plWalkingStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller);

    void Apply(float delSecs) override;
    void Update(float delSecs) override;

    void Reset(bool newAge) override;

    void AddContact(plPhysical* phys, const hsPoint3& pos, const hsVector3& normal) override;

    void RecalcVelocity(double timeNow, float elapsed, bool useAnim = true) override;

    bool HitGroundInThisAge() const { return fFlags & kHitGroundInThisAge; }
    bool IsOnGround() const
    {
        if (fFlags & kGroundContact)
            return true;
        return fTimeInAir < kAirTimeThreshold;
    }

    /** Toggles the character's ability to ride on moving platforms. */
    void ToggleRiding(bool status) { hsChangeBits(fFlags, kRidePlatform, status); }

    float GetAirTime() const { return fTimeInAir; }
    void ResetAirTime() { fTimeInAir = 0.0f; }

    float GetImpactTime() const { return fImpactTime; }
    const hsVector3& GetImpactVelocity() const { return fImpactVelocity; }

    bool EnableControlledFlight(bool status);
    bool IsControlledFlight() const { return fControlledFlight != 0; }

    plPhysical* GetPushingPhysical() const;
    bool GetFacingPushingPhysical() const;

    bool IsRiding() const override { return fFlags & kRidePlatform; }

    /**
     * Disallow PhysX's built-in sliding algorithm.
     * Empirical results show that PhysX's built-in sliding along
     * non-walkable surfaces will trigger getting stuck on some
     * slightly smaller than 90 degree colliders.
     */
    bool AllowSliding() const override { return false; }

protected:
    enum
    {
        /** Indicates that the character is standing on a valid platform. */
        kGroundContact = (1<<0),

        kHitGroundInThisAge = (1<<1),
        kClearImpact = (1<<2),

        /**
         * Indicates the character should inherit the X/Y velocity of any platform it stands on.
         */
        kRidePlatform = (1<<3),

        kResetMask = kGroundContact,
    };

    static const float kAirTimeThreshold;
    static const float kControlledFlightThreshold;

    std::vector<plControllerHitRecord> fContacts;
    hsVector3 fImpactVelocity;
    float fImpactTime;

    float fTimeInAir;

    float fControlledFlightTime;
    int fControlledFlight;

    uint32_t fFlags;
};

class plSwimStrategy : public plAnimatedMovementStrategy
{
public:
    plSwimStrategy(plAGApplicator* rootApp, plPhysicalControllerCore* controller);

    void Apply(float delSecs) override;

    void AddContact(plPhysical* phys, const hsPoint3& pos, const hsVector3& normal) override
    {
        fHadContacts = true;
    }

    void SetSurface(plSwimRegionInterface* region, float surfaceHeight);

    float GetBuoyancy() const { return fBuoyancy; }
    bool HadContacts() const { return fHadContacts; }

protected:
    void IAdjustBuoyancy();

    float fBuoyancy;
    float fSurfaceHeight;

    plSwimRegionInterface *fCurrentRegion;

    bool fHadContacts;
};

#endif// PLPHYSICALCONTROLLERCORE_H

