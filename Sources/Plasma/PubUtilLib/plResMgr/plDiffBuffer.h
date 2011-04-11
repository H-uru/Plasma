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
//
//	plDiffBuffer - A utility class for writing and applying a difference
//				   buffer--i.e. a buffer containing a series of modifications
//				   (specifically, adds and copys) that will modify an old
//				   data buffer to match a new one. It's a useful utility
//				   class when doing binary file patching, for example, as you
//				   can write out the changes to this class, get back a data
//				   buffer suitable for writing, then use this class again
//				   later to reconstruct the new buffer.
//
//					This class is meant to construct diff buffers using two
//					ops: add and copy. Basically, the syntax is defined so
//					that to reconstruct the new buffer, you run through the 
//					list of ops sequentially, each one defining the next
//					chunk of data in the new buffer. Add ops will add new data
//					to the buffer that didn't exist in the old buffer, and 
//					copy ops will copy data that existed in the old buffer
//					(from an arbitrary offset, to facilitate encoding data
//					shuffling). Delete ops are implicit, as they simply aren't
//					defined in the stream of ops.
//
//// History /////////////////////////////////////////////////////////////////
//
//	7.24.2002 mcn	- Created (Happy late b-day to me!)
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plDiffBuffer_h
#define _plDiffBuffer_h

#include "hsTypes.h"
#include "hsStream.h"

//// Class Definition ////////////////////////////////////////////////////////

class hsRAMStream;
class plBSDiffBuffer;
class plDiffBuffer
{
	protected:

		hsBool		fWriting, f16BitMode;
		UInt32		fNewLength;
		hsRAMStream	*fStream;
		
		// Support for BSDiff patch buffers (Patching only)
		plBSDiffBuffer	*fBSDiffBuffer;
		hsBool			fIsBSDiff;

	public:

		plDiffBuffer( UInt32 newLength, UInt32 oldLength = 0 );		// Constructor for writing new buffers. oldLength isn't required but helpful for optimizations
		plDiffBuffer( void *buffer, UInt32 length );	// Constructor for applying a given diff set
														// to an old buffer
		virtual ~plDiffBuffer();


		/// Creation/write functions

		// Add() appends an Add-New-Data operation to the diff buffer. The data supplied will be copied internally.
		void	Add( Int32 length, void *newData );

		// Copy() appends a Copy-Data-From-Old operation to the diff buffer
		void	Copy( Int32 length, UInt32 oldOffset );

		// GetBuffer() will copy the diff stream into a new buffer and return it. You are responsible for freeing the buffer.
		void	GetBuffer( UInt32 &length, void *&bufferPtr );


		/// Apply functions

		// Apply() will take this diff buffer and apply it to the given old buffer, allocating and producing a new buffer. You are responsible for freeing the new buffer.
		void	Apply( UInt32 oldLength, void *oldBuffer, UInt32 &newLength, void *&newBuffer );

};

#endif // _plDiffBuffer_h
