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

#include "plVersion.h"

#include "plCreatableIndex.h"

#include "pnFactory/plFactory.h"

#include <vector>
#include <cstring>

#define ChangedCreatable(ver, creatable) if (minorVersion == ver) creatables.push_back(CLASS_INDEX_SCOPED(creatable));

//
// Every time you bump the minor version number, add entries here for each
// creatable that was changed.  Every time the minor version is reset (for a
// major version change), delete all the entries.
//
static void GetChangedCreatables(int minorVersion, std::vector<uint16_t>& creatables)
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
        std::vector<uint16_t> changedTypes;
        changedTypes.reserve(10);

        GetChangedCreatables(minorVer, changedTypes);

        for (size_t i = 0; i < changedTypes.size(); i++)
        {
            uint16_t changedType = changedTypes[i];
            CreatableVersions[changedType] = minorVer;

            // Bump any classes that derive from this one
            for (uint16_t toCheck = 0; toCheck < plFactory::GetNumClasses(); toCheck++)
            {
                if (plFactory::DerivesFrom(changedType, toCheck))
                    CreatableVersions[toCheck] = minorVer;
            }
        }
    }
}

uint16_t plVersion::GetMajorVersion() { return PLASMA2_MAJOR_VERSION; }
uint16_t plVersion::GetMinorVersion() { return PLASMA2_MINOR_VERSION; }

int plVersion::GetCreatableVersion(uint16_t creatableIndex)
{
    static bool calced = false;
    if (!calced)
    {
        calced = true;
        CalcCreatableVersions();
    }

    return CreatableVersions[creatableIndex];
}
