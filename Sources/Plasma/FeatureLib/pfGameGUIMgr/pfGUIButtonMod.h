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
//	pfGUIButtonMod Header													//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIButtonMod_h
#define _pfGUIButtonMod_h

#include "pfGUIControlMod.h"

class plMessage;
class plPostEffectMod;
class plAGMasterMod;
class pfGUIDraggableMod;

class pfGUIButtonMod : public pfGUIControlMod
{
	protected:

		hsTArray<plKey>	fAnimationKeys;
		char			*fAnimName;

		hsTArray<plKey>	fMouseOverAnimKeys;
		char			*fMouseOverAnimName;

		hsBool			fClicking;
		hsBool			fTriggering;

		hsPoint3			fOrigMouseDownPt;
		pfGUIDraggableMod	*fDraggable;
		pfGUICtrlProcObject	*fOrigHandler;
		hsBool				fOrigReportedDrag;


		Int32			fNotifyType;

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		virtual UInt32		IGetDesiredCursor( void ) const;	// As specified in plInputInterface.h

	public:

		pfGUIButtonMod();
		virtual ~pfGUIButtonMod();

		CLASSNAME_REGISTER( pfGUIButtonMod );
		GETINTERFACE_ANY( pfGUIButtonMod, pfGUIControlMod );


		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	SetInteresting( hsBool i );

		virtual void	HandleMouseDown( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseUp( hsPoint3 &mousePt, UInt8 modifiers );
		virtual void	HandleMouseDrag( hsPoint3 &mousePt, UInt8 modifiers );

		virtual void	UpdateBounds( hsMatrix44 *invXformMatrix = nil, hsBool force = false );

		virtual void	SetNotifyType(Int32 kind);
		virtual Int32	GetNotifyType();
		virtual hsBool	IsButtonDown();
		virtual hsBool	IsTriggering() { return fTriggering; }
		enum SoundEvents
		{
			kMouseDown,
			kMouseUp,
			kMouseOver,
			kMouseOff
		};

		enum
		{
			kRefDraggable = kRefDerivedStart
		};

		enum NotifyType
		{
			kNotifyOnUp = 0,
			kNotifyOnDown,
			kNotifyOnUpAndDown
		};

		void	StartDragging( void );
		void	StopDragging( hsBool cancel );

		// Export only
		void	SetAnimationKeys( hsTArray<plKey> &keys, const char *name );
		void	SetMouseOverAnimKeys( hsTArray<plKey> &keys, const char *name );
};

#endif // _pfGUIButtonMod_h
