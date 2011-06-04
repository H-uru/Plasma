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
#include "plVersion.h"
#include "../pnFactory/plFactory.h"
#include <vector>

#include "plCreatableIndex.h"
#define ChangedCreatable(ver, creatable) if (minorVersion == ver) creatables.push_back(CLASS_INDEX_SCOPED(creatable));

//
// Every time you bump the minor version number, add entries here for each
// creatable that was changed.  Every time the minor version is reset (for a
// major version change), delete all the entries.
//
static void GetChangedCreatables(int minorVersion, std::vector<UInt16>& creatables)
{
	ChangedCreatable(1, plLoadAvatarMsg); 
	ChangedCreatable(1, plArmatureMod);
	ChangedCreatable(2, plAvBrainHuman);
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


static int CreatableVersions[plCreatableIndex::plNumClassIndices];

static void CalcCreatableVersions()
{
	memset(CreatableVersions, 0, sizeof(CreatableVersions));

 	for (int minorVer = 1; minorVer <= PLASMA2_MINOR_VERSION; minorVer++)
	{
		std::vector<UInt16> changedTypes;
		changedTypes.reserve(10);

		GetChangedCreatables(minorVer, changedTypes);

		for (int i = 0; i < changedTypes.size(); i++)
		{
			UInt16 changedType = changedTypes[i];
			CreatableVersions[changedType] = minorVer;

			// Bump any classes that derive from this one
			for (UInt16 toCheck = 0; toCheck < plFactory::GetNumClasses(); toCheck++)
			{
				if (plFactory::DerivesFrom(changedType, toCheck))
					CreatableVersions[toCheck] = minorVer;
			}
		}
	}
}

UInt16 plVersion::GetMajorVersion() { return PLASMA2_MAJOR_VERSION; }
UInt16 plVersion::GetMinorVersion() { return PLASMA2_MINOR_VERSION; }

int plVersion::GetCreatableVersion(UInt16 creatableIndex)
{
	static bool calced = false;
	if (!calced)
	{
		calced = true;
		CalcCreatableVersions();
	}

	return CreatableVersions[creatableIndex];
}
