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
#ifndef plWin32Sound_h
#define plWin32Sound_h

#include "hsTemplates.h"
#include "plSound.h"
#include "hsThread.h"
#include "plSoundEvent.h"

#define NUM_MAX_HANDLES		16
#define REPEAT_INFINITE		0xffffffff

struct hsVector3;
class hsResMgr;
class plEventCallbackMsg;
class plSoundMsg;
class DSoundCallbackHandle;
class plSoundEvent;
class plDSoundBuffer;

class plWin32Sound : public plSound
{
public:

	plWin32Sound();
	virtual ~plWin32Sound();

	CLASSNAME_REGISTER( plWin32Sound );
	GETINTERFACE_ANY( plWin32Sound, plSound );
	
	virtual void		Activate(hsBool forcePlay = false);
	virtual void		DeActivate();

	virtual void		AddCallbacks(plSoundMsg* pMsg);
	virtual void		RemoveCallbacks(plSoundMsg* pMsg);

	virtual plSoundMsg*	GetStatus(plSoundMsg* pMsg);
	virtual hsBool		MsgReceive(plMessage* pMsg);
	virtual void		Update();
	
	virtual void 	SetMin(const int m); // sets minimum falloff distance
	virtual void 	SetMax(const int m); // sets maximum falloff distance
	virtual void	SetConeOrientation(hsScalar x, hsScalar y, hsScalar z);
	virtual void	SetOuterVolume( const int v ); // volume for the outer cone (if applicable)
	virtual void	SetConeAngles( int inner, int outer );
	virtual void	SetPosition(const hsPoint3 pos);
	virtual void	SetVelocity(const hsVector3 vel);

	enum ChannelSelect
	{
		kLeftChannel,
		kRightChannel
	};

	// Selects a channel source from a multi-channel (stereo) file. Ignored if source is mono
	void			SetChannelSelect( ChannelSelect source ) { fChannelSelect = (UInt8)source; }
	virtual UInt8	GetChannelSelect( void ) const { return fChannelSelect; }
	
protected:

	plDSoundBuffer *	fDSoundBuffer;

	hsBool				fFailed;
	hsBool				fPositionInited, fAwaitingPosition;
	hsBool				fReallyPlaying;
	UInt32				fTotalBytes;

	hsBool				fWasPlaying;
	
	UInt8				fChannelSelect;		// For selecting a mono channel from a stereo file

	hsTArray<plSoundEvent *>	fSoundEvents;

	virtual void	ISetActualVolume(const float v);
	virtual void	IActuallyStop( void );
	virtual hsBool	IActuallyPlaying( void ) { return fReallyPlaying; }
	virtual void	IActuallyPlay( void );
	virtual void	IFreeBuffers( void );
	virtual hsBool	IActuallyLoaded( void ) { return ( fDSoundBuffer != nil ) ? true : false; }

	// Override to make sure the buffer is available before the base class is called
	virtual void	IRefreshParams( void );

	virtual void	IDerivedActuallyPlay( void ) = 0;

	virtual void	IAddCallback( plEventCallbackMsg *pMsg );
	virtual void	IRemoveCallback( plEventCallbackMsg *pMsg );
	plSoundEvent	*IFindEvent( plSoundEvent::Types type, UInt32 bytePos = 0 );

	virtual void	IRead( hsStream *s, hsResMgr *mgr );
	virtual void	IWrite( hsStream *s, hsResMgr *mgr );

	virtual void	IRefreshEAXSettings( hsBool force = false );
};

#endif //plWin32Sound_h
