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
//	plRawPageAccessor - Dangerous little class that lets you take a
//					    plRegistryPageNode and load the objects in raw (i.e.
//						as block memory buffers).
//						This should NOT be used in any normal app, only 
//						utility apps that don't want to load objects in
//						normally (which basically means if you're not mcn,
//						don't use this!)
//
//// Why We're Bad ///////////////////////////////////////////////////////////
//
//	To store all the raw buffers, we stuff them as pointers into the keys
//	themselves. This is Way Bad(tm) because those pointers are expecting
//	hsKeyedObjects, and what we're giving them certainly ain't those.
//	This is why it's only safe to use this class in a very small, controlled
//	environment, one where we know the keys won't be accessed in a normal
//	fashion so we know nobody will try to use our pointers in a bad way.
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStream.h"
#include "hsResMgr.h"
#include "plRawKeyedObject.h"
#include "../pnKeyedObject/plKeyImp.h"


//// Tiny Yet Now Famous Key Hack ////////////////////////////////////////////

class plPublicKeyImp : public plKeyImp
{
	public:

		void	SetObjectPtrDirect( hsKeyedObject *obj ) 
		{
			fObjectPtr = obj;
		}

		void	SetAsEmpty( void )
		{
			fStartPos = (UInt32)-1;
			fDataLen = (UInt32)-1;
		}

		void	SetStartPosFromStream( hsStream *stream )
		{
			fStartPos = stream->GetPosition();
		}

		void	SetLengthFromStream( hsStream *stream ) 
		{ 
			fDataLen = stream->GetPosition() - fStartPos;
		}
};

//// Constructor/Destructor //////////////////////////////////////////////////

plRawKeyedObject::plRawKeyedObject()
{
	fSrcKey = nil;
	fBuffer = nil;
	fBufferSize = 0;
}

plRawKeyedObject::plRawKeyedObject( const plKey &key, UInt32 size, UInt8 *data )
{
	fSrcKey = key;
	( (plPublicKeyImp *)(plKeyImp *)key )->SetObjectPtrDirect( this );

	fBuffer = nil;
	fBufferSize = 0;

	SetBuffer( size, data );
}

plRawKeyedObject::~plRawKeyedObject()
{
	if( fSrcKey != nil )
	{
		( (plPublicKeyImp *)(plKeyImp *)fSrcKey )->SetObjectPtrDirect( nil );
		fSrcKey = nil;
	}

	delete [] fBuffer;
}

void	plRawKeyedObject::SetBuffer( UInt32 size, UInt8 *data )
{
	delete [] fBuffer;

	if( data == nil )
	{
		fBufferSize = 0;
		fBuffer = nil;
		return;
	}

	fBufferSize = size;
	fBuffer = new UInt8[ size ];
	memcpy( fBuffer, data, size );
}

void	plRawKeyedObject::SetKey( plKey k )
{
	if( fSrcKey != nil )
	{
		( (plPublicKeyImp *)(plKeyImp *)fSrcKey )->SetObjectPtrDirect( nil );
	}

	fSrcKey = k;
	if( fSrcKey != nil )
	{
		( (plPublicKeyImp *)(plKeyImp *)fSrcKey )->SetObjectPtrDirect( this );
	}
}

void	plRawKeyedObject::MarkAsEmpty( plKey &key )
{
	( (plPublicKeyImp *)(plKeyImp *)key )->SetAsEmpty();
}

void	plRawKeyedObject::Write( hsStream *stream )
{
	// BEFORE we write out, somewhere at the top of our buffer is the key to ourselves
	// that all hsKeyedObjects write out as part of their Write() function. We need
	// to REPLACE that key with our new key, since our location has now changed. Note
	// that this will ONLY work if our location changes, NOT if our name changes, 
	// because we're relying on the fact that the written size of our key is not
	// going to change!!!
	{
		hsWriteOnlyStream	replaceStream( fBufferSize, fBuffer );

		// Here's the part that REALLY sucks, 'cause it assumes our written format will never change!!!!
		// It ALSO assumes, VERY dangerously, that ReadSwap16() will ALWAYS read a size UInt16

		replaceStream.SetPosition( sizeof( UInt16 ) );	// Get past creatable class that resManager writes out

		hsgResMgr::ResMgr()->WriteKey( &replaceStream, fSrcKey, hsResMgr::kWriteNoCheck );
	}

	( (plPublicKeyImp *)(plKeyImp *)fSrcKey )->SetStartPosFromStream( stream );

	stream->Write( fBufferSize, fBuffer );

	( (plPublicKeyImp *)(plKeyImp *)fSrcKey )->SetLengthFromStream( stream );
}


//// Warning Stubs ///////////////////////////////////////////////////////////

void	plRawKeyedObject::Validate()
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
}

hsBool	plRawKeyedObject::IsFinal()
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
	return false;
}

void	plRawKeyedObject::Read(hsStream *s, hsResMgr *mgr )
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
}

void	plRawKeyedObject::Write(hsStream *s, hsResMgr *mgr )
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
}

hsBool	plRawKeyedObject::MsgReceive( plMessage *msg )
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
	return false;
}

hsKeyedObject *plRawKeyedObject::GetSharedObject()
{
	hsAssert( false, "Invalid call on plRawKeyedObject" );
	return nil;
}

