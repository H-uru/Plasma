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
#ifndef plPXPhysical_h_inc
#define plPXPhysical_h_inc

#include <memory>

#include "hsBitVector.h"
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "plPhysical.h"
#include "hsQuat.h"

#include "pnKeyedObject/plKey.h"

#include "plPhysical/plSimDefs.h"

class plDrawableSpans;
class plGenRefMsg;
class hsGMaterial;
class plLOSHit;
class plMessage;
class plPhysicalProxy;
struct hsPlane3;
struct hsPoint3;
class hsQuat;
class plPhysicalSndGroup;
class plSceneObject;
class plSDLModifier;
class plSimulationMsg;
class hsVectorStream;

namespace physx
{
    class PxBoxGeometry;
    class PxConvexMesh;
    class PxRigidActor;
    class PxSphereGeometry;
    class PxTriangleMesh;
};

class PhysRecipe
{
public:
    float mass;
    float friction;
    float restitution;
    plSimDefs::Bounds bounds;
    plSimDefs::Group group;
    uint32_t reportsOn;
    plKey objectKey;
    plKey sceneNode;
    plKey worldKey;

    // The local to subworld transform (or local to world if worldKey is nil)
    hsPoint3 l2sP;
    hsQuat l2sQ;

    physx::PxConvexMesh* convexMesh;
    physx::PxTriangleMesh* triMesh;

    // For spheres only
    float radius;
    hsPoint3 offset;

    // For Boxes
    hsPoint3 bDimensions;
    hsPoint3 bOffset;

    std::unique_ptr<hsVectorStream> meshStream;

    PhysRecipe();
    ~PhysRecipe();
};

class plPXPhysical : public plPhysical
{
protected:
    void InitProxy();
    void IMoveProxy(const hsMatrix44& l2w);

    void InitSDL();
    void InitRefs() const;

public:
    /**
     * Returns if this physical is a kinematic rigid body.
     * \remarks Kinematic rigid bodies are bodies that are movable in the simulation
     *          but will ignore external forces such as gravity and collision.
     *          Manually applying forces, impulses, etc. has no meaning on these bodies.
     */
    bool IsKinematic() const;

    /**
     * Returns if this physical is a dynamic rigid body.
     * \remarks Dynamic rigid bodies are bodies that are movable in the simulation
     *          and can respond to external forces such as gravity and collision.
     */
    bool IsDynamic() const;

    /**
     * Returns if this physical is a static actor.
     * \remarks Static actors are fixed in the simulation and should not be moved--doing so incurs
     *          a performance penalty.
     */
    bool IsStatic() const;

protected:
    /**
     * Returns if this physical represents a trigger shape.
     */
    bool IsTrigger() const;

    /**
     * Sets the actor to be either a trigger or simulation shape.
     */
    void IUpdateShapeFlags();

    void ISyncFilterData();

protected:
    void ISanityCheckGeometry(physx::PxBoxGeometry& geometry) const;
    void ISanityCheckGeometry(physx::PxSphereGeometry& geometry) const;
    void ISanityCheckBounds();
    void ISanityCheckRecipe();

public:
    friend class plSimulationMgr;
    friend class plPXSimulation;

    enum PhysRefType
    {
        kPhysRefWorld,
        kPhysRefSndGroup
    };

    plPXPhysical();
    ~plPXPhysical() override;

    CLASSNAME_REGISTER(plPXPhysical);
    GETINTERFACE_ANY(plPXPhysical, plPhysical);

    // Export time and internal use only
    bool InitActor();
    physx::PxConvexMesh* ICookHull(hsStream* s);
    physx::PxTriangleMesh* ICookTriMesh(hsStream* s);

    void Read(hsStream* s, hsResMgr* mgr) override;
    void Write(hsStream* s, hsResMgr* mgr) override;

    bool MsgReceive(plMessage* msg) override;

    //
    // From plPhysical
    //
    plPhysical& SetProperty(int prop, bool b) override;
    bool GetProperty(int prop) const override { return fProps.IsBitSet(prop) != 0; }

    void SetObjectKey(plKey key) override { fObjectKey = key; }
    plKey GetObjectKey() const override { return fObjectKey; }

    void SetSceneNode(plKey node) override;
    plKey GetSceneNode() const override { return fSceneNode; }

    bool GetLinearVelocitySim(hsVector3& vel) const override;
    void SetLinearVelocitySim(const hsVector3& vel, bool wakeup=true) override;
    void ClearLinearVelocity() override;

    bool GetAngularVelocitySim(hsVector3& vel) const override;
    void SetAngularVelocitySim(const hsVector3& vel, bool wakeup=true) override;

    void SetImpulseSim(const hsVector3& imp, bool wakeup=true);

    void SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, bool force=false) override;
    void GetTransform(hsMatrix44& l2w, hsMatrix44& w2l) override;

    int GetGroup() const override { return fGroup; }
    void SetGroup(int group) override
    {
        fGroup = (plSimDefs::Group)group;
        IUpdateShapeFlags();
        ISyncFilterData();
        if (fActor)
            InitProxy();
    }

    void AddLOSDB(uint16_t flag) override
    {
        hsSetBits(fLOSDBs, flag);
        ISyncFilterData();
    }

    void RemoveLOSDB(uint16_t flag) override
    {
        hsClearBits(fLOSDBs, flag);
        ISyncFilterData();
    }

    uint16_t GetAllLOSDBs() override { return fLOSDBs; }
    bool IsInLOSDB(uint16_t flag) override { return hsCheckBits(fLOSDBs, flag); }

    plKey GetWorldKey() const override { return fWorldKey; }

    plPhysicalSndGroup* GetSoundGroup() const override { return fSndGroup; }

    void GetPositionSim(hsPoint3& pos) const override
    {
        hsQuat junk;
        IGetPoseSim(pos, junk);
    }

    void SendNewLocation(bool synchTransform = false, bool isSynchUpdate = false) override;

    void GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV) override;
    void SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV) override;
    void ResetSyncState() override;

    plDrawableSpans* CreateProxy(hsGMaterial* mat, std::vector<uint32_t>& idx, plDrawableSpans* addTo) override;

    float GetMass() override { return fRecipe.mass; }

    PhysRecipe& GetRecipe() { return fRecipe; }
    const PhysRecipe& GetRecipe() const { return fRecipe; }

protected:
    void IUpdateSubworld();
    void DestroyActor();
    bool CanSynchPosition(bool isSynchUpdate) const;

    void IGetPoseSim(hsPoint3& pos, hsQuat& rot) const;
    void ISetPoseSim(const hsPoint3* pos, const hsQuat* rot, bool wakeup=true);

    /** Handle messages about our references. */
    bool HandleRefMsg(plGenRefMsg * refM);

    /** See if the object is in a valid, non-overlapping position.
        A valid overlap is one which is approved by the collision
        masking code, i.e. my memberOf has no intersection with your
        bounceOff and vice-versa
        */
    // Set overlapText to get a string naming all the overlapping physicals (that you must delete)
    bool CheckValidPosition(char** overlapText=nullptr);

    /////////////////////////////////////////////////////////////
    //
    // NETWORK SYNCHRONIZATION
    //
    /////////////////////////////////////////////////////////////

    /** Remember that we need to do a synch soon. */
    bool DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags) override;

    double GetLastSyncTime() { return fLastSyncTime; }

    /** Get the simulation transform of the physical, in world
    coordinates (factoring in the subworld if necessary */
    void IGetTransformGlobal(hsMatrix44 &l2w) const;
    void ISetTransformGlobal(const hsMatrix44& l2w);

    // Enable/disable collisions and dynamic movement
    void IEnable(bool enable);


    physx::PxRigidActor* fActor;
    plKey fWorldKey;    // either a subworld or nil

    PhysRecipe fRecipe;
    plSimDefs::Bounds fBounds;
    plSimDefs::Group fGroup;
    uint32_t fReportsOn;          // bit vector for groups we report interactions with
    uint16_t fLOSDBs;             // Which LOS databases we get put into
    hsBitVector fProps;         // plSimulationInterface::plSimulationProperties kept here

    plKey fObjectKey;           // the key to our scene object
    plKey fSceneNode;           // the room we're in

    // we need to remember the last matrices we sent to the coordinate interface
    // so that we can recognize them when we send them back and not reapply them,
    // which would reactivate our body. inelegant but effective
    hsMatrix44 fCachedLocal2World;

    // Syncronization
    double          fLastSyncTime;
    plSDLModifier*  fSDLMod;

    plPhysicalSndGroup* fSndGroup;

    std::unique_ptr<plPhysicalProxy> fProxyGen;             // visual proxy for debugging

    static int  fNumberAnimatedPhysicals;
    static int  fNumberAnimatedActivators;
};

#endif
