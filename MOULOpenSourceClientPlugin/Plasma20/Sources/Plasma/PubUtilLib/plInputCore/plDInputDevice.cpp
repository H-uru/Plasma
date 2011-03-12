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
// plDInputDevice.cpp

#include "hsConfig.h"
#include "hsWindows.h"

#include "plDInputDevice.h"
#include "plgDispatch.h"
#include "../plMessage/plInputEventMsg.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
//
//
//
// plDInputDevice
//
//

plDInputDevice::plDInputDevice() :
fX(0.5),
fY(0.5)
{
}

plDInputDevice::~plDInputDevice()
{
}

void plDInputDevice::Update(DIDEVICEOBJECTDATA* js)
{
	switch(js->uAppData)
    {
        case A_CONTROL_MOVE:
			{
				int i = (int)(js->dwData);
				if (i <= -1)
				{	
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlCode( B_CONTROL_MOVE_FORWARD );
					pMsg->SetControlActivated( true );
					plgDispatch::MsgSend( pMsg );
				}
				else
				if (i >= 1)
				{	
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlCode( B_CONTROL_MOVE_BACKWARD );
					pMsg->SetControlActivated( true );
					plgDispatch::MsgSend( pMsg );
				}
				else
				if (i == 0)
				{
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlCode( B_CONTROL_MOVE_BACKWARD );
					pMsg->SetControlActivated( false );
					plgDispatch::MsgSend( pMsg );
					plControlEventMsg* pMsg2 = TRACKED_NEW plControlEventMsg;
					pMsg2->SetControlCode( B_CONTROL_MOVE_FORWARD );
					pMsg2->SetControlActivated( false );
					plgDispatch::MsgSend( pMsg2 );
				}
					
			}
			break;
        case A_CONTROL_TURN:
			{
				int i = (int)(js->dwData);
				float f = ((float)i) * 0.001f;
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( A_CONTROL_TURN );
				if (f <= 0.02 && f >= -0.02)
					pMsg->SetControlActivated( false );
				else
					pMsg->SetControlActivated( true );
				pMsg->SetControlPct(f);
				plgDispatch::MsgSend( pMsg );
			}
			break;
		case B_CONTROL_ACTION:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_ACTION );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);	
			}
			break;
		case B_CONTROL_MODIFIER_FAST:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_MODIFIER_FAST );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_JUMP:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_JUMP );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_STRAFE_LEFT:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_STRAFE_LEFT );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_STRAFE_RIGHT:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_STRAFE_RIGHT);
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_EQUIP:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_EQUIP );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_DROP:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_DROP );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_MOVE_FORWARD:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_MOVE_FORWARD );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_MOVE_BACKWARD:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_MOVE_BACKWARD );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_ROTATE_RIGHT:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_ROTATE_RIGHT);
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_ROTATE_LEFT:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_ROTATE_LEFT );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CONTROL_TURN_TO:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CONTROL_TURN_TO );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		case B_CAMERA_RECENTER:
			{	
				plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
				pMsg->SetControlCode( B_CAMERA_RECENTER );
				pMsg->SetControlActivated(js->dwData & 0x80);
				plgDispatch::MsgSend(pMsg);
			}
			break;
		
		case A_CONTROL_MOUSE_X:
			{
				int i = (int)(js->dwData);
				float f = ((float)i) * 0.001f;
				if (f <= 0.02 && f >= -0.02)
				{
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlActivated( false );
					pMsg->SetControlCode(B_CAMERA_ROTATE_DOWN);
					pMsg->SetControlPct(0);
					plgDispatch::MsgSend( pMsg );
					plControlEventMsg* pMsg2 = TRACKED_NEW plControlEventMsg;
					pMsg2->SetControlActivated( false );
					pMsg2->SetControlCode(B_CAMERA_ROTATE_UP);
					pMsg2->SetControlPct(0);
					plgDispatch::MsgSend( pMsg2 );
				}	
				else
				{
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlActivated( true );
					if (f < 0)
						pMsg->SetControlCode(B_CAMERA_ROTATE_DOWN);
					else 
						pMsg->SetControlCode(B_CAMERA_ROTATE_UP);
					
					pMsg->SetControlPct(f);
					plgDispatch::MsgSend( pMsg );
				}
			}
			break;
		case A_CONTROL_MOUSE_Y:
			{
				int i = (int)(js->dwData);
				float f = ((float)i) * 0.001f;
				if (f <= 0.02 && f >= -0.02)
				{
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlActivated( false );
					pMsg->SetControlCode(B_CAMERA_ROTATE_RIGHT);
					pMsg->SetControlPct(0);
					plgDispatch::MsgSend( pMsg );
					plControlEventMsg* pMsg2 = TRACKED_NEW plControlEventMsg;
					pMsg2->SetControlActivated( false );
					pMsg2->SetControlCode(B_CAMERA_ROTATE_LEFT);
					pMsg2->SetControlPct(0);
					plgDispatch::MsgSend( pMsg2 );
				}	
				else
				{
					plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
					pMsg->SetControlActivated( true );
					if (f < 0)
						pMsg->SetControlCode(B_CAMERA_ROTATE_RIGHT);
					else 
						pMsg->SetControlCode(B_CAMERA_ROTATE_LEFT);
					
					pMsg->SetControlPct(f);
					plgDispatch::MsgSend( pMsg );
				}
			}
			break;
        
		default:
            break;
    }

}

