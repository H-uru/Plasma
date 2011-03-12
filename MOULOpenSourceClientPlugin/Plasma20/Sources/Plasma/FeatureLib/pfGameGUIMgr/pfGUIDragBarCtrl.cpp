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
//	pfGUIDragBarCtrl Definition												//
//																			//
//	DragBars are draggable controls that take their dialogs along with		//
//	them. Because they're essentially part of the dialog directly (the part	//
//	that can be dragged), they're processed after the normal hit testing.	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIDragBarCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "../plInputCore/plInputInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"
#include "../plMessage/plAnimCmdMsg.h"
#include "../plAvatar/plAGModifier.h"
#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIDragBarCtrl::pfGUIDragBarCtrl()
{
	SetFlag( kWantsInterest );
	fDragging = false;
	fAnchored = false;
}

pfGUIDragBarCtrl::~pfGUIDragBarCtrl()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIDragBarCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIDragBarCtrl::MsgReceive( plMessage *msg )
{
	return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIDragBarCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);
}

void	pfGUIDragBarCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );
}

//// UpdateBounds ////////////////////////////////////////////////////////////

void	pfGUIDragBarCtrl::UpdateBounds( hsMatrix44 *invXformMatrix, hsBool force )
{
	pfGUIControlMod::UpdateBounds( invXformMatrix, force );
	fBoundsValid = false;
}

//// HandleMouseDown/Up //////////////////////////////////////////////////////

void	pfGUIDragBarCtrl::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	// if we are anchored <to the floor> then don't let it be moved
	if ( fAnchored )
		return;

	fDragging = true;
	fDragOffset = fScreenCenter - mousePt;

	SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );

	// We know that the entire dialog is going to move, so we better make 
	// sure to update the bounds on all the controls
	fDialog->UpdateAllBounds();
}

void	pfGUIDragBarCtrl::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	// if we are anchored <to the floor> then don't let it be moved
	if ( fAnchored )
		return;

	fDragging = false;
	SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );
	fDialog->UpdateAllBounds();
}

void	pfGUIDragBarCtrl::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	// if we are anchored <to the floor> then don't let it be moved
	if ( fAnchored )
		return;

	SetObjectCenter( mousePt.fX + fDragOffset.fX, mousePt.fY + fDragOffset.fY );
	fDialog->UpdateAllBounds();
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUIDragBarCtrl::IGetDesiredCursor( void ) const
{
	// if we are anchored, then no cursors that say we can move
	if ( fAnchored )
		return 0;

	if( fDragging )
		return plInputInterface::kCursor4WayDragging;

	return plInputInterface::kCursor4WayDraggable;
}

