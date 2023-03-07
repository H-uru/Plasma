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
//////////////////////////////////////////////////////////////////////
//
// pyDrawControl   - a wrapper class all the draw/pipeline control functions
//
//////////////////////////////////////////////////////////////////////

#include "plgDispatch.h"
#include "hsResMgr.h"

#include "pyDrawControl.h"

#include <string_theory/format>

#ifndef BUILDING_PYPLASMA
#   include "plAvatar/plArmatureMod.h"
#   include "plGLight/plShadowCaster.h"
#   include "plGLight/plShadowMaster.h"
#   include "plInputCore/plInputDevice.h"
#   include "pnMessage/plClientMsg.h"
#   include "plMessage/plInputEventMsg.h"
#endif

void pyDrawControl::SetGamma2(float gamma)
{
#ifndef BUILDING_PYPLASMA
    ST::string command = ST::format("Graphics.Renderer.Gamma2 {}", gamma);
    // create message to send to the console
    plControlEventMsg* pMsg = new plControlEventMsg;
    pMsg->SetBCastFlag(plMessage::kBCastByType);
    pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
    pMsg->SetControlActivated(true);
    pMsg->SetCmdString(command);
    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
#endif
}

#ifndef BUILDING_PYPLASMA

#endif

void pyDrawControl::SetShadowVisDistance(float distance)
{
#ifndef BUILDING_PYPLASMA
    plShadowMaster::SetGlobalShadowQuality(distance);
#endif
}

float pyDrawControl::GetShadowVisDistance()
{
#ifndef BUILDING_PYPLASMA
    return plShadowMaster::GetGlobalShadowQuality();
#else
    return 1.0;
#endif
}

void pyDrawControl::EnableShadows()
{
#ifndef BUILDING_PYPLASMA
    plShadowCaster::EnableShadowCast();
#endif
}
void pyDrawControl::DisableShadows()
{
#ifndef BUILDING_PYPLASMA
    plShadowCaster::DisableShadowCast();
#endif
}
bool pyDrawControl::IsShadowsEnabled()
{
#ifndef BUILDING_PYPLASMA
    return !plShadowCaster::ShadowCastDisabled();
#else
    return true;
#endif
}

bool pyDrawControl::CanShadowCast()
{
#ifndef BUILDING_PYPLASMA
    return plShadowCaster::CanShadowCast();
#else
    return false;
#endif
}

void pyDrawControl::DisableRenderScene()
{
#ifndef BUILDING_PYPLASMA
    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
    plClientMsg* msg = new plClientMsg(plClientMsg::kDisableRenderScene);
    msg->AddReceiver( clientKey );
    msg->Send();
#endif
}

void pyDrawControl::EnableRenderScene()
{
#ifndef BUILDING_PYPLASMA
    plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
    plClientMsg* msg = new plClientMsg(plClientMsg::kEnableRenderScene);
    msg->AddReceiver( clientKey );
    msg->Send();
#endif
}

void pyDrawControl::SetMouseInverted()
{
#ifndef BUILDING_PYPLASMA
    plMouseDevice::SetInverted(true);
#endif
}

void pyDrawControl::SetMouseUninverted()
{
#ifndef BUILDING_PYPLASMA
    plMouseDevice::SetInverted(false);
#endif
}

bool pyDrawControl::IsMouseInverted()
{
#ifndef BUILDING_PYPLASMA
    return plMouseDevice::GetInverted();
#else
    return false;
#endif
}

void pyDrawControl::SetClickToTurn(bool state)
{
#ifndef BUILDING_PYPLASMA
    plArmatureMod::fClickToTurn = state;
#endif
}

bool pyDrawControl::IsClickToTurn()
{
#ifndef BUILDING_PYPLASMA
    if (plArmatureMod::fClickToTurn)
        return true;
#endif
    return false;
}
