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
#ifndef plSimulationMgr_H
#define plSimulationMgr_H

#include <map>
#include "pnKeyedObject/hsKeyedObject.h"
#include "plStatusLog/plStatusLog.h"
#include "hsTemplates.h"

class plPXPhysical;
class plLOSDispatch;
class plStatusLog;
class plPhysicsSoundMgr;
class NxPhysicsSDK;
class NxScene;
class plCollideMsg;
struct hsPoint3;

class plSimulationMgr : public hsKeyedObject
{
public:
    CLASSNAME_REGISTER(plSimulationMgr);
    GETINTERFACE_ANY(plSimulationMgr, hsKeyedObject);

    static plSimulationMgr* GetInstance();
    static void Init();
    static void Shutdown();

    static bool fExtraProfile;
    static bool fSubworldOptimization;
    static bool fDoClampingOnStep;

    // initialiation of the PhysX simulation
    virtual bool InitSimulation();

    // Advance the simulation by the given number of seconds
    void Advance(float delSecs);

    // The simulation won't run at all if it is suspended
    void Suspend() { fSuspended = true; }
    void Resume() { fSuspended = false; }
    bool IsSuspended() { return fSuspended; }

    static void ClearLog();

    // We've detected a collision, which may be grounds for synchronizing the involved
    // physicals over the network.
    void ConsiderSynch(plPXPhysical *physical, plPXPhysical *other);

    NxPhysicsSDK* GetSDK() const { return fSDK; }
    NxScene* GetScene(plKey world);
    // Called when an actor is removed from a scene, checks if it's time to delete
    // the scene
    void ReleaseScene(plKey world);

    int GetMaterialIdx(NxScene* scene, float friction, float restitution);

    uint32_t GetStepCount() const { return fStepCount; }

    //Fix to Move collision messages and their handling out of the simulation step
    void AddCollisionMsg(plKey hitee, plKey hitter, bool entering);
    void AddCollisionMsg(plCollideMsg* msg);

#ifndef PLASMA_EXTERNAL_RELEASE
    static bool fDisplayAwakeActors;
#endif //PLASMA_EXTERNAL_RELEASE

protected:
    friend class ContactReport;

    void ISendUpdates();

    plSimulationMgr();
    virtual ~plSimulationMgr();

    // Walk through the synchronization requests and send them as appropriate.
    void IProcessSynchs();

    NxPhysicsSDK* fSDK;

    plPhysicsSoundMgr* fSoundMgr;

    // Pending collision messages
    typedef std::vector<plCollideMsg*> CollisionVec;
    CollisionVec fCollideMsgs;

    // A mapping from a key to a PhysX scene.  The key is either the
    // SimulationMgr key, for the main world, or a SceneObject key if it's a
    // subworld.
    typedef std::map<plKey, NxScene*> SceneMap;
    SceneMap fScenes;

    plLOSDispatch* fLOSDispatch;

    // Is the entire physics world suspended? If so, the clock can still advance
    // but nothing will move.
    bool fSuspended;

    float fAccumulator;
    uint32_t fStepCount;

    // A utility class to keep track of a request for a physical synchronization.
    // These requests must pass a certain criteria (see the code for the latest)
    // before they are actually either sent over the network or rejected.
    class SynchRequest
    {
    public:
        double fTime;   // when to synch
        plKey fKey;     // key of the object to be synched, so we can make sure it still lives

        static const double kDefaultTime;
        SynchRequest() : fTime(kDefaultTime) {};
    };

    // All currently pending synch requests. Keyed by the physical in question
    // so we can quickly eliminate redundant requests, which are very common.
    typedef std::map<plPXPhysical*, SynchRequest> PhysSynchMap;
    PhysSynchMap fPendingSynchs;

    plStatusLog *fLog;
#ifndef PLASMA_EXTERNAL_RELEASE
    void IDrawActiveActorList();
#endif //PLASMA_EXTERNAL_RELEASE

public:
    // Output the given debug text to the simulation log.
    template<typename... _Args>
    static void Log(const char* formatStr, _Args&&... args)
    {
        if (GetInstance() && GetInstance()->fLog) {
            GetInstance()->fLog->AddLine(formatStr, std::forward<_Args>(args)...);
        }
    }
};

#define SIM_VERBOSE

#ifdef SIM_VERBOSE
template<typename... _Args>
inline void SimLog(const char* format, _Args&&... args)
{
    plSimulationMgr::Log(format, std::forward<_Args>(args)...);
}

#else
template<typename... _Args>
inline void SimLog(const char *str, _Args&&... args)
{
    // will get stripped out
}
#endif

#endif
