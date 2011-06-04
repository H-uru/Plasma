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
#ifndef plWin32StreamingSound_h
#define plWin32StreamingSound_h

#include "plWin32Sound.h"
#include "../pnUtils/pnUtils.h"

class plDSoundBuffer;
class DSoundCallbackHandle;
class plAudioFileReader;
class plStreamingSoundThread;
enum CallbackHandleType;
class plSoundDeswizzler;

class plWin32StreamingSound : public plWin32Sound
{
public:
	plWin32StreamingSound();
	~plWin32StreamingSound();

	CLASSNAME_REGISTER( plWin32StreamingSound );
	GETINTERFACE_ANY( plWin32StreamingSound, plWin32Sound );

	virtual void		DeActivate();
	virtual hsBool		LoadSound( hsBool is3D );
	virtual float		GetActualTimeSec();
	virtual unsigned	GetByteOffset();
	virtual StreamType	GetStreamType() const { return fStreamType; }
	virtual void		SetFilename(const char *filename, bool isCompressed);
	virtual void		Update();	// temp
	void				StreamUpdate();
	virtual hsBool		MsgReceive( plMessage *pMsg );
	
protected:
	hsScalar			fTimeAtBufferStart;
	plAudioFileReader	*fDataStream;
	hsScalar			fBufferLengthInSecs;
	UInt8				fBlankBufferFillCounter;
	plSoundDeswizzler	*fDeswizzler;
	char				fSrcFilename[ 256 ];
	StreamType			fStreamType;
	bool				fIsCompressed;		// this applies only to the new sound file specified in fNewFilename, so we can play both ogg's and wav's
	std::string			fNewFilename;		// allow the filename to be changed so we can play from a different source.
											// ultimately this filename will be given to fDataBuffer, but since it's not always around we'll store it here
	hsBool				fStopping;	

	double				fLastStreamingUpdate;
	bool				fPlayWhenStopped;
	unsigned			fStartPos;

	hsScalar			IGetTimeAtBufferStart( void ) { return fTimeAtBufferStart; }
	virtual void		SetStartPos(unsigned bytes);

	virtual void		IDerivedActuallyPlay( void );
	void				IActuallyStop();
	virtual void		ISetActualTime( double t );
	
	virtual void		IAddCallback( plEventCallbackMsg *pMsg );
	virtual void		IRemoveCallback( plEventCallbackMsg *pMsg );

	virtual void		IFreeBuffers( void );
	void				IStreamUpdate();
	virtual plSoundBuffer::ELoadReturnVal IPreLoadBuffer( hsBool playWhenLoaded, hsBool isIncidental = false  );
};

#endif //plWin32StreamingSound_h
