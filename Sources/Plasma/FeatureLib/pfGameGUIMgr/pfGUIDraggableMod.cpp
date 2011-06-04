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
//	pfGUIDraggableMod Definition											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIDraggableMod.h"
#include "pfGameGUIMgr.h"

#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "../plInputCore/plInputInterface.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIDraggableMod::pfGUIDraggableMod()
{
	SetFlag( kWantsInterest );
	fDragging = false;
}

pfGUIDraggableMod::~pfGUIDraggableMod()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIDraggableMod::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIDraggableMod::MsgReceive( plMessage *msg )
{
	return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIDraggableMod::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);
}

void	pfGUIDraggableMod::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIDraggableMod::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIControlMod::UpdateBounds( invXformMatrix, force );
	fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUIDraggableMod::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( !fDragging )
	{
		fLastMousePt = mousePt;
		fOrigCenter = fScreenCenter;

		fDragging = true;
		fDragOffset = fScreenCenter - mousePt;

		SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );

		HandleExtendedEvent( kStartingDrag );
	}
}

void	pfGUIDraggableMod::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fDragging )
	{
		fLastMousePt = mousePt;
		fDragging = false;
		SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );

		DoSomething();

		if( HasFlag( kAlwaysSnapBackToStart ) )
			SetObjectCenter( fOrigCenter.fX, fOrigCenter.fY );
	}
}

void	pfGUIDraggableMod::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fDragging )
	{
		fLastMousePt = mousePt;

		SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );

		if( HasFlag( kReportDragging ) )
			HandleExtendedEvent( kDragging );
	}
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUIDraggableMod::IGetDesiredCursor( void ) const
{
	// if we are anchored, then no cursors that say we can move
	if( fDragging )
	{
		if( HasFlag( kHideCursorWhileDragging ) )
			return plInputInterface::kCursorHidden;

		return plInputInterface::kCursor4WayDragging;
	}

	return plInputInterface::kCursor4WayDraggable;
}

void	pfGUIDraggableMod::StopDragging( hsBool cancel )
{
	if( fDragging )
	{
		fDragging = false;
		if( cancel )
			HandleExtendedEvent( kCancelled );

		if( HasFlag( kAlwaysSnapBackToStart ) )
			SetObjectCenter( fOrigCenter.fX, fOrigCenter.fY );
	}
}

