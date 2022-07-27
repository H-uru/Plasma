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
#ifndef plPXPhysicalControllerCore_H
#define plPXPhysicalControllerCore_H

#include <memory>
#include <set>
#include <tuple>
#include <vector>

#include "plAvatar/plPhysicalControllerCore.h"

namespace physx
{
    class PxActor;
    class PxCapsuleGeometry;
    class PxController;
    class PxRigidDynamic;
    class PxTransform;
    class PxVec3;
};

class plPhysicalProxy;
class plDrawableSpans;
class hsGMaterial;
class plSceneObject;
class plPXPhysical;
class plPXControllerBehaviorCallback;

class plPXPhysicalControllerCore: public plPhysicalControllerCore
{
public:
    plPXPhysicalControllerCore(plKey ownerSO, float height, float radius);
    ~plPXPhysicalControllerCore();

    // An ArmatureMod has its own idea about when physics should be enabled/disabled.
    // Use plArmatureModBase::EnablePhysics() instead.
    void Enable(bool enable) override;

    // Subworld
    void SetSubworld(const plKey& world) override;

    // For the avatar SDL only
    void GetState(hsPoint3& pos, float& zRot) override;
    void SetState(const hsPoint3& pos, float zRot) override;

    // The LOS DB this avatar is in (only one)
    void SetLOSDB(plSimDefs::plLOSDB losDB) override;

    // Movement strategy
    void SetMovementStrategy(plMovementStrategy* strategy) override;

    // Global location
    void SetGlobalLoc(const hsMatrix44& l2w) override;

    // Local Sim Position
    void GetPositionSim(hsPoint3& pos) override;
    void SetPositionSim(const hsPoint3& pos) override;

    /** Moves the controller using a collide and slide algorithm. */
    Collisions Move(const hsVector3& displacement, float delSecs, plSimDefs::Group colGroups) override;

protected:
    [[nodiscard]]
    std::vector<plControllerHitRecord> ISweepMulti(const hsPoint3& origin,
                                                   const hsVector3& dir,
                                                   float distance,
                                                   plSimDefs::Group simGroups) const;

    [[nodiscard]]
    std::optional<plControllerHitRecord> ISweepSingle(const hsPoint3& origin,
                                                      const hsVector3& dir,
                                                      float distance,
                                                      plSimDefs::Group simGroups) const;

public:
    /**
     * Sweeps the character's capsule from startPos through endPos and reports all hits
     * along the path.
     * \param[in] startPos: Position from which the capsule sweep should begin.
     * \param[in] endPos: Position from which the capsule sweep should end.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns All hits along the path of the sweep.
     */
    [[nodiscard]]
    std::vector<plControllerHitRecord> SweepMulti(const hsPoint3& startPos,
                                                  const hsPoint3& endPos,
                                                  plSimDefs::Group simGroups) const override;

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
    std::vector<plControllerHitRecord> SweepMulti(const hsPoint3& origin,
                                                  const hsVector3& dir,
                                                  float distance,
                                                  plSimDefs::Group simGroups) const override
    {
        return ISweepMulti(origin, dir, distance, simGroups);
    }

    /**
     * Sweeps the character's capsule from startPos through endPos and reports the first blocking
     * hit along the path.
     * \param[in] startPos: Position from which the capsule sweep should begin.
     * \param[in] endPos: Position from which the capsule sweep should end.
     * \param[in] simGroups A bit mask of groups the swept shape should hit.
     * \returns The first hit along the path of the sweep.
     */
    [[nodiscard]]
    std::optional<plControllerHitRecord> SweepSingle(const hsPoint3& startPos,
                                                     const hsPoint3& endPos,
                                                     plSimDefs::Group simGroups) const override;

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
    std::optional<plControllerHitRecord> SweepSingle(const hsPoint3& origin,
                                                     const hsVector3& dir,
                                                     float distance,
                                                     plSimDefs::Group simGroups) const override
    {
        return ISweepSingle(origin, dir, distance, simGroups);
    }

    // any clean up for the controller should go here
    void LeaveAge() override;

    // Create Proxy for debug rendering
    plDrawableSpans* CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo);

    /**
     * Gets the key of the physical representing the ground collision.
     */
    plKey GetGround() const override;

    /** Handles contacts generated by the simulation. */
    void AddContact(plPXPhysical* phys, const hsPoint3& pos, const hsVector3& normal, const hsVector3& dir, float length);

//////////////////////////////////////////
//Static Helper Functions
////////////////////////////////////////

    // Call pre-sim to apply movement to controllers
    static void Apply(float delSecs);

    // Call post-sim to update controllers
    static void Update(int numSubSteps, float alpha);

    // Update controllers when not performing a physics step
    static void UpdateNonPhysical(float alpha);

#ifndef PLASMA_EXTERNAL_RELEASE
    static bool fDebugDisplay;
#endif

protected:
    /**
     * Gets the physics position of the character's capsule.
     * \remarks Plasma stores the position of avatars from their "foot" position while PhysX
     *          uses the center of the capsule shape as its center. This returns the current position
     *          to be used for PhysX.
     * \param[in] footPos The simulation space position of the character's foot.
     * \param[in] upDir A unit vector defining the simulation space direction considered "up."
     * \returns The simulation position of the character's capsule actor.
     */
    hsPoint3 IGetCapsulePos(const hsPoint3& footPos, const hsVector3& upDir=hsVector3(0.f, 0.f, 1.f)) const;

    /**
     * Gets the Plasma position of the character.
     * \remarks Plasma stores the position of avatars from their "foot" position while PhysX
     *          uses the center of the capsule shape as its center. This returns the current position
     *          to be used for Plasma.
     * \param[in] capPos The simulation space position of the character's capsule.
     * \param[in] upDir A unit vector defining the simulation space direction considered "up."
     * \returns The simulation space position of the character.
     */
    hsPoint3 IGetCapsuleFoot(const hsPoint3& capPos, const hsVector3& upDir=hsVector3(0.f, 0.f, 1.f)) const;

    /**
     * Gets the global transform of the character's capsule.
     * \remarks PhysX capsules by default are not "upright" - this returns a transform that stands
     *          the capsule upright.
     * \param[in] upDir A unit vector defining the simulation space direction considered "up."
     * \returns The global transform of the capsule shape.
     */
    physx::PxTransform IGetCapsulePose(const hsPoint3& pos, const hsVector3& upDir) const;

    /**
     * Gets a capsule and global pose from the current character
     * appropriate for use in scene queries.
     */
    std::tuple<physx::PxCapsuleGeometry, physx::PxTransform> IGetCapsule(const hsPoint3& pos) const;

protected:
    friend class plPXControllerBehaviorCallback;
    friend class plPXControllerHitReport;

    void ISynchProxy();
    void IChangePhysicalOwnership();

    void ICreateController(const hsPoint3& pos);
    void IDeleteController();

#ifndef PLASMA_EXTERNAL_RELEASE
    void IDrawDebugDisplay(int controllerIdx);
    std::vector<plControllerHitRecord> fDbgCollisionInfo;
#endif

    std::set<plKey> fHitObjects;
    physx::PxController* fController;
    std::unique_ptr<plPXControllerBehaviorCallback> fBehaviorCallback;

    plPhysicalProxy* fProxyGen;
};

#endif
