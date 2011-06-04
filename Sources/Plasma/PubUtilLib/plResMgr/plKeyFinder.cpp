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
#include "plKeyFinder.h"

#include "hsTemplates.h"
#include "hsStlUtils.h"

#include "hsResMgr.h"
#include "plResManager.h"

#include "plRegistryHelpers.h"
#include "plRegistryNode.h"
#include "plRegistryKeyList.h"
#include "plPageInfo.h"
#include "../pnFactory/plFactory.h"
#include "hsUtils.h"
#include "plCreatableIndex.h"

plResManager* IGetResMgr() { return (plResManager*)hsgResMgr::ResMgr(); }

plKeyFinder& plKeyFinder::Instance()
{
	static plKeyFinder theInstance;
	return theInstance;
}

const char* plKeyFinder::GetLastErrorString() // For Console display
{
	// For Console display
	static const char* KeyFinderErrors[] =
	{
		"Ok",
		"Age not found",
		"Page not found",
		"Invalid class",
		"None of those classes in this page",
		"Object not found"
	};
	return KeyFinderErrors[fLastError];
}

//
// Does name string compare with potentially mangled (ie. [1 0 0]foo) names
//
hsBool NameMatches(const char* obName, const char* pKName, hsBool subString)
{
	if (!obName || !pKName)
		return false;

	const char *o = obName;
	const char *p = pKName;

	// If names are mangled, unmangle
	if (*o != '[' || *p != '[')
	{
		// skip past ']' in both names in case mangled
		while (*o && *o != ']')
			o++;
		o = (*o==']') ? o+1 : obName;

		while (*p && *p != ']')
			p++;
		p = (*p==']') ? p+1 : pKName;
	}

	if (!subString)
	{
		if (!_stricmp(o, p))
			return true;			// FOUND IT!!!!!!!!!!!!!!!!!!!
	}
	else
	{
		char oCopy[256], pCopy[256];
		strcpy(oCopy, o);
		strcpy(pCopy, p);
		hsStrLower(oCopy);
		hsStrLower(pCopy);
		if (strstr(pCopy, oCopy))
			return true;
	}

	return false;
}

plKey plKeyFinder::StupidSearch(const char * age, const char * rm, 
								 const char *className, const char *obName, hsBool subString)
{
	UInt16 ty = plFactory::FindClassIndex(className);
	return StupidSearch(age, rm, ty, obName, subString);
}

class plKeyFinderIter : public plRegistryKeyIterator, public plRegistryPageIterator
{
protected:
	UInt16		fClassType;
	const char	*fObjName;
	hsBool		fSubstr;
	plKey		fFoundKey;
	const char	*fAgeName;

public:
	plKey	GetFoundKey( void ) const { return fFoundKey; }

	plKeyFinderIter( UInt16 classType, const char *obName, hsBool substr ) 
			: fFoundKey( nil ), fClassType( classType ), fObjName( obName ), fSubstr( substr ) { }

	plKeyFinderIter( UInt16 classType, const char *obName, hsBool substr, const char *ageName ) 
		: fFoundKey( nil ), fClassType( classType ), fObjName( obName ), fSubstr( substr ),
			fAgeName( ageName ) {}

	virtual hsBool	EatKey( const plKey& key )
	{
		if( key->GetUoid().GetClassType() == fClassType &&
			NameMatches( fObjName, key->GetUoid().GetObjectName(), fSubstr ) )
		{
			fFoundKey = key;
			return false;
		}

		return true;
	}

	virtual hsBool	EatPage( plRegistryPageNode *pageNode )
	{
#ifndef _DEBUG
		try
		{
#endif
			if( stricmp( pageNode->GetPageInfo().GetAge(), fAgeName ) == 0 )
			{
				// Try loading and searching thru this page
				hsTArray<plKey>	keyRefs;

				IGetResMgr()->LoadPageKeys( pageNode );
				plKeyCollector coll( keyRefs );
				pageNode->IterateKeys( &coll );

				if( !pageNode->IterateKeys( this ) )
					return false;
			}
#ifndef _DEBUG
		} catch (...)
		{
		}
#endif
		return true;
	}
};

plKey plKeyFinder::StupidSearch(const char * age, const char * rm, 
								 UInt16 classType, const char *obName, hsBool subString)
{
	if (!obName)
		return nil;

	plUoid newOid;

	fLastError = kOk;

	UInt16 maxClasses = plFactory::GetNumClasses();

	UInt16 ty = classType;
	if (ty == maxClasses)	// error
	{	fLastError = kInvalidClass;
		return nil;
	}	

	if( age != nil && rm != nil )
	{
		const plLocation &loc = IGetResMgr()->FindLocation( age, rm );
		if( !loc.IsValid() )
		{
			fLastError = kPageNotFound;
			return nil;
		}

		plKeyFinderIter	keyFinder( classType, obName, subString );

		if( !IGetResMgr()->IterateKeys( &keyFinder, loc ) )
			// Return value of false means it stopped somewhere, i.e. found something
			return keyFinder.GetFoundKey();
	}
	else if( age != nil )
	{
		plKeyFinderIter	keyFinder(classType, obName, subString, age);

		if( !IGetResMgr()->IterateAllPages( &keyFinder ) )
			return keyFinder.GetFoundKey();
	}
	else
	{
		plKeyFinderIter	keyFinder( classType, obName, subString );

		if( !IGetResMgr()->IterateKeys( &keyFinder ) )
			// Return value of false means it stopped somewhere, i.e. found something
			return keyFinder.GetFoundKey();
	}

	fLastError = kObjectNotFound;
	return nil;
}

void plKeyFinder::ReallyStupidResponderSearch(const char *name, std::vector<plKey>& foundKeys, const plLocation &hintLocation )
{
	ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plResponderModifier), foundKeys, hintLocation);
}

void plKeyFinder::ReallyStupidActivatorSearch(const char *name, std::vector<plKey>& foundKeys, const plLocation &hintLocation)
{
	// use the createable macro so we don't have to pull in all of Python
	ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plLogicModifier), foundKeys, hintLocation);
	ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plPythonFileMod), foundKeys, hintLocation);
	ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plSittingModifier), foundKeys, hintLocation);
}

void plKeyFinder::IGetNames(std::vector<std::string>& names, const char* searchName, int index)
{
	// Not really searching for any particular key, just need all the logic mods
	std::vector<plKey> keys;
	ReallyStupidSubstringSearch(searchName, index, keys);

	for (int i = 0; i < keys.size(); i++)
	{
		// Only allow loaded ones to cut down on the crap
		plKey key = keys[i];
		if (key->ObjectIsLoaded())
			names.push_back(key->GetName());
	}
}

void plKeyFinder::GetResponderNames(std::vector<std::string>& names)
{
	IGetNames(names, "", CLASS_INDEX_SCOPED(plResponderModifier));
}

void plKeyFinder::GetActivatorNames(std::vector<std::string>& names)
{
	IGetNames(names, "", CLASS_INDEX_SCOPED(plLogicModifier));
}

class plKeyFinderIterator : public plRegistryKeyIterator, public plRegistryPageIterator
{
	protected:

		UInt16		fClassType;
		const char	*fObjName;

		std::vector<plKey>	&fFoundKeys;

	public:
	
		plKeyFinderIterator( UInt16 classType, const char *obName, std::vector<plKey>& foundKeys ) 
				: fClassType( classType ), fObjName( obName ), fFoundKeys( foundKeys ) { }

		virtual hsBool	EatKey( const plKey& key )
		{
			if( key->GetUoid().IsValid() && key->GetUoid().GetClassType() == fClassType &&
				strstr( key->GetUoid().GetObjectName(), fObjName ) )
			{
				fFoundKeys.push_back( key );
			}

			return true;
		}

		virtual hsBool EatPage( plRegistryPageNode *page )
		{
			hsBool ret = page->IterateKeys( this );
			return ret;
		}
};

void plKeyFinder::ReallyStupidSubstringSearch(const char *name, UInt16 objType, std::vector<plKey>& foundKeys, const plLocation &hintLocation )
{
	if (!name)
		return;

	plKeyFinderIterator	collector( objType, name, foundKeys );
	if( hintLocation.IsValid() )
	{
		plRegistryPageNode *hintPage = IGetResMgr()->FindPage( hintLocation );
		if( hintPage != nil )
		{
			// Try all pages in the same age as that page
			IGetResMgr()->IteratePages( &collector, hintPage->GetPageInfo().GetAge() );
		}

		if (foundKeys.size() > 0)
		{
			return;
		}
	}

	//fpReg->IterateKeys( &collector );
	IGetResMgr()->IterateAllPages( &collector );
}

//// Helper Class for FindSceneNodeKey ///////////////////////////////////////

class plPageFinder : public plRegistryPageIterator
{
	protected:

		plRegistryPageNode	**fPagePtr;
		const char			*fFindString, *fAgeString;

	public:

		plPageFinder( plRegistryPageNode **page, const char *find ) : fPagePtr( page ), fFindString( find ), fAgeString( nil )
		{ *fPagePtr = nil; }

		plPageFinder( plRegistryPageNode **page, const char *ageS, const char *pageS ) : fPagePtr( page ), fFindString( pageS ), fAgeString( ageS )
		{ *fPagePtr = nil; }

		virtual hsBool	EatPage( plRegistryPageNode *node )
		{
			static char			str[ 512 ];
			const plPageInfo	&info = node->GetPageInfo();

			// Are we searching by age/page?
			if( fAgeString != nil )
			{
				if( stricmp( info.GetAge(), fAgeString ) == 0 && 
					stricmp( info.GetPage(), fFindString ) == 0 )
				{
					*fPagePtr = node;
					return false;
				}
				return true;
			}

			// Try for page only
			if( stricmp( info.GetPage(), fFindString ) == 0 )
			{
				*fPagePtr = node;
				return false;
			}

			// Try for full location
			sprintf( str, "%s_%s_%s", info.GetAge(), info.GetPage() );
			if( stricmp( str, fFindString ) == 0 )
			{
				*fPagePtr = node;
				return false;
			}

			return true;	// Keep searching
		}
};

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//	Given a string for either a page name or a fully qualified location name,
//	finds the page and then returns the key for the sceneNode in that page.
//	Note: in our case, we want to force the page's keys to load if necessary,
//	since the only time we call this function will be to actually load
//	the darned thing.

plKey	plKeyFinder::FindSceneNodeKey( const char *pageOrFullLocName ) const
{
	plRegistryPageNode	*pageNode;
	plPageFinder		pageFinder( &pageNode, pageOrFullLocName );

	
	// Use our own page finder, since we want to do nifty things like partial
	// matches, etc.
	if( IGetResMgr()->IterateAllPages( &pageFinder ) || pageNode == nil )
		return nil;

	return IFindSceneNodeKey( pageNode );
}

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//	Age/page pair version

plKey	plKeyFinder::FindSceneNodeKey( const char *ageName, const char *pageName ) const
{
	plRegistryPageNode	*pageNode;
	plPageFinder		pageFinder( &pageNode, ageName, pageName );

	// Use our own page finder, since we want to do nifty things like partial
	// matches, etc.
	if (IGetResMgr()->IterateAllPages(&pageFinder) || pageNode == nil)
		return nil;

	return IFindSceneNodeKey( pageNode );
}

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//	plLocation version 

plKey plKeyFinder::FindSceneNodeKey(const plLocation& location) const
{
	plRegistryPageNode* pageNode = IGetResMgr()->FindPage(location);
	if (pageNode == nil)
		return nil;

	return IFindSceneNodeKey(pageNode);
}

//// IFindSceneNodeKey ///////////////////////////////////////////////////////


plKey plKeyFinder::IFindSceneNodeKey(plRegistryPageNode* page) const
{
	// Got the pageNode, try a find before loading
	plRegistryKeyList* keyList = page->IGetKeyList(CLASS_INDEX_SCOPED(plSceneNode));
	if (keyList)
	{
		if (keyList->fStaticKeys.size() == 1)
		{
			return plKey::Make((plKeyData*)keyList->fStaticKeys[0]);
		}
		else if (keyList->fDynamicKeys.size() == 1) // happens during export
		{
			plRegistryKeyList::DynSet::const_iterator it = keyList->fDynamicKeys.begin();
			plKeyImp* keyImp = *it;
			return plKey::Make(keyImp);
		}
	}

	// Try loading and see if that helps
	if (page->IsFullyLoaded())
		return nil;

	IGetResMgr()->LoadPageKeys(page);

	// Get the list of all sceneNodes
	plKey retVal(nil);
	keyList = page->IGetKeyList(CLASS_INDEX_SCOPED(plSceneNode));
	if (keyList && keyList->fStaticKeys.size() == 1)
	{
		retVal = plKey::Make((plKeyData*)keyList->fStaticKeys[0]);
	}
	// If we just loaded up all the keys for this page, then we
	// may have a bunch of keys with a refcount of 0. For any of 
	// these keys that nothing else refs (yes, we have unused objects
	// in the data), they don't get deleted because the refcount never
	// rises above zero or falls back to zero. So we'll go ahead and
	// ref and unref all of them. The ones in use stay put, the ones
	// not being used go away. This is less than ideal.
	IGetResMgr()->DumpUnusedKeys(page);

	return retVal;
}

//// FindLocation ////////////////////////////////////////////////////////////

const plLocation	&plKeyFinder::FindLocation( const char *age, const char *page ) const
{
	if (age == nil)
	{
		static plLocation	invalidLoc;
		plRegistryPageNode	*pageNode;
		plPageFinder		pageFinder( &pageNode, page );

		if( IGetResMgr()->IterateAllPages( &pageFinder ) || pageNode == nil )
			return invalidLoc;

		return pageNode->GetPageInfo().GetLocation();
	}

	return IGetResMgr()->FindLocation( age, page );
}

//// GetLocationInfo /////////////////////////////////////////////////////////

const plPageInfo* plKeyFinder::GetLocationInfo( const plLocation &loc ) const
{
	plRegistryPageNode *node = IGetResMgr()->FindPage( loc );
	if (node)
		return &node->GetPageInfo();
	else
		return nil;
}

