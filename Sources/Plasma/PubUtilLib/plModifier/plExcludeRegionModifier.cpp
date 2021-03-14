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
#include "plExcludeRegionModifier.h"
#include "plDetectorLog.h"

#include "plPhysical.h"
#include "hsResMgr.h"

#include "pnMessage/plSDLModifierMsg.h"
#include "pnSceneObject/plCoordinateInterface.h"
#include "pnSceneObject/plSceneObject.h"
#include "pnSceneObject/plSimulationInterface.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrainGeneric.h"
#include "plMessage/plAvatarMsg.h"
#include "plMessage/plCollideMsg.h"
#include "plMessage/plExcludeRegionMsg.h"
#include "plPhysical/plSimDefs.h"
#include "plSDL/plSDL.h"

static plPhysical* GetPhysical(plSceneObject* obj)
{
    if (obj)
    {
        const plSimulationInterface* si = obj->GetSimulationInterface();
        if (si)
            return si->GetPhysical();
    }
    return nullptr;
}

plExcludeRegionModifier::plExcludeRegionModifier() :
    fSDLModifier(),
    fSeek(),
    fSeekTime(10.0f)
{
}


plExcludeRegionModifier::~plExcludeRegionModifier()
{
}

void plExcludeRegionModifier::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);

    int numPoints = stream->ReadLE32();
    for (int i = 0; i < numPoints; i++)
    {
        fSafePoints.push_back(mgr->ReadKey(stream));
    }
    fSeek = stream->ReadBool();
    fSeekTime = stream->ReadLEFloat();
}

void plExcludeRegionModifier::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);

    int numPoints = fSafePoints.size();
    stream->WriteLE32(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        mgr->WriteKey(stream,fSafePoints[i]);
    }
    stream->WriteBool(fSeek);
    stream->WriteLEFloat(fSeekTime);
}

void plExcludeRegionModifier::ISetPhysicalState(bool cleared)
{
    plPhysical* phys = GetPhysical(GetTarget());
    if (phys)
    {
        if (cleared)
        {
            phys->SetGroup(plSimDefs::kGroupExcludeRegion);
            phys->AddLOSDB(plSimDefs::kLOSDBUIBlockers);
            if (HasFlag(kBlockCameras))
                phys->AddLOSDB(plSimDefs::kLOSDBCameraBlockers);
        }
        else
        {
            phys->SetGroup(plSimDefs::kGroupDetector);
            phys->RemoveLOSDB(plSimDefs::kLOSDBUIBlockers);
            if (HasFlag(kBlockCameras))
                phys->RemoveLOSDB(plSimDefs::kLOSDBCameraBlockers);
        }
    }
}

bool plExcludeRegionModifier::MsgReceive(plMessage* msg)
{
    plExcludeRegionMsg *exclMsg = plExcludeRegionMsg::ConvertNoRef(msg);
    if (exclMsg)
    {
        if (exclMsg->GetCmd() == plExcludeRegionMsg::kClear)
        {
            plDetectorLog::Special("Clearing exclude region {}", GetKeyName());
            IMoveAvatars();
            fContainedAvatars.clear();
            ISetPhysicalState(true);
        }
        else if (exclMsg->GetCmd() == plExcludeRegionMsg::kRelease)
        {
            plDetectorLog::Special("Releasing exclude region {}", GetKeyName());
            ISetPhysicalState(false);
        }

        GetTarget()->DirtySynchState(kSDLXRegion, exclMsg->fSynchFlags);
    
        return true;
    }

    // Avatar entering or exiting our volume.  Only the owner of the SO logs this since
    // it does all the moving at clear time.
    plCollideMsg *collideMsg = plCollideMsg::ConvertNoRef(msg);
    
    if (collideMsg)
    {
        if (collideMsg->fEntering)
        {
            plDetectorLog::Special("Avatar enter exclude region {}", GetKeyName());
            fContainedAvatars.emplace_back(collideMsg->fOtherKey);
        }
        else
        {
            plDetectorLog::Special("Avatar exit exclude region {}", GetKeyName());
            auto idx = std::find(fContainedAvatars.cbegin(), fContainedAvatars.cend(),
                                 collideMsg->fOtherKey);
            if (idx != fContainedAvatars.cend())
                fContainedAvatars.erase(idx);
        }

        return true;
    }

    return plSingleModifier::MsgReceive(msg);
}

void plExcludeRegionModifier::AddTarget(plSceneObject* so)
{
    if (so)
    {
        delete fSDLModifier;
        fSDLModifier = new plExcludeRegionSDLModifier(this);
        so->AddModifier(fSDLModifier);
    }
    plSingleModifier::SetTarget(so);
}

void plExcludeRegionModifier::RemoveTarget( plSceneObject *so )
{
    if (so && fSDLModifier)
    {
        so->RemoveModifier(fSDLModifier);
        delete fSDLModifier;
        fSDLModifier = nullptr;
    }
    plSingleModifier::RemoveTarget(so);
}

void plExcludeRegionModifier::AddSafePoint(plKey& key)
{
    fSafePoints.push_back(key);
}

int plExcludeRegionModifier::IFindClosestSafePoint(plKey avatar)
{
    float closestDist = 0.f;
    int closestIdx = -1;

    plSceneObject* avObj = plSceneObject::ConvertNoRef(avatar->GetObjectPtr());
    if (!avObj)
        return -1;

    hsVector3 avPos;
    avObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&avPos);

    for (int i = 0; i < fSafePoints.size(); i++)
    {
        plSceneObject* safeObj = plSceneObject::ConvertNoRef(fSafePoints[i]->GetObjectPtr());
        hsVector3 safePos;
        safeObj->GetCoordinateInterface()->GetLocalToWorld().GetTranslate(&safePos);

        float dist = (safePos - avPos).Magnitude();

        if (dist < closestDist || closestIdx == -1)
        {
            closestDist = dist;
            closestIdx = i;
        }
    }

    return closestIdx;
}

// Move avatars out of volume
void plExcludeRegionModifier::IMoveAvatars()
{
    for (const plKey& avatarKey : fContainedAvatars) {
        plSceneObject* so = plSceneObject::ConvertNoRef(avatarKey->ObjectIsLoaded());
        auto constAvMod = (const plArmatureMod*)so->GetModifierByType(plArmatureMod::Index());
        if (constAvMod) {
            plAvBrainGeneric *curGenBrain = (plAvBrainGeneric *)constAvMod->FindBrainByClass(plAvBrainGeneric::Index());
            // *** warning; if there's more than one generic brain active, this will only look at the first
            if (curGenBrain  && curGenBrain->GetType() == plAvBrainGeneric::kLadder) {
                plAvBrainGenericMsg* pMsg = new plAvBrainGenericMsg(
                    nullptr,
                    constAvMod->GetKey(),
                    plAvBrainGenericMsg::kGotoStage,
                    -1,
                    false,
                    0.0,
                    false,
                    false,
                    0.0
                );
                pMsg->Send();
            } else {
                int closestIdx = IFindClosestSafePoint(avatarKey);

                if (closestIdx != -1) {
                    plAvSeekMsg* msg = new plAvSeekMsg;
                    msg->SetBCastFlag(plMessage::kPropagateToModifiers);
                    msg->AddReceiver(avatarKey);
                    msg->fSmartSeek = fSeek;
                    msg->fDuration = fSeekTime;
                    msg->fSeekPoint = fSafePoints[closestIdx];
                    msg->fFlags |= plAvSeekMsg::kSeekFlagUnForce3rdPersonOnFinish;
                    msg->Send();
                }
            }
        }
    }
}


///////////////////////////////////////////////////////////////////////////////

plExcludeRegionSDLModifier::plExcludeRegionSDLModifier() : fXRegion()
{
}

plExcludeRegionSDLModifier::plExcludeRegionSDLModifier(plExcludeRegionModifier* xregion) :
    fXRegion(xregion)
{
}

static const char* kXRegionSDLCleared = "cleared";

void plExcludeRegionSDLModifier::IPutCurrentStateIn(plStateDataRecord* dstState)
{
    plSimpleStateVariable* var = dstState->FindVar(kXRegionSDLCleared);
    if (var) {
        plPhysical* phys = GetPhysical(GetTarget());
        if (phys) {
            bool cleared = phys->GetGroup() == plSimDefs::kGroupExcludeRegion;
            var->Set(cleared);
        }
    }
}

void plExcludeRegionSDLModifier::ISetCurrentStateFrom(const plStateDataRecord* srcState)
{
    plSimpleStateVariable* var = srcState->FindVar(kXRegionSDLCleared);
    if (var)
    {
        bool cleared;
        var->Get(&cleared);

        plDetectorLog::Special("SDL {} exclude region {}", cleared ? "clearing" : "releasing", fXRegion->GetKeyName());
        fXRegion->ISetPhysicalState(cleared);
    }
}
