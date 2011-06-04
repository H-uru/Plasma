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
#include "cyParticleSys.h"

#include "hsStream.h"
#include "../pnMessage/plMessage.h"
#include "hsResMgr.h"
#include "plgDispatch.h"
#include "../plMessage/plParticleUpdateMsg.h"

cyParticleSys::cyParticleSys(plKey sender, plKey recvr)
{
	SetSender(sender);
	AddRecvr(recvr);
	fNetForce = false;
}

// setters
void cyParticleSys::SetSender(plKey &sender)
{
	fSender = sender;
}

void cyParticleSys::AddRecvr(plKey &recvr)
{
	if ( recvr != nil )
		fRecvr.Append(recvr);
}

void cyParticleSys::SetNetForce(hsBool state)
{
	// set our flag
	fNetForce = state;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ISendParticleSysMsg
//  PARAMETERS : 
//
//  PURPOSE    : send the message to the Particle System
//
void cyParticleSys::ISendParticleSysMsg(UInt32 param, hsScalar value)
{
	plParticleUpdateMsg* pMsg = TRACKED_NEW plParticleUpdateMsg(fSender, nil, nil, param, value);
	// check if this needs to be network forced to all clients
	if (fNetForce )
	{
		// set the network propagate flag to make sure it gets to the other clients
		pMsg->SetBCastFlag(plMessage::kNetPropagate);
		pMsg->SetBCastFlag(plMessage::kNetForce);
	}
	pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);
	// add all our receivers to the message receiver list
	int i;
	for ( i=0; i<fRecvr.Count(); i++ )
	{
		pMsg->AddReceiver(fRecvr[i]);
	}
	plgDispatch::MsgSend(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
//
// All these methods just call the IsendParticleSysMsg to do the real work
//
void cyParticleSys::SetParticlesPerSecond(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamParticlesPerSecond,value);
}

void cyParticleSys::SetInitPitchRange(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamInitPitchRange,value);
}

void cyParticleSys::SetInitYawRange(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamInitYawRange,value);
}

void cyParticleSys::SetVelMin(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamVelMin,value);
}

void cyParticleSys::SetVelMax(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamVelMax,value);
}

void cyParticleSys::SetXSize(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamXSize,value);
}

void cyParticleSys::SetYSize(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamYSize,value);
}

void cyParticleSys::SetScaleMin(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamScaleMin,value);
}

void cyParticleSys::SetScaleMax(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamScaleMax,value);
}

void cyParticleSys::SetGenLife(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamGenLife,value);
}

void cyParticleSys::SetPartLifeMin(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamPartLifeMin,value);
}

void cyParticleSys::SetPartLifeMax(hsScalar value)
{
	ISendParticleSysMsg(plParticleUpdateMsg::kParamPartLifeMax,value);
}
