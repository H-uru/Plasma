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

#include "pfConsoleCommandUtilities.h"

#include <string_theory/string>

#include "pnFactory/plFactory.h"
#include "pnKeyedObject/plKey.h"
#include "pnSceneObject/plSceneObject.h"

#include "plAgeLoader/plAgeLoader.h"
#include "plNetClient/plNetClientMgr.h"
#include "plResMgr/plKeyFinder.h"

plKey FindObjectByName(const ST::string& name, int type, const ST::string& ageName,
                       ST::string& statusStr, bool subString)
{
    if (name.empty())
    {
        statusStr = ST_LITERAL("Object name is nil");
        return nullptr;
    }
    
    if (type<0 || type>=plFactory::GetNumClasses())
    {
        statusStr = ST_LITERAL("Illegal class type val");
        return nullptr;
    }

    plKey key;
    // Try restricted to our age first, if we're not given an age name. This works
    // around most of the problems associated with unused keys in pages causing the pages to be marked
    // as not loaded and thus screwing up our searches
    if (ageName.empty() && plNetClientMgr::GetInstance() != nullptr)
    {
        ST::string thisAge = plAgeLoader::GetInstance()->GetCurrAgeDesc().GetAgeName();
        if (!thisAge.empty())
        {
            key = plKeyFinder::Instance().StupidSearch(thisAge, ST::string(), type, name, subString);
            if (key != nullptr)
            {
                statusStr = ST_LITERAL("Found Object");
                return key;
            }
        }
    }
    // Fallback
    key = plKeyFinder::Instance().StupidSearch(ageName, ST::string(), type, name, subString);

    if (!key)
    {
        statusStr = ST_LITERAL("Can't find object");
        return nullptr;
    }
    
    if (!key->ObjectIsLoaded())
    {
        statusStr = ST_LITERAL("Object is not loaded");
    }

    statusStr = ST_LITERAL("Found Object");

    return key;
}

plKey FindSceneObjectByName(const ST::string& name, const ST::string& ageName,
                            ST::string& statusStr, bool subString)
{
    plKey key=FindObjectByName(name, plSceneObject::Index(), ageName, statusStr, subString);

    if (!plSceneObject::ConvertNoRef(key ? key->ObjectIsLoaded() : nullptr))
    {
        statusStr = ST_LITERAL("Can't find SceneObject");
        return nullptr;
    }

    return key;
}

plKey FindObjectByNameAndType(const ST::string& name, const char* typeName, const ST::string& ageName,
                              ST::string& statusStr, bool subString)
{
    if (!typeName)
    {
        statusStr = ST_LITERAL("TypeName is nil");
        return nullptr;
    }
    
    return FindObjectByName(name, plFactory::FindClassIndex(typeName), ageName, statusStr, subString);
}
