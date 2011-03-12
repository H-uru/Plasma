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
// local includes
#include "plAvBrainDrive.h"
#include "plArmatureMod.h"
#include "plAvCallbackAction.h"

// global includes
#include "hsTimer.h"
#include "hsGeometry3.h"

// other includes
#include "hsQuat.h"
#include "../plMessage/plSimStateMsg.h"
#include "../pnMessage/plCameraMsg.h"

// messages
#include "../plMessage/plInputEventMsg.h"

// CTOR default
plAvBrainDrive::plAvBrainDrive()
: fMaxVelocity(20), fTurnRate(1)
{

}

// CTOR max velocity, turn rate
plAvBrainDrive::plAvBrainDrive(hsScalar maxVelocity, hsScalar turnRate)
: fMaxVelocity(maxVelocity), fTurnRate(turnRate)
{
}

// ACTIVATE
void plAvBrainDrive::Activate(plArmatureModBase *avMod)
{
	plArmatureBrain::Activate(avMod);

	IEnablePhysics(false, avMod->GetTarget(0)->GetKey());
	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
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
		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
		pMsg->SetCmd(plCameraMsg::kNonPhysOff);
		pMsg->SetBCastFlag(plMessage::kBCastByExactType);
		pMsg->Send();				
	}
}

void plAvBrainDrive::IEnablePhysics(bool enable, plKey avKey)
{
	fAvMod->EnablePhysics(enable);
}

// APPLY
hsBool plAvBrainDrive::Apply(double timeNow, hsScalar elapsed)
{
	plSceneObject * avSO = fAvMod->GetTarget(0);
	hsScalar eTime = hsTimer::GetDelSysSeconds();
	hsMatrix44 targetMatrix = avSO->GetLocalToWorld();

	hsPoint3 playerPos = targetMatrix.GetTranslate();
	hsVector3 view, up, right;
	targetMatrix.GetAxis(&view, &up, &right);
	hsScalar speed = fMaxVelocity;
	hsScalar turn = fTurnRate;

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

	hsVector3 rotUp(0,0,1);
	hsVector3 rotRight(1,0,0);
	hsMatrix44 rot;
	hsScalar angle = 0;

	if ( fAvMod->GetInputFlag( B_CONTROL_ROTATE_RIGHT ) || fAvMod->GetInputFlag( B_CONTROL_ROTATE_LEFT ) || fAvMod->GetInputFlag( A_CONTROL_TURN ) )
	{
		angle = fTurnRate * eTime * fAvMod->GetTurnStrength();
	}

	hsMatrix44 justRot(targetMatrix);
	hsPoint3 zero(0,0,0);
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
hsBool plAvBrainDrive::MsgReceive(plMessage* msg)
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

