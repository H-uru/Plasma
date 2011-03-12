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
//	plBSDiffBuffer - A utility class for writing and applying a difference
//					buffer--i.e. a buffer containing a series of modifications
//					that will modify an old data buffer to match a new one. 
//					It's a useful utility class when doing binary file
//				    patching, for example, as you can write out the changes
//				    to this class, get back a data buffer suitable for writing,
//					then use this class again later to reconstruct the new buffer.
//
//					This class is copied in structure (not substance) from
//					plDiffBuffer. It is based on bsdiff-4.1 from BSD
//                  Linux (http://www.daemonology.org/bsdiff). It's *extremely*
//					hard to read code (written by a PhD), but it works well. The
//					original BSD code has been modified to have the bzip2 pipes 
//					it used to compress data removed. It has also been converted
//					to work a C++ utility class.
//
//					There isn't really an Add or Copy command in bsdiff. It just
//					uses three control numbers and two diff/data buffers.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plBSDiffBuffer_h
#define _plBSDiffBuffer_h

#include "hsTypes.h"
#include "hsStream.h"

//// Class Definition ////////////////////////////////////////////////////////

class hsRAMStream;
class plBSDiffBuffer
{
	protected:

		hsBool			fWriting;
		UInt32			fNewLength, fPatchLength;
		unsigned char*	fPatchBuffer;

	public:

		plBSDiffBuffer( UInt32 newLength, UInt32 oldLength = 0 );		// Constructor for writing new buffers. oldLength isn't required but helpful for optimizations
		plBSDiffBuffer( void *buffer, UInt32 length );	// Constructor for applying a given diff set
														// to an old buffer
		virtual ~plBSDiffBuffer();


		/// Creation/write functions

		//	Diff() creates the diff buffer from the new and old. 
		UInt32	Diff( UInt32 oldLength, void *oldBuffer, UInt32 newLength, void *newBuffer );

		// GetBuffer() will copy the diff stream into a new buffer and return it. You are responsible for freeing the buffer.
		void	GetBuffer( UInt32 &length, void *&bufferPtr );


		/// Apply functions

		// Apply() is another way to call Patch().
		UInt32	Apply( UInt32 oldLength, void *oldBuffer, UInt32 &newLength, void *&newBuffer )
		{ return Patch(oldLength, oldBuffer, newLength, newBuffer); };

		// Patch() will take this diff buffer and apply it to the given old buffer,
		// allocating and producing a new buffer. You are responsible for freeing the new buffer.
		UInt32	Patch( UInt32 oldLength, void *oldBuffer, UInt32 &newLength, void *&newBuffer );

	private:
		
		UInt32	IReadUnsignedInt8(unsigned char *buf);
		void	IWriteUnsignedInt8(UInt32 x,unsigned char *buf);
		void	ISafeMemcpy(unsigned char *dest, unsigned char *src, size_t nbytes,
							unsigned char *destend, unsigned char *srcend);
		void	ISplit(Int32 *I,Int32 *V,UInt32 start,UInt32 len,UInt32 h);
		void	IQSuffixSort(Int32 *I,Int32 *V,unsigned char *old,UInt32 oldsize);
		UInt32	IMatchLen( unsigned char *oldBuffer, UInt32 oldLength,
						   unsigned char *newBuffer, UInt32 newLength);
		UInt32	ISearch( Int32 *I,
					     unsigned char *oldBuffer, UInt32 oldLength,
						 unsigned char *newBuffer, UInt32 newLength,
						 UInt32 st, UInt32 en, Int32 *pos);


};

#endif // _plBSDiffBuffer_h
