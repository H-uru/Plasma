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

#include "plObjectInBoxConditionalObject.h"

#include "plgDispatch.h"

#include "pnMessage/plFakeOutMsg.h"
#include "pnMessage/plNotifyMsg.h"
#include "pnMessage/plPlayerPageMsg.h"
#include "pnModifier/plLogicModBase.h"
#include "pnNetCommon/plNetApp.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAvatar/plArmatureMod.h"
#include "plMessage/plActivatorMsg.h"

plObjectInBoxConditionalObject::plObjectInBoxConditionalObject()
{
    SetSatisfied(true);
}

bool plObjectInBoxConditionalObject::MsgReceive(plMessage* msg)
{
    plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
    if (pActivateMsg)
    {
        if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeEnter)
        {
            fInside.emplace_back(pActivateMsg->fHitterObj);
            return true;
        }
        else
        if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeExit)
        {
            for (auto iter = fInside.begin(); iter != fInside.end(); )
            {
                if (*iter == pActivateMsg->fHitterObj)
                {
                    iter = fInside.erase(iter);
                    if (pActivateMsg->fHitterObj == fCurrentTrigger && fCurrentTrigger && fLogicMod->HasFlag(plLogicModBase::kTriggered) && !IsToggle())
                    {
                        fCurrentTrigger = nullptr;
                        fLogicMod->GetNotify()->AddContainerEvent(pActivateMsg->fHiteeObj, pActivateMsg->fHitterObj, false);
                        fLogicMod->RequestUnTrigger();
                    }
                }
                else
                {
                    ++iter;
                }
            }
            return true;
        }

        return false;
    }
    return plConditionalObject::MsgReceive(msg);
}

bool plObjectInBoxConditionalObject::Verify(plMessage* msg)
{
    plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
    if (pActivateMsg)
    {
        for (const plKey& inKey : fInside)
        {
            if (pActivateMsg->fHitterObj == inKey)
            {
                fLogicMod->GetNotify()->AddContainerEvent(pActivateMsg->fHiteeObj, pActivateMsg->fHitterObj, true);
                fCurrentTrigger = pActivateMsg->fHiteeObj;
                return true;
            }
        }
    }

    plFakeOutMsg* pFakeMsg = plFakeOutMsg::ConvertNoRef(msg);
    if (pFakeMsg && plNetClientApp::GetInstance()->GetLocalPlayerKey())
    {
        for (const plKey& inKey : fInside)
        {
            if (plNetClientApp::GetInstance()->GetLocalPlayerKey() == inKey)
                return true;
        }
    }
    return false;
}


//
// volume sensor conditional 
//
plVolumeSensorConditionalObject::plVolumeSensorConditionalObject() :
    fTrigNum(-1),
    fType(0),
    fFirst(false),
    fTriggered(false),
    fFlags(kIgnoreExtraEnters)
{
    SetSatisfied(true);
}

void plVolumeSensorConditionalObject::IgnoreExtraEnters(bool ignore)
{
    if (ignore)
        fFlags |= kIgnoreExtraEnters;
    else
        fFlags &= ~kIgnoreExtraEnters;
}

void plVolumeSensorConditionalObject::NoServerArbitration(bool noArbitration)
{
    if (noArbitration) {
        plgDispatch::Dispatch()->RegisterForExactType(plPlayerPageMsg::Index(), GetKey());
        fFlags |= kNoServerArbitration;
    } else {
        plgDispatch::Dispatch()->UnRegisterForExactType(plPlayerPageMsg::Index(), GetKey());
        fFlags &= ~kNoServerArbitration;
    }
}

bool plVolumeSensorConditionalObject::IIsLocal(const plKey& key) const
{
    if (key == plNetClientApp::GetInstance()->GetLocalPlayerKey())
        return true;

    const plSceneObject* hitter = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());
    if (hitter) {
        for (size_t i = 0; i < hitter->GetNumModifiers(); ++i) {
            const plArmatureMod* am = plArmatureMod::ConvertNoRef(hitter->GetModifier(i));
            if (am && !am->IsLocalAI())
                return false;
        }
        if (hitter->IsLocallyOwned() != plSynchedObject::kYes)
            return false;
    }

    // Yes, I know that we're saying YES for not loaded objects. This matches the previous behavior.
    return true;
}

bool plVolumeSensorConditionalObject::MsgReceive(plMessage* msg)
{
    plActivatorMsg* pActivateMsg = plActivatorMsg::ConvertNoRef(msg);
    if (pActivateMsg) {
        // single player hack
        if (!fLogicMod->HasFlag(plLogicModBase::kRequestingTrigger))
            fLogicMod->GetNotify()->ClearEvents();

        // Track the hittee for the NoArbitration case so we can trigger the exit volume on link out
        fHittee = pActivateMsg->fHiteeObj;

        // Track the enters/exits on all clients
        if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeEnter) {
            auto it = fInside.find(pActivateMsg->fHitterObj);
            if (it != fInside.end() && fFlags & kIgnoreExtraEnters) {
                // This is normally what we should do. You're already inside the region,
                // so we don't care about a dupe enter. However, PhysX is weird, so sometimes
                // we might want to allow dupe enters.
                return false;
            }
            fInside.insert(pActivateMsg->fHitterObj);

            // From here on out, we only care about local avatars
            if (!IIsLocal(pActivateMsg->fHitterObj))
                return false;

            if (fType == kTypeEnter) {
                fLogicMod->GetNotify()->AddCollisionEvent(true, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
                if ((fFlags & kNoServerArbitration)) {
                    if (Satisfied())
                        fLogicMod->Trigger(false);
                } else {
                    fLogicMod->RequestTrigger(false);
                }
            } else if (fType == kTypeExit && !(fFlags & kNoServerArbitration)) {
                fLogicMod->GetNotify()->AddCollisionEvent(false, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
                fLogicMod->RequestUnTrigger();
            }
            return false;
        } else if (pActivateMsg->fTriggerType == plActivatorMsg::kVolumeExit) {
            auto it = fInside.find(pActivateMsg->fHitterObj);
            if (it == fInside.end())
                return false;
            fInside.erase(it);

            // From here on out, we only care about local avatars
            if (!IIsLocal(pActivateMsg->fHitterObj))
                return false;

            if (fType == kTypeExit) {
                fLogicMod->GetNotify()->AddCollisionEvent(false, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
                if (fFlags & kNoServerArbitration) {
                    if (Satisfied())
                        fLogicMod->Trigger(false);
                } else {
                    fLogicMod->RequestTrigger(false);
                }
            } else if (fType == kTypeEnter && !(fFlags & kNoServerArbitration)) {
                fLogicMod->GetNotify()->AddCollisionEvent(true, pActivateMsg->fHitterObj, pActivateMsg->fHiteeObj, false);
                fLogicMod->RequestUnTrigger();
            }
            return true;
        }
        return true;
    }

    plPlayerPageMsg* page = plPlayerPageMsg::ConvertNoRef(msg);
    if (page && page->fUnload) {
        hsAssert(fFlags & kNoServerArbitration, "WTF -- should only get here if the VSCO skips arbitration!");

        auto it = fInside.find(page->fPlayer);
        if (it != fInside.end()) {
            fInside.erase(it);
            if (fHittee && fType == kTypeExit) {
                const plSceneObject* hitteeSO = plSceneObject::ConvertNoRef(fHittee->ObjectIsLoaded());
                if (hitteeSO && hitteeSO->IsLocallyOwned() == plSynchedObject::kYes) {
                    fLogicMod->GetNotify()->AddCollisionEvent(false, page->fPlayer, fHittee, false);
                    if (Satisfied())
                        fLogicMod->Trigger(false);
                }
            }
        }
        return true;
    }

    return plConditionalObject::MsgReceive(msg);
}

bool plVolumeSensorConditionalObject::Satisfied()
{
    if (fType == kTypeExit && fFirst && !fTriggered)
    {
        if (!fInside.empty())
            fTriggered = true;
        return true;
    }
    if (fTriggered)
    {
        if (fInside.empty())
            fTriggered = false;
        return false;
    }

    if (fTrigNum == -1)
        return true;
    if (fInside.size() == fTrigNum)
        return true;
    else
        return false;
}

void plVolumeSensorConditionalObject::Read(hsStream* stream, hsResMgr* mgr)
{
    plConditionalObject::Read(stream, mgr);
    fTrigNum = stream->ReadLE32();
    fType = stream->ReadLE32();
    fFirst = stream->ReadBool();
}

void plVolumeSensorConditionalObject::Write(hsStream* stream, hsResMgr* mgr)
{
    plConditionalObject::Write(stream, mgr);
    stream->WriteLE32(fTrigNum);
    stream->WriteLE32(fType);
    stream->WriteBool(fFirst);
}

void plVolumeSensorConditionalObjectNoArbitration::Read(hsStream* stream, hsResMgr* mgr)
{
    plVolumeSensorConditionalObject::Read(stream, mgr);

    // We must have a valid fpKey before we do this, hence why this is not in the constructor
    NoServerArbitration(true);
}
