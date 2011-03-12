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
#ifndef CoreLib_Thread
#define CoreLib_Thread

#include "hsThread.h"
#include "hsUtils.h"


//////////////////////////////////////////////////////////////////////////////
hsReaderWriterLock::hsReaderWriterLock( const char * name, Callback * cb )
: fReaderCount( 0 )
, fWriterSema( 1 )
, fCallback( cb )
, fName( nil )
{
	fName = hsStrcpy( name );
}

hsReaderWriterLock::~hsReaderWriterLock()
{
	delete [] fName;
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
