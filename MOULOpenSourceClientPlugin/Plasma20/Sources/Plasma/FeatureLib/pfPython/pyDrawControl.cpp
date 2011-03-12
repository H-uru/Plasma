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

#include "pyDrawControl.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


#ifndef BUILDING_PYPLASMA
#include "../plMessage/plInputEventMsg.h"
#include "../pnMessage/plClientMsg.h"
#include "../plInputCore/plInputDevice.h"
#include "../plAvatar/plArmatureMod.h"
#endif

void pyDrawControl::SetGamma2(hsScalar gamma)
{
#ifndef BUILDING_PYPLASMA
	char command[256];
	sprintf(command,"Graphics.Renderer.Gamma2 %f",gamma);
	// create message to send to the console
	plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
	pMsg->SetBCastFlag(plMessage::kBCastByType);
	pMsg->SetControlCode(B_CONTROL_CONSOLE_COMMAND);
	pMsg->SetControlActivated(true);
	pMsg->SetCmdString(command);
	plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
#endif
}

#ifndef BUILDING_PYPLASMA
#include "../plGLight/plShadowMaster.h"
#endif

void pyDrawControl::SetShadowVisDistance(hsScalar distance)
{
#ifndef BUILDING_PYPLASMA
	plShadowMaster::SetGlobalShadowQuality(distance);
#endif
}

hsScalar pyDrawControl::GetShadowVisDistance()
{
#ifndef BUILDING_PYPLASMA
	return plShadowMaster::GetGlobalShadowQuality();
#else
	return 1.0;
#endif
}

#ifndef BUILDING_PYPLASMA
#include "../plGLight/plShadowCaster.h"
#endif

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
hsBool pyDrawControl::IsShadowsEnabled()
{
#ifndef BUILDING_PYPLASMA
	return !plShadowCaster::ShadowCastDisabled();
#else
	return true;
#endif
}

hsBool pyDrawControl::CanShadowCast()
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
	plClientMsg* msg = TRACKED_NEW plClientMsg(plClientMsg::kDisableRenderScene);
	msg->AddReceiver( clientKey );
	msg->Send();
#endif
}

void pyDrawControl::EnableRenderScene()
{
#ifndef BUILDING_PYPLASMA
	plKey clientKey = hsgResMgr::ResMgr()->FindKey( kClient_KEY );
	plClientMsg* msg = TRACKED_NEW plClientMsg(plClientMsg::kEnableRenderScene);
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

hsBool pyDrawControl::IsMouseInverted()
{
#ifndef BUILDING_PYPLASMA
	return plMouseDevice::GetInverted();
#else
	return false;
#endif
}

void pyDrawControl::SetClickToTurn(hsBool state)
{
#ifndef BUILDING_PYPLASMA
	plArmatureMod::fClickToTurn = state;
#endif
}

hsBool pyDrawControl::IsClickToTurn()
{
#ifndef BUILDING_PYPLASMA
	if (plArmatureMod::fClickToTurn)
		return true;
#endif
	return false;
}
