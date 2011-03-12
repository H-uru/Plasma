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
//	plWAVClipBuffer - Helper class for writing out WAV data in a buffered	//
//					  manner, with support for clipping off the specified	//
//					  amount at the end, but without knowing beforehand		//
//					  exactly how much data we'll have.						//
//																			//
//	The algorithm goes something like this: we keep two buffers, both the	//
//	size of the amount we want to clip. We then start filling in the first	//
//	buffer, overflowing into the second buffer and wrapping back to the		//
//	first again in a circular fashion. When we fill up one buffer and are	//
//	about to advance to the next, we write that next buffer out. Why?		//
//	Because we know that, even if we got no more data in, we have enough	//
//	data in the first buffer to clip out the amount we want, so the other	//
//	half (which will have older data, being a circular buffer) can be		//
//	written out safely.														//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "plWAVClipBuffer.h"
#include "hsStream.h"
#include "hsUtils.h"

#include "plWavFile.h"


//// Constructor/Destructor //////////////////////////////////////////////////

plWAVClipBuffer::plWAVClipBuffer( UInt32 clipSize, CWaveFile *outFile )
{
	fBuffers[ 0 ] = fBuffers[ 1 ] = nil;
	fFlushCalled = true;
	Init( clipSize, outFile );
}

plWAVClipBuffer::~plWAVClipBuffer()
{
	IShutdown();
}

//// Init & IShutdown ////////////////////////////////////////////////////////

void	plWAVClipBuffer::Init( UInt32 clipSize, CWaveFile *outFile )
{
	IShutdown();
	if( clipSize > 0 )
	{
		fBuffers[ 0 ] = TRACKED_NEW UInt8[ clipSize ];
		fBuffers[ 1 ] = TRACKED_NEW UInt8[ clipSize ];
		memset( fBuffers[ 0 ], 0, clipSize );
		memset( fBuffers[ 1 ], 0, clipSize );
	}
	fWhichBuffer = 0;
	fBufferSize = clipSize;
	fCursor = 0;
	fFirstFlip = true;
	fOutFile = outFile;
	fFlushCalled = false;
}

void	plWAVClipBuffer::IShutdown( void )
{
	hsAssert( fFlushCalled, "WAVClipBuffer shut down without flushing it!!!" );

	delete [] fBuffers[ 0 ];
	delete [] fBuffers[ 1 ];
}

//// WriteData ///////////////////////////////////////////////////////////////
//	The main workhorse; call this to add data to the buffer.

hsBool	plWAVClipBuffer::WriteData( UInt32 size, UInt8 *data )
{
	while( size > 0 )
	{
		UInt32	toWrite = fBufferSize - fCursor;
		if( size < toWrite )
		{
			// Just write, haven't filled a buffer yet
			memcpy( fBuffers[ fWhichBuffer ] + fCursor, data, size );
			data += size;
			fCursor += size;
			return true;	// All done!
		}

		// Fill up to the end of a buffer, then flip
		memcpy( fBuffers[ fWhichBuffer ] + fCursor, data, toWrite );
		data += toWrite;
		fCursor += toWrite;
		size -= toWrite;

		// Flip now...
		fWhichBuffer = 1 - fWhichBuffer;
		fCursor = 0;

		// Now we can write out this buffer, since it'll be old data and
		// we have enough in the other buffer to clip with. The *only* 
		// time we don't want to do this is the first time we flip, since
		// at that point, the buffer we just flipped to hasn't been filled yet.
		// (Every time afterwards, we'll always be flipping to a buffer with old
		// data).
		if( fFirstFlip )
			fFirstFlip = false;
		else
		{
			// Write it out before we overwrite it!
			UINT written;
			HRESULT hr = fOutFile->Write( fBufferSize, fBuffers[ fWhichBuffer ], &written );

			if( FAILED( hr ) )
			{
				hsAssert( false, "ERROR writing WMA stream to WAV file" );
				return false;
			}
			else if( written != fBufferSize )
			{
				hsAssert( false, "Unable to write all of WMA stream to WAV file" );
				return false;
			}
		}
	}

	// Cleanly got here, so just return success
	return true;
}

//// Flush ///////////////////////////////////////////////////////////////////
// Writes out the remaining data, minus our clip value (which is fBufferSize)
// So here's our situation: at this point, one of two things could be true:
//
// 1) We haven't received enough data to clip by, at which point we don't
// write any more and bail (this will be true if fFirstFlip is still true)
//
// 2) Our cursor is at 0, which means we have one filled buffer that hasn't been
// written out and our current buffer is empty. At this point, we discard the
// filled buffer (which is precisely the length we want to clip by) and we're done.
//
// 3) The buffer we're on should be partially filled, while the other one is older 
// data. So, we want to write out the older data and the partial buffer all the way,
// except for the clip size. Since we can therefore never write out any data in the
// partial buffer (since that count will always be less than the clip size and thus be
// the second half of what we clip), we simply figure out how much of the other one we
// clip and write out the rest.

hsBool	plWAVClipBuffer::Flush( void )
{
	fFlushCalled = true;

	if( fFirstFlip )
		return false;	// We failed--not enough data to clip with

	if( fCursor == 0 )
	{
		// Our current buffer is empty, so the other buffer is precisely what we clip.
		// So just discard and return successfully
		return true;
	}

	// The hard case--we always discard the partial buffer we're on, so figure out
	// how much we want to save of the other buffer. The math is:
	//		Partial buffer amount we're clipping = fCursor
	//		Amount of other buffer we're clipping = fBufferSize - fCursor
	//		Amount of other buffer we're writing = fBufferSize - ( fBufferSize - fCursor ) = fCursor
	//	Go figure :)

	UInt32 toWrite = fCursor;

	UINT written;
	HRESULT hr = fOutFile->Write( toWrite, fBuffers[ 1 - fWhichBuffer ], &written );

	if( FAILED( hr ) )
	{
		hsAssert( false, "ERROR writing WMA stream to WAV file" );
		return false;
	}
	else if( written != toWrite )
	{
		hsAssert( false, "Unable to write all of WMA stream to WAV file" );
		return false;
	}
	
	// All done!
	return true;
}
