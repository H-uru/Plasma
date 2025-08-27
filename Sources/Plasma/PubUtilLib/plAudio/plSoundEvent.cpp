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
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  plSoundEvent                                                            //
//                                                                          //
//// Notes ///////////////////////////////////////////////////////////////////
//                                                                          //
//  10.30.2001 - Created by mcn.                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plSoundEvent.h"

#include "plgDispatch.h"
#include "pnMessage/plEventCallbackMsg.h"
#include "pnMessage/plSoundMsg.h"
#include "plSound.h"

plSoundEvent::~plSoundEvent()
{
    for (plEventCallbackMsg* callback : fCallbacks)
        hsRefCnt_SafeUnRef(callback);
}

void    plSoundEvent::AddCallback( plEventCallbackMsg *msg )
{
    hsRefCnt_SafeRef( msg ); 
    fCallbacks.emplace_back(msg);
    fCallbackEndingFlags.emplace_back(0);
}

bool    plSoundEvent::RemoveCallback( plEventCallbackMsg *msg )
{
    auto iter = std::find(fCallbacks.cbegin(), fCallbacks.cend(), msg);
    if (iter != fCallbacks.cend())
    {
        hsRefCnt_SafeUnRef( msg );
        const auto idx = iter - fCallbacks.cbegin();
        fCallbacks.erase(iter);
        fCallbackEndingFlags.erase(fCallbackEndingFlags.cbegin() + idx);
        return true;
    }

    return false;
}

void    plSoundEvent::SendCallbacks()
{
    for (hsSsize_t j = fCallbacks.size() - 1; j >= 0; j--)
    {
        plEventCallbackMsg *msg = fCallbacks[ j ];
        
        if (!msg->HasBCastFlag(plMessage::kNetPropagate) || !fOwner ||
            fOwner->IsLocallyOwned() == plSynchedObject::kYes )     
        {
            /// Do this a bit differently so we can do our MsgSend last
            plSoundMsg* sMsg = nullptr;

            // Ref to make sure the dispatcher doesn't delete it on us
            hsRefCnt_SafeRef( msg );
            if( msg->fRepeats == 0 && fCallbackEndingFlags[ j ] == 0 )
            {
                // Note: we get fancy here. We never want to remove the callback directly,
                // because the sound won't know about it. So instead, send it a message to
                // remove the callback for us
                sMsg = new plSoundMsg();
                sMsg->SetBCastFlag( plMessage::kLocalPropagate, true );
                sMsg->AddReceiver( fOwner->GetKey() );
                sMsg->SetCmd( plSoundMsg::kRemoveCallbacks );
                sMsg->AddCallback( msg );
            }

            // If this isn't infinite, decrement the number of repeats
            if( msg->fRepeats > 0 )
                msg->fRepeats--;

            // And finally...
            if( fCallbackEndingFlags[ j ] == 0 )
            {
                plgDispatch::MsgSend( msg, true );
            }

            if (sMsg != nullptr)
            {
                plgDispatch::MsgSend( sMsg, true );
                fCallbackEndingFlags[ j ] = 0xff;       // Our special flag to mean "hey, don't
                                                        // process this, just waiting for 
                                                        // it to die"
            }
        }
    }
}

int plSoundEvent::GetType() const
{
    return (int)fType;
}

void    plSoundEvent::SetType( Types type )
{
    fType = type;
}

uint32_t  plSoundEvent::GetTime() const
{
    return fBytePosTime;
}

plSoundEvent::Types plSoundEvent::GetTypeFromCallbackMsg( plEventCallbackMsg *msg )
{
    switch( msg->fEvent )
    {
        case plEventCallbackMsg::kStart: return kStart;
        case plEventCallbackMsg::kTime:  return kTime;
        case plEventCallbackMsg::kStop:  return kStop;
        case plEventCallbackMsg::kLoop:  return kLoop;
        default:       return kStop;
    }
}
