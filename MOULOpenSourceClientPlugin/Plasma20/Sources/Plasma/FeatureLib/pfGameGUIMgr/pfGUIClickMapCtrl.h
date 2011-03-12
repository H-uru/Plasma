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
//	pfGUIClickMapCtrl Header												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIClickMapCtrl_h
#define _pfGUIClickMapCtrl_h

#include "pfGUIControlMod.h"

class plMessage;

class pfGUIClickMapCtrl : public pfGUIControlMod
{
	protected:

		hsPoint3		fLastMousePt, fLastMouseUpPt, fLastMouseDragPt;
		hsBool			fTracking;
		Int32			fCustomCursor;

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		virtual UInt32		IGetDesiredCursor( void ) const;	// As specified in plInputInterface.h

	public:

		pfGUIClickMapCtrl();
		virtual ~pfGUIClickMapCtrl();

		CLASSNAME_REGISTER( pfGUIClickMapCtrl );
		GETINTERFACE_ANY( pfGUIClickMapCtrl, pfGUIControlMod );

		enum OurFlags
		{
			kReportDragging = kDerivedFlagsStart,
			kReportHovering
		};

		// Extended event types
		enum ExtendedEvents
		{
			kMouseDragged,
			kMouseHovered
		};

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseHover( hsPoint3 &mousePt, UInt8 modifiers );

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		const hsPoint3	&GetLastMousePt( void ) const { return fLastMousePt; }
		const hsPoint3	&GetLastMouseUpPt( void ) const { return fLastMouseUpPt; }
		const hsPoint3	&GetLastMouseDragPt( void ) const { return fLastMouseDragPt; }

		void	SetCustomCursor( Int32 cursor = -1 ) { fCustomCursor = cursor; }
};

#endif // _pfGUIClickMapCtrl_h
