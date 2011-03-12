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
//	plRegistryHelpers - Little helper classes for the registry and resManager
//
//// History /////////////////////////////////////////////////////////////////
//
//	3.25.2002 mcn	- Created
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _plRegistryHelpers_h
#define _plRegistryHelpers_h

#include "hsTypes.h"
#include "hsTemplates.h"
#include "../pnKeyedObject/plKey.h"

class plKey;
class plRegistryPageNode;

//// Little Iterator Class Defs //////////////////////////////////////////////

class plRegistryKeyIterator 
{
public:
	virtual ~plRegistryKeyIterator() {}
	virtual hsBool EatKey(const plKey& key) = 0;
};

class plRegistryPageIterator 
{
public:
	virtual ~plRegistryPageIterator() {}
	virtual hsBool EatPage(plRegistryPageNode* keyNode) = 0;
};


//// plKeyCollector //////////////////////////////////////////////////////////
//	Helper key iterator that collects the given keys into the given hsTArray
class plKeyCollector : public plRegistryKeyIterator
{
protected:
	hsTArray<plKey> &fKeys;

public:
	plKeyCollector(hsTArray<plKey>& keys);
	virtual hsBool EatKey(const plKey& key);
};

// If you loaded keys with another iterator, this will ensure that they're unloaded
class plIndirectUnloadIterator : public plRegistryPageIterator, public plRegistryKeyIterator
{
public:
	plIndirectUnloadIterator() {}

	hsBool EatKey(const plKey& key) { return true; }

	hsBool EatPage(plRegistryPageNode* page);
};

#endif // _plRegistryHelpers_h
