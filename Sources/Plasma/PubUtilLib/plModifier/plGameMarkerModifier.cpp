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

#include "plGameMarkerModifier.h"

#include "hsResMgr.h"

#include "pnMessage/plNotifyMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plCollideMsg.h"

bool plGameMarkerModifier::MsgReceive(plMessage* msg)
{
    plCollideMsg *collideMsg = plCollideMsg::ConvertNoRef(msg);
    if (collideMsg)
    {
        if (collideMsg->fEntering)
        {
            plNotifyMsg* notify = new plNotifyMsg;
            notify->AddCollisionEvent(true, collideMsg->fOtherKey, GetTarget()->GetKey());
            notify->Send(hsgResMgr::ResMgr()->FindKey(kMarkerMgr_KEY));
        }
    }

    return plSingleModifier::MsgReceive(msg);
}

plKey plGameMarkerModifier::IFindCloneKey(const plKey& baseKey)
{
    const plUoid& myUoid = GetKey()->GetUoid();
    plUoid cloneUoid = baseKey->GetUoid();
    cloneUoid.SetClone(myUoid.GetClonePlayerID(), myUoid.GetCloneID());
    return hsgResMgr::ResMgr()->FindKey(cloneUoid);
}

void plGameMarkerModifier::FixupAnimKeys()
{
    fGreenAnimKey   = IFindCloneKey(fGreenAnimKey);
    fRedAnimKey     = IFindCloneKey(fRedAnimKey);
    fOpenAnimKey    = IFindCloneKey(fOpenAnimKey);
    fBounceAnimKey  = IFindCloneKey(fBounceAnimKey);
}

void plGameMarkerModifier::Read(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Read(stream, mgr);

    fGreenAnimKey   = mgr->ReadKey(stream);
    fRedAnimKey     = mgr->ReadKey(stream);
    fOpenAnimKey    = mgr->ReadKey(stream);
    fBounceAnimKey  = mgr->ReadKey(stream);
    fPlaceSndIdx    = stream->ReadLE16();
    fHitSndIdx      = stream->ReadLE16();
}

void plGameMarkerModifier::Write(hsStream* stream, hsResMgr* mgr)
{
    plSingleModifier::Write(stream, mgr);

    mgr->WriteKey(stream, fGreenAnimKey);
    mgr->WriteKey(stream, fRedAnimKey);
    mgr->WriteKey(stream, fOpenAnimKey);
    mgr->WriteKey(stream, fBounceAnimKey);
    stream->WriteLE16(fPlaceSndIdx);
    stream->WriteLE16(fHitSndIdx);
}
