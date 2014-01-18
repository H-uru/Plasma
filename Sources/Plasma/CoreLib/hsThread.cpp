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
#ifndef CoreLib_Thread
#define CoreLib_Thread

#include "hsThread.h"



//////////////////////////////////////////////////////////////////////////////
hsReaderWriterLock::hsReaderWriterLock(Callback * cb)
: fReaderCount( 0 )
, fWriterSema( 1 )
, fCallback( cb )
{
}

void hsReaderWriterLock::LockForReading()
{
    if ( fCallback )
        fCallback->OnLockingForRead( this );
    fReaderCountLock.Lock();
    fReaderLock.Lock();
    fReaderCount++;
    if ( fReaderCount==1 )
        fWriterSema.Wait();
    fReaderLock.Unlock();
    fReaderCountLock.Unlock();
    if ( fCallback )
        fCallback->OnLockedForRead( this );
}

void hsReaderWriterLock::UnlockForReading()
{
    if ( fCallback )
        fCallback->OnUnlockingForRead( this );
    fReaderLock.Lock();
    fReaderCount--;
    if ( fReaderCount==0 )
        fWriterSema.Signal();
    fReaderLock.Unlock();
    if ( fCallback )
        fCallback->OnUnlockedForRead( this );
}

void hsReaderWriterLock::LockForWriting()
{
    if ( fCallback )
        fCallback->OnLockingForWrite( this );
    fReaderCountLock.Lock();
    fWriterSema.Wait();
    hsAssert( fReaderCount==0, "Locked for writing, but fReaderCount>0" );
    if ( fCallback )
        fCallback->OnLockedForWrite( this );
}

void hsReaderWriterLock::UnlockForWriting()
{
    if ( fCallback )
        fCallback->OnUnlockingForWrite( this );
    fWriterSema.Signal();
    fReaderCountLock.Unlock();
    if ( fCallback )
        fCallback->OnUnlockedForWrite( this );
}



#endif // CoreLib_Thread
