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

#include "cyParticleSys.h"

#include "plgDispatch.h"

#include "pnMessage/plMessage.h"

#include "plMessage/plParticleUpdateMsg.h"

cyParticleSys::cyParticleSys(plKey sender, plKey recvr)
{
    SetSender(std::move(sender));
    AddRecvr(std::move(recvr));
    fNetForce = false;
}

// setters
void cyParticleSys::SetSender(plKey sender)
{
    fSender = std::move(sender);
}

void cyParticleSys::AddRecvr(plKey recvr)
{
    if (recvr != nullptr)
        fRecvr.emplace_back(std::move(recvr));
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ISendParticleSysMsg
//  PARAMETERS : 
//
//  PURPOSE    : send the message to the Particle System
//
void cyParticleSys::ISendParticleSysMsg(uint32_t param, float value)
{
    plParticleUpdateMsg* pMsg = new plParticleUpdateMsg(fSender, nullptr, nullptr, param, value);
    // check if this needs to be network forced to all clients
    if (fNetForce )
    {
        // set the network propagate flag to make sure it gets to the other clients
        pMsg->SetBCastFlag(plMessage::kNetPropagate);
        pMsg->SetBCastFlag(plMessage::kNetForce);
    }
    pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
    // add all our receivers to the message receiver list
    for (const plKey& rcKey : fRecvr)
        pMsg->AddReceiver(rcKey);

    plgDispatch::MsgSend(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
//
// All these methods just call the IsendParticleSysMsg to do the real work
//
void cyParticleSys::SetParticlesPerSecond(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamParticlesPerSecond,value);
}

void cyParticleSys::SetInitPitchRange(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamInitPitchRange,value);
}

void cyParticleSys::SetInitYawRange(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamInitYawRange,value);
}

void cyParticleSys::SetVelMin(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamVelMin,value);
}

void cyParticleSys::SetVelMax(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamVelMax,value);
}

void cyParticleSys::SetXSize(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamXSize,value);
}

void cyParticleSys::SetYSize(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamYSize,value);
}

void cyParticleSys::SetScaleMin(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamScaleMin,value);
}

void cyParticleSys::SetScaleMax(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamScaleMax,value);
}

void cyParticleSys::SetGenLife(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamGenLife,value);
}

void cyParticleSys::SetPartLifeMin(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamPartLifeMin,value);
}

void cyParticleSys::SetPartLifeMax(float value)
{
    ISendParticleSysMsg(plParticleUpdateMsg::kParamPartLifeMax,value);
}
