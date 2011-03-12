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

#ifndef _plWAVClipBuffer_h
#define _plWAVClipBuffer_h

//// Class Definition ////////////////////////////////////////////////////////

class CWaveFile;
class plWAVClipBuffer
{
public:

	plWAVClipBuffer( UInt32 clipSize, CWaveFile *outFile );
	~plWAVClipBuffer();

	// Inits the buffer. Can re-init if you wish
	void	Init( UInt32 clipSize, CWaveFile *outFile );

	// Writes/adds data to the buffer
	hsBool	WriteData( UInt32 size, UInt8 *data );

	// Call at the end, flushes the buffer and performs the clipping
	hsBool	Flush( void );

protected:
	UInt8	*fBuffers[ 2 ];
	UInt8	fWhichBuffer;	// 0 or 1
	UInt32	fCursor, fBufferSize;
	hsBool	fFirstFlip, fFlushCalled;

	CWaveFile	*fOutFile;

	void	IShutdown( void );
};

#endif //_plWAVClipBuffer_h
