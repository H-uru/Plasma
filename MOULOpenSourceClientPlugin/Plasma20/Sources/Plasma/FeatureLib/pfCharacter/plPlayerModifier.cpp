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
//#pragma warning(disable: 4503 4786)  
//#define HK_HARDCORE
//
//#include <hkmath/vector3.h>			// for havok Vector3
////#include <.//gpi/math/quaternion.h>		// for havok Vector3
//#include <hkgeometry/geomdef.h>	// for havok Vertex
//
//
//#include "hsTypes.h"
//#include "../plInterp/plController.h"
//#include "plPlayerModifier.h"
//#include "hsTimer.h"
//#include "../pnSceneObject/plSceneObject.h"
//#include "../pnSceneObject/plSimulationInterface.h"
//#include "../pnInputCore/plControlEventCodes.h"
//#include "../pnMessage/plTimeMsg.h"
//#include "../pnMessage/plWarpMsg.h"
//#include "../pnMessage/plCameraMsg.h"
//#include "../pnSceneObject/plCoordinateInterface.h"
//#include "plgDispatch.h"
//#include "../pfCamera/plCameraModifier.h"
//#include "hsResMgr.h"
//#include "../pnKeyedObject/plKey.h"
//#include "../plNetClient/plNetClientMgr.h"
//#include "../plModifier/plSpawnModifier.h"
//#include "../plMessage/plMatrixUpdateMsg.h"
//
//#include "../pnTimer/plTimerCallbackManager.h"
//#include "../plAudio/plAudioSystem.h"
//#include "../plMessage/plInputEventMsg.h"
//#include "../plMessage/plSpawnRequestMsg.h"
//#include "../plMessage/plSpawnModMsg.h"
//#include "../plMessage/plPlayerMsg.h"
//#include "../pnMessage/plAudioSysMsg.h"
//#include "../pfCamera/plCameraBrain.h"
//
//#include "../plHavok1/plHKPhysical.h"
//
//hsScalar plPlayerModifier::fTurnRate = 1.0f;
//hsScalar plPlayerModifier::fAcceleration = 80.0f;
//hsScalar plPlayerModifier::fDeceleration = 80.0f;
//hsScalar plPlayerModifier::fMaxVelocity = 200.0f;
//
//plPlayerModifier::plPlayerModifier() :
//bUseDesiredFacing(false),
//bUseDesiredMatrix(false)
//{
//	fCurSpeed = 0.0f;
//	fLastTime = 0.0;
//	bMoving	= false;
//	fRotationScalar = 1.0f;
//	bIgnoreDesiredMatrix = false;
//	SetFlag( kWantsToSpawn );
//}
//
//plPlayerModifier::~plPlayerModifier()
//{
//	for (int i = 0; i < fSpawnPoints.Count(); i++)
//		delete fSpawnPoints[i];
//	fSpawnPoints.SetCount(0);
//}
//
//// Adding RemoveTarget override of plSingleModifier to tell everyone we
//// told in AddTarget about our subject that he's gone now.
//void plPlayerModifier::RemoveTarget(plSceneObject* so)
//{
//	if( fTarget && fTarget->IsLocallyOwned()==plSynchedObject::kYes )
//	{
//		plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
//		pMsg->SetCmd(plCameraMsg::kSetSubject);
//		pMsg->SetSubject(nil);
//		pMsg->SetBCastFlag( plMessage::kBCastByExactType );
//		plgDispatch::MsgSend(pMsg);
//
//		plAudioSysMsg* pAudMsg1 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerCoordinateRefCamera);
//		plAudioSysMsg* pAudMsg2 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerVelocityRefCamera);
//		plAudioSysMsg* pAudMsg3 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerFacingRefCamera);
//		plgDispatch::MsgSend(pAudMsg1);
//		plgDispatch::MsgSend(pAudMsg2);
//		plgDispatch::MsgSend(pAudMsg3);
//	}
//	plSingleModifier::RemoveTarget(so);
//}
//
//void plPlayerModifier::AddTarget(plSceneObject* so)
//{
//	fTarget = so;
//	plSimulationInterface * pSI = IGetTargetSimulationInterface(0); // so->GetSimulationInterface(); // 
//	 
//	plgDispatch::Dispatch()->RegisterForExactType(plEvalMsg::Index(), GetKey());
//
//	// set the desired rotation vector...
////	hsAssert(fTarget->GetCoordinateInterface(), "Player modifier target has no coordinate interface");
//
//	// These are now set in the component
////	if(pSI) 
////	{
////		pSI->SetProperty(plSimulationInterface::kAffectLOS, false);
////		pSI->SetProperty(kUpright, true);
////	}
//	
//	//
//	// setup for local player if necessary
//	//
//	int locallyOwned=so->IsLocallyOwned();
//	if (locallyOwned==plSynchedObject::kMaybe)		// don't know since we're still loading, defer
//		SetFlag(kNeedsLocalSetup);
//	else if (locallyOwned==plSynchedObject::kYes)
//		IDoLocalSetup(so);	
//}
//
//void plPlayerModifier::IDoLocalSetup(plSceneObject* so)
//{
//	plCameraMsg* pMsg = TRACKED_NEW plCameraMsg;
//	pMsg->SetCmd(plCameraMsg::kSetSubject);
//	pMsg->SetSubject(so);
//	pMsg->SetBCastFlag( plMessage::kBCastByExactType );
//	plgDispatch::MsgSend(pMsg);
//
//	// this is to solve the problem of physical vs nonphysical players...
////	plCameraMsg* pMsg2 = TRACKED_NEW plCameraMsg;
////	pMsg2->SetBCastFlag(plMessage::kBCastByExactType);
////	pMsg2->SetCmd(plCameraMsg::kSetOffset);
////	pMsg2->SetCmd(plCameraMsg::kEntering);
////	pMsg2->SetTriggerer(so->GetKey());
////	pMsg2->SetOffsetY(50);
////	pMsg2->SetOffsetZ(10);
////	plgDispatch::MsgSend(pMsg2);
//}
//
//void	plPlayerModifier::IMakeUsListener( plSceneObject *so )
//{
//	// set the listener to use us...
//	plAudioSysMsg* pAudMsg1 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerFacingRef);
//	pAudMsg1->SetSceneObject(so->GetKey());
//	plAudioSysMsg* pAudMsg2 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerCoordinateRef);
//	pAudMsg2->SetSceneObject(so->GetKey());
//	plAudioSysMsg* pAudMsg3 = TRACKED_NEW plAudioSysMsg(plAudioSysMsg::kSetListenerVelocityRef);
//	pAudMsg3->SetSceneObject(so->GetKey());
//	plgDispatch::MsgSend(pAudMsg1);
//	plgDispatch::MsgSend(pAudMsg2);
//	plgDispatch::MsgSend(pAudMsg3);
//
//	// Now that we have a valid listener, unmute the audio system
//	plgAudioSys::SetMuted( false );
//}
//
//hsBool plPlayerModifier::MsgReceive(plMessage* msg)
//{
//	plControlEventMsg* pCommandMsg = plControlEventMsg::ConvertNoRef(msg);
//	if (pCommandMsg)
//		return(HandleControlInput(pCommandMsg));
//	
//	plMatrixUpdateMsg* pMMsg = plMatrixUpdateMsg::ConvertNoRef( msg );
//	if (pMMsg && HasFlag(kHasSpawned))
//	{
//		hsAssert(GetTarget()->IsLocallyOwned()==plSynchedObject::kNo, "master objects should not get correction msgs");
//		fDesiredMatrix = pMMsg->fMatrix;
//		if (bIgnoreDesiredMatrix)
//			bIgnoreDesiredMatrix = false;
//		else
//		{
//			bUseDesiredMatrix = true;
//		}
//		return true;
//	}
//
//	plSpawnModMsg* pSpawn = plSpawnModMsg::ConvertNoRef(msg);
//	if (pSpawn && HasFlag(kWantsToSpawn))
//	{
//		spawnPt* pt = TRACKED_NEW spawnPt;
//		pt->pt = pSpawn->fPos;
//		
//		hsVector3 temp(fTarget->GetCoordinateInterface()->GetLocalToWorld().GetTranslate() - pt->pt);
//		pt->dist = temp.MagnitudeSquared();
//		fSpawnPoints.Append(pt);
//	}
//	plPlayerMsg* pPMsg = plPlayerMsg::ConvertNoRef(msg);
//	if (pPMsg)
//	{
//		if (pPMsg->Cmd(plPlayerMsg::kWarpToSpawnPoint))
//		{	
//			WarpToSpawnPoint();
//			return true;
//		}
//	}	
//	return plSingleModifier::MsgReceive(msg);
//}
//
//hsBool plPlayerModifier::HandleControlInput(plControlEventMsg* pMsg)
//{
//	hsBool ret=false;
//	
//	if (pMsg->ControlActivated() && (pMsg->GetControlCode() == B_CONTROL_ROTATE_RIGHT || pMsg->GetControlCode() == B_CONTROL_ROTATE_LEFT || pMsg->GetControlCode() == A_CONTROL_TURN))
//	{
//		fRotationScalar = pMsg->GetPct();
//		if ( HasMovementFlag( pMsg->GetControlCode() ) )
//			bIgnoreDesiredMatrix = true;
//	}
//	if (pMsg->ControlActivated() && !HasMovementFlag( pMsg->GetControlCode() ) )
//	{
//		SetMovementFlag( pMsg->GetControlCode() );
//		if ( pMsg->GetControlCode() == B_CONTROL_TURN_TO )
//		{
//			//fFacingTarget = pMsg->GetTurnToPt();
//		}
//	}
//	else
//	if ( !pMsg->ControlActivated() && HasMovementFlag( pMsg->GetControlCode() ) )
//	{
//		ClearMovementFlag( pMsg->GetControlCode() );
//	}
//
//	ret = true;
//	return ret;
//}
//
//void plPlayerModifier::SetMoving(hsBool b)
//{
//	if (b != bMoving)
//	{
//		plPlayerMsg* pMsg = TRACKED_NEW plPlayerMsg;
//		
//		if (b)
//			pMsg->SetCmd( plPlayerMsg::kMovementStarted );
//		else	
//			pMsg->SetCmd( plPlayerMsg::kMovementStopped );
//		
//		plgDispatch::MsgSend( pMsg );
//		bMoving = b;
//	}
//}
//
//
//hsPoint3 forceForward(0,-200,0);
//hsPoint3 forceRight(-200,0,0);
//hsPoint3 forceUp(0,0,15);
//
//hsBool plPlayerModifier::IEval(double secs, hsScalar del, UInt32 dirty)
//{
//	// setup for local player if necessary
//	if (HasFlag(kNeedsLocalSetup))
//	{
//		int locallyOwned=fTarget->IsLocallyOwned();
//		if (locallyOwned==plSynchedObject::kYes)
//			IDoLocalSetup(fTarget);
//		else
//		if (locallyOwned==plSynchedObject::kNo)
//			ClearFlag(kNeedsLocalSetup);
//	}
//	
//	if (HasFlag(kWantsToSpawn))
//	{
//		if (fTarget->IsLocallyOwned()==plSynchedObject::kNo)
//		{
//			// if our target is a proxy player, don't warp him to a spawn point;
//			// we will receive his location as a state update.
//			ClearFlag(kWantsToSpawn);
//		}
//		else
//		if (fSpawnPoints.Count()
//			// if MP game, make sure we're connected before spawning
//			&& (!plNetClientMgr::GetInstance()->IsEnabled() || 
//			  plNetClientMgr::GetInstance()->HasJoined()) 
//			)
//		{
//			int i;
//#if 0
//			for (i = 0; i < fSpawnPoints.Count(); i++)
//			{
//				for (int j = i + 1; j < fSpawnPoints.Count(); j++)
//				{
//					if (fSpawnPoints[j]->dist < fSpawnPoints[i]->dist)
//					{
//						spawnPt* pt;
//						pt = fSpawnPoints[j];
//						fSpawnPoints[j] = fSpawnPoints[i];
//						fSpawnPoints[i] = pt;
//					}
//				}
//			}
//			hsPoint3 warpPoint = fSpawnPoints[0]->pt;
//#else
//			// choose spawnPoint based on netID, not distance
//			int netID = plNetClientMgr::GetInstance()->GetClientNum();
//			if (netID==-1)
//				netID=0;
//			hsPoint3 warpPoint = netID>=fSpawnPoints.Count() ? 
//				fSpawnPoints[fSpawnPoints.Count()-1]->pt : fSpawnPoints[netID]->pt;
//#endif
//			// Send msg for net synchronization
//			plWarpMsg* warpMsg = TRACKED_NEW plWarpMsg;
//			warpMsg->fPos = warpPoint;
//			warpMsg->AddReceiver( fTarget->GetKey() );
//			warpMsg->SetWarpFlags(warpMsg->GetWarpFlags() | plWarpMsg::kFlushTransform | plWarpMsg::kZeroVelocity );
//			plgDispatch::MsgSend( warpMsg );
//#ifdef HS_DEBUGGING
//			char str[256];
//			sprintf(str, "%s has %d spawnPoints.  Using pt %f %f %f\n", 
//				GetKeyName(), fSpawnPoints.GetCount(), 
//				fSpawnPoints[0]->pt.fX,fSpawnPoints[0]->pt.fY,fSpawnPoints[0]->pt.fZ);
//			hsStatusMessage(str);
//#endif
//			for (i = 0; i < fSpawnPoints.Count(); i++)
//				delete fSpawnPoints[i];
//
//			fSpawnPoints.SetCount(0);
//			ClearFlag(kWantsToSpawn);
//		}
//		else 
//		{
//			plSpawnRequestMsg* pMsg = TRACKED_NEW plSpawnRequestMsg;
//			pMsg->SetSender(GetKey());
//			plgDispatch::MsgSend( pMsg );
//		}
//		bIgnoreDesiredMatrix = true;
//		return true;
//	}
//	else
//	{
//		if( !HasFlag( kHasSpawned ) )
//		{
//			// Don't make us listener until we have actually spawned
//			IMakeUsListener( fTarget );
//			SetFlag(kHasSpawned);
//		}
//	}
//	
//	if (!fTarget->GetCoordinateInterface())
//		return true;
//
//	// update our desired position:
////	hsScalar eTime = secs - fLastTime;
//	hsScalar eTime = hsTimer::GetDelSysSeconds();
//	
//	hsPoint3	newLinearForce(0,0,0);
//
//	hsMatrix44 targetMatrix;
//	if (bUseDesiredMatrix)
//		targetMatrix = fDesiredMatrix;
//	else
//		targetMatrix = fTarget->GetCoordinateInterface()->GetLocalToWorld();
//	hsPoint3 playerPos = targetMatrix.GetTranslate();
//	hsVector3 view, up, right;
//	targetMatrix.GetAxis(&view, &up, &right);
//	
//	hsScalar speed = fMaxVelocity;
//	hsScalar turn = fTurnRate;
//
//	if (HasMovementFlag(B_CONTROL_MODIFIER_FAST))
//	{
//		turn *= 0.25;
//		speed *= 3.5;
//	}
//	if (HasMovementFlag(B_CONTROL_MOVE_FORWARD))
//	{
//		playerPos += view * speed * eTime;
//		newLinearForce = newLinearForce + forceForward * speed * eTime;		// calc force for physics
//	}
//	if (HasMovementFlag(B_CONTROL_MOVE_BACKWARD))
//	{
//		playerPos += view * speed * eTime * -1;
//		newLinearForce = newLinearForce + forceForward * speed * eTime * -1;		// calc force for physics
//	}
//	if (HasMovementFlag(B_CONTROL_STRAFE_LEFT))
//	{
//		playerPos += right * speed * eTime * -1;
//
//		newLinearForce = newLinearForce + forceRight * speed * eTime * -1;
//	}
//	if (HasMovementFlag(B_CONTROL_STRAFE_RIGHT))
//	{
//		playerPos += right * speed * eTime;
//
//		newLinearForce = newLinearForce + forceRight * speed * eTime;
//	}
//	if (HasMovementFlag(B_CONTROL_MOVE_DOWN))
//	{
//		playerPos += up * speed * eTime * -1;
//
//		newLinearForce = newLinearForce + forceUp * speed * eTime * -1;
//	}
//	if (HasMovementFlag(B_CONTROL_MOVE_UP))
//	{
//		playerPos += up * speed * eTime;
//
//		newLinearForce = newLinearForce + forceUp * speed * eTime;
//	}
//
//
//	fDesiredPosition = playerPos;
//	
//	// move toward our desired position...
//	
//	hsPoint3 curPos = targetMatrix.GetTranslate();
//	hsPoint3 newPos;
//
//	hsVector3 dir(fDesiredPosition - curPos);
// 	hsScalar distToGoal=dir.Magnitude();
//
//	if (dir.MagnitudeSquared() > 0.0f)
//		dir.Normalize();
//	
//	hsVector3 vel( view * fCurSpeed );
//
//	IAdjustVelocity(fAcceleration, fDeceleration, &dir, &vel, fMaxVelocity, distToGoal, eTime);
//	fCurSpeed = vel.Magnitude();
//	
//	hsScalar distMoved = IClampVelocity(&vel, fMaxVelocity, eTime);
//
//	// compute final pos
//	if (distMoved > distToGoal)
//		newPos = fDesiredPosition;
//	else
//		newPos = curPos + vel;
//	
//	// calculate rotation matrix
//
//	hsVector3 rotUp(0,0,1);
//	hsVector3 rotRight(1,0,0);
//	hsMatrix44 rot;
//	
//	if ( HasMovementFlag( B_CONTROL_TURN_TO ) )
//	{	
//		// compute view goal
//		
//		hsVector3 fPlayerViewGoal(&fFacingTarget,&curPos);
//		fPlayerViewGoal.fZ = 0;
//		fPlayerViewGoal.Normalize();
//
//		// compute degrees needed to turn left/right		
//		hsVector3 cross = fPlayerViewGoal % view;
//		hsScalar dot = fPlayerViewGoal * view;
//		hsScalar rad = hsACosine(dot);
//		fRotationScalar = 1.0f;
//		
//		if (cross.fZ<0)
//		{
//			SetMovementFlag( B_CONTROL_ROTATE_LEFT );
//		}
//		else
//		{
//			SetMovementFlag( B_CONTROL_ROTATE_RIGHT );
//		}
//		if (dot >= 0.999f)
//		{
//			ClearMovementFlag( B_CONTROL_TURN_TO );
//			ClearMovementFlag( B_CONTROL_ROTATE_RIGHT );
//			ClearMovementFlag( B_CONTROL_ROTATE_LEFT );
//		}
//	}
//	
//	hsScalar angle = 0;
//
//	if ( HasMovementFlag( B_CONTROL_ROTATE_RIGHT ) )
//	{
//		angle = fTurnRate * eTime * -1 * fRotationScalar;
//	}
//	
//	if ( HasMovementFlag( B_CONTROL_ROTATE_LEFT ) || HasMovementFlag( A_CONTROL_TURN ) )
//	{
//		angle = fTurnRate * eTime * fRotationScalar;
//	}
//
//	hsMatrix44 justRot(targetMatrix);
//	hsPoint3 zero(0,0,0);
//	justRot.SetTranslate(&zero);
//
//	if(angle) {
//		hsQuat q(angle, &rotUp);	
//		q.NormalizeIfNeeded();
//		q.MakeMatrix(&rot);
//
//		justRot = rot * justRot;
//
//		targetMatrix = rot * targetMatrix;
//	}
//
//	// use the desired rotation matrix to set position and rotation:
//
//
//	plSimulationInterface * SI = IGetTargetSimulationInterface(0);
//
//	if(SI)
//	{
//		Havok::Vector3 hkLocalForce(newLinearForce.fX, newLinearForce.fY, newLinearForce.fZ);
//		if (bUseDesiredMatrix)
//		{
//			hsMatrix44 inv;
//
//			fDesiredMatrix.GetInverse(&inv);
//
//			// we're just going to set the position on the simulation interface directly
//			// because it will then be further modified by the simulation and its final position
//			// will *then* be sent to the coordinate interface
//			SI->SetTransform(fDesiredMatrix, inv);
//		}
//
//		SI->SetRotation(justRot);//rot);
//		SI->ApplyForce(plSimulationInterface::kForce, hkLocalForce);
//	} else {
//		hsMatrix44 inv;
//		targetMatrix.SetTranslate(&newPos);
//		targetMatrix.GetInverse(&inv);
//
//		plCoordinateInterface* pCI = pCI = IGetTargetCoordinateInterface(0);
//		pCI->SetTransform(targetMatrix, inv);
//
//			
//	}
//	
//	fLastTime = secs;
//	SetMoving(fCurSpeed);
//
//	if (bUseDesiredMatrix)
//		bUseDesiredMatrix = false;
//	return true;
//}
//
////
//// vector version.  dir vector should be normalized
////
//void plPlayerModifier::IAdjustVelocity(hsScalar adjAccelRate, hsScalar adjDecelRate, 
//									   hsVector3* dir, hsVector3* vel, hsScalar maxSpeed, 
//									   hsScalar distToGoal, double elapsedTime)
//{
//	hsScalar speed = vel->Magnitude();		// save current speed
//	*vel = *dir * speed;					// change vel to correct dir
//
//	// compute accel/decel
//	hsScalar finalAccelRate;
//	if (IShouldDecelerate(adjDecelRate, speed, distToGoal))
//	{
//		finalAccelRate = -adjDecelRate;
//	}
//	else
//	{
//		finalAccelRate = adjAccelRate;
//	}
//
//	if (finalAccelRate  != 0)
//	{
//		// compute accel vector in the direction of the goal
//		hsVector3 accelVec = *dir * finalAccelRate;
//		accelVec = accelVec * elapsedTime;
//
//		// add acceleration to velocity
//		*vel = *vel + accelVec;
//	}
//	else
//	{
//		*vel = *dir * maxSpeed;
//	}
//}
//
//hsScalar plPlayerModifier::IClampVelocity(hsVector3* vel, hsScalar maxSpeed, double elapsedTime)
//{
//	*vel = *vel * elapsedTime;
//	maxSpeed *= elapsedTime;
//
//	// clamp speed	(clamp if going negative?)
//	hsScalar distMoved = vel->Magnitude();
//	if (distMoved > maxSpeed)
//	{
//		vel->Normalize();
//		*vel = *vel * maxSpeed;
//		return maxSpeed;
//	}
//	return distMoved;
//}
//
//hsBool32 plPlayerModifier::IShouldDecelerate(hsScalar decelSpeed, hsScalar curSpeed, hsScalar distToGoal)
//{
//	if (decelSpeed == 0)
//		// no deceleration
//		return false;
//
//	// compute distance required to stop, given decel speed (in units/sec sq)
//	hsScalar stopTime = curSpeed / decelSpeed;		
//	hsScalar avgSpeed = curSpeed * .5f;
//	hsScalar stopDist = avgSpeed * stopTime;
//
//	return (hsABS(distToGoal) <= hsABS(stopDist));	// stopDist+avgSpeed?	
//}
//
