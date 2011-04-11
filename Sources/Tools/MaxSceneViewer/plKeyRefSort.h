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
#include "hsTypes.h"
#include <vector>
#include "../pnKeyedObject/plKey.h"


// A really crappy sort of a list of keys from the most referenced key to the least.
// This allows you to write out and read in the keys without having to worry about
// one referencing another that isn't loaded yet (ie, in the SceneViewer)
class plKeyRefSort
{
protected:
	class KeyRefs
	{
	public:
		plKey fKey;
		int fNumRefs;
		KeyRefs() : fKey(nil), fNumRefs(-1) {}
		KeyRefs(plKey key, int numRefs) : fKey(key), fNumRefs(numRefs) {}
		hsBool operator== (const plKey key)
		{
			return (fKey == key);
		}
	};

	static std::vector<plKey>* fKeys;
	static std::vector<KeyRefs> fNumRefs;

	friend class RefComp;

	static int CountRefsRecur(plKey key, std::vector<plKey>& traversedKeys);

public:
	static void Sort(std::vector<plKey>* keys);
};
