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
#include "plCloneSpawnModifier.h"

#include "hsResMgr.h"
#include "plResMgr/plResManager.h"
#include "plResMgr/plKeyFinder.h"
#include "pnSceneObject/plSceneObject.h"

#include "pnMessage/plClientMsg.h"
#include "plgDispatch.h"
#include "pnMessage/plWarpMsg.h"
#include "pnMessage/plNodeRefMsg.h"
#include "plMessage/plLoadCloneMsg.h"

plCloneSpawnModifier::plCloneSpawnModifier() : fExportTime(false)
{
}

void plCloneSpawnModifier::Read(hsStream *s, hsResMgr *mgr)
{
    fTemplateName = s->ReadSafeString();
    plSingleModifier::Read(s, mgr);
}

void plCloneSpawnModifier::Write(hsStream *s, hsResMgr *mgr)
{
    s->WriteSafeString(fTemplateName);
    plSingleModifier::Write(s, mgr);
}

void plCloneSpawnModifier::SetTarget(plSceneObject* so)
{
    fTarget = so;
    // Spawning the clone here since at Read time fTarget isn't set.  Kind of a hack though.
    if (fTarget && !fExportTime)
    {
        // Assume the clone template is in the same age we are
        const plLocation& loc = GetKey()->GetUoid().GetLocation();
        ST::string ageName;
        ((plResManager*)hsgResMgr::ResMgr())->GetLocationStrings(loc, &ageName, nullptr);

        // Spawn the clone
        plKey cloneKey = SpawnClone(fTemplateName, ageName, GetTarget()->GetLocalToWorld(), GetKey());
    }
}


plKey plCloneSpawnModifier::SpawnClone(const ST::string& cloneName, const ST::string& cloneAge, const hsMatrix44& pos, plKey requestor)
{
    plResManager* resMgr = (plResManager*)hsgResMgr::ResMgr();

    // Find the clone room for this age
    const plLocation& loc = plKeyFinder::Instance().FindLocation(cloneAge, "BuiltIn");

    // Find the clone template key
    plUoid objUoid(loc, plSceneObject::Index(), cloneName);
    plKey key = resMgr->FindKey(objUoid);

    if (key)
    {
        plLoadCloneMsg* cloneMsg = new plLoadCloneMsg(objUoid, std::move(requestor), 0);
        cloneMsg->SetBCastFlag(plMessage::kMsgWatch);
        plKey cloneKey = cloneMsg->GetCloneKey();//resMgr->CloneKey(key);
        cloneMsg->Send();

        // Put the clone into the clone room, which also forces it to load.
        plKey cloneNodeKey = resMgr->FindKey(kNetClientCloneRoom_KEY);
        plNodeRefMsg* nodeRefCloneMsg = new plNodeRefMsg(cloneNodeKey, plNodeRefMsg::kOnRequest, -1, plNodeRefMsg::kObject);
        resMgr->AddViaNotify(cloneKey, nodeRefCloneMsg, plRefFlags::kActiveRef);

        // Warp it into position
        plWarpMsg *warpMsg = new plWarpMsg;
        warpMsg->AddReceiver(cloneKey);
        warpMsg->SetTransform(pos);
        plgDispatch::MsgSend(warpMsg);

        return cloneKey;
    }

    return nullptr;
}
