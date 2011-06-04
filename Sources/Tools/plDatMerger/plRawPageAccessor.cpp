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
//	Also assumes the current global resManager is a plRawResManager!
//
//////////////////////////////////////////////////////////////////////////////

#include "hsTypes.h"
#include "hsStream.h"
#include "plRawPageAccessor.h"
#include "plRawResManager.h"
#include "plRawKeyedObject.h"

#include "hsTemplates.h"
#include "../pnKeyedObject/plKeyImp.h"
#include "../plResMgr/plRegistryNode.h"
#include "../plResMgr/plRegistryHelpers.h"
#include "../plResMgr/plRegistrySource.h"


//// Constructor/Destructor //////////////////////////////////////////////////

plRawPageAccessor::plRawPageAccessor( plRegistryPageNode *source, hsBool read )
{
	fSource = source;
	if( read )
		ReadFromSource();
}

plRawPageAccessor::~plRawPageAccessor()
{
	Release();
}

//// Iterators ///////////////////////////////////////////////////////////////

class plRawReaderIter : public plRegistryKeyIterator
{
	public:

		virtual hsBool	EatKey( plKey key )
		{
			plRawResManager *mgr = (plRawResManager *)hsgResMgr::ResMgr();

			UInt32 len;
			plKeyImp *imp = (plKeyImp *)key;
			UInt8 *buffer = mgr->ReadObjectBuffer( imp, len );

			// This will also set the object ptr in the key
			plRawKeyedObject *obj = new plRawKeyedObject( key, len, buffer );
			delete [] buffer;		// rawKeyedObject keeps a copy

			return true;
		}
};

class plRawWriterIter : public plRegistryKeyIterator
{
	hsStream	*fStream;

	public:

		plRawWriterIter( hsStream *stream ) : fStream( stream ) {}

		virtual hsBool	EatKey( plKey key )
		{
			plRawResManager *mgr = (plRawResManager *)hsgResMgr::ResMgr();

			plRawKeyedObject *obj = (plRawKeyedObject *)key->ObjectIsLoaded();
			if( obj == nil )
			{
				// Mark the key as not written
				plRawKeyedObject::MarkAsEmpty( key );
				return true;
			}

			obj->Write( fStream );
			return true;
		}
};

class plRawReleaseIter : public plRegistryKeyIterator
{
	public:

		virtual hsBool	EatKey( plKey key )
		{
			plRawKeyedObject *obj = (plRawKeyedObject *)key->ObjectIsLoaded();
			delete obj;

			return true;
		}
};


//// Various Functions ///////////////////////////////////////////////////////

void	plRawPageAccessor::ReadFromSource( void )
{
	if( !fSource->IsLoaded() )
		fSource->LoadKeysFromSource();

	plRawReaderIter	iter;
	fSource->IterateKeys( &iter );
}

void	plRawPageAccessor::WriteToSource( void )
{
	if( fSource->GetSource() == nil )
	{
		hsAssert( false, "Unable to write accessor to disk; no source defined!" );
		return;
	}

	// Write out objects first
	hsStream *stream = fSource->GetSource()->OpenDataStream( fSource, true );
	if( stream == nil )
		return;

	plRawWriterIter	writer( stream );
	fSource->IterateKeys( &writer );

	fSource->GetSource()->CloseDataStream( fSource );
	
	// Now write out the keys
	fSource->WriteKeysToSource();
}

void	plRawPageAccessor::Release( void )
{
	plRawReleaseIter	iter;
	fSource->IterateKeys( &iter );

	fSource->ClearKeyLists();
}

void	plRawPageAccessor::AddCopy( const plKey &origKey )
{
	plRawResManager *mgr = (plRawResManager *)hsgResMgr::ResMgr();
	plKey			newKey;


	// Get the source object
	plRawKeyedObject *srcObj = (plRawKeyedObject *)origKey->ObjectIsLoaded();

	// Construct a new uoid
	plUoid			newUoid( fSource->GetPageInfo().GetLocation(), 
							 origKey->GetUoid().GetClassType(), 
							 origKey->GetUoid().GetObjectName() );

	// Does it already exist?
	newKey = mgr->FindKey( newUoid );
	if( newKey != nil )
	{
		// Yup, gotta get rid of old object (if there is one)
		plRawKeyedObject *obj = (plRawKeyedObject *)newKey->ObjectIsLoaded();
		delete obj;
	}
	else
	{
		// Nope, gotta create key first
		newKey = mgr->NewBlankKey( newUoid );
	}

	// Force the key's uoid to the right uoid, now that it's in the right page
	( (plKeyImp *)newKey )->SetUoid( origKey->GetUoid() );

	// Assign a new buffer to the key
	if( srcObj != nil )
	{
		// Will set obj pointer in key
		plRawKeyedObject *obj = new plRawKeyedObject( newKey, srcObj->GetBufferSize(), srcObj->GetBuffer() );
	}
}

void	plRawPageAccessor::UpdateDataVersion( plRegistryPageNode *from )
{
	plPageInfo &orig = from->GetPageInfo();

	fSource->GetPageInfo().SetVersion( orig.GetMajorVersion(), orig.GetMinorVersion() );
	fSource->GetPageInfo().SetReleaseVersion( orig.GetReleaseVersion() );
}

