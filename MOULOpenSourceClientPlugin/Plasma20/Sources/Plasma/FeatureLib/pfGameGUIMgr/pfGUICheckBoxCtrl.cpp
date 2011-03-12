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
//	pfGUICheckBoxCtrl Definition											//
//																			//
//	Almost like buttons, only they keep their stated (pressed/unpressed)	//
//	when you click them, instead of reverting on mouse up.					//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUICheckBoxCtrl.h"
#include "pfGameGUIMgr.h"

#include "../plInputCore/plInputInterface.h"
 #include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUICheckBoxCtrl::pfGUICheckBoxCtrl()
{
	fAnimName = nil;
	SetFlag( kWantsInterest );
	fChecked = false;
	fClicking = false;
	fPlaySound = true;
}

pfGUICheckBoxCtrl::~pfGUICheckBoxCtrl()
{
	delete [] fAnimName;
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUICheckBoxCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUICheckBoxCtrl::MsgReceive( plMessage *msg )
{
	return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUICheckBoxCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);

	fAnimationKeys.Reset();
	UInt32 i, count = s->ReadSwap32();
	for( i = 0; i < count; i++ )
		fAnimationKeys.Append( mgr->ReadKey( s ) );

	fAnimName = s->ReadSafeString();
	fChecked = s->ReadBool();
}

void	pfGUICheckBoxCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );

	UInt32 i, count = fAnimationKeys.GetCount();
	s->WriteSwap32( count );
	for( i = 0; i < count; i++ )
		mgr->WriteKey( s, fAnimationKeys[ i ] );

	s->WriteSafeString( fAnimName );
	s->WriteBool( fChecked );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUICheckBoxCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIControlMod::UpdateBounds( invXformMatrix, force );
	if( fAnimationKeys.GetCount() > 0 )
		fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUICheckBoxCtrl::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	fClicking = true;
	if(fPlaySound)
		IPlaySound( kMouseDown );
}

void	pfGUICheckBoxCtrl::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fClicking )
	{
		fClicking = false;

		if(fPlaySound)
			IPlaySound( kMouseUp );

		// Don't run the command if the mouse is outside our bounds
		if( fBounds.IsInside( &mousePt ) )
		{
			SetChecked( !fChecked );
			DoSomething();
		}
	}
}

//// SetChecked //////////////////////////////////////////////////////////////

void	pfGUICheckBoxCtrl::SetChecked( hsBool checked, hsBool immediate /*= false*/ )
{
	fChecked = checked;
	if( fAnimationKeys.GetCount() > 0 )
	{
		plAnimCmdMsg *msg = TRACKED_NEW plAnimCmdMsg();
		if( fChecked )
		{
			// Moving to true
			if( immediate )
			{
				msg->SetCmd( plAnimCmdMsg::kGoToEnd );
			}
			else
			{
				msg->SetCmd( plAnimCmdMsg::kContinue );
				msg->SetCmd( plAnimCmdMsg::kSetForewards );
				msg->SetCmd( plAnimCmdMsg::kGoToBegin );
			}
		}
		else
		{
			// Moving to false
			if( immediate )
			{
				msg->SetCmd( plAnimCmdMsg::kGoToBegin );
			}
			else
			{
				msg->SetCmd( plAnimCmdMsg::kContinue );
				msg->SetCmd( plAnimCmdMsg::kSetBackwards );
				msg->SetCmd( plAnimCmdMsg::kGoToEnd );
			}
		}
		msg->SetAnimName( fAnimName );
		msg->AddReceivers( fAnimationKeys );
		plgDispatch::MsgSend( msg );
	}
}

void	pfGUICheckBoxCtrl::SetAnimationKeys( hsTArray<plKey> &keys, const char *name )
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

//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUICheckBoxCtrl::IGetDesiredCursor( void ) const
{
	if( fClicking )
		return plInputInterface::kCursorClicked;

	return plInputInterface::kCursorPoised;
}

