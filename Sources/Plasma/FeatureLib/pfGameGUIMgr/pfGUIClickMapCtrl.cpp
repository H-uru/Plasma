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
//	pfGUIClickMapCtrl Definition											//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "pfGUIClickMapCtrl.h"
#include "pfGameGUIMgr.h"
#include "pfGUIDialogMod.h"

#include "../plInputCore/plInputInterface.h"
#include "../pnMessage/plRefMsg.h"
#include "../pfMessage/pfGameGUIMsg.h"

#include "plgDispatch.h"
#include "hsResMgr.h"


//// Constructor/Destructor //////////////////////////////////////////////////

pfGUIClickMapCtrl::pfGUIClickMapCtrl()
{
	fTracking = false;
	fCustomCursor = -1;
}

pfGUIClickMapCtrl::~pfGUIClickMapCtrl()
{
}

//// IEval ///////////////////////////////////////////////////////////////////

hsBool	pfGUIClickMapCtrl::IEval( double secs, hsScalar del, UInt32 dirty )
{
	return pfGUIControlMod::IEval( secs, del, dirty );
}

//// MsgReceive //////////////////////////////////////////////////////////////

hsBool	pfGUIClickMapCtrl::MsgReceive( plMessage *msg )
{
	return pfGUIControlMod::MsgReceive( msg );
}

//// Read/Write //////////////////////////////////////////////////////////////

void	pfGUIClickMapCtrl::Read( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Read(s, mgr);
}

void	pfGUIClickMapCtrl::Write( hsStream *s, hsResMgr *mgr )
{
	pfGUIControlMod::Write( s, mgr );
}

void	pfGUIClickMapCtrl::HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers )
{
	IScreenToLocalPt( mousePt );
	fLastMousePt = fLastMouseDragPt = mousePt;
	fTracking = true;
}

void	pfGUIClickMapCtrl::HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fTracking )
	{
		IScreenToLocalPt( mousePt );
		fLastMousePt = fLastMouseUpPt = fLastMouseDragPt = mousePt;
		DoSomething();
		fTracking = false;
	}
}

void	pfGUIClickMapCtrl::HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers )
{
	if( fTracking )
	{
		IScreenToLocalPt( mousePt );
		fLastMousePt = fLastMouseDragPt = mousePt;
		if( HasFlag( kReportDragging ) )
			HandleExtendedEvent( kMouseDragged );
	}
}

void	pfGUIClickMapCtrl::HandleMouseHover( hsPoint3 &mousePt, UInt8 modifiers )
{
	IScreenToLocalPt( mousePt );
	fLastMousePt = mousePt;

	if( HasFlag( kReportHovering ) )
		HandleExtendedEvent( kMouseHovered );
}

//// IGetDesiredCursor ///////////////////////////////////////////////////////

UInt32		pfGUIClickMapCtrl::IGetDesiredCursor( void ) const
{
	if( fCustomCursor != -1 )
		return (UInt32)fCustomCursor;

	return plInputInterface::kCursorPoised;
}

