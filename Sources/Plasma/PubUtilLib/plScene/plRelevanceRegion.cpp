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

#include "plRelevanceRegion.h"
#include "plRelevanceMgr.h"

#include "hsResMgr.h"

#include "pnMessage/plRefMsg.h"

#include "plIntersect/plRegionBase.h"


void plRelevanceRegion::Read(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Read(s, mgr);
    
    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, 0), plRefFlags::kActiveRef);
    
    // Added to the manager when read in.
    // Removed when paged out, due to passive ref.
    if (plRelevanceMgr::Instance())
    {
        plGenRefMsg *msg = new plGenRefMsg(plRelevanceMgr::Instance()->GetKey(), plRefMsg::kOnCreate, -1, -1);
        hsgResMgr::ResMgr()->AddViaNotify(GetKey(), msg, plRefFlags::kPassiveRef);
    }
}

void plRelevanceRegion::Write(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Write(s, mgr);
    
    mgr->WriteKey(s, fRegion);
}

bool plRelevanceRegion::MsgReceive(plMessage* msg)
{
    plGenRefMsg *genMsg = plGenRefMsg::ConvertNoRef(msg);
    if (genMsg)
    {
        plRegionBase *base = plRegionBase::ConvertNoRef(genMsg->GetRef());
        if( genMsg->GetContext() & (plRefMsg::kOnCreate) )
        {
            fRegion = base;
        }
        else if( genMsg->GetContext() & (plRefMsg::kOnDestroy|plRefMsg::kOnRemove) )
        {
            fRegion = nullptr;
        }
        return true;
    }           

    return plObjInterface::MsgReceive(msg);
}


void plRelevanceRegion::SetMgrIndex(uint32_t index)
{
    if (fMgrIdx != (uint32_t)-1)
        fRegionsICareAbout.SetBit(fMgrIdx, false);

    fMgrIdx = index;
    fRegionsICareAbout.SetBit(index, true); // I care about myself. Awww...
}

