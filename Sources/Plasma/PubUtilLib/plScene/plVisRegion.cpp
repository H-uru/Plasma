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

#include "plVisRegion.h"

#include "HeadSpin.h"
#include "hsResMgr.h"
#include "hsStream.h"

#include "plVisMgr.h"

#include "pnMessage/plEnableMsg.h"
#include "pnMessage/plRefMsg.h"

#include "plIntersect/plRegionBase.h"

plVisRegion::plVisRegion()
:   fIndex(),
    fRegion(),
    fMgr()
{
    fMgr = plGlobalVisMgr::Instance();

    SetProperty(kReplaceNormal, true);
}

plVisRegion::~plVisRegion()
{
    if( fMgr )
        fMgr->UnRegister(this, GetProperty(kIsNot));
}

bool plVisRegion::Eval(const hsPoint3& pos) const
{
    if( GetProperty(kDisable) )
        return false;

    if( !fRegion )
        return true;

    return fRegion->IsInside(pos);
}

bool plVisRegion::MsgReceive(plMessage* msg)
{
    plEnableMsg* enaMsg = plEnableMsg::ConvertNoRef(msg);
    if( enaMsg )
    {
        SetProperty(kDisable, enaMsg->Cmd(plEnableMsg::kDisable));
        return true;
    }
    plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef(msg);
    if( refMsg )
    {
        switch( refMsg->fType )
        {
        case kRefRegion:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                fRegion = plRegionBase::ConvertNoRef(refMsg->GetRef());
            }
            else
            {
                fRegion = nullptr;
            }
            return true;
        case kRefVisMgr:
            if( refMsg->GetContext() & (plRefMsg::kOnCreate|plRefMsg::kOnRequest|plRefMsg::kOnReplace) )
            {
                if( fMgr )
                    fMgr->UnRegister(this, GetProperty(kIsNot));
                fMgr = plVisMgr::ConvertNoRef(refMsg->GetRef());
                hsAssert(fMgr, "Just set my manager to nil.");
                fMgr->Register(this, GetProperty(kIsNot));
            }
            else
            {
                fMgr = nullptr;
            }
            return true;

        default:
            break;
        }
    }
    return plObjInterface::MsgReceive(msg);
}

void plVisRegion::Read(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Read(s, mgr);

    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefRegion), plRefFlags::kActiveRef);
    mgr->ReadKeyNotifyMe(s, new plGenRefMsg(GetKey(), plRefMsg::kOnCreate, 0, kRefVisMgr), plRefFlags::kActiveRef);

    if( fMgr )
        fMgr->Register(this, GetProperty(kIsNot));
}

void plVisRegion::Write(hsStream* s, hsResMgr* mgr)
{
    plObjInterface::Write(s, mgr);

    mgr->WriteKey(s, fRegion);

    mgr->WriteKey(s, fMgr);
}

