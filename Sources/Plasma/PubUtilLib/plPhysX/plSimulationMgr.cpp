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
#include "plSimulationMgr.h"

#include <algorithm>

#include "plgDispatch.h"
#include "hsTimer.h"
#include "plProfile.h"

#include "plLOSDispatch.h"
#include "plPXConvert.h"
#include "plPXPhysical.h"
#include "plPXPhysicalControllerCore.h"
#include "plPXSimulation.h"
#include "plPXSubWorld.h"

#include "pnMessage/plRefMsg.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plSimulationInterface.h"
#include "pnSceneObject/plCoordinateInterface.h"

#include "plMessage/plCollideMsg.h"
#include "plMessage/plAgeLoadedMsg.h"
#include "plModifier/plDetectorLog.h"
#include "plModifier/plExcludeRegionModifier.h"
#include "plPhysical/plPhysicsSoundMgr.h"
#include "plStatusLog/plStatusLog.h"

/////////////////////////////////////////////////////////////////
//
// CONSTRUCTION, INITIALIZATION, DESTRUCTION
//
/////////////////////////////////////////////////////////////////

plProfile_CreateTimer(  "ProcessSyncs", "Simulation", ProcessSyncs);
plProfile_CreateTimer(  "UpdateContexts", "Simulation", UpdateContexts);
plProfile_CreateCounter("  MaySendLocation", "Simulation", MaySendLocation);
plProfile_CreateCounter("  LocationsSent", "Simulation", LocationsSent);
plProfile_CreateTimer(  "  PhysicsUpdates","Simulation", PhysicsUpdates);
plProfile_CreateCounter("SetTransforms Accepted", "Simulation", SetTransforms);

// declared at file scope so that both GetInstance and the destructor can access it.
static plSimulationMgr* gTheInstance;
bool plSimulationMgr::fExtraProfile = false;

void plSimulationMgr::Init()
{
    hsAssert(!gTheInstance, "Initializing the sim when it's already been done");
    gTheInstance = new plSimulationMgr;
    if (gTheInstance->fSimulation->Init())
        gTheInstance->RegisterAs(kSimulationMgr_KEY);
    else
        gTheInstance = nullptr;
}

// when the app is going away completely
void plSimulationMgr::Shutdown()
{
    hsAssert(gTheInstance, "Simulation manager missing during shutdown.");
    if (gTheInstance)
        gTheInstance->UnRegisterAs(kSimulationMgr_KEY);     // this will destroy the instance
}

plSimulationMgr* plSimulationMgr::GetInstance()
{
    return gTheInstance;
}

//////////////////////////////////////////////////////////////////////////

plSimulationMgr::plSimulationMgr()
    : fSimulation(std::make_unique<plPXSimulation>()),
      fSuspended(true),
      fLOSDispatch(new plLOSDispatch()),
      fSoundMgr(new plPhysicsSoundMgr),
      fLog()
{
    fLog = plStatusLogMgr::GetInstance().CreateStatusLog(40, "Simulation.log", plStatusLog::kFilledBackground | plStatusLog::kAlignToTop);
}

plSimulationMgr::~plSimulationMgr()
{
    fLOSDispatch->UnRef();
    fLOSDispatch = nullptr;

    delete fSoundMgr;
    fSoundMgr = nullptr;

    delete fLog;
    fLog = nullptr;
}

//////////////////////////////////////////////////////////////////////////

bool plSimulationMgr::MsgReceive(plMessage* msg)
{
    if (plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg)) {
        switch (refMsg->fType) {
        case kPhysical:
            switch (refMsg->GetContext()) {
            case plRefMsg::kOnCreate:
            case plRefMsg::kOnRequest:
                fPhysicals.push_back(plPXPhysical::ConvertNoRef(refMsg->GetRef()));
                break;

            case plRefMsg::kOnDestroy:
            case plRefMsg::kOnRemove:
                {
                    auto it = std::find(fPhysicals.begin(), fPhysicals.end(), refMsg->GetRef());
                    if (it != fPhysicals.end())
                        fPhysicals.erase(it);
                }
                break;
            }
            return true;

        DEFAULT_FATAL(refMsg->fType);
        }
    }

    return hsKeyedObject::MsgReceive(msg);
}

//////////////////////////////////////////////////////////////////////////

void plSimulationMgr::AddCollisionMsg(plKey hitee, plKey hitter, bool enter)
{
    // First, make sure we have no dupes
    for (CollisionVec::iterator it = fCollideMsgs.begin(); it != fCollideMsgs.end(); ++it)
    {
        plCollideMsg* pMsg = *it;

        // Should only ever be one receiver.
        // Oh, it seems we should update the hit status. The latest might be different than the older...
        // Even in the same frame >.<
        if (pMsg->fOtherKey == hitter && pMsg->GetReceiver(0) == hitee)
        {
            pMsg->fEntering = enter;
            plDetectorLog::Red("DUPLICATE COLLISION: {} hit {}",
                (hitter ? hitter->GetName() : ST_LITERAL("(nil)")),
                (hitee ? hitee->GetName() : ST_LITERAL("(nil)")));
            return;
        }
    }

    // Still here? Then this must be a unique hit!
    plCollideMsg* pMsg = new plCollideMsg;
    pMsg->AddReceiver(std::move(hitee));
    pMsg->fOtherKey = std::move(hitter);
    pMsg->fEntering = enter;
    fCollideMsgs.push_back(pMsg);
}
void plSimulationMgr::AddCollisionMsg(plCollideMsg* msg)
{
    fCollideMsgs.push_back(msg);
}

void plSimulationMgr::AddContactSound(plPhysical* phys1, plPhysical* phys2,
                                      const hsPoint3& pos, const hsVector3& normal)
{
    fSoundMgr->AddContact(phys1, phys2, pos, normal);
}

void plSimulationMgr::ResetKickables()
{
    for (auto i : fPhysicals) {
        if (i->IsDynamic())
            i->ResetSyncState();
    }
}

void plSimulationMgr::Advance(float delSecs)
{
    if (fSuspended)
        return;

    // Only pump the sounds if the simulation actually advanced. Otherwise we get fascinating
    // (read: bad) sounds stopping/starting when the fps is greater than the simulation frequency.
    if (fSimulation->Advance(delSecs))
        fSoundMgr->Update();

    plProfile_BeginTiming(ProcessSyncs);
    IProcessSynchs();
    plProfile_EndTiming(ProcessSyncs);

    plProfile_BeginTiming(UpdateContexts);
    ISendUpdates();
    plProfile_EndTiming(UpdateContexts);
}

void plSimulationMgr::ISendUpdates()
{
    for (auto pMsg : fCollideMsgs) {
        plDetectorLog::Yellow("Collision: {} was triggered by {}. Sending an {} msg",
                              pMsg->GetReceiver(0)->GetName(),
                              pMsg->fOtherKey ? pMsg->fOtherKey->GetName() : ST_LITERAL("(nil)"),
                              pMsg->fEntering ? "'enter'" : "'exit'");
        plgDispatch::Dispatch()->MsgSend(pMsg);
    }
    fCollideMsgs.clear();

    for (auto physical : fPhysicals) {
        if (physical->GetSceneNode())
            physical->SendNewLocation();
    }
}


/////////////////////////////////////////////////////////////////
//
//  SYNCHRONIZATION
//  Very much a work in progress.
//  *** would like to synchronize interacting groups as an atomic unit
//  *** need a "morphing synch" that incrementally approaches the target
//
/////////////////////////////////////////////////////////////////

const double plSimulationMgr::SynchRequest::kDefaultTime = -1000.0;

void plSimulationMgr::ConsiderSynch(plPXPhysical* physical, plPXPhysical* other)
{
    if ((!physical || physical->GetProperty(plSimulationInterface::kNoSynchronize)) &&
        (!other || other->GetProperty(plSimulationInterface::kNoSynchronize)))
        return;

    // We only need to sync if a dynamic is colliding with something.
    // Set it up so the dynamic is in 'physical'
    if (other && other->GetGroup() == plSimDefs::kGroupDynamic)
    {
        plPXPhysical* temp = physical;
        physical = other;
        other = temp;
    }
    // Neither is dynamic, so we can exit now
    else if (physical->GetGroup() != plSimDefs::kGroupDynamic)
        return;

    bool syncPhys = !physical->GetProperty(plSimulationInterface::kNoSynchronize) &&
                    physical->IsDynamic() &&
                    physical->IsLocallyOwned();
    bool syncOther = other &&
                    !other->GetProperty(plSimulationInterface::kNoSynchronize) &&
                    other->IsDynamic() != 0.f &&
                    other->IsLocallyOwned();

    if (syncPhys)
    {
        double timeNow = hsTimer::GetSysSeconds();
        double timeElapsed = timeNow - physical->GetLastSyncTime();

        // If both objects are capable of syncing, we want to do it at the same
        // time, so no interpenetration issues pop up on other clients
        if (syncOther)
            timeElapsed = std::max(timeElapsed, timeNow - other->GetLastSyncTime());

        // Set the sync time to 1 second from the last sync
        double syncTime = 0.0;
        if (timeElapsed > 1.0)
            syncTime = hsTimer::GetSysSeconds();
        else
            syncTime = hsTimer::GetSysSeconds() + (1.0 - timeElapsed);

        // This line will create and insert the request if it's not there already.
        SynchRequest& physReq = fPendingSynchs[physical];
        if (physReq.fTime == SynchRequest::kDefaultTime)
            physReq.fKey = physical->GetKey();
        physReq.fTime = syncTime;

        if (syncOther)
        {
            SynchRequest& otherReq = fPendingSynchs[other];
            if (otherReq.fTime == SynchRequest::kDefaultTime)
                otherReq.fKey = other->GetKey();
            otherReq.fTime = syncTime;
        }
    }
}

void plSimulationMgr::IProcessSynchs()
{
    double time = hsTimer::GetSysSeconds();

    PhysSynchMap::iterator i = fPendingSynchs.begin();

    while (i != fPendingSynchs.end())
    {
        SynchRequest req = (*i).second;
        if (req.fKey->ObjectIsLoaded())
        {
            plPXPhysical* phys = (*i).first;
            bool timesUp = (time >= req.fTime);
            bool allQuiet = false;//phys->GetActo GetBody()->isActive() == false;

            if (timesUp || allQuiet)
            {
                phys->DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
                i = fPendingSynchs.erase(i);
            }
            else
            {
                i++;
            }
        }
        else
        {
            i = fPendingSynchs.erase(i);
        }
    }
}

void plSimulationMgr::ClearLog()
{
    if(gTheInstance)
    {
        plStatusLog *log = GetInstance()->fLog;
        if(log)
        {
            log->Clear();
        }
    }
}
