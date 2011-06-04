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
//	plCommonObjLib - Base class for a library of objects that are stored	//
//					 in common pages (for now, that means Textures or		//
//					 BuiltIn pages).										//
//																			//
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"
#include "hsTypes.h"
#include "plCommonObjLib.h"
#include "../pnKeyedObject/hsKeyedObject.h"
#include "../pnKeyedObject/plUoid.h"
#include "plPluginResManager.h"


//////////////////////////////////////////////////////////////////////////////
//// Static Array And Functions //////////////////////////////////////////////
//	Used only by the export resManager, to create and maintain a list of the
//	commonObjLibs to be used.
//////////////////////////////////////////////////////////////////////////////

class plCommonObjLibList
{
	public:
		UInt32						fRefCount;
		hsTArray<plCommonObjLib *>	fLibs;

		plCommonObjLibList() { fRefCount = 0; }

		void	Add( plCommonObjLib *lib )
		{
			fLibs.Append( lib );
			fRefCount++;
		}

		hsBool	Remove( plCommonObjLib *lib )
		{
			int idx = fLibs.Find( lib );
			if( idx != fLibs.kMissingIndex )
				fLibs.Remove( idx );
			else
			{
				hsAssert( false, "Common Object Lib not found in list upon deletion. Are you misusing this class? Tsk tsk!" );
			}

			fRefCount--;
			return ( fRefCount == 0 ) ? true : false;
		}
};

plCommonObjLibList	*plCommonObjLib::fLibList = nil;

UInt32	plCommonObjLib::GetNumLibs( void )
{
	return ( fLibList != nil ) ? fLibList->fLibs.GetCount() : 0;
}

plCommonObjLib	*plCommonObjLib::GetLib( UInt32 idx )
{
	if( fLibList == nil )
		return nil;

	if( idx < fLibList->fLibs.GetCount() )
		return fLibList->fLibs[ idx ];

	return nil;
}


//////////////////////////////////////////////////////////////////////////////
//// Constructor/Destructor //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plCommonObjLib::plCommonObjLib()
{
	// Make sure we have a list to add ourselves to
	if( fLibList == nil )
		fLibList = TRACKED_NEW plCommonObjLibList();

	// Add ourselves to the list of libs
	fLibList->Add( this );
}

plCommonObjLib::~plCommonObjLib()
{
	ClearObjectList();

	// Remove ourselves from the list of libs
	if( fLibList->Remove( this ) )
	{
		// List is no longer needed
		delete fLibList;
		fLibList = nil;
	}
}


//////////////////////////////////////////////////////////////////////////////
//// Base Utility Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ClearObjectList /////////////////////////////////////////////////////////

void	plCommonObjLib::ClearObjectList( void )
{
	int		i;


	// Unref our object list, so they'll go away properly
	for( i = 0; i < fObjects.GetCount(); i++ )
		fObjects[ i ]->GetKey()->UnRefObject();
	fObjects.Reset();
}

//// AddObject ///////////////////////////////////////////////////////////////
//	Adds the given object to our lib. The object must have a key already.

void	plCommonObjLib::AddObject( hsKeyedObject *object )
{
	if( object == nil || object->GetKey() == nil )
	{
		hsAssert( false, "Trying to add an object to a commonLib that doesn't have a key" );
		return;
	}

	// Ref it so it won't go away on us
	object->GetKey()->RefObject();
	fObjects.Append( object );
}

//// RemoveObjectAndKey //////////////////////////////////////////////////////
//	Given the key to an object, completely nukes the object and the key. After
//	this function call, the key should no longer exist in the registry and be
//	free to use elsewhere.

hsBool	plCommonObjLib::RemoveObjectAndKey( plKey &key )
{
	if (!key)
	{
		hsAssert( false, "Received RemoveObjectAndKey() call for a key that is invalid. Nillifying key anyway." );
		key = nil;
		return true;
	}
	hsKeyedObject *object = hsKeyedObject::ConvertNoRef( key->ObjectIsLoaded() );
	if( object == nil )
	{
		hsAssert( false, "Received RemoveObjectAndKey() call for a key that isn't loaded. Nillifying key anyway." );
		key = nil;
		return true;
	}

	int	idx = fObjects.Find( object );
	if( idx == fObjects.kMissingIndex )
	{
		hsAssert( false, "Trying to RemoveObjectAndKey() for a common object not in the lib." );
		key = nil;
		return true;
	}

	// Unref and remove from our list
	fObjects[ idx ]->GetKey()->UnRefObject();
	fObjects.Remove( idx );

	// Nuke out the key and its object
	if( !plPluginResManager::ResMgr()->NukeKeyAndObject( key ) )
	{
		hsAssert( false, "Trouble nuking out the key for this texture. Problems abound...." );
		return false;
	}

	// All done!
	return true;
}

//// FindObject //////////////////////////////////////////////////////////////
//	Given a name and an optional class type, tries to find that object in
//	our lib. Returns nil if not found. Use to find out if you already have a
//	object of a given name that was previously exported.

hsKeyedObject	*plCommonObjLib::FindObject( const char *name, UInt16 classType /* = -1 */ )
{
	int		i;


	for( i = 0; i < fObjects.GetCount(); i++ )
	{
		const plUoid	&uoid = fObjects[ i ]->GetKey()->GetUoid();


		if( stricmp( uoid.GetObjectName(), name ) == 0 &&
			( classType == (UInt16)-1 || classType == uoid.GetClassType() ) )
		{
			return fObjects[ i ];
		}
	}

	return nil;
}
