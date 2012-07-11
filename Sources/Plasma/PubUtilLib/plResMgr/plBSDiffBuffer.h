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

Additional permissions under GNU GPL version 3 section 7

If you modify this Program, or any covered work, by linking or
combining it with any of RAD Game Tools Bink SDK, Autodesk 3ds Max SDK,
NVIDIA PhysX SDK, Microsoft DirectX SDK, OpenSSL library, Independent
JPEG Group JPEG library, Microsoft Windows Media SDK, or Apple QuickTime SDK
(or a modified version of those libraries),
containing parts covered by the terms of the Bink SDK EULA, 3ds Max EULA,
PhysX SDK EULA, DirectX SDK EULA, OpenSSL and SSLeay licenses, IJG
JPEG Library README, Windows Media SDK EULA, or QuickTime SDK EULA, the
licensors of this Program grant you additional
permission to convey the resulting work. Corresponding Source for a
non-source form of such a combination shall include the source code for
the parts of OpenSSL and IJG JPEG Library used as well as that of the covered
work.

You can contact Cyan Worlds, Inc. by email legal@cyan.com
 or by snail mail at:
      Cyan Worlds, Inc.
      14617 N Newport Hwy
      Mead, WA   99021

*==LICENSE==*/
//////////////////////////////////////////////////////////////////////////////
//
//  plBSDiffBuffer - A utility class for writing and applying a difference
//                  buffer--i.e. a buffer containing a series of modifications
//                  that will modify an old data buffer to match a new one. 
//                  It's a useful utility class when doing binary file
//                  patching, for example, as you can write out the changes
//                  to this class, get back a data buffer suitable for writing,
//                  then use this class again later to reconstruct the new buffer.
//
//                  This class is copied in structure (not substance) from
//                  plDiffBuffer. It is based on bsdiff-4.1 from BSD
//                  Linux (http://www.daemonology.org/bsdiff). It's *extremely*
//                  hard to read code (written by a PhD), but it works well. The
//                  original BSD code has been modified to have the bzip2 pipes 
//                  it used to compress data removed. It has also been converted
//                  to work a C++ utility class.
//
//                  There isn't really an Add or Copy command in bsdiff. It just
//                  uses three control numbers and two diff/data buffers.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plBSDiffBuffer_h
#define _plBSDiffBuffer_h

#include "HeadSpin.h"
#include "hsStream.h"

//// Class Definition ////////////////////////////////////////////////////////

class hsRAMStream;
class plBSDiffBuffer
{
    protected:

        bool            fWriting;
        uint32_t        fNewLength, fPatchLength;
        unsigned char*  fPatchBuffer;

    public:

        plBSDiffBuffer( uint32_t newLength, uint32_t oldLength = 0 );       // Constructor for writing new buffers. oldLength isn't required but helpful for optimizations
        plBSDiffBuffer( void *buffer, uint32_t length );  // Constructor for applying a given diff set
                                                        // to an old buffer
        virtual ~plBSDiffBuffer();


        /// Creation/write functions

        //  Diff() creates the diff buffer from the new and old. 
        uint32_t  Diff( uint32_t oldLength, void *oldBuffer, uint32_t newLength, void *newBuffer );

        // GetBuffer() will copy the diff stream into a new buffer and return it. You are responsible for freeing the buffer.
        void    GetBuffer( uint32_t &length, void *&bufferPtr );


        /// Apply functions

        // Apply() is another way to call Patch().
        uint32_t  Apply( uint32_t oldLength, void *oldBuffer, uint32_t &newLength, void *&newBuffer )
        { return Patch(oldLength, oldBuffer, newLength, newBuffer); };

        // Patch() will take this diff buffer and apply it to the given old buffer,
        // allocating and producing a new buffer. You are responsible for freeing the new buffer.
        uint32_t  Patch( uint32_t oldLength, void *oldBuffer, uint32_t &newLength, void *&newBuffer );

    private:
        
        uint32_t  IReadUnsignedint8_t(unsigned char *buf);
        void    IWriteUnsignedint8_t(uint32_t x,unsigned char *buf);
        void    ISafeMemcpy(unsigned char *dest, unsigned char *src, size_t nBytes,
                            unsigned char *destend, unsigned char *srcend);
        void    ISplit(int32_t *I,int32_t *V,uint32_t start,uint32_t len,uint32_t h);
        void    IQSuffixSort(int32_t *I,int32_t *V,unsigned char *old,uint32_t oldsize);
        uint32_t  IMatchLen( unsigned char *oldBuffer, uint32_t oldLength,
                           unsigned char *newBuffer, uint32_t newLength);
        uint32_t  ISearch( int32_t *I,
                         unsigned char *oldBuffer, uint32_t oldLength,
                         unsigned char *newBuffer, uint32_t newLength,
                         uint32_t st, uint32_t en, int32_t *pos);


};

#endif // _plBSDiffBuffer_h
