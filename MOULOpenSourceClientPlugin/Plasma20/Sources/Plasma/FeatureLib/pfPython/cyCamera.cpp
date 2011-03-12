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
#include "cyCamera.h"

#include "../pnMessage/plCameraMsg.h"
#include "../plMessage/plInputEventMsg.h"
#include "../pnKeyedObject/plFixedKey.h"
#include "../pnKeyedObject/plUoid.h"
#include "hsResMgr.h"
#include "pyKey.h"
#include "plgDispatch.h"

#include "../pfCamera/plVirtualCamNeu.h"
#include "../pfCamera/plCameraModifier.h"
#include "../pfCamera/plCameraBrain.h"

cyCamera::cyCamera()
{
	// get _the_ virtual camera
	plUoid pU( kVirtualCamera1_KEY );
	hsResMgr* hrm = hsgResMgr::ResMgr();
	if ( hrm)
		fTheCam = hrm->FindKey( pU );
	else
		fTheCam = nil;
}

// setters
void cyCamera::SetSender(plKey &sender)
{
	fSender = sender;
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Push
//  PARAMETERS :
//
//  PURPOSE    : Save the current state of the virtual camera
//
// NOTE: doesn't work by itself at the moment
//
void cyCamera::Push(pyKey& newCamKey)
{
	// create message
	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
	if ( fSender )
		pMsg->SetSender(fSender);

	// if we're sending to the virtual camera
	if ( fTheCam )
		pMsg->AddReceiver(fTheCam);
	else
		// otherwise, broadcast by type
		pMsg->SetBCastFlag(plMessage::kBCastByType);

	// set command to do the transition
	pMsg->SetCmd(plCameraMsg::kResponderTrigger);
	pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
	// set the new camera
	pMsg->SetNewCam(newCamKey.getKey());

	plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : Pop
//  PARAMETERS :
//
//  PURPOSE    : Restore the state of the virtual camera with a previously saved setting
//
void cyCamera::Pop(pyKey& oldCamKey)
{
	// create message
	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
	if ( fSender )
		pMsg->SetSender(fSender);

	// if we're sending to the virtual camera
	if ( fTheCam )
		pMsg->AddReceiver(fTheCam);
	else
		// otherwise, broadcast by type
		pMsg->SetBCastFlag(plMessage::kBCastByType);

	// set command to undo the camera... somehow not saying ResponderTrigger but Push means Pop...whatever
	pMsg->SetCmd(plCameraMsg::kRegionPushCamera);
	// set the new camera
	pMsg->SetNewCam(oldCamKey.getKey());

	plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ControlKey
//  PARAMETERS : controlKey  - what command key to simulate being hit
//             : activate    - whether its being pressed or released (activated or deactivated)
//
//  PURPOSE    : Send controlKey commands to the virtual camera (should be like a pass thru)
//
void cyCamera::ControlKey(Int32 controlKey, hsBool activated)
{
	// make sure that we have a virtual camera to send this to
	if ( fTheCam )
	{
		plControlEventMsg* pMsg = TRACKED_NEW plControlEventMsg;
		// set sender if there is one
		if ( fSender )
			pMsg->SetSender(fSender);

		// if we're sending to the virtual camera
		pMsg->AddReceiver(fTheCam);

		// set the control key and activateFlag
		pMsg->SetControlCode((ControlEventCode)controlKey);
		pMsg->SetControlActivated(activated);

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


/////////////////////////////////////////////////////////////////////////////
//
//  Function   : TransitionTo
//  PARAMETERS : newCamKey  - what to switch the camera to
//             : time       - how long it takes to transition to new camera
//
//  PURPOSE    : Transition to a new camera (position and settings)
//
void cyCamera::TransitionTo(pyKey& newCamKey, double time, hsBool save)
{
	// create message
	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
	if ( fSender )
		pMsg->SetSender(fSender);
	// must have a receiver!
	if ( fTheCam )
	{
		pMsg->AddReceiver(fTheCam);
		// set command to do the transition
		pMsg->SetCmd(plCameraMsg::kTransitionTo);
		// set the new camera
		pMsg->SetNewCam(newCamKey.getKey());
		// set the transition time
		pMsg->SetTransTime(time);
		// test to see if they want to save
		if ( save )
			pMsg->SetCmd(plCameraMsg::kPush);

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}

void cyCamera::SetEnableFirstPersonOverride(hsBool state)
{
	// must have a receiver!
	if ( fTheCam )
	{
		// create message
		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
		if ( fSender )
			pMsg->SetSender(fSender);

		pMsg->AddReceiver(fTheCam);
		// set command to do the transition
		pMsg->SetCmd(plCameraMsg::kPythonSetFirstPersonOverrideEnable);
		// set the state
		pMsg->SetActivated(state);

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


void cyCamera::UndoFirstPerson()
{
	// create message
	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
	if ( fSender )
		pMsg->SetSender(fSender);
	// must have a receiver!
	if ( fTheCam )
	{
		pMsg->AddReceiver(fTheCam);
		// set command to do the transition
		pMsg->SetCmd(plCameraMsg::kPythonUndoFirstPerson);

		plgDispatch::MsgSend( pMsg );	// whoosh... off it goes
	}
}


hsScalar cyCamera::GetFOV()
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			plCameraModifier1* curCam = virtCam->GetCurrentCamera();
			if ( curCam )
			{
				return curCam->GetFOVh();
			}
		}
	}
	return 0.0;
}

void cyCamera::SetFOV(hsScalar fov, double t)
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			plCameraModifier1* curCam = virtCam->GetCurrentCamera();
			if ( curCam )
			{
				plCameraBrain1*	camBrain = curCam->GetBrain();
				if (camBrain)
				{
					camBrain->SetFOVGoal(fov,t);
				}
			}
		}
	}
}


void cyCamera::SetSmootherCam(hsBool state)
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			if (state)
			{
				virtCam->fUseAccelOverride = false;
			}
			else
			{
				virtCam->fAccel = 50.0;
				virtCam->fDecel = 50.0;
				virtCam->fVel = 100.0;
				virtCam->fUseAccelOverride = true;
			}

		}
	}
}

hsBool cyCamera::IsSmootherCam()
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			if ( virtCam->fUseAccelOverride )
				return false;
			else
				return true;
		}

	}
	return false;
}

void cyCamera::SetWalkAndVerticalPan(hsBool state)
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			if (state)
				virtCam->WalkPan3rdPerson = true;
			else
				virtCam->WalkPan3rdPerson = false;

		}
	}
}


hsBool cyCamera::IsWalkAndVerticalPan()
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			return virtCam->WalkPan3rdPerson;
		}
	}
	return false;
}


void cyCamera::SetStayInFirstPerson(hsBool state)
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			if (state)
				virtCam->StayInFirstPersonForever = true;
			else
				virtCam->StayInFirstPersonForever = false;
		}
	}
}

hsBool cyCamera::IsStayInFirstPerson()
{
	if ( fTheCam )
	{
		plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
		if ( virtCam )
		{
			return virtCam->StayInFirstPersonForever;
		}
	}
	return false;
}

void cyCamera::SetAspectRatio(float aspectratio)
{
	plVirtualCam1::SetAspectRatio(aspectratio);
}

float cyCamera::GetAspectRatio()
{
	return plVirtualCam1::GetAspectRatio();
}

void cyCamera::RefreshFOV()
{
	plVirtualCam1::SetFOV(plVirtualCam1::GetFOVw(), plVirtualCam1::GetFOVh());
}