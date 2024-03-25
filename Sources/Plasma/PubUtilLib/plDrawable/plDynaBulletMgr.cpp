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

#include "HeadSpin.h"
#include "plDynaBulletMgr.h"
#include "plDynaDecal.h"

#include "plMessage/plBulletMsg.h"

#include "plCutter.h"

#include "plgDispatch.h"


#include "hsStream.h"
#include "hsResMgr.h"
#include "hsTimer.h"
#include "pnMessage/plRefMsg.h"


#include "hsFastMath.h"


size_t plDynaBulletMgr::INewDecal()
{
    size_t idx = fDecals.size();
    fDecals.emplace_back(new plDynaSplot);

    return idx;
}

plDynaBulletMgr::plDynaBulletMgr()
{
}

plDynaBulletMgr::~plDynaBulletMgr()
{
}

void plDynaBulletMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Read(stream, mgr);

    plgDispatch::Dispatch()->RegisterForExactType(plBulletMsg::Index(), GetKey());
}

void plDynaBulletMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Write(stream, mgr);
}

bool plDynaBulletMgr::IHandleEnableMsg(const plDynaDecalEnableMsg* enaMsg)
{
    return true;
}

bool plDynaBulletMgr::IHandleShot(plBulletMsg* bull)
{
    hsVector3 up = IRandomUp(bull->Dir());

    hsPoint3 pos = bull->From() + bull->Dir() * (bull->Range() * 0.5f);
    fCutter->SetLength(hsVector3(bull->Radius() * fScale.fX, bull->Radius() * fScale.fY, bull->Range()));
    fCutter->Set(pos, up, -bull->Dir());

    plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(this), GetKey());

    if( bull->PartyTime() > 0 )
        fPartyTime = bull->PartyTime();

    double secs = hsTimer::GetSysSeconds();

    if( ICutoutTargets(secs) )
        info.fLastTime = secs;
    
    return true;
}

bool plDynaBulletMgr::MsgReceive(plMessage* msg)
{
    plBulletMsg* bullMsg = plBulletMsg::ConvertNoRef(msg);
    if( bullMsg )
    {
        if( bullMsg->Shot() )
        {
            return IHandleShot(bullMsg);
        }
        return true;
    }
    return plDynaDecalMgr::MsgReceive(msg);
}

