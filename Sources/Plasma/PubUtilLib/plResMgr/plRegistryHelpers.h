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
//
//  plRegistryHelpers - Little helper classes for the registry and resManager
//
//// History /////////////////////////////////////////////////////////////////
//
//  3.25.2002 mcn   - Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plRegistryHelpers_h
#define _plRegistryHelpers_h

#include "HeadSpin.h"
#include "hsTemplates.h"
#include "pnKeyedObject/plKey.h"

class plKey;
class plRegistryPageNode;
class hsStream;

//// Little Iterator Class Defs //////////////////////////////////////////////

class plRegistryKeyIterator 
{
public:
    virtual ~plRegistryKeyIterator() {}
    virtual bool EatKey(const plKey& key) = 0;
};

class plRegistryPageIterator 
{
public:
    virtual ~plRegistryPageIterator() {}
    virtual bool EatPage(plRegistryPageNode* keyNode) = 0;
};


//// plKeyCollector //////////////////////////////////////////////////////////
//  Helper key iterator that collects the given keys into the given hsTArray
class plKeyCollector : public plRegistryKeyIterator
{
protected:
    hsTArray<plKey> &fKeys;

public:
    plKeyCollector(hsTArray<plKey>& keys);
    virtual bool EatKey(const plKey& key);
};

// If you loaded keys with another iterator, this will ensure that they're unloaded
class plIndirectUnloadIterator : public plRegistryPageIterator, public plRegistryKeyIterator
{
public:
    plIndirectUnloadIterator() {}

    bool EatKey(const plKey& key) { return true; }
    bool EatPage(plRegistryPageNode* page);
};

//// plWriteIterator /////////////////////////////////////////////////////////
//  Key iterator for writing objects
class plWriteIterator : public plRegistryKeyIterator
{
protected:
    hsStream* fStream;

public:
    plWriteIterator() : fStream(nullptr) { }
    plWriteIterator(hsStream* const s) : fStream(s) { }

    void SetStream(hsStream* const s) { fStream = s; }
    virtual bool EatKey(const plKey& key);
};

#endif // _plRegistryHelpers_h
