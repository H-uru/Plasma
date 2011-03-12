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
// pyNotify   - a wrapper class to provide interface to send a NotifyMsg
//
//////////////////////////////////////////////////////////////////////

#include "plgDispatch.h"
#include "../pnMessage/plNotifyMsg.h"
#include "pyKey.h"
#include "plPythonFileMod.h"
#include "pyGeometry3.h"

#include "pyNotify.h"

pyNotify::pyNotify()
{
	fSenderKey = nil;
	fNetPropagate = true;
	fNetForce = false;
	fBuildMsg.fType = plNotifyMsg::kActivator;
	fBuildMsg.fState = 0.0f;
	fBuildMsg.fID = 0;
}

pyNotify::pyNotify(pyKey& selfkey) 
{
	fSenderKey = selfkey.getKey();
	fNetPropagate = true;
	fNetForce = false;
	fBuildMsg.fType = plNotifyMsg::kActivator;
	fBuildMsg.fState = 0.0f;
	fBuildMsg.fID = 0;
	// loop though adding the ones that want to be notified of the change
	int j;
	for ( j=0 ; j<selfkey.NotifyListCount() ; j++ )
		fReceivers.Append(selfkey.GetNotifyListItem(j));
}

pyNotify::~pyNotify()
{
}

void pyNotify::SetSender(pyKey& selfKey)
{
	fSenderKey = selfKey.getKey();
	fReceivers.Reset();
	for (int j = 0; j < selfKey.NotifyListCount(); j++)
		fReceivers.Append(selfKey.GetNotifyListItem(j));
}

// methods that will be exposed to Python
void pyNotify::ClearReceivers()
{
	fReceivers.Reset();
}

void pyNotify::AddReceiver(pyKey* key)
{
	if (key)
		fReceivers.Append(key->getKey());
}

void pyNotify::SetNetPropagate(hsBool propagate)
{
	fNetPropagate = propagate;
}

void pyNotify::SetNetForce(hsBool state)
{
	fNetForce = state;
}


void pyNotify::SetActivateState(hsScalar state)
{
	fBuildMsg.SetState(state);
}

void pyNotify::SetType(Int32 type)
{
	fBuildMsg.fType = type;
}


//////////////////////////////////////////////////
//  Add event record helpers
//////////////////////////////////////////////////

void pyNotify::AddCollisionEvent( hsBool enter, pyKey* other, pyKey* self )
{
	fBuildMsg.AddCollisionEvent(enter, other ? other->getKey() : plKey(),
										self ? self->getKey() : plKey() );
}

void pyNotify::AddPickEvent( hsBool enabled, pyKey* other, pyKey* self, pyPoint3 hitPoint)
{
	fBuildMsg.AddPickEvent( other ? other->getKey() : plKey(),
								self ? self->getKey() : plKey(),
								enabled,
								hitPoint.fPoint );
}

void pyNotify::AddControlKeyEvent( Int32 key, hsBool down )
{
	fBuildMsg.AddControlKeyEvent(key,down);
}

void pyNotify::AddVarNumber(const char* name, hsScalar number)
{
	fBuildMsg.AddVariableEvent(name,number);
}

void pyNotify::AddVarKey(const char* name, pyKey* key)
{
	fBuildMsg.AddVariableEvent(name, key ? key->getKey() : plKey() );
}

void pyNotify::AddFacingEvent( hsBool enabled, pyKey* other, pyKey* self, hsScalar dot)
{
	fBuildMsg.AddFacingEvent( other ? other->getKey() : plKey(),
								self ? self->getKey() : plKey(),
								dot, enabled);
}

void pyNotify::AddContainerEvent( hsBool entering, pyKey* contained, pyKey* container)
{
	fBuildMsg.AddContainerEvent( container ? container->getKey() : plKey(),
									contained ? contained->getKey() : plKey() ,
									entering);
}

void pyNotify::AddActivateEvent( hsBool active, hsBool activate )
{
	fBuildMsg.AddActivateEvent(activate);
}

void pyNotify::AddCallbackEvent( Int32 event )
{
	fBuildMsg.AddCallbackEvent(event);
}

void pyNotify::AddResponderState(Int32 state)
{
	fBuildMsg.AddResponderStateEvent(state);
}

void pyNotify::Send()
{
	if (!fReceivers.Count())		// Notify msgs must have receivers, can't be bcast by type
		return;

	// create new notify message to do the actual send with
	plNotifyMsg* pNMsg = TRACKED_NEW plNotifyMsg;

	if ( fNetPropagate )
		pNMsg->SetBCastFlag(plMessage::kNetPropagate);
	else
		pNMsg->SetBCastFlag(plMessage::kNetPropagate,false);
	// set whether this should be forced over the network (ignoring net-cascading)
	if ( fNetForce )
		pNMsg->SetBCastFlag(plMessage::kNetForce);

	// copy data and event records to new NotifyMsg
	pNMsg->fType = fBuildMsg.fType;
	pNMsg->fState = fBuildMsg.fState;
	pNMsg->fID = fBuildMsg.fID;
	// need to recreate all the events in the new message by Adding them
	int i;
	for ( i=0; i<fBuildMsg.fEvents.GetCount(); i++ )
	{
		proEventData* pED = fBuildMsg.fEvents.Get(i);
		pNMsg->AddEvent( pED );
	}

	// add receivers
	// loop though adding the ones that want to be notified of the change
	int j;
	for ( j=0 ; j<fReceivers.Count() ; j++ )
		pNMsg->AddReceiver(fReceivers[j]);

	pNMsg->SetSender(fSenderKey);
	plgDispatch::MsgSend( pNMsg );
}
