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
#include "plgDispatch.h"
#include "hsResMgr.h"
#include "hsStream.h"
#include "hsTimer.h"

#include "plCutter.h"
#include "plDynaFootMgr.h"
#include "plDynaDecal.h"
#include "plPrintShape.h"

#include "pnEncryption/plRandom.h"

#include "plAvatar/plArmatureMod.h"
#include "plAvatar/plAvBrainHuman.h"
#include "plMessage/plAvatarFootMsg.h"
#include "plMessage/plDynaDecalEnableMsg.h"

static plRandom sRand;

size_t plDynaFootMgr::INewDecal()
{
    size_t idx = fDecals.size();
    fDecals.emplace_back(new plDynaSplot);

    return idx;
}

plDynaFootMgr::plDynaFootMgr()
{
    fPartIDs = {
        plAvBrainHuman::RFootPrint,
        plAvBrainHuman::LFootPrint
    };
}

plDynaFootMgr::~plDynaFootMgr()
{
}

void plDynaFootMgr::Read(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Read(stream, mgr);

    plgDispatch::Dispatch()->RegisterForExactType(plAvatarFootMsg::Index(), GetKey());
}

void plDynaFootMgr::Write(hsStream* stream, hsResMgr* mgr)
{
    plDynaDecalMgr::Write(stream, mgr);
}


bool plDynaFootMgr::MsgReceive(plMessage* msg)
{
    plAvatarFootMsg* footMsg = plAvatarFootMsg::ConvertNoRef(msg);
    if( footMsg )
    {
        uint32_t id = footMsg->IsLeft() ? plAvBrainHuman::LFootPrint : plAvBrainHuman::RFootPrint;

        plArmatureMod* armMod = footMsg->GetArmature();
        const plPrintShape* shape = IGetPrintShape(armMod, id);
        if( shape )
        {
            plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());
            if( IPrintFromShape(shape, footMsg->IsLeft()) )
            {
                INotifyActive(info, armMod->GetKey(), id);
            }
            else
            {
                INotifyInactive(info, armMod->GetKey(), id);
            }
        }

        return true;
    }

    return plDynaDecalMgr::MsgReceive(msg);
}

bool plDynaFootMgr::IPrintFromShape(const plPrintShape* shape, bool flip)
{
    bool retVal = false;

    if( shape )
    {
        plDynaDecalInfo& info = IGetDecalInfo(uintptr_t(shape), shape->GetKey());

        double secs = hsTimer::GetSysSeconds();
        float wetness = IHowWet(info, secs);
        fInitAtten = wetness;

        if( wetness <= 0 )
            return true;

        hsMatrix44 shapeL2W = shape->GetOwner()->GetLocalToWorld();
        hsPoint3 newPos = shapeL2W.GetTranslate();
        hsVector3 newDir = shapeL2W.GetAxis(hsMatrix44::kView);
        hsVector3 newUp(0.f, 0.f, 1.f);

        hsVector3 size(shape->GetWidth() * fScale.fX, shape->GetLength() * fScale.fY, shape->GetHeight() * fScale.fZ * 2.f);
        fCutter->SetLength(size);
        fCutter->Set(newPos, newDir, newUp, flip);

        // Should this be moved inside the if( ICutout() ) clause? I think so. Probably doesn't
        // matter for foot prints, but it seems more correct, since fLastPos/fLastTime is the
        // last time and position we actually dropped a print, not tried to.
        info.fLastPos = newPos;
        info.fLastTime = secs;

        if( ICutoutTargets(secs) )
        {
            retVal = true;
        }
    }
    return retVal;
}
