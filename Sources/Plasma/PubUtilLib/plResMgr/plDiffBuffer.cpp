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
//  plDiffBuffer - A utility class for writing and applying a difference
//                 buffer--i.e. a buffer containing a series of modifications
//                 (specifically, adds and copys) that will modify an old
//                 data buffer to match a new one. It's a useful utility
//                 class when doing binary file patching, for example, as you
//                 can write out the changes to this class, get back a data
//                 buffer suitable for writing, then use this class again
//                 later to reconstruct the new buffer.
//
//// History /////////////////////////////////////////////////////////////////
//
//  7.24.2002 mcn   - Created (Happy late b-day to me!)
//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plDiffBuffer.h"
#include "plBSDiffBuffer.h"

#include "hsStream.h"


//////////////////////////////////////////////////////////////////////////////
//// Constructor/Destructors /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Creation Constructor ////////////////////////////////////////////////////
//  Use this constructor when creating a new diff buffer. Pass in the length
//  of the final new buffer. You don't need to pass in the length of the old
//  buffer, but if you do, it'll help this class do some internal space 
//  optimization.

plDiffBuffer::plDiffBuffer( uint32_t newLength, uint32_t oldLength )
: fBSDiffBuffer( nil )
, fIsBSDiff( false )
{
    // Basically, if the new and old lengths can both fit into 16 bits 
    // (not including a potential negation), then we can store all the
    // segment info as 16-bit values instead of 32.
    if( oldLength > 0 && oldLength < 32767 && newLength < 32767 )
        f16BitMode = true;
    else
        f16BitMode = false;

    fNewLength = newLength;
    fStream = new hsRAMStream();
    fStream->WriteLE32( fNewLength );
    fStream->WriteBool( f16BitMode );
    fWriting = true;
}

//// Application Constructor /////////////////////////////////////////////////
//  Use this constructor when taking a diff buffer and applying it to an old
//  buffer. Pass in a pointer to the diff buffer and its length. The buffer
//  will be copied, so you don't need to keep it around after you construct
//  this object.

plDiffBuffer::plDiffBuffer( void *buffer, uint32_t length )
: fBSDiffBuffer( nil )
, fStream( nil )
, fIsBSDiff( false )
, fWriting( false )
{
    // Check to see if this uses the newer BSDiff format
    if ( buffer && length > 32 &&
         memcmp(buffer,"BSDIFF40",8)==0 )
    {
        // This is a bsdiff buffer. Use plBSDiffBuffer to handle it.
        fBSDiffBuffer = new plBSDiffBuffer(buffer, length);
        fIsBSDiff = true;
    }
    else
    {
        fStream = new hsRAMStream();
        fStream->Write( length, buffer );
        fStream->Rewind();

        fNewLength = fStream->ReadLE32();
        f16BitMode = fStream->ReadBool();
    }
}

plDiffBuffer::~plDiffBuffer()
{
    if (fStream)
        delete fStream;
    if (fBSDiffBuffer)
        delete fBSDiffBuffer;
}


//////////////////////////////////////////////////////////////////////////////
//// Creation/Write Functions ////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Add /////////////////////////////////////////////////////////////////////
//  Add() appends an Add-New-Data operation to the diff buffer. The data 
//  supplied will be copied internally, so you can discard it after you call
//  this function. 

void    plDiffBuffer::Add( int32_t length, void *newData )
{
    hsAssert( fWriting, "Trying to Add() to a difference buffer that's reading" );

    // We flag our two different op types by the sign of the length. Negative
    // lengths are an add operation, positive ones are copy ops.
    if( f16BitMode )
        fStream->WriteLE16( -( (int16_t)length ) );
    else
        fStream->WriteLE32( -length );
    fStream->Write( length, newData );
}

//// Copy ////////////////////////////////////////////////////////////////////
//  Copy() appends a Copy-Data-From-Old operation to the diff buffer. 

void    plDiffBuffer::Copy( int32_t length, uint32_t oldOffset )
{
    hsAssert( fWriting, "Trying to Copy() to a difference buffer that's reading" );

    // We flag our two different op types by the sign of the length. Negative
    // lengths are an add operation, positive ones are copy ops.
    if( f16BitMode )
    {
        fStream->WriteLE16( (int16_t)length );
        fStream->WriteLE16( (uint16_t)oldOffset );
    }
    else
    {
        fStream->WriteLE32( length );
        fStream->WriteLE32( oldOffset );
    }
}

//// GetBuffer ///////////////////////////////////////////////////////////////
//  GetBuffer() will copy the diff stream into a new buffer and return it.
//  You are responsible for freeing the buffer. Call this once you're done 
//  adding ops and want the raw data to write out somewhere. Note: this
//  function will rewind the diff stream, so once you call it, you can't do
//  anything else on the object.

void    plDiffBuffer::GetBuffer( uint32_t &length, void *&bufferPtr )
{
    hsAssert( fWriting, "Trying to GetBuffer() on a difference buffer that's reading" );

    length = fStream->GetPosition();
    bufferPtr = (void *)new uint8_t[ length ];

    fStream->Rewind();
    fStream->Read( length, bufferPtr );
}


//////////////////////////////////////////////////////////////////////////////
//// Application Functions ///////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// Apply ///////////////////////////////////////////////////////////////////
//  Apply() will take this diff buffer and apply it to the given old buffer, 
//  allocating and producing a new buffer. You are responsible for freeing 
//  the new buffer.

#define hsAssertAndBreak( cond, msg ) { if( cond ) { hsAssert( false, msg ); break; } }

void    plDiffBuffer::Apply( uint32_t oldLength, void *oldBuffer, uint32_t &newLength, void *&newBuffer )
{
    hsAssert( !fWriting, "Trying to Apply() a difference buffer that's writing" );

    // Is this is a BSDiff patch, use plBSDiffBuffer and return.
    if (fIsBSDiff)
    {
        fBSDiffBuffer->Apply(oldLength, oldBuffer, newLength, newBuffer);
        return;
    }

    /// Step 1: Allocate the new buffer
    newLength = fNewLength;
    uint8_t *new8Buffer = new uint8_t[ newLength ];
    uint8_t *old8Buffer = (uint8_t *)oldBuffer;
    newBuffer = (void *)new8Buffer;


    /// Step 2: Loop through the difference stream
    int32_t   opLength;
    uint32_t  newBufferPos = 0;
    while( newBufferPos < newLength )
    {
        // Read in the op length
        if( f16BitMode )
        {
            int16_t opLen16 = fStream->ReadLE16();
            if( opLen16 < 0 )
                opLength = -( (int32_t)( -opLen16 ) );
            else
                opLength = (uint32_t)opLen16;
        }
        else
            opLength = fStream->ReadLE32();

        // As defined, negative ops are add ops, positive ones are copys
        if( opLength < 0 )
        {
            hsAssertAndBreak( newBufferPos - opLength > newLength, "Destination buffer offset in plDiffBuffer() is out of range!" );

            // Add op, read in the added data
            fStream->Read( -opLength, &new8Buffer[ newBufferPos ] );
            newBufferPos += -opLength;
        }
        else
        {
            // Copy op, so get the old offset and copy from there
            uint32_t oldOffset = f16BitMode ? fStream->ReadLE16() : fStream->ReadLE32();

            hsAssertAndBreak( newBufferPos + opLength > newLength, "Destination buffer offset in plDiffBuffer() is out of range!" );
            hsAssertAndBreak( oldOffset + opLength > oldLength, "Difference buffer offset in plDiffBuffer() is out of range of the old buffer!" );

            memcpy( &new8Buffer[ newBufferPos ], old8Buffer + oldOffset, opLength );
            newBufferPos += opLength;
        }
    }

    hsAssert( newBufferPos == newLength, "Invalid sequence of difference ops in plDiffBuffer::Apply()" );
}
