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
#include "plPXSubWorld.h"
#include "plSimulationMgr.h"

#include "hsColorRGBA.h"
#include "hsGeometry3.h"
#include "plProfile.h"
#include "hsQuat.h"
#include "hsResMgr.h"
#include "hsSTLStream.h"
#include "hsTimer.h"

#include "pnMessage/plCorrectionMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "pnMessage/plObjRefMsg.h"
#include "pnNetCommon/plNetApp.h"
#include "pnNetCommon/plSDLTypes.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plMessage/plAngularVelocityMsg.h"
#include "plMessage/plLinearVelocityMsg.h"
#include "plMessage/plSimStateMsg.h"
#include "plPhysical/plPhysicalSDLModifier.h"
#include "plPhysical/plPhysicalSndGroup.h"
#include "plPhysical/plPhysicalProxy.h"

// ==========================================================================

plProfile_Extern(MaySendLocation);
plProfile_Extern(LocationsSent);
plProfile_Extern(PhysicsUpdates);
plProfile_Extern(SetTransforms);

constexpr float kMaxNegativeZPos = -2000.f;

// ==========================================================================

static void ClearMatrix(hsMatrix44& m)
{
    m.fMap[0][0] = 0.0f; m.fMap[0][1] = 0.0f; m.fMap[0][2] = 0.0f; m.fMap[0][3]  = 0.0f;
    m.fMap[1][0] = 0.0f; m.fMap[1][1] = 0.0f; m.fMap[1][2] = 0.0f; m.fMap[1][3]  = 0.0f;
    m.fMap[2][0] = 0.0f; m.fMap[2][1] = 0.0f; m.fMap[2][2] = 0.0f; m.fMap[2][3]  = 0.0f;
    m.fMap[3][0] = 0.0f; m.fMap[3][1] = 0.0f; m.fMap[3][2] = 0.0f; m.fMap[3][3]  = 0.0f;
    m.NotIdentity();
}

// ==========================================================================

int plPXPhysical::fNumberAnimatedPhysicals = 0;
int plPXPhysical::fNumberAnimatedActivators = 0;

// ==========================================================================

PhysRecipe::PhysRecipe()
    : mass(), friction(), restitution(), bounds(plSimDefs::kBoundsMax),
      group(plSimDefs::kGroupMax), reportsOn(), convexMesh(), triMesh(),
      radius(), offset()
{
}

PhysRecipe::~PhysRecipe()
{
}

plPXPhysical::plPXPhysical()
    : fActor(), fGroup(plSimDefs::kGroupMax), fReportsOn(), fLOSDBs(plSimDefs::kLOSDBNone),
      fLastSyncTime(), fSDLMod(), fSndGroup()
{
}

plPXPhysical::~plPXPhysical()
{
    DestroyActor();

    // remove sdl modifier
    plSceneObject* sceneObj = plSceneObject::ConvertNoRef(fObjectKey->ObjectIsLoaded());
    if (sceneObj && fSDLMod)
        sceneObj->RemoveModifier(fSDLMod);
    delete fSDLMod;
}

// ==========================================================================

bool plPXPhysical::MsgReceive( plMessage* msg )
{
    if (plGenRefMsg *refM = plGenRefMsg::ConvertNoRef(msg)) {
        return HandleRefMsg(refM);
    } else if (plSimulationMsg *simM = plSimulationMsg::ConvertNoRef(msg)) {
        plLinearVelocityMsg* velMsg = plLinearVelocityMsg::ConvertNoRef(msg);
        if (velMsg) {
            SetLinearVelocitySim(velMsg->Velocity());
            return true;
        }

        plAngularVelocityMsg* angvelMsg = plAngularVelocityMsg::ConvertNoRef(msg);
        if (angvelMsg) {
            SetAngularVelocitySim(angvelMsg->AngularVelocity());
            return true;
        }
        return false;
    }
    return plPhysical::MsgReceive(msg);
}

bool plPXPhysical::HandleRefMsg(plGenRefMsg* refMsg)
{
    uint8_t refCtxt = refMsg->GetContext();

    switch (refMsg->fType) {
    case kPhysRefWorld:
        switch (refCtxt) {
        case plRefMsg::kOnCreate:
        case plRefMsg::kOnRequest:
            // PotS files specify a plHKSubWorld as the subworld key. For everything else,
            // the subworlds are a plSceneObject key. For sanity purposes, we will allow
            // references to the plPXSubWorld here. HOWEVER, we will need to grab the target
            // and replace our reference...
            if (plPXSubWorld* subWorldIface = plPXSubWorld::ConvertNoRef(refMsg->GetRef())) {
                hsAssert(subWorldIface->GetOwnerKey(), "subworld owner is NULL?! Uh oh...");
                plGenRefMsg* replaceRefMsg = new plGenRefMsg(GetKey(), plRefMsg::kOnReplace, 0, kPhysRefWorld);
                hsgResMgr::ResMgr()->AddViaNotify(subWorldIface->GetOwnerKey(), replaceRefMsg, plRefFlags::kActiveRef);
            }

        // fall-thru is intentional :)
        case plRefMsg::kOnReplace:
            // loading into a subworld
            if (plSceneObject* subSO = plSceneObject::ConvertNoRef(refMsg->GetRef())) {
                fWorldKey = subSO->GetKey();

                // Cyan produced files will never have plPXSubWorld as this is a H'uru-ism.
                // Let us make a default one such that we can play with default subworlds at runtime.
                // HAAAAAAAAAX!!!
                plPXSubWorld* subWorldIface = plPXSubWorld::ConvertNoRef(subSO->GetGenericInterface(plPXSubWorld::Index()));
                if (!subWorldIface) {
                    subWorldIface = new plPXSubWorld();
                    hsgResMgr::ResMgr()->NewKey(ST::format("{}_DefSubWorld", subSO->GetKeyName()),
                                                subWorldIface, GetKey()->GetUoid().GetLocation());
                    plObjRefMsg* subIfaceRef = new plObjRefMsg(subSO->GetKey(), plRefMsg::kOnCreate, 0, plObjRefMsg::kInterface);
                    // this will send the reference immediately
                    hsgResMgr::ResMgr()->SendRef(subWorldIface, subIfaceRef, plRefFlags::kActiveRef);
                }

                // Now, we can initialize the physical...
                InitActor();
                IGetTransformGlobal(fCachedLocal2World);
                return true;
            }
            break;

        case plRefMsg::kOnDestroy:
        case plRefMsg::kOnRemove:
            fWorldKey = nullptr;
            IUpdateSubworld();
            break;
        }
        break;

    case kPhysRefSndGroup:
        switch (refCtxt) {
        case plRefMsg::kOnCreate:
        case plRefMsg::kOnRequest:
            fSndGroup = plPhysicalSndGroup::ConvertNoRef(refMsg->GetRef());
            break;

        case plRefMsg::kOnDestroy:
            fSndGroup = nullptr;
            break;
        }
        break;

    default:
        hsAssert(0, "Unknown ref type, who sent us this?");
        break;
    }

    return true;
}

// ==========================================================================

void plPXPhysical::GetTransform(hsMatrix44& l2w, hsMatrix44& w2l)
{
    IGetTransformGlobal(l2w);
    l2w.GetInverse(&w2l);
}

void plPXPhysical::SetSceneNode(plKey newNode)
{
    plKey oldNode = GetSceneNode();
    if (oldNode == newNode)
        return;

    // If we don't do this, we get leaked keys and a crash on exit with certain clones
    // Note this has nothing do to with the world that the physical is in
    if (newNode) {
        plNodeRefMsg* refMsg = new plNodeRefMsg(newNode, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kPhysical);
        hsgResMgr::ResMgr()->SendRef(GetKey(), refMsg, plRefFlags::kActiveRef);
    }
    if (oldNode)
        oldNode->Release(GetKey());
}

// ==========================================================================

void plPXPhysical::InitProxy()
{
    hsColorRGBA physColor;
    float opac = 1.0f;

    if (fGroup == plSimDefs::kGroupAvatar) {
        // local avatar is light purple and transparent
        physColor.Set(.2f, .1f, .2f, 1.f);
        opac = 0.4f;
    } else if (fGroup == plSimDefs::kGroupDynamic) {
        // Dynamics are red
        physColor.Set(1.f,0.f,0.f,1.f);
    } else if (fGroup == plSimDefs::kGroupDetector) {
        if (!fRecipe.worldKey) {
            // Detectors are blue, and transparent
            physColor.Set(0.f,0.f,1.f,1.f);
            opac = 0.3f;
        } else {
            // subworld Detectors are green
            physColor.Set(0.f,1.f,0.f,1.f);
            opac = 0.3f;
        }
    } else if (fGroup == plSimDefs::kGroupStatic) {
        if (GetProperty(plSimulationInterface::kPhysAnim)) {
            // Statics that are animated are more reddish?
            physColor.Set(1.f, 0.6f, 0.2f, 1.f);
        } else {
            // Statics are yellow
            physColor.Set(1.f, 0.8f, 0.2f, 1.f);
        }

        // if in a subworld... slightly transparent
        if (fRecipe.worldKey)
            opac = 0.6f;
    } else if (fGroup == plSimDefs::kGroupExcludeRegion) {
        physColor.Set(.96f, .02f, .99f, 1.f);
        opac = .6f;
    } else {
        // don't knows are grey
        physColor.Set(0.6f,0.6f,0.6f,1.f);
    }

    fProxyGen = std::make_unique<plPhysicalProxy>(hsColorRGBA().Set(0,0,0,1.f), physColor, opac);
    fProxyGen->Init(this);
}

void plPXPhysical::IMoveProxy(const hsMatrix44& l2w)
{
    if (fProxyGen) {
        hsMatrix44 w2l;
        l2w.GetInverse(&w2l);
        fProxyGen->SetTransform(l2w, w2l);
    }
}

void plPXPhysical::InitSDL()
{
    // add SDL modifier
    plSceneObject* sceneObj = plSceneObject::ConvertNoRef(fObjectKey->ObjectIsLoaded());
    hsAssert(sceneObj, "nil sceneObject, failed to create and attach SDL modifier");

    delete fSDLMod;
    fSDLMod = new plPhysicalSDLModifier;
    sceneObj->AddModifier(fSDLMod);
}

void plPXPhysical::InitRefs() const
{
    plNodeRefMsg* nodeRefMsg = new plNodeRefMsg(fSceneNode, plRefMsg::kOnCreate, -1, plNodeRefMsg::kPhysical); 
    hsgResMgr::ResMgr()->AddViaNotify(GetKey(), nodeRefMsg, plRefFlags::kActiveRef);

    plGenRefMsg* simRefMsg = new plGenRefMsg(plSimulationMgr::GetInstance()->GetKey(),
                                             plRefMsg::kOnCreate, -1, plSimulationMgr::kPhysical);
    hsgResMgr::ResMgr()->AddViaNotify(GetKey(), simRefMsg, plRefFlags::kPassiveRef);
}

// ==========================================================================

void plPXPhysical::Read(hsStream* stream, hsResMgr* mgr)
{
    plPhysical::Read(stream, mgr);
    ClearMatrix(fCachedLocal2World);

    fRecipe.mass = stream->ReadLEFloat();
    fRecipe.friction = stream->ReadLEFloat();
    fRecipe.restitution = stream->ReadLEFloat();
    fRecipe.bounds = (plSimDefs::Bounds)stream->ReadByte();
    fRecipe.group = (plSimDefs::Group)stream->ReadByte();
    fRecipe.reportsOn = stream->ReadLE32();
    fLOSDBs = stream->ReadLE16();
    // hack for swim regions currently they are labeled as static av blockers
    if (fLOSDBs==plSimDefs::kLOSDBSwimRegion) {
        fRecipe.group=plSimDefs::kGroupMax;
    }

    fRecipe.objectKey = mgr->ReadKey(stream);
    fRecipe.sceneNode = mgr->ReadKey(stream);
    fRecipe.worldKey = mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefWorld), plRefFlags::kActiveRef);
    mgr->ReadKeyNotifyMe(stream, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kPhysRefSndGroup), plRefFlags::kActiveRef);

    fRecipe.l2sP.Read(stream);
    fRecipe.l2sQ.Read(stream);
    DirtyRecipe();

    fProps.Read(stream);

    // Note that fBounds may not be the same as fRecipe.bounds due to limitations in whatever physics
    // engine is being used. The readers will compensate for the change by testing fRecipe.bounds.
    if (fBounds == plSimDefs::kSphereBounds) {
        fRecipe.radius = stream->ReadLEFloat();
        fRecipe.offset.Read(stream);
    } else if (fBounds == plSimDefs::kBoxBounds) {
        fRecipe.bDimensions.Read(stream);
        fRecipe.bOffset.Read(stream);
    } else if (fBounds == plSimDefs::kHullBounds) {
        fRecipe.convexMesh = ICookHull(stream);
    } else {
        fRecipe.triMesh = ICookTriMesh(stream);
    }

    // If we do not have a world specified, we go ahead and init into the main world...
    // This will been done in MsgReceive otherwise
    if (!fRecipe.worldKey)
        InitActor();
    InitProxy();
}

void plPXPhysical::Write(hsStream* stream, hsResMgr* mgr)
{
    plPhysical::Write(stream, mgr);

    stream->WriteLEFloat(fRecipe.mass);
    stream->WriteLEFloat(fRecipe.friction);
    stream->WriteLEFloat(fRecipe.restitution);
    stream->WriteByte((uint8_t)fRecipe.bounds);
    stream->WriteByte((uint8_t)fGroup);
    stream->WriteLE32(fReportsOn);
    stream->WriteLE16(fLOSDBs);
    mgr->WriteKey(stream, fObjectKey);
    mgr->WriteKey(stream, fSceneNode);
    mgr->WriteKey(stream, fWorldKey);
    mgr->WriteKey(stream, fSndGroup);

    fRecipe.l2sP.Write(stream);
    fRecipe.l2sQ.Write(stream);

    fProps.Write(stream);

    if (fRecipe.bounds == plSimDefs::kSphereBounds) {
        stream->WriteLEFloat(fRecipe.radius);
        fRecipe.offset.Write(stream);
    } else if (fRecipe.bounds == plSimDefs::kBoxBounds) {
        fRecipe.bDimensions.Write(stream);
        fRecipe.bOffset.Write(stream);
    } else {
        // We hide the stream we used to create this mesh away in the shape user data.
        // Pull it out and write it to disk.
        hsAssert(fRecipe.meshStream, "meshStream is null?");
        stream->Write(fRecipe.meshStream->GetEOF(), fRecipe.meshStream->GetData());
    }
}

// ==========================================================================

bool plPXPhysical::DirtySynchState(const ST::string& SDLStateName, uint32_t synchFlags)
{
    if (GetObjectKey()) {
        plSynchedObject* so = plSynchedObject::ConvertNoRef(GetObjectKey()->ObjectIsLoaded());
        if (so) {
            fLastSyncTime = hsTimer::GetSysSeconds();
            return so->DirtySynchState(SDLStateName, synchFlags);
        }
    }

    return false;
}

void plPXPhysical::GetSyncState(hsPoint3& pos, hsQuat& rot, hsVector3& linV, hsVector3& angV)
{
    IGetPoseSim(pos, rot);
    GetLinearVelocitySim(linV);
    GetAngularVelocitySim(angV);
}

void plPXPhysical::SetSyncState(hsPoint3* pos, hsQuat* rot, hsVector3* linV, hsVector3* angV)
{
    bool isLoading = plNetClientApp::GetInstance()->IsLoadingInitialAgeState();
    bool isFirstIn = plNetClientApp::GetInstance()->GetJoinOrder() == 0;
    bool initialSync = isLoading && isFirstIn;
    bool wakeup = !(initialSync && GetProperty(plSimulationInterface::kStartInactive));

    // If the physical has fallen out of the sim, and this is initial age state, and we're
    // the first person in, reset it to the original position.  (ie, prop the default state
    // we've got right now)
    if (pos && pos->fZ < kMaxNegativeZPos && initialSync) {
        SimLog("Physical {} loaded out of range state.  Forcing initial state to server.", GetKeyName());
        DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
        return;
    }

    ISetPoseSim(pos, rot, wakeup);
    if (linV)
        SetLinearVelocitySim(*linV, wakeup);
    if (angV)
        SetAngularVelocitySim(*angV, wakeup);

    SendNewLocation(false, true);
}

void plPXPhysical::ResetSyncState()
{
    if (fSDLMod) {
        hsVector3 zero;
        bool wakeup = GetProperty(plSimulationInterface::kStartInactive);

        ISetPoseSim(&fRecipe.l2sP, &fRecipe.l2sQ, wakeup);
        SetLinearVelocitySim(zero, wakeup);
        SetAngularVelocitySim(zero);
        SendNewLocation(true, true);
        DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients |
                                      plSynchedObject::kSkipLocalOwnershipCheck |
                                      plSynchedObject::kSendImmediately);
    }
}

// ==========================================================================

void plPXPhysical::SendNewLocation(bool synchTransform, bool isSynchUpdate)
{
    // Called after the simulation has run....sends new positions to the various scene objects
    // *** want to do this in response to an update message....
    if (CanSynchPosition(isSynchUpdate)) {
        plProfile_Inc(MaySendLocation);

        if (!GetProperty(plSimulationInterface::kPassive)) {
            hsMatrix44 curl2w = fCachedLocal2World;
            // we're going to cache the transform before sending so we can recognize if it comes back
            IGetTransformGlobal(fCachedLocal2World);

            if (!curl2w.Compare(fCachedLocal2World, .0001f)) {
                plProfile_Inc(LocationsSent);
                plProfile_BeginLap(PhysicsUpdates, GetKeyName().c_str());

                if (fCachedLocal2World.GetTranslate().fZ < kMaxNegativeZPos)
                {
                    SimLog("Physical {} fell to {.1f} ({.1f} is the max).  Suppressing.", GetKeyName(), fCachedLocal2World.GetTranslate().fZ, kMaxNegativeZPos);
                    // Since this has probably been falling for a while, and thus not getting any syncs,
                    // make sure to save its current pos so we'll know to reset it later
                    DirtySynchState(kSDLPhysical, plSynchedObject::kBCastToClients);
                    IEnable(false);
                }

                hsMatrix44 w2l;
                fCachedLocal2World.GetInverse(&w2l);
                plCorrectionMsg *pCorrMsg = new plCorrectionMsg(GetObjectKey(), fCachedLocal2World, w2l, synchTransform);
                pCorrMsg->Send();
                if (fProxyGen)
                    fProxyGen->SetTransform(fCachedLocal2World, w2l);
                plProfile_EndLap(PhysicsUpdates, GetKeyName().c_str());
            }
        }
    }
}

void plPXPhysical::SetTransform(const hsMatrix44& l2w, const hsMatrix44& w2l, bool force)
{
    //  make sure there is some difference between the matrices...
    // ... but not when a subworld... because the subworld may be animating and if the object is still then it is actually moving within the subworld
    if (force || (!IsStatic() && (fWorldKey || !l2w.Compare(fCachedLocal2World, .0001f)))) {
        ISetTransformGlobal(l2w);
        plProfile_Inc(SetTransforms);
    }
}
