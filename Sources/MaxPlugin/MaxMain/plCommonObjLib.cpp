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
//  plCommonObjLib - Base class for a library of objects that are stored    //
//                   in common pages (for now, that means Textures or       //
//                   BuiltIn pages).                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "HeadSpin.h"

#include "plCommonObjLib.h"
#include "pnKeyedObject/hsKeyedObject.h"
#include "pnKeyedObject/plUoid.h"
#include "plPluginResManager.h"


//////////////////////////////////////////////////////////////////////////////
//// Static Array And Functions //////////////////////////////////////////////
//  Used only by the export resManager, to create and maintain a list of the
//  commonObjLibs to be used.
//////////////////////////////////////////////////////////////////////////////

class plCommonObjLibList
{
    public:
        uint32_t                      fRefCount;
        std::vector<plCommonObjLib *> fLibs;

        plCommonObjLibList() { fRefCount = 0; }

        void    Add( plCommonObjLib *lib )
        {
            fLibs.emplace_back(lib);
            fRefCount++;
        }

        bool    Remove( plCommonObjLib *lib )
        {
            auto iter = std::find(fLibs.cbegin(), fLibs.cend(), lib);
            if (iter != fLibs.cend())
                fLibs.erase(iter);
            else
            {
                hsAssert( false, "Common Object Lib not found in list upon deletion. Are you misusing this class? Tsk tsk!" );
            }

            fRefCount--;
            return ( fRefCount == 0 ) ? true : false;
        }
};

plCommonObjLibList  *plCommonObjLib::fLibList = nullptr;

size_t plCommonObjLib::GetNumLibs()
{
    return (fLibList != nullptr) ? fLibList->fLibs.size() : 0;
}

plCommonObjLib* plCommonObjLib::GetLib(size_t idx)
{
    if (fLibList == nullptr)
        return nullptr;

    if (idx < fLibList->fLibs.size())
        return fLibList->fLibs[ idx ];

    return nullptr;
}


//////////////////////////////////////////////////////////////////////////////
//// Constructor/Destructor //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

plCommonObjLib::plCommonObjLib()
{
    // Make sure we have a list to add ourselves to
    if (fLibList == nullptr)
        fLibList = new plCommonObjLibList();

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
        fLibList = nullptr;
    }
}


//////////////////////////////////////////////////////////////////////////////
//// Base Utility Functions //////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//// ClearObjectList /////////////////////////////////////////////////////////

void    plCommonObjLib::ClearObjectList()
{
    // Unref our object list, so they'll go away properly
    for (hsKeyedObject* obj : fObjects)
        obj->GetKey()->UnRefObject();
    fObjects.clear();
}

//// AddObject ///////////////////////////////////////////////////////////////
//  Adds the given object to our lib. The object must have a key already.

void    plCommonObjLib::AddObject( hsKeyedObject *object )
{
    if (object == nullptr || object->GetKey() == nullptr)
    {
        hsAssert( false, "Trying to add an object to a commonLib that doesn't have a key" );
        return;
    }

    // Ref it so it won't go away on us
    object->GetKey()->RefObject();
    fObjects.emplace_back(object);
}

//// RemoveObjectAndKey //////////////////////////////////////////////////////
//  Given the key to an object, completely nukes the object and the key. After
//  this function call, the key should no longer exist in the registry and be
//  free to use elsewhere.

bool    plCommonObjLib::RemoveObjectAndKey( plKey &key )
{
    if (!key)
    {
        hsAssert( false, "Received RemoveObjectAndKey() call for a key that is invalid. Nillifying key anyway." );
        key = nullptr;
        return true;
    }
    hsKeyedObject *object = hsKeyedObject::ConvertNoRef( key->ObjectIsLoaded() );
    if (object == nullptr)
    {
        hsAssert( false, "Received RemoveObjectAndKey() call for a key that isn't loaded. Nillifying key anyway." );
        key = nullptr;
        return true;
    }

    auto iter = std::find(fObjects.begin(), fObjects.end(), object);
    if (iter == fObjects.end())
    {
        hsAssert( false, "Trying to RemoveObjectAndKey() for a common object not in the lib." );
        key = nullptr;
        return true;
    }

    // Unref and remove from our list
    (*iter)->GetKey()->UnRefObject();
    fObjects.erase(iter);

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
//  Given a name and an optional class type, tries to find that object in
//  our lib. Returns nullptr if not found. Use to find out if you already have a
//  object of a given name that was previously exported.

hsKeyedObject   *plCommonObjLib::FindObject( const ST::string &name, uint16_t classType /* = -1 */ )
{
    for (hsKeyedObject* obj : fObjects)
    {
        const plUoid    &uoid = obj->GetKey()->GetUoid();


        if( uoid.GetObjectName().compare( name, ST::case_insensitive ) == 0 &&
            ( classType == (uint16_t)-1 || classType == uoid.GetClassType() ) )
        {
            return obj;
        }
    }

    return nullptr;
}
