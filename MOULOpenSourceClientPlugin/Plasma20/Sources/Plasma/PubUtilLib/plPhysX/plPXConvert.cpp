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
#include "plPXConvert.h"

bool plPXConvert::Validate()
{
	NxVec3 nxVec;
	hsVector3 plVec;

	int nxVecXOffset = ((char*)&nxVec.x) - ((char*)&nxVec);
	int nxVecYOffset = ((char*)&nxVec.y) - ((char*)&nxVec);
	int nxVecZOffset = ((char*)&nxVec.z) - ((char*)&nxVec);

	int plVecXOffset = ((char*)&plVec.fX) - ((char*)&plVec);
	int plVecYOffset = ((char*)&plVec.fY) - ((char*)&plVec);
	int plVecZOffset = ((char*)&plVec.fZ) - ((char*)&plVec);

	NxQuat nxQuat;
	hsQuat plQuat;

	int nxQuatXOffset = ((char*)&nxQuat.x) - ((char*)&nxQuat);
	int nxQuatYOffset = ((char*)&nxQuat.y) - ((char*)&nxQuat);
	int nxQuatZOffset = ((char*)&nxQuat.z) - ((char*)&nxQuat);
	int nxQuatWOffset = ((char*)&nxQuat.w) - ((char*)&nxQuat);

	int plQuatXOffset = ((char*)&plQuat.fX) - ((char*)&plQuat);
	int plQuatYOffset = ((char*)&plQuat.fY) - ((char*)&plQuat);
	int plQuatZOffset = ((char*)&plQuat.fZ) - ((char*)&plQuat);
	int plQuatWOffset = ((char*)&plQuat.fW) - ((char*)&plQuat);

	bool offsetsOK =
		nxVecXOffset == plVecXOffset &&
		nxVecYOffset == plVecYOffset &&
		nxVecZOffset == plVecZOffset &&
		nxQuatXOffset == plQuatXOffset &&
		nxQuatYOffset == plQuatYOffset &&
		nxQuatZOffset == plQuatZOffset &&
		nxQuatWOffset == plQuatWOffset;

	hsAssert(offsetsOK, "PhysX or Plasma offsets have changed, need to rewrite conversion code");

	return offsetsOK;
}
