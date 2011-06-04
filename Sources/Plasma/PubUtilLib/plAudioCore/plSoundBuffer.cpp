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
#include <process.h>
#include "hsTypes.h"
#include "plSoundBuffer.h"

#include "hsStream.h"
#include "hsUtils.h"

#include "plgDispatch.h"
#include "hsResMgr.h"
#include "../pnMessage/plRefMsg.h"
#include "../plFile/plFileUtils.h"
#include "../plFile/hsFiles.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../pnUtils/pnUtils.h"
#include "../plStatusLog/plStatusLog.h"
#include "hsTimer.h"

static bool s_running;
static LISTDECL(plSoundBuffer, link) s_loading;
static CCritSect s_critsect;
static CEvent s_event(kEventAutoReset);

static void GetFullPath( const char filename[], char *destStr )
{
	char	path[ kFolderIterator_MaxPath ];

	if( strchr( filename, '\\' ) != nil )
		strcpy( path, filename );
	else
		sprintf( path, "sfx\\%s", filename );

	strcpy( destStr, path );
}

//// IGetReader //////////////////////////////////////////////////////////////
//	Makes sure the sound is ready to load without any extra processing (like
//	decompression or the like), then opens a reader for it.
//  fullpath tells the function whether to append 'sfx' to the path or not (we don't want to do this if were providing the full path)
static plAudioFileReader *CreateReader( hsBool fullpath, const char filename[], plAudioFileReader::StreamType type, plAudioCore::ChannelSelect channel )
{
	char path[512];
	if(fullpath) GetFullPath(filename, path);
	else
		strcpy(path, filename);
	
	plAudioFileReader* reader = plAudioFileReader::CreateReader(path, channel, type);

	if( reader == nil || !reader->IsValid() )
	{
		delete reader;
		return nil;
	}

	return reader;
}

// our loading thread
static void LoadCallback(void *)
{
	LISTDECL(plSoundBuffer, link) templist;
	while(s_running)
	{
		s_critsect.Enter(); 
		{
			while(plSoundBuffer *buffer = s_loading.Head())
			{
				templist.Link(buffer);
			}
		}
		s_critsect.Leave();
		
		if(!templist.Head())
		{
			s_event.Wait(kEventWaitForever);
		}
		else
		{
			plAudioFileReader *reader = nil;
			while(plSoundBuffer *buffer = templist.Head())
			{		
				if(buffer->GetData())
				{
					reader = CreateReader(true, buffer->GetFileName(), buffer->GetAudioReaderType(), buffer->GetReaderSelect());  
					
					if( reader )
					{
						unsigned readLen = buffer->GetAsyncLoadLength() ? buffer->GetAsyncLoadLength() : buffer->GetDataLength(); 
						reader->Read( readLen, buffer->GetData() );
						buffer->SetAudioReader(reader);		// give sound buffer reader, since we may need it later
					}
					else
						buffer->SetError();
				}
				
				templist.Unlink(buffer);
				buffer->SetLoaded(true);
			}
		}
	}

	// we need to be sure that all buffers are removed from our load list when shutting this thread down or we will hang,
	// since the sound buffer will wait to be destroyed until it is marked as loaded 
	s_critsect.Enter();
	{
		while(plSoundBuffer *buffer = s_loading.Head())
		{
			buffer->SetLoaded(true);
			s_loading.Unlink(buffer);
		}
	}
	s_critsect.Leave();
}

void plSoundBuffer::Init()
{
	s_running = true;
	_beginthread(LoadCallback, 0, 0);
}

void plSoundBuffer::Shutdown()
{
	s_running = false;
	s_event.Signal();
}

//// Constructor/Destructor //////////////////////////////////////////////////

plSoundBuffer::plSoundBuffer() 
{	
	IInitBuffer();
}

plSoundBuffer::plSoundBuffer( const char *fileName, UInt32 flags ) 
{
	IInitBuffer();
	SetFileName( fileName );
	fFlags = flags;
	fValid = IGrabHeaderInfo();
}

plSoundBuffer::~plSoundBuffer()
{ 
	// if we are loading a sound we need to wait for the loading thread to be completely done processing this buffer.
	// otherwise it may try to access this buffer after it's been deleted
	if(fLoading)
	{
		while(!fLoaded)
		{
			Sleep(10);
		}
	}

	ASSERT(!link.IsLinked());
	delete [] fFileName;
	UnLoad();
}

void plSoundBuffer::IInitBuffer()
{
	fError = false;
	fValid = false;
	fFileName = nil;
	fData = nil;
	fDataLength = 0;
	fFlags = 0;
	fDataRead = 0;
	fReader = nil;
	fLoaded = 0;
	fLoading = false;
	fHeader.fFormatTag = 0;
	fHeader.fNumChannels = 0;
	fHeader.fNumSamplesPerSec = 0;
	fHeader.fAvgBytesPerSec = 0;
	fHeader.fBlockAlign = 0;
	fHeader.fBitsPerSample = 0;
}

//// GetDataLengthInSecs /////////////////////////////////////////////////////

hsScalar	plSoundBuffer::GetDataLengthInSecs( void ) const
{
	return (hsScalar)fDataLength / (hsScalar)fHeader.fAvgBytesPerSec;
}

//// Read/Write //////////////////////////////////////////////////////////////

void	plSoundBuffer::Read( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Read( s, mgr );

	s->ReadSwap( &fFlags );
	s->ReadSwap( &fDataLength );
	fFileName = s->ReadSafeString();

	s->ReadSwap( &fHeader.fFormatTag );
	s->ReadSwap( &fHeader.fNumChannels );
	s->ReadSwap( &fHeader.fNumSamplesPerSec );
	s->ReadSwap( &fHeader.fAvgBytesPerSec );
	s->ReadSwap( &fHeader.fBlockAlign );
	s->ReadSwap( &fHeader.fBitsPerSample );

	fValid = false;
	if( !( fFlags & kIsExternal ) )
	{
		fData = TRACKED_NEW UInt8[ fDataLength ];
		if( fData == nil )
			fFlags |= kIsExternal;
		else
		{
			s->Read( fDataLength, fData );
			fValid = true;
			SetLoaded(true);
		}
	}
	else
	{
		fData = nil;
//		fValid = EnsureInternal();
		fValid = true;
	}
}

void	plSoundBuffer::Write( hsStream *s, hsResMgr *mgr )
{
	hsKeyedObject::Write( s, mgr );

	if( fData == nil )
		fFlags |= kIsExternal;

	s->WriteSwap( fFlags );
	s->WriteSwap( fDataLength );
	
	// Truncate the path to just a file name on write
	if( fFileName != nil )
	{
		char *nameOnly = strrchr( fFileName, '\\' );
		if( nameOnly != nil )
			s->WriteSafeString( nameOnly + 1 );
		else
			s->WriteSafeString( fFileName );
	}
	else
		s->WriteSafeString( nil );

	s->WriteSwap( fHeader.fFormatTag );
	s->WriteSwap( fHeader.fNumChannels );
	s->WriteSwap( fHeader.fNumSamplesPerSec );
	s->WriteSwap( fHeader.fAvgBytesPerSec );
	s->WriteSwap( fHeader.fBlockAlign );
	s->WriteSwap( fHeader.fBitsPerSample );

	if( !( fFlags & kIsExternal ) )
		s->Write( fDataLength, fData );
}

//// SetFileName /////////////////////////////////////////////////////////////

void	plSoundBuffer::SetFileName( const char *name )
{
	if(fLoading)
	{
		hsAssert(false, "Unable to set SoundBuffer filename");
		return;
	}

	delete [] fFileName;
	if( name != nil )
		fFileName = hsStrcpy( name );
	else
		fFileName = nil;

	// Data is no longer valid
	UnLoad();
}


//// GetReaderSelect /////////////////////////////////////////////////////////
//	Translates our flags into the ChannelSelect enum for plAudioFileReader

plAudioCore::ChannelSelect	plSoundBuffer::GetReaderSelect( void ) const
{
	if( fFlags & kOnlyLeftChannel )
		return plAudioCore::kLeft;
	else if( fFlags & kOnlyRightChannel )
		return plAudioCore::kRight;
	else
		return plAudioCore::kAll;
}

//// IGetFullPath ////////////////////////////////////////////////////////////
//	Construct our current full path to our sound.

void	plSoundBuffer::IGetFullPath( char *destStr )
{
	if(!fFileName)
	{
		*destStr = 0;
		return;
	}
	char	path[ kFolderIterator_MaxPath ];


	if( strchr( fFileName, '\\' ) != nil )
		strcpy( path, fFileName );
	else
		sprintf( path, "sfx\\%s", fFileName );

	strcpy( destStr, path );
}


//============================================================================
// Asyncload will queue up a buffer for loading in our loading list the first time it is called.
// It will load in "length" number of bytes, if length is non zero. If length is zero the entire file will be loaded
// When called subsequent times it will check to see if the data has been loaded.
// Returns kPending while still loading the file. Returns kSuccess when the data has been loaded.
// While a file is loading(fLoading == true, and fLoaded == false) a buffer, no paremeters of the buffer should be modified.
plSoundBuffer::ELoadReturnVal plSoundBuffer::AsyncLoad(plAudioFileReader::StreamType type, unsigned length /* = 0 */ )
{
	if(!s_running)
		return kError;	// we cannot load the data since the load thread is no longer running
	if(!fLoading && !fLoaded)
	{
		fAsyncLoadLength = length;
		fStreamType = type;
		if(fData == nil )
		{
			fData = TRACKED_NEW UInt8[ fAsyncLoadLength ? fAsyncLoadLength : fDataLength ];
			if( fData == nil )
				return kError;
		}
		s_critsect.Enter();
		{
			fLoading = true;
			s_loading.Link(this);
		}
		s_critsect.Leave();
		s_event.Signal();
	}
	if(fLoaded) 
	{	
		if(fLoading)	// ensures we only do this stuff one time
		{
			ELoadReturnVal retVal = kSuccess;
			if(fError)
			{
				retVal = kError;
				fError = false;
			}
			if(fReader)
			{
				fHeader = fReader->GetHeader();
				SetDataLength(fReader->GetDataSize());
			}

			fFlags &= ~kIsExternal;
			fLoading = false;
			return retVal;
		}
		return kSuccess;
	}
	
	return kPending;
}

//// ForceNonInternal ////////////////////////////////////////////////////////
// destroys loaded, and frees data
void	plSoundBuffer::UnLoad( void )
{
	if(fLoaded)
		int i = 0;
	if(fLoading) 
		return;

	if(fReader)
		fReader->Close();

	delete fReader;
	fReader = nil;

	delete [] fData;
	fData = nil;
	SetLoaded(false);
	fFlags |= kIsExternal;
	
}

//// IRoundDataPos ///////////////////////////////////////////////////////////

void	plSoundBuffer::RoundDataPos( UInt32 &pos )
{
	UInt32 extra = pos & ( fHeader.fBlockAlign - 1 );
	pos -= extra;
}

// transfers ownership to caller
plAudioFileReader *plSoundBuffer::GetAudioReader() 
{ 
	plAudioFileReader * reader = fReader;
	fReader = nil; 
	return reader; 
}		
 	
// WARNING:  called by the loader thread(only) 
// the reader will be handed off for later use. This is useful for streaming sound if we want to load the first chunk of data 
//  and the continue streaming the file from disk.
void plSoundBuffer::SetAudioReader(plAudioFileReader *reader)
{
	if(fReader)
		fReader->Close();
	delete fReader;
	fReader = reader;
}

void plSoundBuffer::SetLoaded(bool loaded)
{
	fLoaded = loaded;
}

		
/*****************************************************************************
*
*   for plugins only
*
***/

//// SetInternalData /////////////////////////////////////////////////////////

void	plSoundBuffer::SetInternalData( plWAVHeader &header, UInt32 length, UInt8 *data )
{
	if(fLoading) return;
	fFileName = nil;
	fHeader = header;
	fFlags = 0;

	fDataLength = length;
	fData = TRACKED_NEW UInt8[ length ];
	memcpy( fData, data, length );
	
	fValid = true;
}

//// EnsureInternal //////////////////////////////////////////////////////////
// for plugins only
plSoundBuffer::ELoadReturnVal plSoundBuffer::EnsureInternal()
{	
	if( fData == nil )
	{
		fData = TRACKED_NEW UInt8[fDataLength ];
		if( fData == nil )
			return kError;
	}
	if(!fReader)
		fReader = IGetReader(true);
	//else
	//	fReader->Open();

	if( fReader == nil )
		return kError;
	
	unsigned readLen = fDataLength; 
	if( !fReader->Read( readLen, fData ) )
	{
		delete [] fData;
		fData = nil;
		return kError;
	}
	
	if(fReader)
	{
		fReader->Close();
		delete fReader;
		fReader = nil;
	}
	return kSuccess;
}

//// IGrabHeaderInfo /////////////////////////////////////////////////////////
hsBool	plSoundBuffer::IGrabHeaderInfo( void )
{
	static char	path[ 512 ];

	if( fFileName != nil )
	{
		IGetFullPath( path );

		// Go grab from the WAV file
		if(!fReader)
		{
			fReader = plAudioFileReader::CreateReader(path, GetReaderSelect(), plAudioFileReader::kStreamNative);
			if( fReader == nil || !fReader->IsValid() )
			{
				delete fReader;
				fReader = nil;
				return false;
			}
		}

		fHeader = fReader->GetHeader();
		fDataLength = fReader->GetDataSize();
		RoundDataPos( fDataLength );

		fReader->Close();
		delete fReader;
		fReader = nil;
	}

	return true;
}

//// IGetReader //////////////////////////////////////////////////////////////
//	Makes sure the sound is ready to load without any extra processing (like
//	decompression or the like), then opens a reader for it.
//  fullpath tells the function whether to append 'sfx' to the path or not (we don't want to do this if were providing the full path)
plAudioFileReader	*plSoundBuffer::IGetReader( hsBool fullpath )
{
	char path[512];
	if(fullpath) IGetFullPath(path);
	else
		strcpy(path, fFileName);

	// Go grab from the WAV file
	plAudioFileReader::StreamType type = plAudioFileReader::kStreamWAV;
	if (HasFlag(kStreamCompressed))
		type = plAudioFileReader::kStreamNative;
	
	plAudioFileReader* reader = plAudioFileReader::CreateReader(path, GetReaderSelect(), type);

	if( reader == nil || !reader->IsValid() )
	{
		delete reader;
		return nil;
	}

	fHeader = reader->GetHeader();
	fDataLength = reader->GetDataSize();
	RoundDataPos( fDataLength );

	return reader;
}