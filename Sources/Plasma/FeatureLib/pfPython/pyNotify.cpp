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
// pyNotify   - a wrapper class to provide interface to send a NotifyMsg
//
//////////////////////////////////////////////////////////////////////

#include <string_theory/string>

#include "plgDispatch.h"
#include "pyGeometry3.h"
#include "pyKey.h"

#include "pyNotify.h"


pyNotify::pyNotify()
    : fNetPropagate(true), fNetForce(false)
{
    fBuildMsg.fType = plNotifyMsg::kActivator;
    fBuildMsg.fState = 0.0f;
    fBuildMsg.fID = 0;
}

pyNotify::pyNotify(const pyKey& selfkey)
{
    fSenderKey = selfkey.getKey();
    fNetPropagate = true;
    fNetForce = false;
    fBuildMsg.fType = plNotifyMsg::kActivator;
    fBuildMsg.fState = 0.0f;
    fBuildMsg.fID = 0;
    // loop though adding the ones that want to be notified of the change
    for (size_t j = 0; j < selfkey.NotifyListCount(); j++)
        fReceivers.emplace_back(selfkey.GetNotifyListItem(j));
}

void pyNotify::SetSender(const pyKey& selfKey)
{
    fSenderKey = selfKey.getKey();
    fReceivers.clear();
    for (size_t j = 0; j < selfKey.NotifyListCount(); j++)
        fReceivers.emplace_back(selfKey.GetNotifyListItem(j));
}

// methods that will be exposed to Python
void pyNotify::ClearReceivers()
{
    fReceivers.clear();
}

void pyNotify::AddReceiver(pyKey* key)
{
    if (key)
        fReceivers.emplace_back(key->getKey());
}


void pyNotify::SetActivateState(float state)
{
    fBuildMsg.SetState(state);
}

void pyNotify::SetType(int32_t type)
{
    fBuildMsg.fType = type;
}


//////////////////////////////////////////////////
//  Add event record helpers
//////////////////////////////////////////////////

void pyNotify::AddCollisionEvent( bool enter, pyKey* other, pyKey* self )
{
    fBuildMsg.AddCollisionEvent(enter, other ? other->getKey() : plKey(),
                                        self ? self->getKey() : plKey() );
}

void pyNotify::AddPickEvent( bool enabled, pyKey* other, pyKey* self, pyPoint3 hitPoint)
{
    fBuildMsg.AddPickEvent( other ? other->getKey() : plKey(),
                                self ? self->getKey() : plKey(),
                                enabled,
                                hitPoint.fPoint );
}

void pyNotify::AddControlKeyEvent( int32_t key, bool down )
{
    fBuildMsg.AddControlKeyEvent(key,down);
}

void pyNotify::AddVarNumber(const ST::string& name, float number)
{
    fBuildMsg.AddVariableEvent(name, number);
}

void pyNotify::AddVarNumber(const ST::string& name, int32_t number)
{
    fBuildMsg.AddVariableEvent(name, number);
}

void pyNotify::AddVarNull(const ST::string& name)
{
    fBuildMsg.AddVariableEvent(name);
}

void pyNotify::AddVarKey(const ST::string& name, pyKey* key)
{
    fBuildMsg.AddVariableEvent(name, key ? key->getKey() : plKey() );
}

void pyNotify::AddFacingEvent( bool enabled, pyKey* other, pyKey* self, float dot)
{
    fBuildMsg.AddFacingEvent( other ? other->getKey() : plKey(),
                                self ? self->getKey() : plKey(),
                                dot, enabled);
}

void pyNotify::AddContainerEvent( bool entering, pyKey* contained, pyKey* container)
{
    fBuildMsg.AddContainerEvent( container ? container->getKey() : plKey(),
                                    contained ? contained->getKey() : plKey() ,
                                    entering);
}

void pyNotify::AddActivateEvent( bool active, bool activate )
{
    fBuildMsg.AddActivateEvent(activate);
}

void pyNotify::AddCallbackEvent( int32_t event )
{
    fBuildMsg.AddCallbackEvent(event);
}

void pyNotify::AddResponderState(int32_t state)
{
    fBuildMsg.AddResponderStateEvent(state);
}

void pyNotify::Send()
{
    if (fReceivers.empty())        // Notify msgs must have receivers, can't be bcast by type
        return;

    // create new notify message to do the actual send with
    plNotifyMsg* pNMsg = new plNotifyMsg;

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
    for (proEventData* pED : fBuildMsg.fEvents)
        pNMsg->AddEvent( pED );

    // add receivers
    // loop though adding the ones that want to be notified of the change
    for (const plKey& rcKey : fReceivers)
        pNMsg->AddReceiver(rcKey);

    pNMsg->SetSender(fSenderKey);
    plgDispatch::MsgSend( pNMsg );
}
