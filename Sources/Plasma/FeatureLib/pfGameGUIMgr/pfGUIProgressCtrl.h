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
//	pfGUIProgressCtrl Header												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _pfGUIProgressCtrl_h
#define _pfGUIProgressCtrl_h

#include "pfGUIValueCtrl.h"

class plMessage;
class plAGMasterMod;

class pfGUIProgressCtrl : public pfGUIValueCtrl
{
	protected:

		hsTArray<plKey>	fAnimationKeys;
		char			*fAnimName;

						// Computed once, once an anim is loaded that we can compute this with
		hsScalar		fAnimBegin, fAnimEnd;
		hsBool			fAnimTimesCalced;
		hsBool			fPlaySound;

		virtual hsBool IEval( double secs, hsScalar del, UInt32 dirty ); // called only by owner object's Eval()

		hsBool			ICalcAnimTimes( void );

		const UInt32	fStopSoundTimer;

	public:

		pfGUIProgressCtrl();
		virtual ~pfGUIProgressCtrl();

		CLASSNAME_REGISTER( pfGUIProgressCtrl );
		GETINTERFACE_ANY( pfGUIProgressCtrl, pfGUIValueCtrl );


		enum OurFlags
		{
			kReverseValues = kDerivedFlagsStart
		};

		virtual hsBool	MsgReceive( plMessage* pMsg );
		
		virtual void Read( hsStream* s, hsResMgr* mgr );
		virtual void Write( hsStream* s, hsResMgr* mgr );

		virtual void	UpdateBounds( hsMatrix44 *invXformMatrix = nil, hsBool force = false );

		virtual void	SetCurrValue( hsScalar v );
		virtual void	AnimateToPercentage( hsScalar percent );

		enum SoundEvents
		{
			kAnimateSound
		};
		
		void DontPlaySounds() { fPlaySound = false; }

		// Export only
		void	SetAnimationKeys( hsTArray<plKey> &keys, const char *name );
};

#endif // _pfGUIProgressCtrl_h
