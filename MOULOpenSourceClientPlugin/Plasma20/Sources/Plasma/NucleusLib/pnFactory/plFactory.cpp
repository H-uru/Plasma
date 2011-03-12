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

#define PLFACTORY_PRIVATE
#include "hsTypes.h"
#include "plFactory.h"
#include "hsStream.h"
#include "plCreatable.h"
#include "plCreator.h"
#include "hsUtils.h"

// For class names
#include "../NucleusLib/inc/plCreatableStrings.h"


static plFactory*	theFactory = nil;

plFactory::plFactory()
{
	fCreators.SetCountAndZero(plCreatableIndex::plNumClassIndices);
}

plFactory::~plFactory()
{
}

hsBool plFactory::ICreateTheFactory()
{
	if( theFactory )
		return true;

	theFactory = TRACKED_NEW plFactory;

	return theFactory != nil;
}

UInt16 plFactory::IGetNumClasses()
{
	return plCreatableIndex::plNumClassIndices;
}

void plFactory::IForceShutdown()
{
	int i;
	for( i = 0; i < fCreators.GetCount(); i++ )
	{
		if( fCreators[i] )
		{
			hsRefCnt_SafeUnRef(this);
			fCreators[i] = nil;
		}
	}
}

void plFactory::IShutdown()
{
	delete theFactory;
	theFactory = nil;
}

UInt16 plFactory::IRegister(UInt16 hClass, plCreator* worker)
{
	delete fCreators[hClass];
	fCreators[hClass] = worker;
	return hClass;
}

//
// return true if creator exists
//
bool plFactory::CanCreate(UInt16 hClass)
{
	if( hClass & 0x8000 )				// nil creatable
		return false;

	if( !theFactory && !ICreateTheFactory() )	// no factory
		return false;

	if( hClass >= theFactory->IGetNumClasses() )	// invalid index
		return false;

	return ( theFactory->fCreators[ hClass ] != nil );	// check creator
}

plCreatable* plFactory::ICreate(UInt16 hClass)
{
	if (CanCreate(hClass))
	{
		return fCreators[hClass]->Create();
	}

	if (!(hClass & 0x8000))
	{
		hsAssert( false, "Invalid class index or nil creator : plFactory::Create()" );	
	}
	return nil;
}

void plFactory::UnRegister(UInt16 hClass, plCreator* worker)
{
	if( theFactory )
	{
		theFactory->IUnRegister(hClass);
		hsRefCnt_SafeUnRef(theFactory);
		if( theFactory->RefCnt() < 2 )
			IShutdown();
	}
}

void plFactory::IUnRegister(UInt16 hClass)
{
	fCreators[hClass] = nil;
}

UInt16 plFactory::Register(UInt16 hClass, plCreator* worker)
{
	if( !theFactory && !ICreateTheFactory() )
			return nil;

	hsRefCnt_SafeRef(theFactory);
	return theFactory->IRegister(hClass, worker);
}

plCreatable* plFactory::Create(UInt16 hClass)
{
	if( !theFactory && !ICreateTheFactory() )
			return nil;

	return theFactory->ICreate(hClass);
}



UInt16 plFactory::GetNumClasses()
{
	if( !theFactory && !ICreateTheFactory() )
		return 0;

	return theFactory->IGetNumClasses();
}

hsBool plFactory::IDerivesFrom(UInt16 hBase, UInt16 hDer)
{
	if( hDer >= fCreators.GetCount() )
		return false;

	return fCreators[hDer] ? fCreators[hDer]->HasBaseClass(hBase) : false;
}

hsBool plFactory::DerivesFrom(UInt16 hBase, UInt16 hDer)
{
	if( !theFactory && !ICreateTheFactory() )
		return 0;

	return theFactory->IDerivesFrom(hBase, hDer);
}

// slow lookup for things like console
UInt16 plFactory::FindClassIndex(const char* className)
{
	int numClasses=GetNumClasses();

	if (className && theFactory)
	{
		int i;
		for( i = 0; i < theFactory->fCreators.GetCount(); i++ )
		{
			if( theFactory->fCreators[i] && !_stricmp(className, theFactory->fCreators[i]->ClassName()) )
			{
				return theFactory->fCreators[i]->ClassIndex();
			}
		}
	}
	return numClasses;		// err
}


hsBool plFactory::IIsValidClassIndex(UInt16 hClass)
{
	return ( hClass < fCreators.GetCount() );
}

hsBool plFactory::IsValidClassIndex(UInt16 hClass)
{
	return theFactory->IIsValidClassIndex(hClass);
}

void plFactory::SetTheFactory(plFactory* fac)
{
	// There are four cases here.
	// 1) Our factory is nil, and we're being given one to use
	//		Just take it and ref it.
	// 2) Our factory is non-nil, and we're being given on to use
	//		Ours is bogus, pitch it and use the new one.
	// 3) Our factory is non-nil, and we're being given a nil one
	//		Means we're being shut down. Unref the old one. If
	//		the refcnt drops to one, we're the last one out, so
	//		go ahead and delete it.
	// 4) Our factory is nil and the new one is nil
	//		Shouldn't happen, but if it does, just ignore it.
	if( !theFactory && fac )
	{
		hsRefCnt_SafeAssign(theFactory, fac);
	}
	else if( theFactory && fac )
	{
		theFactory->IForceShutdown();
		hsRefCnt_SafeAssign(theFactory, fac);
	}
	else if( theFactory && !fac )
	{
		hsRefCnt_SafeUnRef(theFactory);
		if( theFactory->RefCnt() < 2 )
			delete theFactory;
		theFactory = nil;
	}

}

plFactory* plFactory::GetTheFactory()
{
	if( !theFactory && !ICreateTheFactory() )
			return nil;

	return theFactory;
}

// For my own nefarious purposes... hsStatusMessage  plCreatableIndex
const char	*plFactory::GetNameOfClass(UInt16 type)
{
	if( type < GetNumClasses() )
	{
		if( theFactory->fCreators[ type ] )
			return theFactory->fCreators[ type ]->ClassName();
		
		static int keyedStringsSize = sizeof(plCreatableStrings::fKeyedStrings)/4;
		// If we don't have a creator yet, try falling back on plCreatableStrings
		if( type < KEYED_OBJ_DELINEATOR && type<keyedStringsSize)
			return plCreatableStrings::fKeyedStrings[ type ];

		if (type < plCreatableIndex::kDatabaseStructIndexesStart)
		{
			static int nonKeyedStringsSize = sizeof(plCreatableStrings::fNonKeyedStrings)/4;
			int idx=type - KEYED_OBJ_DELINEATOR;
			if (idx<nonKeyedStringsSize)
				return plCreatableStrings::fNonKeyedStrings[ idx ];
		}

		static int nonKeyedPostDBStringsSize = sizeof(plCreatableStrings::fNonKeyedPostDBStrings)/4;
		int idx=type - plCreatableIndex::kDatabaseStructIndexesEnd -1;
		if (idx<nonKeyedPostDBStringsSize)
			return plCreatableStrings::fNonKeyedPostDBStrings[ idx ];
	}

	hsAssert(type < GetNumClasses() || type==0xffff,"InValid type");
	return nil;
}

#ifdef HS_DEBUGGING

/*
**
**	Function Name:			Validate
**	Input(s):				Void
**	Output(s):				Void
**	Function Description:	This function examines all the Workers in the Factory and compares their Factory 
**								index to the Enums found in plCreatableIndex.  If they are Keyed objects, and
**								larger than 512 on the Factory index, or non-Keyed objects with a Factory
**								index of less than 512, exit with an Error Message.  Otherwise continue through
**								the iteration of Factory Indices.
**
**
*/

void plFactory::IValidate(UInt16 keyIndex)
{

	int FactoryIndex = GetNumClasses();

	hsBool bogus = false;

	for(int iter=0; iter < FactoryIndex; iter++)
	{
		if (IDerivesFrom(keyIndex, iter))
		{
			if(iter >= KEYED_OBJ_DELINEATOR && theFactory->fCreators[iter])
			{
				char Buffer[512];
				sprintf(Buffer, "Object %s is a hsKeyedObject, Must appear before 'KEYED_OBJ_DELINEATOR' in plCreatableIndex.h\n",GetNameOfClass(iter));
				hsStatusMessage(Buffer);
				bogus = true;
			} 
			
		}
		else
		{
			if(iter < KEYED_OBJ_DELINEATOR && theFactory->fCreators[iter])
			{
				char Buffer[512];
				sprintf(Buffer, "Object %s is NOT a hsKeyedObject, Must appear after 'KEYED_OBJ_DELINEATOR' in plCreatableIndex.h\n",GetNameOfClass(iter));
				hsStatusMessage(Buffer);
				bogus = true;
			
			} 
		}
	}
	hsAssert(!bogus,"The class(s) you just added to plCreatableIndex.h in wrong spot, see output window");

}  

void plFactory::Validate(UInt16 keyIndex)
{
	theFactory->IValidate(keyIndex);

}


#endif
