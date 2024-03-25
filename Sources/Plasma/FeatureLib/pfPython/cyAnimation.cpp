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
/////////////////////////////////////////////////////////////////////////////
//
// NAME: cyAnimation
//
// PURPOSE: Class wrapper to map animation functions to plasma2 message
//

#include "cyAnimation.h"

#include "plgDispatch.h"

#include "plMessage/plAnimCmdMsg.h"

#include "pyKey.h"

cyAnimation::cyAnimation()
    : fAnimName(), fNetForce()
{
}

cyAnimation::cyAnimation(const pyKey& sender)
    : fAnimName(), fNetForce()
{
    SetSender(sender);
}

// setters
void cyAnimation::SetSender(const pyKey& sender)
{
    fSender = sender.getKey();
}

void cyAnimation::AddRecvr(const pyKey& recvr)
{
    fRecvr.emplace_back(recvr.getKey());
}

PyObject* cyAnimation::GetFirstRecvr()
{
    if (!fRecvr.empty())
        return pyKey::New(fRecvr[0]);
    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Play
//  PARAMETERS : 
//
//  PURPOSE    : Play animation from start to end (whatever is already set)
//
void cyAnimation::Play()
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);

        // NOTE: The animation modifier will set the animation back to the starting point automatically

        // then continue from there
        pMsg->SetCmd(plAnimCmdMsg::kGoToBegin);
        pMsg->SetCmd(plAnimCmdMsg::kContinue);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Stop
//  PARAMETERS : 
//
//  PURPOSE    : Stop an animation
//
void cyAnimation::Stop()
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kStop);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Resume
//  PARAMETERS : 
//
//  PURPOSE    : Continue playing animation from wherever it last stopped
//
void cyAnimation::Resume()
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kContinue);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PlayRange
//  PARAMETERS : start   - start time for the range
//             : end     - end time of the range
//
//  PURPOSE    : Play an animation only from specific time start to end
//
void cyAnimation::PlayRange(float start, float end)
{
    SkipToTime(start);
    PlayToTime(end);
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PlayToTime
//  PARAMETERS : time     - where to stop playing animation
//
//  PURPOSE    : Play (continue) an animation until the specified time is reached
//
void cyAnimation::PlayToTime(float time)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kPlayToTime);
        pMsg->fTime = time;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : PlayToPercentage
//  PARAMETERS : zeroToOne    - How far (scale of 0 to 1) to play into the anim
//
//  PURPOSE    : Play (continue) an animation until the specified point is reached
//
void cyAnimation::PlayToPercentage(float zeroToOne)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kPlayToPercentage);
        pMsg->fTime = zeroToOne;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SkipToTime
//  PARAMETERS : 
//
//  PURPOSE    : Jump the animation to the specified time
//             : Doesn't start or stop playing of animation
//
void cyAnimation::SkipToTime(float time)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kGoToTime);
        pMsg->fTime = time;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Looped
//  PARAMETERS : looped    - state to change to
//
//  PURPOSE    : Set whether the animation is to be looped or not
//
void cyAnimation::Looped(bool looped)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        if ( looped )
            pMsg->SetCmd(plAnimCmdMsg::kSetLooping);
        else
            pMsg->SetCmd(plAnimCmdMsg::kUnSetLooping);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Backwards
//  PARAMETERS : backwards   - state of the backwards flag
//
//  PURPOSE    : Sets the backwards state for the animation
//
void cyAnimation::Backwards(bool backwards)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        if ( backwards )
            pMsg->SetCmd(plAnimCmdMsg::kSetBackwards);
        else
            pMsg->SetCmd(plAnimCmdMsg::kSetForewards);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : SetLoopStart and SetLoopEnd
//  PARAMETERS : value  - sets the start or the end of the animation
//
void cyAnimation::SetLoopStart(float start)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kSetLoopBegin);
        pMsg->fLoopBegin = start;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void cyAnimation::SetLoopEnd(float end)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kSetLoopEnd);
        pMsg->fLoopEnd = end;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}



/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Speed
//  PARAMETERS : speed   - speed to set the animation to
//
//  PURPOSE    : Sets the speed of the animation
//             : Doesn't start or stop playing animation
//
void cyAnimation::Speed(float speed)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(plAnimCmdMsg::kSetSpeed);
        pMsg->fSpeed = speed;
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}


void cyAnimation::IRunOneCmd(int cmd)
{
    // must have a receiver!
    if (!fRecvr.empty())
    {
        // create message
        plAnimCmdMsg* pMsg = new plAnimCmdMsg;
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
        for (const plKey& rcKey : fRecvr)
            pMsg->AddReceiver(rcKey);

        // set the notetrack name (if there is one)
        if (fAnimName != nullptr)
            pMsg->SetAnimName(fAnimName);
        pMsg->SetCmd(cmd);
        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}

void cyAnimation::SkipToBegin()
{
    IRunOneCmd(plAnimCmdMsg::kGoToBegin);
}

void cyAnimation::SkipToEnd()
{
    IRunOneCmd(plAnimCmdMsg::kGoToEnd);
}

void cyAnimation::SkipToLoopBegin()
{
    IRunOneCmd(plAnimCmdMsg::kGoToLoopBegin);
}

void cyAnimation::SkipToLoopEnd()
{
    IRunOneCmd(plAnimCmdMsg::kGoToLoopEnd);
}

//  Bump the animation ahead one frame (whatever deltime is)
//
void cyAnimation::IncrementForward()
{
    IRunOneCmd(plAnimCmdMsg::kIncrementForward);
}

//  Bump the animation back one frame (whatever deltime is)
//
void cyAnimation::IncrementBackward()
{
    IRunOneCmd(plAnimCmdMsg::kIncrementBackward);
}
