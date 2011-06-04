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
//	plSoundBuffer - Quick, dirty, and highly optimized class for reading	//
//					  in the samples of a WAV file when you're in a hurry.	//
//					  ONLY WORKS WITH PCM (i.e. uncompressed) DATA			//
//																			//
//					Now loads data asynchronously. When fLoading is true	//
//					do not touch any data in the soundbuffer				//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plSoundBuffer_h
#define _plSoundBuffer_h

#include "../pnKeyedObject/hsKeyedObject.h"
#include "plAudioCore.h"
#include "plAudioFileReader.h"
#include "../pnUtils/pnUtils.h"

//// Class Definition ////////////////////////////////////////////////////////

class plUnifiedTime;
class plAudioFileReader;
class plSoundBuffer : public hsKeyedObject
{		
public:
	plSoundBuffer();
	plSoundBuffer( const char *fileName, UInt32 flags = 0 );
	~plSoundBuffer();
	
	CLASSNAME_REGISTER( plSoundBuffer );
	GETINTERFACE_ANY( plSoundBuffer, hsKeyedObject );
	
	LINK(plSoundBuffer) link;

	enum Flags
	{
		kIsExternal			= 0x0001,
		kAlwaysExternal		= 0x0002,
		kOnlyLeftChannel	= 0x0004,
		kOnlyRightChannel	= 0x0008,
		kStreamCompressed	= 0x0010,
	};

	enum ELoadReturnVal
	{
		kSuccess,
		kError,
		kPending,
	};

	void			RoundDataPos( UInt32 &pos );

	virtual void	Read( hsStream *s, hsResMgr *mgr );
	virtual void	Write( hsStream *s, hsResMgr *mgr );

	plWAVHeader	&GetHeader( void )				{ return fHeader; }
	UInt32		GetDataLength( void ) const		{ return fDataLength; }
	void		SetDataLength(unsigned length)  { fDataLength = length; } 
	void		*GetData( void ) const			{ return fData; }
	const char	*GetFileName( void ) const		{ return fFileName; }
	hsBool		IsValid( void ) const			{ return fValid; }
	hsScalar	GetDataLengthInSecs( void ) const;

	void				SetFileName( const char *name );
	hsBool				HasFlag( UInt32 flag ) { return ( fFlags & flag ) ? true : false; }
	void				SetFlag( UInt32 flag, hsBool yes = true ) { if( yes ) fFlags |= flag; else fFlags &= ~flag; }

	// Must be called until return value is kSuccess. starts an asynchronous load first time called. returns kSuccess when finished.
	ELoadReturnVal		AsyncLoad( plAudioFileReader::StreamType type, unsigned length = 0 );	
	void				UnLoad( );

	plAudioCore::ChannelSelect	GetReaderSelect( void ) const;

	
	static void			Init();
	static void			Shutdown();
	plAudioFileReader *	GetAudioReader();	// transfers ownership to caller
	void				SetAudioReader(plAudioFileReader *reader);
	void				SetLoaded(bool loaded);

	plAudioFileReader::StreamType	GetAudioReaderType() { return fStreamType; }
	unsigned						GetAsyncLoadLength() { return fAsyncLoadLength ? fAsyncLoadLength : fDataLength; }

	// for plugins only
	void				SetInternalData( plWAVHeader &header, UInt32 length, UInt8 *data );
	ELoadReturnVal		EnsureInternal( );	
	void				SetError() { fError = true; }

protected:

	// plSoundBuffers can be two ways--they can either have a filename and no
	// data, in which case they reference a file in the sfx folder, or they
	// can store the data directly
	
	void			IInitBuffer();

	hsBool			IGrabHeaderInfo( void );
	void			IAddBuffers( void *base, void *toAdd, UInt32 lengthInBytes, UInt8 bitsPerSample );
	void			IGetFullPath( char *destStr );
	
	UInt32			fFlags;
	hsBool			fValid;
	UInt32			fDataRead;
	char			*fFileName;
	
	bool			fLoaded;
	bool			fLoading;
	bool			fError;
	
	plAudioFileReader *	fReader;	
	UInt8 *				fData;
	plWAVHeader			fHeader;
	UInt32				fDataLength;
	UInt32				fAsyncLoadLength;
	plAudioFileReader::StreamType fStreamType;

	// for plugins only
	plAudioFileReader	*IGetReader( hsBool fullpath );
};

#endif //_plSoundBuffer_h
