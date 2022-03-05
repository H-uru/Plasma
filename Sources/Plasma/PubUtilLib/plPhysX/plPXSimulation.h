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
#ifndef plPXSimulation_H
#define plPXSimulation_H

#include "plFileSystem.h"
#include "pnKeyedObject/plKey.h"

#include <map>
#include <optional>
#include <string_theory/string>
#include <vector>

class hsKeyedObject;
struct hsPoint3;
class plPXFilterData;
class plPXPhysical;
class plPXPhysicalControllerCore;
class hsQuat;

namespace physx
{
    class PxActor;
    class PxConvexMesh;
    class PxCooking;
    class PxController;
    class PxControllerDesc;
    class PxControllerManager;
    class PxDefaultCpuDispatcher;
    class PxFoundation;
    class PxGeometry;
    class PxMaterial;
    class PxPvd;
    class PxPvdTransport;
    class PxPhysics;
    class PxRigidActor;
    class PxScene;
    class PxTransform;
    class PxTriangleMesh;
};

enum class plPXActorType
{
    kUnset,
    kStaticActor,
    kDynamicActor,
    kKinematicActor,
};

enum class plPXDebuggerStatus
{
    kNotSupported,
    kConnectionFailed,
    kConnectedToFile,
    kConnectedToTcp,
    kDisconnected,
};

class plPXActorData
{
    plKey fKey;
    plPXPhysical* fPhysical;
    plPXPhysicalControllerCore* fController;
    ST::char_buffer fNameBuf;

public:
    plPXActorData() = delete;
    plPXActorData(const plPXActorData&) = delete;
    plPXActorData(plPXActorData&&) = delete;
    plPXActorData(plPXPhysical* physical);
    plPXActorData(plPXPhysicalControllerCore* controller);

    /** Gets the key of the owner object. */
    [[nodiscard]]
    plKey GetKey() const { return fKey; }

    [[nodiscard]]
    plPXPhysical* GetPhysical() const { return fPhysical; }

    [[nodiscard]]
    plPXPhysicalControllerCore* GetController() const { return fController; }

    [[nodiscard]]
    const char* c_str() const { return fNameBuf.c_str(); }

    [[nodiscard]]
    ST::string str() const { return fNameBuf; }
};

class plPXSimulation
{
    friend class plSimulationMgr;

protected:
    struct World
    {
        physx::PxScene* fScene{};
        physx::PxControllerManager* fControllers{};
    };

    physx::PxFoundation* fPxFoundation;
    physx::PxPvd* fDebugger;
    physx::PxPvdTransport* fTransport;
    physx::PxPhysics* fPxPhysics;
    physx::PxCooking* fPxCooking;
    physx::PxDefaultCpuDispatcher* fPxCpuDispatcher;
    std::map<plKey, World> fWorlds;
    float fAccumulator;

protected:
    bool IConnectDebugger(physx::PxPvdTransport* transport);

public:
    plPXSimulation();
    plPXSimulation(const plPXSimulation&) = delete;
    plPXSimulation(plPXSimulation&&) = delete;

    ~plPXSimulation();

    /** Initializes the PhysX simulation. */
    bool Init();

public:
    /**
     * Sets the default endpoint for the PhysX Visual Debugger.
     * This sets the location that the Pvd module should connect to by default
     * when it is created. To connect to the PhysX Visual Debugger over TCP to localhost,
     * pass in a null string. Note that once the simulation is initialized, changing this value
     * will have no effect.
     */
    static void SetDefaultDebuggerEndpoint(plFileName endpoint={});

    /**
     * Connects to the PhysX Visual Debugger.
     * This connects to the PhysX Visual Debugger. Caution should be used if a connection to the
     * PVD has already been made -- ensure that the previous connection is completely terminated
     * before opening a new one, otherwise the PhysX SDK may crash. If no endpoint is provided,
     * the default endpoint will be used. A null string indicates TCP/localhost.
     */
    plPXDebuggerStatus ConnectDebugger(std::optional<plFileName> endpoint=std::nullopt);

    /** Disconnects the PhysX Visual Debugger. */
    plPXDebuggerStatus DisconnectDebugger();

    bool IsDebuggerConnected() const;

protected:
    /** Creates a scene/subworld. */
    [[nodiscard]]
    World& InitSubworld(const plKey& world);

    /** Releases and, if needed, frees a scene/subworld. */
    void ReleaseSubworld(const hsKeyedObject* world);

    /** Finds or creates a PhysX Material. */
    [[nodiscard]]
    physx::PxMaterial* InitMaterial(float uStatic, float uDynamic, float restitution);

public:
    /** Cooks and inserts a convex mesh into the simulation. */
    [[nodiscard]]
    physx::PxConvexMesh* InsertConvexHull(const std::vector<uint32_t>& tris,
                                          const std::vector<hsPoint3>& verts);

    /** Cooks and inserts a triangle mesh into the simulation. */
    [[nodiscard]]
    physx::PxTriangleMesh* InsertTriangleMesh(const std::vector<uint32_t>& tris,
                                              const std::vector<hsPoint3>& verts);

    [[nodiscard]]
    physx::PxRigidActor* CreateRigidActor(const physx::PxGeometry& geometry,
                                          const physx::PxTransform& globalPose,
                                          const physx::PxTransform& localPose,
                                          float uStatic, float uDynamic, float restitution,
                                          plPXActorType type);

    /**
     * Adds a PhysX Actor to a specific subworld.
     * The actor should have been already initialized eg with \sa CreateRigidActor().
     */
    void AddToWorld(physx::PxActor* actor, const plKey& world={});

    /**
     * Creates and adds a PhysX Character Controller to a specific wubworld.
     */
    physx::PxController* CreateCharacterController(physx::PxControllerDesc& desc, const plKey& world={});

    /**
     * Finds the PhysX Scene corresponding to the requested subworld.
     * \note This does NOT create a new PhysX Scene if the world is not found.
     */
    [[nodiscard]]
    physx::PxScene* FindScene(const plKey& world);

    /**
     * Removes a PhysX Actor from a specific subworld.
     * This will destroy the actor and its shape. You are responsible for cleaning up any
     * geometry passed to the initialization function(s) such as \sa CreateRigidActor().
     */
    void RemoveFromWorld(physx::PxRigidActor* actor);

    /** Removes a PhysX Character Controller from a specific subworld.
     */
    void RemoveFromWorld(physx::PxController* controller);

    /** Advances the simulation. */
    bool Advance(float delta);
};

#endif
