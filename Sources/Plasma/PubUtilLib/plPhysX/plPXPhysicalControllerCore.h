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
#include "plAvatar/plPhysicalControllerCore.h"

class NxCapsuleController;
class NxActor;
class NxCapsule;
class PXControllerHitReport;
class plPhysicalProxy;
class plDrawableSpans;
class hsGMaterial;
class plSceneObject;
class plPXPhysical;
class plCollideMsg;

#ifndef PLASMA_EXTERNAL_RELEASE
class plDbgCollisionInfo
{
public:
    plSceneObject *fSO;
    hsVector3 fNormal;
    bool fOverlap;
};
#endif // PLASMA_EXTERNAL_RELEASE

class plPXPhysicalControllerCore: public plPhysicalControllerCore
{
public:
    plPXPhysicalControllerCore(plKey ownerSO, float height, float radius, bool human);
    ~plPXPhysicalControllerCore();

    // An ArmatureMod has its own idea about when physics should be enabled/disabled.
    // Use plArmatureModBase::EnablePhysics() instead.
    virtual void Enable(bool enable);

    // Subworld
    virtual void SetSubworld(plKey world);

    // For the avatar SDL only
    virtual void GetState(hsPoint3& pos, float& zRot);
    virtual void SetState(const hsPoint3& pos, float zRot);

    // Movement strategy
    virtual void SetMovementStrategy(plMovementStrategy* strategy);

    // Global location
    virtual void SetGlobalLoc(const hsMatrix44& l2w);

    // Local Sim Position
    virtual void GetPositionSim(hsPoint3& pos);

    // Move kinematic controller
    virtual void Move(hsVector3 displacement, unsigned int collideWith, unsigned int &collisionResults);

    // Set linear velocity on dynamic controller
    virtual void SetLinearVelocitySim(const hsVector3& linearVel);

    // Sweep the controller path from startPos through endPos
    virtual int SweepControllerPath(const hsPoint3& startPos, const hsPoint3& endPos, bool vsDynamics,
        bool vsStatics, uint32_t& vsSimGroups, std::vector<plControllerSweepRecord>& hits);

    // any clean up for the controller should go here
    virtual void LeaveAge();

    // Capsule
    void GetWorldSpaceCapsule(NxCapsule& cap) const;

    // Create Proxy for debug rendering
    plDrawableSpans* CreateProxy(hsGMaterial* mat, hsTArray<uint32_t>& idx, plDrawableSpans* addTo);

    // Dynamic hits
    void AddDynamicHit(plPXPhysical* phys);

//////////////////////////////////////////
//Static Helper Functions
////////////////////////////////////////

    // Call pre-sim to apply movement to controllers
    static void Apply(float delSecs);

    // Call post-sim to update controllers
    static void Update(int numSubSteps, float alpha);

    // Update controllers when not performing a physics step
    static void UpdateNonPhysical(float alpha);

    // Rebuild the controller cache, required when a static actor in the scene has changed.
    static void RebuildCache();

    // Returns the plPXPhysicalControllerCore associated with the given NxActor
    static plPXPhysicalControllerCore* GetController(NxActor& actor);

    // Subworld controller queries
    static bool AnyControllersInThisWorld(plKey world);
    static int GetNumberOfControllersInThisSubWorld(plKey world);
    static int GetControllersInThisSubWorld(plKey world, int maxToReturn, plPXPhysicalControllerCore** bufferout);

    // Controller count
    static int NumControllers();
    static void SetMaxNumberOfControllers(int max) { fPXControllersMax = max; }
    static int fPXControllersMax;

#ifndef PLASMA_EXTERNAL_RELEASE
    static bool fDebugDisplay;
#endif

protected:
    friend class PXControllerHitReport;

    virtual void IHandleEnableChanged();

    void IInformDetectors(bool entering);

    void ICreateController(const hsPoint3& pos);
    void IDeleteController();

    void IDispatchQueuedMsgs();
    void IProcessDynamicHits();

#ifndef PLASMA_EXTERNAL_RELEASE
    void IDrawDebugDisplay(int controllerIdx);
    hsTArray<plDbgCollisionInfo> fDbgCollisionInfo;
#endif

    std::vector<plCollideMsg*> fQueuedCollideMsgs;
    std::vector<plPXPhysical*> fDynamicHits;

    NxCapsuleController* fController;
    NxActor* fActor;

    plPhysicalProxy* fProxyGen;
    bool fKinematicCCT;
    bool fHuman;
};
