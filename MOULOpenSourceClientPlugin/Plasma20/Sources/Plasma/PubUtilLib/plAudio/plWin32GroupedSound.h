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
//	plWin32GroupedSound - Grouped version of a static sound. Lots of short	//
//						  sounds stored in the buffer, all share the same	//
//						  DSound playback buffer and only one plays at a	//
//						  time.												//
//																			//
//////////////////////////////////////////////////////////////////////////////

#ifndef plWin32GroupedSound_h
#define plWin32GroupedSound_h

#include "plWin32StaticSound.h"

class hsResMgr;
class plDSoundBuffer;
class plEventCallbackMsg;

#include "plSoundEvent.h"

class plWin32GroupedSound : public plWin32StaticSound
{
public:
	plWin32GroupedSound();
	~plWin32GroupedSound();

	CLASSNAME_REGISTER( plWin32GroupedSound );
	GETINTERFACE_ANY( plWin32GroupedSound, plWin32StaticSound );
	
	virtual hsBool	LoadSound( hsBool is3D );
	virtual hsBool	MsgReceive( plMessage *pMsg );
	void			SetPositionArray( UInt16 numSounds, UInt32 *posArray, hsScalar *volumeArray );
	hsScalar		GetSoundLength( Int16 soundIndex );
	virtual double	GetLength() { return GetSoundLength( fCurrentSound ); }

protected:
	UInt16				fCurrentSound;
	UInt32				fCurrentSoundLength;
	hsTArray<UInt32>	fStartPositions;	// In bytes
	hsTArray<hsScalar>	fVolumes;

	// Some extra handy info for us
	UInt8				fNumDestChannels, fNumDestBytesPerSample;

	virtual void	IDerivedActuallyPlay( void );

	virtual void	IRead( hsStream *s, hsResMgr *mgr );
	virtual void	IWrite( hsStream *s, hsResMgr *mgr );

	UInt32			IGetSoundByteLength( Int16 soundIndex );
	void			IFillCurrentSound( Int16 newCurrent = -1 );
	
	// Abstracting a few things here for the incidentalMgr
	virtual void *	IGetDataPointer( void ) const; 
	virtual UInt32	IGetDataLength( void ) const;
};

#endif //plWin32GroupedSound_h
