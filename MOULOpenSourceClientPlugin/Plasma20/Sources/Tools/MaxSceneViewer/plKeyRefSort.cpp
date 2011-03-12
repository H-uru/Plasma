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
#include "plKeyRefSort.h"
#include "../pnKeyedObject/plKeyImp.h"

#include <algorithm>

std::vector<plKey>* plKeyRefSort::fKeys = nil;
std::vector<plKeyRefSort::KeyRefs> plKeyRefSort::fNumRefs;

int plKeyRefSort::CountRefsRecur(plKey key, std::vector<plKey>& traversedKeys)
{
	int numRefs = 0;

	if (std::find(traversedKeys.begin(), traversedKeys.end(), key) == traversedKeys.end())
	{
		traversedKeys.push_back(key);

		plKeyImp* iKey = (plKeyImp*)key;
		for (int i = 0; i < iKey->GetNumRefs(); i++)
		{
			plKey refKey = iKey->GetRef(i);
			if (std::find(fKeys->begin(), fKeys->end(), refKey) != fKeys->end())
				numRefs++;

			numRefs += CountRefsRecur(refKey, traversedKeys);
		}
	}

	return numRefs;
}


class RefComp
{
public:
	bool operator() (plKey key1, plKey key2) const
	{
		std::vector<plKeyRefSort::KeyRefs>::iterator it1 = std::find(plKeyRefSort::fNumRefs.begin(), plKeyRefSort::fNumRefs.end(), key1);
		std::vector<plKeyRefSort::KeyRefs>::iterator it2 = std::find(plKeyRefSort::fNumRefs.begin(), plKeyRefSort::fNumRefs.end(), key2);

		return ((*it1).fNumRefs < (*it2).fNumRefs);
	}
};

void plKeyRefSort::Sort(std::vector<plKey>* keys)
{
	fKeys = keys;
	int numKeys = keys->size();
	fNumRefs.resize(numKeys);

	int i;
	for (i = 0; i < numKeys; i++)
	{
		plKey curKey = (*keys)[i];

		std::vector<plKey> traversedKeys;
		int numRefs = CountRefsRecur(curKey, traversedKeys);

		fNumRefs[i] = KeyRefs(curKey, numRefs);
	}

	std::sort(fKeys->begin(), fKeys->end(), RefComp());
}