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
#include "plPXPhysical.h"

#include <NxPhysics.h>
#include <NxCooking.h>

#include "plProfile.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsQuat.h"

#include "plPXCooking.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXConvert.h"
#include "plPXStream.h"
#include "plSimulationMgr.h"

#include "plDrawable/plDrawableGenerator.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plSDLModifierMsg.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "plSurface/hsGMaterial.h"
#include "plSurface/plLayer.h"

// ==========================================================================

bool plPXPhysical::InitActor()
{
    bool    startAsleep = false;
    fGroup = fRecipe.group;
    fObjectKey = fRecipe.objectKey;
    fReportsOn = fRecipe.reportsOn;
    fSceneNode = fRecipe.sceneNode;

    NxActorDesc actorDesc;
    NxSphereShapeDesc sphereDesc;
    NxConvexShapeDesc convexShapeDesc;
    NxTriangleMeshShapeDesc trimeshShapeDesc;
    NxBoxShapeDesc boxDesc;

    plPXConvert::Matrix(fRecipe.l2s, actorDesc.globalPose);

    switch (fRecipe.bounds)
    {
    case plSimDefs::kSphereBounds:
        {
            hsMatrix44 sphereL2W;
            sphereL2W.Reset();
            sphereL2W.SetTranslate(&fRecipe.offset);

            sphereDesc.radius = fRecipe.radius;
            plPXConvert::Matrix(sphereL2W, sphereDesc.localPose);
            sphereDesc.group = fRecipe.group;
            actorDesc.shapes.pushBack(&sphereDesc);
        }
        break;
    case plSimDefs::kHullBounds:
        {
            convexShapeDesc.meshData = fRecipe.convexMesh;
            convexShapeDesc.group = fRecipe.group;
            actorDesc.shapes.pushBack(&convexShapeDesc);
        }
        break;
    case plSimDefs::kBoxBounds:
        {
            boxDesc.dimensions = plPXConvert::Point(fRecipe.bDimensions);

            hsMatrix44 boxL2W;
            boxL2W.Reset();
            boxL2W.SetTranslate(&fRecipe.bOffset);
            plPXConvert::Matrix(boxL2W, boxDesc.localPose);

            boxDesc.group = fRecipe.group;
            actorDesc.shapes.push_back(&boxDesc);
        }
        break;
    case plSimDefs::kExplicitBounds:
    case plSimDefs::kProxyBounds:
        if (fRecipe.group == plSimDefs::kGroupDetector)
        {
            SimLog("Someone using an Exact on a detector region: {}", GetKeyName());
        }
        trimeshShapeDesc.meshData = fRecipe.triMesh;
        trimeshShapeDesc.group = fRecipe.group;
        actorDesc.shapes.pushBack(&trimeshShapeDesc);
        break;
    default:
        hsAssert(false, "Unknown geometry type during read.");
        return false;
        break;
    }

    //  Now fill out the body, or dynamic part of the physical
    NxBodyDesc bodyDesc;
    if (fRecipe.mass != 0)
    {
        bodyDesc.mass = fRecipe.mass;
        actorDesc.body = &bodyDesc;

        if (GetProperty(plSimulationInterface::kPinned))
        {
            bodyDesc.flags |= NX_BF_FROZEN;
            startAsleep = true;             // put it to sleep if they are going to be frozen
        }

        if (fRecipe.group != plSimDefs::kGroupDynamic || GetProperty(plSimulationInterface::kPhysAnim))
        {
            SetProperty(plSimulationInterface::kPassive, true);

            // Even though the code for animated physicals and animated activators are the same
            // keep these code snippets separated for fine tuning. Thanks.
            if (fRecipe.group == plSimDefs::kGroupDynamic)
            {
                // handle the animated physicals.... make kinematic for now.
                fNumberAnimatedPhysicals++;
                bodyDesc.flags |= NX_BF_KINEMATIC;
                startAsleep = true;
            }
            else
            {
                // handle the animated activators.... 
                fNumberAnimatedActivators++;
                bodyDesc.flags |= NX_BF_KINEMATIC;
                startAsleep = true;
            }

        }
    }
    else
    {
        if ( GetProperty(plSimulationInterface::kPhysAnim) )
            SimLog("An animated physical that has no mass: {}", GetKeyName());
    }

    actorDesc.userData = this;
    actorDesc.name = GetKeyName().c_str();

    // Put the dynamics into actor group 1.  The actor groups are only used for
    // deciding who we get contact reports for.
    if (fRecipe.group == plSimDefs::kGroupDynamic)
        actorDesc.group = 1;

    NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
    try
    {
        fActor = scene->createActor(actorDesc);
    } catch (...)
    {
        hsAssert(false, "Actor creation crashed");
        return false;
    }
    hsAssert(fActor, "Actor creation failed");
    if (!fActor)
        return false;

    NxShape* shape = fActor->getShapes()[0];
    shape->setMaterial(plSimulationMgr::GetInstance()->GetMaterialIdx(scene, fRecipe.friction, fRecipe.restitution));

    // Turn on the trigger flags for any detectors.
    //
    // Normally, we'd set these flags on the shape before it's created.  However,
    // in the case where the detector is going to be animated, it'll have a rigid
    // body too, and that will cause problems at creation.  According to Ageia,
    // a detector shape doesn't actually count as a shape, so the SDK will have
    // problems trying to calculate an intertial tensor.  By letting it be
    // created as a normal dynamic first, then setting the flags, we work around
    // that problem.
    if (fRecipe.group == plSimDefs::kGroupDetector)
    {
        shape->setFlag(NX_TRIGGER_ON_ENTER, true);
        shape->setFlag(NX_TRIGGER_ON_LEAVE, true);
    }

    if (GetProperty(plSimulationInterface::kStartInactive) || startAsleep)
    {
        if (!fActor->isSleeping())
        {
            if (plSimulationMgr::fExtraProfile)
                SimLog("Deactivating {} in SetPositionAndRotationSim", GetKeyName());
            fActor->putToSleep();
        }
    }

    if (GetProperty(plSimulationInterface::kDisable))
        IEnable(false);
    if (GetProperty(plSimulationInterface::kSuppressed_DEAD))
        IEnable(false);

    plNodeRefMsg* refMsg = new plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kPhysical); 
    hsgResMgr::ResMgr()->AddViaNotify(GetKey(), refMsg, plRefFlags::kActiveRef);

    // only dynamic physicals without noSync need SDLs
    if (fRecipe.group == plSimDefs::kGroupDynamic && !fProps.IsBitSet(plSimulationInterface::kNoSynchronize))
        InitSDL();

    return true;
}

void plPXPhysical::DestroyActor()
{
    if (fActor) {
        // Grab any mesh we may have (they need to be released manually)
        NxConvexMesh* convexMesh = nil;
        NxTriangleMesh* triMesh = nil;
        NxShape* shape = fActor->getShapes()[0];
        if (NxConvexShape* convexShape = shape->isConvexMesh())
            convexMesh = &convexShape->getConvexMesh();
        else if (NxTriangleMeshShape* trimeshShape = shape->isTriangleMesh())
            triMesh = &trimeshShape->getTriangleMesh();

        if (!fActor->isDynamic())
            plPXPhysicalControllerCore::RebuildCache();

        if (fActor->isDynamic() && fActor->readBodyFlag(NX_BF_KINEMATIC)) {
            if (fGroup == plSimDefs::kGroupDynamic)
                fNumberAnimatedPhysicals--;
            else
                fNumberAnimatedActivators--;
        }

        // Release the actor
        NxScene* scene = plSimulationMgr::GetInstance()->GetScene(fWorldKey);
        scene->releaseActor(*fActor);
        fActor = nil;

        // Now that the actor is freed, release the mesh
        if (convexMesh)
            plSimulationMgr::GetInstance()->GetSDK()->releaseConvexMesh(*convexMesh);
        if (triMesh)
            plSimulationMgr::GetInstance()->GetSDK()->releaseTriangleMesh(*triMesh);

        // Release the scene, so it can be cleaned up if no one else is using it
        plSimulationMgr::GetInstance()->ReleaseScene(fWorldKey);
    }
}

// ==========================================================================

void plPXPhysical::IEnable(bool enable)
{
    fProps.SetBit(plSimulationInterface::kDisable, !enable);
    if (!enable)
    {
        fActor->raiseActorFlag(NX_AF_DISABLE_COLLISION);
        if (fActor->isDynamic())
            fActor->raiseBodyFlag(NX_BF_FROZEN);
        else
            plPXPhysicalControllerCore::RebuildCache();
    }
    else
    {
        fActor->clearActorFlag(NX_AF_DISABLE_COLLISION);

        if (fActor->isDynamic())
            fActor->clearBodyFlag(NX_BF_FROZEN);
        else
            plPXPhysicalControllerCore::RebuildCache();
    }
}

plPhysical& plPXPhysical::SetProperty(int prop, bool status)
{
    if (GetProperty(prop) == status)
    {
        const char* propName = "(unknown)";
        switch (prop)
        {
        case plSimulationInterface::kDisable:           propName = "kDisable";              break;
        case plSimulationInterface::kPinned:            propName = "kPinned";               break;
        case plSimulationInterface::kPassive:           propName = "kPassive";              break;
        case plSimulationInterface::kPhysAnim:          propName = "kPhysAnim";             break;
        case plSimulationInterface::kStartInactive:     propName = "kStartInactive";        break;
        case plSimulationInterface::kNoSynchronize:     propName = "kNoSynchronize";        break;
        }

        ST::string name = ST_LITERAL("(unknown)");
        if (GetKey())
            name = GetKeyName();
        if (plSimulationMgr::fExtraProfile)
            plSimulationMgr::Log("Warning: Redundant physical property set (property {}, value {}) on {}", propName, status ? "true" : "false", name);
    }

    switch (prop)
    {
    case plSimulationInterface::kDisable:
        IEnable(!status);
        break;

    case plSimulationInterface::kPinned:
        if (fActor->isDynamic())
        {
            // if the body is already unpinned and you unpin it again,
            // you'll wipe out its velocity. hence the check.
            bool current = fActor->readBodyFlag(NX_BF_FROZEN);
            if (status != current)
            {
                if (status)
                    fActor->raiseBodyFlag(NX_BF_FROZEN);
                else
                {
                    fActor->clearBodyFlag(NX_BF_FROZEN);
                    fActor->wakeUp();
                }
            }
        }
        break;
    }

    fProps.SetBit(prop, status);

    return *this;
}

bool plPXPhysical::CanSynchPosition(bool isSynchUpdate) const
{
    return (!fActor->isSleeping() || isSynchUpdate) && fActor->isDynamic();
}

void plPXPhysical::ApplyHitForce()
{
    if (fActor && fWeWereHit)
    {
        fActor->addForceAtPos(plPXConvert::Vector(fHitForce), plPXConvert::Point(fHitPos), NX_FORCE);
        fWeWereHit = false;
    }
}

void plPXPhysical::ISetTransformGlobal(const hsMatrix44& l2w)
{
    if (!fActor->isDynamic()) {
        SimLog("Tried to move a static actor '{}'", GetKeyName());
        return;
    }

    // If we wake up normal dynamic actors, they might explode.
    // However, kinematics won't update if they are asleep. Thankfully, kinematics don't
    //          explode, move, or cause spontaneous nuclear warfare.
    if (fActor->readBodyFlag(NX_BF_KINEMATIC))
        fActor->wakeUp();

    NxMat34 mat;

    if (fWorldKey) {
        plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
        hsAssert(so, "Scene object not loaded while accessing subworld.");
        // physical to subworld (simulation space)
        hsMatrix44 p2s = so->GetCoordinateInterface()->GetWorldToLocal() * l2w;
        plPXConvert::Matrix(p2s, mat);
        IMoveProxy(p2s);
    } else {
        // No need to localize
        plPXConvert::Matrix(l2w, mat);
        IMoveProxy(l2w);
    }

    // This used to check for the kPhysAnim flag, however animated detectors
    // are also kinematic but not kPhysAnim, therefore, this would break on PhysX
    // SDKs (yes, I'm looking at you, 2.6.4) that actually obey the ***GlobalPose 
    // rules set forth in the SDK documentation.
    if (fActor->readBodyFlag(NX_BF_KINEMATIC))
        fActor->moveGlobalPose(mat);
    else
        fActor->setGlobalPose(mat);
}

// the physical may have several parents between it and the subworld object,
// but the *havok* transform is only one level away from the subworld.
// to avoid any confusion about this difference, we avoid referring to the 
// subworld as "parent" and use, for example, "l2s" (local-to-sub) instead
// of the canonical plasma "l2p" (local-to-parent)
void plPXPhysical::IGetTransformGlobal(hsMatrix44& l2w) const
{
    plPXConvert::Matrix(fActor->getGlobalPose(), l2w);

    if (fWorldKey) {
        plSceneObject* so = plSceneObject::ConvertNoRef(fWorldKey->ObjectIsLoaded());
        hsAssert(so, "Scene object not loaded while accessing subworld.");
        if (so->GetCoordinateInterface()) {
             const hsMatrix44& s2w = so->GetCoordinateInterface()->GetLocalToWorld();
             l2w = s2w * l2w;
        }
    }
}

void plPXPhysical::IGetPositionSim(hsPoint3& pos) const
{
    pos = plPXConvert::Point(fActor->getGlobalPosition());
}

void plPXPhysical::IGetRotationSim(hsQuat& rot) const
{
    rot = plPXConvert::Quat(fActor->getGlobalOrientationQuat());
}
void plPXPhysical::ISetPositionSim(const hsPoint3& pos)
{
    if (GetProperty(plSimulationInterface::kPhysAnim))
        fActor->moveGlobalPosition(plPXConvert::Point(pos));
    else
        fActor->setGlobalPosition(plPXConvert::Point(pos));
}

void plPXPhysical::ISetRotationSim(const hsQuat& rot)
{
    if (GetProperty(plSimulationInterface::kPhysAnim))
        fActor->moveGlobalOrientation(plPXConvert::Quat(rot));
    else
        fActor->setGlobalOrientation(plPXConvert::Quat(rot));
}

bool plPXPhysical::GetLinearVelocitySim(hsVector3& vel) const
{
    bool result = false;

    if (fActor->isDynamic())
    {
        vel = plPXConvert::Vector(fActor->getLinearVelocity());
        result = true;
    }
    else
        vel.Set(0, 0, 0);

    return result;
}

void plPXPhysical::SetLinearVelocitySim(const hsVector3& vel)
{
    if (fActor->isDynamic())
        fActor->setLinearVelocity(plPXConvert::Vector(vel));
}

void plPXPhysical::ClearLinearVelocity()
{
    SetLinearVelocitySim(hsVector3(0, 0, 0));
}

bool plPXPhysical::GetAngularVelocitySim(hsVector3& vel) const
{
    bool result = false;
    if (fActor->isDynamic())
    {
        vel = plPXConvert::Vector(fActor->getAngularVelocity());
        result = true;
    }
    else
        vel.Set(0, 0, 0);

    return result;
}

void plPXPhysical::SetAngularVelocitySim(const hsVector3& vel)
{
    if (fActor->isDynamic())
        fActor->setAngularVelocity(plPXConvert::Vector(vel));
}

// ==========================================================================

NxConvexMesh* plPXPhysical::IReadHull(hsStream* s)
{
    std::vector<uint32_t> tris;
    std::vector<hsPoint3> verts;
    try {
         plPXCooking::ReadConvexHull26(s, tris, verts);
    } catch (const plPXCookingException& ex) {
        SimLog("Failed to uncook convex hull '{}': {}", GetKeyName(), ex.what());
        return nullptr;
    }

    // Unfortunately, the only way I know of to accomplish this is to cook to a RAM stream,
    // then have PhysX read the cooked data from the RAM stream. Yes, this is very sad.
    // I blame PhysX. It needs to die in a fiaaaaaaaaaaah
    hsRAMStream ram;
    plPXStream pxs(&ram);

    NxConvexMeshDesc desc;
    desc.numVertices = verts.size();
    desc.pointStrideBytes = sizeof(hsPoint3);
    desc.points = &verts[0];
    desc.triangleStrideBytes = sizeof(uint32_t);
    desc.triangles = &tris[0];
    desc.flags = NX_CF_COMPUTE_CONVEX | NX_CF_USE_UNCOMPRESSED_NORMALS;
    if (!NxCookConvexMesh(desc, pxs)) {
        SimLog("Failed to cook hull for '{}'", GetKey()->GetName());
        return nullptr;
    }

    ram.Rewind();
    return plSimulationMgr::GetInstance()->GetSDK()->createConvexMesh(pxs);
}

NxTriangleMesh* plPXPhysical::IReadTriMesh(hsStream* s)
{
    std::vector<uint32_t> tris;
    std::vector<hsPoint3> verts;
    try {
         plPXCooking::ReadTriMesh26(s, tris, verts);
    } catch (const plPXCookingException& ex) {
        SimLog("Failed to uncook triangle mesh '{}': {}", GetKeyName(), ex.what());
        return nullptr;
    }

    // Unfortunately, the only way I know of to accomplish this is to cook to a RAM stream,
    // then have PhysX read the cooked data from the RAM stream. Yes, this is very sad.
    // I blame PhysX. It needs to die in a fiaaaaaaaaaaah
    hsRAMStream ram;
    plPXStream pxs(&ram);

    NxTriangleMeshDesc desc;
    desc.numVertices = verts.size();
    desc.pointStrideBytes = sizeof(hsPoint3);
    desc.points = &verts[0];
    desc.numTriangles = tris.size() / 3;
    desc.triangleStrideBytes = sizeof(uint32_t) * 3;
    desc.triangles = &tris[0];
    desc.flags = 0;
    if (!NxCookTriangleMesh(desc, pxs)) {
        SimLog("Failed to cook trimesh for '{}'", GetKey()->GetName());
        return nullptr;
    }

    ram.Rewind();
    return plSimulationMgr::GetInstance()->GetSDK()->createTriangleMesh(pxs);
}

// ==========================================================================

void plPXPhysical::ExcludeRegionHack(bool cleared)
{
    NxShape* shape = fActor->getShapes()[0];
    shape->setFlag(NX_TRIGGER_ON_ENTER, !cleared);
    shape->setFlag(NX_TRIGGER_ON_LEAVE, !cleared);
    fGroup = cleared ? plSimDefs::kGroupExcludeRegion : plSimDefs::kGroupDetector;
    shape->setGroup(fGroup);
    /*if switching a static need to inform the controller that it needs to rebuild
    the collision cache otherwise will still think that the detector is still static or that
    the static is still a detector*/
    plPXPhysicalControllerCore::RebuildCache();

}
bool plPXPhysical::OverlapWithController(const plPXPhysicalControllerCore* controller)
{
    NxCapsule cap;
    controller->GetWorldSpaceCapsule(cap);
    NxShape* shape = fActor->getShapes()[0];
    return shape->checkOverlapCapsule(cap);
}

bool plPXPhysical::IsDynamic() const
{
    return fGroup == plSimDefs::kGroupDynamic &&
        !GetProperty(plSimulationInterface::kPhysAnim);
}

// Some helper functions for pulling info out of a PhysX trimesh description
inline hsPoint3& GetTrimeshVert(NxTriangleMeshDesc& desc, int idx)
{
    return *((hsPoint3*)(((char*)desc.points)+desc.pointStrideBytes*idx));
}

// ==========================================================================

static void GetTrimeshTri(const NxTriangleMeshDesc& desc, int idx, uint16_t* out)
{
    if (hsCheckBits(desc.flags, NX_MF_16_BIT_INDICES))
    {
        uint16_t* descTris = ((uint16_t*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
        out[0] = descTris[0];
        out[1] = descTris[1];
        out[2] = descTris[2];
    }
    else
    {
        uint32_t* descTris = ((uint32_t*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
        out[0] = (uint16_t)descTris[0];
        out[1] = (uint16_t)descTris[1];
        out[2] = (uint16_t)descTris[2];
    }
}

// Some helper functions for pulling info out of a PhysX trimesh description
static inline hsPoint3& GetConvexVert(NxConvexMeshDesc& desc, int idx)
{
    return *((hsPoint3*)(((char*)desc.points)+desc.pointStrideBytes*idx));
}

static void GetConvexTri(const NxConvexMeshDesc& desc, int idx, uint16_t* out)
{
    if (hsCheckBits(desc.flags, NX_MF_16_BIT_INDICES))
    {
        uint16_t* descTris = ((uint16_t*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
        out[0] = descTris[0];
        out[1] = descTris[1];
        out[2] = descTris[2];
    }
    else
    {
        uint32_t* descTris = ((uint32_t*)(((char*)desc.triangles)+desc.pointStrideBytes*idx));
        out[0] = (uint16_t)descTris[0];
        out[1] = (uint16_t)descTris[1];
        out[2] = (uint16_t)descTris[2];
    }
}

// Make a visible object that can be viewed by users for debugging purposes.
plDrawableSpans* plPXPhysical::CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo)
{
    plDrawableSpans* myDraw = addTo;
    hsMatrix44 l2w, unused;
    GetTransform(l2w, unused);
    
    bool blended = ((mat->GetLayer(0)->GetBlendFlags() & hsGMatState::kBlendMask));

    NxShape* shape = fActor->getShapes()[0];

    NxTriangleMeshShape* trimeshShape = shape->isTriangleMesh();
    if (trimeshShape)
    {
        NxTriangleMeshDesc desc;
        trimeshShape->getTriangleMesh().saveToDesc(desc);

        hsTArray<hsPoint3>  pos;
        hsTArray<uint16_t>    tris;

        const int kMaxTris = 10000;
        const int kMaxVerts = 32000;
        if ((desc.numVertices < kMaxVerts) && (desc.numTriangles < kMaxTris))
        {
            pos.SetCount(desc.numVertices);
            tris.SetCount(desc.numTriangles * 3);

            for (int i = 0; i < desc.numVertices; i++ )
                pos[i] = GetTrimeshVert(desc, i);

            for (int i = 0; i < desc.numTriangles; i++)
                GetTrimeshTri(desc, i, &tris[i*3]);

            myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
                                            pos.AcquireArray(),
                                            nil,    // normals - def to avg (smooth) norm
                                            nil,    // uvws
                                            0,      // uvws per vertex
                                            nil,    // colors - def to white
                                            true,   // do a quick fake shade
                                            nil,    // optional color modulation
                                            tris.GetCount(),
                                            tris.AcquireArray(),
                                            mat,
                                            l2w,
                                            blended,
                                            &idx,
                                            myDraw);
        }
        else
        {
            int curTri = 0;
            int trisToDo = desc.numTriangles;
            while (trisToDo > 0)
            {
                int trisThisRound = trisToDo > kMaxTris ? kMaxTris : trisToDo;
                
                trisToDo -= trisThisRound;

                pos.SetCount(trisThisRound * 3);
                tris.SetCount(trisThisRound * 3);

                for (int i = 0; i < trisThisRound; i++)
                {
                    GetTrimeshTri(desc, curTri, &tris[i*3]);
                    pos[i*3 + 0] = GetTrimeshVert(desc, tris[i*3+0]);
                    pos[i*3 + 1] = GetTrimeshVert(desc, tris[i*3+1]);
                    pos[i*3 + 2] = GetTrimeshVert(desc, tris[i*3+2]);

                    curTri++;
                }
                myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
                                                pos.AcquireArray(),
                                                nil,    // normals - def to avg (smooth) norm
                                                nil,    // uvws
                                                0,      // uvws per vertex
                                                nil,    // colors - def to white
                                                true,   // do a quick fake shade
                                                nil,    // optional color modulation
                                                tris.GetCount(),
                                                tris.AcquireArray(),
                                                mat,
                                                l2w,
                                                blended,
                                                &idx,
                                                myDraw);
            }
        }
    }

    NxConvexShape* convexShape = shape->isConvexMesh();
    if (convexShape)
    {
        NxConvexMeshDesc desc;
        convexShape->getConvexMesh().saveToDesc(desc);

        hsTArray<hsPoint3>  pos;
        hsTArray<uint16_t>    tris;

        pos.SetCount(desc.numVertices);
        tris.SetCount(desc.numTriangles * 3);

        for (int i = 0; i < desc.numVertices; i++ )
            pos[i] = GetConvexVert(desc, i);

        for (int i = 0; i < desc.numTriangles; i++)
            GetConvexTri(desc, i, &tris[i*3]);

        myDraw = plDrawableGenerator::GenerateDrawable(pos.GetCount(), 
            pos.AcquireArray(),
            nil,    // normals - def to avg (smooth) norm
            nil,    // uvws
            0,      // uvws per vertex
            nil,    // colors - def to white
            true,   // do a quick fake shade
            nil,    // optional color modulation
            tris.GetCount(),
            tris.AcquireArray(),
            mat,
            l2w,
            blended,
            &idx,
            myDraw);
    }

    NxSphereShape* sphere = shape->isSphere();
    if (sphere)
    {
        float radius = sphere->getRadius();
        hsPoint3 offset = plPXConvert::Point(sphere->getLocalPosition());
        myDraw = plDrawableGenerator::GenerateSphericalDrawable(offset, radius,
            mat, l2w, blended,
            nil, &idx, myDraw);
    }

    NxBoxShape* box = shape->isBox();
    if (box)
    {
        hsPoint3 dim = plPXConvert::Point(box->getDimensions());
        myDraw = plDrawableGenerator::GenerateBoxDrawable(dim.fX*2.f, dim.fY*2.f, dim.fZ*2.f,
            mat,l2w,blended,
            nil,&idx,myDraw);
    }
    return myDraw;
}
