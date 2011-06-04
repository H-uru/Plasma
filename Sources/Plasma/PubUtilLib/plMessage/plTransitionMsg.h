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

#ifndef plTransitionMsg_inc
#define plTransitionMsg_inc

#include "hsTypes.h"
#include "hsStream.h"
#include "../pnMessage/plMessageWithCallbacks.h"

class plTransitionMsg : public plMessageWithCallbacks
{
protected:

	UInt32		fEffect;
	hsScalar	fLengthInSecs;
	hsBool		fHoldUntilNext;
public:
	enum 
	{
		kFadeIn,
		kFadeOut,
		kFadeInNoSound,
		kFadeOutNoSound
	};

	plTransitionMsg() : plMessageWithCallbacks(nil, nil, nil), fEffect( 0 ) { SetBCastFlag(kBCastByExactType);  }
	plTransitionMsg( UInt32 type, hsScalar lengthInSecs, hsBool holdUntilNext = false ) : 
				plMessageWithCallbacks(nil, nil, nil), fEffect( type ), fLengthInSecs( lengthInSecs ), fHoldUntilNext( holdUntilNext )
				{ SetBCastFlag( kBCastByExactType );  }
	
	~plTransitionMsg();

	CLASSNAME_REGISTER( plTransitionMsg );
	GETINTERFACE_ANY( plTransitionMsg, plMessageWithCallbacks );

	UInt32		GetEffect( void ) const { return fEffect; }
	hsScalar	GetLengthInSecs( void ) const { return fLengthInSecs; }
	hsBool		GetHoldState( void ) const { return fHoldUntilNext; }

	virtual void Read(hsStream* s, hsResMgr* mgr) 
	{ 
		plMessageWithCallbacks::Read(s, mgr); 
		s->ReadSwap(&fEffect);
		s->ReadSwap(&fLengthInSecs);
		s->ReadSwap(&fHoldUntilNext);
	}
	
	virtual void Write(hsStream* s, hsResMgr* mgr) 
	{ 
		plMessageWithCallbacks::Write(s, mgr); 
		s->WriteSwap(fEffect);
		s->WriteSwap(fLengthInSecs);
		s->WriteSwap(fHoldUntilNext);
	}
};

#endif // plTransitionMsg_inc
