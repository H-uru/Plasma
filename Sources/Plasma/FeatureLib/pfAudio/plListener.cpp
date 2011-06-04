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
#include "hsMatrix44.h"
#include "hsTypes.h"
#include "plListener.h"
#include "plgDispatch.h"
#include "../plAudio/plAudioSystem.h"
#include "../pnMessage/plTimeMsg.h"
#include "../pnMessage/plAudioSysMsg.h"
#include "../pnKeyedObject/plKey.h"
#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnSceneObject/plSimulationInterface.h"
#include "../pfCamera/plVirtualCamNeu.h"
#include "../plMessage/plListenerMsg.h"
#include "../plNetClient/plNetClientMgr.h"
#include "../plPipeline/plDebugText.h"

#include "../plAvatar/plAvatarMgr.h"
#include "../plAvatar/plArmatureMod.h"
#include "../plAvatar/plAvCallbackAction.h"

hsBool		plListener::fPrintDbgInfo = false;

hsBool plListener::IEval(double secs, hsScalar del, UInt32 dirty)
{
//	if (!plgAudioSys::Active())
//		return true;
	plSceneObject *pRefObject = nil;

	int y = 16 + 12, x = 400;
	if( fPrintDbgInfo ) 
		plDebugText::Instance().DrawString( x, 16, "Listener:", (UInt32)0xffffffff, plDebugText::kStyleBold );

	// Get the avatar's SceneObject
	plKey key = plNetClientMgr::GetInstance()->GetLocalPlayerKey();
	if(key)
		pRefObject = plSceneObject::ConvertNoRef(key->ObjectIsLoaded());

	if( pRefObject == nil && fVCam == nil )
	{
		// We don't have a position to init by, so do NOT eval yet!!!
		if( fPrintDbgInfo ) 
			plDebugText::Instance().DrawString( x, y, "Not eval-ing yet", (UInt32)0xffffffff );
		return true;
	}

	// Changed 2.19.02 mcn - Basing it off the head bone isn't what we really want, esp. since
	// it isn't what the camera uses. What we *really* want is a head-ish-positioned non-bobbing node
	// that we base both off of. Until then, we're just going to have to use the avatar's root (i.e. his
	// feet) and add in an appropriate height. See plAvBrain.cpp::BindAudioListener() for the other half
	// of the hack.
	// Note the 2nd: since GetAxis() is buggy, we'll just add in a constant vector. Of course, this implies
	// that the avatar is always oriented up, but then it also implies he's always of constant height, so
	// there.
	const hsVector3	kAvatarHeightVector = hsVector3( 0, 0, 6.33f );	// isn't *everyone* 6'4"?

	/// Collect the current values for our parameters
	hsPoint3	position;
	hsVector3	velocity, dir, up;

	enum 
	{
		kInvalid = 0,
		kVCam,
		kObject
	} facingType = kInvalid, posType = kInvalid, velType = kInvalid;

	// Facing
	if( fFacingRatio == 1.f )
	{
		if( pRefObject != nil && pRefObject->GetCoordinateInterface() )
		{
			hsMatrix44 facingL2W = pRefObject->GetCoordinateInterface()->GetLocalToWorld();
			dir = facingL2W.GetAxis( hsMatrix44::kView );
			up = facingL2W.GetAxis( hsMatrix44::kUp );

			facingType = kObject;
		}
	}
	else if( fVCam != nil )
	{
		dir = hsVector3( fVCam->GetCameraPOA() - fVCam->GetCameraPos() );
		up = fVCam->GetCameraUp();
		facingType = kVCam;
	}

	// Position
	if( fPosRatio == 1.f )
	{
		if( pRefObject != nil && pRefObject->GetCoordinateInterface() )
		{
			position = pRefObject->GetCoordinateInterface()->GetLocalToWorld().GetTranslate();
			position += kAvatarHeightVector;
			posType = kObject;
		}
	}
	else if( fVCam != nil )
	{
		position = fVCam->GetCameraPos();
		posType = kVCam;
	}

	// Velocity
	if( fVelRatio == 1.f )
	{
		if( pRefObject != nil )
		{
			plArmatureMod* arm = plAvatarMgr::GetInstance()->GetLocalAvatar();
			if (arm)
			{
				plPhysicalControllerCore* controller = arm->GetController();
				if (controller)
				{
					velocity = controller->GetLinearVelocity();
					velType = kObject;
				}
			}
		}
	}
	else if( fVCam != nil )
	{
		// Darn, can't do it
	}

	if( facingType == kInvalid || posType == kInvalid || velType == kInvalid )
	{
		if( fPrintDbgInfo ) 
			plDebugText::Instance().DrawString( x, y, "Not eval-ing: missing one or more parameter bases", (UInt32)0xff0000ff );
		return true;
	}

	// Got the params, now construct and send out the message, as well as update the audio system
	plListenerMsg* msg = TRACKED_NEW plListenerMsg;
	msg->SetDirection( dir );
	msg->SetUp( up );
	msg->SetPosition( position );
	msg->SetVelocity( velocity );

	plgAudioSys::SetListenerOrientation( dir, up );
	plgAudioSys::SetListenerPos( position );
	plgAudioSys::SetListenerVelocity( velocity );

	if( fPrintDbgInfo ) 
	{
		char str[ 256 ];
		sprintf( str, "Direction: (%3.2f,%3.2f,%3.2f) from %s", dir.fX, dir.fY, dir.fZ, ( facingType == kObject ) ? pRefObject->GetKey()->GetUoid().GetObjectName() : "VCam" );
		plDebugText::Instance().DrawString( x, y, str, (UInt32)0xffffffff );
		y += 12;

		sprintf( str, "Up: (%3.2f,%3.2f,%3.2f) from %s", up.fX, up.fY, up.fZ, ( facingType == kObject ) ? pRefObject->GetKey()->GetUoid().GetObjectName() : "VCam" );
		plDebugText::Instance().DrawString( x, y, str, (UInt32)0xffffffff );
		y += 12;

		sprintf( str, "Position: (%3.2f,%3.2f,%3.2f) from %s", position.fX, position.fY, position.fZ, ( posType == kObject ) ? pRefObject->GetKey()->GetUoid().GetObjectName() : "VCam" );
		plDebugText::Instance().DrawString( x, y, str, (UInt32)0xffffffff );
		y += 12;

		sprintf( str, "Velocity: (%3.2f,%3.2f,%3.2f) from %s", velocity.fX, velocity.fY, velocity.fZ, ( velType == kObject ) ? pRefObject->GetKey()->GetUoid().GetObjectName() : "VCam" );
		plDebugText::Instance().DrawString( x, y, str, (UInt32)0xffffffff );
		y += 12;
	}
	plgDispatch::MsgSend( msg );

	return true;
}

void	plListener::ISetRef( const plKey &ref, hsBool binding, int type )
{
	if( binding )
		hsgResMgr::ResMgr()->AddViaNotify( ref, TRACKED_NEW plGenRefMsg( GetKey(), plGenRefMsg::kOnReplace, -1, type ), plRefFlags::kPassiveRef );
	else
		GetKey()->Release( ref );
}

void	plListener::IEnsureVCamValid( void )
{
	if( fPosRatio == 1.f && fFacingRatio == 1.f && fVelRatio == 1.f )
	{
		// All of our params are purely using objects, so we don't need a virtual camera pointer at all
		if( fVCam != nil )
			ISetRef( fVCam->GetKey(), false, kRefVCam );
	}
	else
	{
		// One or more of our params are using the vcam as a basis, so make sure we have it
		if( fVCam == nil )
		{
			plVirtualCam1 *vCam = plVirtualCam1::Instance();
			if( vCam == nil )
			{
				hsAssert( false, "Unable to grab virtual camera instance; no basis for listener!!!" );
				return;
			}

			ISetRef( vCam->GetKey(), true, kRefVCam );
		}
	}
}

void	plListener::ICheckAudio( void ) const
{
	if( ( fPosRatio < 1.f || fFacingRatio < 1.f || fVelRatio < 1.f ) && fVCam == nil )
		plgAudioSys::SetMuted( true );
}

hsBool plListener::MsgReceive(plMessage* msg)
{
	plSetListenerMsg *setMsg = plSetListenerMsg::ConvertNoRef( msg );
	if( setMsg != nil )
	{
		hsBool useVCam;

		if( setMsg->GetType() & plSetListenerMsg::kVCam )
		{
			// Reset any ratios
			if( setMsg->GetType() & plSetListenerMsg::kPosition )
				fPosRatio = 0.f;

			if( setMsg->GetType() & plSetListenerMsg::kVelocity )
				fVelRatio = 0.f;

			if( setMsg->GetType() & plSetListenerMsg::kFacing )
				fFacingRatio = 0.f;

			IEnsureVCamValid();
		}
		else
		{
			useVCam = setMsg->IsBinding();
			
			if( setMsg->GetType() & plSetListenerMsg::kPosition )
				fPosRatio = 1.f;

			if( setMsg->GetType() & plSetListenerMsg::kVelocity )
				fVelRatio = 1.f;

			if( setMsg->GetType() & plSetListenerMsg::kFacing )
				fFacingRatio = 1.f;

			if( fPosRatio > 0.f || fVelRatio > 0.f || fFacingRatio > 0.f )
				// Need this, so store it now
				ISetRef( setMsg->GetSrcKey(), setMsg->IsBinding(), kRefObject );
		}

		return true;
	}

	plEvalMsg* pEMsg = plEvalMsg::ConvertNoRef(msg);
	if (pEMsg)
	{	
		IEval(pEMsg->GetTimeStamp(), pEMsg->DelSeconds(), true);

		if( fInitMe )
		{
			// By default, position and orientation are camera based
			plSetListenerMsg *set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kFacing, nil, true );
			set->Send();
			set = TRACKED_NEW plSetListenerMsg( plSetListenerMsg::kVCam | plSetListenerMsg::kPosition, nil, true );
			set->Send();

			fInitMe = false;
		}

		return true;
	}

	plGenRefMsg* refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil )
	{
		if( refMsg->fType == kRefVCam )
		{
			if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
			{
				fVCam = plVirtualCam1::ConvertNoRef( refMsg->GetRef() );
			}
			else if( refMsg->GetContext() & ( plRefMsg::kOnRemove | plRefMsg::kOnDestroy ) )
			{
				if( plVirtualCam1::ConvertNoRef( refMsg->GetRef() ) == fVCam )
					fVCam = nil;
			}
			ICheckAudio();
		}

		return true;
	}

	return plSingleModifier::MsgReceive(msg);
}