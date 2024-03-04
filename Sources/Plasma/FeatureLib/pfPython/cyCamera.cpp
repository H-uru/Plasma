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

#include "cyCamera.h"

#include "hsResMgr.h"
#include "plgDispatch.h"

#include "pnKeyedObject/plFixedKey.h"
#include "pnKeyedObject/plUoid.h"
#include "pnMessage/plCameraMsg.h"

#include "plMessage/plInputEventMsg.h"

#include "pfCamera/plCameraBrain.h"
#include "pfCamera/plCameraModifier.h"
#include "pfCamera/plVirtualCamNeu.h"

#include "pyKey.h"

cyCamera::cyCamera()
{
    // get _the_ virtual camera
    plUoid pU( kVirtualCamera1_KEY );
    hsResMgr* hrm = hsgResMgr::ResMgr();
    if ( hrm)
        fTheCam = hrm->FindKey( pU );
    else
        fTheCam = nullptr;
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
    plCameraMsg* pMsg = new plCameraMsg;
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

    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
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
    plCameraMsg* pMsg = new plCameraMsg;
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

    plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
}

/////////////////////////////////////////////////////////////////////////////
//
//  Function   : ControlKey
//  PARAMETERS : controlKey  - what command key to simulate being hit
//             : activate    - whether its being pressed or released (activated or deactivated)
//
//  PURPOSE    : Send controlKey commands to the virtual camera (should be like a pass thru)
//
void cyCamera::ControlKey(int32_t controlKey, bool activated)
{
    // make sure that we have a virtual camera to send this to
    if ( fTheCam )
    {
        plControlEventMsg* pMsg = new plControlEventMsg;
        // set sender if there is one
        if ( fSender )
            pMsg->SetSender(fSender);

        // if we're sending to the virtual camera
        pMsg->AddReceiver(fTheCam);

        // set the control key and activateFlag
        pMsg->SetControlCode((ControlEventCode)controlKey);
        pMsg->SetControlActivated(activated);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
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
void cyCamera::TransitionTo(pyKey& newCamKey, double time, bool save)
{
    if (fTheCam) {
        plCameraMsg* pMsg = new plCameraMsg;
        if (fSender)
            pMsg->SetSender(fSender);
        pMsg->SetCmd(plCameraMsg::kTransitionTo);
        pMsg->SetNewCam(newCamKey.getKey());
        pMsg->SetTransTime(time);
        if (save)
            pMsg->SetCmd(plCameraMsg::kPush);
        pMsg->Send(fTheCam);
    }
}

void cyCamera::SetEnableFirstPersonOverride(bool state) const
{
    // must have a receiver!
    if ( fTheCam )
    {
        // create message
        plCameraMsg* pMsg = new plCameraMsg;
        if ( fSender )
            pMsg->SetSender(fSender);

        pMsg->AddReceiver(fTheCam);
        // set command to do the transition
        pMsg->SetCmd(plCameraMsg::kPythonSetFirstPersonOverrideEnable);
        // set the state
        pMsg->SetActivated(state);

        plgDispatch::MsgSend( pMsg );   // whoosh... off it goes
    }
}


void cyCamera::UndoFirstPerson()
{
    if (fTheCam) {
        plCameraMsg* pMsg = new plCameraMsg;
        if (fSender)
            pMsg->SetSender(fSender);
        pMsg->SetCmd(plCameraMsg::kPythonUndoFirstPerson);
        pMsg->Send(fTheCam);
    }
}


float cyCamera::GetFOV()
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

void cyCamera::SetFOV(float fov, double t)
{
    if ( fTheCam )
    {
        plVirtualCam1* virtCam = plVirtualCam1::ConvertNoRef( fTheCam->ObjectIsLoaded() );
        if ( virtCam )
        {
            plCameraModifier1* curCam = virtCam->GetCurrentCamera();
            if ( curCam )
            {
                plCameraBrain1* camBrain = curCam->GetBrain();
                if (camBrain)
                {
                    camBrain->SetFOVGoal(0.f, fov, t);
                }
            }
        }
    }
}


void cyCamera::SetSmootherCam(bool state)
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

bool cyCamera::IsSmootherCam()
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

void cyCamera::SetWalkAndVerticalPan(bool state)
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


bool cyCamera::IsWalkAndVerticalPan()
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


void cyCamera::SetStayInFirstPerson(bool state)
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

bool cyCamera::IsStayInFirstPerson()
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

void cyCamera::RefreshFOV()
{
    plCameraMsg* pMsg = new plCameraMsg();
    pMsg->SetSender(fSender);
    pMsg->SetCmd(plCameraMsg::kRefreshFOV);
    pMsg->Send(fTheCam);
}
