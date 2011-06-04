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
#include "../pnUtils/pnUtils.h"
#include "../plUnifiedTime/plUnifiedTime.h"
#include "../plVault/plAgeInfoSource.h"
#include "../plVault/plVault.h"
#include "pyDniInfoSource.h"
#include "pyDniCoordinates.h"

pyDniInfoSource::pyDniInfoSource()
:	fAgeName(nil)
{}

pyDniInfoSource::~pyDniInfoSource() {
	FREE(fAgeName);
}

PyObject* pyDniInfoSource::GetAgeCoords( void )
{
#if 0 // this may get retooled for another purpose someday...
	const plDniCoordinateInfo * coords = plNetPlayerVNodeMgr::GetInstance()->GetAgeInfo()->GetAgeCoords();
	if (coords)
		return pyDniCoordinates::New((plDniCoordinateInfo*)coords);
#endif
	// just return a None object
	PYTHON_RETURN_NONE;
}

UInt32 pyDniInfoSource::GetAgeTime( void ) const
{
	RelVaultNode * node = VaultGetAgeInfoNodeIncRef();
	if (!node)
		return 0;
	
	unsigned result;
	VaultAgeInfoNode ageInfo(node);
	if (const plUnifiedTime * utime = ageInfo.GetAgeTime())
		result = utime->GetSecs();
	else
		result = 0;
	node->DecRef();

	return result;
}

const char * pyDniInfoSource::GetAgeName( void ) const
{
	RelVaultNode * node = VaultGetAgeInfoNodeIncRef();
	if (!node)
		return "";

	VaultAgeInfoNode ageInfo(node);

	fAgeName = StrDupToAnsi(ageInfo.ageInstName);
	node->DecRef();

	return fAgeName;
}

const char * pyDniInfoSource::GetAgeGuid( void ) const
{
	RelVaultNode * node = VaultGetAgeInfoNodeIncRef();
	if (!node)
		return "";

	VaultAgeInfoNode ageInfo(node);

	GuidToString(ageInfo.ageInstUuid, fAgeGuid, arrsize(fAgeGuid));
	node->DecRef();

	return fAgeGuid;
}
