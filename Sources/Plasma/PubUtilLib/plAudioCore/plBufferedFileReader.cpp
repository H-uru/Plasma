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
//                                                                          //
//  plBufferedFileReader - Reads in a given file into a RAM buffer, then    //
//                         "reads" from that buffer as requested. Useless   //
//                         for normal sounds, but perfect for streaming     //
//                         from RAM.                                        //
//                                                                          //
//// Notes ///////////////////////////////////////////////////////////////////
//                                                                          //
//  11.1.2002 - Created by mcn.                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "plBufferedFileReader.h"
#include "plFileSystem.h"
//#include "plProfile.h"


//plProfile_CreateMemCounter( "BufferedRAM", "Sound", SndBufferedMem );

//// Constructor/Destructor //////////////////////////////////////////////////

plBufferedFileReader::plBufferedFileReader( const plFileName &path, plAudioCore::ChannelSelect whichChan )
{
    // Init some stuff
    fBufferSize = 0;
    fBuffer = nullptr;
    fCursor = 0;

    hsAssert( path.IsValid(), "Invalid path specified in plBufferedFileReader" );

    // Ask plAudioFileReader for another reader to get this file
    // Note: have this reader do the chanSelect for us
    plAudioFileReader *reader = plAudioFileReader::CreateReader( path, whichChan );
    if (reader == nullptr || !reader->IsValid())
    {
        delete reader;
        IError( "Unable to open file to read in to RAM buffer" );
        return;
    }

    fHeader = reader->GetHeader();

    fBufferSize = reader->GetDataSize();
    fBuffer = new uint8_t[ fBufferSize ];
    //plProfile_NewMem( SndBufferedMem, fBufferSize );
    if (fBuffer == nullptr)
    {
        delete reader;
        IError( "Unable to allocate RAM buffer" );
        return;
    }

    if( !reader->Read( fBufferSize, fBuffer ) )
    {
        delete reader;
        IError( "Unable to read file into RAM buffer" );
        return;
    }

    // All done!
    delete reader;
}

plBufferedFileReader::~plBufferedFileReader()
{
    Close();
}

void    plBufferedFileReader::Close()
{
    //plProfile_DelMem( SndBufferedMem, fBufferSize );

    delete fBuffer;
    fBuffer = nullptr;
    fBufferSize = 0;
    fCursor = 0;
}

void    plBufferedFileReader::IError( const char *msg )
{
    hsAssert( false, msg );
    Close();
}

plWAVHeader &plBufferedFileReader::GetHeader()
{
    hsAssert( IsValid(), "GetHeader() called on an invalid RAM buffer" );

    return fHeader;
}

float   plBufferedFileReader::GetLengthInSecs()
{
    hsAssert( IsValid(), "GetLengthInSecs() called on an invalid RAM buffer" );

    return (float)fBufferSize / (float)fHeader.fAvgBytesPerSec;
}

bool    plBufferedFileReader::SetPosition( uint32_t numBytes )
{
    hsAssert( IsValid(), "SetPosition() called on an invalid RAM buffer" );

    if( numBytes > fBufferSize )
    {
        hsAssert( false, "Invalid position in SetPosition()" );
        return false;
    }

    fCursor = numBytes;
    return true;
}

bool    plBufferedFileReader::Read( uint32_t numBytes, void *buffer )
{
    hsAssert( IsValid(), "Read() called on an invalid RAM buffer" );


    bool valid = true;

    if( fCursor + numBytes > fBufferSize )
    {
        numBytes = fBufferSize - fCursor;
        valid = false;
    }

    memcpy( buffer, fBuffer + fCursor, numBytes );
    fCursor += numBytes;

    return valid;
}

uint32_t  plBufferedFileReader::NumBytesLeft()
{
    hsAssert( IsValid(), "NumBytesLeft() called on an invalid RAM buffer" );

    return fBufferSize - fCursor;
}
