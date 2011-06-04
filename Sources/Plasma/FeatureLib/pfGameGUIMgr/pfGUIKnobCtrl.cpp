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
//	pfGUIKnobCtrl Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIKnobCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "../plInputCore/plInputInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
// #include "../plAvatar/plAGModifier.h"
#include "../plAvatar/plAGMasterMod.h"
#include "../plAvatar/plAGAnimInstance.h"
#include "../plSurface/plLayerAnimation.h"

#include "../pnSceneObject/plSceneObject.h"
#include "../pnSceneObject/plCoordinateInterface.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIKnobCtrl::pfGUIKnobCtrl() :
	fAnimName(nil),
	fDragStart(0.f, 0.f, 0.f),
	fDragging(false),
	fAnimStartPos(0.f, 0.f, 0.f),
	fAnimEndPos(0.f, 0.f, 0.f),
	fDragRangeMin(0.f),
	fDragRangeMax(0.f),
	fAnimBegin(0.f),
	fAnimEnd(0.f),
	fAnimTimesCalced(false)
{
	SetFlag( kWantsInterest );
}

pfGUIKnobCtrl::~pfGUIKnobCtrl()
{
	delete [] fAnimName;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIKnobCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIValueCtrl::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIKnobCtrl::MsgReceive( plMessage *msg )
{
	return pfGUIValueCtrl::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIKnobCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Read(s, mgr);

	fAnimationKeys.Reset();
	UInt32 i, count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
		fAnimationKeys.Append( mgr->ReadKey( s ) );
	fAnimName = s->ReadSafeString();

	fAnimTimesCalced = false;

	fAnimStartPos.Read( s );
	fAnimEndPos.Read( s );
}

void	pfGUIKnobCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIValueCtrl::Write( s, mgr );

	UInt32 i, count = fAnimationKeys.GetCount();
	s->WriteSwap32( count );
	for( i = 0; i < count; i++ )
		mgr->WriteKey( s, fAnimationKeys[ i ] );
	s->WriteSafeString( fAnimName );

	fAnimStartPos.Write( s );
	fAnimEndPos.Write( s );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIKnobCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIValueCtrl::UpdateBounds( invXformMatrix, force );
	if( fAnimationKeys.GetCount() > 0 )
		fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUIKnobCtrl::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	fDragStart = mousePt;
	fDragValue = fValue;
	fDragging = true;

	if( HasFlag( kMapToAnimationRange ) )
	{
		hsPoint3	scrnStart, scrnEnd;

		// At mouse-down, we take our local-space start and end points and
		// translate them by our parent object's local-to-world to get the
		// right points in world-space. We do this now because our parent
		// might be animated, which could complicate matters a tad.
		scrnStart = fAnimStartPos;
		scrnEnd = fAnimEndPos;

		plSceneObject *target = GetTarget();
		if( target != nil )
		{
			const plCoordinateInterface *ci = target->GetCoordinateInterface();
			if( ci != nil )
			{
				const plCoordinateInterface *parentCI = ci->GetParent();
				if( parentCI != nil )
				{
					const hsMatrix44 &parentLocalToWorld = parentCI->GetLocalToWorld();
	
					scrnStart = parentLocalToWorld * scrnStart;
					scrnEnd = parentLocalToWorld * scrnEnd;
				}
			}
		}

		scrnStart = fDialog->WorldToScreenPoint( scrnStart );
		scrnEnd = fDialog->WorldToScreenPoint( scrnEnd );

		if( HasFlag( kLeftRightOrientation ) )
		{
			fDragRangeMin = scrnStart.fX;
			fDragRangeMax = scrnEnd.fX;
		}
		else
		{
			fDragRangeMin = scrnStart.fY;
			fDragRangeMax = scrnEnd.fY;
		}
	}
	else if( HasFlag( kMapToScreenRange ) )
	{
		fDragRangeMin = 0.f;
		fDragRangeMax = 1.f;
	}
	else
		fDragRangeMin = -1;
}

void	pfGUIKnobCtrl::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	fDragging = false;
	HandleMouseDrag( mousePt, modifiers );
}

void	pfGUIKnobCtrl::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	hsScalar oldValue = fValue, newValue = fDragValue;

	if( fDragRangeMin != -1 )
	{
		if( HasFlag( kLeftRightOrientation ) )
		{
			if( mousePt.fX < fDragRangeMin )
				newValue = fMin;
			else if( mousePt.fX > fDragRangeMax )
				newValue = fMax;
			else
				newValue = ( ( mousePt.fX - fDragRangeMin ) / ( fDragRangeMax - fDragRangeMin ) ) *
							( fMax - fMin ) + fMin;
		}
		else
		{
			if( mousePt.fY > fDragRangeMin )
				newValue = fMin;
			else if( mousePt.fY < fDragRangeMax )
				newValue = fMax;
			else
				newValue = ( (  fDragRangeMin - mousePt.fY) / ( fDragRangeMin - fDragRangeMax ) ) *
							( fMax - fMin ) + fMin;
		}

		if( HasFlag( kReverseValues ) )
			SetCurrValue( fMax - ( newValue - fMin ) );
		else
			SetCurrValue( newValue );
	}
	else
	{
		hsScalar diff;
		if( HasFlag( kLeftRightOrientation ) )
			diff = ( mousePt.fX - fDragStart.fX ) * 20.f;
		else
			diff = ( fDragStart.fY - mousePt.fY ) * 20.f;

		if( HasFlag( kReverseValues ) )
			SetCurrValue( fDragValue - diff );
		else
			SetCurrValue( fDragValue + diff );
	}

	// !fDragging = We're mousing-up, so if we're still dragging, we need to not have the only-
	// on-mouse-up flag set. Just FYI
	if( !fDragging || !HasFlag( kTriggerOnlyOnMouseUp ) )
		DoSomething();
}

//// SetAnimationKeys ////////////////////////////////////////////////////////

void	pfGUIKnobCtrl::SetAnimationKeys( hsTArray<plKey> &keys, const char *name )
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

hsBool	pfGUIKnobCtrl::ICalcAnimTimes( void )
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

void	pfGUIKnobCtrl::SetCurrValue( hsScalar v )
{
	int old = (int)fValue;
	pfGUIValueCtrl::SetCurrValue( v );

//	if( old == (int)fValue )
//		return;

	if( fAnimationKeys.GetCount() > 0 )
	{
		ICalcAnimTimes();

		hsScalar tLength = fAnimEnd - fAnimBegin;
		hsScalar newTime = fMin;

		if (fMin != fMax) // Protect against div by zero
		{
			if( HasFlag( kReverseValues ) )
				newTime = ( ( fMax - fValue ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
			else
				newTime = ( ( fValue - fMin ) / ( fMax - fMin ) ) * tLength + fAnimBegin;
		}
		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		msg->SetCmd( plAnimCmdMsg::kGoToTime );	
		msg->SetAnimName( fAnimName );
		msg->fTime = newTime;
		msg->AddReceivers( fAnimationKeys );
		plgDispatch::MsgSend( msg );
	}
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUIKnobCtrl::IGetDesiredCursor( void ) const
{
	if( HasFlag( kLeftRightOrientation ) )
	{
		if( fDragging )
			return plInputInterface::kCursorLeftRightDragging;

		return plInputInterface::kCursorLeftRightDraggable;
	}
	else
	{
		if( fDragging )
			return plInputInterface::kCursorUpDownDragging;

		return plInputInterface::kCursorUpDownDraggable;
	}
}

