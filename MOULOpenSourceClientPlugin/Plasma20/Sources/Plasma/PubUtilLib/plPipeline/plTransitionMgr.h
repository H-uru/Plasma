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
//	plTransitionMgr - Class to handle fullscreen transitions (fades, etc)	//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plTransitionMgr_h
#define _plTransitionMgr_h

#include "hsTypes.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "hsUtils.h"


//// Class Definition ////////////////////////////////////////////////////////

class plPlate;
class plEventCallbackMsg;

class plTransitionMgr : public hsKeyedObject
{
	protected:

		plPlate		*fEffectPlate;
		
		enum
		{
			kIdle,
			kFadeOut,
			kFadeIn,
			kTransitionFadeIn,
			kTransitionFadeOut
		};

		UInt8		fCurrentEffect;
		hsBool		fRegisteredForTime, fHoldAtEnd, fPlaying, fNoSoundFade;
		hsScalar	fEffectLength, fCurrOpacity, fOpacDelta;
		hsScalar	fLastTime;

		void	IStartFadeIn( hsScalar lengthInSecs, UInt8 effect = kFadeIn );
		void	IStartFadeOut( hsScalar lengthInSecs, UInt8 effect = kFadeOut );

		void	ICreatePlate( void );

		void	IStop( hsBool aboutToStartAgain = false );

		hsTArray<plEventCallbackMsg	*>	fCallbacks;

	public:

		plTransitionMgr();
		virtual ~plTransitionMgr();
		
		CLASSNAME_REGISTER( plTransitionMgr );
		GETINTERFACE_ANY( plTransitionMgr, hsKeyedObject );

		void	Init( void );

		virtual hsBool MsgReceive( plMessage* msg );
};


#endif //_plTransitionMgr_h

