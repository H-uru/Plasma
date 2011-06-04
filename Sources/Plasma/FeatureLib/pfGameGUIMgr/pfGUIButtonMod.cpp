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
//	pfGUIButtonMod Definition												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIButtonMod.h"
#include "pfGUIDraggableMod.h"
#include "pfGameGUIMgr.h"
#include "pfGUIControlHandlers.h"
#include "pfGUIDialogMod.h"

#include "../plInputCore/plInputInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Control Proc For Managing the Draggable /////////////////////////////////

class pfGUIButtonDragProc : public pfGUICtrlProcObject
{
	protected:

		pfGUICtrlProcObject	*fOrigProc;

		pfGUIButtonMod		*fParent;
		pfGUIDraggableMod	*fDraggable;
		hsBool				fReportDrag;

	public:

		pfGUIButtonDragProc( pfGUIButtonMod *parent, pfGUIDraggableMod *draggable, pfGUICtrlProcObject *origProc, hsBool reportDrag )
		{
			fParent = parent;
			fDraggable = draggable;
			fOrigProc = origProc;
			fReportDrag = reportDrag;
		}

		virtual void	DoSomething( pfGUIControlMod *ctrl )
		{
			// The draggable was let up, so now we stop dragging, disable the draggable again, and pass
			// on the event to our original proc
			if( fOrigProc != nil && fParent->IsTriggering() )
				fOrigProc->DoSomething( ctrl );
			if (!fParent->IsButtonDown())
				fParent->StopDragging( false );
		}

		virtual void	HandleExtendedEvent( pfGUIControlMod *ctrl, UInt32 event )
		{
			if( event == pfGUIDraggableMod::kDragging )
			{
				// First test if we're inside our button (if so, we stop dragging)
				if( fParent->PointInBounds( fDraggable->GetLastMousePt() ) )
				{
					// Cancel the drag
					fParent->StopDragging( true );
					return;
				}

				if( !fReportDrag )
					return;
			}
			
			if( fOrigProc != nil )
				fOrigProc->HandleExtendedEvent( ctrl, event );
		}

		virtual void	UserCallback( UInt32 userValue )
		{
			if( fOrigProc != nil )
				fOrigProc->UserCallback( userValue );
		}
};


void	pfGUIButtonMod::StopDragging( hsBool cancel )
{
	fDraggable->StopDragging( cancel );
	fDraggable->SetVisible( false );
	fDraggable->SetHandler( fOrigHandler );
	fOrigHandler = nil;

	if( !fOrigReportedDrag )
		fDraggable->ClearFlag( pfGUIDraggableMod::kReportDragging );

	// Steal interest back
	fDialog->SetControlOfInterest( this );
}

void	pfGUIButtonMod::StartDragging( void )
{
	fOrigReportedDrag = fDraggable->HasFlag( pfGUIDraggableMod::kReportDragging );
	fDraggable->SetFlag( pfGUIDraggableMod::kReportDragging );

	fOrigHandler = fDraggable->GetHandler();
	fDraggable->SetVisible( true );
	fDraggable->SetHandler( TRACKED_NEW pfGUIButtonDragProc( this, fDraggable, fOrigHandler, fOrigReportedDrag ) );
	fDraggable->HandleMouseDown( fOrigMouseDownPt, 0 );
}

//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIButtonMod::pfGUIButtonMod()
{
	fAnimName = nil;
	fMouseOverAnimName = nil;
	fDraggable = nil;
	fOrigHandler = nil;

	fClicking = false;
	fTriggering = false;
	fNotifyType = kNotifyOnUp;
	SetFlag( kWantsInterest );
}

pfGUIButtonMod::~pfGUIButtonMod()
{
	delete [] fAnimName;
	delete [] fMouseOverAnimName;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIButtonMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIButtonMod::MsgReceive( plMessage *msg )
{
	plGenRefMsg *refMsg = plGenRefMsg::ConvertNoRef( msg );
	if( refMsg != nil && refMsg->fType == kRefDraggable )
	{
		if( refMsg->GetContext() & ( plRefMsg::kOnCreate | plRefMsg::kOnRequest | plRefMsg::kOnReplace ) )
		{
			fDraggable = pfGUIDraggableMod::ConvertNoRef( refMsg->GetRef() );
			fDraggable->SetVisible( false );		// Disable until we're dragging
		}
		else
			fDraggable = nil;
		return true;
	}

	return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIButtonMod::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);

	fAnimationKeys.Reset();
	UInt32 i, count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
		fAnimationKeys.Append( mgr->ReadKey( s ) );
	fAnimName = s->ReadSafeString();

	fMouseOverAnimKeys.Reset();
	count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
		fMouseOverAnimKeys.Append( mgr->ReadKey( s ) );
	fMouseOverAnimName = s->ReadSafeString();

	fNotifyType = s->ReadSwap32();
	mgr->ReadKeyNotifyMe( s, TRACKED_NEW plGenRefMsg( GetKey(), plRefMsg::kOnCreate, -1, kRefDraggable ), plRefFlags::kActiveRef );
}

void	pfGUIButtonMod::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );

	UInt32 i, count = fAnimationKeys.GetCount();
	s->WriteSwap32( count );
	for( i = 0; i < count; i++ )
		mgr->WriteKey( s, fAnimationKeys[ i ] );
	s->WriteSafeString( fAnimName );

	count = fMouseOverAnimKeys.GetCount();
	s->WriteSwap32( count );
	for( i = 0; i < count; i++ )
		mgr->WriteKey( s, fMouseOverAnimKeys[ i ] );
	s->WriteSafeString( fMouseOverAnimName );

	s->WriteSwap32( fNotifyType );

	mgr->WriteKey( s, fDraggable != nil ? fDraggable->GetKey() : nil );

}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIButtonMod::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIControlMod::UpdateBounds( invXformMatrix, force );
	if( fAnimationKeys.GetCount() > 0 || fMouseOverAnimKeys.GetCount() > 0 )
		fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUIButtonMod::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	fClicking = true;
	if( fAnimationKeys.GetCount() > 0 )
	{
		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		msg->SetCmd( plAnimCmdMsg::kContinue );
		msg->SetCmd( plAnimCmdMsg::kSetForewards );
		msg->SetCmd( plAnimCmdMsg::kGoToBegin );	
		msg->SetAnimName( fAnimName );
		msg->AddReceivers( fAnimationKeys );
		plgDispatch::MsgSend( msg );
	}

	IPlaySound( kMouseDown );

	fOrigMouseDownPt = mousePt;
	if ( fNotifyType == kNotifyOnDown || fNotifyType == kNotifyOnUpAndDown)
	{
		fTriggering = true;
		DoSomething();
		fTriggering = false;
	}
}

void	pfGUIButtonMod::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{

	// make sure that we got the down click first
	if ( !fClicking )
		return;

	fClicking = false;
	if( fAnimationKeys.GetCount() > 0 )
	{
		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		msg->SetCmd( plAnimCmdMsg::kContinue );
		msg->SetCmd( plAnimCmdMsg::kSetBackwards );
		msg->SetCmd( plAnimCmdMsg::kGoToEnd );	
		msg->SetAnimName( fAnimName );
		msg->AddReceivers( fAnimationKeys );
		plgDispatch::MsgSend( msg );
	}

	IPlaySound( kMouseUp );

	// Don't run the command if the mouse is outside our bounds
	if( !fBounds.IsInside( &mousePt ) && fNotifyType != kNotifyOnUpAndDown )
		return;		

	if ( fNotifyType == kNotifyOnUp || fNotifyType == kNotifyOnUpAndDown)
		fTriggering = true;
	DoSomething();
	fTriggering = false;
}

void	pfGUIButtonMod::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( !fClicking )
		return;

	if( fDraggable == nil )
		return;

	if( !fDraggable->IsVisible() )
	{
		// Are we outside ourselves?
		if( !PointInBounds( mousePt ) )
		{
			// Yes, start dragging
			StartDragging();

			// Hand off our interest to the draggable
			fDialog->SetControlOfInterest( fDraggable );
		}
	}
}

void	pfGUIButtonMod::SetNotifyType(Int32 kind)
{
	fNotifyType = kind;
}

Int32	pfGUIButtonMod::GetNotifyType()
{
	return fNotifyType;
}

hsBool	pfGUIButtonMod::IsButtonDown()
{
	return fClicking;
}

//// SetInteresting //////////////////////////////////////////////////////////
//	Overridden to play mouse over animation when we're interesting

void	pfGUIButtonMod::SetInteresting( hsBool i )
{
	pfGUIControlMod::SetInteresting( i );

	if( fMouseOverAnimKeys.GetCount() )
	{
		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		msg->SetCmd( plAnimCmdMsg::kContinue );
		msg->SetCmd( fInteresting ? plAnimCmdMsg::kSetForewards : plAnimCmdMsg::kSetBackwards );
		msg->SetAnimName( fMouseOverAnimName );
		msg->AddReceivers( fMouseOverAnimKeys );
		plgDispatch::MsgSend( msg );
	}

	if( i )
		IPlaySound( kMouseOver );
	else
		IPlaySound( kMouseOff );
}


void	pfGUIButtonMod::SetAnimationKeys( hsTArray<plKey> &keys, const char *name )
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

void	pfGUIButtonMod::SetMouseOverAnimKeys( hsTArray<plKey> &keys, const char *name )
{
	fMouseOverAnimKeys = keys;
	delete [] fMouseOverAnimName;
	if( name != nil )
	{
		fMouseOverAnimName = TRACKED_NEW char[ strlen( name ) + 1 ];
		strcpy( fMouseOverAnimName, name );
	}
	else
		fMouseOverAnimName = nil;
}


//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUIButtonMod::IGetDesiredCursor( void ) const
{
	if( fHandler == nil )
		return 0;

	if( fClicking )
		return plInputInterface::kCursorClicked;

	return plInputInterface::kCursorPoised;
}

