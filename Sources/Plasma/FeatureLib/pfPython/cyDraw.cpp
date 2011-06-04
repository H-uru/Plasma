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
#include "cyDraw.h"

#include "plgDispatch.h"
#include "hsBitVector.h"
#include "../pnMessage/plEnableMsg.h"

cyDraw::cyDraw(plKey sender, plKey recvr)
{
	SetSender(sender);
	AddRecvr(recvr);
	fNetForce = false;
}

// setters
void cyDraw::SetSender(plKey &sender)
{
	fSender = sender;
}

void cyDraw::AddRecvr(plKey &recvr)
{
	if ( recvr != nil )
		fRecvr.Append(recvr);
}


void cyDraw::SetNetForce(hsBool state)
{
	// set our flag
	fNetForce = state;
}


void cyDraw::EnableT(hsBool state)
{
	// must have a receiver!
	if ( fRecvr.Count() > 0 )
	{
		// create message
		plEnableMsg* pMsg = TRACKED_NEW plEnableMsg;
		// check if this needs to be network forced to all clients
		if (fNetForce )
		{
			// set the network propagate flag to make sure it gets to the other clients
			pMsg->SetBCastFlag(plMessage::kNetPropagate);
			pMsg->SetBCastFlag(plMessage::kNetForce);
		}
		if ( fSender )
			pMsg->SetSender(fSender);

		// add all our receivers to the message receiver list
		int i;
		for ( i=0; i<fRecvr.Count(); i++ )
		{
			pMsg->AddReceiver(fRecvr[i]);
		}
		// set the interface to the draw
		pMsg->SetCmd(plEnableMsg::kDrawable);
		pMsg->AddType(plEnableMsg::kDrawable);

		pMsg->SetBCastFlag(plMessage::kPropagateToModifiers);

		// which way are we doin' it?
		if ( state )
			pMsg->SetCmd(plEnableMsg::kEnable);
		else
			pMsg->SetCmd(plEnableMsg::kDisable);
		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void cyDraw::Enable()
{
	EnableT(true);
}

void cyDraw::Disable()
{
	EnableT(false);
}
