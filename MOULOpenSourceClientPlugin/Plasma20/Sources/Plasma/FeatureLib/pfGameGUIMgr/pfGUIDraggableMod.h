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
//	pfGUIDraggableMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIDraggableMod_h
#define _pfGUIDraggableMod_h

#include "pfGUIControlMod.h"

class plMessage;

class pfGUIDraggableMod : public pfGUIControlMod
{
	protected:

		hsPoint3	fDragOffset, fLastMousePt;
		hsPoint3	fOrigCenter;
		hsBool		fDragging;

		
		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		virtual UInt32		IGetDesiredCursor( void ) const;	// As specified in plInputInterface.h

	public:

		pfGUIDraggableMod();
		virtual ~pfGUIDraggableMod();

		CLASSNAME_REGISTER( pfGUIDraggableMod );
		GETINTERFACE_ANY( pfGUIDraggableMod, pfGUIControlMod );

		enum OurFlags
		{
			kReportDragging = kDerivedFlagsStart,
			kHideCursorWhileDragging,
			kAlwaysSnapBackToStart
		};

		// Extended event types (endDrag is the default event)
		enum ExtendedEvents
		{
			kDragging,
			kCancelled,
			kStartingDrag
		};

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );

		virtual void	UpdateBounds( hsMatrix44 *invXformMatrix = nil, hsBool force = false );

		void			StopDragging( hsBool cancel );
		const hsPoint3	&GetLastMousePt( void ) const { return fLastMousePt; }
};

#endif // _pfGUIDraggableMod_h
