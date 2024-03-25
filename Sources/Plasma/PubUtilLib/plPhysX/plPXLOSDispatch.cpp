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
#include "plLOSDispatch.h"
#include "plPhysXAPI.h"
#include "plPXConvert.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXSimDefs.h"
#include "plPXSimulation.h"
#include "plSimulationMgr.h"

#include "hsGeometry3.h"
#include "hsMatrix44.h"

#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

// ==========================================================================

plLOSDispatch::RaycastResult plLOSDispatch::IRaycast(hsPoint3 origin, hsPoint3 destination,
                                                     const plKey& world, plSimDefs::plLOSDB db,
                                                     bool closest, plSimDefs::plLOSDB cullDB)
{
    RaycastResult result(LOSResult::kMiss, nullptr, { 0.f, 0.f, 0.f }, { 0.f, 0.f, 0.f }, FLT_MAX);

    plPXSimulation* sim = plSimulationMgr::GetInstance()->GetPhysX();
    physx::PxScene* scene = sim->FindScene(world);
    if (!scene)
        return result;

    // The raycast comes in as worldspace, but if the player is in a subworld, we'll need
    // to convert it to subworld space.
    hsMatrix44 l2w;
    l2w.Reset();
    if (world) {
        if (plSceneObject* so = plSceneObject::ConvertNoRef(world->ObjectIsLoaded())) {
            l2w = so->GetLocalToWorld();
            origin = so->GetWorldToLocal() * origin;
            destination = so->GetWorldToLocal() * destination;
        }
    }

    hsVector3 direction = hsVector3(destination - origin);
    float magnitude = direction.Magnitude();
    if (magnitude <= 0.f)
        return result;
    direction.Normalize();

    plPXFilterData data;
    data.SetLOSDBs((plSimDefs::plLOSDB)((physx::PxU32)db | (physx::PxU32)cullDB));
    physx::PxQueryFilterData filter(data, physx::PxQueryFlag::eSTATIC |
                                          physx::PxQueryFlag::eDYNAMIC |
                                          physx::PxQueryFlag::ePREFILTER |
                                          physx::PxQueryFlag::ePOSTFILTER);
    if (!closest)
        filter.flags |= physx::PxQueryFlag::eANY_HIT;

    class plPXRaycastQueryFilter : public physx::PxQueryFilterCallback
    {
        plLOSDispatch* fDispatch;
        plSimDefs::plLOSDB fCullDB;

    public:
        plPXRaycastQueryFilter(plLOSDispatch* self, plSimDefs::plLOSDB cullDB)
            : fDispatch(self), fCullDB(cullDB)
        { }

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
            if (data->GetController() && !data->GetController()->IsEnabled())
                return physx::PxQueryHitType::eNONE;

            if (plSceneObject* so = plSceneObject::ConvertNoRef(data->GetKey()->ObjectIsLoaded())){
                if (!fDispatch->ITestHit(so))
                    return physx::PxQueryHitType::eNONE;
            }

            // Ensures all hits are returned.
            return physx::PxQueryHitType::eTOUCH;
        }

        physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData,
                                               const physx::PxQueryHit& hit) override
        {
            // If we are culling the LOS hits, any cull hit should prevent touches beyond that hit.
            if (fCullDB != plSimDefs::kLOSDBNone) {
                if (static_cast<const plPXFilterData&>(filterData).TestLOSDBs(fCullDB))
                    return physx::PxQueryHitType::eBLOCK;
            }

            return physx::PxQueryHitType::eTOUCH;
        }
    } filterCallback(this, cullDB);

    class plPXRaycastCallback : public physx::PxRaycastCallback
    {
        RaycastResult& fResult;
        plSimDefs::plLOSDB fCullDB;
        physx::PxRaycastHit fHits[32];

    public:
        plPXRaycastCallback(RaycastResult& result, plSimDefs::plLOSDB cullDB)
            : fResult(result),
              fCullDB(cullDB),
              physx::PxRaycastCallback(fHits, std::size(fHits))
        { }

        physx::PxAgain processTouches(const physx::PxRaycastHit* hits, physx::PxU32 nbHits) override
        {
            for (physx::PxU32 i = 0; i < nbHits; ++i) {
                const physx::PxRaycastHit& hit = hits[i];
                if (hit.distance < fResult.fDistance && hit.distance != 0.f) {
                    auto data = static_cast<plPXActorData*>(hit.actor->userData);
                    fResult.fResult = LOSResult::kHit;
                    fResult.fHitObj = data->GetKey();
                    fResult.fPoint = plPXConvert::Point(hit.position);
                    fResult.fNormal = plPXConvert::Vector(hit.normal);
                    fResult.fDistance = hit.distance;
                }
            }
            return true;
        }

        void finalizeQuery() override
        {
            // !closest/eANY_HIT will return the single hit as a blocking hit, even if it is
            // a touch, and will not call processTouches(). Also, any cull-hit is returned as
            // a blocking hit. So, we need to be careful about this...
            if (!hasBlock)
                return;

            if (fCullDB != plSimDefs::kLOSDBNone) {
                auto filterData = (plPXFilterData)block.shape->getSimulationFilterData();
                if (filterData.TestLOSDBs(fCullDB)) {
                    fResult.fResult = LOSResult::kCull;
                    return;
                }
            }

            if (block.distance < fResult.fDistance && block.distance != 0.f) {
                auto data = static_cast<plPXActorData*>(block.actor->userData);
                fResult.fResult = LOSResult::kHit;
                fResult.fHitObj = data->GetKey();
                fResult.fPoint = plPXConvert::Point(block.position);
                fResult.fNormal = plPXConvert::Vector(block.normal);
                fResult.fDistance = block.distance;
            }
        }
    } raycast(result, cullDB);

    bool hit = scene->raycast(plPXConvert::Point(origin),
                              plPXConvert::Vector(direction),
                              magnitude, raycast,
                              (physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL),
                              filter, &filterCallback);

    // Convert back to worldspace
    if (hit) {
        result.fPoint = l2w * result.fPoint;
        result.fNormal = l2w * result.fNormal;
    }
    return result;
}
