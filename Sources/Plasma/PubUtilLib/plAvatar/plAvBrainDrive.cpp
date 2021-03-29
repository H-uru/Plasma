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
// local includes
#include "plAvBrainDrive.h"
#include "plArmatureMod.h"

// global includes
#include "hsGeometry3.h"
#include "hsMatrix44.h"
#include "hsQuat.h"
#include "hsTimer.h"

// other includes
#include "pnMessage/plCameraMsg.h"
#include "pnMessage/plCmdIfaceModMsg.h"
#include "pnSceneObject/plSceneObject.h"

#include "plMessage/plInputEventMsg.h"
#include "plMessage/plSimStateMsg.h"

// CTOR default
plAvBrainDrive::plAvBrainDrive()
: fMaxVelocity(20), fTurnRate(1)
{

}

// CTOR max velocity, turn rate
plAvBrainDrive::plAvBrainDrive(float maxVelocity, float turnRate)
: fMaxVelocity(maxVelocity), fTurnRate(turnRate)
{
}

// ACTIVATE
void plAvBrainDrive::Activate(plArmatureModBase *avMod)
{
    plArmatureBrain::Activate(avMod);

    IEnablePhysics(false, avMod->GetTarget(0)->GetKey());
    IToggleCtrlCodes(true);

    plCameraMsg* pMsg = new plCameraMsg;
    pMsg->SetCmd(plCameraMsg::kNonPhysOn);
    pMsg->SetBCastFlag(plMessage::kBCastByExactType);
    pMsg->Send();               
}

// DEACTIVATE
void plAvBrainDrive::Deactivate()
{
    if (fAvMod)
    {   
        IEnablePhysics(true, fAvMod->GetTarget(0)->GetKey());
        IToggleCtrlCodes(false);

        plCameraMsg* pMsg = new plCameraMsg;
        pMsg->SetCmd(plCameraMsg::kNonPhysOff);
        pMsg->SetBCastFlag(plMessage::kBCastByExactType);
        pMsg->Send();
    }
}

void plAvBrainDrive::IToggleCtrlCodes(bool on) const
{
    if (fAvMod->IsLocalAvatar()) {
        plCmdIfaceModMsg* pUpMsg = new plCmdIfaceModMsg;
        pUpMsg->fCmd.SetBit(on ? plCmdIfaceModMsg::kEnableControlCode : plCmdIfaceModMsg::kDisableControlCode);
        pUpMsg->fControlCode = B_CONTROL_MOVE_UP;
        pUpMsg->Send();

        plCmdIfaceModMsg* pDownMsg = new plCmdIfaceModMsg;
        pDownMsg->fCmd.SetBit(on ? plCmdIfaceModMsg::kEnableControlCode : plCmdIfaceModMsg::kDisableControlCode);
        pDownMsg->fControlCode = B_CONTROL_MOVE_DOWN;
        pDownMsg->Send();
    }
}

void plAvBrainDrive::IEnablePhysics(bool enable, plKey avKey)
{
    fAvMod->EnablePhysics(enable);
}

// APPLY
bool plAvBrainDrive::Apply(double timeNow, float elapsed)
{
    plSceneObject * avSO = fAvMod->GetTarget(0);
    float eTime = hsTimer::GetDelSysSeconds();
    hsMatrix44 targetMatrix = avSO->GetLocalToWorld();

    hsPoint3 playerPos = targetMatrix.GetTranslate();
    hsVector3 view, up, right;
    targetMatrix.GetAxis(&view, &up, &right);
    float speed = fMaxVelocity;
    float turn = fTurnRate;

    if (fAvMod->FastKeyDown())
    {
        turn *= 0.25;
        speed *= 3.5;
    }
    if (fAvMod->GetInputFlag(B_CONTROL_MOVE_FORWARD))
    {
        playerPos += view * speed * eTime;
    }
    if (fAvMod->GetInputFlag(B_CONTROL_MOVE_BACKWARD))
    {
        playerPos += view * speed * eTime * -1;
    }
    if (fAvMod->StrafeLeftKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnLeftKeyDown()))
    {
        playerPos += right * speed * eTime * -1;
    }
    if (fAvMod->StrafeRightKeyDown() || (fAvMod->StrafeKeyDown() && fAvMod->TurnRightKeyDown()))
    {
        playerPos += right * speed * eTime;
    }
    if (fAvMod->GetInputFlag(B_CONTROL_MOVE_DOWN))
    {
        playerPos += up * speed * eTime * -1;
    }
    if (fAvMod->GetInputFlag(B_CONTROL_MOVE_UP))
    {
        playerPos += up * speed * eTime;
    }

    hsPoint3 desiredPosition = playerPos;
    // calculate rotation matrix

    hsVector3 rotUp(0.f, 0.f, 1.f);
    hsVector3 rotRight(1.f, 0.f, 0.f);
    hsMatrix44 rot;
    float angle = 0;

    if ( fAvMod->GetInputFlag( B_CONTROL_ROTATE_RIGHT ) || fAvMod->GetInputFlag( B_CONTROL_ROTATE_LEFT ) || fAvMod->GetInputFlag( A_CONTROL_TURN ) )
    {
        angle = fTurnRate * eTime * fAvMod->GetTurnStrength();
    }

    hsMatrix44 justRot(targetMatrix);
    hsPoint3 zero;
    justRot.SetTranslate(&zero);

    if( angle ) {
        hsQuat q( angle, &rotUp );  
        q.NormalizeIfNeeded();
        q.MakeMatrix( &rot );

        justRot = rot * justRot;

        targetMatrix = rot * targetMatrix;
    }

    // use the desired rotation matrix to set position and rotation:
    hsMatrix44 inv;
    targetMatrix.SetTranslate( &desiredPosition );
    targetMatrix.GetInverse( &inv );
    avSO->SetTransform( targetMatrix, inv );
    avSO->FlushTransform();

    return true;
}

// IHANDLECONTROLMSG
bool plAvBrainDrive::MsgReceive(plMessage* msg)
{
    plControlEventMsg *ctrlMsg = plControlEventMsg::ConvertNoRef(msg);
    if(ctrlMsg)
    {
        if( ctrlMsg->ControlActivated() )
        {
            if(ctrlMsg->GetControlCode() == B_CONTROL_TOGGLE_PHYSICAL)
            {
                fAvMod->PopBrain();
                return true;
            }
        }
    }
    return false;
}

