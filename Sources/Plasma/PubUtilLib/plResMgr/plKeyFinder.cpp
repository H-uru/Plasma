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
#include "plKeyFinder.h"
#include <string_theory/format>

#include "plCreatableIndex.h"
#include "hsResMgr.h"

#include "plPageInfo.h"
#include "plRegistryHelpers.h"
#include "plRegistryKeyList.h"
#include "plRegistryNode.h"
#include "plResManager.h"

#include "pnFactory/plFactory.h"
#include "pnKeyedObject/plKeyImp.h"

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
bool NameMatches(const char* obName, const char* pKName, bool subString)
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
        if (!stricmp(o, p))
            return true;            // FOUND IT!!!!!!!!!!!!!!!!!!!
    }
    else
    {
        if (ST::string(p).contains(o, ST::case_insensitive))
            return true;
    }

    return false;
}

plKey plKeyFinder::StupidSearch(const ST::string & age, const ST::string & rm,
                                 const char *className, const ST::string &obName, bool subString)
{
    uint16_t ty = plFactory::FindClassIndex(className);
    return StupidSearch(age, rm, ty, obName, subString);
}

class plKeyFinderIter : public plRegistryKeyIterator, public plRegistryPageIterator
{
protected:
    uint16_t    fClassType;
    ST::string  fObjName;
    bool        fSubstr;
    plKey       fFoundKey;
    ST::string  fAgeName;

public:
    plKey   GetFoundKey() const { return fFoundKey; }

    plKeyFinderIter( uint16_t classType, const ST::string &obName, bool substr )
        : fFoundKey(nullptr), fClassType(classType), fObjName(obName), fSubstr(substr) { }

    plKeyFinderIter( uint16_t classType, const ST::string &obName, bool substr, const ST::string &ageName )
        : fFoundKey(nullptr), fClassType(classType), fObjName(obName), fSubstr(substr),
            fAgeName( ageName ) {}

    bool  EatKey(const plKey& key) override
    {
        if( key->GetUoid().GetClassType() == fClassType &&
            NameMatches( fObjName.c_str(), key->GetUoid().GetObjectName().c_str(), fSubstr ) )
        {
            fFoundKey = key;
            return false;
        }

        return true;
    }

    bool  EatPage(plRegistryPageNode *pageNode) override
    {
#ifndef HS_DEBUGGING
        try
        {
#endif
            if (pageNode->GetPageInfo().GetAge().compare_i(fAgeName) == 0)
            {
                // Try loading and searching thru this page
                std::set<plKey> keyRefs;

                IGetResMgr()->LoadPageKeys( pageNode );
                plKeyCollector coll( keyRefs );
                pageNode->IterateKeys( &coll );

                if( !pageNode->IterateKeys( this ) )
                    return false;
            }
#ifndef HS_DEBUGGING
        } catch (...)
        {
        }
#endif
        return true;
    }
};

plKey plKeyFinder::StupidSearch(const ST::string & age, const ST::string & rm,
                                 uint16_t classType, const ST::string &obName, bool subString)
{
    if (obName.empty())
        return nullptr;

    plUoid newOid;

    fLastError = kOk;

    uint16_t maxClasses = plFactory::GetNumClasses();

    uint16_t ty = classType;
    if (ty == maxClasses)   // error
    {
        fLastError = kInvalidClass;
        return nullptr;
    }

    if (!age.empty() && !rm.empty())
    {
        const plLocation &loc = IGetResMgr()->FindLocation( age, rm );
        if( !loc.IsValid() )
        {
            fLastError = kPageNotFound;
            return nullptr;
        }

        plKeyFinderIter keyFinder( classType, obName, subString );

        if( !IGetResMgr()->IterateKeys( &keyFinder, loc ) )
            // Return value of false means it stopped somewhere, i.e. found something
            return keyFinder.GetFoundKey();
    }
    else if (!age.empty())
    {
        plKeyFinderIter keyFinder(classType, obName, subString, age);

        if( !IGetResMgr()->IterateAllPages( &keyFinder ) )
            return keyFinder.GetFoundKey();
    }
    else
    {
        plKeyFinderIter keyFinder( classType, obName, subString );

        if( !IGetResMgr()->IterateKeys( &keyFinder ) )
            // Return value of false means it stopped somewhere, i.e. found something
            return keyFinder.GetFoundKey();
    }

    fLastError = kObjectNotFound;
    return nullptr;
}

void plKeyFinder::ReallyStupidResponderSearch(const ST::string &name, std::vector<plKey>& foundKeys, const plLocation &hintLocation )
{
    ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plResponderModifier), foundKeys, hintLocation);
}

void plKeyFinder::ReallyStupidActivatorSearch(const ST::string &name, std::vector<plKey>& foundKeys, const plLocation &hintLocation)
{
    // use the createable macro so we don't have to pull in all of Python
    ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plLogicModifier), foundKeys, hintLocation);
    ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plPythonFileMod), foundKeys, hintLocation);
    ReallyStupidSubstringSearch(name, CLASS_INDEX_SCOPED(plSittingModifier), foundKeys, hintLocation);
}

void plKeyFinder::IGetNames(std::vector<ST::string>& names, const ST::string& searchName, int index)
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

void plKeyFinder::GetResponderNames(std::vector<ST::string>& names)
{
    IGetNames(names, ST::string(), CLASS_INDEX_SCOPED(plResponderModifier));
}

void plKeyFinder::GetActivatorNames(std::vector<ST::string>& names)
{
    IGetNames(names, ST::string(), CLASS_INDEX_SCOPED(plLogicModifier));
}

class plKeyFinderIterator : public plRegistryKeyIterator, public plRegistryPageIterator
{
    protected:

        uint16_t    fClassType;
        ST::string  fObjName;

        std::vector<plKey>  &fFoundKeys;

    public:
    
        plKeyFinderIterator( uint16_t classType, const ST::string &obName, std::vector<plKey>& foundKeys )
                : fClassType( classType ), fObjName( obName ), fFoundKeys( foundKeys ) { }

        bool  EatKey(const plKey& key) override
        {
            if( key->GetUoid().IsValid() && key->GetUoid().GetClassType() == fClassType &&
                key->GetUoid().GetObjectName().contains( fObjName ) )
            {
                fFoundKeys.push_back( key );
            }

            return true;
        }

        bool EatPage(plRegistryPageNode *page) override
        {
            bool ret = page->IterateKeys( this );
            return ret;
        }
};

void plKeyFinder::ReallyStupidSubstringSearch(const ST::string &name, uint16_t objType, std::vector<plKey>& foundKeys, const plLocation &hintLocation )
{
    if (name.empty())
        return;

    plKeyFinderIterator collector( objType, name, foundKeys );
    if( hintLocation.IsValid() )
    {
        plRegistryPageNode *hintPage = IGetResMgr()->FindPage( hintLocation );
        if (hintPage != nullptr)
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

        plRegistryPageNode  **fPagePtr;
        ST::string          fFindString, fAgeString;

    public:

        plPageFinder( plRegistryPageNode **page, const ST::string &find ) : fPagePtr( page ), fFindString( find )
        { *fPagePtr = nullptr; }

        plPageFinder( plRegistryPageNode **page, const ST::string &ageS, const ST::string &pageS ) : fPagePtr( page ), fFindString( pageS ), fAgeString( ageS )
        { *fPagePtr = nullptr; }

        bool  EatPage(plRegistryPageNode *node) override
        {
            const plPageInfo    &info = node->GetPageInfo();

            // Are we searching by age/page?
            if (!fAgeString.empty())
            {
                if (info.GetAge().compare_i(fAgeString) == 0 && info.GetPage().compare_i(fFindString) == 0)
                {
                    *fPagePtr = node;
                    return false;
                }
                return true;
            }

            // Try for page only
            if (info.GetPage().compare_i(fFindString) == 0)
            {
                *fPagePtr = node;
                return false;
            }

            // Try for full location
            if (ST::format("{}_{}", info.GetAge(), info.GetPage()).compare_i(fFindString) == 0)
            {
                *fPagePtr = node;
                return false;
            }

            return true;    // Keep searching
        }
};

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//  Given a string for either a page name or a fully qualified location name,
//  finds the page and then returns the key for the sceneNode in that page.
//  Note: in our case, we want to force the page's keys to load if necessary,
//  since the only time we call this function will be to actually load
//  the darned thing.

plKey   plKeyFinder::FindSceneNodeKey( const ST::string &pageOrFullLocName ) const
{
    plRegistryPageNode  *pageNode;
    plPageFinder        pageFinder( &pageNode, pageOrFullLocName );

    
    // Use our own page finder, since we want to do nifty things like partial
    // matches, etc.
    if (IGetResMgr()->IterateAllPages(&pageFinder) || pageNode == nullptr)
        return nullptr;

    return IFindSceneNodeKey( pageNode );
}

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//  Age/page pair version

plKey   plKeyFinder::FindSceneNodeKey( const ST::string &ageName, const ST::string &pageName ) const
{
    plRegistryPageNode  *pageNode;
    plPageFinder        pageFinder( &pageNode, ageName, pageName );

    // Use our own page finder, since we want to do nifty things like partial
    // matches, etc.
    if (IGetResMgr()->IterateAllPages(&pageFinder) || pageNode == nullptr)
        return nullptr;

    return IFindSceneNodeKey( pageNode );
}

//// FindSceneNodeKey ////////////////////////////////////////////////////////
//  plLocation version 

plKey plKeyFinder::FindSceneNodeKey(const plLocation& location) const
{
    plRegistryPageNode* pageNode = IGetResMgr()->FindPage(location);
    if (pageNode == nullptr)
        return nullptr;

    return IFindSceneNodeKey(pageNode);
}

//// IFindSceneNodeKey ///////////////////////////////////////////////////////


plKey plKeyFinder::IFindSceneNodeKey(plRegistryPageNode* page) const
{
    // Got the pageNode, try a find before loading
    plRegistryKeyList* keyList = page->IGetKeyList(CLASS_INDEX_SCOPED(plSceneNode));
    if (keyList)
    {
        if (keyList->fKeys.size() == 1)
        {
            return plKey::Make(keyList->fKeys[0]);
        }
    }

    // Try loading and see if that helps
    if (page->IsFullyLoaded())
        return nullptr;

    IGetResMgr()->LoadPageKeys(page);

    // Get the list of all sceneNodes
    plKey retVal;
    keyList = page->IGetKeyList(CLASS_INDEX_SCOPED(plSceneNode));
    if (keyList && keyList->fKeys.size() == 1)
    {
        retVal = plKey::Make(keyList->fKeys[0]);
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

const plLocation    &plKeyFinder::FindLocation(const ST::string &age, const ST::string &page) const
{
    if (age.empty())
    {
        static plLocation   invalidLoc;
        plRegistryPageNode *pageNode;
        plPageFinder        pageFinder( &pageNode, page );

        if (IGetResMgr()->IterateAllPages(&pageFinder) || pageNode == nullptr)
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
        return nullptr;
}

