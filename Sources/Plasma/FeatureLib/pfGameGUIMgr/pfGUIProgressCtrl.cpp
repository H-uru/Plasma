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
//////////////////////////////////////////////////////////////////////////////
//																			//
//	pfGUIProgressCtrl Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIProgressCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "../plInputCore/plInputInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plMessage/plTimerCallbackMsg.h"
// #include "../plAvatar/plAGModifier.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plAvatar/plAGAnimInstance.h"
#include "../plSurface/plLayerAnimation.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"
#include "../pnTimer/plTimerCallbackManager.h"

#include "plgDispatch.h"
#include "hsResMgr.h"

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIProgressCtrl::pfGUIProgressCtrl() : fStopSoundTimer(99)
{
	fAnimTimesCalced = false;
	fAnimName = nil;
	fPlaySound = true;
}

pfGUIProgressCtrl::~pfGUIProgressCtrl()
{
	delete [] fAnimName;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIProgressCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIValueCtrl::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIProgressCtrl::MsgReceive( plMessage *msg )
{
	plTimerCallbackMsg *timerMsg = plTimerCallbackMsg::ConvertNoRef(msg);
	if (timerMsg)
	{
		if (timerMsg->fID == fStopSoundTimer)
		{
			// we've finished animating, stop the sound that's playing
			StopSound(kAnimateSound);
		}
	}
	return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIProgressCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Read(s, mgr);

	fAnimationKeys.Reset();
	UInt32 i, count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
		fAnimationKeys.Append( mgr->ReadKey( s ) );
	fAnimName = s->ReadSafeString();

	fAnimTimesCalced = false;
}

void	pfGUIProgressCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Write( s, mgr );

	UInt32 i, count = fAnimationKeys.GetCount();
	s->WriteSwap32( count );
	for( i = 0; i < count; i++ )
		mgr->WriteKey( s, fAnimationKeys[ i ] );
	s->WriteSafeString( fAnimName );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIProgressCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIValueCtrl::UpdateBounds( invXformMatrix, force );
	if( fAnimationKeys.GetCount() > 0 )
		fBoundsValid = false;
}

//// SetAnimationKeys ////////////////////////////////////////////////////////

void	pfGUIProgressCtrl::SetAnimationKeys( hsTArray<plKey> &keys, const char *name )
{
	fAnimationKeys = keys;
	delete [] fAnimName;
	if( name != nil )
	{
		fAnimName = TRACKED_NEW char[ strlen( name ) + 1 ];
		strcpy( fAnimName, name );
	}
	else
		fAnimName = nil;
}

//// ICalcAnimTimes //////////////////////////////////////////////////////////
//	Loops through and computes the max begin and end for our animations. If
//	none of them are loaded and we're not already calced, returns false.

hsBool	pfGUIProgressCtrl::ICalcAnimTimes( void )
{
	if( fAnimTimesCalced )
		return true;

	hsScalar tBegin = 1e30, tEnd = -1e30;
	bool	 foundOne = false;

	for( int i = 0; i < fAnimationKeys.GetCount(); i++ )
	{
		// Handle AGMasterMods
		plAGMasterMod *mod = plAGMasterMod::ConvertNoRef( fAnimationKeys[ i ]->ObjectIsLoaded() );
		if( mod != nil )
		{
			for( int j = 0; j < mod->GetNumAnimations(); j++ )
			{
				hsScalar begin = mod->GetAnimInstance( j )->GetTimeConvert()->GetBegin();
				hsScalar end = mod->GetAnimInstance( j )->GetTimeConvert()->GetEnd();
				if( begin < tBegin )
					tBegin = begin;
				if( end > tEnd )
					tEnd = end;
			}
			foundOne = true;
		}
		// Handle layer animations
		plLayerAnimation *layer = plLayerAnimation::ConvertNoRef( fAnimationKeys[ i ]->ObjectIsLoaded() );
		if( layer != nil )
		{
			hsScalar begin = layer->GetTimeConvert().GetBegin();
			hsScalar end = layer->GetTimeConvert().GetEnd();
			if( begin < tBegin )
				tBegin = begin;
			if( end > tEnd )
				tEnd = end;
			foundOne = true;
		}
	}

	if( foundOne )
	{
		fAnimBegin = tBegin;
		fAnimEnd = tEnd;

		fAnimTimesCalced = true;
	}

	return fAnimTimesCalced;
}

//// SetCurrValue ////////////////////////////////////////////////////////////

void	pfGUIProgressCtrl::SetCurrValue( hsScalar v )
{
	int old = (int)fValue;

	pfGUIValueCtrl::SetCurrValue( v );

//	if( old == (int)fValue )
//		return;

	if( fAnimationKeys.GetCount() > 0 )
	{
		ICalcAnimTimes();

		hsScalar tLength = fAnimEnd - fAnimBegin;
		hsScalar newTime;

		if( HasFlag( kReverseValues ) )
			newTime = ( ( fMax - fValue ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
		else
			newTime = ( ( fValue - fMin ) / ( fMax - fMin ) ) * tLength + fAnimBegin;

		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		msg->SetCmd( plAnimCmdMsg::kGoToTime );	
		msg->SetAnimName( fAnimName );
		msg->fTime = newTime;
		msg->AddReceivers( fAnimationKeys );
		plgDispatch::MsgSend( msg );
	}
}

void pfGUIProgressCtrl::AnimateToPercentage( hsScalar percent )
{
	// percent should be a value in range 0.0 to 1.0
	if (percent >= 0.0f && percent <= 1.0f)
	{
		pfGUIValueCtrl::SetCurrValue( (fMax - fMin) * percent + fMin );

		if( fAnimationKeys.GetCount() > 0 )
		{
			plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
			msg->SetCmd( plAnimCmdMsg::kPlayToPercentage );	
			msg->SetAnimName( fAnimName );
			msg->fTime = percent;
			msg->AddReceivers( fAnimationKeys );
			plgDispatch::MsgSend( msg );

			if (fPlaySound)
			{
				// play the sound, looping
				PlaySound(kAnimateSound, true);

				// setup a timer to call back when we finish animating
				hsScalar elapsedTime = (fAnimEnd - fAnimBegin) * percent;
				plTimerCallbackMsg *timerMsg = TRACKED_NEW plTimerCallbackMsg(GetKey(), fStopSoundTimer);
				plgTimerCallbackMgr::NewTimer(elapsedTime, timerMsg);
			}
		}
	}
}